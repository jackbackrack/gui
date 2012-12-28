//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Copyright (c) 2003-2008, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "port_audio.h"

float samps_per_sec;

// ON WINDOWS
// export PA_MIN_LATENCY_MSEC=100 

extern "C" void post(const char* string, ...) ;

int audio_initialize (void) {
  int err = Pa_Initialize();
  if ( err != paNoError ) 
    post(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
  return err;
}
  
int  audio_open_stream (char* devname, AudioStream *stream, int buf_size, AudioCallback fun, void* userdata) {
    PaStreamParameters outputParameters;
    PaError err;
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
      post("Error: No default output device.\n");
      // goto error;
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              stream,
              NULL, /* no input */
              &outputParameters,
              SAMPS_PER_SEC,
              buf_size,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              fun,
              userdata );
    //   int err;
    //   err = Pa_OpenDefaultStream(
    //       stream,         /* passes back stream pointer */
    //       2,              /* stereo input */
    //       2,              /* stereo output */
    //       // paInt16,        /* 32 bit floating point output */
    //       paFloat32,      /* 32 bit floating point output */
    //       SAMPS_PER_SEC,  /* sample rate */
    //       buf_size,       /* frames per buffer */
    //       fun,            /* specify our custom callback */
    //       userdata );     /* pass our data through to callback */
  samps_per_sec = SAMPS_PER_SEC;
}

extern int do_graphics_loop (void);

void audio_dispatch (AudioStream stream, int is_graphics) {
  if (is_graphics) 
    do_graphics_loop();
  else {
    for (;;) {
      Pa_Sleep(1000);
    }
  }
}

int  audio_start_stream (AudioStream stream) { 
  return Pa_StartStream(stream);
}

int  audio_stop_stream (AudioStream stream) { 
  return Pa_StopStream(stream);
}

int  audio_close_stream (AudioStream stream) { 
  return Pa_CloseStream(stream);
}

void audio_terminate (void) {
  Pa_Terminate();
}

