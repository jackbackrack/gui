//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "lisp.h"
using namespace std;

/// ENV

void env_add (env_t& env, obj_t* name, obj_t* val) {
  // post("ENV ADD %p %p\n", name, val);
  env.push_back(name);
  env.push_back(val);
}

void env_add (env_t& env, char* name, obj_t* val) {
  return env_add(env, new_sym(name), val);
}

obj_t* env_lookup (env_t& env, obj_t* name) {
  for (int i = env.size()/2 - 1; i >= 0; i--) {
    obj_t* sname = env[2*i];
    // post("NAME1 %s NAME2 %s\n", sym_name(name), sym_name(sname));
    if (name == sname) 
      return env[2*i+1];
  }
  uerror("Unable to find binding %s\n", sym_name(name));
  return NULL;
}

obj_t* env_lookup (env_t& env, char* name) {
  for (int i = env.size()/2 - 1; i >= 0; i--) {
    obj_t* sname = env[2*i];
    // post("NAME1 %s NAME2 %s\n", name, sym_name(sname));
    if (strcmp(name, sym_name(sname)) == 0) 
      return env[2*i+1];
  }
  uerror("Unable to find binding %s\n", name);
  return NULL;
}

/// OBJ

string obj_t::blat(string& str) {
  stringstream ss;
  blat_into(ss);
  str.assign(ss.str());
  return str;
}
  
/// CLASS

obj_t* class_class;
obj_t* any_class;

class_t::class_t (char* name_, obj_t* super_) { 
  type = class_class; 
  name  = name_; 
  super = super_;
}

obj_t* class_t::eval(env_t& env) {
  return this;
}

void class_t::blat_into(stringstream& ss) {
  ss << "[CLASS" << name << "]";
}

bool is_subclass (obj_t* sc, obj_t* c) {
  if (c == any_class)
    return true;
  for (;;) {
    // post("CHECKING %s == %s\n", sc->name, c->name);
    if (sc == c)
      return true;
    else if (sc == any_class)
      return false;
    sc = class_super(sc);
  }
}

bool isa (obj_t* x, obj_t* c) {
  return is_subclass(obj_class(x), c);
}

void check_type (obj_t* e, obj_t* c) {
  void **x = NULL;
  if (obj_class(e) != c) {
    post("TYPE CHECK FAILURE %s SHOULD BE %s\n", 
         class_name(obj_class(e)), class_name(c));
    *x = (void*)1;
  }
}

void check_isa (obj_t* e, obj_t* c) {
  void **x = NULL;
  if (!isa(e, c)) {
    post("TYPE CHECK FAILURE %s SHOULD BE %s\n", 
         class_name(obj_class(e)), class_name(c));
    *x = (void*)1;
  }
}

/// LIST

obj_t* list_class;

void format_list (std::string& buf, obj_t *object) {
  obj_t* list = object;
  int    is_first = 1;
  stringstream ss;
  buf.append("(");
  for (;;) {
    if (list == lisp_nil)
      break;
    else {
      if (is_first)
	is_first = 0;
      else
	buf.append(" ");
      format_object(buf, head(list));
      list = tail(list);
    }
  }
  buf.append(")");
}

static obj_t* eval_args(env_t& env, obj_t* args) {
  obj_t* res = lisp_nil;
  while (args != lisp_nil) {
    res = cons(head(args)->eval(env), res);
    args = tail(args);
  }
  return list_rev(res);
}

static obj_t* eval_apply(env_t& env, obj_t* op, obj_t* args) {
  obj_t* f = op->eval(env);
  if (isa(f, fun_class)) {
    fun_t* fun = (fun_t*)f;
    return fun->apply(env, eval_args(env, args));
  } else {
    uerror("Unable to apply %s to args\n", class_name(obj_class(op)));
    return NULL;
  }
}

obj_t* list_t::eval(env_t& env) {
  obj_t* op = car;
  if (isa(op, sym_class)) {
    if (op == quote_sym)
      return list_elt(this, 1);
    else 
      return eval_apply(env, car, cdr);
  } else 
    return eval_apply(env, car, cdr);
}

void list_t::blat_into(stringstream& ss) {
  obj_t* list = this;
  bool is_first = true;
  ss << '(';
  for (obj_t* list = this; list != lisp_nil; list = tail(list)) {
    if (is_first)
      is_first = false;
    else
      ss << ' ';
    head(list)->blat_into(ss);
  }
  ss << ')';
}

list_t::list_t (obj_t* head, obj_t* tail) { 
  type = list_class; 
  car = head; 
  cdr = tail;
}

list_t::~list_t (void) {
  delete car;
  delete cdr;
}

obj_t* list_elt(obj_t* e, int offset) {
  for (int i = 0; i < offset; i++)
    e = tail(e);
  return head(e);
}

obj_t* list_elt_set(obj_t* e, int offset, obj_t* val) {
  for (int i = 0; i < offset; i++)
    e = tail(e);
  return head_set(e, val);
}

int list_len(obj_t* e) {
  for (int i = 0; ; i++) {
    if (e == lisp_nil)
      return i;
    else 
      e = tail(e);
  }
}

obj_t* list_last (obj_t* e) {
  for (; ;) {
    obj_t *nxt = tail(e);
    if (nxt == lisp_nil)
      return head(e);
    else 
      e = nxt;
  }
}

obj_t* list_rev (obj_t* list) {
  obj_t* l = list;
  obj_t* r = lisp_nil;
  for (;;) {
    if (l == lisp_nil) {
      return r;
    } else {
      obj_t* tl = tail(l);
      tail_set(l, r);
      r = l;
      l = tl;
    }
  }
}

obj_t* list (obj_t* head, ...) {
  int i, n;
  va_list ap; 
  obj_t* res;
  va_start(ap, head);
  vector<obj_t*> objs;
  objs.push_back(head);
  for (n = 1; ; n++) {
    obj_t* obj = va_arg(ap, obj_t*);
    if (obj == NULL) break;
    objs.push_back(obj);
  }
  va_end(ap);
  res = lisp_nil;
  for (i = n-1; i >= 0; i--)
    res = cons(objs[i], res);
  return res;
}

obj_t* list_copy (obj_t* x) {
  obj_t* l = x;
  obj_t* r = lisp_nil;
  for (;;) {
    if (l == lisp_nil) {
      return list_rev(r);
    } else {
      r = cons(head(l), r);
    }
  }
}

/// SYM

// #include <ext/hash_map>
#include <unordered_map>
// using namespace __gnu_cxx;

struct sym_cmp {
  bool operator() (const char* a, const char* b) const {
    // post("A %s B %s == %d\n", a, b, strcmp(a, b) == 0);
    return strcmp(a, b) == 0;
  }
};

// hash_map<const char*,obj_t*,hash<const char*>, sym_cmp> sym_tab;
unordered_map<const char*,obj_t*,hash<const char*>, sym_cmp> sym_tab;

obj_t* sym_class;

sym_t::sym_t (char* name_) { 
  type = sym_class; 
  name  = name_; 
}

obj_t* sym_tab_lookup(char* name) {
  // post("LOOK UP %s\n", name);
  unordered_map<const char*,obj_t*,hash<const char*>, sym_cmp>::iterator st_iter;
  st_iter = sym_tab.find(name);
  if (st_iter == sym_tab.end()) 
    return NULL;
  else
    return st_iter->second;
}

void sym_tab_add(obj_t* val) {
  // post("ADD %s\n", sym_name(val));
  sym_tab[sym_name(val)] = val;
}

obj_t* new_sym (char* name) { 
  char up_name[256];
  for (int i = 0; i < strlen(name); i++)
    up_name[i] = toupper(name[i]);
  up_name[strlen(name)] = 0;
  obj_t* val = sym_tab_lookup(up_name);
  // post("SYM LOOKUP %s VAL == %p\n", up_name, val);
  if (val != NULL)
    return val;
  else {
    char* dup_name = (char*)malloc(strlen(up_name)+1);
    strcpy(dup_name, up_name);
    obj_t* sym = new sym_t(dup_name); 
    sym_tab_add(sym);
    // post("INSERTING SYM %s == %p -> %d\n", up_name, sym, sym_tab.size());
    // hash_map<const char*,obj_t*,hash<const char*>, sym_cmp>::iterator st_iter;
    // for ( st_iter = sym_tab.begin( ); st_iter != sym_tab.end( ); st_iter++ )
    //   post("%s ", st_iter->first);
    // post("\n");
    return sym;
  }
}

int sym_num = 0;

obj_t *gen_sym (char *name) {
  char str[100];
  sprintf(str, "%s%d", name, sym_num++);
  return new_sym(str);
}

obj_t* sym_t::eval(env_t& env) {
  return env_lookup(env, this);
}

void sym_t::blat_into(stringstream& ss) {
  ss << name;
}

/// FUN

obj_t* fun_class;

obj_t* fun_t::apply(env_t& env, obj_t* args) {
  env_t new_env = env;
  obj_t* a = args;
  for (int i = 0; i < n_required; i++, a = tail(a)) 
    env_add(new_env, params[i], head(a));
  if (is_nary) 
    env_add(new_env, params[n_required], a);
  eval_fun_t run = exec;
  return run(new_env, args);
}

std::vector< char* > params (char* arg, ...) {
  std::vector< char* > res;
  if (arg != NULL) {
    res.push_back(arg);
    va_list ap; 
    va_start(ap, arg);
    for (int n = 1; ; n++) {
      char* str = va_arg(ap, char*);
      if (str == NULL) break;
      res.push_back(str);
    }
    va_end(ap);
  }
  return res;
}


fun_t::fun_t (char* name_, eval_fun_t exec_, std::vector< char* > params_, int n_required_, int is_nary_) { 
  type = fun_class; 
  name = name_;
  exec = exec_;
  std::vector< obj_t* > res;
  for (int i = 0; i < params_.size(); i++)
    res.push_back(new_sym(params_[i]));
  params = res;
  n_required = n_required_;
  is_nary = is_nary_;
}

obj_t* fun_t::eval(env_t& env) {
  return this;
}

void fun_t::blat_into(stringstream& ss) {
  ss << "(fun ";
  ss << name;
  ss << "(";
  for (int i = 0; i < params.size(); i++) {
    if (i > 0) ss << " ";
    ss << sym_name(params[i]);
  }
  if (is_nary)
    ss << " ...";
  ss << "))";
}

/// STR

obj_t* str_class;

str_t::str_t (char* name_) { 
  type = str_class; 
  name  = name_; 
}

obj_t* str_t::eval(env_t& env) {
  return this;
}

void str_t::blat_into(stringstream& ss) {
  ss << name;
}

/// NUM

obj_t* num_class;

num_t::num_t (float val_) { 
  type = num_class; 
  val  = val_; 
}

obj_t* num_t::eval(env_t& env) {
  return this;
}

void num_t::blat_into(stringstream& ss) {
  ss << val;
}

void format_int (std::string& buf, obj_t *x) {
  check_type(x, num_class);
  stringstream ss;
  ss << num_val(x);
  buf.append(ss.str());
}

/// PIC

obj_t* pic_class;

pic_t::pic_t (picraw_t* val_) { 
  type = pic_class; 
  val  = val_; 
}

obj_t* pic_t::eval(env_t& env) {
  return this;
}

void pic_t::blat_into(stringstream& ss) {
  ss << "pic";
}

/// VEC3

obj_t* vec3_class;

vec3_t::vec3_t (vec_t<3> val_) { 
  type = vec3_class; 
  val  = val_; 
}

obj_t* vec3_t::eval(env_t& env) {
  return this;
}

void vec3_t::blat_into(stringstream& ss) {
  ss << "(vec";
  ss << " ";
  ss << val.x;
  ss << " ";
  ss << val.y;
  ss << " ";
  ss << val.z;
  ss << ")";
}

/// BOOL

obj_t* bool_class;

bool_t::bool_t (bool val_) { 
  type = bool_class; 
  val  = val_; 
}

obj_t* bool_t::eval(env_t& env) {
  return this;
}

void bool_t::blat_into(stringstream& ss) {
  ss << val;
}

obj_t* quote_sym;
obj_t* lisp_nil;
obj_t* lisp_true; 
obj_t* lisp_false; 
obj_t* empty_string; 

// void format_vec (std::string& buf, obj_t *vec) {
//   int is_first = 1;
//   buf.append("[");
//   for (int i = 0; i < vec_len(vec); i++) {
//     if (is_first)
//       is_first = 0;
//     else
//       buf.append(" ");
//     format_object(buf, vec_elt(vec, i));
//   }
//   buf.append("]");
// }

void format_object (std::string& buf, obj_t *x) {
  if (x != NULL) {
    obj_t *klass = obj_class(x);
    // post("%s\n", class->name);
    stringstream ss;

    if (klass == str_class) {
      ss << "|" << str_name(x) << "|";
      buf.append(ss.str());
    } else if (klass == sym_class) {
      ss << str_name(x);
      buf.append(ss.str());
    } else if (klass == list_class) {
      format_list(buf, x);
      // } else if (isa(x, vec_class)) {
      //   format_vec(buf, x);
    } else if (klass == num_class) {
      ss << num_val(x);
      buf.append(ss.str());
    } else if (klass == class_class) {
      ss << "<CLASS " << class_name(klass);
      //       if (c->param != NULL) {
      //         ss << " ";
      //         buf.append(ss.str());
      //         format_object(buf, c->param);
      //         if (c->dim != (Class*)-1)
      //           post_into(buf, " %d", (int)c->dim);
      //       }
      buf.append(">");
    } else {
      ss << "<OBJECT " << class_name(klass) << ">";
    }
  }
}

void print_object (obj_t *x) {
  std::string buf;
  format_object(buf, x);
  post(buf.c_str());
}

void post_object (char *msg, obj_t *x) {
  post(msg); print_object(x); post("\n");
}

void post_form (obj_t *x) {
  print_object(x); post("\n");
}

void init_lisp (void) {
  any_class = new_class("ANY", any_class);
  ((class_t*)any_class)->super = any_class;
  class_class = new_class("CLASS", any_class);
  list_class = new_class("LIST", any_class);
  sym_class = new_class("SYM", any_class);
  str_class = new_class("STR", any_class);
  num_class = new_class("NUM", num_class);
  bool_class = new_class("BOOL", bool_class);
  fun_class = new_class("FUN", fun_class);
  class_class->type = class_class;
  lisp_nil = cons(NULL, NULL);
  lisp_true  = new_bool(1);
  lisp_false = new_bool(0);
  quote_sym = new_sym("quote");
  head_set(lisp_nil, lisp_nil);
  tail_set(lisp_nil, lisp_nil);
  empty_string = new_str(" ");
}

