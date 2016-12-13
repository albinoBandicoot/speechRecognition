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

/* This is the k-means clustering algorithm, applied to feature vectors. It could be used for 
 a Vector Quantization approach to acoustic modeling. This code is not currently used anywhere
 in the rest of the project.
 */

vector<featurevec> kmeans (vector<featurevec*> fvs, featurevec &mu, featurevec &var, int k);


#endif /* cluster_hpp */
