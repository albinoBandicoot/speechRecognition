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

/* Definitions of types for phones, phone contexts, mappings for use in parameter tying, as well as 
   the pronunciation lexicon. 
*/

namespace phone {
    
    static int NUM_PH = 40;

    // list of all phones
    enum phone {
        SIL = 0, AA, AE, AH, AO, AW, AY, B, CH, D, DH, EH, ER, EY, F, G, HH, IH, IY, JH, K, L, M, N, NG, OW, OY, P, R, S, SH, T, TH, UH, UW, V, W, Y, Z, ZH
    };
    
    // for use with subphone states. Not currently implemented fully.
    enum subphone {
        BEGIN = 0, MIDDLE, END
    };
   
    // represents a phone along with 1 phone of left and right context.
    class context {
    public:
        phone prev, curr, next;
        
        context (phone c) : prev(SIL), curr(c), next(SIL) {}
        context (phone p, phone c, phone n) : prev(p), curr(c), next(n) {}
        
        // these are useful for iterating over all contexts
        bool operator< (const context& c) const;
        context operator++ ();
        
    };
    
    
    // this maps a phone context to a canonical representative of its equivalence class, for parameter tying
    class ties {
    public:
        
        // three different modes currently implemented: IDENT just maps each context to itself.
        // PHONE_CLASSES maps the previous and next phones to a representative of their general phone class,
        // leaving curr as is.
        // NULL_CONTEXT maps each context to SIL curr SIL; that is, all that matters is the current phone, no
        // context considered.
        enum mode {
            IDENT, PHONE_CLASSES, NULL_CONTEXT
        };
        
        map<phone, phone> c;    // this map controls how the previous and next phones are mapped to their representative.
                                // curr is always left alone
        
        ties (mode m);
        
        context operator() (context ctx) ;
        
    };
    
    static const char* names[] = {"_", "AA", "AE", "AH", "AO", "AW", "AY", "B", "CH", "D", "DH", "EH", "ER", "EY", "F", "G", "HH", "IH", "IY", "JH", "K", "L", "M", "N", "NG", "OW", "OY", "P", "R", "S", "SH", "T", "TH", "UH", "UW", "V", "W", "Y", "Z", "ZH"};

}

typedef pair<phone::phone, phone::subphone> phspec;

phone::phone findPhone (string s);

/* Pronunciation Lexicon */

// a list of pronunciations (phone vectors) for a given word.
class pronlist {
public:
    vector<vector<phone::phone> > pronunciations;
    
    pronlist ();
    
    void add (vector<phone::phone> pron);
};

// represents a whole pronunciation lexicon. Maps strings to their pronunciation lists
class pronlex {
public:
    map<string, pronlist> pr;
    
    pronlex ();
    pronlex (istream &in);
    
    void add (string w, vector<phone::phone> pron);
    pronlist* get (string w);
    
    vector<phone::phone> pronounce (string w);
    
};

// incorporates a lexicon of prefixes and suffixes, to aid in pronouncing words not in the
// main lexicon.
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
