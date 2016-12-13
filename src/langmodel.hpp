//
//  langmodel.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/28/16.
//
//

#ifndef langmodel_hpp
#define langmodel_hpp

#include <stdio.h>
#include <map>
#include <istream>
#include <math.h>
using namespace std;

/* N-gram models. Currently only unigram and bigram are implemented; only unigram is currently used anywhere in the recognizer. */

class ngram_model {
public:
    map<string, float> probs;
    int n;
    long total_count;
    float logtotal;
    
    ngram_model (int n, long total_count);
};

class unigram_model : public ngram_model {
public:
    
    // construct from file: 1 unigram per line; word followed by count, separated by whitespace
    unigram_model (istream &in, long total_count) : ngram_model(1,total_count) {
        string w;
        long count;
        while (in.good()) {
            in >> w;
            in >> count;
            put (w, count);
        }
    }
    
    void put (string s, long count);
    float get (string s) const;
};

class bigram_model : public ngram_model {
public:
    
    unigram_model *uni; // model to fall back on for non-existant bigrams
    
    // build from file
    bigram_model (istream &in, unigram_model *u, long total_count) : ngram_model(2,total_count), uni(u) {
        string w1, w2;
        long count;
        while (in.good()) {
            in >> w1 >> w2 >> count;
            put (w1, w2, count);
        }
    }
    
    void put (string s1, string s2, long count);
    float get (string s1, string s2) const;
};
#endif /* langmodel_hpp */
