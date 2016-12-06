//
//  hmm.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/24/16.
//
//

#include "hmm.hpp"

hmm_state::hmm_state (phspec p) : ctx(phone::context(p.first)){
    ph = p;
}

float hmm_state::get_prob (int i) const {
    return tr[i].second;
}

void hmm_state::set_prob (int i, float p) {
    tr[i].second = p;
}

hmm::hmm (vector<phone::phone> ph, acoustic_model &ac, state_model &smo) : acm(ac), sm(smo) {
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

        }
        states[i].tr.push_back (transition(&states[i], LOG_HALF));      // self loop
        states[i].tr.push_back (transition(&states[i+1], LOG_HALF));    // advance
    }
}

// should we use logprobs for all the transition probabilities?
// currently uses unigram probabilities
hmm::hmm (pronlex pr, unigram_model uni, acoustic_model &ac, state_model &sm) : acm(ac), sm(sm){
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
        for (int i=s; i < states.size()-2; i++) { // loop over states we just created for this word
            bool last = i == states.size()-2;
            states[i].tr.push_back (transition(&states[i], last ? LOG_THIRD : LOG_HALF));
            states[i].tr.push_back (transition(&states[i+1], last ? LOG_THIRD : LOG_HALF));
            if (last) states[i].tr.push_back (transition(&states[0], LOG_THIRD));
        }
        int e = states.size()-1;    // terminating silence state
        states[e].tr.push_back (transition(&states[e], LOG_HALF)); // self loop
        states[e].tr.push_back (transition(&states[0], LOG_HALF)); // return to start state, ready for next word

    }
}

typedef map<hmm_state*, float> layer;
typedef map<hmm_state*, hmm_state*> backptr_map;
typedef pair<hmm_state*, float> cell;
typedef layer::iterator liter;
typedef vector<transition>::iterator triter;
typedef list<hmm_state*> path;

/* The heart of the speech recognizer */

// where does the language model scaling factor come in?
path hmm::viterbi (vector<featurevec*> fvs, float beam_width=1e10) {
    layer map1, map2;
    layer &curr = map1;
    layer &next = map2;
    path path;
    vector<backptr_map> back;
    float current_best_logp = -1e10f;
    curr[&states[0]] = 0;   // initialize to just contain the start state, probability 1
    
    for (int i=0; i < fvs.size(); i++) {
        backptr_map bk;
        float next_best_logp = -1e10f;
        for (liter it = curr.begin(); it != curr.end(); it++) {   // loop over active states
            hmm_state *src = it->first;
            float logp = it->second;  // likelihood of best path up to src
            if (logp < current_best_logp - beam_width) continue;    // early abort for unpromising candidates
            for (triter t = src->tr.begin(); t != src->tr.end(); t++) { // loop over transitions from src
                logp += t->second;    // multiply by transition probability
                logp += acm(*fvs[i], t->first->ctx);    // multiply by the acoustic model probability of observing feature vector i given the target state's phoneme
                // now look up the target state in 'next'
                hmm_state *target = t->first;
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

void hmm::train (vector<utterance> ut) {
    vector<path> paths;
    for (int i=0; i < ut.size(); i++) {
        paths.push_back(viterbi (ut[i].features));
    }
}

// update the transitions to reflect the state model
// TODO: use logprobs, or make other stuff not use them.
void hmm::update_states () {
    for (int i=0; i < states.size(); i++) {
        float p = sm(states[i].ctx);
        if (states[i].tr.size() == 2) {
            states[i].set_prob(0, 1-p);   // self loop
            states[i].set_prob(1, p);
        } else if (states[i].tr.size() == 3) {
            states[i].set_prob(0, 1-p); // self
            states[i].set_prob(1, (1-SIL_SKIP_P)*p);    // silence
            states[i].set_prob(2, SIL_SKIP_P*p);    // skip
        } else {    // it's the start state in the big model; don't change anything
        }
        
    }
}
