//
//  pronounce.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/23/16.
//
//

#include "pronounce.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

pronlist::pronlist () {
}

void pronlist::add (vector<phone::phone> pr) {
    pronunciations.push_back(pr);
}

phone::phone findPhone (string w) {
    int num_idx = w.find_first_of("0123456789");
    if (num_idx != -1) {
        w.erase(w.begin()+num_idx);
    }
    for (int i=0; i < phone::NUM_PH; i++ ) {
        if (w == phone::names[i]) {
            return (phone::phone) i;
        }
    }
}

pronlex::pronlex () {
}

pronlex::pronlex (istream &in) {
    char buf[256];
    memset (buf, 0, 256);
    if (in.fail()) {
        cout << "Failed to open pronunciation lexicon file " << endl;
        exit(1);
    }
    while (in.good()) {
        in.getline(buf, 256);
        if (buf[0] == ';') continue;    // comment
      //  cout << "Line is: " << buf << endl;
        istringstream is((string(buf)));
        string word, ph;
        is >> word;
        int paren_idx = word.find("(");
        if (paren_idx != -1) {
            word.erase(word.begin()+paren_idx, word.end());
        }
        vector<phone::phone> p;
        while (is.good()) {
            is >> ph;
            p.push_back (findPhone(ph));
        }
        /*
        cout << "adding pronunciation for " << word << ": ";
        for (int i=0; i < p.size(); i++) {
            cout << phone::names[p[i]] << " ";
        }
        cout << endl;
         */
        pronlist &pl = pr[word];    // this will create it if it doesn't exist
        pl.add(p);
    }
}

pronlist *pronlex::get (string w) {
    if (pr.count(w) == 1) {
        return &pr[w];
    }
    return NULL;
}
/*
vector<phone::phone> pronlex::pronounce (string s) {
    vector<phone::phone> res;
    istringstream is(s);
    bool first = true;
    while (is.good()) {
        string w;
        is >> w;
        if (w[w.length()-1] == '-') w = w.substr(0,w.length()-1);
        pronlist *p = get(w);
        if (p) {
            res.push_back (phone::SIL);
            vector<phone::phone> wp = p->pronunciations[0];
            for (int i=0; i < wp.size(); i++) {
                res.push_back(wp[i]);
            }
        } else {
            // first check for common suffixes
            // now check to see if we can partition the word into two words in the dictionary
            cout << "Warning! Cannot pronounce " << w << endl;
        }
    }
    res.push_back(phone::SIL);
    return res;
}
*/
pronouncer::pronouncer (const char *main_fn, const char *pre_fn, const char *suf_fn) {
    ifstream m_in(main_fn), p_in(pre_fn), s_in(suf_fn);
    main = *new pronlex (m_in);
    prefixes = *new pronlex (p_in);
    suffixes = *new pronlex (s_in);
}

void append_pron (vector<phone::phone> res, vector<phone::phone> wp, bool leading_silence=true) {
    if (leading_silence) res.push_back (phone::SIL);
    for (int i=0; i < wp.size(); i++) {
        res.push_back(wp[i]);
    }
}

void append_pron (vector<phone::phone> res, pronlist *p, bool leading_silence=true) {
    append_pron (res, p->pronunciations[0]);
}

vector<phone::phone> pronouncer::pronounce_word (string w) {
    vector<phone::phone> res;
    if (w[w.length()-1] == '-') w = w.substr(0,w.length()-1);
    pronlist *p = main.get(w);
    if (p) {
        append_pron(res, p);
        return res;
    } else if (w.length() >= 4) {
        // first check for hyphens
        int idx = w.find('-');
        if (idx != string::npos) {
            bool first = true;
            while ((idx = w.find('-')) != string::npos) {
                string pre = w.substr(0,idx);
                append_pron (res, pronounce_word (pre), first);
                w = w.substr(idx+1, w.length()-(idx+1));
                first = false;
            }
            append_pron (res, pronounce_word (w));
            return res;
        }
        // first check for common affixes
        // first just suffix
        for (map<string,pronlist>::iterator iter2 = suffixes.pr.begin(); iter2 != suffixes.pr.end(); iter2++) {
            string suf = (*iter2).first;
            if (w.length() >= suf.length() and w.substr(w.length()-suf.length(), w.length()) == suf) {
                p = main.get (w.substr(0, w.length()-suf.length()));
                if (p) {
                    append_pron (res, p);
                    return res;
                }
            }
        }
        // now prefix
        for (map<string,pronlist>::iterator iter = prefixes.pr.begin(); iter != prefixes.pr.end(); iter++) {
            string pre = (*iter).first;
            if (pre.length() < w.length() and w.substr(0,pre.length()) == pre) {
                p = main.get(w.substr(pre.length(), w.length()-pre.length()));   // first alone
                if (p) {
                    append_pron (res, p);
                    return res;
                } else {
                    // now also with suffix
                    for (map<string,pronlist>::iterator iter2 = suffixes.pr.begin(); iter2 != suffixes.pr.end(); iter2++) {
                        string suf = (*iter2).first;
                        if (w.length()-pre.length() >= suf.length() and w.substr(w.length()-suf.length(), suf.length()) == suf) {
                            p = main.get (w.substr(pre.length(), w.length()-suf.length()-pre.length()));
                            if (p) {
                                append_pron (res, p);
                                return res;
                            }
                        }
                    }
                }
            }
            
            // now check to see if we can partition the word into two words in the dictionary
            for (int split=0; split < w.length()/2-1; split++) {    // this should make 3-letter minimum
                for (int side = -1; side <= 1; side += 2) {
                    int s = w.length()/2 + side*split;
                    string pre = w.substr(0, s);
                    string suf = w.substr(s, w.length()-s);
                    pronlist *p = main.get(pre);
                    if (p) {
                        pronlist *q = main.get(suf);
                        if (q) {
                            append_pron (res, p, true);
                            append_pron (res, q, false);
                            return res;
                        }
                    }
                    if (split == 0) break;  // second iteration would be a duplicate
                }
            }
            cout << "Warning! Cannot pronounce " << w << endl;
            throw 1;
        }
    }
}

vector<phone::phone> pronouncer::pronounce (string s) {
    vector<phone::phone> res;
    istringstream is(s);
    bool first = true;
    while (is.good()) {
        string w;
        is >> w;
        try {
            vector<phone::phone> word_pron = pronounce_word(w);
            for (int i=0; i < word_pron.size(); i++) {
                res.push_back (word_pron[i]);
            }
        } catch (int ex) {
            
        }
    }
    res.push_back(phone::SIL);
    return res;
}