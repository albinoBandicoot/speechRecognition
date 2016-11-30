//
//  utils.hpp
//  speakerID
//
//  Created by Nathan Paige on 10/19/16.
//
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>
#include <math.h>

float rectangular_window (int,int);
float hanning_window (int,int);
float hamming_window (int,int);





/*
template<class T>
T min (T a, T b) {
    return a < b ? a : b;
}

template<class T>
T max (T a, T b) {
    return a > b ? a : b;
}
*/
template<class T>
T min (T *a, int len) {
    T best = a[0];
    for (int i=1; i < len; i++) {
        if (a[i] < best) best = a[i];
    }
    return best;
}

template<class T>
T max (T *a, int len) {
    T best = a[0];
    for (int i=1; i < len; i++) {
        if (a[i] > best) best = a[i];
    }
    return best;
}

template<class T>
int argmax (T *a, int len) {
    int best = 0;
    for (int i=1; i < len; i++) {
        if (a[i] > a[best]) {
            best = i;
        }
    }
    return best;
}


template<class T>
void log (T *buf, int len) {
    for (int i=0; i < len; i++) {
        buf[i] = log(buf[i]);
    }
}

template<class T>
void convolve (T *out, T *in, T *kernel, int len, int klen) {
    int koff = klen/2;
    memset (out, 0, len*sizeof(T));
    for (int i=0; i < len; i++) {
        for (int k=0; k < klen; k++) {
            int z = i+k-koff;
            if (z >= 0 && z < len) {
                out[i+k-koff] += in[i]*kernel[k];
            }
        }
    }
}


#endif /* utils_hpp */
