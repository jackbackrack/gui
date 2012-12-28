//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef IS_LISP
#define IS_LISP

#include "utils.h"
#include <vector>
#include <string>
#include <sstream>
#include "vec.h"

class obj_t;
extern obj_t* lisp_nil; 
extern obj_t* lisp_true; 
extern obj_t* lisp_false; 

typedef std::vector<obj_t*> env_t;

extern obj_t* env_lookup (env_t& env, obj_t* name);
extern obj_t* env_lookup (env_t& env, char* name);
extern void env_add (env_t& env, obj_t* name, obj_t* val);
extern void env_add (env_t& env, char* name, obj_t* val);

class obj_t {
 public:
  obj_t* type;
  std::string blat (std::string& str);
  virtual obj_t* eval(env_t& env) { return lisp_nil; }
  virtual void blat_into(std::stringstream& ss) { }
};

inline obj_t* obj_class (obj_t* obj) { return obj->type; }

extern void check_type (obj_t* e, obj_t* c);
extern void check_isa (obj_t* e, obj_t* c);

class class_t : public obj_t {
 public:
  char* name;
  obj_t* super;
  class_t (char* name, obj_t* super);
  obj_t* eval(env_t& env);
  void blat_into(std::stringstream& ss);
};

inline obj_t* new_class (char* name, obj_t* super) { 
  return (obj_t*)new class_t(name, super); 
}

inline char* class_name (obj_t* obj) { return ((class_t*)obj)->name; }
inline obj_t* class_super (obj_t* obj) { return ((class_t*)obj)->super; }

extern obj_t* class_class;

class list_t : public obj_t {
 public:
  obj_t* car;
  obj_t* cdr;
  obj_t* eval(env_t& env);
  list_t (obj_t* head_, obj_t* tail_);
  ~list_t (void);
  void blat_into (std::stringstream& ss);
};

inline obj_t* cons (obj_t* head, obj_t* tail) { return new list_t(head, tail); }
inline obj_t* head (obj_t* o) { return ((list_t*)o)->car; }
inline obj_t* tail (obj_t* o) { return ((list_t*)o)->cdr; }
inline obj_t* head_set (obj_t* o, obj_t* v) { return ((list_t*)o)->car = v; }
inline obj_t* tail_set (obj_t* o, obj_t* v) { return ((list_t*)o)->cdr = v; }

extern obj_t* list_rev (obj_t* list);
extern obj_t* list (obj_t* head, ...);
extern int    list_len (obj_t* e);
extern obj_t* list_elt (obj_t* e, int offset);
extern obj_t* list_elt_set (obj_t* e, int offset, obj_t* val);
extern obj_t* list_last (obj_t* e);
inline obj_t* nth (obj_t* e, int offset) { return list_elt(e, offset); }
inline obj_t* nth_set (obj_t* e, int offset, obj_t *val) { return list_elt_set(e, offset, val); }
extern obj_t* list_copy (obj_t* x);

extern obj_t* list_class;
extern obj_t* lisp_nil;

class sym_t : public obj_t {
 public:
  char* name;
  sym_t (char* name_);
  obj_t* eval (env_t& env);
  void blat_into (std::stringstream& ss);
};

extern obj_t* new_sym (char* name);

inline char* sym_name (obj_t* obj) { return ((sym_t*)obj)->name; }

extern obj_t *gen_sym (char *name);

extern obj_t* sym_class;

class str_t : public obj_t {
 public:
  char* name;
  str_t (char* name_);
  obj_t* eval (env_t& env);
  void blat_into (std::stringstream& ss);
};

inline obj_t* new_str (char* name) { return new str_t(name); }

inline char* str_name (obj_t* obj) { return ((str_t*)obj)->name; }

extern obj_t* str_class;

extern obj_t* empty_str;

class num_t : public obj_t {
 public:
  float val;
  num_t (float val);
  obj_t* eval(env_t& env);
  void blat_into(std::stringstream& ss);
};

inline obj_t* new_num (float val) { return new num_t(val); }

inline float num_val (obj_t* obj) { return ((num_t*)obj)->val; }
inline float num_val_set (obj_t* obj, flo num) { return ((num_t*)obj)->val = num; }

extern obj_t* num_class;

class vec3_t : public obj_t {
 public:
  vec_t<3> val;
  vec3_t (vec_t<3> val);
  obj_t* eval(env_t& env);
  void blat_into(std::stringstream& ss);
};

inline obj_t* new_vec3 (vec_t<3> val) { return new vec3_t(val); }

inline vec_t<3> vec_val (obj_t* obj) { return ((vec3_t*)obj)->val; }
inline vec_t<3> vec_val_set (obj_t* obj, vec_t<3> vec) { return ((vec3_t*)obj)->val = vec; }

extern obj_t* vec3_class;

class bool_t : public obj_t {
 public:
  bool val;
  bool_t (bool val);
  obj_t* eval(env_t& env);
  void blat_into(std::stringstream& ss);
};

inline obj_t* new_bool (float val) { return new bool_t(val); }

inline bool bool_val (obj_t* obj) { return ((bool_t*)obj)->val; }

extern obj_t* bool_class;

/*
class param_t {
 public:
  char*    name;
  int      is_keyword;
  class_t* type;
};

class fun_type_t {
 public:
  class_t* result_type;
  bool is_nary;
  std::vector<param_t*> params;
  fun_type_t(class_t* result_type, std::vector<param_t*> params, bool is_nary);
};

class fun_t : public obj_t {
 public:
  char* name;
  fun_type_t* type;
  eval_fun_t* run;
  fun_t (char* name_, fun_type_t* type);
  obj_t* eval (env_t& env);
  void blat_into (std::stringstream& ss);
};
*/

extern obj_t* fun_class;

typedef obj_t* (*eval_fun_t)(env_t& env, obj_t* args);

std::vector< char* > params (char* arg, ...);

class fun_t : public obj_t {
 public:
  char* name;
  int n_required;
  int is_nary;
  std::vector< obj_t* > params;
  eval_fun_t exec;
  fun_t (char* name_, eval_fun_t exec_, std::vector< char* > params_, int n_required_, int is_nary_ = 0);
  // fun_t (char* name_, props_t* props);
  obj_t* apply (env_t& env, obj_t* args);
  obj_t* eval (env_t& env);
  void blat_into (std::stringstream& ss);
};

extern void format_object (std::string& buf, obj_t *x);
extern void print_object (obj_t *x);
extern void post_object (char *msg, obj_t *x);
extern void post_form (obj_t *x);

extern obj_t* quote_sym;

extern obj_t* any_class;

extern void init_lisp (void);

extern void post_form (obj_t *x);

extern void print_object (obj_t *x);

#endif
