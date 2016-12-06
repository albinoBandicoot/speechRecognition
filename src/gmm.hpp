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
    float logwt;
    float mean[FV_LEN];
    float var[FV_LEN];
    
    gaussian ();
    
    void set_wt (float wt);
    float operator() (featurevec &fv);
};

class gmm {
public:
    vector<gaussian> &gaussians;
    
    gmm (int n) : gaussians(*new vector<gaussian>()) {
        for (int i=0; i < n; i++) {
            gaussians.push_back (gaussian());
        }
    }
    
    float operator() (featurevec &fv);
};

class acoustic_model {
public:
    map<phone::context, gmm*> mixtures;
    phone::ties &ties;
    
    acoustic_model (phone::ties &t, int nmix = 1);
    
    float operator() (featurevec &fv, phone::context ph);
};

#endif /* gmm_hpp */
