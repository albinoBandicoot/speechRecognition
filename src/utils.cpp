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

vector<string> readlines (const char *fname) {
    vector<string> lines;
    char buf[1024];
    ifstream in(fname);
    in.getline(buf, 1024);
    do {
        lines.push_back (string(buf));
        in.getline(buf,1024);
    } while (in.good());
    in.close();
    return lines;
}

vector<string> readlines (string fname) {
    return readlines (fname.c_str());
}