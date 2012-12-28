//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "port_midi.h"
#include "porttime.h"
// #include "utils.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "utils.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
// typedef PmTimestamp (*PmTimeProcPtr)(void *time_info);
PtTimestamp TIME_PROC (void *time_info) { return Pt_Time(); }
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define MIDINOTEOFF 0x80
#define MIDINOTEON  0x90
#define MIDICONTROL 0xc0

static int n_midi_ins = 0;
static int n_midi_outs = 0;
static PmStream * midi_in;
static PmStream * midi_out;

extern void inmidi_noteon(int portno, int channel, int pitch, int velo);
extern void inmidi_control(int portno, int channel, int pitch, int velo);
extern void inmidi_misc(int portno, int channel, int dat1, int dat2);

#define STRING_MAX 100

static void prompt_and_exit(void) {
  char line[80];
  post("type ENTER...");
  fgets(line, STRING_MAX, stdin);
  // this will clean up open ports:
  exit(-1);
}

void Debug(PmError error) {
  /* note that errors are negative and some routines return
   * positive values to indicate success status rather than error
   */
  if (error < 0) {
    post("PortMidi call failed...\n");
    post(Pm_GetErrorText(error));
    prompt_and_exit();
  }
}

void DebugStream(PmError error, PortMidiStream * stream) {
  if (error == pmHostError) {
    char msg[PM_HOST_ERROR_MSG_LEN];
    // this function handles bogus stream pointer
    // Pm_GetHostErrorText(stream, msg, PM_HOST_ERROR_MSG_LEN);
    Pm_GetHostErrorText(msg, PM_HOST_ERROR_MSG_LEN);
    post(msg);
    prompt_and_exit();
  } else if (error < 0) {
    Debug(error);
  }
}

void sys_init_midi( void ) {
  PmError status, length;
  PmEvent buffer[1];
  int i, n, k, l;

  /* list device information */
  n = Pm_CountDevices();
  midi_in = NULL;
  midi_out = NULL;
  if (n > 0) {
    for (n_midi_ins = n_midi_outs = i = 0; i < n; i++) {
      const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
      post("%d: %s, %s", i, info->interf, info->name);
      if (info->input) { k = i; n_midi_ins++; post(" (input)"); }
      if (info->output) { l = i; n_midi_outs++; post(" (output)"); }
      post("\n");
    }

    post("NINPUTS %d K %d\n", n_midi_ins, k);

    if (n_midi_ins >= 1) {

    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;

    /* open input device */
    Debug(Pm_OpenInput(&midi_in, 
		       k,
		       DRIVER_INFO, 
		       INPUT_BUFFER_SIZE, 
		       TIME_PROC, 
		       TIME_INFO 
		       /* , NULL */ // no midi thru
		       )); 

    post("Midi Input opened. Reading Midi messages...\n");
    /* empty the buffer after setting filter, just in case anything
       got through */
    Pm_SetFilter(midi_in, PM_FILT_ACTIVE | PM_FILT_CLOCK);
    /* now start paying attention to messages */
    while (Pm_Poll(midi_in)) {
      // post("FOUND MSG %d\n", status);
      // DebugStream(Pm_Read(midi_in, buffer, 1), midi_in);
      Pm_Read(midi_in, buffer, 1);
      // post("READ %d\n", status);
    }
  }

#define MIN(a,b) ((a)<(b) ? (a) : (b))

  if (n_midi_outs >= 1) {
    Debug(Pm_OpenOutput(&midi_out, MIN(2, l), NULL, 0, NULL, NULL, 0));
    post("Midi Output opened.\n");
  }
  }
}

void sys_putmidimess(int portno, int a, int b, int c) {
  if (midi_out != NULL) {
  PmEvent buffer;
  // fprintf(stderr, "put 1 msg %d %d\n", portno, n_midi_outs); 
  // if (portno >= 0 && portno < n_midi_outs)
  // {
  buffer.message = Pm_Message(a, b, c);
  buffer.timestamp = 0;
  // fprintf(stderr, "put msg\n"); 
  Pm_Write(midi_out, &buffer, 1);
  // }
  }
}

void sys_pollmidiqueue( void ) {
  int status, length;
  PmEvent buffer[1];
  if (midi_in != NULL) {
  while (1) {
    status = Pm_Poll(midi_in);
    if (status == TRUE) {
      length = Pm_Read(midi_in,buffer,1);
      if (length > 0) {
	long msg = buffer[0].message;
	int cmd  = (msg & 0xf0);
	int chan = (msg & 0xf);
	int dat1 = Pm_MessageData1(msg);
	int dat2 = Pm_MessageData2(msg);
	switch (cmd) {
	case MIDINOTEON:  inmidi_noteon(0, chan, dat1, dat2); break;
	case MIDINOTEOFF: inmidi_noteon(0, chan, dat1, 0);    break;
	case MIDICONTROL: inmidi_control(0, chan, dat1, dat2);    break;
	default:          inmidi_misc(cmd, chan, dat1, dat2); break;
	  // post("GOT MIDI INPUT %d %d %d %d\n", cmd, chan, dat1, dat2);
	}
      } 
    } else
      return;
  }
  }
}

void sys_close_midi ( void ) {
  if (midi_in != NULL) {
    /* close device (this not explicitly needed in most implementations) */
    post("ready to close...");
    Pm_Close(midi_in);
    post("done closing...");
  }
  if (midi_out != NULL) {
    /* close device (this not explicitly needed in most implementations) */
    post("ready to close...");
    Pm_Close(midi_out);
    post("done closing...");
  }
}

