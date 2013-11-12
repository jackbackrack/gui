//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include "utils.h"
#include <unistd.h>
// #include "gc.h"
#define GC_malloc malloc
#define GC_free   free

void post(const char* string, ...) {
  va_list ap;
  va_start(ap, string);
  vfprintf(stdout, string, ap);
  va_end(ap);
  fflush(stdout);
}

void post(std::string string, ...) {
  va_list ap;
  va_start(ap, string);
  vfprintf(stdout, string.c_str(), ap);
  va_end(ap);
  fflush(stdout);
}

void post_into(Strbuf *buf, char* pstring, ...) {
  int n;
  va_list ap;
  va_start(ap, pstring);
  buf->idx += vsprintf(&buf->data[buf->idx], pstring, ap);
  va_end(ap);
  fflush(stdout);
}

void debug(const char* dstring, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, dstring);
  vsprintf(buf, dstring, ap);
  va_end(ap);
  fputs(buf, stderr);
  fflush(stderr);
}

void uerror(const char* string, ...) {
  va_list ap;
  va_start(ap, string);
  vfprintf(stdout, string, ap);
  va_end(ap);
  fflush(stdout);
}

void uerror(std::string string, ...) {
  va_list ap;
  va_start(ap, string);
  vfprintf(stdout, string.c_str(), ap);
  va_end(ap);
  fflush(stdout);
}

int time_year (Time *time) {
  return time->year;
}

int time_month (Time *time) {
  return time->month;
}

int time_day (Time *time) {
  return time->day;
}

int time_hours (Time *time) {
  return time->hour;
}

int time_mins (Time *time) {
  return time->minute;
}

int time_secs (Time *time) {
  return time->second;
}

int time_msecs (Time *time) {
  return time->milliseconds;
}


char *fill_zeroes (char *text, char *format, flo x, int n) {
  int i, k = n - (int)log10(x);
  if (x >= 1000)
    k = 4;
  else if (x >= 100)
    k = 3;
  else if (x >= 10)
    k = 2;
  else 
    k = 1;
  k = n - k;
  for (i = 0; i < k; i++)
    text[i] = '0';
  sprintf(&text[i], format, x);
  return text;
}

double extract_time (double time, int *hours, int *mins, flo *secs) {
  *hours = (int)(time / 3600);
  time  -= *hours*3600;
  *mins  = (int)(time / 60);
  time  -= *mins*60;
  *secs  = (flo)time;
  return (double)time;
}

char *format_time (char *timecode, double time, char *secs_format) {
  char hourscode[10];
  char minscode[10];
  char secscode[10];
  int  hours, mins;
  flo secs;

  extract_time(time, &hours, &mins, &secs);

  fill_zeroes(hourscode, "%.0f", (flo)hours, 2);
  fill_zeroes(minscode,  "%.0f", (flo)mins, 2);
  fill_zeroes(secscode,  secs_format, (flo)secs, 2);
  sprintf(timecode, "%s:%s:%s", hourscode, minscode, secscode);
  return timecode;
}

unsigned long memcount = 0;

void *MALLOC(unsigned long size) {
  if (size <= 0)
    return NULL;
  else { 
    void *res = GC_malloc(size);
    // post("M %ld\n", memcount);
    if (res == NULL) {
      // debug("Unable to allocate\n");
      uerror("UNABLE TO ALLOCATE");
    }
    memcount += size;
    // post("MALLOC %p\n", res);
    return res;
  }
}

void FREE(void *ptr_) {
  void **ptr = (void**)ptr_;
  // post("FREEING %p\n", *ptr);
  if (*ptr == NULL)
    uerror("TRYING TO FREE ALREADY FREED MEM\n");
  GC_free(*ptr);
  *ptr = NULL;
}

void set_rnd ( unsigned int seed ) {
  srand(seed);
}

float rndf () {
  return (float)((double)rand() / (double)RAND_MAX);
}

float rndf (float mn, float mx) {
  return rndf() * (mx-mn) + mn;
}

int rndi () {
  return rand();
}

int rndi (int mn, int mx) {
  return (rndi() % (mx-mn+1)) + mn;
}

size_t get_mem_size ( void ) {
  pid_t pid = getpid();
  char cmd[256];
  char res_filename[256];
  sprintf(res_filename, "vsz-%d.txt", pid);
  sprintf(cmd, "ps -av | grep %d | head | awk '{print $8}' > %s", pid, res_filename);
  system(cmd);
  FILE *f = fopen(res_filename, "r");
  size_t vsz;
  fscanf(f, "%ld", &vsz);
  fclose(f);
  sprintf(cmd, "rm %s", res_filename);
  system(cmd);
  return vsz * 1024;
}

#include <sys/resource.h>

double get_cpu_time( void ) { 
  struct timeval tim;        
  struct rusage ru;        
  getrusage(RUSAGE_SELF, &ru);        
  tim=ru.ru_utime;        
  double t=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0;        
  tim=ru.ru_stime;        
  t+=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0;        
  return t; 
}        
