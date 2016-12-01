//
//  mfcc.hpp
//  speakerID
//
//  Created by Nathan Paige on 10/20/16.
//
//

#ifndef mfcc_hpp
#define mfcc_hpp

#include <stdio.h>
#include "clip.hpp"
#include <vector>

#define FV_LEN 39
#define NPARAMS 13

class featurevec {
public:
    float coeffs[NPARAMS];
    float deltas[NPARAMS];
    float delta2s[NPARAMS];
    
    featurevec ();
    featurevec (float *c);
    
    float operator[] (int i) const;
    
    void compute_deltas (vector<featurevec*> fvs, int i);
    void compute_delta2s (vector<featurevec*> fvs, int i);

};

class filter {
public:
    float center;
    float width;    // on either side
    float height;
    
    filter (float c, float w, int dft_bins=256, int sps=16000);
    
    float get (int bin) const;
    float operator() (float *spectrum);
};


class filterbank {
public:
    vector<filter> filters;
    
    filterbank ();
    
    int length() const;
    float *operator() (float *spectrum);
    
};

filterbank* linear_filterbank (float start, float step, int n, float width, int dft_bins=256, int sps=16000);
filterbank* mel_filterbank (float end, int n, int dft_bins=256, int sps=16000);

float *dct (float *filtered, int len, int nparams);

float *mfcc (Clip c, filterbank &fb);

vector<featurevec*> read_features (string fname);
void write_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb, string fname);
vector<featurevec*> get_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb);

#endif /* mfcc_hpp */
