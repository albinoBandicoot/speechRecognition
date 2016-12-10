//
//  cluster.hpp
//  speakerID
//
//  Created by Nathan Paige on 12/9/16.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <stdio.h>
#include <vector>
#include "mfcc.hpp"
using namespace std;

vector<featurevec> kmeans (vector<featurevec*> fvs, featurevec &mu, featurevec &var, int k);


#endif /* cluster_hpp */
