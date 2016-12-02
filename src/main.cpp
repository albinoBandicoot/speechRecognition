#include "ofMain.h"
#include "ofApp.h"
#include "mfcc.hpp"
#include "wav.hpp"
#include "corpus.hpp"
#include "langmodel.hpp"
#include "utils.hpp"
#include "clip.hpp"
#include <dirent.h>

#define VOXFORGE_DIR "/Users/Nathan/Documents/2016H/Security/project/audio/voxforge/"
//========================================================================
int main( ){
//	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
//	ofRunApp(new ofApp());
    
    char cwd[256];
    getcwd(cwd, 256);
    cout << "Current dir: " << cwd << endl;
//    pronouncer pron("../res/lexicon/cmu.txt", "../res/lexicon/prefixes.txt", "../res/lexicon/suffixes.txt");
    ifstream lm("../res/ngram/unigram");
//    unigram_model uni(lm,111452435771L);
    
    filterbank fbank = *mel_filterbank(7200, 48, 320, 16000);
    
    ofstream err(VOXFORGE_DIR + string("bad"));
    
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
            
            try {
                write_features(wav, 320, 160, fbank, VOXFORGE_DIR + folders[i] + string("mfc/") + wavname);
                delete wav.buf;
            } catch (int e) {
                if (e == 1) {
                    cout << "IO ERROR" << endl;
                    err << folders[i] << endl;
                }
            }
        }
    }
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
#if 0
    Clip c = read_wav ("/Applications/of_v0.9.3_osx_release/apps/myApps/speakerID/res/wav/02.wav");
    cout << "File reading complete" << endl;
    ifstream in("/Applications/of_v0.9.3_osx_release/apps/myApps/speakerID/res/trn/02.trn");
    vector<utterance> ut;
    while (in.good()) {
        try {
            utterance u(in, c, 22050, pron);
            ut.push_back(u);
        } catch (int ex) {
            cout << "Caught exception " << ex << "; last utterance is " << ut[ut.size()-1].text << endl;
        }
      /*
        cout << u.text << endl;
        for (int i=0; i < u.pronunciation.size(); i++) {
            cout << phone::names[u.pronunciation[i]] << " ";
        }
        cout << endl;
       */
    }
#endif
//    Clip sub(c,100000,200000);
//    write_wav (sub, 22050, 16, "/Users/Nathan/Desktop/test.wav");
//    write_features(c, 384, 192, *mel_filterbank(7200, 48, 384, 16000), "/Applications/of_v0.9.3_osx_release/apps/myApps/speakerID/res/test.mfcc");
}
