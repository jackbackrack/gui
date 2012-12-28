//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_VIZ__
#define __IS_VIZ__

#include <vector>
#include <string>

#include "vec.h"
#include "glut.h"

#ifdef IS_AUDIO
#include "port_audio.h"
#include "port_midi.h"
#endif

#define MAX_PICKS 100
// #define MAX_SLIDERS 8
#define MAX_KEYS 128
#define MOVE_AMOUNT 20000

#ifdef IS_CV
#include "opencv/cv.h"
#include "opencv/highgui.h"
#else
#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#endif

class viz_t {
 public:
  std::string cmd;
  int is_console;
  int mouse_button;
  int click;
  int click_fuzz;  // # pixels mouse can move before click turns into drag
  int key_modifiers;
  int but_modifiers;
  int is_dumping_avi;
  int is_dumping_jpgs;
  int is_white_background;
  int is_step;
  int is_stepping;
  int is_dumping;
  int is_dump_all;
  int is_tracing;
  int is_audio;
  int is_graphics;
  int is_lighting;
  int is_show_lights;
  int is_shadowing;
  int use_textures;
  int is_full_screen;
  int seq_inc;
  int is_process_frame;
  int shadow_width;
  int shadow_height;
  int is_ambient_shadow_available;
  int is_npot_textures_available;
  int max_tex_size;
  GLuint shadow_texture_id;
  vec_t<2> scale;
  double dt;
  double fps;
  double target_fps;
  double tfps;
  double tdecay;

  flo MIN_X;
  flo MAX_X;
  flo MIN_Y;
  flo MAX_Y;
  flo MIN_Z;
  flo MAX_Z;

  vec_t<2> window_pos;
  vec_t<2,int> window_size;

  vec_t<2> view_size;
  double aspect_ratio;
  vec_t<2> size;
  int n_steps;

  std::vector< int > vals;

  #ifdef IS_AUDIO
  AudioStream playback_stream;
  Aud **in_samples;
  Aud **out_samples;

  double audio_tick;
  double first_audio_tick;
  bool is_first_audio_tick;
  int is_playing_back;
  int is_enable_audio;

  int n_aud_frames;
  int audio_callback
    (const void *input, void *output, unsigned long n_frames, 
     const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status, void* user);
  Aud *speakers;
  #endif
  void audio_init (void);
  void audio_start (void);
  void audio_stop (void);

  char dump_filename[256];
  #ifdef IS_CV
  CvVideoWriter* dumper;
  int fourcc;
  // int fourcc = CV_FOURCC('A', 'V', 'C', '1'); // h.264
  // int fourcc = CV_FOURCC('D','I','V','X'); // mp4
  // int fourcc = CV_FOURCC('H','F','Y','U');
  IplImage *dump_image;
  IplImage *pic_image;
  #else
  void *dumper;
  void *dump_image;
  void *pic_image;
  #endif
  int num_frames;
  int num_dumped_frames;
  int speed;
  flo mag_fac;

  flo fit_z;

  vec_t<2> mouse_pos;

  int picked;
  vec_t<2> drag_old_pos;

  int is_key_hit[MAX_KEYS];
  std::vector< int > picks;

  int dragging_machine;

  int parse_args (int offset, int argc, const char *argv[]);
  void base_init ( int argc, const char* argv[] );
  void idle (void);
  void base_render (void);
  void prepare_view (void);
  void pick_nodes ( int x, int y, bool is_add );
  void base_keyboard_handler( unsigned char key, int x, int y );
  void on_passive_mouse_motion( int x, int y );
  void on_start_drag(int x, int y);
  void on_end_drag(void);
  void base_handle_drag(int x, int y);
  void resize (GLsizei new_width, GLsizei new_height);
  void special_handler( int key, int x, int y );
  void on_mouse_motion( int x, int y );
  void on_button_press ( int button, int state, int x, int y );
  void regenerate_shadow_map(void);
  void setup_rc(void);
  viz_t () : 
    is_dumping_avi(false), 
    is_dumping_jpgs(false),
    is_step(0),
    is_stepping(0),
    is_dumping(0),
    is_dump_all(0),
    is_tracing(0),
    is_audio(0),
    is_graphics(1),
    is_lighting(0),
    is_shadowing(0),
    is_show_lights(0),
    use_textures(1),
    is_full_screen(0),
    seq_inc(0),
    is_process_frame(1),
    tfps(0.0),
    tdecay(0.95),
    mag_fac(1.0),
    target_fps(-1.0),
    fps(0.0),
    MIN_X(-80),
    MAX_X( 80),
    MIN_Y(-60),
    MAX_Y( 60),
    MIN_Z(-60),
    MAX_Z( 60),
    window_pos(800, 0),
    window_size(640, 480),
    view_size(100.0, 100.0),
    aspect_ratio(1.0),
#ifdef IS_AUDIO
    is_first_audio_tick(true),
    audio_tick(-1.0),
    is_playing_back(0),
    is_enable_audio(1),
#endif
#ifdef IS_CV
    fourcc(-1),
#endif
    num_frames(0),
    is_console(0),
    num_dumped_frames(0),
    speed(2),
    dumper(NULL),
    dump_image(NULL),
    pic_image(NULL),
    fit_z(1),
    click(0),
    click_fuzz(5),
    picked(-1),
    mouse_button(-1),
    dragging_machine(0),
    drag_old_pos() { };

  virtual int init (void) { }
  virtual int open (int arg_offset, int argc, const char *argv[]) { }
  virtual int close (void) { if (is_full_screen) toggle_full_screen(); }
  virtual int exec (int is_pause) { }
  virtual int key_hit (int key, int modifiers) { }
  virtual int render (int is_picking) { }
  virtual int render_frame_monitors (void) { }
  virtual int process_picks (std::vector< int >& picks, bool is_add) { }
  virtual int handle_drag (vec_t<2> del) { }
  void open_avi_out (char *filename);
  void close_avi_out (void);
  void write_avi_out (void);
  void start_view3d (int is_picking, int x, int y);
  void end_view3d (void);
  void toggle_full_screen (void);
  void single_step (void);
  void resume_execution (void);
  void set_light_look_at (vec_t<3> pos);
  void write_pic_out (void);
  void quit_sim ( void );
  vec_t<2> get_mouse_pos ( void );
  void determine_grid (int n, flo desired_aspect_ratio, vec_t<2> size, vec_t<2>& tot_size, vec_t<2>& pat_size, vec_t<2>& dims);
  void auto_zoom (vec_t<2> sdims, vec_t<2> dims, double mag, double& last_fac);

  inline double get_width ( void ) { return MAX_X - MIN_X; }
  inline double get_height ( void ) { return MAX_Y - MIN_Y; }
  inline double get_depth ( void ) { return MAX_Z - MIN_Z; }
  inline vec_t<2> get_size ( void ) { return vec_t<2>(get_width(), get_height()); }
  inline double get_z_factor ( void ) { return get_depth() / get_width(); }
};

void draw_rect (vec_t<2> size);
void draw_quad (vec_t<2> size);
void draw_text (vec_t<2> size, char *txt);
double stroke_text_width (void* font, char* text);

extern viz_t *viz;
extern viz_t* new_viz (void);
extern void reset_view (void);

#include "keys.h"
#include "cmds.h"

extern void abort_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern void key_not_found_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* key_not_found;
extern void reset_view_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* reset_view_cmd;
extern void toggle_full_screen_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* toggle_full_screen_cmd;
extern void single_step_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* single_step_cmd;
extern void resume_execution_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* resume_execution_cmd;
extern void quit_fun (cmd_t* cmd, int key, int modifiers, void* obj);
extern cmd_t* quit_cmd;


#endif
