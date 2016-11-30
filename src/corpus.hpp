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
using namespace std;

class utterance {
public:
    string text;
    vector<phone::phone> pronunciation;
    vector<featurevec*> features;

    utterance (istream &in, char *mfcc_fname, pronouncer &pr);
};

#endif /* corpus_hpp */
