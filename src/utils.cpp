//
//  utils.cpp
//  speakerID
//
//  Created by Nathan Paige on 10/19/16.
//
//

#include "utils.hpp"
#include "ofMain.h"
/* Windowing functions */

float rectangular_window (int x, int len) {
    return 1.0f;
}

float hanning_window (int x, int len) {
    return 0.5f * cos(1 - (2*M_PI*x)/(len-1));
}

float hamming_window (int x, int len) {
    return 0.54f + 0.46 * cos(2*M_PI*x/(len-1));
}


// --------
#define BINOMIAL_SUMMANDS 20
float rand_gaussian (float mu, float variance) {
    float res = 0;
    for (int i=0; i < BINOMIAL_SUMMANDS; i++) {
        res += ofRandom(-1,1);
    }
    // res has mean 0 and variance (1/3) * BINOMIAL_SUMMANDS
    res *= sqrt (variance / (BINOMIAL_SUMMANDS/3.0));
    // now res has variance 'variance'
    res += mu;
    return res;
}

vector<string> readlines (const char *fname) {
    vector<string> lines;
    char buf[1024];
    ifstream in(fname);
    in.getline(buf, 1024);
    do {
        lines.push_back (string(buf));
        in.getline(buf,1024);
    } while (in.good());
    in.close();
    return lines;
}

vector<string> readlines (string fname) {
    return readlines (fname.c_str());
}