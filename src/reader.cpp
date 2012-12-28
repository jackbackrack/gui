//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "utils.h"
#include "lisp.h"
#include "reader.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

//// DIRS

static std::string ensure_ends (std::string& s, char c) {
  int len = s.size();
  if (len > 0 && s[len-1] != c) 
    s.push_back(c);
  return s;
}

static std::vector< std::string >* split_at (char sc, std::string s, std::vector< std::string >* ss) {
  std::string::iterator se = s.end();
  std::string::iterator sp = s.begin();
  std::string str;
  for (;;) {
    if (sp == se) {
      ss->push_back(str);
      return ss;
    }
    char c = *sp++;
    if (c == sc) {
      ss->push_back(str);
      str.clear();
    } else
      str.push_back(c);
  }
}

std::vector<std::string>* parse_path (char* path_, std::vector<std::string>* dirs) {
  std::string path;
  split_at(';', path.assign(path_), dirs);
  for (int i = 0; i < dirs->size(); i++)
    ensure_ends(dirs->at(i), '/');
  return dirs;
}

dirs_t::dirs_t (char* path_) {
  path.assign(path_);
  parse_path(path_, &filenames);
}

int dirs_t::nth_filename (int i, const char* filename, std::string& pathname) {
  // post("NTH %d\n", i);
  if (i == 0) {
    pathname.assign(filename);
    return true;
  }
  if (i >= filenames.size()) {
    pathname.assign("");
    return false;
  }
  pathname.assign(filenames[i-1]).append(filename);
  // post("PATHNAME %d %s\n", i, pathname);
  return true;
} 

int dirs_t::add_filename (const char* filename) {
  std::string pathname(filename);
  filenames.push_back(pathname);
}

obj_t* dirs_t::read_object (const char* filename) {
  std::string pathname;
  for (int i = 0; ; i++) {
    if (nth_filename(i, filename, pathname)) {
      obj_t* res = read_object_from(pathname.c_str());
      // post("PATHNAME %s => %p\n", pathname, res);
      if (res != NULL) 
	return res;
    } else
      return NULL;
  }
} 

obj_t* dirs_t::read_objects (const char *filename) {
  std::string pathname;
  for (int i = 0; ; i++) {
    if (nth_filename(i, filename, pathname)) {
      obj_t* res = read_objects_from(pathname.c_str());
      if (res != NULL) 
	return res;
    } else
      return NULL;
  }
} 

//// READER PROPER

char* dup_name (const char* str) {
  char* name = (char*)malloc(strlen(str)+1);
  strcpy(name, str);
  return name;
}

class tok_t : public obj_t {
 public:
  char* name;
  tok_t (char* name_);
};

inline char* tok_name (obj_t* tok) { return ((tok_t*)tok)->name; }

obj_t* tok_class;

tok_t::tok_t (char* name_) {
  type = tok_class;
  name = name_;
}

obj_t* new_tok (char* name) {
  return (obj_t*)new tok_t(name);
}

obj_t* token_left_paren;
obj_t* token_right_paren;
obj_t* token_hash_slash;
obj_t* token_quote;
obj_t* token_eof;

bool is_num (const char *name) {
  int i;
  bool nump;
  nump = isdigit(name[0]) || (name[0] == '-' && strlen(name) > 1);
  for (i = 1; i < strlen(name); i++) {
    nump = nump && (isdigit(name[i]) || name[i] == '.');
  }
  return nump;
}

obj_t* new_sym_or_num (const char* name) {
  if (is_num(name)) {
    int inum; flo fnum;
    int res = sscanf(name, "%f", &fnum);
    if (res == 1) {
      return new_num(fnum);
    } else {
      res = sscanf(name, "%d", &inum);
      if (res == 1) {
	return new_num((flo)inum);
      } else
	uerror("UNABLE TO PARSE NUM %s", name);
    }
  } else
    return new_sym(dup_name(name));
}

#define OSTR_CHR '|'
#define  STR_CHR '\"'
#define RSTR_CHR '\''
#define Q_CHR    '\''
#define H_CHR    '#'
#define C_CHR    '\''
// #define C_CHR    '\\'

obj_t* read_token (const char* str, int* start) {
  int  is_str = 0;
  int  i = *start;
  int  j = 0;
  int  len = strlen(str);
  bool is_raw_str = false;
  bool is_ostr = false;
  bool is_hash = false;
  char c;
  std::string buf;
  // post("READING TOKEN %s %d\n", string, *start);
  for (;;) {
    if (i < len) {
      c = str[i++]; *start = i;
      switch (c) {
      case ' ': case '\t': case '\n': case '\r': break;
      case '(': return token_left_paren;
      case ')': return token_right_paren;
      case H_CHR:  
	if (i < len) {
	  c = str[i++];  *start = i;
	  if (c == C_CHR) 
	    return token_hash_slash;
	  else if (c == 'T' || c == 't')
	    return lisp_true;
	  else if (c == 'F' || c == 'f')
	    return lisp_false;
	  else
	    uerror("BAD CHAR TOKEN %c %s %d\n", c, str, i);
	} else
	  uerror("BAD CHAR TOKEN %s %d\n", str, i);
      case OSTR_CHR:  
	is_ostr = true;
      case STR_CHR:  
	is_str = true; goto ready;
	// case RSTR_CHR: is_raw_str = true; is_str = true; goto ready;
      case Q_CHR: return token_quote;
      case ';': while (i < len) {
	          c = str[i++]; *start = i;
		  if (c == '\n' || c == '\r') break;
                } 
	        return read_token(str, start);
      default:  buf.push_back(c); goto ready;
      } 
    } else
      return token_eof;
  }
 ready:
  if (is_str) {
    bool is_esc = false;
    for (; i < len; ) {
      c      = str[i++];
      *start = i;
      if (!is_raw_str && c == '\\') {
	is_esc = true;
	continue;
      }
      if ((!is_esc && !is_raw_str && 
             ((!is_ostr && c == STR_CHR) || (is_ostr && c == OSTR_CHR))) || 
          (is_raw_str && c == RSTR_CHR)) {
	return new_str(dup_name(buf.c_str()));
      }
      buf.push_back((is_esc && c == 'n') ? '\n' : c); 
      is_esc = false;
    }
    uerror("unable to find end of string %s\n", buf.c_str());
  } else {
    for (; i < len; ) {
      c      = str[i++];
      *start = i;
      if (c == ')' || c == '('  || c == ' ' || c == '\t' || c == '\n' || c == '\r') {
	if (c == ')' || c == '(')
	  *start -= 1;
	return new_sym_or_num(buf.c_str());
      }
      buf.push_back(c);
    }
    *start = i;
    return new_sym_or_num(buf.c_str());
  }
}

extern obj_t* read_from (obj_t* token, const char* str, int* start);

obj_t* read_list (const char* str, int* start) {
  obj_t* list = lisp_nil;
  // debug("READING LIST %d\n", *start);
  for (;;) {
    obj_t* token = read_token(str, start);
    obj_t* expr;
    // print_token(token); debug(" READ LIST TOKEN %d\n", *start);
    if (token == token_right_paren || token == token_eof) {
      std::string buf; list->blat(buf);
      // post("READ-LIST %s\n", buf.c_str());
      return list_rev(list);
    }
    expr = read_from(token, str, start);
    // post("PAIRING "); print_object(list); post("\n");
    list = cons(expr, list);
  }
}

obj_t* read_from (obj_t* token, const char* str, int* start) {
  // post("READING FROM %s\n", &str[*start]);
  if (obj_class(token) == tok_class) {
    if (token == token_quote) {
      return list(new_sym("QUOTE"), read_object(str, start), NULL);
    } else if (token == token_hash_slash) {
      obj_t* obj = read_object(str, start);
      if (obj_class(obj) == num_class)
        return new_num('0' + num_val(obj));
      else if (obj_class(obj) == sym_class)
        return new_num(sym_name(obj)[0]);
      else
        uerror("BAD CHAR TOKEN\n");
    } else if (token == token_left_paren) {
      return read_list(str, start);
    } else if (token == token_right_paren) {
      uerror("Unbalanced parens %s\n", str);
    } else if (token == token_eof) {
      return NULL;
    } else {
      uerror("Unknown token %s\n", tok_name(token));
    }
  } else
    return token;
}

obj_t* read_object (const char* string, int* start) {
  obj_t* token = read_token(string, start);
  return read_from(token, string, start);
}

bool copy_from_file (const char* filename, std::string& buf) {
  FILE *file;
  if ((file = fopen(filename, "r")) <= 0) 
    return false;
  for (;;) {
    char str[100];
    int k = fread(str, 1, sizeof(str), file);
    if (k > 0) {
      buf.append(str, k);
    } else
      break;
  }
  fclose(file);
  return true;
}

obj_t* read_object_from (const char* filename) {
  int    start = 0;
  obj_t* res;
  std::string buf;

  if (!copy_from_file(filename, buf))
    return NULL;
  return read_object(to_prefix(buf.c_str()), &start);
}

obj_t* read_objects_from (const char* filename) {
  int    start = 0;
  obj_t* objs = lisp_nil;
  std::string buf;

  if (!copy_from_file(filename, buf))
    return NULL;
  for (;;) {
    obj_t* obj = read_object(to_prefix(buf.c_str()), &start);
    if (obj == NULL)
      return list_rev(objs);
    else
      objs = cons(obj, objs);
  }
}

//// QUASIQUOTE SUPPORT

env_t nul_env;

env_t qq_env (char* str, obj_t* val, ...) {
  int i, n;
  va_list ap; 
  env_t res;
  res.push_back(new_sym(str)); res.push_back(val);
  va_start(ap, val);
  for (n = 1; ; n++) {
    char* s = va_arg(ap, char *);
    if (s == NULL) break;
    obj_t* v = va_arg(ap, obj_t*);
    if (v == NULL) break;
    res.push_back(new_sym(s)); res.push_back(v);
  }
  va_end(ap);
  return res;
}

obj_t* read_from_str (char* str) {
  int    j = 0;
  obj_t* obj = read_object(str, &j); 
  return obj;
}

obj_t* copy_eval_quasi_quote(obj_t* obj, env_t& env) {
  if (obj_class(obj) == sym_class) {
    if (sym_name(obj)[0] == '$') {
      return obj->eval(env);
    } else
      return obj;
  } else if (obj_class(obj) == num_class || obj_class(obj) == str_class) {
    return obj;
  } else if (obj_class(obj) == list_class) {
    int is_dot = 0;
    obj_t* args = lisp_nil;
    for (int i = 0; i < list_len(obj); i++) {
      obj_t* copy = copy_eval_quasi_quote(nth(obj, i), env);
      if (is_dot) {
	args = list_rev(args);
	obj_t* a = args;
	while (tail(a) != lisp_nil)
	  a = tail(a);
	tail_set(a, copy);
	return args;
      } else if (obj_class(copy) == sym_class && strcmp(sym_name(copy), ".") == 0)
	is_dot = 1;
      else
	args = cons(copy, args);
    }
    return list_rev(args);
  } else
    uerror("Unknown quasi quote element %s", class_name(obj_class(obj)));
}

obj_t* read_qq (char* str, env_t env) {
  obj_t* obj = read_from_str(str); 
  return copy_eval_quasi_quote(obj, env);
}

static int is_initd = 0;

void init_reader (void) {
  if (!is_initd) {
    is_initd = 1;
    tok_class = new_class("TOK", tok_class);
    token_left_paren = new_tok("(");
    token_right_paren = new_tok(")");
    token_hash_slash = new_tok("HASH-SLASH");
    token_quote = new_tok("QUOTE");
    token_eof = new_tok("EOF");
  }
}  

