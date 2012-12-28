//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "lisp.h"
#include "lay.h"
#include "timer.h"

extern dirs_t* lay_dirs;

//// LAY

obj_t* lay_class;

defnumpropmod(flo, zoom, 1, 1, 0, 1024); 

void lay_t::init (obj_t* class_, std::vector<lay_t*> children_) {
  type = class_;
  percent = 0;
  is_open = false;
  children = children_;
}

std::vector<lay_t*> list_to_layvec (obj_t* l) {
  std::vector<lay_t*> res;
  for (obj_t* lp = l; lp != lisp_nil; lp = tail(lp))
    res.push_back((lay_t*)head(lp));
  return res;
}

std::vector<lay_t*> layvec (lay_t* elt, ...) {
  std::vector< lay_t* > res;
  res.push_back(elt);
  va_list ap;
  va_start(ap, elt);
  for (;;) {
    lay_t* e = va_arg(ap, lay_t*);
    if (e == NULL)
      break;
    res.push_back(e);
  }
  va_end(ap);
  return res;
}

void lay_t::init (obj_t* class_, obj_t* children_) {
  init(class_, list_to_layvec(children_));
}

void lay_t::open_children (void) {
  for (int i = 0; i < children.size(); i++)
    children[i]->open();
}

void lay_t::close_children (void) {
  for (int i = 0; i < children.size(); i++)
    children[i]->close();
}

void lay_t::exec_children (void) {
  for (int i = 0; i < children.size(); i++)
    children[i] = children[i]->exec();
}

void lay_t::render_children (bool is_picking, flo w, flo h) {
  for (int i = 0; i < children.size(); i++)
    children[i]->render(is_picking, w, h);
}

void lay_t::do_children (do_lay_fun_t fun, void *a, void *b) {
  if (is_open) {
    fun(this, a, b);
    for (int i = 0; i < children.size(); i++) 
      children[i]->do_children(fun, a, b);
  }
}

void lay_t::do_children_kind (class_t *kind, do_lay_fun_t fun, void *a, void *b) {
  if (is_open) {
    if (type == kind)
      fun(this, a, b);
    for (int i = 0; i < children.size(); i++) 
      children[i]->do_children_kind(kind, fun, a, b);
  }
}

//// SIM

obj_t* sim_lay_class;

sim_lay_t::sim_lay_t(obj_t* a) {
  init(sim_lay_class, lisp_nil);
}


void sim_lay_t::open (void) {
}

void sim_lay_t::close (void) {
}

lay_t* sim_lay_t::exec (void) {
  return this;
}

void sim_lay_t::render (bool is_picking, flo w, flo h) {
}

//// TEX

obj_t* tex_lay_class;

void tex_lay_t::init_tex (obj_t* args) {
  filename = str_name(nth(args, 0));
}

void tex_lay_t::open (void) {
  is_texture = false;
  glGenTextures( 1, (GLuint*)&texture );
  // post("TEXTURE %d\n", texture);
  glBindTexture( GL_TEXTURE_2D, texture );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

void tex_lay_t::close (void) {
#ifdef IS_CV
  cvRelease( (void**)&pic );
  size = cvSize(0, 0);
#endif
  if (is_texture) {
    // post("DELETING TEXTURE %d\n", sim->texture);
    // textures[sim->texture] = 0;
    glDeleteTextures( 1, (GLuint*)&texture );
    is_texture = false;
  }
}

void tex_lay_t::render (bool is_picking, flo w, flo h) {
#ifdef IS_CV
  CvSize size = cvGetSize(pic);
  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT);
  glBindTexture( GL_TEXTURE_2D, texture );

  if (!is_texture) {
    // post("BINDING TEXTURE %d %lx %d %d\n", sim->texture, sim->pic->imageData, sim->size.width, sim->size.height);
    // gluBuild2DMipmaps(GL_TEXTURE_2, 3, sim->size.width, sim->size.height,
    // 	                 GL_RGB, GL_UNSIGNED_BYTE, sim->pic->imageData);
    glTexImage2D
      (GL_TEXTURE_2D, 0, 3, size.width, size.height, 
       0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pic_dat(pic));
    is_texture = 1;
  } 

  glTexSubImage2D
    (GL_TEXTURE_2D, 0, 0, 0, size.width, size.height, 
     GL_BGR_EXT, GL_UNSIGNED_BYTE, pic_dat(pic));

  glEnable(GL_TEXTURE_2D);

  glPushMatrix();
  glScalef(0.5*w, 0.5*h, 1);

  glBegin( GL_QUADS );
  glTexCoord2d(0.0, 0.0); glVertex2d(-1.0, -1.0);
  glTexCoord2d(1.0, 0.0); glVertex2d( 1.0, -1.0);
  glTexCoord2d(1.0, 1.0); glVertex2d( 1.0,  1.0);
  glTexCoord2d(0.0, 1.0); glVertex2d(-1.0,  1.0);
  glEnd();

  // glFlush( );
  glPopMatrix();
  glPopAttrib();
  glDisable(GL_TEXTURE_2D);
#endif
}

void calc_aspect_size (siz_t size, flo gw, flo gh, flo *rw, flo *rh) {
#ifdef IS_CV
  flo w = /* 0.95 * */ gw;
  flo h = /* 0.95 * */ gh;
  flo iratio = (flo)size.width / size.height;
  flo sratio = w / h;
  flo rratio = iratio / sratio;
  if (rratio > 1)
    h /= rratio;
  else
    w *= rratio;
  *rw = w;
  *rh = h;
#endif
}

//// PIC/MOV SUPPORT

obj_t* vid_lay_class;

cap_t* vid_lay_t::open_avi ( char *filename ) {
#ifdef IS_CV
  std::string pathname;
  for (int i = 0; ; i++) {
    if (lay_dirs->nth_filename(i, filename, pathname)) {
      CvCapture *cap = cvCaptureFromAVI( pathname.c_str() );
      // post("AVI OPENING %d %s %lx\n", i, pathname, cap);
      if (cap != NULL)
	return cap;
    } else 
      return NULL;
  }
#endif
}

void vid_lay_t::avi_reset (void) {
#ifdef IS_CV
  if (1 /* !is_first*/) {
    // post("AVI RELEASING %lx\n", sim->mov);
    cvSetCaptureProperty(mov, CV_CAP_PROP_POS_FRAMES, 0);
  }
#endif
  // debug("DONE AVI\n");
}

pic_t* vid_lay_t::get_next_frame (cap_t *capture) {
#ifdef IS_CV
  IplImage* frame = NULL;
  if( capture != NULL && cvGrabFrame( capture )) 
    frame = cvRetrieveFrame( capture );
  return frame;
#endif
}

pic_t* vid_lay_t::get_next_avi_frame ( void ) {
#ifdef IS_CV
  IplImage* frame = get_next_frame( mov );
  
  // post("AVI FRAME %lx\n", frame);
  if (frame == NULL) {
    avi_reset();
    frame = get_next_frame(mov);
  }
  return frame;
#endif
}

static int next_pow2 (int val) {
  int i;
  int t = 1;
  for (i = 0; i < 31; i++, t *= 2) {
    if (val <= t)
      return t;
  }
  return 256;
}

#ifdef IS_CV
static CvSize next_pow2_size (CvSize size) {
  CvSize res = size;
  int dim = next_pow2(max(size.width, size.height));
  return cvSize(dim, dim);
}
#endif

//// PIC

obj_t* pic_lay_class;

pic_lay_t::pic_lay_t(obj_t* a) {
  init(pic_lay_class, lisp_nil);
  init_tex(a);
}

obj_t* pic_eval (env_t& env, obj_t* args) {
  return new pic_lay_t(args);
}

pic_t* pic_lay_t::open_jpg ( char *filename, int depth ) {
#ifdef IS_CV
  IplImage *image;
  std::string pathname;
  for (int i = 0; ; i++) {
    if (lay_dirs->nth_filename(i, filename, pathname)) {
      image = cvLoadImage( pathname.c_str(), depth );
      // post("OPENING %s %lx\n", pathname, image);
      if (image != NULL)
	return image;
    } else {
      post("Unable to open jpg %s\n", filename); exit(-1);
    }
  }
#endif
}

void pic_lay_t::open (void) {
#ifdef IS_CV
  IplImage *image;
  CvSize size;
  // post("OPENING %s\n", txt);
  image = open_jpg( filename, 3 );
  size = next_pow2_size(cvGetSize(image));
  pic  = cvCreateImage( size, 8, 3 );
  size = cvGetSize(image);
  cvResize(image, pic, 0 /* CV_INTER_LINEAR */ );
  cvRelease( (void**)&image );
  cvFlip( pic, pic, 0 );
  tex_lay_t::open();
#endif
}

void pic_lay_t::close (void) {
  tex_lay_t::close();
}

lay_t* pic_lay_t::exec (void) {
  return this;
}

void pic_lay_t::render (bool is_picking, flo aw, flo ah) {
  if (pic != NULL && !is_picking) {
    flo w, h; 
    // post("RENDER PIC %lx\n", pic->imageData);
    calc_aspect_size(size, aw, ah, &w, &h);
    glColor3f(1, 1, 1);
    tex_lay_t::render(is_picking, w, h);
  }
}

//// VID

vid_lay_t:: vid_lay_t(obj_t* a) {
  init(vid_lay_class, lisp_nil);
  init_tex(a);
}

obj_t* vid_eval (env_t& env, obj_t* args) {
  return new vid_lay_t(args);
}

void vid_lay_t::open (void) {
  tex_lay_t::open();
  mov = open_avi( filename );
}

void vid_lay_t::close (void) {
#ifdef IS_CV
  cvReleaseCapture( &mov );
  tex_lay_t::close();
#endif
}

lay_t* vid_lay_t::exec (void) {
#ifdef IS_CV
  IplImage *image = get_next_avi_frame();
  if (pic == NULL) {
    CvSize size;
    size = next_pow2_size(cvGetSize(image));
    pic  = cvCreateImage( size, 8, 3 );
    size = cvGetSize(image);
  }
  cvResize(image, pic, 0 /* CV_INTER_LINEAR */ );
#endif
  return this;
}

void vid_lay_t::render (bool is_picking, flo aw, flo ah) {
  if (pic != NULL && !is_picking) {
    flo w, h;
    calc_aspect_size(size, aw, ah, &w, &h);
    glColor3f(1, 1, 1);
    tex_lay_t::render(is_picking, w, h);
  }
}

//// TXT

obj_t* txt_lay_class;

bool is_show_txt = true;
bool is_show_title = false;
bool is_show_code = false;
bool is_noting = false;

txt_lay_t::txt_lay_t (obj_t* a) {
  fuzz = 0;
  dir = TD_CENTERED;
  for (; a != lisp_nil; a = tail(tail(a))) {
    if (list_len(a) > 1) {
      obj_t* arg = nth(a, 0);
      char* name = &sym_name(arg)[1];
      if (strcasecmp(name, "fuzz") == 0) 
	fuzz = num_val(nth(a, 1));
      else if (strcasecmp(name, "dir") == 0) {
	char *td = sym_name(nth(a, 1));
	if (strcasecmp(td, "left") == 0)
	  dir = TD_LEFT;
	else if (strcasecmp(td, "centered") == 0)
	  dir = TD_CENTERED;
	else if (strcasecmp(td, "right") == 0)
	  dir = TD_RIGHT;
	else
	  uerror("UNKNOWN TXT DIR %s\n", td);
      } else
	uerror("Unknown txt keyword %s", sym_name(arg));
    } else
      break;
  }
  if (list_len(a) == 1)
    txt.assign(str_name(head(a)));
  init(txt_lay_class, lisp_nil);
}

obj_t* txt_eval (env_t& env, obj_t* args) {
  return new txt_lay_t(args);
}

txt_lay_t::txt_lay_t (char* s, txt_dir_t dir_, int fuzz_) {
  fuzz = fuzz_;
  dir = dir_;
  txt.assign(s);
  init(txt_lay_class, lisp_nil);
}

void txt_lay_t::open (void) {
}

void txt_lay_t::close (void) {
}

lay_t* txt_lay_t::exec (void) {
  return this;
}

const flo TXT_Z = 10;


void txt_lay_t::render_title (char* title, flo w, flo h) {
  glPushMatrix(); glPushAttrib(GL_CURRENT_BIT);
  glTranslatef( 0, -h/2+h/5/1.8, is_noting ? 0 : TXT_Z); // /2
  // glColor3f(1, 0.5, 0.0);
  glColor4f(1, 0.25, 0, 1);
  // post("TITLE WIDTH %f\n", w);
  draw_text_block(TD_LEFT, w, h/5, title);
  glPopAttrib(); glPopMatrix();
}

// void lay_t::render_code (flo w, flo h) {
//   if (is_show_txt && is_show_code) {
//     push_font(find_font(FONT_FIXED, 1));
//     render_title(script, w, h);
//     pop_font();
//   }
// }

void txt_lay_t::render (bool is_picking, flo w, flo h) {
  if (is_show_txt && !is_picking) {
    // post("SHOWING TXT %s %d %d\n", sim->class->name, sim->id, sim->is_show_txt);
    char* str = (char*)(txt.c_str());
    if (is_show_title)
      render_title(str, w, h);
    else {
      if (is_show_code) { 
        push_font(find_font(FONT_FIXED, 1));
        glPushMatrix();
        glTranslatef(0, 0, is_noting ? 0 : TXT_Z/2);
      }
      draw_text_block_fuzz
        (dir, 2/viz->scale.x, fuzz, w, h, str);
      if (is_show_code) {
        glPopMatrix();
        pop_font();
      }
    }
  }
}

//// SYS

obj_t* sys_lay_class;

sys_lay_t::sys_lay_t (obj_t* a) {
  cmd.assign(str_name(head(a)));
  init(txt_lay_class, lisp_nil);
}

obj_t* sys_eval (env_t& env, obj_t* args) {
  return new sys_lay_t(args);
}

sys_lay_t::sys_lay_t (char* s) {
  cmd.assign(s);
  init(sys_lay_class, lisp_nil);
}

void sys_lay_t::open (void) {
  system(cmd.c_str());
}

void sys_lay_t::close (void) {
}

lay_t* sys_lay_t::exec (void) {
  // seq_inc = 1;
  return this;
}

void sys_lay_t::render (bool is_picking, flo w, flo h) {
}

//// SEQ

obj_t* seq_lay_class;

double seq_timestamp = 0;
flo seq_zoom = 1;
bool is_seq_zooming = false;

seq_lay_t::seq_lay_t (obj_t* a) {
  init(seq_lay_class, a);
  idx = 0;
}

obj_t* seq_eval (env_t& env, obj_t* args) {
  return new seq_lay_t(args);
}

void seq_lay_t::open (void) {
  idx = 0;
  children[idx]->open();
}

void seq_lay_t::close (void) {
  children[idx]->close();
}

lay_t* seq_lay_t::exec (void) {
  double secs = get_secs();
  double elapsed = secs - seq_timestamp;
  double period = 0.25;
  if (is_seq_zooming) {
    if (viz->seq_inc != 0) {
      if (seq_timestamp == 0) 
        seq_timestamp = secs;
      else if (elapsed < period)
        seq_zoom = 1 + (10 * elapsed)/period;
      else {
        children[idx]->close();
        idx = (idx + viz->seq_inc + children.size()) % children.size();
        children[idx]->open();
        seq_zoom = 0;
        viz->seq_inc = 0;
      }
    } else if (seq_zoom < 1) {
      seq_zoom = min(1.0, (elapsed-period)/period);
    } else {
      seq_timestamp = 0;
      seq_zoom = 1;
    }
  } else {
    if (viz->seq_inc != 0) {
      children[idx]->close();
      idx = (idx + viz->seq_inc + children.size()) % children.size();
      children[idx]->open();
      viz->seq_inc = 0;
    }
  }
  children[idx] = children[idx]->exec();
  return this;
}

void seq_lay_t::render (bool is_picking, flo w, flo h) {
  glPushMatrix();
  glScalef(seq_zoom, seq_zoom, 1);
  children[idx]->render(is_picking, w, h);
  glPopMatrix();
}

//// ALL

obj_t* all_lay_class;

all_lay_t::all_lay_t (obj_t* a) {
  init(all_lay_class, a);
}

obj_t* all_eval (env_t& env, obj_t* args) {
  return new all_lay_t(args);
}

void all_lay_t::open (void) {
  open_children();
  for (int i = 0; i < children.size(); i++)
    children[i]->open();
}

void all_lay_t::close (void) {
  close_children();
  for (int i = 0; i < children.size(); i++)
    children[i]->close();
}

lay_t* all_lay_t::exec (void) {
  exec_children();
  return this;
}

void all_lay_t::render (bool is_picking, flo w, flo h) {
  render_children(is_picking, w, h);
}

//// RGB

obj_t* rgb_lay_class;

rgb_lay_t::rgb_lay_t (obj_t* a_) {
  r = 1;
  g = 1;
  b = 1;
  a = 1;
  init(rgb_lay_class, a_);
}

obj_t* rgb_eval (env_t& env, obj_t* args) {
  return new rgb_lay_t(args);
}

void rgb_lay_t::open (void) {
  open_children();
}

void rgb_lay_t::close (void) {
  close_children();
}

lay_t* rgb_lay_t::exec (void) {
  exec_children();
  return this;
}

void rgb_lay_t::render (bool is_picking, flo w, flo h) {
  glPushAttrib(GL_CURRENT_BIT);
  glColor4f(r, g, b, a);
  render_children(is_picking, w, h);
  glPopAttrib();
}

//// MAG

obj_t* mag_lay_class;

mag_lay_t::mag_lay_t (obj_t* a) {
  init(mag_lay_class, a);
}

obj_t* mag_eval (env_t& env, obj_t* args) {
  return new mag_lay_t(args);
}

void mag_lay_t::open (void) {
  open_children();
}

void mag_lay_t::close (void) {
  close_children();
}

lay_t* mag_lay_t::exec (void) {
  exec_children();
  return this;
}

void mag_lay_t::render (bool is_picking, flo w, flo h) {
  glPushMatrix();
  glScalef(amount,amount,amount);
  render_children(is_picking, w, h);
  glPopMatrix();
}

//// ROT

obj_t* rot_lay_class;

rot_lay_t::rot_lay_t (obj_t* a) {
  init(rot_lay_class, a);
}

obj_t* rot_eval (env_t& env, obj_t* args) {
  return new rot_lay_t(args);
}

void rot_lay_t::open (void) {
  open_children();
}

void rot_lay_t::close (void) {
  close_children();
}

lay_t* rot_lay_t::exec (void) {
  exec_children();
  return this;
}

void rot_lay_t::render (bool is_picking, flo w, flo h) {
  glPushMatrix();
  flo a = fmod(amount + 360, 360);
  flo now_h, now_w;
  switch ((int)a) {
    case 0:   now_w = w; now_h = h; break;
    case 90:  now_w = h; now_h = w; break;
    case 180: now_w = w; now_h = h; break;
    case 270: now_w = h; now_h = w; break;
  }
  glRotatef(a,0,0,1);
  render_children(is_picking, w, h);
  glPopMatrix();
}

//// MOV

obj_t* mov_lay_class;

mov_lay_t::mov_lay_t (obj_t* a) {
  init(mov_lay_class, a);
}

mov_lay_t::mov_lay_t (lay_t* a, flo x_, flo y_, flo z_) {
  x = x_;
  y = y_;
  z = z_;
  init(mov_lay_class, list(a, NULL));
}

obj_t* mov_eval (env_t& env, obj_t* args) {
  return new mov_lay_t(args);
}

void mov_lay_t::open (void) {
  open_children();
}

void mov_lay_t::close (void) {
  close_children();
}

lay_t* mov_lay_t::exec (void) {
  exec_children();
  return this;
}

void mov_lay_t::render (bool is_picking, flo w, flo h) {
  glPushMatrix();
  glScalef(x, y, z);
  render_children(is_picking, w, h);
  glPopMatrix();
}

//// FONT

obj_t* font_lay_class;

font_lay_t::font_lay_t (obj_t* a) {
  init(font_lay_class, a);
}

obj_t* font_eval (env_t& env, obj_t* args) {
  return new font_lay_t(args);
}

void font_lay_t::open (void) {
  open_children();
}

void font_lay_t::close (void) {
  close_children();
}

lay_t* font_lay_t::exec (void) {
  exec_children();
  return this;
}

void font_lay_t::render (bool is_picking, flo w, flo h) {
  push_font(find_font(font, is_filled));
  render_children(is_picking, w, h);
  pop_font();
}

//// FIL

obj_t* fil_lay_class;

fil_lay_t::fil_lay_t (obj_t* a) {
}

obj_t* fil_eval (env_t& env, obj_t* args) {
  return new fil_lay_t(args);
}

void fil_lay_t::open (void) {
}

void fil_lay_t::close (void) {
}

lay_t* fil_lay_t::exec (void) {
  return this;
}

void fil_lay_t::render (bool is_picking, flo w, flo h) {
  glPushMatrix(); 
  if (is_noting)
    glScalef(2 * w/2, 2 * h/2, 1); 
  else
    glScalef(w/2, h/2, 1); 
  draw_quad(2, 2); 
  glPopMatrix();
}

//// VTILE

obj_t* vtile_lay_class;

void lay_t::eval_percentages (void) {
  flo tot = 100.0;
  int n = children.size();
  for (int i = 0; i < n; i++) {
    lay_t* child = children[i];
    if (child->is_percent()) {
      tot -= child->percent;
      n   -= 1;
    }
  }  
  for (int i = 0; i < n; i++) {
    lay_t* child = children[i];
    if (!child->is_percent()) {
      child->percent = tot / n;
    }
  }  
}

vtile_lay_t::vtile_lay_t (obj_t* a) {
  init(vtile_lay_class, a);
}

obj_t* vtile_eval (env_t& env, obj_t* args) {
  return new vtile_lay_t(args);
}

vtile_lay_t::vtile_lay_t (std::vector< lay_t* > children) {
  init(vtile_lay_class, children);
}

void vtile_lay_t::open (void) {
  open_children();
}

void vtile_lay_t::close (void) {
  close_children();
}

lay_t* vtile_lay_t::exec (void) {
  exec_children();
  return this;
}

void vtile_lay_t::render (bool is_picking, flo w, flo h) {
  int n = children.size();
  flo tot_fac = 0.0;
  eval_percentages();
  for (int i = 0; i < n; i++) {
    lay_t* child = children[i];
    flo now_fac = child->percent / 100.0;
    flo now_h = now_fac * h;
    glPushMatrix();
    glTranslatef(0, h/2 - (tot_fac+now_fac/2)*h, 0);
    child->render(is_picking, w, now_h);
    glPopMatrix();
    tot_fac += now_fac;
  }
}

//// HTILE

obj_t* htile_lay_class;

htile_lay_t::htile_lay_t (void) {
  init(htile_lay_class, lisp_nil);
}

htile_lay_t::htile_lay_t (obj_t* a) {
  init(htile_lay_class, a);
}

obj_t* htile_eval (env_t& env, obj_t* args) {
  return new htile_lay_t(args);
}

htile_lay_t::htile_lay_t (std::vector< lay_t* > children) {
  init(htile_lay_class,children);
}

void htile_lay_t::open (void) {
  open_children();
}

void htile_lay_t::close (void) {
  close_children();
}

lay_t* htile_lay_t::exec (void) {
  exec_children();
  return this;
}

void htile_lay_t::render (bool is_picking, flo w, flo h) {
  int n = children.size();
  flo tot_fac = 0.0;
  eval_percentages();
  for (int i = 0; i < n; i++) {
    lay_t* child = children[i];
    flo now_fac = child->percent / 100.0;
    flo now_w = now_fac * w;
    glPushMatrix();
    glTranslatef(-w/2 + (tot_fac+now_fac/2)*w, 0, 0);
    // glTranslatef(MIN_X + (tot_fac+now_fac/2)*WIDTH, 0, 0);
    // glScalef(now_fac, 1, 1);
    // glPushMatrix(); glScalef(now_w/2, h/2, 1); draw_square(1); glPopMatrix();
    children[i]->render(is_picking, now_w, h);
    glPopMatrix();
    tot_fac += now_fac;
  }
}

//// PERCENT

obj_t* percent_lay_class;

percent_lay_t::percent_lay_t (obj_t* a) {
  init(percent_lay_class, tail(a));
  percent = num_val(nth(a, 0));
}

obj_t* percent_eval (env_t& env, obj_t* args) {
  return new percent_lay_t(args);
}

void percent_lay_t::open (void) {
  open_children();
}

void percent_lay_t::close (void) {
  close_children();
}

lay_t* percent_lay_t::exec (void) {
  exec_children();
  return this;
}

void percent_lay_t::render (bool is_picking, flo w, flo h) {
  render_children(is_picking, w, h);
}

//// TIMELINE

obj_t* timeline_lay_class;

static inline bool is_odd (int val) { return val&1; }

obj_t* eval_timeline_action_expr ( obj_t* arg );

obj_t* eval_timeline_time_expr ( obj_t* arg ) {
  if (obj_class(arg) == num_class) {
    return arg;
  } else if (obj_class(arg) == list_class) {
    char *name = sym_name(nth(arg, 0));
    if (strcasecmp(name, "hms") == 0) {
      extern int frame_rate;
      extern int speed;
      obj_t* res = new_num
	(num_val(eval_timeline_time_expr(nth(arg, 1)))*3600 +
	 num_val(eval_timeline_time_expr(nth(arg, 2)))*60 +
	 num_val(eval_timeline_time_expr(nth(arg, 3)))*1);
      return res;
		     
    } else if (strcasecmp(name, "in") == 0) {
      return cons(new_num(0),
             cons(eval_timeline_time_expr(nth(arg, 1)),
	     cons(eval_timeline_time_expr(nth(arg, 2)),
  	     list_len(arg) == 4 ? 
	     cons(eval_timeline_time_expr(nth(arg, 3)), lisp_nil) :
	     lisp_nil)));
    } else if (strcasecmp(name, "delta") == 0) {
      obj_t* start = eval_timeline_time_expr(nth(arg, 1)); 
      obj_t* delta = eval_timeline_time_expr(nth(arg, 2));
      return cons(new_num(0),
             cons(start,
	     cons(new_num(num_val(start) + num_val(delta)),
  	     list_len(arg) == 4 ? 
	     cons(eval_timeline_time_expr(nth(arg, 3)), lisp_nil) :
	     lisp_nil)));
    } else
      uerror("UNABLE TO FIND TIME EXPR %s\n", name);
  }
}

obj_t* expand_timeline_action_args ( obj_t* args ) {
  obj_t* res = lisp_nil;
  for (obj_t* a = args; a != lisp_nil; a = tail(a)) {
    res = cons(eval_timeline_action_expr(head(a)), res);
  }
  return list_rev(res);
}

obj_t* read_expand_qq (char* str, std::vector<obj_t*> env) {
  return eval_timeline_action_expr(read_qq(str, env));
        
}

static obj_t* unsupplied = NULL;

obj_t* eval_timeline_action_expr ( obj_t* arg ) {
  if (obj_class(arg) == num_class || obj_class(arg) == sym_class || obj_class(arg) == str_class) {
    return arg;
  } else if (obj_class(arg) == list_class) {
    obj_t* op = nth(arg, 0);
    obj_t* args = tail(arg);
    char* name = sym_name(op);
    if (strcasecmp(name, "servo") == 0 || 
        strcasecmp(name, "pow-servo") == 0 || 
        strcasecmp(name, "cycle") == 0) {
      return(cons(op, cons(unsupplied, expand_timeline_action_args(args))));
    } else if (strcasecmp(name, "servo-top-view") == 0) {
      return read_expand_qq("(all (servo view-target-x 0) (servo view-target-y 0) (servo view-target-z 0) (angle-servo view-angle 0) (servo view-z 6) (servo view-radius 0.01))", nul_env);
    } else if (strcasecmp(name, "set-top-view") == 0) {
      return read_expand_qq("(all (set view-target-x 0) (set view-target-y 0) (set view-target-z 0) (set view-angle 0) (set view-z 6) (set view-radius 0.01))", nul_env);
    } else if (strcasecmp(name, "servo-angle-view") == 0) {
      return read_expand_qq("(all (servo view-target-x 0) (servo view-target-y 0) (servo view-target-z 0) (angle-servo view-angle 0) (servo view-z 6) (servo view-radius 10))", nul_env);
    } else if (strcasecmp(name, "set-angle-view") == 0) {
      return read_expand_qq("(all (set view-target-x 0) (set view-target-y 0) (set view-target-z 0) (set view-angle 0) (set view-z 6) (set view-radius 10))", nul_env);
    } else if (strcasecmp(name, "servo-low-angle-view") == 0) {
      return read_expand_qq("(all (servo view-target-x 0) (servo view-target-y 0) (servo view-target-z -66) (angle-servo view-angle 0) (servo view-z 6) (servo view-radius 10))", nul_env);
    } else if (strcasecmp(name, "servo-bot-top-view") == 0) {
      env_t env
        = qq_env("$k", eval_timeline_action_expr(nth(args, 0)), NULL);
      post("EXPANDING SERVO_BOT_TOP_VIEW\n");
      return read_expand_qq("(finally (set view-target-bot $k) (all (dt-servo view-target-x (machine-x $k)) (dt-servo view-target-y (machine-y $k)) (dt-servo view-target-z (machine-z $k)) (angle-servo view-angle 0) (servo view-z 4) (servo view-radius 0.01)))", env);
    } else if (strcasecmp(name, "servo-bot-angle-view") == 0) {
      env_t env
        = qq_env("$k", eval_timeline_action_expr(nth(args, 0)), NULL);
      return read_expand_qq("(finally (all (set view-target-bot $k)) (all (dt-servo view-target-x (machine-x $k)) (dt-servo view-target-y (machine-y $k)) (dt-servo view-target-z (machine-z $k)) (servo view-z 4) (servo view-radius 10)))", env);
      // (angle-servo view-angle (machine-angle $k))
      // (set view-angle-bot $k)
    } else if (strcasecmp(name, "fade-out") == 0) {
      post("EXPANDING FADE OUT\n");
      env_t env
        = qq_env("$v", eval_timeline_action_expr(nth(args, 0)),
                 "$f", eval_timeline_action_expr(nth(args, 1)),
                 NULL);
      post("QQ ENVD\n");
      return read_expand_qq("(all (servo accum-value $v) (servo accum-fade $f))", env);
    } else if (strcasecmp(name, "servo-rect") == 0) {
      // post("EXPANDING SERVO RECT\n");
      env_t env
        = qq_env("$x", eval_timeline_action_expr(nth(args, 0)),
                 "$y", eval_timeline_action_expr(nth(args, 1)),
                 "$w", eval_timeline_action_expr(nth(args, 2)),
                 "$h", eval_timeline_action_expr(nth(args, 3)),
                 NULL);
      // post("QQ ENVD\n");
      return read_expand_qq("(all (servo rect_w $w) (servo rect_h $h) (servo pos_bx $x) (servo pos_by $y))", env);
    } else if (strcasecmp(name, "set-rect") == 0) {
      // post("EXPANDING SERVO RECT\n");
      env_t env
        = qq_env("$x", eval_timeline_action_expr(nth(args, 0)),
                 "$y", eval_timeline_action_expr(nth(args, 1)),
                 "$w", eval_timeline_action_expr(nth(args, 2)),
                 "$h", eval_timeline_action_expr(nth(args, 3)),
                 NULL);
      // post("QQ ENVD\n");
      return read_expand_qq("(all (set rect_w $w) (set rect_h $h) (set pos_bx $x) (set pos_by $y))", env);
    } else if (strcasecmp(name, "fade-in") == 0) {
      return read_expand_qq("(all (servo accum-value 1) (servo accum-fade 0))", nul_env);
    } else {
      return cons(op, expand_timeline_action_args(args));
    }
  } else {
    uerror("UNHANDLE TYPE\n");
  }
}

obj_t* timeline_eval (env_t& env, obj_t* args) {
  post("TIMELINE EVAL\n");
  return new timeline_lay_t(args);
}

timeline_lay_t::timeline_lay_t (obj_t* args) {
  obj_t* a = args;
  obj_t* res = lisp_nil;
  if (is_odd(list_len(a))) {
    res = cons(eval_timeline_time_expr(nth(a, 0)), res);
    a   = tail(a);
  }
  for (; a != lisp_nil; a = tail(tail(a))) {
    if (list_len(a) > 1) {
      obj_t* time   = nth(a, 0);
      obj_t* action = nth(a, 1);
      obj_t* acto   = eval_timeline_action_expr(action);
      obj_t* timo   = eval_timeline_time_expr(time);
      res = cons(timo, cons(acto, res));
    }
  }
  actions = res;
  init(timeline_lay_class, lisp_nil);
}

void timeline_lay_t::open (void) {
}

void timeline_lay_t::close (void) {
}

bool is_initially (double s, double e, double t, double dt ) {
  return s >= (t-dt) && s <= t;
}

bool is_finally (double s, double e, double t, double dt ) {
  return t >= e;
}

flo set_prop_val (prop_t *p, flo res) {
  extern flo set_zoom (flo val);
  if (strcasecmp(p->name, "zoom") == 0) {
    return set_zoom(res);
  } else
    return p->set.f(p, NULL, res);
  // return num_val_set(var, res);
}

flo get_prop_val (prop_t *p) {
  extern flo get_zoom ( void );
  if (strcasecmp(p->name, "zoom") == 0) {
    return get_zoom();
  } else
    return p->get.f(p, NULL);
}

env_t var_env;

prop_t* lookup_prop (char *name) {
  obj_t *var = env_lookup(var_env, name);
  return (prop_t*)var;
}

void add_var (char *name, prop_t *prop) {
  env_add(var_env, name, (obj_t*)prop);
}

void add_var (prop_t *prop) {
  add_var(prop->name, prop);
}

flo eta (obj_t *arg, double s, double e, double t, double dt ) {
  // TODO: FILL IN USING PROPS
  if (arg->type == num_class) {
    return num_val(arg);
  } else if (arg->type == sym_class) {
    return get_prop_val(lookup_prop(sym_name(arg)));
  } else if (arg->type == list_class) {
    char *name = sym_name(nth(arg, 0));
    if (strcasecmp(name, "all") == 0) {
      obj_t *a = tail(arg);
      // post("EVAL ALL\n");
      for (; a != lisp_nil; a = tail(a)) {
	if (list_len(a) > 0) 
	  eta(nth(a, 0), s, e, t, dt);
      }
    } else if (strcasecmp(name, "set") == 0) {
      char *name   = sym_name(nth(arg, 1));
      prop_t *prop = lookup_prop(name);
      if (prop == NULL)
        uerror("UNABLE TO FIND SYM %s\n", name);
      if (prop->type == bool_prop_class) {
        bool_prop_t *p = (bool_prop_t*)prop;
        obj_t *obj     = nth(arg, 2);
        p->set.s(p, obj, str_name(obj));
      } else if (prop->type == int_prop_class) {
        int_prop_t *p = (int_prop_t*)prop;
        obj_t *obj    = nth(arg, 2);
        p->set.i(p, obj, (int)num_val(obj));
      } else {
        flo val = eta(nth(arg, 2), s, e, t, dt);
        set_prop_val(prop, val);
      }
    } else if (strcasecmp(name, "servo") == 0) {
      if (list_len(arg) != 4)
        uerror("SERVO REQUIRES 3 ARGS GOT %d\n", list_len(arg)-1);
      obj_t  *num_from = nth(arg, 1);
      prop_t *prop     = lookup_prop(sym_name(nth(arg, 2)));
      flo from = num_val(num_from);
      if (num_from == unsupplied) {
        from = get_prop_val(prop);
        nth_set(arg, 1, new_num(from));
      }
      if (is_initially(s,e,t,dt)) {
        // if (var == view_angle)
        //   post("STARTING SERVO\n");
        set_prop_val(prop, from);
      }
      flo to   = eta(nth(arg, 3), s, e, t, dt);
      flo res  = is_finally(s, e, t, dt) ? to : from + ((t-s)/(e-s))*(to-from);
      flo val  = set_prop_val(prop, res);
      // if (strcasecmp(prop->name, "rect_w") == 0)
      //   post("TO %f FROM %f RES %f [%.2f,%.2f,%.2f]\n", to, from, res, s, e, t);
      // if (var == view_angle)
      //   post("DOING SERVO VL %f FR %f TO %f T %f S %f E %f\n", val, from, to, t, s, e);
        // post("SERVOING ANGLE %f -> %f\n", from, res);
      return val;
    } else if (strcasecmp(name, "pow-servo") == 0) {
      if (list_len(arg) != 4)
        uerror("SERVO REQUIRES 3 ARGS GOT %d\n", list_len(arg)-1);
      prop_t *prop     = lookup_prop(sym_name(nth(arg, 2)));
      obj_t  *num_from = nth(arg, 1);
      flo from = num_val(num_from);
      if (num_from == unsupplied) {
        from = get_prop_val(prop);
        nth_set(arg, 1, new_num(from));
      }
      if (is_initially(s,e,t,dt)) {
        set_prop_val(prop, from);
      }
      flo to   = eta(nth(arg, 3), s, e, t, dt);
      flo max  = std::max(from, to);
      flo min  = std::min(from, to);
      flo r    = log(max-min+1) / (e-s);
      flo tt   = from > to ? (e-t) : (t-s);
      flo res  = is_finally(s, e, t, dt) ? to : (exp(r*tt)-1)+min;
      flo val  = set_prop_val(prop, res);
      // post("DOING POW-SERVO VL %f FR %f TO %f T %f S %f E %f\n", res, from, to, t, s, e);
      return val;
    } else if (strcasecmp(name, "exit") == 0) {
      viz->close();
    } else if (strcasecmp(name, "set-base-time") == 0) {
      post("SETTING BASE TIME\n");
      set_base_time_at(0 - eta(nth(arg, 1),s,e,t,dt));
    } else {
      post("UNKNOWN COMMAND %s\n", name);
    }
  }
  return 0.0;
}

flo eval_timeline_action ( obj_t* a, double s, double e, double t, double dt ) {
  return eta(a, s, e, t, dt);
}

lay_t* timeline_lay_t::exec (void) {
  static double last_t = 0;
  obj_t* a      = actions;
  double t      = get_secs();
  double dt     = t - last_t;
  double offset = 0.0;
  if (is_odd(list_len(a))) {
    offset = num_val(nth(a, 0));
    a      = tail(a);
  }
  for (; a != lisp_nil; a = tail(tail(a))) {
    if (list_len(a) > 1) {
      obj_t* timeo   = nth(a, 0);
      obj_t* actiono = nth(a, 1);
      if (obj_class(timeo) == num_class) {
        // post_object("ACTION: ", actiono);
        double start = num_val(timeo) + offset;
	if (start >= (t-dt) && start <= t ) {
	  eval_timeline_action( actiono, start, start, t, dt );
	}
      } else if (obj_class(timeo) == list_class) {
	double start = num_val(nth(timeo, 1)) + offset;
	double end   = num_val(nth(timeo, 2)) + offset;
	double each  = list_len(timeo) == 4 ? num_val(nth(timeo, 3)) : -1;
	if (t >= start && (t-dt) <= end) {
          // post_object("ACTION: ", actiono);
          // post("START %f END %f OFFSET %f T %f LT %f DT %f => %d\n",
          //      start, end, offset, t, last_t, dt, t >= start && (t-dt) <= end);
          obj_t* beg = nth(timeo, 0);
          if (start >= (t-dt) && start <= t ) {
            // post("SETTING BEGINNING AT %f\n", start);
            num_val_set(beg, start);
          }
          start = num_val(beg);
          eval_timeline_action( actiono, start, end, t, dt );
	} 
      }
    }      
  }
  last_t = t;
  return this;
}

void timeline_lay_t::render (bool is_picking, flo w, flo h) {
}

//// GRID

obj_t* grid_lay_class;

#include <stdarg.h>

grid_lay_t::grid_lay_t (lay_t* elt, ...) {
  std::vector< lay_t* > cs;
  cs.push_back(elt);
  va_list ap;
  va_start(ap, elt);
  for (;;) {
    lay_t* e = va_arg(ap, lay_t*);
    if (elt == NULL)
      break;
    cs.push_back(e);
  }
  va_end(ap);
  init(grid_lay_class, cs);
}

grid_lay_t::grid_lay_t (std::vector< lay_t* > elts) {
  init(grid_lay_class, elts);
}

grid_lay_t::grid_lay_t (obj_t* elts) {
  init(grid_lay_class, elts);
}

obj_t* grid_eval (env_t& env, obj_t* args) {
  return new grid_lay_t(args);
}

void grid_lay_t::open (void) {
  open_children();
}

void grid_lay_t::close (void) {
  close_children();
}

lay_t* grid_lay_t::exec (void) {
  exec_children();
  return this;
}

void grid_lay_t::render (bool is_picking, flo w, flo h) {
  int n = children.size();
  int n_cols = (int)roundf(sqrtf((float)n));
  int n_rows = (int)ceilf((flo)n / n_cols);
  flo ew = w/n_cols;
  flo eh = h/n_rows;
  int k = 0;
  for (int i = 0; i < n_cols; i++) {
    for (int j = 0; j < n_rows; j++, k++) {
      if (k < n) {
        glPushMatrix();
        glTranslatef((i+0.5)*ew - w/2, (n_rows-j-0.5)*eh - h/2, 0);
        children[k]->render(is_picking, ew, eh);
        glPopMatrix();
      }
    }
  }
}

//// HSPLIT

obj_t* hsplit_lay_class;

hsplit_lay_t::hsplit_lay_t (std::vector< lay_t* > elts) {
  to_merge = NULL;
  parent = NULL;
  init(hsplit_lay_class, elts);
}

hsplit_lay_t::hsplit_lay_t (obj_t* elts) {
  to_merge = NULL;
  parent = NULL;
  init(hsplit_lay_class, elts);
}

obj_t* hsplit_eval (env_t& env, obj_t* args) {
  return new hsplit_lay_t(args);
}

lay_t* hsplit_lay_t::exec (void) {
  if (to_merge != NULL) {
    // TODO: CLOSE 
    post("HSPLIT MERGING\n");
    lay_t* res = to_merge;
    to_merge = false;
    res->parent = parent;
    return res;
  } else 
    return htile_lay_t::exec();
}

void hsplit_lay_t::open (void) {
  htile_lay_t::open();
}

void hsplit_lay_t::close (void) {
  htile_lay_t::close();
}

void hsplit_lay_t::render (bool is_picking, flo w, flo h) {
  htile_lay_t::render(is_picking, w, h);
}

//// STACK

obj_t* stack_lay_class;

stack_lay_t::stack_lay_t (lay_t* top) {
  parent = NULL;
  is_focus = false;
  std::vector< lay_t* > cs;
  cs.push_back(top);
  init(stack_lay_class, cs);
}

stack_lay_t::stack_lay_t (std::vector< lay_t* > cs) {
  parent = NULL;
  is_focus = false;
  init(stack_lay_class, cs);
}

stack_lay_t::stack_lay_t (obj_t* cs) {
  parent = NULL;
  is_focus = false;
  init(stack_lay_class, cs);
}

obj_t* stack_eval (env_t& env, obj_t* args) {
  return new stack_lay_t(args);
}

lay_t* stack_lay_t::clone (void) {
  return new stack_lay_t(children);
}

void stack_lay_t::push (lay_t* lay) {
  children.back()->close();
  lay->open();
  children.push_back(lay);
}

void stack_lay_t::pop (void) {
  if (children.size() > 1) {
    children.back()->close();
    delete children.back();
    children.pop_back();
    children.back()->open();
  }
}

void stack_lay_t::open (void) {
  to_push = NULL;
  is_pop = false;
  is_split = false;
  is_merge = false;
  children.back()->open();
}

void stack_lay_t::close (void) {
  children.back()->close();
}

lay_t* stack_lay_t::exec (void) {
  lay_t* res = this;
  if (to_push != NULL) {
    push(to_push);
    to_push = NULL;
  } else if (is_pop) {
    pop();
    is_pop = false;
  }
  children[children.size()-1] = children.back()->exec();
  if (is_split) {
    stack_lay_t* copy = (stack_lay_t*)(this->clone());
    lay_t* htile = new hsplit_lay_t(layvec(this, copy, NULL));
    htile->parent = parent;
    parent = copy->parent = htile;
    res = htile;
    post("CLOSING THIS\n");
    this->close();
    post("OPENING HTILE\n");
    htile->open();
    is_split = false;
    post("SPLIT %lx %lx %d\n", this, copy, htile->children.size());
  } else if (is_merge) {
    post("ASKING TO MERGE %lx %lx\n", this, parent);
    if (parent != NULL && parent->type == hsplit_lay_class) {
      ((hsplit_lay_t*)parent)->to_merge = this;
    }
    is_merge = false;
  } 
  return res;
}

void stack_lay_t::render (bool is_picking, flo w, flo h) {
  if (is_focus) {
  glPushAttrib(GL_CURRENT_BIT);
  glColor4f(1, 0, 0, 0.5);
  draw_rect(vec_t<2>(w, h));
  glPopAttrib();
  }
  children.back()->render(is_picking, w, h);
}

//// PROP

obj_t* prop_lay_class;

prop_lay_t::~prop_lay_t (void) {
  // TODO: probably want to do this automatically
  // but want to avoid double deletes and dangling references
  std::vector< lay_t* > names = tile->children[0]->children;
  std::vector< lay_t* > values = tile->children[1]->children;
  for (int i = 0; i < names.size(); i++)
    delete names[i];
  for (int i = 0; i < values.size(); i++)
    delete values[i];
  delete tile->children[0];
  delete tile->children[1];
  delete tile;
  delete highlight;
}

std::vector< lay_t* >& prop_lay_t::prop_lay_values (void) {
  std::vector< lay_t* > columns = tile->children;
  return columns[1]->children;
}

void prop_lay_t::highlight_prop_num (void) {
  std::vector< lay_t* >& columns = prop_lay_values();
  if (columns[prop_num] == highlight)
    post("TRYING TO HIGHLIGHT AN ALREADY HIGHLIGHTED PROP\n");
  else {
    highlight->children[0] = columns[prop_num];
    columns[prop_num] = highlight;
  }
}

void prop_lay_t::unhighlight_prop_num (void) {
  std::vector< lay_t* >& columns = prop_lay_values();
  lay_t* hl = columns[prop_num];
  if (hl != highlight)
    post("TRYING TO UNHIGHLIGHT A NON HIGHLIGHTED PROP\n");
  else
    columns[prop_num] = hl->children[0];
}

void prop_lay_t::prop_lay_init(std::vector<prop_t*>* props_) {
  prop_num = 0;
  type = prop_lay_class;
  props = *props_;
  std::vector< lay_t* > names;
  std::vector< lay_t* > values;
  for (int i = 0; i < props.size(); i++) {
    names.push_back(new txt_lay_t(props[i]->name, TD_RIGHT));
    values.push_back(new txt_lay_t("", TD_LEFT));
  }
  std::vector< lay_t* > vtiles;
  vtiles.push_back(new vtile_lay_t(names));
  vtiles.push_back(new vtile_lay_t(values));
  highlight = new rgb_lay_t(list(values[0], NULL));
  highlight->r = 1;
  highlight->g = 0;
  highlight->b = 0;
  highlight->a = 1;
  tile = new htile_lay_t(vtiles);
}

prop_lay_t::prop_lay_t(std::vector<prop_t*>* props_) {
  prop_lay_init(props_);
}

prop_lay_t::prop_lay_t(obj_t* props_) {
  std::vector<prop_t*> ps;
  for (obj_t* p = props_; p != lisp_nil; p = tail(p))
    ps.push_back((prop_t*)head(p));
  prop_lay_init(&ps);
}

obj_t* prop_eval (env_t& env, obj_t* args) {
  return new prop_lay_t(args);
}

void prop_lay_t::open (void) { 
  // create highlight
  // create layout
}

void prop_lay_t::close (void) { 
  // kill layout
  // kill highlight
}

lay_t* prop_lay_t::exec (void) { 
  std::vector< lay_t* >& values = prop_lay_values();
  prop_num = (prop_num + inc_prop_num + props.size()) % props.size();
  inc_prop_num = 0;
  for (int i = 0; i < props.size(); i++) {
    txt_lay_t* value = (txt_lay_t*)values[i];
    props[i]->post(value->txt);
  }
  return this;
}

void prop_lay_t::prop_num_set (int n) {
  prop_num = n;
}

void prop_lay_t::render (bool is_picking, flo w, flo h) { 
  highlight_prop_num();
  tile->render(is_picking, w, h);
  unhighlight_prop_num();
}

env_t lay_env;

static int is_init = 0;

void init_lay (void) {
  if (!is_init) {
    is_init = 1;
    lay_class = new_class("LAY", any_class);
    sim_lay_class = new_class("SIM-LAY", lay_class);
    tex_lay_class = new_class("TEX-LAY", lay_class);
    pic_lay_class = new_class("PIC-LAY", lay_class);
    env_add(lay_env, "PIC", new fun_t("PIC", pic_eval, params("child", NULL), 1));
    vid_lay_class = new_class("VID-LAY", lay_class);
    env_add(lay_env, "VID", new fun_t("VID", vid_eval, params("child", NULL), 1));
    txt_lay_class = new_class("TXT-LAY", lay_class);
    env_add(lay_env, "TXT", new fun_t("TXT", txt_eval, params("txt", "dir", "fuzz", NULL), 3));
    sys_lay_class = new_class("SYS-LAY", lay_class);
    env_add(lay_env, "SYS", new fun_t("SYS", sys_eval, params("cmd", NULL), 1));
    seq_lay_class = new_class("SEQ-LAY", lay_class);
    env_add(lay_env, "SEQ", new fun_t("SEQ", seq_eval, params("child", NULL), 0, 1));
    all_lay_class = new_class("ALL-LAY", lay_class);
    env_add(lay_env, "ALL", new fun_t("ALL", all_eval, params("child", NULL), 0, 1));
    rgb_lay_class = new_class("RGB-LAY", lay_class);
    env_add(lay_env, "RGB", new fun_t("RGB", rgb_eval, params("r", "g", "b", "a", "child", NULL), 5));
    mag_lay_class = new_class("MAG-LAY", lay_class);
    env_add(lay_env, "MAG", new fun_t("MAG", mag_eval, params("fac", "child", NULL), 2));
    mov_lay_class = new_class("MOV-LAY", lay_class);
    env_add(lay_env, "MOV", new fun_t("MOV", mov_eval, params("x", "y", "z", "child", NULL), 4));
    rot_lay_class = new_class("ROT-LAY", lay_class);
    env_add(lay_env, "ROT", new fun_t("ROT", rot_eval, params("amount", "child", NULL), 2));
    font_lay_class = new_class("FONT-LAY", lay_class);
    fil_lay_class = new_class("FIL-LAY", lay_class);
    env_add(lay_env, "FIL", new fun_t("FIL", fil_eval, params(NULL), 0));
    vtile_lay_class = new_class("VTILE-LAY", lay_class);
    env_add(lay_env, "VTILE", new fun_t("VTILE", vtile_eval, params("child", NULL), 0, 1));
    htile_lay_class = new_class("HTILE-LAY", lay_class);
    env_add(lay_env, "HTILE", new fun_t("HTILE", htile_eval, params("child", NULL), 0, 1));
    percent_lay_class = new_class("PERCENT-LAY", lay_class);
    env_add(lay_env, "PERCENT", new fun_t("PERCENT", percent_eval, params("percent", "child", NULL), 2));
    timeline_lay_class = new_class("TIMELINE-LAY", lay_class);
    env_add(lay_env, "TIMELINE", new fun_t("TIMELINE", timeline_eval, params("child", NULL), 1, 1));
    hsplit_lay_class = new_class("HSPLIT-LAY", lay_class);
    env_add(lay_env, "HSPLIT", new fun_t("HSPLIT", hsplit_eval, params("child", NULL), 0, 1));
    grid_lay_class = new_class("GRID-LAY", lay_class);
    env_add(lay_env, "GRID", new fun_t("grid", grid_eval, params("child", NULL), 0, 1));
    stack_lay_class = new_class("STACK-LAY", lay_class);
    env_add(lay_env, "STACK", new fun_t("STACK", stack_eval, params("child", NULL), 0, 1));
    prop_lay_class = new_class("PROP-LAY", lay_class);
    env_add(lay_env, "PROP", new fun_t("PROP", prop_eval, params("child", NULL), 0, 1));
    // defaults_lay_class = new_class("DEFAULTS-LAY", lay_class);
    add_var(zoom_var);
    unsupplied = new_num(0);
  }
}
