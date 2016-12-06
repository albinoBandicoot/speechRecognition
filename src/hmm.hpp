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
#include "corpus.hpp"

#define USE_SUBPHONES true

#define LOG_HALF -0.6931471805f
#define LOG_THIRD -1.09861228866f

#define SIL_SKIP_P 0.3

#define LMSF 5      // language model scaling factor
class hmm_state;

typedef pair<hmm_state*, float> transition;

class hmm_state {
public:
    phspec ph;
    phone::context ctx;
    vector<transition> tr;   // transitions and probabilities
    string *word;   // what word this state is part of (not needed for the training HMMs)
    
    hmm_state (phspec p);
    
    float get_prob (int i) const;
    void set_prob (int i, float p);
};



class state_model {
public:
    phone::ties &ties;
    map<phone::context, float> advance_probs;   // probability of transition to next state. self loop probability is 1 - advance_prob. Not sure what to do about word-end nodes that have 3 transitions (self, advance to silence, skip silence)
    
    state_model (phone::ties &t) : ties(t) {}
    
    float operator() (phone::context ctx);
};

class hmm {
public:
    vector<hmm_state> states;
    acoustic_model &acm;
    state_model &sm;
    
    hmm (vector<phone::phone> ph, acoustic_model &ac, state_model &smo);  // this builds the HMM for embedded training on a training sentence
    hmm (pronlex pr, unigram_model uni, acoustic_model &ac, state_model &smo);  // builds the big HMM representing the entire pronunciation lexicon
    
    list<hmm_state*> viterbi (vector<featurevec*> fvs, float beam_width);
    void train (vector<utterance> ut);
    void update_states ();
    
};



#endif /* hmm_hpp */
