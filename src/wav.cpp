//
//  wav.cpp
//  speakerID
//
//  Created by Nathan Paige on 11/27/16.
//
//

#include "wav.hpp"

/* The WAV file format is as follows:
 
 0  ChunkID  = 'RIFF'
 4  ChunkSize  = file length - 8
 8  Format  = 'WAVE'
 
 12 Subchunk1ID  = 'fmt '
 16 Subchunk1Size = 16
 20 AudioFormat  = 1 (for PCM)
 22 NumChannels
 24 SampleRate (Hz)
 28 ByteRate = SampleRate * NumChannels * bytesPerChannel
 32 BlockAlign = NumChannels * bytesPerSample
 34 BitsPerSample 
 
 36 Subchunk2ID = 'data'
 40 Subchunk2Size = NumSamples * NumChannels * BytesPerSample
 44 Data
 
*/

wav_io_exception::wav_io_exception (const char*msg) {
    m = msg;
}

float read_sample_mono (FILE *in, int bps) {
    if (bps == 8) {
        unsigned char s;
        fread (&s, 1, 1, in);
        return s/255.0f;
    } else if (bps == 16) {
        signed short s;
        fread (&s, 2, 1, in);
        return s/32768.0f;
    }
}

float read_sample (FILE *in, int nchannels, int bps) {
    if (nchannels == 1) return read_sample_mono (in, bps);
    float left  = read_sample_mono (in, bps);
    float right = read_sample_mono (in, bps);
    return (left+right)/2;
}

Clip read_wav (char *fname) {
    FILE *in = fopen (fname, "r");
    if (in == NULL) throw wavex_fopen_fail;
    char buf[4];
    fread (buf, 1, 4, in);
    if (strncmp(buf,"RIFF",4)) throw wavex_not_riff;
    fseek (in, 4, SEEK_CUR);
    fread (buf, 1, 4, in);
    if (strncmp(buf,"WAVE",4)) throw wavex_not_wave;
    fread (buf, 1, 4, in);
    if (strncmp(buf,"fmt ",4)) throw wavex_bad;
    int sc1size, sample_rate;
    short aformat, nchannels, bps;
    fread (&sc1size, 4, 1, in);
    //if (sc1size != 16) throw wavex_bad;
    fread (&aformat, 2, 1, in);
    fread (&nchannels, 2, 1, in);
    fread (&sample_rate, 4, 1, in);
    fseek (in, 6, SEEK_CUR);
    fread (&bps, 2, 1, in);
    
    if (aformat != 1) throw wavex_not_pcm;
    cout << "Found " << nchannels << " channels, at " << sample_rate << "Hz; bit depth: " << bps << endl;
    
    fread (buf, 1, 4, in);
    if (strncmp (buf,"data",4)) throw wavex_bad;
    int data_len;
    fread (&data_len, 4, 1, in);
    int nframes = data_len / nchannels / (bps/8);
    cout << "There are " << nframes << " frames totalling " << data_len << " bytes" << endl;
    
    ClipArrayBuffer *abuf = new ClipArrayBuffer(nframes);
    for (int i=0; i < nframes; i++) {
        (*abuf)[i] = read_sample (in, nchannels, bps);
    }
    return Clip(*abuf);
}

void write_wav (Clip c, int sps, short bps, char *fname) {
    FILE *out = fopen (fname, "w");
    if (out == NULL) throw 1;
    short aformat = 1;
    short nchannels = 1;
    int data_len = bps/8 * c.length();
    int sc2size = data_len;
    int sc1size = 16;
    int chsize = data_len + 36;
    int byte_rate = sps * bps/8 * nchannels;
    short block_align = nchannels * bps/8;
    
    fwrite ("RIFF", 1, 4, out);
    fwrite (&chsize, 4, 1, out);
    fwrite ("WAVE", 1, 4, out);
    
    fwrite ("fmt ", 1, 4, out);
    fwrite (&sc1size, 4, 1, out);
    fwrite (&aformat, 2, 1, out);
    fwrite (&nchannels, 2, 1, out);
    fwrite (&sps, 4, 1, out);
    fwrite (&byte_rate, 4, 1, out);
    fwrite (&block_align, 2, 1, out);
    fwrite (&bps, 2, 1, out);
    
    fwrite ("data", 1, 4, out);
    fwrite (&sc2size, 4, 1, out);
    
    for (int i=0; i < c.length(); i++) {
        if (bps == 8) {
            unsigned char s = (unsigned char) ((CLAMP(c[i], -1, 1)/2+0.5f)*255);
            fwrite (&s, 1, 1, out);
        } else if (bps == 16) {
            signed short s = (signed short) (CLAMP(c[i],-1,1)*32767);
            fwrite (&s, 2, 1, out);
        } else {
            throw 2;
        }
    }
    fclose (out);
}