//
//  wav.hpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#ifndef wav_hpp
#define wav_hpp

#include <stdio.h>
#include <exception>
#include "clip.hpp"

class wav_io_exception : public exception {
public:
    const char *m;
    
    wav_io_exception (const char *msg);
    
    virtual const char *what () const throw() {
        return m;
    }
};

static wav_io_exception wavex_not_riff("Not an RIFF file"),
                        wavex_not_wave("Not a WAV file"),
                        wavex_not_pcm ("Not a PCM-encoded WAV file"),
                        wavex_bad ("Bad WAV file"),
                        wavex_fopen_fail ("Could not open WAV file");

Clip read_wav (char *file);
void write_wav (Clip c, int sps, short bps, char *fname);

#endif /* wav_hpp */
