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
    zetasum = 0;
    logwt = 0;  // weight 1
    weight = 1;
    for (int i=0; i < FV_LEN; i++) {
        mean[i] = 0;
        var[i] = 1;
    }
}

void gaussian::set_wt (float wt) {
    weight = wt;
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

float gaussian::logprob (featurevec &fv) {
    return (*this)(fv);
}

float gaussian::prob (featurevec &fv) {
    return exp((*this)(fv));
}

void gaussian::clear () {
    set_wt (1);
    for (int i=0; i < FV_LEN; i++) {
        mean[i] = 0;
        var[i] = 0;
    }
}

void gaussian::divide_means () {    // this divides the means by zetasum
    for (int i=0; i < FV_LEN; i++) {
        mean[i] /= zetasum;
    }
}

void gaussian::divide_variances () {
    for (int i=0; i < FV_LEN; i++) {
        var[i] /= zetasum;
    }
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

// ----------------------------------------------------------------------
 
float gmm::operator() (featurevec &fv) {
    double res = 0;
    for (int i=0; i < gaussians.size(); i++) {
        res += exp(gaussians[i](fv));
    }
    return log(res);
}

void gmm::clear () {
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].clear();
    }
}

void gmm::sum_zetas () {
    zetasum = 0;
    for (int i=0; i < gaussians.size(); i++) {
        zetasum += gaussians[i].zetasum;
    }
}

void gmm::clear_zetas() {
    zetasum = 0;
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].zetasum = 0;
    }
}

void gmm::divide_means () {
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].divide_means();
    }
}
void gmm::divide_variances () {
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].divide_variances();
    }
}

// ----------------------------------------------------------------------

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
    return (*(*mixtures.find(ties(ph))).second)(fv);
}

void acoustic_model::clear () {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->clear();
    }
}

void acoustic_model::sum_zetas () {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->sum_zetas();
    }
}

void acoustic_model::clear_zetas () {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->clear_zetas();
    }
}

void acoustic_model::divide_means () {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->divide_means();
    }
}

void acoustic_model::divide_variances () {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->divide_variances();
    }
}

acm_iter acoustic_model::begin () {
    return mixtures.begin();
}
acm_iter acoustic_model::end () {
    return mixtures.end();
}