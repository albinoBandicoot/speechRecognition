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
    features = read_features (path_base + mfcc_fname, 2);
    cout << "\ttext: " << text << endl;
    cout << "\tpronunciation: ";
    for (int i=0; i < pronunciation.size(); i++) {
        cout << phone::names[pronunciation[i]] << " ";
    }
    cout << endl;
}


featurevec mean (vector<utterance> &ut) {
    featurevec mu;
    int N = 0;
    for (int s=0; s < ut.size(); s++) {
        for (int i=0; i < ut[s].features.size(); i++) {
            for (int j=0; j < FV_LEN; j++) {
                mu[j] += (*ut[s].features[i])[j];
            }
        }
        N += ut[s].features.size();
    }
    for (int j=0; j < FV_LEN; j++) {
        mu[j] /= N;
    }
    return mu;
}

featurevec variance (vector<utterance> &ut, featurevec &mu) {
    featurevec var;
    int N = 0;
    for (int s=0; s < ut.size(); s++) {
        for (int i=0; i < ut[s].features.size(); i++) {
            for (int j=0; j < FV_LEN; j++) {
                float f =(*ut[s].features[i])[j] - mu[j];
                var[j] += f*f;
            }
        }
        N += ut[s].features.size();
    }
    for (int j=0; j < FV_LEN; j++) {
        var[j] /= N;
    }
    return var;
}