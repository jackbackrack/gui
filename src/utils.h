//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_UTILS__
#define __IS_UTILS__

#include <math.h>
#include <stddef.h>

typedef struct _time {
  int year;
  int month;
  int weekday;
  int day;
  int hour;
  int minute;
  int second;
  int milliseconds;
} Time;

extern void get_local_time (Time *time);
extern int time_year (Time *time);
extern int time_month (Time *time);
extern int time_day (Time *time);
extern int time_hours (Time *time);
extern int time_mins (Time *time);
extern int time_secs (Time *time);
extern int time_msecs (Time *time);

#define MAX_STRBUF 1024

extern float rndf ();
extern float rndf (float mn, float mx);

extern int rndi ();
extern int rndi (int lo, int hi);

extern "C" {

typedef struct {
  int idx;
  char data[MAX_STRBUF];
} Strbuf;

typedef float flo;
extern void post(const char* string, ...) ;
// extern void post(std::string string, ...) ;
extern void uerror(const char* string, ...) ;
// extern void uerror(std::string, ...) ;
extern void post_into(Strbuf *buf, char* pstring, ...);
extern void debug(const char* dstring, ...);
extern char *format_time (char *timecode, double time, char *secs_format);

extern void *MALLOC(unsigned long size);
extern void  FREE(void *ptr);
extern unsigned long memcount;
extern size_t get_mem_size( void );
extern double get_cpu_time( void );
}

#endif
