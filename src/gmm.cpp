//
//  gmm.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/23/16.
//
//

#include "gmm.hpp"
#include "hmm.hpp"
#include "corpus.hpp"

gaussian::gaussian () {
    zetasum = 0;
    logwt = 0;  // weight 1
    weight = 1;
    for (int i=0; i < FV_LEN; i++) {
        mean[i] = 0;
        var[i] = 1;
    }
}

void gaussian::initialize (featurevec &mu, featurevec &sigma2) {
    for (int i=0; i < FV_LEN; i++) {
        float sd = sqrt(sigma2[i]);
        float K = 1.5f;
        mean[i] = mu[i] + ofRandom(-K*sd, K*sd);
        var[i] = sigma2[i] * ofRandom(0.8f,1.2f);
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
        res += l2pi + log(var[i]) + (t*t)/var[i];
    }
    return -0.5f * res + logwt;
}

float gaussian::logprob (featurevec &fv) {
    return (*this)(fv);
}

prob_t gaussian::prob (featurevec &fv) {
    return exp((prob_t)(*this)(fv));
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

void gaussian::blend (gaussian &g, float f) {
    for (int i=0; i < FV_LEN; i++) {
        mean[i] = mean[i]*f + g.mean[i]*(1-f);
        var[i]  = var[i]*f  + g.var[i]*(1-f);
    }
    set_wt (weight*f + g.weight*(1-f));
}

// ----------------------------------------------------------------------

gmm::gmm (gmm *g) : gaussians(*new vector<gaussian>()) {
    for (int i=0; i < g->gaussians.size(); i++) {
        gaussian ng = g->gaussians[i];    // this will make an independent copy.
        gaussians.push_back(ng);
    }
}

logprob_t gmm::operator() (featurevec &fv) {
    return gaussians[0](fv);
    /*
    double res = 0;
    for (int i=0; i < gaussians.size(); i++) {
        res += exp(gaussians[i](fv));
    }
    return log(res);
     */
}

void gmm::initialize (featurevec &mu, featurevec &var) {
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].initialize(mu, var);
    }
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

void gmm::blend (gmm *g, float f) {
    for (int i=0; i < gaussians.size(); i++) {
        gaussians[i].blend(g->gaussians[i], f);
    }
}

// ----------------------------------------------------------------------

acoustic_model::acoustic_model (phone::ties &t, int nmix) : ties(t) {
    phone::context start(phone::SIL);
    phone::context c = start;
    do {
        if (mixtures.count(t(c)) == 0) {
            mixtures[t(c)] = new gmm(nmix);
        }
        ++c;
    } while (start < c);
}

acoustic_model::acoustic_model (acoustic_model *acm) : ties(acm->ties) {
    for (acm_iter it = acm->begin(); it != acm->end(); it++) {
        mixtures[it->first] = new gmm(it->second);
    }
}

void acoustic_model::initialize (featurevec &mu, featurevec &var) {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->initialize(mu, var);
    }
}

logprob_t acoustic_model::operator() (featurevec &fv, phone::context ph) {
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

void acoustic_model::blend (acoustic_model *a, float f) {
    for (acm_iter it = begin(); it != end(); it++) {
        it->second->blend(a->mixtures[it->first], f);
    }
}

acm_iter acoustic_model::begin () {
    return mixtures.begin();
}
acm_iter acoustic_model::end () {
    return mixtures.end();
}