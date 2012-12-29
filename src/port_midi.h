//////////////////////////////////////////////////////////////////////
// Copyright (c) 2003-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_PORT_MIDI__
#define __IS_PORT_MIDI__

#include "portmidi.h"

extern void sys_pollmidiqueue( void );
extern void sys_putmidimess(int portno, int a, int b, int c);
extern void sys_close_midi ( void );
extern void sys_init_midi( void );

#define N_SLIDERS 8

#endif
