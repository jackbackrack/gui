//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#ifndef __IS_GFX__
#define __IS_GFX__

#include <string>
#include <vector>
#include "vec.h"
#include "utils.h"
#include "viz.h"

#define GL_BGR_EXT                        0x80E0
#define GL_BGRA_EXT                       0x80E1

enum txt_dir_t {
  TD_LEFT,
  TD_CENTERED,
  TD_RIGHT
};

// Drawing functions
extern void draw_circle (double r);
extern void draw_disk (double r);
extern void draw_square (double r);
extern void draw_rect (double w, double h);
extern void draw_quad (double w, double h);
extern void draw_3quad (double w, double h, double d);
extern void draw_text (double w, double h, char *txt);
extern void draw_text_raw (double w, double h, char *txt);
extern void draw_text_left_raw (double w, double h, char *txt);
extern void draw_text_left (double w, double h, char *txt);
extern void draw_text_right_raw (double w, double h, char *txt);
extern void draw_text_right (double w, double h, char *txt);
extern void draw_text_dir (txt_dir_t td, double w, double h, char *txt);
extern void draw_text_raw_dir (txt_dir_t td, double w, double h, char *txt);
extern void draw_text_halo (double w, double h, char *txt);
extern void draw_text_block (txt_dir_t td, double w, double h, char *txt);
extern void draw_text_block_fuzz (txt_dir_t td, double s, int n, double w, double h, char *txt);
extern void view_doc_strings (std::vector< std::string >& strings);
extern void auto_zoom (vec_t<2> sdims, vec_t<2> dims, double mag, double& last_fac);
extern double determine_rect_grid (int n, double pat_aspect_ratio, double desired_aspect_ratio, vec_t<2> size, vec_t<2>& tot_size, vec_t<2>& dims);
extern void hsv_to_rgb (double h, double s, double v, double *r, double *g, double *b);
extern void rgb_to_hsv (double r, double g, double b, double *h, double *s, double *v);
extern void draw_pixmap (double x, double y, double w, double h, int iw, int ih, void* image);
extern void* find_font (int font, int is_filled);
extern void push_font (void *font);
extern void pop_font (void);
enum {
  FONT_FIXED = 0,
  FONT_VARIABLE = 1
};

class Layout {
 public:
  vec2d_t off;
  vec2d_t mag;
  void begin( void ) { 
    glPushMatrix(); 
    gl_translate(off);
    gl_scale(mag);
  }
  void end( void ) { 
    glPopMatrix(); 
  }
 Layout(vec2d_t off, vec2d_t mag) : off(off), mag(mag) { }
};

std::vector< double > layout_nums( double x, ... );
std::vector< double > layout_rep_num( int n, double x );

void layout (double total, std::vector< double >& ratios, double spacing, std::vector< double >& offsets, std::vector< double >& amounts, bool is_reversed = false );

std::vector< Layout > vbox (std::vector< double >& ratios, double spacing, bool is_reversed = false );
std::vector< Layout > hbox (std::vector< double >& ratios, double spacing, bool is_reversed = false );

extern void init_gfx ();

#endif
