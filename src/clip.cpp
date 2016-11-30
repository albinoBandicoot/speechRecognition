//
//  clip.cpp
//  speakerID
//
//  Created by Nathan Paige on 10/19/16.
//
//

#include "clip.hpp"

void ClipBuffer::append ( const ofSoundBuffer &buf) {
}

ClipBuffer::~ClipBuffer () {
}

// Clip Array Buffer

ClipArrayBuffer::ClipArrayBuffer (int n) {
    this->n = n;
    this->buf = new float[n];
}

ClipArrayBuffer::ClipArrayBuffer (ofSoundBuffer &b) {
    this->n = b.getNumFrames();
    this->buf = new float[n];
    for (int i=0; i < n; i++) {
        buf[i] = b.getSample (i, 0);
    }
}

ClipArrayBuffer::~ClipArrayBuffer () {
    delete[] buf;
}

float& ClipArrayBuffer::operator[] (int idx)  {
    return buf[idx];
}

int ClipArrayBuffer::length() const {
    return n;
}

// Clip Vector Buffer

ClipVectorBuffer::ClipVectorBuffer () {
}

void ClipVectorBuffer::ensure (int additional) {
    if (samples.capacity() - samples.size() < additional) {
        samples.reserve(samples.capacity() + additional);
    }
}

void ClipVectorBuffer::append (const ofSoundBuffer &buf) {
    ensure (buf.getNumFrames());
    for (int i=0; i < buf.getNumFrames(); i++) {
        samples.push_back (buf.getSample(i,0));
    }
}

void ClipVectorBuffer::append (float *buf, int n) {
    ensure(n);
    for (int i=0; i < n; i++) {
        samples.push_back (buf[i]);
    }
}

float& ClipVectorBuffer::operator[] (int idx)  {
    return samples[idx];
}

int ClipVectorBuffer::length () const {
    return samples.size();
}


// Clip

Clip::Clip () {
}

Clip::Clip (ClipBuffer &buf) {
    this->buf = &buf;
    start = 0;
    end = buf.length();
    this->window = rectangular_window;
}

Clip::Clip (ClipBuffer &buf, int start, int end) {
    this->buf = &buf;
    this->start = start;
    this->end = end;
    this->window = rectangular_window;
}

Clip::Clip (const Clip &clip, int start, int end) {
    this->buf = clip.buf;
    this->start = clip.start + start;
    this->end = clip.start + end;
    this->window = rectangular_window;
}

bool Clip::slide (int n) {
    start += n;
    end += n;
    return start >= 0 && end < buf->length();
}

int Clip::length () const {
    return end - start;
}

float& Clip::operator[] (int idx)  {
    return (*buf)[start+idx];
}

float Clip::operator[] (int idx) const {
    return (*buf)[start+idx] * window(idx, length());
}

void Clip::operator+=(Clip &c) {
    for (int i=0; i < min(length(), c.length()); i++) {
        (*this)[i] += c[i];
    }
}

void Clip::operator-=(Clip &c) {
    for (int i=0; i < min(length(), c.length()); i++) {
        (*this)[i] -= c[i];
    }
}
void Clip::operator*=(Clip &c) {
    for (int i=0; i < min(length(), c.length()); i++) {
        (*this)[i] *= c[i];
    }
}
void Clip::operator*=(float f) {
    for (int i=0; i < length(); i++) {
        (*this)[i] *= f;
    }
}

double Clip::rmsAmplitude () const {
    double amp = 0;
    for (int i=0; i < length(); i++) {
        float s = (*this)[i];
        amp += s*s;
    }
    return sqrt(amp);
}

void Clip::dft (float *out, bool stack) const {
    partial_dft (out, length(), stack);
}

void Clip::partial_dft (float *out, int len, bool stack) const {
    int N = length();
    float rootn_inv = 1.0f/sqrt(N);
    bool mirror = len > N/2;
    for (int k=0; k < len; k++) {
        float re = 0;
        float im = 0;
        for (int n=0; n < N/2; n++) {
            re += (*this)[n] * cos(-2*M_PI*k*n/N);
            im += (*this)[n] * sin(-2*M_PI*k*n/N);
        }
        float res = sqrt(re*re + im*im) * rootn_inv;
        if (stack) {
            out[k] += res;
            if (mirror and N-k-1 < len) out[N-k-1] += res;
        } else {
            out[k] = res;
            if (mirror and N-k-1 < len) out[N-k-1] = res;
        }
    }
}

void Clip::cepstrum (float *out) const {
    int N = length();
    ClipArrayBuffer spec(N);
    dft (spec.buf);
    for (int i=0; i < N; i++) {
        spec[i] = log(spec[i]*spec[i]);
    }
    Clip(spec).dft(out);
    for (int i=0; i < N; i++) {
        out[i] *= out[i];
    }
}

void Clip::preemphasis (float alpha) {
    float save = 0;
    for (int i=length()-1; i > 0; i--) {
        (*this)[i] -= alpha * (*this)[i-1];
    }
    (*this)[0] = (*this)[1];
}
