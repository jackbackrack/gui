//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "utils.h"
#include "reader.h"
#include "lisp.h"

const char* to_prefix (const char* str) { return str; }

int main (int argc, char* argv[]) {
  std::string s;
  init_lisp();
  init_reader();
  char *lisp_path = ".;../lib;../usr;../lib/core";
  dirs_t* dirs = new dirs_t(lisp_path);
  for (int i = 0; i < dirs->filenames.size(); i++)
    post("DIR%d %s\n", i, dirs->filenames[i].c_str());

  obj_t* l0 = new_sym("abc");
  s.clear(); l0->blat(s);
  post("L0 %s\n", s.c_str());
  obj_t* l1 = cons(l0, lisp_nil);
  s.clear(); l1->blat(s);
  post("L1 %s\n", s.c_str());
  obj_t* l2 = new_num(1.234);
  s.clear(); l2->blat(s);
  post("L2 %s\n", s.c_str());

  obj_t* t0 = read_from_str("abc");
  s.clear(); t0->blat(s);
  post("T0 %s\n", s.c_str());
  obj_t* t1 = read_from_str("(abc)");
  s.clear(); t1->blat(s);
  post("T1 %s\n", s.c_str());
}
