//
//  corpus.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#include "corpus.hpp"
#include "hmm.hpp"

utterance::utterance (istream &in, string path_base, acoustic_model &acm, pronouncer &pr) {
    string mfcc_fname;
    in >> mfcc_fname;
    cout << mfcc_fname << endl;
    getline(in, text);
    pronunciation = pr.pronounce (text);
    hmm = new class hmm(pronunciation, &acm);
    features = read_features (path_base + mfcc_fname);
    cout << "\ttext: " << text << endl;
    cout << "\tpronunciation: ";
    for (int i=0; i < pronunciation.size(); i++) {
        cout << phone::names[pronunciation[i]] << " ";
    }
    cout << endl;
}
