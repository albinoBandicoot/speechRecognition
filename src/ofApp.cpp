#include "ofApp.h"
#include "mfcc.hpp"
#include "wav.hpp"
#include "corpus.hpp"
#include "langmodel.hpp"
#include "utils.hpp"
#include "clip.hpp"
#include "gmm.hpp"
#include <vector>
//--------------------------------------------------------------
void ofApp::setup(){
    cout << "Loading pronunciation lexicon..." << endl;
    pronouncer pron("../../../../res/lexicon/cmu.txt", "../../../../res/lexicon/prefixes.txt", "../../../../res/lexicon/suffixes.txt");
    cout << "Loading language model..." << endl;
    ifstream lm("../../../../res/ngram/unigram_small");
    unigram_model uni(lm,111452435771L);
    
    cout << "Initializing acoustic model..." << endl;
    phone::ties acm_ties(phone::ties::NULL_CONTEXT);
    phone::ties hmm_ties(phone::ties::PHONE_CLASSES);
    acoustic_model acm (acm_ties, 1);
    
    cout << "Reading training data... " << endl;
    vector<string> folders = readlines (VOXFORGE_DIR + string("/_train"));
    vector<utterance> ut;
    for (int i=0; i < folders.size(); i++) {
        ifstream prompts(VOXFORGE_DIR + folders[i] + string("etc/PROMPTS").c_str());
        while (prompts.peek() != EOF) {
            try {
                ut.push_back (utterance(prompts, string(VOXFORGE_DIR), acm, pron));
            } catch (int ex) {
                cout << "Caught exception " << ex << endl;
            }
        }
    }
    cout << "Got " << ut.size() << " utterances" << endl;
    mu = mean (ut);
    sigma = variance (ut, mu);
    for (int i=0 ; i < FV_LEN; i++) {
        sigma[i] = sqrt(sigma[i]);
        cout << "COEFF " << i << ": mean = " << mu[i] << "; stddev = " << sigma[i] << endl;
    }
    
    recording = new ClipVectorBuffer();
    recording->ensure(500000);
    noise_spectrum = new ClipArrayBuffer(analysis_bufsize);
 //   fbank = linear_filterbank(100, 100, 35, 100, analysis_bufsize, 44100);
    fbank = mel_filterbank(7200, 48, analysis_bufsize, 44100);
    nframes = 0;
    nccoeffs = 13;//fbank->length();
    nscoeffs = 80;
    xscale = 1;
    
    ofSoundStreamSetup(0, 2, this, 44100, analysis_bufsize, 4);
    ofSoundStreamStop();
    gui.setup();
    gui.add(scale.setup("Cepstrum scale",0.5,-5,20,300,25));
    gui.add(offset.setup("Spectrum scale", 3.2, -15, 15, 300, 25));
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
        if (recording->length() > 500000-1) {
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
        ofDrawBitmapString ("...", 100, 650);
        return;
    }
    float xperbuf = xscale * 900.0f / nframes;
    float yperbin = 250.0f / nccoeffs;
    float xs = 50 - xshift;
    float ys = 650;
    
    int WL = 3;
    for (int i=WL; i < nframes-WL-1; i++) {
        for (int j=0; j < nccoeffs; j++) {
            float c = 0;
            for (int w = -WL; w <= WL; w++) {
                c += (WL - abs(w) + 1) * cepstra[(i+w)*nccoeffs+j];
            }
            c /= WL*WL;
            c -= mu[j];
            c /= sigma[j];
            //c = log(c+1);//pow(c,0.4);
            /*
            c += offset;
            c = ofMap(c, 0, 1, 0, exp(scale));
            c = ofClamp(c,0,255);
            ofSetColor (c,c*0.3,255-c);
             */
            c *= scale;
            c = ofClamp(c, -1, 1);
            if (c < 0) {
                ofSetColor (48-(c*200),48,48);
            } else {
                ofSetColor (48,48+c*200,48);
            }
            
            ofDrawRectangle(xs + i*xperbuf, ys - j*yperbin, xperbuf, yperbin);
        }
    }
    
    yperbin = 250.0f / nscoeffs;
    xs = 50 - xshift;
    ys = 350;
    
    float K = 0.1;
    float logK = log(K);
    for (int i=0; i < nframes; i++) {
        for (int j=0; j < nscoeffs; j++) {
            float c = spectra[i*nscoeffs+j];
            c = log(K+fabs(c)) - logK;
            c *= exp(offset);
            c = ofMap(c, 0, 1, 0, 4);
            if (c < 1) {
                ofSetColor (5+c*10,10+c*20, 10+c*130);
            } else if (c < 2) {
                c -= 1;
                ofSetColor (15+c*135, 30+c*10, 140-c*20);
            } else if (c < 3){
                c -= 2;
                ofSetColor (150+c*70, 40+c*80, 120-c*60);
            } else if (c < 4) {
                c -= 3;
                ofSetColor (220+c*25, 120+c*135, 60+c*195);
            } else {
                ofSetColor (255, 255, 255);
            }
            
            ofDrawRectangle(xs + i*xperbuf, ys - j*yperbin, xperbuf, yperbin);
        }
    }
    
    ys = 395;
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
    recording->ensure(500001);
    ofSoundStreamStart();
}

void ofApp::stopRecording () {
    state = COMPUTE;
    ofSoundStreamStop();
    if (recording->length() < analysis_bufsize) return;
    if (spectra) {
        delete[] cepstra;
        delete[] spectra;
        delete[] amplitudes;
    }
    // first normalize volume
    Clip all = Clip (*recording);
    Clip sl = Clip (all, 0, 5000);
    float max_amp = 0;
    int ct = 0;
    while (sl.slide(1500)) {
        max_amp += sl.rmsAmplitude();
        ct ++;
    }
    max_amp /= ct;
    float rms_amp = all.rmsAmplitude();
    cout << "RMS amplitude = " << rms_amp << endl;
    cout << "Max smoothed amplitude = " << max_amp << endl;
    all *= 200 / max_amp;
    cout << "Doing DFTs" << endl;
    nframes = recording->length()*buf_shift_frac / analysis_bufsize;
    spectra = new float[nframes * nscoeffs];
    cepstra = new float[nframes * nccoeffs];
    amplitudes = new float[nframes];
    all.preemphasis (0.96f);
    Clip slice = Clip (*recording, 0, analysis_bufsize);
    slice.window = hamming_window;
//    float temp[analysis_bufsize];
//    float kernel[7] = {0.1, 0.3, 0.6, 1.0, 0.6, 0.3, 0.1};
    float *s = new float[nscoeffs];
    for (int i=0; i < nframes; i++) {
        amplitudes[i] = slice.rmsAmplitude();
        
        slice.partial_dft(s, nscoeffs);
        memcpy (&spectra[i*nscoeffs], s, nscoeffs*sizeof(float));
        
        float *m = mfcc(slice, *fbank);
        memcpy(&cepstra[i*nccoeffs], m, nccoeffs*sizeof(float));
        delete[] m;
        /*
        slice.dft(temp);
        convolve(&spectra[i*analysis_bufsize], temp, kernel, analysis_bufsize, 7);
        maxima[i] = argmax(&spectra[i*analysis_bufsize], analysis_bufsize/2);
        */
        slice.slide(analysis_bufsize/buf_shift_frac);
    }
    delete[] s;
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
        
    } else if (key == '=') {
        xscale *= 1.35;
        
    } else if (key == '-') {
        xscale /= 1.35;
    } else if (key == 'l') {
        xshift += 300;
    } else if (key == 'h') {
        xshift -= 300;
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
