//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "utils.h"
#include "props.h"

using namespace std;

obj_t* prop_class;

void prop_t::copy (void* dst, void* src) {
  std::string str;
  post(str, src);
  scan((char*)(str.c_str()), dst);
}

/// INT PROP

obj_t* int_prop_class;

static void init_int_prop 
(prop_t *prop, obj_t* class_, char *name_, int_get_fun_t get_, int_set_fun_t set_, 
 void* spec_, int delta_, int min_, int max_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->delta = delta_;
  prop->min   = min_;
  prop->max   = max_;
  prop->get.i = get_;
  prop->set.i = set_;
  prop->spec  = spec_;
}

int int_var_get (prop_t* prop, void* obj) {
  int* var = (int*)(prop->spec);
  return *var;
}

int int_var_set (prop_t* prop, void* obj, int val) {
  int* var = (int*)(prop->spec);
  *var = val;
  return 1;
}

int_prop_t::int_prop_t (void) { }

int_prop_t::int_prop_t (char *name_, int* var, int delta_, int min_, int max_) {
  init_int_prop
    (this, int_prop_class, name_, &int_var_get, &int_var_set, (void*)var, delta_, min_, max_);
}

int_prop_t::int_prop_t (char *name_, int* var, int_set_fun_t set_, int delta_, int min_, int max_) {
  init_int_prop
    (this, int_prop_class, name_, &int_var_get, set_, (void*)var, delta_, min_, max_);
}

int_prop_t::int_prop_t (char *name_, int_get_fun_t get_, int_set_fun_t set_, int delta_, int min_, int max_) {
  init_int_prop
    (this, int_prop_class, name_, get_, set_, NULL, delta_, min_, max_);
}

void int_prop_t::copy (void* dst, void* src) {
  set.i(this, dst, get.i(this, src));
}

int int_prop_t::scan (char *str, void* obj) {
  int v; sscanf(str, "%d", &v); 
  return set.i(this, obj, v);
}

int int_prop_t::scan (obj_t *val, void* obj) {
  return set.i(this, obj, num_val(val));
}

string int_prop_t::post (string& str, void* obj) {
  int val = get.i(this, obj);
  stringstream ss;
  ss << val;
  str.assign(ss.str());
  return str;
}

int int_prop_t::incr (void* obj) {
  int val = get.i(this, obj);
  int rng = max-min+1;
  int nxt = is_min_max() ? (((int)(val-min+delta))%rng)+min : val+delta;
  printf("INCR %d/%f\n", nxt, max);
  return set.i(this, obj, nxt);
}

int int_prop_t::decr (void* obj) {
  int val = get.i(this, obj);
  int rng = max-min+1;
  int nxt = is_min_max() ? (((int)(val-min+delta+rng))%rng)+min : val+delta;
  return set.i(this, obj, val-delta);
}

/// CHR MODEL

obj_t* vec_prop_class;

static void init_vec_prop 
    (prop_t *prop, obj_t* class_, 
     char *name_, vec_get_fun_t get_, vec_set_fun_t set_, void* spec_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->get.v = get_;
  prop->set.v = set_;
  prop->spec  = spec_;
}

vec_prop_t::vec_prop_t (char *name, vec_get_fun_t get, vec_set_fun_t set) {
  init_vec_prop(this, vec_prop_class, name, get, set, NULL);
}

vec3d_t vec_var_get (prop_t* prop, void* obj) {
  vec3d_t* var = (vec3d_t*)(prop->spec);
  return *var;
}

int vec_var_set (prop_t* prop, void* obj, vec3d_t val) {
  vec3d_t* var = (vec3d_t*)(prop->spec);
  *var = val;
  return 1;
}

vec_prop_t::vec_prop_t (char *name, vec3d_t *var) {
  init_vec_prop(this, vec_prop_class, name, &vec_var_get, &vec_var_set, (void*)var);
}

void vec_prop_t::copy (void* dst, void* src) {
  set.v(this, dst, get.v(this, src));
}

int vec_prop_t::scan (char *str, void* obj) {
  vec3d_t res;
  int n = 0;
  for (int i = 0; i < strlen(str); i++)
    n += ( str[i] == ',');
  res.z = 0;
  if (n == 1)
    sscanf(str, "%lf,%lf", &res.x, &res.y); 
  else
    sscanf(str, "%lf,%lf,%lf", &res.x, &res.y, &res.z); 
  return set.v(this, obj, res);
}

int vec_prop_t::scan (obj_t *val, void* obj) {
  return set.v(this, obj, vec_val(val));
}

string vec_prop_t::post (string& str, void* obj_) {
  obj_t *obj = (obj_t*)obj_;
  vec3d_t val = get.v(this, obj);
  stringstream ss;
  obj->blat_into(ss);
  str.assign(ss.str());
  return str;
}

int vec_prop_t::incr (void* obj) { 
  return 0;
}

int vec_prop_t::decr (void* obj) { 
  return 0;
}

/// CHR MODEL

obj_t* chr_prop_class;

chr_prop_t::chr_prop_t (char *name_, int_get_fun_t get, int_set_fun_t set, int delta_, int min_, int max_) {
  init_int_prop(this, chr_prop_class, name_, get, set, NULL, delta_, min_, max_);
}

int chr_prop_t::scan (char *str, void* obj) {
  char c; sscanf(str, "%c", &c); 
  return set.i(this, obj, c);
}

int chr_prop_t::scan (obj_t *val, void* obj) {
  return set.i(this, obj, str_name(val)[0]);
}

string chr_prop_t::post (string& str, void* obj) {
  char val = get.i(this, obj);
  stringstream ss;
  ss << val;
  str.assign(ss.str());
  return str;
}

int chr_prop_t::incr (void* obj) { 
  return 0;
}

int chr_prop_t::decr (void* obj) { 
  return 0;
}

/// FLO MODEL

obj_t* flo_prop_class;

static void init_flo_prop 
    (prop_t *prop, obj_t* class_, 
     char *name_, flo_get_fun_t get_, flo_set_fun_t set_, void* spec_, flo delta_, flo min_, flo max_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->delta = delta_;
  prop->min   = min_;
  prop->max   = max_;
  prop->get.f = get_;
  prop->set.f = set_;
  prop->spec  = spec_;
}

flo_prop_t::flo_prop_t 
    (char *name_, flo_get_fun_t get_, flo_set_fun_t set_, flo delta_, flo min_, flo max_) {
  init_flo_prop(this, flo_prop_class, name_, get_, set_, NULL, delta_, min_, max_);
}

flo flo_var_get (prop_t* prop, void* obj) {
  flo* var = (flo*)(prop->spec);
  return *var;
}

int flo_var_set (prop_t* prop, void* obj, flo val) {
  flo* var = (flo*)(prop->spec);
  *var = val;
  return 1;
}

flo_prop_t::flo_prop_t (char *name_, flo* var, flo delta_, flo min_, flo max_) {
  init_flo_prop
    (this, flo_prop_class, name_, &flo_var_get, &flo_var_set, (void*)var, delta_, min_, max_);
}

flo_prop_t::flo_prop_t (char *name_, flo* var, flo_set_fun_t set_, flo delta_, flo min_, flo max_) {
  init_flo_prop
    (this, flo_prop_class, name_, &flo_var_get, set_, (void*)var, delta_, min_, max_);
}

void flo_prop_t::copy (void* dst, void* src) {
  set.f(this, dst, get.f(this, src));
}

int flo_prop_t::scan (char *str, void* obj) {
  float f; sscanf(str, "%f", &f); 
  return set.f(this, obj, f);
}

int flo_prop_t::scan (obj_t *val, void* obj) {
  return set.i(this, obj, num_val(val));
}

string flo_prop_t::post (string& str, void* obj) {
  float fval = get.f(this, obj);
  int   ival = (int)fval;
  stringstream ss;
  if (ival == fval)
    ss << ival;
  else
    ss << fval;
  str.assign(ss.str());
  return str;
}

int flo_prop_t::incr (void* obj) { 
  flo val = get.f(this, obj);
  flo nxt = is_min_max() ? fmod(((val-min)+delta), max-min)+min : val+delta;
  return set.f(this, obj, nxt);
}

int flo_prop_t::decr (void* obj) { 
  flo val = get.f(this, obj);
  flo nxt = is_min_max() ? fmod(((val-min)-delta+max-min), max-min)+min : val-delta;
  return set.f(this, obj, nxt);
}

/// STR MODEL

obj_t* str_prop_class;

static void init_str_prop 
    (prop_t *prop, obj_t* class_, 
     char *name_, str_get_fun_t get_, str_set_fun_t set_, void* spec_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->get.s = get_;
  prop->set.s = set_;
  prop->spec  = spec_;
}

str_prop_t::str_prop_t (void) { }

str_prop_t::str_prop_t (char *name_, str_get_fun_t get_, str_set_fun_t set_) {
  init_str_prop(this, str_prop_class, name_, get_, set_, NULL);
}

std::string str_var_get (prop_t* prop, void* obj) {
  std::string* var = (std::string*)(prop->spec);
  return *var;
}

int str_var_set (prop_t* prop, void* obj, std::string val) {
  std::string* var = (std::string*)(prop->spec);
  *var = val;
  return 1;
}

str_prop_t::str_prop_t (char *name_, char** var) {
  init_str_prop(this, str_prop_class, name_, &str_var_get, &str_var_set, (void*)var);
}

str_prop_t::str_prop_t (char *name_, char** var, str_set_fun_t set_) {
  init_str_prop(this, str_prop_class, name_, &str_var_get, set_, (void*)var);
}

string string_var_get (prop_t* prop, void* obj) {
  string* var = (string*)(prop->spec);
  return (var->c_str());
}

int string_var_set (prop_t* prop, void* obj, std::string val) {
  string* var = (string*)(prop->spec);
  var->assign(val);
  return 1;
}

str_prop_t::str_prop_t (char *name_, string* var) {
  init_str_prop(this, str_prop_class, name_, &string_var_get, &string_var_set, (void*)var);
}

void str_prop_t::copy (void* dst, void* src) {
  set.s(this, dst, get.s(this, src));
}

int str_prop_t::scan (char *str, void* obj) {
  return set.s(this, obj, str);
}

int str_prop_t::scan (obj_t *val, void* obj) {
  return set.s(this, obj, str_name(val));
}

string str_prop_t::post (string& str, void* obj) {
  str.assign(get.s(this, obj));
  return str;
}

/// ENUM MODEL

obj_t* enum_prop_class;

enum_prop_t::enum_prop_t (void) {
}

static void init_enum_prop 
    (enum_prop_t* prop, obj_t* class_, 
     char* name_, str_get_fun_t get_, str_set_fun_t set_, vector< string > choices_, void* spec_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->get.s = get_;
  prop->set.s = set_;
  prop->choices = choices_;
  prop->spec  = spec_;
}

enum_prop_t::enum_prop_t 
    (char *name_, str_get_fun_t get_, str_set_fun_t set_, vector< string > choices_) {
  init_enum_prop(this, enum_prop_class, name_, get_, set_, choices_, NULL);
}

int enum_prop_t::str_to_enum (string choice) {
  int i = 0;
  for (i = 0; i < choices.size(); i++) {
    if (choices[i] == choice) {
      return i;
    }
  }
  return 0;
}

string enum_prop_t::enum_to_str (int choice) {
  return choices[choice];
}

string enum_var_get (prop_t* prop_, void* obj) {
  enum_prop_t* prop = (enum_prop_t*)prop_;
  int* var = (int*)(prop->spec);
  string res = prop->enum_to_str(*var);
  return res;
}

int enum_var_set (prop_t* prop_, void* obj, string s) {
  enum_prop_t* prop = (enum_prop_t*)prop_;
  int* var = (int*)(prop->spec);
  *var = prop->str_to_enum(s);
  return 1;
}

enum_prop_t::enum_prop_t (char *name_, int* var, vector< string > choices_) {
  init_enum_prop(this, enum_prop_class, name_, &enum_var_get, &enum_var_set, choices_, (void*)var);
}

enum_prop_t::enum_prop_t (char *name_, int* var, str_set_fun_t set_, vector< string > choices_) {
  init_enum_prop(this, enum_prop_class, name_, &enum_var_get, set_, choices_, (void*)var);
}

int enum_prop_t::incr (void* obj) {
  // TODO: FIX
  string choice = get.s(this, obj);
  int n = choices.size();
  int i = str_to_enum(choice);
  char* ns = (char*)choices[(i+1)%n].c_str();
  return set.s(this, obj, ns); 
}

int enum_prop_t::decr (void* obj) {
  // TODO: FIX
  string choice(get.s(this, obj));
  int n = choices.size();
  int i = str_to_enum(choice);
  return set.s(this, obj, (char*)choices[(i-1+n)%n].c_str()); 
}

string bool_to_enum (int val) {
  return (val != 0 ? "true" : "false");
}

int enum_to_bool (string val) {
  if (val == "true")
    return 1;
  else if (val == "false")
    return 0;
  else
    return 0;
}

bool is_bool_choices = false;
vector<string> bool_choices;

std::vector< std::string > new_choices(std::string a, std::string b) {
  std::vector< std::string > choices;
  choices.push_back(a);
  choices.push_back(b);
  return choices;
}

std::vector< std::string > new_choices(std::string a, std::string b, std::string c) {
  std::vector< std::string > choices;
  choices.push_back(a);
  choices.push_back(b);
  choices.push_back(c);
  return choices;
}

std::vector< std::string > new_choices(std::string a, std::string b, std::string c, std::string d) {
  std::vector< std::string > choices;
  choices.push_back(a);
  choices.push_back(b);
  choices.push_back(c);
  choices.push_back(d);
  return choices;
}

/// BOOL PROP

obj_t* bool_prop_class;

static void lazy_bool_choices (void) {
  if (!is_bool_choices) {
    is_bool_choices = true;
    bool_choices = new_choices("false", "true");
  }
}

bool_prop_t::bool_prop_t (char *name_, str_get_fun_t get_, str_set_fun_t set_) {
  lazy_bool_choices();
  init_enum_prop(this, bool_prop_class, name_, get_, set_, bool_choices, NULL);
}

bool_prop_t::bool_prop_t (char *name_, int* var, str_set_fun_t set_) {
  lazy_bool_choices();
  init_enum_prop(this, bool_prop_class, name_, &enum_var_get, set_, bool_choices, (void*)var);
}

bool_prop_t::bool_prop_t (char *name_, int* var) {
  lazy_bool_choices();
  init_enum_prop(this, bool_prop_class, name_, &enum_var_get, &enum_var_set, bool_choices, (void*)var);
}


/// FILENAME MODEL

obj_t* filename_prop_class;

static void init_filename_prop 
    (filename_prop_t* prop, obj_t* class_,
     char* name_, str_get_fun_t get_, str_set_fun_t set_, char* pathname_, char* suffix_, void* spec_) {
  prop->type  = class_;
  prop->name  = name_;
  prop->get.s = get_;
  prop->set.s = set_;
  prop->pathname = pathname_;
  prop->suffix = suffix_;
  prop->spec  = spec_;
}

filename_prop_t::filename_prop_t 
    (char *name_, str_get_fun_t get_, str_set_fun_t set_, char* pathname_, char* suffix_) {
  init_filename_prop(this, filename_prop_class, name_, get_, set_, pathname_, suffix_, NULL);
}

/// REC MODEL

obj_t* rec_prop_class;

rec_prop_t::rec_prop_t (char *name_, rec_get_fun_t get_, props_t *props_) {
  type     = rec_prop_class;
  name     = name_;
  get.r    = get_;
  set.p    = NULL;
  props    = props_;
}

void rec_prop_t::copy (void* dst, void* src) {
  for (int i = 0; i < props->props->size(); i++)
    props->props->at(i)->copy(dst, src);
}

string rec_prop_t::post (string& str, void* obj) {
  props_t* props = get.r(this, obj);
  // post("Rec Props n_props %d\n", props->n_props); 
  // if (props->n_props != 0)
  // TODO:
  // props->props->at(props->primary_index)->post(str);
  str.assign("...");
  return str;
}

/// ARGS PROCESSING

props_t::props_t(void) {
  props = new vector<prop_t*>();
}

props_t::~props_t (void) {
  delete props;
}

props_t::props_t (vector<prop_t*> *props_) {
  props = props_;
}

void props_t::install (prop_t* prop) {
  props->push_back(prop);
}

int props_t::parse_arg (vector<const char*>::iterator& ap, void* obj) {
  const char *in_name = *ap;
  char name[256];
  for (int i = 0; i < strlen(in_name); i++)
    name[i] = in_name[i] == '-' ? '_' : in_name[i];
  name[strlen(in_name)] = 0;
  if (name[0] == ':') {
    for (int i = 0; i < props->size(); i++) {
      prop_t *prop = props->at(i);
      // fprintf(stderr, "LOOKING AT %s\n", prop->name);
      if (strcasecmp(&name[1], prop->name) == 0) {
        ap += 1;
        const char *arg = *ap++;
        // fprintf(stderr, "  SCANNING %s\n", prop->name);
        if (prop->scan((char*)arg, obj) == 0)
          uerror("Unable to process property %s on %s", name, arg);
        return 1;
      }
    }
  } 
  ap += 1;
  // post("Unable to find keyword arg %s\n", name);
  return 0;
}

int props_t::parse_args (vector<const char*>::iterator& ap, vector<const char*>::iterator eap, void* obj) {
  int count = 0;
  // post("PARSING ARGS\n");
  while (ap != eap) {
    count += parse_arg(ap, obj);
  }
  return count;
}

int props_t::parse_args (int argc, const char *argv[], void *obj) {
  std::vector<const char*> args;
  for (int i = 1; i < argc; i++) 
    args.push_back(argv[i]);
  std::vector<const char*>::iterator ap  = args.begin();
  std::vector<const char*>::iterator eap = args.end();
  return parse_args(ap, eap, obj);
}

int props_t::parse_arg (obj_t *sym, obj_t *val, void* obj) {
  char *name = &sym_name(sym)[1];
  if (name[0] == ':') {
    for (int i = 0; i < props->size(); i++) {
      prop_t *prop = props->at(i);
      if (strcasecmp(&name[1], prop->name) == 0) {
        post("SCANNING %s\n", prop->name);
        if (prop->scan(val, obj) == 0)
          uerror("Unable to process property %s on %s", name, val);
        return 1;
      }
    }
  }
  post("Unable to find keyword arg %s\n", name);
  return 0;
}

int props_t::parse_args (obj_t *args, void* obj) {
  int count = 0;
  for (obj_t* a = args; a != lisp_nil; a = tail(tail(a))) {
    if (list_len(a) > 1) {
      count += parse_arg(list_elt(a, 0), list_elt(a, 1), obj);
    } else
      break;
  }
  return count;
}

void init_props (void) {
  lazy_bool_choices();
  prop_class = new_class("PROP", any_class);
  int_prop_class = new_class("INT-PROP", prop_class);
  chr_prop_class = new_class("CHR-PROP", prop_class);
  flo_prop_class = new_class("FLO-PROP", prop_class);
  str_prop_class = new_class("STR-PROP", prop_class);
  vec_prop_class = new_class("VEC-PROP", prop_class);
  enum_prop_class = new_class("ENUM-PROP", prop_class);
  bool_prop_class = new_class("BOOL-PROP", prop_class);
  filename_prop_class = new_class("FILENAME-PROP", prop_class);
  rec_prop_class = new_class("REC-PROP", prop_class);
}
