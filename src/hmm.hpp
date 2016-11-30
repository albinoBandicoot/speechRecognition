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

class hmm_state;

typedef pair<hmm_state*, float> transition;

class hmm_state {
public:
    phspec ph;
    vector<transition> tr;   // transitions and probabilities
    string *word;   // what word this state is part of (not needed for the training HMMs)
    
    hmm_state (phspec p);
    
    float get_prob (int i) const;
    void set_prob (int i, float p);
};

class hmm {
public:
    vector<hmm_state> states;
    acoustic_model &acm;
    
    hmm (vector<phone::phone> ph, acoustic_model &ac);  // this builds the HMM for embedded training on a training sentence
    hmm (pronlex pr, unigram_model uni, acoustic_model &ac);   // builds the big HMM representing the entire pronunciation lexicon
    
    list<hmm_state*> viterbi (vector<featurevec*> fvs, float beam_width);
    void train (utterance &u);
    
};



#endif /* hmm_hpp */
