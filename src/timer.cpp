//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "utils.h"
#include "time.h"

int is_offline = 0;
flo target_fps = 15.0;
int ticks = 0;
double last_tval = 0.0;

#ifdef __WIN32__
#include <windows.h>

double do_get_secs () {
  if (is_offline) {
    return ticks * 1.0 / target_fps;
  } else {
    DWORD tv = timeGetTime();
    return (double)(tv / 1000.0);
  }
}

#else

#include <sys/time.h>

double do_get_secs () { 
  if (is_offline) {
    return ticks * 1.0 / target_fps;
  } else {
    struct timeval t;
    gettimeofday(&t, NULL);
    double res = (double)(t.tv_sec + t.tv_usec / 1000000.0);
    return res;
  }
}

#endif

#define INIT_REM -1000000
static double rem = INIT_REM;

static double start_secs = 0.0;
static double now_secs   = 0.0;

// get_secs(): configure_motes() connects to last_time, 
//   dump_mote_state() [on motes], idle(), is_machine_eligible(),
//   exec_machine_timed(), random seed in main(), 
//   a similar fn is used 5 times in ProtoM.nc
double get_secs(void) { return now_secs; }
// set_secs(): used only in sim-opengl.c:render()
double set_secs(void) { return now_secs = do_get_secs() - start_secs; }
// get_msecs(): not used in simulator; a similar fn is used 1 time in ProtoM.nc
int    get_msecs(void) { return (int)(get_secs() * 1000); }

// This time setup appears at first to be a bit weird.  The reason is that
// we need to call all of the simulated nodes with the time, but don't want
// to call gettimeofday a bazillion times.

// used in main to initialize the internal time
void set_base_time_at(double offset) {  
  rem = INIT_REM;
  start_secs = do_get_secs() + offset; 
  set_secs();
  // root_sim->t = 0.0;
  // root_sim->dt = 0.0;
}
void set_base_time() {  
  set_base_time_at(0.0);
}

