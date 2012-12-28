//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __READER__
#define __READER__

#include "lisp.h"
#include <vector>
#include <string>
#include <sstream>

class dirs_t {
 public:
  std::string path;
  std::vector<std::string> filenames;
  dirs_t (char* path_);
  int nth_filename (int i, const char* filename, std::string& pathname);
  int add_filename (const char* filename);
  obj_t* read_object (const char* filename);
  obj_t* read_objects (const char* filename);
};

extern std::vector< std::string >* parse_path (char* path_, std::vector< std::string >* dirs);
extern obj_t* read_object (const char* str, int *start);
extern obj_t* read_object_from (const char* filename);
extern obj_t* read_objects_from (const char* filename);
extern const char* to_prefix (const char* str);

extern env_t nul_env;

extern env_t qq_env (char* str, obj_t* val, ...);

extern obj_t* read_qq (char* str, env_t env);

extern obj_t* read_from_str (char* str);

extern void init_reader(void);

#endif
