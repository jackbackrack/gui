//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_LAY__
#define __IS_LAY__

#ifdef IS_CV
#include "opencv/cv.h"
#include "opencv/highgui.h"
#endif

#include "utils.h"
#include "timer.h"
#include "viz.h"
#include "time.h"
#include "lisp.h"
#include "reader.h"
#include "gfx.h"

#include <vector>
#include <string>

class lay_t;

typedef void (*do_lay_fun_t)(lay_t* lay, void *a, void *b);

class lay_t : public obj_t {
 public:
  lay_t* parent;
  int is_open;
  std::vector<lay_t*> children;
  void init (obj_t* class_, obj_t* args);
  void init (obj_t* class_, std::vector<lay_t*> args);
  flo percent;
  void eval_percentages (void);
  virtual void open (void) { }
  virtual void close (void) { }
  virtual lay_t* exec (void) { return this; }
  virtual void render (bool is_picking, flo w, flo h) { }
  virtual bool is_percent (void) { return false; }
  void open_children (void);
  void close_children (void);
  void exec_children (void);
  void render_children (bool is_picking, flo w, flo h);
  void do_children (do_lay_fun_t fun, void *a, void *b);
  void do_children_kind (class_t *kind, do_lay_fun_t fun, void *a, void *b);
};

extern std::vector<lay_t*> layvec(lay_t* e, ...);
extern std::vector<lay_t*> list_to_layvec (obj_t* l);

class sim_lay_t : public lay_t {
 public:
  sim_lay_t(obj_t* a);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

class tex_lay_t : public lay_t {
 public:
  int  texture;
  bool is_texture;
  siz_t size;
  picraw_t *image;
  picraw_t *pic;
  char* filename;
  void init_tex (obj_t* args);
  void open (void);
  void close (void);
  void render (bool is_picking, flo w, flo h);
};

class pic_lay_t : public tex_lay_t {
 public:
  pic_lay_t(obj_t* a);
  pic_lay_t(char* filename);
  picraw_t* open_jpg ( char *filename, int depth );
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

class vid_lay_t : public tex_lay_t {
 public:
  cap_t *mov;
  int mov_ticks;
  vid_lay_t(obj_t* a);
  vid_lay_t(char* filename);
  picraw_t* get_next_avi_frame ( void );
  picraw_t* get_next_frame (cap_t *capture);
  void avi_reset (void);
  cap_t* open_avi ( char *filename );
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

class vfx_lay_t : public tex_lay_t {
 public:
  vfx_lay_t(obj_t* a);
  vfx_lay_t(char* filename);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

class txt_lay_t : public lay_t {
 public:
  int       fuzz;
  txt_dir_t dir;
  std::string txt;
  txt_lay_t(obj_t* a);
  txt_lay_t(char* txt, txt_dir_t dir=TD_CENTERED, int fuzz=0);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
  void render_title (char* title, flo w, flo h);
};

class seq_lay_t : public lay_t {
 public:
  int idx;
  seq_lay_t(obj_t* a);
  seq_lay_t(std::vector<lay_t*> lays);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (seq child ...)

class all_lay_t : public lay_t {
 public:
  all_lay_t(obj_t* a);
  all_lay_t(std::vector<lay_t*> lays);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (all child ...)

class rgb_lay_t : public lay_t {
 public:
  flo r, g, b, a;
  rgb_lay_t(obj_t* a);
  rgb_lay_t(lay_t* lay, flo r, flo g, flo b, flo a=1);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (rgb child r g b a=1)

class mag_lay_t : public lay_t {
 public:
  flo amount;
  mag_lay_t(obj_t* a);
  mag_lay_t(lay_t* lay, flo amount);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (rgb :amount child)

class mov_lay_t : public lay_t {
 public:
  flo x, y, z;
  mov_lay_t(obj_t* a);
  mov_lay_t(lay_t* lay, flo x, flo y, flo z);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

class rot_lay_t : public lay_t {
 public:
  float amount;
  rot_lay_t(obj_t* a);
  rot_lay_t(lay_t* lay, flo amount);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (rot child :amount)

class font_lay_t : public lay_t {
 public:
  int font;
  bool is_filled;
  font_lay_t(obj_t* a);
  font_lay_t(lay_t* lay, int font, bool is_filled);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (font child :font :is-filled)

class fil_lay_t : public lay_t {
 public:
  fil_lay_t(obj_t* a);
  fil_lay_t(void);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (fil)

class vtile_lay_t : public lay_t {
 public:
  vtile_lay_t(obj_t* a);
  vtile_lay_t(std::vector<lay_t*> lays);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (vtile child ...)

class htile_lay_t : public lay_t {
 public:
  htile_lay_t(void);
  htile_lay_t(obj_t* a);
  htile_lay_t(std::vector<lay_t*> lays);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (htile child ...)

class percent_lay_t : public lay_t {
 public:
  float percent;
  percent_lay_t(obj_t* a);
  percent_lay_t(lay_t* lay, float percent);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
  bool is_percent (void) { return true; }
};

/// (percent lay :percent)

class timeline_lay_t : public lay_t {
 public:
  obj_t* actions;
  timeline_lay_t(obj_t* a);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (timeline ...) // SPECIAL

class defaults_lay_t : public lay_t {
 public:
  std::vector<char*> defaults;
  defaults_lay_t(obj_t* a);
  defaults_lay_t(lay_t* lay, std::vector<char*> defaults);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (defaults ... child ...) // SPECIAL

class grid_lay_t : public lay_t {
 public:
  grid_lay_t(obj_t* a);
  grid_lay_t(lay_t* elt, ...);
  grid_lay_t(std::vector< lay_t* > elts);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (grid child ...)

class sys_lay_t : public lay_t {
 public:
  std::string cmd;
  sys_lay_t(obj_t* a);
  sys_lay_t(char* txt);
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

/// (sys str)

class hsplit_lay_t : public htile_lay_t {
 public:
  hsplit_lay_t (obj_t* elts);
  hsplit_lay_t(std::vector< lay_t* > elts);
  lay_t* to_merge;
  lay_t* exec (void);
  void open (void);
  void close (void);
  void render (bool is_picking, flo w, flo h);
};

/// (hsplit child ...)

class stack_lay_t : public lay_t {
 public:
  stack_lay_t(obj_t* a);
  stack_lay_t(lay_t* top);
  stack_lay_t (std::vector< lay_t* > cs);
  lay_t* to_push;
  bool is_focus;
  bool is_pop;
  bool is_split;
  bool is_merge;
  void push (lay_t* lay);
  void pop (void);
  void open (void);
  void close (void);
  lay_t* clone (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
};

//// (stack child ...)

#include "props.h"

class prop_lay_t : public lay_t {
 public:
  int prop_num;
  int inc_prop_num;
  std::vector<prop_t*> props;
  rgb_lay_t* highlight;
  htile_lay_t* tile;
  void open (void);
  void close (void);
  lay_t* exec (void);
  void render (bool is_picking, flo w, flo h);
  std::vector< lay_t* >& prop_lay_values (void);
  void prop_num_set (int n);
  void highlight_prop_num (void);
  void unhighlight_prop_num (void);
  prop_lay_t (obj_t* props);
  prop_lay_t(std::vector<prop_t*>* props_);
  void prop_lay_init(std::vector<prop_t*>* props_);
  ~prop_lay_t (void);
};

/// (PROP ...)

extern obj_t* lay_class;
extern obj_t* sim_lay_class;
extern obj_t* tex_lay_class;
extern obj_t* pic_lay_class;
extern obj_t* vfx_lay_class;
extern obj_t* vid_lay_class;
extern obj_t* txt_lay_class;
extern obj_t* seq_lay_class;
extern obj_t* all_lay_class;
extern obj_t* rgb_lay_class;
extern obj_t* mag_lay_class;
extern obj_t* mov_lay_class;
extern obj_t* rot_lay_class;
extern obj_t* font_lay_class;
extern obj_t* fil_lay_class;
extern obj_t* vtile_lay_class;
extern obj_t* htile_lay_class;
extern obj_t* percent_lay_class;
extern obj_t* timeline_lay_class;
extern obj_t* defaults_lay_class;
extern obj_t* grid_lay_class;
extern obj_t* hsplit_lay_class;
extern obj_t* stack_lay_class;
extern obj_t* prop_lay_class;

extern void init_lay (void);

extern env_t lay_env;
extern void add_var (char *name, prop_t *prop);
extern void add_var (prop_t *prop);
extern obj_t* timeline_eval (env_t& env, obj_t* args);

#endif
