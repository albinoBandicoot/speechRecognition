//
//  clip.hpp
//  speakerID
//
//  Created by Nathan Paige on 10/19/16.
//
//

#ifndef clip_hpp
#define clip_hpp

#include <stdio.h>
#include <vector>
#include "ofMain.h"
#include "utils.hpp"

/* Methods for working with audio data, once it's been read in. 
 ClipBuffer stores the actual data; Clip is a view into a ClipBuffer. 
*/

// virtual base class for two types of clip buffers, one based on arrays and one on vectors.
class ClipBuffer {
public:
    virtual float& operator[](int) =0;
    virtual int length() const=0;
    virtual ~ClipBuffer();
    
    void append (const ofSoundBuffer &buf);
};

// array-based clip buffer
class ClipArrayBuffer : public ClipBuffer {
private:
    int n;
public:
    
    float *buf;
    
    ClipArrayBuffer (int);
    ClipArrayBuffer (ofSoundBuffer &buf);
    ~ClipArrayBuffer ();
    
    float& operator[](int) ;
    int length() const;
    
};

// vector-based clip buffer
class ClipVectorBuffer : public ClipBuffer {
private:
    vector<float> samples;
    
public:
    
    ClipVectorBuffer ();
    
    void append (const ofSoundBuffer &buf);
    void append (float *buf, int n);
    void ensure (int n);
    
    float& operator[](int) ;
    int length () const;
    
};

// view into a buffer of audio data
class Clip {
public:
    ClipBuffer *buf;
    int start, end;
    float (*window)(int, int);  // pointer to windowing function. will be applied in the [] operator
    
    Clip ();
    Clip (ClipBuffer &b);
    Clip (ClipBuffer &b, int start, int end);
    Clip (const Clip &c, int start, int end);
    
    bool slide (int);
    float& operator[] (int) ;
    float operator[] (int) const;
    int length() const;
    
    void operator+=(Clip&);
    void operator-=(Clip&);
    void operator*=(Clip&);
    void operator*=(float);

    void normalizeVolume (float targetRMS);
    
    double rmsAmplitude () const;
    void dft (float *out, bool stack=false) const;
    void partial_dft (float*out, int len, bool stack=false) const;

    void preemphasis (float alpha);
    
    
};

#endif /* clip_hpp */
