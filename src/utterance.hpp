//
//  corpus.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#ifndef corpus_hpp
#define corpus_hpp

#include <stdio.h>
#include <string>
#include <istream>
#include "pronounce.hpp"
#include "clip.hpp"
#include "mfcc.hpp"
#include "hmm.hpp"
using namespace std;

class hmm;

/* The utterance class represents one training or test utterance. It includes the MFCC vectors, a pointer to the 
 HMM built from it (if it's a training utterance), and the text and pronunciation (filled in by reading from a 
 file for training, filled in by speech recognition for test sentences) */
class utterance {
public:
    string text;
    vector<phone::phone> pronunciation;
    hmm *hmm;
    vector<featurevec*> features;

    utterance (istream &in, string path_base, acoustic_model &acm, pronouncer &pr);
};

featurevec mean (vector<utterance> &);
featurevec variance (vector<utterance> &, featurevec&);

#endif /* corpus_hpp */
