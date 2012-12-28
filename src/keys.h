//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef IS_KEYS
#define IS_KEYS

#include <map>
#include <string>
#include <sstream>
#include <vector>

class keys_t;
class cmd_t;

const char KEY_NULL = ((char)-1);

class kee_t {
 public:
  char     key;
  bool     is_control;
  bool     is_meta;
  bool     is_shift;
  int      special;
  bool     is_default;
  kee_t*   parent;
  keys_t*  sub_keys;
  std::string   doc;
  cmd_t*   cmd;

  void   do_print (std::stringstream& ss);
  char*  print (void);
  kee_t* scan (char *name);
  kee_t(char key_, bool is_control_, bool is_meta_, bool is_shift_, int special_,
        kee_t* parent_, keys_t* sub_keys_, cmd_t* cmd_);
  kee_t(char *key_name, kee_t* parent_, keys_t* sub_keys_, cmd_t* cmd_);
  kee_t(char *doc, cmd_t* cmd_);
};

typedef bool (*do_string_read_fun_t)(char *str);

class string_read_t {
 public:
  bool is_prompt;
  bool is_string_arg;
  bool is_completion;
  bool is_scan_error;
  keys_t* old_key_bindings;
  char* completion_pathname;
  char* completion_suffix;
  do_string_read_fun_t do_read;
  std::vector<char*> completion_strings;
  std::vector<std::string> filename_strings;
  void reset();
  void init_string_read (char *prompt, do_string_read_fun_t fun, std::vector<char*> *strings, keys_t *old_bindings);
  void read (int key, int modifiers);
  bool install_filename_strings (char *pathname, char *suffix);
  void do_completion ();
  void start_completion ();
  void end_completion ();
};

extern string_read_t* string_read_state;

class keys_t {
 public:
  std::vector<kee_t*> *keys;
  kee_t* backstop;
  ~keys_t();
  keys_t (cmd_t* backstop_);
  kee_t* install(char *key_name, keys_t* sub_keys_);
  kee_t* install(char *key_name, cmd_t* cmd_);
  kee_t* install(char *key_name, kee_t* parent_, cmd_t* cmd_);
  void do_process_keys (int key, int special, int modifiers, void* obj = NULL);
  kee_t* find(char key, bool is_control, bool is_meta, bool is_shift, int special);
};

extern keys_t* string_read_key_bindings;
extern keys_t* now_keys;
extern keys_t* top_keys;
extern void key_press_handler( unsigned char key, int x, int y );

extern void init_keys (void);

extern std::string user_prompt;
extern std::string user_msg;

#endif
