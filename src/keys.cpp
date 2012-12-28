//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "utils.h"
#include "cmds.h"
#include "keys.h"

// struct ltint {
//   bool operator()(const int a, const int b) const {
//     return a < b;
//   }
// };

std::map<int, char*> special_keys;

char *find_special_key_name (int code) {
  std::map<int, char*>::iterator i;
  for (i = special_keys.begin(); i != special_keys.end(); i++) {
    if (i->first == code) 
      return i->second;
  }
  return "";
}

int find_special_key_code (char *name) {
  std::map<int, char*>::iterator i;
  for (i = special_keys.begin(); i != special_keys.end(); i++) {
    if (strcmp(i->second, name) == 0)
      return i->first;
  }
}

#include "viz.h"

void init_special_keys () {
  special_keys[GLUT_KEY_F1] = "F1";
  special_keys[GLUT_KEY_F2] = "F2";
  special_keys[GLUT_KEY_F3] = "F3";
  special_keys[GLUT_KEY_F4] = "F4";
  special_keys[GLUT_KEY_F5] = "F5";
  special_keys[GLUT_KEY_F6] = "F6";
  special_keys[GLUT_KEY_F7] = "F7";
  special_keys[GLUT_KEY_F8] = "F8";
  special_keys[GLUT_KEY_F9] = "F9";
  special_keys[GLUT_KEY_F10] = "F10";
  special_keys[GLUT_KEY_F11] = "F11";
  special_keys[GLUT_KEY_F12] = "F12";
  special_keys[GLUT_KEY_HOME] = "HOME";
  special_keys[GLUT_KEY_INSERT] = "INS";
  special_keys[GLUT_KEY_END] = "END";
  special_keys[GLUT_KEY_PAGE_UP] = "PU";
  special_keys[GLUT_KEY_PAGE_DOWN] = "PD";
  special_keys[GLUT_KEY_RIGHT] = "RT";
  special_keys[GLUT_KEY_LEFT] = "LF";
  special_keys[GLUT_KEY_UP] = "UP";
  special_keys[GLUT_KEY_DOWN] = "DN";
}

kee_t* kee_t::scan (char *name) {
  int i = 0;
  char *sp = index(name, ' ');
  char *ep = (sp == NULL) ? &name[strlen(name)] : sp;
  char key_name[100];
  strncpy(key_name, name, ep - name + 1);
  char *kp = rindex(key_name, '-');
  kp = kp == NULL ? key_name : kp+1;
  key        = 0;
  special    = 0;
  is_control = (strstr(key_name, "C-") != NULL);
  is_meta    = (strstr(key_name, "M-") != NULL);
  is_shift   = (strstr(key_name, "S-") != NULL);
  if (strcmp(name, " ") == 0 || strcmp(name, "-") == 0) {
    key     = name[0];
  } else if (strlen(kp) > 1) 
    special = find_special_key_code(kp);
  else {
    key     = *kp;
    if (isupper(key)) 
      is_shift = 1;
  }
}

void kee_t::do_print (std::stringstream& ss) {
  if (parent != NULL) {
    parent->do_print(ss);
    // post("PARENT %s\n", name);
    ss << ' ';
    // post("NOW %s\n", name);
  } 
  if (is_control) {
    ss << "C-";
  } 
  if (is_meta) {
    ss << "M-";
  } 
  if (is_shift) {
    ss << "S-";
  }
  switch (key) {
  case KEY_NULL: break;
  case 0:    ss << find_special_key_name(special); break;
  case '\r': ss << "CR"; break;
  case '\t': ss << "TB"; break;
  case '': ss << "DL"; break;
  case '': ss << "BS"; break;
  default:   ss << key; break;
  }
  // post("KEY %d SPECIAL %d\n", key, special);
  // post("FINALLY %s\n", name);
}

char* kee_t::print (void) {
  std::stringstream ss;
  do_print(ss);
  doc = ss.str();
  return (char*)doc.c_str();
}



/// KEY HANDLING

bool is_debug_keys = false;
bool is_key_doc = false;
keys_t* now_keys = top_keys;

void keys_t::do_process_keys (int key, int special, int modifiers, void* obj) {
  int is_ctrl  = (modifiers & GLUT_ACTIVE_CTRL) !=0;
  int is_meta  = (modifiers & GLUT_ACTIVE_ALT)  !=0;
  int is_shift = (modifiers & GLUT_ACTIVE_SHIFT)!=0;
  // post("KEY %c %d\n", key, special);
  kee_t* binding
    = find(key != 0 ? (is_ctrl ? (int)key + 64 : (is_meta ? toupper(key) : (int)key)) : 0,
           is_ctrl, is_meta, is_shift,
           special);
  
  if (binding->cmd == NULL) {
    // post("SUB KEYS\n");
    binding->print();
    // sprintf(user_msg, "%s ", binding->doc);
    now_keys = binding->sub_keys;
  } else {
    // post("KEY CMD %s\n", binding->cmd->name);
    now_keys = top_keys;
    if (is_key_doc) {
      // strcpy(user_msg, binding->cmd->name);
      is_key_doc = 0;
    } else
      binding->cmd->run(binding->cmd, key == 0 ? special : key, modifiers, obj);
  } 
}

void key_press_handler( unsigned char key, int x, int y ) {
  now_keys->do_process_keys(key, 0, glutGetModifiers());
}

kee_t* keys_t::find(char key, bool is_control, bool is_meta, bool is_shift, int special) {
  if (is_debug_keys)
  post("LOOKING FOR %c %d %d %d %d\n", key, is_control, is_meta, is_shift, special);
  for (int i = 0; i < keys->size(); i++) {
    kee_t* b = keys->at(i);
    if (is_debug_keys)
    post("LOOKING AT %c(%d) C %d(%d) M %d(%d) S %d(%d) SP %d(%d)\n", 
	 b->key, b->key == key,
	 b->is_control, b->is_control == is_control,
	 b->is_meta, b->is_meta == is_meta,
	 b->is_shift, b->is_shift == is_shift,
	 b->special, b->special == special);
    if (b->key == key && b->is_control == is_control && b->is_meta == is_meta
        && b->is_shift == is_shift && b->special == special) {
      if (is_debug_keys)
      post("FOUND KEY BINDING %d\n", i);
      return b;
    }
  }
  return backstop;
}

kee_t::kee_t (char key_, bool is_control_, bool is_meta_, bool is_shift_, int special_,
              kee_t* parent_, keys_t* sub_keys_, cmd_t* cmd_) {
  // post("KEY %d SPECIAL %d\n", (int)key, special);
  key        = key_;
  is_control = is_control_;
  is_meta    = is_meta_;
  is_shift   = is_shift_;
  is_default = 0;
  special    = special_;
  cmd        = cmd_;
  sub_keys   = sub_keys_;
  parent     = parent_;
  if (cmd_ != NULL)
    cmd_->key = this;
}

kee_t::kee_t(char *key_name, kee_t* parent_, keys_t* sub_keys_, cmd_t* cmd_) {
  this->scan(key_name);
  // post("KEY %d SPECIAL %d\n", (int)key.key, key.special);
  is_default = 0;
  cmd        = cmd_;
  sub_keys   = sub_keys_;
  parent     = parent_;
  if (cmd_ != NULL)
    cmd_->key = this;
}

kee_t::kee_t(char *doc_, cmd_t* cmd_) {
  doc.assign(doc_);
  cmd        = cmd_;
  sub_keys   = NULL;
  parent     = NULL;
  if (cmd_ != NULL)
    cmd_->key = this;
}

keys_t::keys_t (cmd_t* backstop_) {
  keys = new std::vector<kee_t*>();
  backstop = new kee_t("backstop", backstop_);
}

keys_t::~keys_t() {
  delete backstop;
  delete keys;
}

kee_t* keys_t::install(char *key_name, kee_t* parent_, cmd_t* cmd_) {
  kee_t* key = new kee_t(key_name, parent_, NULL, cmd_);
  keys->push_back(key);
  return key;
}

kee_t* keys_t::install(char *key_name, keys_t* sub_keys_) {
  kee_t* key = new kee_t(key_name, NULL, sub_keys_, NULL);
  keys->push_back(key);
  return key;
}

kee_t* keys_t::install(char *key_name, cmd_t* cmd_) {
  kee_t* key = new kee_t(key_name, NULL, NULL, cmd_);
  keys->push_back(key);
  return key;
}

std::string user_prompt;
std::string user_msg;
string_read_t* string_read_state;
keys_t* string_read_key_bindings;

void string_read_t::init_string_read (char* prompt, do_string_read_fun_t fun, std::vector<char*>* strings, keys_t* old_bindings) {
  user_prompt.assign(prompt);
  user_msg.assign(user_prompt); 
  is_prompt = true;
  is_string_arg = false;
  is_completion = false;
  is_scan_error = false;
  old_key_bindings = old_bindings;
  now_keys = string_read_key_bindings;
  do_read = fun;
  completion_strings.clear();
  for (int i = 0; i < strings->size(); i++)
    completion_strings.push_back(strings->at(i));
}

void string_read_t::start_completion () {
  if (!is_completion) {
    // post("START_COMPLETION %d\n", view->style);
    is_completion = 1;
  }
}

void string_read_t::end_completion () {
  // post("TRYING END COMPLETION %d\n", is_completion);
  if (is_completion) { 
    is_completion = 0; 
    // browser_pop();
    // post("END_COMPLETION %d\n", view->style);
  }
}

void string_read_t::do_completion () {
  // post("COMPLETION STRINGS %d\n", completion_strings.size());
  if (!completion_strings.empty()) {
    int j, k;
    std::string match_string;
    std::vector<char*> match_strings;
    std::string str;
    const char* cp = &(user_msg.c_str())[user_prompt.size()];
    while ((*cp) != 0)
      str.push_back(toupper(*cp++));
    // TODO: PARAMETERIZE BELOW
    // char str[100];
    // if (completion_strings == cmd_names) strtoupper(str, str);
    // for (int i = 0; i < completion_strings.size(); i++)
    //   post("C%d %s\n", i, completion_strings[i]);
    match_strings = 
      max_tab_point((char*)(str.c_str()), completion_strings, match_string, k);
    // post("MATCH %d %s -> %s\n", k, str.c_str(), match_string.c_str());
    user_msg.assign(user_prompt).append(match_string);
  }
}

extern void abort_fun (cmd_t* cmd, int key, int modifiers, void* obj);

void string_read_t::read (int key, int modifiers) {
  // is_key_doc = 0;
  now_keys = string_read_key_bindings;
  if (modifiers == GLUT_ACTIVE_CTRL) {
    key += 64;
    switch (key) {
      // TODO:PUT NAVIGATION KEYS HERE
    case 'G':
      abort_fun(NULL, 0, 0, NULL);
      break;
    case 'H': {
      int len = user_msg.size(); 
      if (len > user_prompt.size()) user_msg.erase(len-1, 1);
      end_completion();
      break; }
    case 'I': do_completion(); break;
    }
  } else if (modifiers == GLUT_ACTIVE_ALT) {
  } else {
    switch (key) {
    case 127:
    case 8: { // backspace
      int len = user_msg.size(); 
      if (len > user_prompt.size()) user_msg.erase(len-1, 1);
      end_completion();
      break; }
    case '\t': do_completion(); break;
    case '\r': {
      std::string msg;
      do_completion();
      end_completion();
      msg.assign(user_msg, user_prompt.size(), user_msg.size());
      // post("DONE READING\n");
      if (do_read((char*)(msg.c_str()))) {
	if (!is_string_arg) {
	  is_prompt = false;
          // post("RESTORING BINDINGS\n");
	  now_keys = old_key_bindings;
	}
      } else {
	is_scan_error = true;
	return;
      }
      break; }
    default: {
      user_msg.push_back(key);
      end_completion();
      break; }
    }
  }
  is_scan_error = false;
}

void string_read_fun (cmd_t* cmd, int key, int modifiers, void* obj) {
  string_read_state->read(key, modifiers);
}

#include <sys/fcntl.h>
// #include <sys/types.h>
#ifdef MACOSX
#include <sys/dirent.h>
#endif
#include <dirent.h>
#include <sys/stat.h>

#define MAX_SIZE 1000

#define CYGWIN 1

int is_dirname (char *name) {
  return (strcmp(name, "..") == 0 || index(name, '.') == NULL);
}

int is_dirent_dir (struct dirent *entry) {
  // #ifdef __APPLE__
  return entry->d_type == DT_DIR;
  // #else
  //   struct stat buf;
  //   fstat(entry->d_fd, &buf);
  //   return S_ISDIR(buf.st_mode);
  // #endif
}

char *strtoupper (char *dst, char *src) {
  int i;
  for (i = 0; i < strlen(src); i++)
    dst[i] = toupper(src[i]);
  dst[i] = (char)0;
  return dst;
}

bool is_strends (char *name, char *suffix) {
  char upsuffix[256];
  strtoupper(upsuffix, suffix);
  return strstr(name, suffix) != NULL || strstr(name, upsuffix) != NULL;
}

bool string_read_t::install_filename_strings (char *pathname, char *suffix) {
  int len;
  DIR *dir;
  completion_pathname = pathname;
  completion_suffix   = suffix;
  len = sizeof(struct dirent);
  dir  = opendir(pathname);
  filename_strings.clear();
  completion_strings.clear();
  if (dir != NULL) {
    // post("OPENING %s %s\n", pathname, suffix);
    for (;;) {
      struct dirent *entry;
      entry = readdir(dir);
      // post("READING %lx\n", entry);
      if (entry == NULL)
	break;
      int is_dir = is_dirent_dir(entry);
      if (is_strends(entry->d_name, suffix) || 
	  (is_dir && strcmp(entry->d_name, ".svn") != 0)) {
	int len = strlen(entry->d_name) - (is_dir?0:strlen(suffix));
        std::string filename;
        filename.assign(entry->d_name, len);
        filename_strings.push_back(filename);
	// post("FN %s\n", strs[k]);
      }
      for (int i = 0; i < filename_strings.size(); i++) {
        completion_strings.push_back((char*)(filename_strings[i].c_str()));
      }
    }
    closedir(dir);
    return true;
    // post("DONE\n");
  } else {
    printf("UNABLE TO OPEN %s\n", pathname);
    return false;
  }
}

cmd_t* string_read_cmd = new cmd_t("string", "string", &string_read_fun);

void init_keys (void) {
  init_special_keys();
  string_read_state = new string_read_t();
  string_read_key_bindings = new keys_t(string_read_cmd);
}

