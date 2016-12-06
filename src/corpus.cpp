//
//  corpus.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#include "corpus.hpp"
#include "hmm.hpp"

utterance::utterance (istream &in, string mfcc_fname, acoustic_model &acm, pronouncer &pr) {
    char buf[500];
    in.getline (buf, 500);
    text = string(buf);
    pronunciation = pr.pronounce (text);
    hmm = new class hmm(pronunciation, acm);
    features = read_features (mfcc_fname);
}
