//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "props.h"
#include "cmds.h"
using namespace std;

void cmd_t::init (char* name_, char* doc_, cmd_fun_t run_, void* dat_) {
  name = name_;
  doc  = doc_;
  run  = run_;
  cat  = NULL;
  dat  = dat_;
}

cmd_t::cmd_t(char* name_, char* doc_, cmd_fun_t run_) {
  init(name_, doc_, run_, NULL);
}

cmd_t::cmd_t(char* name_, char* doc_, cmd_fun_t run_, void* dat_) {
  init(name_, doc_, run_, dat_);
}

cmd_t::cmd_t(void) {
  init("", "", NULL, NULL);
}


/*
char* cmd_t::cmd_doc (void) {
  int i;
  char *doc = str;
  strcpy(doc, "");
  key_binding_print(cmd->key_binding);
  strcpy(doc, key_doc(cmd->key_binding));
  for (i = strlen(doc); i < 8; i++)
    doc[i] = ' ';
  sprintf(&doc[i], "%s", cmd->name);
  // post("CMD %s %s\n", cmd->name, key_doc(cmd->key_binding));
  return doc;
}
*/

/// CMD_PROP

void run_prop_cmd (cmd_t* cmd_, int key, int modifier, void* obj) {
  prop_cmd_t* cmd = (prop_cmd_t*)cmd_;
  cmd->prop->incr(obj);
}

prop_cmd_t::prop_cmd_t(prop_t* prop_, char* doc_) {
  prop = prop_;
  run  = &run_prop_cmd;
  doc  = doc_;
}

/// CMDS

cmds_t::cmds_t (void) {
  cmds = new vector<cmd_t*>();
}

cmds_t::~cmds_t () {
  delete cmds;
}

cmd_t* cmds_t::install (cmd_t *cmd) {
  cmds->push_back(cmd);
  cmd->cat = this;
  return cmd;
}

cmd_t* cmds_t::install (cmd_t *cmd, char* name, char* doc, cmd_fun_t run, void* dat) {
  cmd->init(name, doc, run, dat);
  install(cmd);
  return cmd;
}

void cmds_t::install_cmd_names (vector<char*>& strs) {
  strs.clear();
  for (int i = 0; i < cmds->size(); i++) {
    strs.push_back(cmds->at(i)->name);
  }
}  


cmd_t* cmds_t::find (const char* name) {
  for (int i = 0; i < cmds->size(); i++) {
    if (strcasecmp(cmds->at(i)->name, name) == 0)
      return cmds->at(i);
  }
  return NULL;
}

vector<char*> apropos (char *name, vector<char*> strings) {
  vector <char*> res;
  for (int i = 0; i < strings.size(); i++) {
    if (strstr(strings[i], name) != NULL) {
      res.push_back(strings[i]);
    }
  }
  return res;
}

vector<cmd_t*> cmds_t::apropos (char *name) {
  vector <cmd_t*> res;
  for (int i = 0; i < cmds->size(); i++) {
    if (strstr(cmds->at(i)->name, name) != NULL) {
      res.push_back(cmds->at(i));
    }
  }
  return res;
}

vector<char*> starts_with (char *name, vector<char*> strings) {
  vector <char*> res;
  for (int i = 0; i < strings.size(); i++) {
    if (strstr(strings[i], name) == strings[i]) {
      res.push_back(strings[i]);
    }
  }
  return res;
}

vector<cmd_t*> cmds_t::starts_with (char *name) {
  vector <cmd_t*> res;
  for (int i = 0; i < cmds->size(); i++) {
    if (strstr(cmds->at(i)->name, name) == cmds->at(i)->name) {
      res.push_back(cmds->at(i));
    }
  }
  return res;
}

static int do_max_tab_point (char *name, vector<char*> matches, string& match) {
  if (matches.size() == 0) {
    match.assign(name);
    return 0;
  } else {
    int j;
    char *match_name = matches[0];
    for (j = 0; j < strlen(match_name); j++) {
      for (int i = 0; i < matches.size(); i++) {
	if (strncmp(matches[i], match_name, j) != 0) {
          post("NOT MATCHED %s %s AT %d\n", matches[i], match_name, j);
          j = j - 1;
          goto done;
	} 
      } 
    }
  done:
    if (strlen(name) > 0 && j > strlen(name)) {
      match.assign(match_name, j);
    } else
      match.assign(name);
    return j;
  }
}

vector<char*> max_tab_point (char *name, vector<char*> strings, string& match, int& tab) {
  vector<char*> matches = starts_with(name, strings);
  post("MATCHES %s %d\n", name, matches.size());
  tab = do_max_tab_point(name, matches, match);
  return matches;
}

vector<cmd_t*> cmds_t::max_tab_point (char *name, string& match, int& tab) {
  vector<cmd_t*> matches = starts_with(name);
  vector<char*> smatches;
  for (int i = 0; i < matches.size(); i++)
    smatches.push_back(matches[i]->name);
  tab = do_max_tab_point(name, smatches, match);
  return matches;
}

void toggle_prop_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  prop_t* prop = (prop_t*)(cmd->dat);
  prop->incr(obj);
}

void inc_prop_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  prop_t* prop = (prop_t*)(cmd->dat);
  prop->incr(obj);
}

void dec_prop_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  prop_t* prop = (prop_t*)(cmd->dat);
  prop->decr(obj);
}

static char* dup_str (char* str) {
  char* res = (char*)malloc(strlen(str)+1);
  strcpy(res, str);
  for (int i = 0; i < strlen(res); i++)
    res[i] = res[i] == '_' ? '-' : toupper(res[i]);
  return res;
}

cmd_t* new_toggle_prop_cmd (prop_t* prop) {
  char name[100];
  char doc[100];
  sprintf(name, "toggle-%s", prop->name);
  sprintf(doc, "toggle %s", prop->name);
  return new cmd_t(dup_str(name), dup_str(doc), &toggle_prop_fun, prop);
}

cmd_t* new_dec_prop_cmd (prop_t* prop) {
  char name[100];
  char doc[100];
  sprintf(name, "dec-%s", prop->name);
  sprintf(doc, "dec %s", prop->name);
  return new cmd_t(dup_str(name), dup_str(doc), &dec_prop_fun, prop);
}

cmd_t* new_inc_prop_cmd (prop_t* prop) {
  char name[100];
  char doc[100];
  sprintf(name, "inc-%s", prop->name);
  sprintf(doc, "inc %s", prop->name);
  return new cmd_t(dup_str(name), dup_str(doc), &inc_prop_fun, prop);
}

void double_prop_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  prop_t* prop = (prop_t*)(cmd->dat);
  if (prop->type == flo_prop_class) {
    flo val = prop->get.f(prop, obj);
    post("FPC VAL %f\n", val);
    prop->set.f(prop, obj, val*2);
  } else if (prop->type == int_prop_class) {
    int val = prop->get.i(prop, obj);
    post("IPC VAL %d\n", val);
    prop->set.i(prop, obj, val*2);
  }
}

void half_prop_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  prop_t* prop = (prop_t*)(cmd->dat);
  if (prop->type == flo_prop_class) {
    flo val = prop->get.f(prop, obj);
    post("FPC VAL %f\n", val);
    prop->set.f(prop, obj, val/2);
  } else if (prop->type == int_prop_class) {
    int val = prop->get.i(prop, obj);
    post("IPC VAL %d\n", val);
    prop->set.i(prop, obj, val/2);
  }
}

cmd_t* new_half_prop_cmd (prop_t* prop) {
  char name[100];
  char doc[100];
  sprintf(name, "half-%s", prop->name);
  sprintf(doc, "half %s", prop->name);
  return new cmd_t(dup_str(name), dup_str(doc), &half_prop_fun, prop);
}

cmd_t* new_double_prop_cmd (prop_t* prop) {
  char name[100];
  char doc[100];
  sprintf(name, "double-%s", prop->name);
  sprintf(doc, "double %s", prop->name);
  return new cmd_t(dup_str(name), dup_str(doc), &double_prop_fun, prop);
}

void init_cmds (void) {
}
