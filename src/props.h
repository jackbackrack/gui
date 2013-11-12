//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef IS_PROPS
#define IS_PROPS

#include "lisp.h"
#include "utils.h"
#include <deque>
#include <vector>
#include <string>
#include <sstream>

class prop_t;
class props_t;

typedef int      (*vec_set_fun_t)(prop_t *spec, void* obj, vec_t<3> val);
typedef vec_t<3> (*vec_get_fun_t)(prop_t *spec, void* obj);
typedef int      (*flo_set_fun_t)(prop_t *spec, void* obj, float val);
typedef float    (*flo_get_fun_t)(prop_t *spec, void* obj);
typedef int      (*int_set_fun_t)(prop_t *spec, void* obj, int val);
typedef int      (*int_get_fun_t)(prop_t *spec, void* obj);
typedef int      (*str_set_fun_t)(prop_t *spec, void* obj, char *val);
typedef char*    (*str_get_fun_t)(prop_t *spec, void* obj);
typedef props_t* (*rec_get_fun_t)(prop_t *spec, void* obj);

class prop_t : public obj_t {
 public:
  union {
    int_get_fun_t i;
    flo_get_fun_t f;
    str_get_fun_t s;
    rec_get_fun_t r;
    vec_get_fun_t v;
  } get;
  union {
    int_set_fun_t i;
    flo_set_fun_t f;
    str_set_fun_t s;
    vec_set_fun_t v;
    void*         p;
  } set;
  char *name;
  // char *value;
  float delta;
  float min;
  float max;
  void *spec;
  bool is_min_max(void) { return max >= min; }
  virtual void   copy (void* dst, void* src);
  virtual int    scan (char *str, void* obj = NULL) { return 0; }
  virtual int    scan (obj_t *val, void* obj = NULL) { return 0; }
  virtual std::string post (std::string& str, void* obj = NULL) { return 0; }
  virtual int    incr (void* obj = NULL) { return 0; }
  virtual int    decr (void* obj = NULL) { return 0; }
};

class int_prop_t : public prop_t {
 public:
  void   copy (void* dst, void* src);
  int    scan (char* str, void* obj = NULL);
  int    scan (obj_t* val, void* obj = NULL);
  std::string post (std::string& str, void* obj = NULL);
  int    incr (void* obj = NULL);
  int    decr (void* obj = NULL);
  int_prop_t (void);
  int_prop_t (char *name_, int *var, int delta_, int min_ = 1, int max_ = -1);
  int_prop_t (char *name_, int *var, int_set_fun_t set_, int delta_, int min_ = 1, int max_ = -1);
  int_prop_t (char *name_, int_get_fun_t get_, int_set_fun_t set_, int delta_, int min_ = 1, int max_ = -1);
};

class chr_prop_t : public int_prop_t {
 public:
  int    scan (char *str, void* obj = NULL);
  int    scan (obj_t* val, void* obj = NULL);
  std::string post (std::string& str, void* obj = NULL);
  int    incr (void* obj = NULL);
  int    decr (void* obj = NULL);
  chr_prop_t (char *name_, char* var, int delta_, int min_ = 1, int max_ = -1);
  chr_prop_t (char *name_, char* var, int_set_fun_t set_, int delta_, int min_ = 1, int max_ = -1);
  chr_prop_t (char *name_, int_get_fun_t get_, int_set_fun_t set_, int delta_, int min_ = 1, int max_ = -1);
};

class flo_prop_t : public prop_t {
 public:
  void   copy (void* dst, void* src);
  int    scan (char *str, void* obj = NULL);
  int    scan (obj_t* val, void* obj = NULL);
  std::string post (std::string& str, void* obj = NULL);
  int    incr (void* obj = NULL);
  int    decr (void* obj = NULL);
  flo_prop_t (char *name_, flo *var, flo delta_, flo min_ = 1, flo max_ = -1);
  flo_prop_t (char *name_, flo *var, flo_set_fun_t set_, flo delta_, flo min_ = 1, flo max_ = -1);
  flo_prop_t (char *name_, flo_get_fun_t get_, flo_set_fun_t set_, flo delta_, flo min_ = 1, flo max_ = -1);
};

class vec_prop_t : public prop_t {
 public:
  void   copy (void* dst, void* src);
  int    scan (char *str, void* obj = NULL);
  int    scan (obj_t* val, void* obj = NULL);
  std::string post (std::string& str, void* obj = NULL);
  int    incr (void* obj = NULL);
  int    decr (void* obj = NULL);
  vec_prop_t (char *name_, vec_get_fun_t get, vec_set_fun_t set);
  vec_prop_t (char *name_, vec_t<3> *var);
};

class str_prop_t : public prop_t {
 public:
  void   copy (void* dst, void* src);
  int    scan (char *str, void* obj = NULL);
  int    scan (obj_t* val, void* obj = NULL);
  std::string post (std::string& str, void* obj = NULL);
  str_prop_t (void);
  str_prop_t (char *name_, char** var);
  str_prop_t (char *name_, std::string* var);
  str_prop_t (char *name_, char** var, str_set_fun_t set_);
  str_prop_t (char *name_, str_get_fun_t get_, str_set_fun_t set_);
};

class enum_prop_t : public str_prop_t {
 public:
  std::vector<char*> *choices;
  int  incr (void* obj = NULL);
  int  decr (void* obj = NULL);
  int str_to_enum (char* choice);
  char* enum_to_str (int choice);
  enum_prop_t (void);
  enum_prop_t (char *name_, int* var, std::vector<char*> *choices_);
  enum_prop_t (char *name_, int* var, str_set_fun_t set_, std::vector<char*> *choices_);
  enum_prop_t (char *name_, str_get_fun_t get_, str_set_fun_t set_, std::vector<char*> *choices_);
};

extern std::vector<char*>* new_choices(char *str, ...);

/// BOOL PROP

char* bool_to_enum (int val);
int  enum_to_bool (char *val);

extern std::vector<char*>* bool_choices;

class bool_prop_t : public enum_prop_t {
 public:
  bool_prop_t (char *name_, int* var);
  bool_prop_t (char *name_, int* var, str_set_fun_t set_);
  bool_prop_t (char *name_, str_get_fun_t get_, str_set_fun_t set_);
};

/// FILENAME PROP

class filename_prop_t : public str_prop_t {
 public:
  char* pathname;
  char* suffix;
  filename_prop_t (char *name_, str_get_fun_t get, str_set_fun_t set, char* pathname_, char* suffix_);
  // filename_prop_t (char *name_, char** var, char* pathname_, char* suffix_);
};

/// REC PROP

class rec_prop_t : public prop_t {
 public:
  void  copy (void* dst, void* src);
  props_t*  props;
  std::string post (std::string& str, void* obj = NULL);
  rec_prop_t (char *name_, rec_get_fun_t get_, props_t *props_);
};

#define defnumprop(type,name,init,inc) \
  type name = init; \
  type ## _prop_t* name ## _var = new type ## _prop_t (#name, & name, inc)

#define defnumpropmod(type,name,init,inc,min,max)  \
  type name = init; \
  type ## _prop_t* name ## _var = new type ## _prop_t (#name, & name, inc, min, max)

#define defenumprop(name,choices) \
  int name = 0; \
  enum_prop_t* name ## _var = new enum_prop_t (#name, & name, choices)

#define defboolprop(name,init) \
  int name = init; \
  bool_prop_t* name ## _var = new bool_prop_t (#name, & name)

#define defstrprop(name,init) \
  std::string name = init;                                       \
  str_prop_t* name ## _var = new str_prop_t (#name, & name)


#define defaccessors(klass, tipe, name)               \
  tipe get_ ## name ( prop_t *spec, void* obj_ ) { klass* obj = (klass*)obj_; return obj->name; } \
  int  set_ ## name ( prop_t *spec, void* obj_, tipe val ) { klass* obj = (klass*)obj_; obj->name = val; return 1; } 

#define defboolaccessors(klass, name)               \
  char* get_ ## name ( prop_t *spec, void* obj_ ) { klass* obj = (klass*)obj_; return bool_to_enum(obj->name); } \
  int set_ ## name ( prop_t *spec, void* obj_, char* val ) { klass* obj = (klass*)obj_; obj->name = enum_to_bool(val); return 1; } 

#define defnumfield(klass,type,name,init,inc)   \
  defaccessors(klass, type, name) \
  type ## _prop_t* name ## _var = new type ## _prop_t (#name, get_ ## name, set_ ## name, inc)

#define defnumfieldmod(klass,type,name,init,inc,min,max)        \
  defaccessors(klass, type, name) \
  type ## _prop_t* name ## _var = new type ## _prop_t (#name, get_ ## name, set_ ## name, inc, min, max)

// TODO: HOW TO HOLD ONTO CHOICES

#define defenumfield(klass, name, choices)                                \
  defaccessors(klass, char*, name)                                         \
  enum_prop_t* name ## _var = new enum_prop_t (#name, get_ ## name, set_ ## name, choices)

#define defboolfield(klass, name, init)                                   \
  defboolaccessors(klass, name) \
  bool_prop_t* name ## _var = new bool_prop_t (#name, get_ ## name, set_ ## name)

#define defstrfield(klass, name, init)                                    \
  defaccessors(klass, char*, name) \
  str_prop_t* name ## _var = new str_prop_t (#name, get_ ## name, set_ ## name)

// #define deffilenameprop(name,pathname,suffix) \
//   char* name = init;                                                  \
//   filename_prop_t* name ## _var = new filename_prop_t (#name, & name, pathname, suffix)

// defprop(int, n_beats, 0)

// typedef prop Type;

class props_t {
 public:
  std::vector<prop_t*> *props;
  props_t(void);
  props_t(std::vector<prop_t*> *props_);
  ~props_t(void);
  void install (prop_t* prop);
  int parse_arg (std::vector<const char*>::iterator& ap, void* obj);
  int parse_arg (obj_t *sym, obj_t *val, void* obj);
  int parse_args (std::vector<const char*>::iterator& ap, std::vector<const char*>::iterator eap, void* obj = NULL);
  int parse_args (obj_t *args, void* obj = NULL);
};

extern obj_t* prop_class;
extern obj_t* int_prop_class;
extern obj_t* chr_prop_class;
extern obj_t* flo_prop_class;
extern obj_t* str_prop_class;
extern obj_t* enum_prop_class;
extern obj_t* bool_prop_class;
extern obj_t* filename_prop_class;
extern obj_t* rec_prop_class;

extern void init_props (void);

#endif
