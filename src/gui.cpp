//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include "viz.h"
#include "utils.h"
#include "gui.h"
using namespace std;

int is_two_screen = false;
int n_beats = true;
flo volume = 0.5;

prop_t* is_two_screen_prop = NULL;
prop_t* n_beats_prop = NULL;
prop_t* volume_prop = NULL;

prop_t* is_two_screen_var = NULL;
prop_t* n_beats_var = NULL;
prop_t* volume_var = NULL;

defnumprop(flo,temp,0,0.1);

char* get_is_two_screen (prop_t *prop_t, void* obj) {
  return (char*)bool_to_enum(is_two_screen).c_str();
}

int set_is_two_screen (prop_t *prop, void* obj, char* val) {
  std::string s(val);
  is_two_screen = enum_to_bool(s);
  return 1;
}

int get_n_beats (prop_t *prop, void* obj) {
  return n_beats;
}

int set_n_beats (prop_t *prop, void* obj, int val) {
  n_beats = val;
  return 1;
}

flo get_volume (prop_t *prop, void* obj) {
  return volume;
}

int set_volume (prop_t *prop, void* obj, flo val) {
  volume = val;
  return 1;
}

void test_top_props (void) {
  std::vector<prop_t*> *models = new std::vector<prop_t*>();
  is_two_screen_prop = new enum_prop_t("IS-TWO-SCREEN", &get_is_two_screen, &set_is_two_screen, bool_choices);
  n_beats_prop       = new int_prop_t("N-BEATS", &get_n_beats, &set_n_beats, 1);
  volume_prop        = new flo_prop_t("VOLUME", &get_volume, &set_volume, 0.1);
  string msg;
  printf("%s = %s\n", is_two_screen_prop->name, is_two_screen_prop->post(msg).c_str());
  printf("%s = %s\n", n_beats_prop->name, n_beats_prop->post(msg).c_str());
  printf("%s = %s\n", volume_prop->name, volume_prop->post(msg).c_str());
  is_two_screen_var = new enum_prop_t("IS-TWO-SCREEN", &is_two_screen, bool_choices);
  n_beats_var       = new int_prop_t("N-BEATS", &n_beats, 1);
  volume_var        = new flo_prop_t("VOLUME", &volume, 0.1);
  printf("%s = %s\n", is_two_screen_var->name, is_two_screen_var->post(msg).c_str());
  printf("%s = %s\n", n_beats_var->name, n_beats_var->post(msg).c_str());
  printf("%s = %s\n", volume_var->name, volume_var->post(msg).c_str());
  printf("%s = %s\n", temp_var->name, temp_var->post(msg).c_str());
  props_t* props = new props_t();
  props->install(is_two_screen_var);
  props->install(n_beats_var);
  props->install(volume_var);
  std::vector<const char*> args;
  args.push_back(":is-two-screen");
  args.push_back("true");
  args.push_back(":n-beats");
  args.push_back("10");
  args.push_back(":volume");
  args.push_back("4.5");
  std::vector<const char*>::iterator ap  = args.begin();
  std::vector<const char*>::iterator eap = args.end();
  post("PARSING %d %d\n", props->props->size(), args.size());
  props->parse_args(ap, eap);
  post("%s = %s\n", is_two_screen_var->name, is_two_screen_var->post(msg).c_str());
  post("%s = %s\n", n_beats_var->name, n_beats_var->post(msg).c_str());
  post("%s = %s\n", volume_var->name, volume_var->post(msg).c_str());
}

void abort_fun (cmd_t* cmd, int key, int modifiers, void* obj) { post("ABORTING\n"); }
void abolish_fun (cmd_t* cmd, int key, int modifiers, void* obj) { post("ABOLISHING\n"); }
void search_fun (cmd_t* cmd, int key, int modifiers, void* obj) { post("SEARCHING\n"); }
void quit_fun (cmd_t* cmd, int key, int modifiers, void* obj) { post("QUITTING\n"); }

cmd_t* abort_cmd   = new cmd_t("ABORT", "Aborts input", &abort_fun);
cmd_t* quit_cmd   = new cmd_t("QUIT", "Quits application", &quit_fun);
cmd_t* abolish_cmd = new cmd_t( "ABOLISH", "Abolish running", &abolish_fun);
cmd_t* search_cmd  = new cmd_t("SEARCH", "Search string", &search_fun);
cmds_t* top_cmds = new cmds_t();
cmds_t* cmds;

bool do_cmd_fun (char* msg) {
  string str;
  cmd_t* cmd;
  for (int i = 0; i < strlen(msg); i++)
    str.push_back(toupper(msg[i]));
  cmd = cmds->find(str.c_str());
  if (cmd == NULL) {
    // sprintf(user_msg, "%s %s", "FAILED", msg);
    return false;
  } else {
    cmd->run(cmd, 0, 0, NULL);
    return true;
  }
}

void abort_fun (int key, int modifiers, void* obj) {
  post("ABORTING\n");
}

void cmd_fun (cmd_t* cmd, int key, int modifiers, void* obj) {
  std::vector<char*> cmd_names;
  // post("CMD FUN\n");
  cmds->install_cmd_names(cmd_names);
  string_read_state->init_string_read("CMD>", &do_cmd_fun, &cmd_names, top_keys);
}

cmd_t* cmd_cmd = new cmd_t("CMD", "Execute command", &cmd_fun);

void test_top_cmds(void) {
  top_cmds->install(abort_cmd);
  top_cmds->install(abolish_cmd);
  top_cmds->install(search_cmd);
  abort_cmd->run(abort_cmd, 0, 0, NULL);
  cmd_t *cmd = top_cmds->find("abort");
  cmd->run(cmd, 0, 0, NULL);
  std::vector<cmd_t*> matches = top_cmds->apropos("AB");
  for (int i = 0; i < matches.size(); i++)
    post("MATCH%d = %s\n", i, matches[i]->name);
  string match;
  int tab;
  matches = top_cmds->max_tab_point("AB", match, tab);
  post("TAB POINT %d MATCH %s\n", tab, match.c_str());
  for (int i = 0; i < matches.size(); i++)
    post("MATCH%d = %s\n", i, matches[i]->name);
}

extern std::string user_msg;

void key_not_found_fun (cmd_t* cmd, int key, int modifiers, void* obj) {
  if (key == '' || (key == 'G' && modifiers == GLUT_ACTIVE_CTRL))
    abort_fun(key, modifiers, NULL);
  else {
    user_msg.assign("???");
    post("key not found %c\n", key);
    now_keys = top_keys;
  }
}

cmd_t* key_not_found = new cmd_t("not-found", "not-found", &key_not_found_fun, NULL);

keys_t* top_keys = new keys_t(key_not_found);
keys_t* misc_keys = new keys_t(key_not_found);
kee_t* misc_key;
kee_t* quit_key;

void test_keys (void) {
  top_keys->install("C-G", abort_cmd);
  top_keys->install("M-X", cmd_cmd);
  misc_key = top_keys->install("C-X", misc_keys);
  quit_key = misc_keys->install("C-C", misc_key, quit_cmd);
  post("QUIT KEY %s\n", quit_key->print());
  now_keys = top_keys;
  now_keys->do_process_keys('', 0, 0);
  now_keys->do_process_keys('X', 0, GLUT_ACTIVE_ALT);
  post("M-X USER MSG %s\n", user_msg.c_str());
  now_keys->do_process_keys('a', 0, 0);
  now_keys->do_process_keys('\t', 0, 0);
  post("M-X TAB USER MSG %s\n", user_msg.c_str());
  now_keys->do_process_keys('R', 0, 0);
  post("R USER MSG %s\n", user_msg.c_str());
  now_keys->do_process_keys('T', 0, 0);
  post("T USER MSG %s\n", user_msg.c_str());
  now_keys->do_process_keys('\r', 0, 0);
  post("RET USER MSG %s\n", user_msg.c_str());
}

int main (int n, char *args[]) {
  // install_top_key_binding
  //  ("M-X", install_cmd(general_cmd_cat, "CMD", "Execute command", &cmd_fun));
  cmds = top_cmds;
  init_props();
  init_cmds();
  init_keys();
  test_top_props();
  test_top_cmds();
  test_keys();
}
