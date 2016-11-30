#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    char cwd[256];
    getcwd(cwd, 256);
    cout << "Current dir: " << cwd << endl;
    // read the pronunciation lexicon
    ifstream in("../../../../res/cmudict_small.txt");
    prlex = *new pronlex(in);
    
    recording = new ClipVectorBuffer();
    recording->ensure(100501);
    noise_spectrum = new ClipArrayBuffer(analysis_bufsize);
 //   fbank = linear_filterbank(100, 100, 35, 100, analysis_bufsize, 44100);
    fbank = mel_filterbank(7200, 48, analysis_bufsize, 16000);
    nframes = 0;
    nparams = 13;//fbank->length();
    
    ofSoundStreamSetup(0, 2, this, 44100, analysis_bufsize, 4);
    ofSoundStreamStop();
    gui.setup();
    gui.add(scale.setup("Scale",5,-5,20,300,25));
    gui.add(offset.setup("Offset", 0, -15, 15, 300, 25));
}

void add (float *out, float *in, float mul, int n) {
    for (int i=0; i < n; i++) {
        out[i] += in[i] * mul;
    }
}

void ofApp::audioIn(ofSoundBuffer &buffer){
    cout << "audio came in " << endl;
    int n = buffer.getNumFrames();
    if (state == RECORD) {
        recording->append (buffer);
        cout << "Appended input; new length = " << recording->length() << endl;
        if (recording->length() > 100000) {
            cout << "Recording length limit exceeded, stopping..." << endl;
            stopRecording();
        }
    } else if (state == NOISE_PROF) {
        state = RECORD;
        cout << "was getting noise profile, now not" << endl;
        return;
        Clip noise(*new ClipArrayBuffer(buffer));
        noise.dft(noise_spectrum->buf, true);
        noise_bufs_done ++;
        if (noise_bufs_done == noise_bufs) {
            state = RECORD;
            noise *= 1.0f/noise_bufs;
        }
    }
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0,0,0);
    ofSetLineWidth(1.0f);
    if (state == NOISE_PROF) {
        ofDrawBitmapString ("Getting Noise Profile...", 100, 650);
        return;
    } else if (state == RECORD) {
        ofDrawBitmapString ("Recording...", 100, 650);
        return;
    } else if (state == START || state == COMPUTE) {
        return;
    }
    float xperbuf = 900.0f / nframes;
    float yperbin = 600.0f / nparams;
    float xs = 50;
    float ys = 600;
    
    for (int i=0; i < nframes; i++) {
        for (int j=0; j < nparams; j++) {
            float c = spectra[i*nparams+j];
            //c = log(c+1);//pow(c,0.4);
            c += offset;
            c = ofMap(c, 0, 1, 0, exp(scale));
            c = ofClamp(c,0,255);
            ofSetColor (c,c*0.3,255-c);
            ofDrawRectangle(xs + i*xperbuf, ys - j*yperbin, xperbuf, yperbin);
        }
    }
    
    ofSetColor(ofColor::white);
    for (int i=0; i < nframes-1; i++) {
        ofDrawLine(xs+i*xperbuf, ofMap(amplitudes[i],0,1,ys,ys-500,true), xs+(i+1)*xperbuf, ofMap(amplitudes[i+1],0,1,ys,ys-500,true));
    }
    gui.draw();
}

void ofApp::startRecording () {
    state = NOISE_PROF;
    noise_bufs_done = 0;
    delete recording;
    recording = new ClipVectorBuffer();
    recording->ensure(100501);
    ofSoundStreamStart();
}

void ofApp::stopRecording () {
    state = COMPUTE;
    ofSoundStreamStop();
    if (recording->length() < analysis_bufsize) return;
    if (spectra) {
        delete[] spectra;
        delete[] amplitudes;
    }
    cout << "Doing DFTs" << endl;
    nframes = recording->length()*buf_shift_frac / analysis_bufsize;
    spectra = new float[nframes * nparams];
    amplitudes = new float[nframes];
    Clip all = Clip (*recording);
    all.preemphasis (0.96f);
    Clip slice = Clip (*recording, 0, analysis_bufsize);
    slice.window = hamming_window;
//    float temp[analysis_bufsize];
//    float kernel[7] = {0.1, 0.3, 0.6, 1.0, 0.6, 0.3, 0.1};
    for (int i=0; i < nframes; i++) {
        amplitudes[i] = slice.rmsAmplitude();
        float *m = mfcc(slice, *fbank);
        memcpy(&spectra[i*nparams], m, nparams*sizeof(float));
        delete[] m;
        /*
        slice.dft(temp);
        convolve(&spectra[i*analysis_bufsize], temp, kernel, analysis_bufsize, 7);
        maxima[i] = argmax(&spectra[i*analysis_bufsize], analysis_bufsize/2);
        */
        slice.slide(analysis_bufsize/buf_shift_frac);
    }
    cout << "done" << endl;
    state = WAIT;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == ' ') {
        cout << "Space released" << endl;
        
        if (state == RECORD || state == NOISE_PROF) {
            cout << "ending recording" << endl;
            stopRecording();
        } else if (state == WAIT || state == START){
            cout << "Starting recording" << endl;
            startRecording();
            
        }
        
    }

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
