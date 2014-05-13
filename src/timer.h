//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_TIME__
#define __IS_TIME__

#include "utils.h"

extern double get_secs (void);
extern double set_secs (void);
extern void set_base_time (void);
extern void set_base_time_at (double offset);
extern double last_tval;
extern int is_offline;
extern flo target_fps;
extern int ticks;
extern flo set_target_fps( flo ntfps );
extern flo get_target_fps( void );

#endif
