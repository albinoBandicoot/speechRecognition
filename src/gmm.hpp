//
//  gmm.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/23/16.
//
//

#ifndef gmm_hpp
#define gmm_hpp

#include <stdio.h>
#include <vector>
#include <utility>
#include "mfcc.hpp"
#include "pronounce.hpp"

class gaussian {
public:
    float weight;
    float logwt;
    float mean[FV_LEN];
    float var[FV_LEN];
    
    float zetasum;
    
    gaussian ();
    
    void set_wt (float wt);
    float operator() (featurevec &fv);
    float prob (featurevec &fv);
    float logprob (featurevec &fv);

    void clear();
    void divide_means();
    void divide_variances();
};

class gmm {
public:
    vector<gaussian> &gaussians;
    float zetasum;
    
    gmm (int n) : gaussians(*new vector<gaussian>()) {
        for (int i=0; i < n; i++) {
            gaussians.push_back (gaussian());
        }
    }
    
    float operator() (featurevec &fv);
    void clear ();
    void sum_zetas();
    void clear_zetas();
    void divide_means();
    void divide_variances();
};

typedef map<phone::context,gmm*>::iterator acm_iter;

class acoustic_model {
public:
    map<phone::context, gmm*> mixtures;
    phone::ties &ties;
    
    acoustic_model (phone::ties &t, int nmix = 1);
    
    float operator() (featurevec &fv, phone::context ph);
    void clear();
    void sum_zetas();
    void clear_zetas();
    void divide_means();
    void divide_variances();
    acm_iter begin();
    acm_iter end();
};

#endif /* gmm_hpp */
