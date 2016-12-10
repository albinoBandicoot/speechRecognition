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

#define FV_LEN 13
#define NPARAMS 13

#define NOISE_SEMGENTS 15

class featurevec {
public:
    float coeffs[NPARAMS];
    float deltas[NPARAMS];
    float delta2s[NPARAMS];
    
    featurevec ();
    featurevec (float *c);
    
    float& operator[] (int i);
    void operator+= (featurevec &);
    void operator*= (float);
    
    float dist (featurevec &f);
    
    featurevec normalize (featurevec &mu, featurevec &sigma) ;
    
    void compute_deltas (vector<featurevec*> fvs, int i);
    void compute_delta2s (vector<featurevec*> fvs, int i);

};

featurevec random_fv (featurevec &mu, featurevec &var);

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

float *mfcc (Clip c, filterbank &fb, float *noise);

vector<featurevec*> read_features (string fname, int WL);
void write_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb, string fname);
//vector<featurevec*> get_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb);

#endif /* mfcc_hpp */
