//
//  gmm.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/23/16.
//
//

#include "gmm.hpp"
#include "hmm.hpp"

gaussian::gaussian () {
    logwt = 0;  // weight 1
    for (int i=0; i < FV_LEN; i++) {
        mean[i] = 0;
        var[i] = 1;
    }
}

void gaussian::set_wt (float wt) {
    logwt = log(wt);
}

float gaussian::operator() (featurevec &fv) {
    float res = 0;
    float l2pi = log(M_2_PI);
    for (int i=0; i < FV_LEN; i++) {
        float t = (fv[i] - mean[i]);
        res += l2pi + var[i] + (t*t)/var[i];
    }
    return -0.5f * res + logwt;
}

featurevec mean (vector<featurevec*> fvs) {
    featurevec mu;
    int N = fvs.size();
    for (int i=0; i < N; i++) {
        for (int j=0; j < FV_LEN; j++) {
            mu[j] += (*fvs[i])[j] / N;
        }
    }
    return mu;
}

featurevec variance (vector<featurevec*> fvs, featurevec &mu) {
    featurevec var;
    int N = fvs.size();
    for (int i=0; i < N; i++) {
        for (int j=0; j < FV_LEN; j++) {
            float f = (*fvs[i])[j] - mu[j];
            var[j] += f*f / N;
        }
    }
    return var;
}

/*
gmm::gmm (int n) {
    for (int i=0; i < n; i++) {
        gaussians.push_back (gaussian());
    }
}
*/
 
float gmm::operator() (featurevec &fv) {
    double res = 0;
    for (int i=0; i < gaussians.size(); i++) {
        res += exp(gaussians[i](fv));
    }
    return log(res);
}

acoustic_model::acoustic_model (phone::ties &t, int nmix) : ties(t) {
    /*
    for (int i=0; i < phone::NUM_PH; i++) {
        if (USE_SUBPHONES) {
            mixtures[phspec((phone::phone) i, phone::BEGIN)] = new gmm(nmix);
            mixtures[phspec((phone::phone) i, phone::MIDDLE)] = new gmm(nmix);
            mixtures[phspec((phone::phone) i, phone::END)] = new gmm(nmix);

        } else {
            mixtures[phspec((phone::phone) i, phone::MIDDLE)] = new gmm(nmix);
        }
    }
     */
}

float acoustic_model::operator() (featurevec &fv, phone::context ph) {
    return (*(*mixtures.find(ph)).second)(fv);
}