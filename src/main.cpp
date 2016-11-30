#include "ofMain.h"
#include "ofApp.h"
#include "mfcc.hpp"
#include "wav.hpp"
#include "corpus.hpp"
#include "langmodel.hpp"

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
    pronouncer pron("../res/lexicon/cmu.txt", "../res/lexicon/prefixes.txt", "../res/lexicon/suffixes.txt");
    ifstream lm("../res/ngram/unigram");
    unigram_model uni(lm,111452435771L);
    
    
    cout << "Probability of 'hello'" << uni.get("HELLO") << endl;
    cout << "Probability of 'smorgasbord'" << uni.get ("SMORGASBORD") << endl;
    
    return 0;
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
    
//    Clip sub(c,100000,200000);
//    write_wav (sub, 22050, 16, "/Users/Nathan/Desktop/test.wav");
//    write_features(c, 384, 192, *mel_filterbank(7200, 48, 384, 16000), "/Applications/of_v0.9.3_osx_release/apps/myApps/speakerID/res/test.mfcc");
}
