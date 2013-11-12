//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gfx.h"
#include "vec.h"

#include <vector>
using namespace std;

// ******   DRAWING PRIMITIVES   ******
// These all draw a simple object centered at (0,0,0), assuming that 
// translation to the appropriate part of space is handled by the caller.
// 2D drawing routines use the XY plane.

// Circles are expensive, so an approximation is cached for fast rendering
#define N_CIRCLE_VERTICES 16
// This structure can be used to represent either a 3D point or 3D vector
struct ptb {
  double x,y;
} circle_vertices[N_CIRCLE_VERTICES];

void draw_cube (double r) {
  double w = r;
  double h = r;
  double d = r;
  glBegin( GL_LINE_STRIP );
  // top
  glVertex3d(-w, -h, -d);
  glVertex3d(-w,  h, -d);
  glVertex3d( w,  h, -d);
  glVertex3d( w, -h, -d);
  glVertex3d(-w, -h, -d);
  // bot
  glVertex3d(-w, -h,  d);
  glVertex3d(-w,  h,  d);
  glVertex3d( w,  h,  d);
  glVertex3d( w, -h,  d);
  glVertex3d(-w, -h,  d);
  // sides
  glVertex3d( w, -h,  d);
  glVertex3d( w, -h, -d);
  glVertex3d( w,  h, -d);
  glVertex3d( w,  h,  d);
  glVertex3d(-w,  h,  d);
  glVertex3d(-w,  h, -d);
  glEnd();
}

// Draws an approximate radius r circle (unfilled)
void draw_circle (double r) {
  glPushMatrix(); // save state
  glScalef(r, r, r);
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < N_CIRCLE_VERTICES-1; i++)
    glVertex2f(circle_vertices[i].x, circle_vertices[i].y);
  glEnd();
  glPopMatrix();
}

// Draws an approximate radius r circle (filled), with a heavy edge
void draw_disk (double r) {
  glPushMatrix(); // save state
  glScalef(r, r, r);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0, 0);
  for (int i = 0; i < N_CIRCLE_VERTICES-1; i++) {
    glVertex2f(circle_vertices[i].x, circle_vertices[i].y);
  }
  glVertex2f(circle_vertices[0].x, circle_vertices[0].y);
  glEnd(); 
  glPopMatrix();
  draw_circle(r); // draw the circle over it, to make the edge heavy
}

// Draws a size 2r square
void draw_square (double r) {
  glBegin(GL_LINE_LOOP);
  glVertex2f(-r, r); glVertex2f( r, r); glVertex2f( r,-r); glVertex2f(-r,-r);
  glEnd();
}

// Draws a rectangle of size width x height
void draw_rect (double w, double h) {
  glPushMatrix(); // save state
  glScalef(w/2, h/2, 1);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-1,-1); glVertex2f(-1, 1); glVertex2f( 1, 1); glVertex2f( 1,-1);
  glEnd(); 
  glPopMatrix();
}

// Draws a filled rectangle of size width x height
void draw_quad (double w, double h) {
  glPushMatrix(); // save state
  glScalef(w/2, h/2, 1);
  glBegin(GL_QUADS);
  glVertex2f(-1,-1); glVertex2f(-1, 1); glVertex2f( 1, 1); glVertex2f( 1,-1);
  glEnd(); 
  glPopMatrix();
}

// Draws a 3D rectangle centered at (0,0,0).
void draw_3quad (double w, double h, double d) {
  glPushMatrix();  // save state
  glScalef(w/2, h/2, d/2);
  glBegin(GL_QUADS);
  glVertex3f(-1,  1, -1); glVertex3f(-1,  1,  1); // Top face
  glVertex3f( 1,  1,  1); glVertex3f( 1,  1, -1);

  glVertex3f(-1, -1, -1); glVertex3f(-1, -1,  1); // Bottom face
  glVertex3f( 1, -1,  1); glVertex3f( 1, -1, -1);

  glVertex3f(-1, -1,  1); glVertex3f(-1,  1,  1); // Front face
  glVertex3f( 1,  1,  1); glVertex3f( 1, -1,  1);

  glVertex3f(-1, -1, -1); glVertex3f(-1,  1, -1); // Back face
  glVertex3f( 1,  1, -1); glVertex3f( 1, -1, -1);

  glVertex3f( 1, -1, -1); glVertex3f( 1, -1,  1); // Right face
  glVertex3f( 1,  1,  1); glVertex3f( 1,  1, -1);

  glVertex3f(-1, -1, -1); glVertex3f(-1, -1,  1); // Left face
  glVertex3f(-1,  1,  1); glVertex3f(-1,  1, -1);

  glEnd(); 
  glPopMatrix();
}

#define MAX_LINES 128
#define MAX_STR   1000

int find_lines ( char *str, char **strs ) {
  if (strlen(str) == 0)
    return 0;
  else {
    int  n = 1, i;
    char *p = str;
    strs[0] = p;
    for (i = 0; *p != 0 && i < MAX_STR; i++) {
      if (*p == '\r') {
	*p++ = 0;
      } else if (*p == '\n') {
	*p        = 0;
	strs[n++] = ++p;
	if (n >= MAX_LINES)
	  return n;
      } else {
	p++;
      }
    }
    return n;
  }
}

#ifdef __WIN32__

#include <windows.h>

void *font =NULL;

struct font_t {
  GLuint base;
  GLYPHMETRICSFLOAT gmf[256];
  Font  *lines;
  Font  *polygons;
};

font_t* font_fixed_filled;
font_t* font_fixed_stroked;
font_t* font_variable_filled;
font_t* font_variable_stroked;

HDC  hDC=NULL;		// Private GDI Device Context

font_t* build_font (char *name, int is_filled) {
  int res;
  HFONT	wfont;				// Windows Font ID
  font_t* font = (Font*)MALLOC(sizeof(Font));
  
  HWND hWnd = FindWindow("GLUT", "Spider");
  hDC  = GetDC(hWnd);

  font->base = glGenLists(256);		// Storage For 256 Characters

  wfont = CreateFont(	-12,		// Height Of Font
                        0,		// Width Of Font
                        0,		// Angle Of Escapement
                        0,		// Orientation Angle
                        FW_BOLD,	// Font Weight
                        FALSE,		// Italic
                        FALSE,		// Underline
                        FALSE,		// Strikeout
                        ANSI_CHARSET,	// Character Set Identifier
                        OUT_TT_PRECIS,			// Output Precision
                        CLIP_DEFAULT_PRECIS,		// Clipping Precision
                        ANTIALIASED_QUALITY,		// Output Quality
                        FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
                        name
                        );		

  SelectObject(hDC, wfont);			// Selects The Font We Created

  res = wglUseFontOutlines(	hDC,			// Select The Current DC
                        0,			// Starting Character
                        255,			// Number Of Display Lists To Build
                        font->base,		// Starting Display Lists
                        0.0f,			// Deviation From The True Outlines
                        0.0f,			// Font Thickness In The Z Direction
                        is_filled ? WGL_FONT_POLYGONS : WGL_FONT_LINES,	// Use Polygons, Not Lines
                        font->gmf);		// Address Of Buffer To Receive Data
  post("BUILT FONT %f %f %f\n", 
       font->gmf['A'].gmfCellIncX, font->gmf['O'].gmfCellIncY, font->gmf['O'].gmfBlackBoxY);
  return font;
}

GLvoid kill_font(font_t* font) {
  glDeleteLists(font->base, 256);			// Delete All 256 Characters
  FREE(&font);
}

double font_height (void *font) {
  GLYPHMETRICSFLOAT *gmf = ((font_t* )font)->gmf;
  return gmf['O'].gmfBlackBoxY * 1.5;
}

double font_char_width (void *font, char c) {
  GLYPHMETRICSFLOAT *gmf = ((font_t* )font)->gmf;
  return gmf[c].gmfCellIncX;
}

void do_stroke_text (void *font, char *text) {
  glPushMatrix();
  glPushAttrib(GL_LIST_BIT);			// Pushes The Display List Bits
  glListBase(((Font*)font)->base);			// Sets The Base Character to 0
  glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text
  glPopAttrib();				// Pops The Display List Bits
  glPopMatrix();
}

void stroke_text (void *font_, char *text) {
  font_t* font = (font_t*)font_;
  do_stroke_text(font, text);
  if (font->lines != font) {
    glPushAttrib(GL_CURRENT_BIT);
    glLineWidth(1);
    glColor4f(0.1, 0.1, 0.1, 1);
    do_stroke_text(font->lines, text);
    glPopAttrib();
  }
}

void *find_font (int font, int is_filled) {
  switch (font) {
  case FONT_FIXED:
    if (is_filled)
      return font_fixed_filled;
    else
      return font_fixed_stroked;
  case FONT_VARIABLE:
    if (is_filled)
      return font_variable_filled;
    else
      return font_variable_stroked;
  default:
    uerror("UNKNOWN FONT %d %d\n", font, is_filled);
  }
}

#else

typedef void font_t;

void* font = (void*)GLUT_STROKE_ROMAN;

double font_height (void *font) {
  // if (font == GLUT_STROKE_ROMAN)
    return 119.05;
    // else
    // return 150.00;
}

double font_char_width (void *font, char c) {
  return glutStrokeWidth(font, c);
}

void stroke_text (void *font, char *txt) {
  glPushMatrix();
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(1.5);  
  for (int i=0; i<strlen(txt); i++)
    glutStrokeCharacter(font, txt[i]);
  glPopMatrix();
}

void *find_font (int font, int is_filled) {
  switch (font) {
  case FONT_FIXED:
    return GLUT_STROKE_MONO_ROMAN;
  case FONT_VARIABLE:
    return GLUT_STROKE_ROMAN;
  default:
    uerror("UNKNOWN FONT %d %d\n", font, is_filled);
  }
}
#endif

std::vector<font_t*> font_stack;

void push_font (void *new_font) {
  font_stack.push_back(font);
  font = new_font;
}

void pop_font (void) {
  font = font_stack.back();
  font_stack.pop_back();
}

// initialization routine which must be called before drawing begins
// Right now, all it needs to do is create the circle approximation
void init_gfx () {
  int i, n = N_CIRCLE_VERTICES;
  // post("INIT DRAW PRIMS\n");
  for (i = 0; i < n; i++) {
    double d = (double)i/(n-1);
    double t = (2*3.1415*(1-d));
    double sx = sin(t);
    double sy = cos(t);
    circle_vertices[i].x = sx;
    circle_vertices[i].y = sy;
  }
#ifdef __WIN32__
  font_fixed_filled  = build_font("Lucida Console", 1);
  font_fixed_stroked = build_font("Lucida Console", 0);
  font_fixed_filled->polygons  = font_fixed_filled;
  font_fixed_stroked->polygons = font_fixed_filled;
  font_fixed_filled->lines  = font_fixed_stroked;
  font_fixed_stroked->lines = font_fixed_stroked;
  font_variable_filled  = build_font("Trebuchet MS", 1);
  font_variable_stroked = build_font("Trebuchet MS", 0);
  font_variable_filled->polygons  = font_variable_filled;
  font_variable_stroked->polygons = font_variable_filled;
  font_variable_filled->lines  = font_variable_stroked;
  font_variable_stroked->lines = font_variable_stroked;
  font = font_variable_filled;
  // "Trebuchet MS"
  // "Comic Sans MS"
#endif
}

// Measure the size of a piece of text when drawn in the specified font.
double stroke_text_size (void* font, char* text, double *height) {
  int i; double l = 0.0;
  for (i=0; i<strlen(text); i++) { l += font_char_width(font, text[i]); }
  *height = font_height(font);
  return l;
}

double stroke_multitext_size (void* font, int n, char** strs, double *height) {
  double w = 0, h = 0;
  int i;
  for (i = 0; i < n; i++) {
    char *s = strs[i];
    double sw, sh;
    sw = stroke_text_size(font, s, &sh);
    w  = max(w, sw);
    h += sh*2;
  }
  *height = h;
  return w;
}

// Draw text centered in a box of specified width and height
// Text is scaled to the box based on width only: height adjusts to
// keep the characters proportional
void draw_text_raw (double w, double h, char *txt) {
  double tw, ts, th, aw, ah;
  glPushMatrix();
  tw = stroke_text_size(font, txt, &th);
  ts = min(w/tw, h/th);
  aw = ts * tw;
  ah = ts * th;
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  glTranslatef(- aw/2, -0.2 * ah, 0.0);  // -0.25
  glScalef(ts, ts, 1.0);
  stroke_text(font, txt);
  glPopMatrix();
}

void draw_text_left_raw (double w, double h, char *txt) {
  double tw, ts, th, aw, ah;
  glPushMatrix();
  tw = stroke_text_size(font, txt, &th);
  ts = min(w/tw, h/th);
  aw = ts * tw;
  ah = ts * th;
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  glTranslatef(- w/2, -0.2 * ah, 0.0);  // -0.25
  glScalef(ts, ts, 1.0);
  stroke_text(font, txt);
  glPopMatrix();
}

void draw_text_left (double w, double h, char *txt) {
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  glLineWidth(2);
  draw_text_left_raw(w, h, txt);
  glPopAttrib();
}

void draw_text_right_raw (double w, double h, char *txt) {
  double tw, ts, th, aw, ah;
  glPushMatrix();
  tw = stroke_text_size(font, txt, &th);
  ts = min(w/tw, h/th);
  aw = ts * tw;
  ah = ts * th;
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  glTranslatef(w/2 - aw, -0.2 * ah, 0.0);  // -0.25
  glScalef(ts, ts, 1.0);
  stroke_text(font, txt);
  glPopMatrix();
}

void draw_text_right (double w, double h, char *txt) {
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  glLineWidth(2);
  draw_text_right_raw(w, h, txt);
  glPopAttrib();
}

void draw_text_dir (txt_dir_t td, double w, double h, char *txt) {
  switch (td) {
  case TD_LEFT:     draw_text_left(w, h, txt); break;
  case TD_RIGHT:    draw_text_right(w, h, txt); break;
  case TD_CENTERED: draw_text(w, h, txt); break;
  }
}

void draw_text_raw_dir (txt_dir_t td, double w, double h, char *txt) {
  switch (td) {
  case TD_LEFT:     draw_text_left_raw(w, h, txt); break;
  case TD_RIGHT:    draw_text_right_raw(w, h, txt); break;
  case TD_CENTERED: draw_text_raw(w, h, txt); break;
  }
}

#define SCALE_FAC 1

void draw_centered_text_block_raw (double w, double h, char *txt) {
  int i, j, n;
  double tw, ts, th, aw, ah;
  char str[MAX_STR];
  char *strs[MAX_LINES];
  strcpy(str, txt);
  n = find_lines(str, strs);
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  aw = SCALE_FAC * w;
  ah = SCALE_FAC * h;
  glPushMatrix();
  glTranslatef(0, ah/2 - ah/n/2, 0.0);
  for (j = 0; j < n; j++) {
    draw_text_raw(aw, ah / n, strs[j]);
    glTranslatef(0, -ah / n, 0);
  }
  glPopMatrix();
}

/*
void draw_centered_text_block_raw (double w, double h, char *txt) {
  int i, j, n;
  double bh, bw, ah, aw, bs;
  double tw, th;
  char str[MAX_STR];
  char *strs[MAX_LINES];
  strcpy(str, txt);
  n = find_lines(str, strs);
  bh = bw = 0;
  for (j = 0; j < n; j++) {
    tw  = stroke_text_size(font, strs[j], &th);
    bw  = max(bw, tw);
    bh += 1.5 * th;
  }
  bs = SCALE_FAC * MIN(w/bw, h/bh);
  aw = bs * bw;
  ah = bs * bh;
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  glPushMatrix();
  // glTranslatef(- aw/2, ah/2 - 0.75 * ah/n, 0.0); // 0.75
  glTranslatef(0 , ah/2 - 0.75 * ah/n, 0.0); // 0.75
  glScalef(bs, bs, 1.0);
  for (j = 0; j < n; j++) {
    tw = stroke_text_size(font, strs[j], &th);
    glPushMatrix();
    glTranslatef(-tw/2, 0, 0);
    stroke_text(font, strs[j]);
    glPopMatrix();
    glTranslatef(0, -th * 1.5, 0);
  }
  glPopMatrix();
}
*/

void draw_justified_text_block_raw (double w, double h, char *txt) {
  int i, j, n;
  double bh, bw, ah, aw, bs;
  double tw, th;
  char str[MAX_STR];
  char *strs[MAX_LINES];
  strcpy(str, txt);
  n = find_lines(str, strs);
  bh = bw = 0;
  for (j = 0; j < n; j++) {
    tw  = stroke_text_size(font, strs[j], &th);
    bw  = max(bw, tw);
    bh += 1.5 * th;
  }
  bs = SCALE_FAC * min(w/bw, h/bh);
  aw = bs * bw;
  ah = bs * bh;
  // glPushMatrix(); glScalef(w/2, h/2, 1); draw_square(1); glPopMatrix();
  glPushMatrix();
  glTranslatef(- aw/2, ah/2 - 0.75 * ah/n, 0.0); // 0.75
  glScalef(bs, bs, 1.0);
  for (j = 0; j < n; j++) {
    tw = stroke_text_size(font, txt, &th);
    glPushMatrix();
    glTranslatef(- (aw - tw), 0.0, 0.0); 
    stroke_text(font, strs[j]);
    glPopMatrix();
    glTranslatef(0, -th * 1.5, 0);
  }
  glPopMatrix();
}

void draw_text_block_raw (txt_dir_t td, double w, double h, char *txt) {
  switch (td) {
  case TD_LEFT:     draw_justified_text_block_raw(w, h, txt); break;
  case TD_RIGHT:    draw_justified_text_block_raw(w, h, txt); break;
  case TD_CENTERED: draw_centered_text_block_raw(w, h, txt); break;
  }
}

void draw_text_block (txt_dir_t td, double w, double h, char *txt) {
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  glLineWidth(2);
  draw_text_block_raw(td, w, h, txt);
  glPopAttrib();
}

double flo_rnd (double min, double max) {
  double val = (double)rand() / (double)RAND_MAX;
  return (val * (max - min)) + min;
}


void draw_text_block_fuzz (txt_dir_t td, double s, int n, double w, double h, char *txt) {
  int i, j, k; 
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  glLineWidth(n > 0 ? 3 : 2);
  double inc = n == 0 ? 0 : s / n;
  // for (k = 0; k <= n/2; k++) {
  for (j = -n; j <= n; j++) {
  for (i = -n; i <= n; i++) {
    glPushMatrix();
    glTranslatef(s * i, s * j, 0);
    // glTranslatef(2 * flo_rnd(-s, s), 2 * flo_rnd(-s, s), 0);
    // glTranslatef(0.5 * s*i + flo_rnd(-inc, inc), 0.5 * s*j + flo_rnd(-inc, inc), 0);
    // glTranslatef(3 * n * flo_rnd(-inc, inc), 3 * n * flo_rnd(-inc, inc), 0);
    draw_text_block_raw(td, w, h, txt);
    glPopMatrix();
  }
  }
  // }
  // draw_text_block_raw(w, h, txt);
  glPopAttrib();
}

// Draw text centered in a box of specified width and height
// Text is scaled to the box based on width only: height adjusts to
// keep the characters proportional
void draw_text (double w, double h, char *txt) {
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  glLineWidth(2);
  draw_text_raw(w, h, txt);
  glPopAttrib();
}

// Draw text centered in a box of specified width and height
// Text is scaled to the box based on width only: height adjusts to
// keep the characters proportional
void draw_text_halo (double w, double h, char *txt) {
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT); // save state
  /*
  glLineWidth(4);
  glColor4f(r, g, b, 0.5);
  draw_text_raw(w, h, txt);
  */
  glLineWidth(2);
  draw_text_raw(w, h, txt);
  glPopAttrib();
}

double determine_rect_grid (int n, double pat_aspect_ratio, double desired_aspect_ratio, vec_t<2> size, vec_t<2>& tot_size, vec_t<2>& dims) {
  double low_err = 1e8;
  int best_cols = n == 1 ? 1 : 1e8;
  int best_rows = n == 1 ? 1 : 1e8;
  // post("DAR %f \n", desired_aspect_ratio);
  for (int c = 1; c < n; c++) {
    int r = ceilf((double)n / (double)c);
    double ar = pat_aspect_ratio * (double)r / c ;
    double err = fabs(ar - desired_aspect_ratio);
    // post("C %d R %d Rf %f CR %d AR %f E %f\n", c, r, roundf((double)n / (double)c), c*r, ar, err);
    if ((err < low_err && r < best_rows) || err == low_err && (c*r < best_cols*best_rows)) {
      best_cols = c;
      best_rows = r;
      low_err = err;
    }
  }
  // post("BEST C %d R %d E %f\n", best_cols, best_rows, low_err);
  dims.x = best_cols;
  dims.y = best_rows;
  double s = min(1.0 / dims.x, 1.0 / dims.y);
  vec_t<2> pat_size = size * s;
  tot_size.x = pat_size.x * dims.x;
  tot_size.y = pat_size.y * dims.y;
  return s;
}

void auto_zoom (vec_t<2> sdims, vec_t<2> dims, double mag, double& mag_fac) {
  // get_bodies_extent_projected(mins, maxs);
  // post("MINS [%f,%f] MAXS [%f, %f]\n", mins.x, mins.y, maxs.x, maxs.y);
  float fac_x = dims.x / sdims.x;
  float fac_y = dims.y / sdims.y;
  float a = 0.01;
  float now_fac = 0.8 * mag * min(fac_x, fac_y);
  mag_fac = a * mag_fac + (1-a)*now_fac;
  // post("FAC %f %f\n", fac, mag_fac);
  glScalef(mag_fac, mag_fac, mag_fac);
}

void view_doc_strings (std::vector< std::string >& strings) {
  double max_name_size = 0;
  int n = strings.size();
  sort(strings.begin(), strings.end());
  double th;
  for (int i = 0; i < n; i++) {
    double tw = stroke_text_size(font, (char*)strings[i].c_str(), &th);
    max_name_size = max(tw, max_name_size);
  }
  double pat_ar = th / max_name_size;
  vec_t<2> vdims(viz->MAX_X - viz->MIN_X, viz->MAX_Y - viz->MIN_Y);
  vec_t<2> tot_size, pat_size, dims;
  vec_t<2> page_size = vec(max_name_size, th);
  vec_t<2> tot_stock_size = page_size * 1.1;
  vec_t<2> stock_size_off = page_size * 0.05;
  double s = determine_rect_grid(n, pat_ar, vdims.y/vdims.x, tot_stock_size, tot_size, dims);
  double mag = 0.8;
  glPushAttrib(GL_CURRENT_BIT);
  int k = 0;
  glPushMatrix();
  static double layout_mag = 1;
  auto_zoom(tot_size, vdims, 1, layout_mag);
  for (int i = 0; i < (int)dims.x; i++) {
    for (int j = 0; j < (int)dims.y; j++, k++) {
      if (k < n) {
        glPushMatrix();
        float x = (i) * s * tot_stock_size.x - tot_size.x/2 + stock_size_off.x/2;
        float y = (dims.y-j-1) * s * tot_stock_size.y - tot_size.y/2 + stock_size_off.y/2;
        glTranslatef( x, y, 0);
        glScalef(s, s, s);
        draw_text_left_raw(page_size.x, page_size.y, (char*)strings[n-k-1].c_str());
        glPopMatrix();
      }
    }
  }
  glPopMatrix();
  glPopAttrib();
}

void draw_pixmap
    (double x, double y, double w, double h, int iw, int ih, void* image) {
  double zx, zy;
  glPushMatrix();
  zx = w * viz->scale.x / iw;
  zy = h * viz->scale.y / ih;
  // glRectf(x, y, x+w, y+h);
  // debug("X %f Y %f W %f H %f\n", x, y, w, h);
  // post("IW %d IH %d\n", iw, ih);
  glRasterPos2f(x - w/2, y - h/2);
  glPixelZoom(zx, zy);
  // glDrawPixels(iw, ih, GL_RGB, GL_UNSIGNED_BYTE, image);
  glDrawPixels(iw, ih, GL_BGR_EXT, GL_UNSIGNED_BYTE, image);
  /*
  glColor3f(x/w, y/h, 0.5);
  a = x - w/2;
  b = y - h/2;
  glLineWidth(2.0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(a, b, 0);
  glVertex3f(a+w, b, 0);
  glVertex3f(a+w, b+h, 0);
  glVertex3f(a, b+h, 0);
  glEnd();
  */
  glPopMatrix();
}

// CLIP forces the number x into the range [min,max]
#define CLIP(x, mn, mx) max(mn, min(mx, x))

// Convert hue/saturation/value to red/green/blue.  Output returned in args.
void hsv_to_rgb (double h, double s, double v, double *r, double *g, double *b) {
  double rt, gt, bt;
  s = CLIP(s, (double)0, (double)1);
  if (s == 0.0) {
    rt = gt = bt = v;
  } else {
    double h_temp = (h == 360.0) ? 0.0 : h;
    double f, p, q, t; 
    h_temp /= 60.0;
    int i = (int)h_temp;
    f = h_temp - i;
    p = v*(1-s);
    q = v*(1-(s*f));
    t = v*(1-(s*(1-f)));
    switch (i) {
    case 0: rt = v; gt = t; bt = p; break;
    case 1: rt = q; gt = v; bt = p; break;
    case 2: rt = p; gt = v; bt = t; break;
    case 3: rt = p; gt = q; bt = v; break;
    case 4: rt = t; gt = p; bt = v; break;
    case 5: rt = v; gt = p; bt = q; break;
    }
  }
  //! Why are these commented out? --jsmb 5/12/06
  // rt = CLIP(rt, 0, 255);
  // gt = CLIP(gt, 0, 255);
  // bt = CLIP(bt, 0, 255);
  *r = rt; *g = gt; *b = bt;
}

// Convert hue/saturation/value to red/green/blue.  Output returned in args.
void rgb_to_hsv (double r, double g, double b, double *h, double *s, double *v) {
  double min   = std::min(r, std::min(g, b));
  double max   = std::max(r, std::max(g, b));
  double delta = max - min;
  *v = max / 255;
  if (max == 0) {
    *s = 0;
    *h = 0; // -1;
    return;
  } else {
    *s = delta / max;
    if (r == max) {
      *h = (g - b) / delta;
    } else if (g == max) {
      *h = 2 + (b - r) / delta;
    } else {
      *h = 4 + (r - g) / delta;
    }
    *h *= 60;
    if (*h < 0)
      *h += 360;
  }
}

std::vector< double > layout_nums( double x, ... ) {
  std::vector< double > res;
  va_list ap;
  va_start(ap, x); 
  for (double a = x; a > 0.0; a = va_arg(ap, double)) 
    res.push_back(a);
  va_end(ap);
  return res;
}

std::vector< double > layout_rep_num( int n, double x ) {
  std::vector< double > res;
  for (int i = 0; i < n; i++)
    res.push_back(x);
  return res;
}

void layout (double total, std::vector< double >& ratios, double spacing, std::vector< double >& offsets, std::vector< double >& amounts, bool is_reversed ) {
  if (ratios.size() > 0) {
    std::vector< double > factors;
    double sum = -spacing;
    for (size_t i = 0; i < ratios.size(); i++) 
      sum += ratios[i] + spacing;
    for (size_t i = 0; i < ratios.size(); i++) 
      amounts.push_back(ratios[i]*total/sum);
    spacing *= total/sum;
    double dir = is_reversed ? -1 : 1;
    double offset = dir * (- total / 2.0 + 0.5 * amounts[0]);
    offsets.push_back(offset);
    for (size_t i = 1; i < ratios.size(); i++) {
      offset = offset + dir * ((0.5 * (amounts[i-1] + amounts[i])) + spacing);
      offsets.push_back(offset);
    }
  }
}

Layout fabLayout(vec2d_t off, vec2d_t mag) {
  Layout res(off, mag);
  return res;
}

std::vector< Layout > vbox (std::vector< double >& ratios, double spacing, bool is_reversed ) {
  std::vector< Layout > res;
  std::vector< double > offsets, amounts;
  layout(2.0, ratios, spacing, offsets, amounts, is_reversed);
  for (int i = 0; i < offsets.size(); i++) {
    Layout lay(vec2d_t(0.0, -1.0 + offsets[i]), vec2d_t(1.0, amounts[i]));
    res.push_back(lay);
  }
  return res;
}

std::vector< Layout > hbox (std::vector< double >& ratios, double spacing, bool is_reversed ) {
  std::vector< Layout > res;
  std::vector< double > offsets, amounts;
  layout(2.0, ratios, spacing, offsets, amounts, is_reversed);
  for (int i = 0; i < offsets.size(); i++) {
    Layout lay(vec2d_t(-1.0 + offsets[i], 0.0), vec2d_t(amounts[i], 1.0));
    res.push_back(lay);
  }
  return res;
}

