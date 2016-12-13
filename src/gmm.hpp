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

/* Everything about the acoustic model. Defining individual gaussian densities, mixtures, and packaging 
 everything together (including parameter tying) into the acoustic_model type */

typedef long double prob_t; // hopefully 80-bit floats are enough precision. Probabilities can get pretty small, and gaussian densities with low variance can get pretty large
typedef float logprob_t;

// a single, multivariate gaussian distribution
class gaussian {
public:
    float weight;
    float logwt;
    prob_t mean[FV_LEN];
    prob_t var[FV_LEN];
    
    prob_t zetasum;
    
    gaussian ();
    void initialize (featurevec &, featurevec &);
    void set_wt (float wt);
    logprob_t operator() (featurevec &fv);
    prob_t prob (featurevec &fv);
    logprob_t logprob (featurevec &fv);

    void clear();
    void divide_means();
    void divide_variances();
    void blend (gaussian &g, float f);
};

// one gaussian mixture model. essentially a list of gaussians
class gmm {
public:
    vector<gaussian> &gaussians;
    prob_t zetasum;
    
    gmm (int n) : gaussians(*new vector<gaussian>()) {
        for (int i=0; i < n; i++) {
            gaussians.push_back (gaussian());
        }
    }
    
    gmm (gmm *g) ;
    
    logprob_t operator() (featurevec &fv);
    void initialize (featurevec &, featurevec &);

    void clear ();
    void sum_zetas();
    void clear_zetas();
    void divide_means();
    void divide_variances();
    void blend (gmm *g, float f);
};

typedef map<phone::context,gmm*>::iterator acm_iter;

// the acoustic_model maps phone contexts to their appropriate mixtures according to a parameter tying model 'ties'
class acoustic_model {
public:
    map<phone::context, gmm*> mixtures;
    phone::ties &ties;
    
    acoustic_model (phone::ties &t, int nmix = 1);
    acoustic_model (acoustic_model *acm);
    
    logprob_t operator() (featurevec &fv, phone::context ph);
    void initialize (featurevec &, featurevec &);

    void clear();
    void sum_zetas();
    void clear_zetas();
    void divide_means();
    void divide_variances();
    void blend (acoustic_model *a, float f);
    acm_iter begin();
    acm_iter end();
};

#endif /* gmm_hpp */
