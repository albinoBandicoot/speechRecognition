#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "clip.hpp"
#include "utils.hpp"
#include "mfcc.hpp"
#include "pronounce.hpp"

#define VOXFORGE_DIR "/Users/Nathan/Documents/2016H/Security/project/audio/voxforge/"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void audioIn(ofSoundBuffer &buf);
    
    ClipVectorBuffer *recording;
    ClipArrayBuffer *noise_spectrum;
    
    float *spectra;
    float *cepstra;
    float *amplitudes;
    filterbank *fbank;
    int nframes;
    int nccoeffs;
    int nscoeffs;
    float xscale, xshift;
    
    featurevec mu, sigma;
    
    enum state_t {
        START, NOISE_PROF, RECORD, COMPUTE, WAIT
    };
    state_t state = START;
    int noise_bufs = 40;
    int noise_bufs_done = 0;
    int analysis_bufsize = 768;
    int buf_shift_frac = 2;
    
    pronlex prlex;
    
    ofxPanel gui;
    ofxFloatSlider scale;
    ofxFloatSlider offset;
    
    void startRecording ();
    void stopRecording ();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
};
