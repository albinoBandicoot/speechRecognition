//
//  mfcc.cpp
//  speakerID
//
//  Created by Nathan Paige on 10/20/16.
//
//

#include "mfcc.hpp"
#include <iostream>
using namespace std;

featurevec::featurevec () {
    for (int i=0; i < NPARAMS; i++) {
        coeffs[i] = deltas[i] = delta2s[i] = 0;
    }
}

featurevec::featurevec (float *c) {
    for (int i=0; i < NPARAMS; i++) {
        coeffs[i] = c[i];
    }
}

float& featurevec::operator[] (int i)  {
    if (i < NPARAMS) return coeffs[i];
    if (i < 2*NPARAMS) return deltas[i-NPARAMS];
    if (i < 3*NPARAMS) return delta2s[i-2*NPARAMS];
    return coeffs[i];
}

void featurevec::compute_deltas (vector<featurevec*> fvs, int i) {
    int j = i-1;
    int k = i+1;
    if (j < 0) j = 0;
    if (k == fvs.size()) k--;
    for (int q = 0; q < NPARAMS; q++) {
        deltas[q] = (fvs[k]->coeffs[q] - fvs[j]->coeffs[q]) / (k-j);
    }
}

void featurevec::compute_delta2s (vector<featurevec*> fvs, int i) {
    int j = i-1;
    int k = i+1;
    if (j < 0) j = 0;
    if (k == fvs.size()) k--;
    for (int q = 0; q < NPARAMS; q++) {
        delta2s[q] = (fvs[k]->deltas[q] - fvs[j]->deltas[q]) / (k-j);
    }
}

void compute_deltas (vector<featurevec*> fvs) {
    for (int i=0; i < fvs.size(); i++) {
        fvs[i]->compute_deltas(fvs, i);
    }
    for (int i=0; i < fvs.size(); i++) {
        fvs[i]->compute_delta2s(fvs, i);
    }
}

// -----------------------------------

filter::filter (float c, float w, int dft_bins, int sps) {
    this->center = ofMap(c, 0, sps, 0, dft_bins);
    this->width = ofMap(w, 0, sps, 0, dft_bins);
    height = 1;
    float h = 0;
    for (int i=(int) (center-width); i <= (int) (center+width); i++) {
        h += get(i);
    }
    height = 1/h;
    cout << "Made filter with center = " << center << "; width = " << width << "; height = " << height << endl;
    
}

float filter::get (int bin) const {
    float diff = center - bin;
    if (diff < 0) diff = -diff;
    if (diff > width) return 0;
    diff /= width;
    diff = 1-diff;
    diff *= height;
    return diff;
    //return ofMap (abs(center-bin), 0, width, height, 0, true);
}

float filter::operator() (float *spectrum) {
    float res = 0;
    for (int i = (int) (center - width); i <= (int)(center+width); i++) {
        res += get(i) * spectrum[i];
    }
    return res;
}

int filterbank::length() const {
    return filters.size();
}

filterbank::filterbank () {
}

float *filterbank::operator() (float *spectrum) {
    float *res = new float[filters.size()];
    for (int i=0; i < filters.size(); i++) {
        res[i] = filters[i](spectrum);
    }
    return res;
}

filterbank* linear_filterbank (float start, float step, int n, float width, int dft_bins, int sps) {
    filterbank *fb = new filterbank();
    for (int i=0; i < n; i++) {
        fb->filters.push_back (filter(start+i*n, width, dft_bins, sps));
    }
    return fb;
}

float melToHz (float mel) {
    return 700 * (exp(mel/1127.0f)-1);
}

float hzToMel (float hz) {
    return 1127 * log(1+hz/700.0f);
}

filterbank* mel_filterbank (float end, int n, int dft_bins, int sps) {
    filterbank *fb = new filterbank();
    float mel_end = hzToMel(end);
    float mel_step = mel_end / (n+1);
    float prev = 0;
    for (int i=0; i < n; i++) {
        float hz = melToHz ((i+1)*mel_step);
        fb->filters.push_back (filter(hz, hz - prev, dft_bins, sps));
        prev = hz;
    }
    return fb;
}

float *dct (float *filtered, int len, int nparams) {
    float *res = new float[nparams];
    for (int j=0; j < nparams; j++) {
        res[j] = 0;
        for (int i=0; i < len; i++){
            res[j] += (filtered[i]+8) * cos(j*(i-0.5f)*M_PI/len);
        }
    }
    return res;
}

// the clip should already be pre-emphasized, if desired
// assumes the last filter has the highest center+width value.
float *mfcc (Clip c, filterbank &fb, float *noise) {
    filter high = fb.filters.back();
    int bin_max = (int) (high.center + high.width) + 1;
    float dft[bin_max];
    c.partial_dft(dft, bin_max);
    for (int i=0; i < bin_max; i++) {
        dft[i] = fmax(0, dft[i] - noise[i]);
    }
    float *filtered = fb(dft);
    log(filtered, fb.length());
    float *coeffs = dct(filtered, fb.length(), NPARAMS);
    delete[] filtered;
    return coeffs;
}

void write_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb, string fname) {
    FILE *out = fopen (fname.c_str(), "w");
    if (out == NULL) throw 1;
    c.preemphasis (0.96f);  // if desired
    Clip window = Clip(c, 0, window_len);
    window.window = hamming_window;
    fseek (out, 4, SEEK_CUR);
    int n = 0;
    long time = clock();
    do {
        float *cc = mfcc (window, fb);
        for (int i=0; i < NPARAMS; i++) {
            fwrite (cc+i, 4, 1, out);
        }
        delete[] cc;
        n++;
        if (n % 100 == 0) {
            cout << "Done " << n << " frames in " << (clock() - time)/1000 << "ms" << endl;
        }
    } while (window.slide(frame_shift));
    rewind(out);
    fwrite (&n, 4, 1, out);
    fclose (out);
}

vector<featurevec*> read_features (string fname, int WL=2) {
    cout << "\treading features... ";
    FILE *in = fopen (fname.c_str(), "r");
    if (in == NULL) throw 1;
    int n;
    fread (&n, 4, 1, in);
    cout << "found " << n << " FVs" << endl;
    vector<featurevec*> fvs;
    vector<featurevec> raw;
    for (int i=0; i < n; i++) {
        featurevec fv;
        for (int q=0; q < NPARAMS; q++) {
            fread (&(fv.coeffs[q]), 4, 1, in);
        }
        raw.push_back(fv);
    }
    fclose (in);
    
    for (int i=0; i < n; i++) {
        fvs.push_back (new featurevec());
        int NF = 0;
        for (int w = -WL; w <= WL; w++) {
            int wt = WL - abs(w) + 1;
            if (i + w >= 0 && i + w < n ) {
                NF += wt;
                for (int j=0; j < 13; j++) {
                    fvs[i]->coeffs[j] += wt * raw[i+w].coeffs[j];
                }
            }
        }
        for (int j=0; j < 13; j++) {
            fvs[i]->coeffs[j] /= NF;
        }
    }
    compute_deltas (fvs);
    return fvs;
}

vector<featurevec*> get_features (Clip c, unsigned window_len, unsigned frame_shift, filterbank &fb, int nparams) {
    c.preemphasis (0.97f);  // if desired
    vector<featurevec*> fvs;
    Clip window = Clip(c, 0, window_len);
    window.window = hamming_window;
    do {
        float *cc = mfcc (window, fb);
        fvs.push_back (new featurevec(cc));
    } while (c.slide(frame_shift));
    // fill in the rest of the feature vectors (deltas and delta-deltas)
    compute_deltas (fvs);
}