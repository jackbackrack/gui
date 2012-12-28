//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "utils.h"
#include "reader.h"
#include "lisp.h"
#include "lay.h"
#include "props.h"
#include "viz.h"

keys_t* top_keys;

const char* to_prefix (const char* str) { return str; }

class lay_viz_t : public viz_t {
  int key_hit (int cmd, int modifiers);
  int exec (int is_pause);
  int render (int is_picking);
  int open (int arg_offset, int argc, const char** argv);
  int close (void);
};

dirs_t* lay_dirs = new dirs_t(".");

lay_t* root = NULL;
stack_lay_t* focus;
bool is_switch = false;
int focus_n = 0;

int lay_viz_t::key_hit (int cmd, int modifiers) {
  stack_lay_t* views = focus;
  prop_lay_t* view   = (prop_lay_t*)(views->children.back());
  if (cmd == 'n') {
    if (view->type == prop_lay_class)
      view->inc_prop_num = 1;
  } else if (cmd == 'p') {
    if (view->type == prop_lay_class)
      view->inc_prop_num = -1;
  } else if (cmd == '+') {
    if (view->type == prop_lay_class) {
      prop_t* prop = view->props[view->prop_num];
      prop->incr();
    }
  } else if (cmd == 'e') {
    prop_lay_t* view   = (prop_lay_t*)(views->children.back());
    if (view->type == prop_lay_class) {
      prop_t* prop = view->props[view->prop_num];
      if (prop->type == rec_prop_class) {
        rec_prop_t* rprop = (rec_prop_t*)prop;
        views->to_push = new prop_lay_t(rprop->props->props);
      }
    }
  } else if (cmd == 'v') {
    extern props_t* props1;
    views->to_push = new prop_lay_t(props1->props);
  } else if (cmd == 'b') {
    views->is_pop = true;
  } else if (cmd == '2') {
    views->is_split = true;
  } else if (cmd == '1') {
    post("IS_MERGE SET\n");
    views->is_merge = true;
  } else if (cmd == 'o') {
    post("IS_SWITCH SET\n");
    is_switch = true;
  } else if (cmd == 'c') {
    int n_elts = random() % 100;
    std::vector< lay_t* > elts;
    for (int i = 0; i < n_elts; i++) {
      char str[8];
      for (int j = 0; j < 7; j++)
        str[j] = random()%26 + 'a';
      str[6] = ' ';
      str[7] = 0;
      elts.push_back(new txt_lay_t(str));
    }
    views->to_push = new grid_lay_t(elts);
  }
}

stack_lay_t* do_find_focus (lay_t* lay, int& n, int target) {
  if (lay->type == stack_lay_class) {
    post("LOOKING AT %d TARGET %d\n", n, target);
    if (n == target)
      return (stack_lay_t*)lay;
    else
      n += 1;
  }
  for (int i = 0; i < lay->children.size(); i++) {
    stack_lay_t* res = do_find_focus(lay->children[i], n, target);
    if (res != NULL)
      return res;
  }
  return NULL;
}

stack_lay_t* find_focus (lay_t* lay, int& n, int target) {
  return do_find_focus(lay, n, target);
}

int lay_viz_t::exec (int is_pause) {
  if (is_switch) {
    int n_stacks = 0;
    focus_n += 1;
    focus->is_focus = false;
    focus = find_focus(root, n_stacks, focus_n);
    post("FOUND %d FOCUS %lx OF %d\n", focus_n, focus, n_stacks);
    if (focus == NULL && n_stacks > 0) {
      focus_n = 0;
      n_stacks = 0;
      focus = find_focus(root, n_stacks, focus_n);
      post("REFOUND %d FOCUS %lx OF %d\n", focus_n, focus, n_stacks);
    }
    if (focus != NULL)
      focus->is_focus = true;
    is_switch = false;
  }
  root = root->exec();
}

int lay_viz_t::render (int is_picking) {
  glColor4f(1, 1, 1, 1);
  root->render(is_picking, 0.9*160, 0.9*120);
}

props_t* props1;
defnumprop(flo,temp,32,0.1);
defnumprop(flo,flow,10,1);
defnumprop(flo,humidity,90,1);
props_t* dim_props;
defnumprop(flo,hite,160,1);
defnumprop(flo,widt,120,1);

props_t* get_rec_props (prop_t* spec, void* obj) {
  return ((rec_prop_t*)spec)->props;
}

int lay_viz_t::open (int arg_offset, int argc, const char** argv) {
  init_lisp();
  init_reader();
  init_lay();
  init_props();
  char str[] = "(htile (txt \"abc\") (txt \"def\"))";
  obj_t* lst = read_from_str(str);
  root = (lay_t*)lst->eval(lay_env);
  /*
  lay_t* txt1 = new txt_lay_t(list(new_str("abc"), NULL));
  lay_t* txt2 = new txt_lay_t(list(new_str("def"), NULL));
  root = new htile_lay_t(list(txt1, txt2, NULL));
  */
  /*
  props1 = new props_t();
  props1->install(temp_var);
  props1->install(flow_var);
  props1->install(humidity_var);
  dim_props = new props_t();
  prop_t* prop12 = new rec_prop_t("dims", get_rec_props, dim_props);
  props1->install(prop12);
  dim_props->install(hite_var);
  dim_props->install(widt_var);
  root = new stack_lay_t(new prop_lay_t(props1->props));
  focus = (stack_lay_t*)root;
  focus->is_focus = true;
  // root = new vtile_lay_t(list(txt1, NULL));
  */
  root->open();
}

int lay_viz_t::close (void) {
  root->close();
}

viz_t* new_viz (void) {
  return new lay_viz_t();
}
