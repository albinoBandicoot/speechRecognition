//
//  hmm.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/24/16.
//
//

#ifndef hmm_hpp
#define hmm_hpp

#include <stdio.h>
#include <vector>
#include <utility>
#include "pronounce.hpp"
#include "gmm.hpp"
#include "mfcc.hpp"
#include "langmodel.hpp"

/* This is the heart of the speech recognizer. In this file are the structures for the HMMs, and the
   methods for decoding and training. */

#define USE_SUBPHONES false // whether to use subphone states or not

#define LOG_HALF -0.6931471805f
#define LOG_THIRD -1.09861228866f

#define SIL_SKIP_P 0.3f     // probability of skipping end-of-word silence
#define MAX_ADV_PROB 0.55f  // cap the probability of advancing to the next state to avoid getting stuck at P(advance) = 1 in the training
#define HMM_BLEND_FAC 0.2f  // how much to blend in the updated estimates of the HMM parameters into the existing ones
#define GMM_BLEND_FAC 0.2f  // same, for GMM parameters

#define LMSF 5      // language model scaling factor: controls the relative importance of language model and acoustic model influences
class hmm_state;

typedef pair<hmm_state*, float> transition;
typedef list<hmm_state*> path;

// one state in the hidden markov model
class hmm_state {
public:
    phspec ph;              // what phone (and subphone, if used) this state represents
    phone::context ctx;     // the context of this state (surrounding phones)
    vector<transition> tr;  // transitions and log probabilities
    const string *word;     // what word this state is part of (not needed for the training HMMs)
    
    hmm_state (phspec p);
    
    float get_logprob (int i) const;
    void set_prob (int i, float p);
    float get_adv_prob () const;
    void set_adv_prob (float p);
};


// represents the parameter tying and updated transition probabilities of the HMM. It is a place to store
// the partial results as they are being computed so the model itself is not in a partially-updated state
// while the training is happening.
class state_model {
public:
    phone::ties *ties;
    map<phone::context, float> probsums;   // probability of transition to next state. self loop probability is 1 - advance_prob. Not sure what to do about word-end nodes that have 3 transitions (self, advance to silence, skip silence)
    map<phone::context, int> counts;
    
    state_model (phone::ties *t) : ties(t) {}
    
    float operator() (hmm_state *state);
    void augment (hmm_state *state);
    void set (hmm_state *state, float v);
};

class utterance;

// The HMM itself. Contains a list of states, a map of state pointers to indices, a reference to the acoustic model (and
// a temporary acoustic model, where the training modifications happen so they do not disrupt the existing model as it is
// being trained), and a reference to the state model.
// There are two constructors, one for building the big HMM for the whole lexicon, one for building the HMMs for training utterances.
class hmm {
public:
    vector<hmm_state> states;
    unordered_map<hmm_state*, int> idx_map;
    acoustic_model *acm, *temp_acm;
    state_model *sm;
    
    hmm (vector<phone::phone> ph, acoustic_model *ac);  // this builds the HMM for embedded training on a training sentence
    hmm (pronlex &pr, unigram_model &uni, acoustic_model *ac, state_model *smo);  // builds the big HMM representing the entire pronunciation lexicon
    
    int indexOf (hmm_state *);
    
    list<hmm_state*> viterbi (vector<featurevec*> fvs, float &prevp, float beam_width);
    void train_transition_probabilities (path p);
    void train (vector<utterance> &ut);
    void update_states ();
    
};

#endif /* hmm_hpp */
