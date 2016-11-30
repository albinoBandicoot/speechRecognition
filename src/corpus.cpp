//
//  corpus.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#include "corpus.hpp"

utterance::utterance (istream &in, const char *mfcc_fname, pronouncer &pr) {
    char buf[500];
    in.getline (buf, 500);
    text = string(buf);
    pronunciation = pr.pronounce (text);
    features = read_features (mfcc_fname);
}
