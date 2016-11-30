//
//  utils.cpp
//  speakerID
//
//  Created by Nathan Paige on 10/19/16.
//
//

#include "utils.hpp"

/* Windowing functions */

float rectangular_window (int x, int len) {
    return 1.0f;
}

float hanning_window (int x, int len) {
    return 0.5f * cos(1 - (2*M_PI*x)/(len-1));
}

float hamming_window (int x, int len) {
    return 0.54f + 0.46 * cos(2*M_PI*x/(len-1));
}