//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef IS_CMDS
#define IS_CMDS

#include "keys.h"

#include <vector>
#include <string>
#include <sstream>

typedef void (*cmd_fun_t)(cmd_t* cmd, int key, int modifier, void* obj);

class cmds_t;

class cmd_t {
 public:
  char*     name;
  int       num;  // WHY NUM?
  cmd_fun_t run;
  kee_t*    key;  // MIGHT NOT WANT THIS
  cmds_t*   cat;
  char*     doc;  
  void*     dat;
  // string    str;  // total doc string with key binding
  char* cmd_doc(void); // TODO
  void init (char* name_, char* doc_, cmd_fun_t run_, void* dat);
  cmd_t(void);
  cmd_t(char* name, char* doc, cmd_fun_t run);
  cmd_t(char* name, char* doc, cmd_fun_t run, void* dat);
};

class prop_t;

class prop_cmd_t : public cmd_t {
 public:
  prop_t* prop;
  prop_cmd_t(prop_t* prop_, char* doc);
};

class cmds_t {
 public:
  std::vector<cmd_t*> *cmds;
  cmds_t (void);
  ~cmds_t ();
  cmd_t* install (cmd_t *cmd);
  cmd_t* install (cmd_t *cmd, char* name, char* doc, cmd_fun_t run, void* dat=NULL);
  void install_cmd_names (std::vector<char*>& strs);
  cmd_t* find (const char *name);
  std::vector<cmd_t*> apropos (char *name);
  std::vector<cmd_t*> max_tab_point (char *name, std::string& match, int &tab);
  std::vector<cmd_t*> starts_with (char *name);
  std::vector<char*> starts_with (char *name, std::vector<char*> strings);
};

extern std::vector<char*> apropos (char *name, std::vector<char*> strings);
extern std::vector<char*> max_tab_point (char *name, std::vector<char*> strings, std::string& match, int& tab);

extern cmd_t* key_not_found;

extern void toggle_prop_fun (cmd_t* cmd, int key, int modifiers);
extern void inc_prop_fun (cmd_t* cmd, int key, int modifiers);
extern void dec_prop_fun (cmd_t* cmd, int key, int modifiers);
extern cmd_t* new_toggle_prop_cmd (prop_t* prop);
extern cmd_t* new_dec_prop_cmd (prop_t* prop);
extern cmd_t* new_inc_prop_cmd (prop_t* prop);
extern void double_prop_fun (cmd_t* cmd, int key, int modifiers);
extern void half_prop_fun (cmd_t* cmd, int key, int modifiers);
extern cmd_t* new_double_prop_cmd (prop_t* prop);
extern cmd_t* new_half_prop_cmd (prop_t* prop);

extern void init_cmds (void);

#endif
