//
//  hmm.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/24/16.
//
//

#include "hmm.hpp"
#include "corpus.hpp"
#include <algorithm>

hmm_state::hmm_state (phspec p) : ctx(phone::context(p.first)){
    ph = p;
}

float hmm_state::get_prob (int i) const {
    return tr[i].second;
}

void hmm_state::set_prob (int i, float p) {
    tr[i].second = log(p);
}

void hmm_state::set_adv_prob (float p) {
    float avail = tr.size() == 3 ? (1 - SIL_SKIP_P) : 1;
    set_prob (0, (1-p)*avail);
    set_prob (1, p*avail);
}

float state_model::operator() (hmm_state *state) {
    if (ties == NULL) return state->get_prob(1);    // hmm, this is a logprob, isn't it
    phone::context c = (*ties)(state->ctx);
    if (counts.count(c) == 0) return 0.5f;
    return probsums[c] / counts[c];
}

void state_model::augment (hmm_state *state) {
    if (ties == NULL) return;
    phone::context c = (*ties)(state->ctx);
    probsums[c] += state->get_prob(1);
    counts[c] += 1;
}

hmm::hmm (vector<phone::phone> ph, acoustic_model *ac) : acm(ac), temp_acm(NULL), sm(new state_model(NULL)) {
    for (int i=0; i < ph.size(); i++) {
        if (USE_SUBPHONES and ph[i] != phone::SIL) {
            states.push_back (hmm_state(phspec(ph[i], phone::BEGIN)));
            states.push_back (hmm_state(phspec(ph[i], phone::MIDDLE)));
            states.push_back (hmm_state(phspec(ph[i], phone::END)));
        } else {
            states.push_back (hmm_state(phspec(ph[i], phone::MIDDLE)));
        }
    }
    for (int i=0; i < states.size()-1; i++) {
        if (states[i+1].ph.first == phone::SIL and i+2 < states.size()) {
            states[i].tr.push_back (transition(&states[i], LOG_THIRD));     // self loop
            states[i].tr.push_back (transition(&states[i+1], LOG_THIRD));   // go to silence
            states[i].tr.push_back (transition(&states[i+2], LOG_THIRD));   // skip past silence

        } else if (i+1 < states.size()){
            states[i].tr.push_back (transition(&states[i], LOG_HALF));      // self loop
            states[i].tr.push_back (transition(&states[i+1], LOG_HALF));    // advance
        }
    }
    int e = states.size()-1;
    states[e].tr.push_back (transition(&states[e], 0)); // self loop, probability 1 for last state

    for (int i=0; i < states.size(); i++) {
        phone::phone prev = (i == 0) ? phone::SIL : states[i-1].ph.first;
        phone::phone next = (i+1 == states.size()) ? phone::SIL : states[i+1].ph.first;
        states[i].ctx = phone::context (prev, states[i].ph.first, next);
    }
}

// should we use logprobs for all the transition probabilities?
// currently uses unigram probabilities
hmm::hmm (pronlex &pr, unigram_model &uni, acoustic_model *ac, state_model *sm) : acm(ac), sm(sm){
    temp_acm = new acoustic_model (acm);   // copy
    states.push_back (hmm_state(phspec(phone::SIL, phone::BEGIN))); // this doesn't really represent silence, it's just the start state.
    for (map<string,pronlist>::iterator iter = pr.pr.begin(); iter != pr.pr.end(); iter++) {
        vector<phone::phone> p = (*iter).second.pronunciations[0]; // for now, ignore alternate pronunciations
        int s = states.size();  // keep track of the index of the first state in the word
        for (int i=0; i < p.size(); i++) {
            if (USE_SUBPHONES) {
                states.push_back (hmm_state (phspec(p[i], phone::BEGIN)));
                states.push_back (hmm_state (phspec(p[i], phone::MIDDLE)));
                states.push_back (hmm_state (phspec(p[i], phone::END)));
            } else {
                states.push_back (hmm_state (phspec(p[i], phone::MIDDLE)));
            }
        }
        states.push_back (hmm_state (phspec(phone::SIL, phone::MIDDLE)));
        states[0].tr.push_back(transition(&states[s], LMSF * uni.get((*iter).first)));
        for (int i=s; i < states.size()-1; i++) { // loop over states we just created for this word
            bool last = i == states.size()-2;
            states[i].tr.push_back (transition(&states[i], last ? LOG_THIRD : LOG_HALF));
            states[i].tr.push_back (transition(&states[i+1], last ? LOG_THIRD : LOG_HALF));
            if (last) states[i].tr.push_back (transition(&states[0], LOG_THIRD));
            
            phone::phone prev = (i == s) ? phone::SIL : states[i-1].ph.first;
            phone::phone next = states[i+1].ph.first;
            states[i].ctx = phone::context (prev, states[i].ph.first, next);
            states[i].word = &(iter->first);
        }
        int e = states.size()-1;    // terminating silence state
        states[e].tr.push_back (transition(&states[e], LOG_HALF)); // self loop
        states[e].tr.push_back (transition(&states[0], LOG_HALF)); // return to start state, ready for next word
        states[e].ctx = phone::context (states[e-1].ph.first, phone::SIL, phone::SIL);
        states[e].word = &(iter->first);
    }
}

typedef map<int, float> layer;
typedef map<int, int> backptr_map;
typedef pair<int, float> cell;
typedef layer::const_iterator liter;
typedef vector<transition>::const_iterator triter;

int hmm::indexOf (hmm_state *s) {
    for (int i = 0; i < states.size(); i++) {
        if (&states[i] == s) return i;
    }
}

path hmm::viterbi (vector<featurevec*> fvs, float beam_width=1e10) {
    layer map1, map2;
    layer &curr = map1;
    layer &next = map2;
    path path;
    vector<backptr_map> back;
    float current_best_logp = -1e10f;
    curr[0] = 0;   // initialize to just contain the start state, probability 1
    
    for (int i=0; i < fvs.size(); i++) {
 //       cout << "layer " << i << "; " << curr.size() << " active states" << endl;
        backptr_map bk;
        float next_best_logp = -1e10f;
        for (liter it = curr.begin(); it != curr.end(); it++) {   // loop over active states
            hmm_state &src = states[it->first];
            float logp = it->second;  // likelihood of best path up to src
            if (logp < current_best_logp - beam_width) continue;    // early abort for unpromising candidates
            for (int ti = 0; ti < src.tr.size(); ti++) { // loop over transitions from src
                transition &t = src.tr[ti];
                logp += t.second;    // multiply by transition probability
                logp += (*acm)(*fvs[i], t.first->ctx);    // multiply by the acoustic model probability of observing feature vector i given the target state's phone
                // now look up the target state in 'next'
                int target = indexOf(t.first);
                if (next.count(target) == 1) {
                    // it was found. If logp is better than what's stored there, update.
                    if (logp > next[target]) {
                        next[target] = logp;
                        next_best_logp = max(next_best_logp, logp);
                        bk[target] = it->first;
                    }
                } else {
                    next[target] = logp;    // this will create an entry for target
                    next_best_logp = max(next_best_logp, logp);
                    bk[target] = it->first;
                }
            }
        }
        back.push_back(bk);
        current_best_logp = next_best_logp;
        layer &temp = curr;
        curr = next;
        next = temp;
        next.clear();
    }
    // ok, trellis has been computed and backpointer maps are filled. now reconstruct the path
    // first find the best state in the last trellis layer
    /*
    float best_logp = -1e10f;
    int best = -1;
    for (liter it = curr.begin(); it != curr.end(); it++) {
        if (it->second > best_logp) {
            best_logp = it->second;
            best = it->first;
        }
    }
    if (best == -1) {
        throw 1729;
        // uh oh. all paths got pruned -- beam width too small (or something majorly wrong)
    }
     */
    int best = states.size()-1;  // WARNING! THIS IS ONLY FOR TRAINING!
    cout << "Viterbi path has logprob " << curr[best] << endl;
    for (int i=fvs.size()-1; i >= 0; i--) {
        path.push_front (&states[best]);
        best = back[i][best];
    }
    return path;
}

/* The heart of the speech recognizer */
/*
typedef map<hmm_state*, float> layer;
typedef map<hmm_state*, hmm_state*> backptr_map;
typedef pair<hmm_state*, float> cell;
typedef layer::const_iterator liter;
typedef vector<transition>::const_iterator triter;
path hmm::viterbi_old (vector<featurevec*> fvs, float beam_width=1e10) {
    layer map1, map2;
    layer &curr = map1;
    layer &next = map2;
    path path;
    vector<backptr_map> back;
    float current_best_logp = -1e10f;
    curr[&states[0]] = 0;   // initialize to just contain the start state, probability 1
    
    for (int i=0; i < fvs.size(); i++) {
        cout << "layer " << i << "; " << curr.size() << " active states" << endl;
        backptr_map bk;
        float next_best_logp = -1e10f;
        for (liter it = curr.begin(); it != curr.end(); it++) {   // loop over active states
            hmm_state *src = it->first;
            float logp = it->second;  // likelihood of best path up to src
            if (logp < current_best_logp - beam_width) continue;    // early abort for unpromising candidates
            for (int ti = 0; ti < src->tr.size(); ti++) { // loop over transitions from src
                transition &t = src->tr[ti];
                logp += t.second;    // multiply by transition probability
                logp += (*acm)(*fvs[i], t.first->ctx);    // multiply by the acoustic model probability of observing feature vector i given the target state's phone
                // now look up the target state in 'next'
                hmm_state *target = t.first;
                if (next.count(target) == 1) {
                    // it was found. If logp is better than what's stored there, update.
                    if (logp > next[target]) {
                        next[target] = logp;
                        next_best_logp = max(next_best_logp, logp);
                        bk[target] = src;
                    }
                } else {
                    next[target] = logp;    // this will create an entry for target
                    next_best_logp = max(next_best_logp, logp);
                    bk[target] = src;
                }
            }
        }
        back.push_back(bk);
        current_best_logp = next_best_logp;
        layer &temp = curr;
        curr = next;
        next = temp;
        next.clear();
    }
    // ok, trellis has been computed and backpointer maps are filled. now reconstruct the path
    // first find the best state in the last trellis layer
    float best_logp = -1e10f;
    hmm_state *best;
    for (liter it = curr.begin(); it != curr.end(); it++) {
        if (it->second > best_logp) {
            best_logp = it->second;
            best = it->first;
        }
    }
    if (best == NULL) {
        throw 1729;
        // uh oh. all paths got pruned -- beam width too small (or something majorly wrong)
    }
    for (int i=fvs.size()-1; i >= 0; i--) {
        path.push_front (best);
        best = back[i][best];
    }
    return path;
}
*/
// this is applied to the training utterance HMMs. In these, there is no way to go
// back to a previous state.
void hmm::train_transition_probabilities (path p) {
    hmm_state *curr = p.front();
    int count = 1;
    path::iterator it = p.begin();
    it++;
    while (it != p.end()) {
        if (*it != curr) {
            curr->set_adv_prob (min(MAX_ADV_PROB,1.0f/count));
            count = 0;
            curr = *it;
        }
        count++;
        it++;
    }
}

typedef pair<phone::context, int> mixcomp;

#define EM_ITERS 4
// this is called on the lexicon HMM
void hmm::train (vector<utterance> &ut) {
    vector<path> paths;
    int em_iter = 0;
    while (em_iter++ < EM_ITERS) {
        cout << "-------------------\n----------------------Iteration " << em_iter << "-------------------------\n--------------------" << endl;
        // compute Viterbi paths for each utterance
        paths.clear();
        for (int i=0; i < ut.size(); i++) {
            path p = ut[i].hmm->viterbi (ut[i].features);
            paths.push_back(p);
            for (path::iterator li = p.begin(); li != p.end(); li++ ) {
                cout << phone::names[(*li)->ph.first] << " ";
            }
            cout << endl;
        }
        // now update the transition probabilities for the utterance HMMs
        for (int i=0; i < ut.size(); i++) {
            ut[i].hmm->train_transition_probabilities (paths[i]);
        }
        // now update the mixture parameters
        // compute zeta values
        temp_acm->clear_zetas();
        for (int i=0; i < ut.size(); i++) {
            path::iterator it = paths[i].begin();
            int obs_idx = 0;
            while (it != paths[i].end()) {
                phone::context rep = acm->ties((*it)->ctx);
                gmm *g = acm->mixtures[rep];
                for (int m=0; m < g->gaussians.size(); m++) {
                    temp_acm->mixtures[rep]->gaussians[m].zetasum += g->gaussians[m].weight * g->gaussians[m].prob(*ut[i].features[obs_idx]);
                }
                it++;
                obs_idx ++;
            }
        }
        temp_acm->sum_zetas();
        temp_acm->clear();
        // compute the weights
        for (acm_iter it = temp_acm->begin(); it != temp_acm->end(); it++) {
            gmm *g = it->second;
            for (int i=0; i < g->gaussians.size(); i++) {
                g->gaussians[i].set_wt(g->gaussians[i].zetasum / g->zetasum);
            }
        }
        // compute the means
        for (int i=0; i < ut.size(); i++) {
            path::iterator it = paths[i].begin();
            int obs_idx = 0;
            while (it != paths[i].end()) {
                phone::context rep = acm->ties((*it)->ctx);
                gmm *g = acm->mixtures[rep];
                for (int k = 0; k < g->gaussians.size(); k++) {
                    featurevec &fv = *ut[i].features[obs_idx];
                    gaussian &comp = g->gaussians[k];
                    float zeta = comp.weight * comp.prob(fv);
                    gaussian &updated = temp_acm->mixtures[rep]->gaussians[k];
                    for (int e=0; e < FV_LEN; e++) {
                        updated.mean[e] += zeta * fv[e];
                    }
                }
                it++;
                obs_idx++;
            }
        }
        temp_acm->divide_means();
        // computing the variances requires using the new means, so that can't be rolled into the above loops.
        for (int i=0; i < ut.size(); i++) {
            path::iterator it = paths[i].begin();
            int obs_idx = 0;
            while (it != paths[i].end()) {
                phone::context rep = acm->ties((*it)->ctx);
                gmm *g = acm->mixtures[rep];
                for (int k = 0; k < g->gaussians.size(); k++) {
                    featurevec &fv = *ut[i].features[obs_idx];
                    gaussian &comp = g->gaussians[k];
                    float zeta = comp.weight * comp.prob(fv);
                    gaussian &updated = temp_acm->mixtures[rep]->gaussians[k];
                    for (int e=0; e < FV_LEN; e++) {
                        float diff = fv[e] - updated.mean[e];
                        updated.var[e] += zeta * diff*diff;
                    }
                }
                it++;
                obs_idx++;
            }
        }
        temp_acm->divide_variances();
        // blend in the current acm
        temp_acm->blend (acm, 0.2f);    // second param is how much of temp_acm to keep. so this is 80% old, 20% new
        
        // swap references
        acoustic_model *temp = acm;
        acm = temp_acm;
        temp_acm = temp;
    }
    // acoustic model is ready to go. But we still need to set the lexicon HMM parameters
    // based on all of the training HMMs.
    for (int i=0; i < ut.size(); i++) {
        path::iterator it = paths[i].begin();
        while (it != paths[i].end()) {
            sm->augment (*it);
            it++;
        }
    }
    update_states();
}

// update the transitions to reflect the state model
void hmm::update_states () {
    for (int i=1; i < states.size(); i++) { // skip the start state
        float p = (*sm)(&states[i]);
        states[i].set_adv_prob(p);
    }
}
