#include "ofMain.h"
#include "ofApp.h"
#include "mfcc.hpp"
#include "wav.hpp"
#include "corpus.hpp"
#include "langmodel.hpp"
#include "utils.hpp"
#include "clip.hpp"
#include "gmm.hpp"
#include <dirent.h>

//#define DEMO
//#define MFCC
#define TRAIN

//========================================================================
int main( ){
#ifdef DEMO
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
#endif
#ifdef TRAIN
    // MODEL TRAINING
    char cwd[256];
    getcwd(cwd, 256);
    cout << "Current dir: " << cwd << endl;
    cout << "Loading pronunciation lexicon..." << endl;
    pronouncer pron("../res/lexicon/cmu.txt", "../res/lexicon/prefixes.txt", "../res/lexicon/suffixes.txt");
    cout << "Loading language model..." << endl;
    ifstream lm("../res/ngram/unigram_small");
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
    featurevec mu = mean (ut);
    featurevec var = variance (ut, mu);
    acm.initialize (mu, var);
    
    cout << "Constructing lexicon HMM..." << endl;
    
    hmm h(pron.main, uni, &acm, new state_model(&hmm_ties));
    cout << "Starting Embedded Training... " << endl;
    h.train (ut);
#endif
#ifdef MFCC
    // THIS IS WHAT IS NEEDED FOR COMPUTING MFCCs FOR NEW FILES
    filterbank fbank = *mel_filterbank(7200, 48, 320, 16000);
    
    vector<string> folders = readlines (VOXFORGE_DIR + string("/_list"));
    for (int i=0; i < folders.size(); i++) {
        cout << "Working on folder " << i << endl;
        vector<string> prompts = readlines ((VOXFORGE_DIR + folders[i] + string("etc/PROMPTS")).c_str());
        for (int j=0; j < prompts.size(); j++) {
            int s = prompts[j].find_last_of('/');
            int e = prompts[j].find_first_of (' ');
            string wavname = prompts[j].substr(s, e-s);
            
            string wav_file = VOXFORGE_DIR + folders[i] + string("wav/") + wavname + ".wav";
            cout << "Reading WAV " << wav_file << endl;
            Clip wav = read_wav (wav_file);
            cout << "Clip amplitude: " << wav.rmsAmplitude() << endl;
            wav.normalizeVolume (1);
            cout << "normalized: " << wav.rmsAmplitude() << endl;
            
            try {
                write_features(wav, 320, 160, fbank, VOXFORGE_DIR + folders[i] + string("mfc/") + wavname);
                delete wav.buf;
            } catch (int e) {
                if (e == 1) {
                    cout << "IO ERROR" << endl;
                }
            }
        }
    }
#endif
     
    /*
    struct dirent *ent;
    DIR *dp;
    dp = opendir ("/Users/Nathan/Documents/2016H/Security/project/audio/voxforge");
    if (dp != NULL) {
        while ((ent = readdir(dp))) {
            DIR *subdir;
            subdir = opendir(ent->d_name);
            while ((sub))
        }
    }
*/
}
