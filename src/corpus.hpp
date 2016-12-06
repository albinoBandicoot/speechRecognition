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

class utterance {
public:
    string text;
    vector<phone::phone> pronunciation;
    hmm *hmm;
    vector<featurevec*> features;

    utterance (istream &in, string mfcc_fname, acoustic_model &acm, pronouncer &pr);
};

#endif /* corpus_hpp */
