//////////////////////////////////////////////////////////////////////
// Copyright (c) 2003-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_PORT_AUDIO__
#define __IS_PORT_AUDIO__

#include "portaudio.h"

// #define N_STREAMS 6
#define N_STREAMS 1
// #define BUF_SIZE 1008
#define BUF_SIZE 64
// #define BUF_SIZE 256
#define BUF_PAD  10

#define MAX_BUF_SIZE (MAX_CHANNELS * (BUF_SIZE + BUF_PAD))

#define MAX_CHANNELS 6
#define STEREO
#define N_CHANNELS        2
#define N_INPUT_CHANNELS  2
#define N_OUTPUT_CHANNELS 2

#define N_DATA     BUF_SIZE

#define SAMPS_PER_SEC 44100

typedef void* AudioStream;
typedef double AudioTime;

typedef float Aud;

#define AudioCallback PaStreamCallback

int  audio_initialize (void);
void audio_terminate (void);
void audio_dispatch (AudioStream, int is_graphics);
int  audio_open_stream (char* devname, AudioStream *stream, int buf_size, AudioCallback fun, void* userdata);
int  audio_start_stream (AudioStream stream);
int  audio_stop_stream (AudioStream stream);
int  audio_close_stream (AudioStream stream);

extern float samps_per_sec;

#endif
