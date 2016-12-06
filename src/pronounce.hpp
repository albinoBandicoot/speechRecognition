//
//  pronounce.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/23/16.
//
//

#ifndef pronounce_hpp
#define pronounce_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
using namespace std;

namespace phone {
    
    static int NUM_PH = 40;

    enum phone {
        SIL = 0, AA, AE, AH, AO, AW, AY, B, CH, D, DH, EH, ER, EY, F, G, HH, IH, IY, JH, K, L, M, N, NG, OW, OY, P, R, S, SH, T, TH, UH, UW, V, W, Y, Z, ZH
    };
    
    enum subphone {
        BEGIN = 0, MIDDLE, END
    };
    
    class context {
    public:
        phone prev, curr, next;
        
        context (phone c) : prev(SIL), curr(c), next(SIL) {}
        context (phone p, phone c, phone n) : prev(p), curr(c), next(n) {}
        
        bool operator< (const context& c) const;
        void operator++ ();
        
    };
    
    
    // this maps a phone context to a canonical representative of its equivalence class
    class ties {
    public:
        
        enum mode {
            IDENT, PHONE_CLASSES, NULL_CONTEXT
        };
        
        map<phone, phone> c;
        //    map<phone::context, phone::context> ties;
        
        ties (mode m);
        
        context operator() (context ctx) ;
        
    };
    
    static const char* names[] = {"_", "AA", "AE", "AH", "AO", "AW", "AY", "B", "CH", "D", "DH", "EH", "ER", "EY", "F", "G", "HH", "IH", "IY", "JH", "K", "L", "M", "N", "NG", "OW", "OY", "P", "R", "S", "SH", "T", "TH", "UH", "UW", "V", "W", "Y", "Z", "ZH"};

}

typedef pair<phone::phone, phone::subphone> phspec;

phone::phone findPhone (string s);

class pronlist {
public:
    vector<vector<phone::phone> > pronunciations;
    
    pronlist ();
    
    void add (vector<phone::phone> pron);
};

class pronlex {     // pronunciation lexicon
public:
    map<string, pronlist> pr;
    
    pronlex ();
    pronlex (istream &in);
    
    void add (string w, vector<phone::phone> pron);
    pronlist* get (string w);
    
    vector<phone::phone> pronounce (string w);
    
};

class pronouncer {
public:
    pronlex main;
    pronlex prefixes;
    pronlex suffixes;
    
    pronouncer (const char *main_fn, const char *pre_fn, const char *suf_fn);
    
    vector<phone::phone> pronounce_word (string w);
    vector<phone::phone> pronounce (string s);
};

#endif /* pronounce_hpp */
