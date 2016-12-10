//
//  cluster.cpp
//  speakerID
//
//  Created by Nathan Paige on 12/9/16.
//
//

#include "cluster.hpp"
vector<featurevec> kmeans (vector<featurevec*> fvs, featurevec &mu, featurevec &var, int k) {
    vector<featurevec> norm_fvs;
    for (int i=0; i < fvs.size(); i++) {
        norm_fvs.push_back (fvs[i]->normalize(mu, var));
    }
    
    // initialize the centers
    vector<featurevec> centers;
    for (int i=0; i < k; i++) {
        centers.push_back (random_fv(mu, var));
    }
    
    int closest[fvs.size()];
    for (int i=0; i < fvs.size(); i++) closest[i] = -1;
    int changecount = 0;
    do {
        changecount = 0;
        // determine closest center for each FV
        for (int i=0; i < fvs.size(); i++) {
            float best = 1e30f;
            int prev_c = closest[i];
            for (int c=0; c < k; c++) {
                float d = norm_fvs[i].dist (centers[c]);
                if (d < best) {
                    best = d;
                    closest[i] = c;
                }
            }
            if (prev_c != closest[i]) changecount++;
        }
        // update center locations
        for (int c=0; c < k; c++) {
            centers[c] *= 0;    // clear it
            int count = 0;
            for (int i=0; i < fvs.size(); i++) {
                if (closest[i] == c) {
                    count++;
                    centers[c] += norm_fvs[i];
                }
            }
            centers[c] *= 1.0f/count;
        }
    } while (changecount > 0);
    return centers;
}

