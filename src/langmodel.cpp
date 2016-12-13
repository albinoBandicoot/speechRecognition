//
//  langmodel.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/28/16.
//
//

#include "langmodel.hpp"


ngram_model::ngram_model (int n, long count) {
    total_count = count;
    logtotal = log(count);
}

void unigram_model::put (string w, long count) {
    float logprob = log(count) - logtotal;
    probs[w] = logprob;
}

// return log-probability of word w
float unigram_model::get(string w) const{
    if (probs.count(w) == 1) {
        return (*probs.find(w)).second;
    } else {
        return -logtotal;   // can't return 0 probability, since log(0) = -inf. Instead assume had count of 1. 
    }
}

void bigram_model::put (string w1, string w2, long count) {
    float logprob = log(count) - logtotal;
    probs[w1 + "^" + w2] = logprob;
}

// this is the log probability of w2 given w1
float bigram_model::get (string w1, string w2) const {
    string w = w1 + "^" + w2;
    if (probs.count(w) == 1) {
        return (*probs.find(w)).second - uni->get(w1);
    } else {
        return uni->get(w2);    // fall back to unigram model
    }
}