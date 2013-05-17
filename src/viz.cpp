//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "glut.h"
#include "time.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include "viz.h"
#include "cmds.h"
#include "keys.h"
#include <strings.h>
#include "trackball.h"
#include "utils.h"
#include "timer.h"

typedef flo NUM_VAL;
typedef int BOOL;

bool is_show_shadows = false;
bool is_quit = false;

viz_t *viz;

// int debug_id = -1;

inline int int_min (int x, int y) { return x < y ? x : y; }
inline int int_max (int x, int y) { return x < y ? y : x; }

void get_view_size ( double *width, double *height ) {
  *width  = viz->view_size.x;
  *height = viz->view_size.y;
}

void viz_t::determine_grid (int n, flo desired_aspect_ratio, vec_t<2> size, 
                            vec_t<2>& tot_size, vec_t<2>& pat_size, vec_t<2>& dims) {
  flo low_err = 1e8;
  int best_cols = n == 1 ? 1 : 1e8;
  int best_rows = n == 1 ? 1 : 1e8;
  // post("DAR %f \n", desired_aspect_ratio);
  for (int c = 1; c < n; c++) {
    int r = ceilf((flo)n / c);
    flo ar = (flo)r / c;
    flo err = fabs(ar - desired_aspect_ratio);
    // post("C %d R %d AR %f E %f\n", c, r, ar, err);
    if ((err < low_err && r < best_rows) || err == low_err && (c*r < best_cols*best_rows)) {
      best_cols = c;
      best_rows = r;
      low_err = err;
    }
  }
  dims.x = best_cols;
  dims.y = best_rows;
  pat_size.x = size.x / dims.x;
  pat_size.y = size.y / dims.x;
  tot_size.x = pat_size.x * dims.x;
  tot_size.y = pat_size.y * dims.y;
}

void viz_t::auto_zoom (vec_t<2> sdims, vec_t<2> dims, double mag, double& last_fac) {
  // get_bodies_extent_projected(mins, maxs);
  // post("MINS [%f,%f] MAXS [%f, %f]\n", mins.x, mins.y, maxs.x, maxs.y);
  double fac_x = dims.x / sdims.x;
  double fac_y = dims.y / sdims.y;
  double a = 0.01;
  double now_fac = 0.8 * mag * min(fac_x, fac_y);
  mag_fac = a * last_fac + (1-a)*now_fac;
  // post("FAC %f %f\n", fac, last_fac);
  last_fac = mag_fac;
  glScalef(mag_fac, mag_fac, mag_fac);
}

void viz_t::open_avi_out (char *filename) {
  if (!is_dumping) {
    post("OPENING AVI OUT\n");
    GLint view_port[4];
    glGetIntegerv(GL_VIEWPORT, view_port);
#ifdef IS_CV
    num_dumped_frames = 0;
    num_frames = 0;
    dump_image = 0;
    is_dumping_avi  = strstr(filename, ".avi") != NULL || strstr(filename, ".mov") != NULL;
    is_dumping_jpgs = strstr(filename, ".jpg") != NULL;
    if (is_dumping_avi) {
      // TODO: make this lazy
      GLint view_port[4];
      glGetIntegerv(GL_VIEWPORT, view_port);
      CvSize sz = cvSize(view_port[2], view_port[3]);
      post("CREATING VIDEO WRITER TO %s\n", filename);
      dumper = cvCreateVideoWriter( filename, fourcc, target_fps, sz, 1);
      post("DUMPER %p\n", dumper);
    } else if (is_dumping_jpgs) {
      char* slash = rindex(filename, '/');
      char dir[256];
      strncpy(dir, filename, slash - filename);
      dir[slash - filename] = 0;
      char cmd[256];
      sprintf(cmd, "mkdir -p %s", dir);
      post("CMD %s\n", cmd);
      system(cmd);
      sprintf(cmd, "rm -f %s/*.jpg", dir);
      post("CMD %s\n", cmd);
      system(cmd);
      int end_index = strstr(filename, ".jpg") - filename;
      strncpy(dump_filename, filename, end_index);
      dump_filename[end_index] = 0;
      post("dump_filename %s\n", dump_filename);
      // int end_index = strstr(filename, ".jpg") - filename;
      // strncpy(dump_filename, filename, end_index);
      // dump_filename[end_index] = 0;
      // post("dump_filename %s\n", dump_filename);
    }
    is_dumping = 1;
#endif
  }
}

void viz_t::toggle_full_screen (void) {
  is_full_screen = !is_full_screen;
  if (is_full_screen)
    glutFullScreen();
  else {
    glutPositionWindow(window_pos.x, window_pos.y);
    glutReshapeWindow(window_size.x, window_size.y);
  }
}

void viz_t::single_step (void) {
  is_stepping = 1;
  is_step = 1;
}

void viz_t::resume_execution (void) {
  is_stepping = 0;
  is_step = 0;
}

void viz_t::close_avi_out (void) {
  if (is_dumping) {
    is_dumping = 0;
#ifdef IS_CV
    if (is_dumping_avi) {
      cvReleaseVideoWriter( &dumper );
    }
    cvReleaseImage(&dump_image);
    post("CLOSING AVI\n");
#endif
    dumper = 0;
  }
}

#ifdef IS_CV
static void grab_frame (IplImage *image) {
  GLint view_port[4];
  GLint last_buffer;
  glGetIntegerv(GL_VIEWPORT, view_port);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ROW_LENGTH, 0);
  glPixelStorei(GL_PACK_SKIP_ROWS, 0);
  glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
  glGetIntegerv(GL_READ_BUFFER, &last_buffer);
  glReadBuffer(GL_FRONT);
  glReadPixels
    (0, 0, view_port[2], view_port[3], 
     GL_BGR_EXT, GL_UNSIGNED_BYTE, image->imageData);
  glReadBuffer(last_buffer);
  cvFlip(image, image, 0);
}
#endif

#include <time.h>

void viz_t::write_pic_out () {
#ifdef IS_CV
  if (!pic_image) {
    GLint view_port[4];
    glGetIntegerv(GL_VIEWPORT, view_port);
    CvSize sz = cvSize(view_port[2], view_port[3]);
    pic_image = cvCreateImage( sz, 8, 3 );
  }
  grab_frame(pic_image);
  char filename[256];
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  strftime(filename,sizeof(filename),"../dump/%Y-%m-%d-%H-%M-%S-pic.jpg", timeinfo);
  post("DUMP PIC FILE %s\n", filename);
  int res = cvSaveImage(filename, pic_image);
#endif
}

void viz_t::write_avi_out (void) {
  if (is_dumping) {
    if (num_frames % speed == 0) {
#ifdef IS_CV
      GLint view_port[4];
      glGetIntegerv(GL_VIEWPORT, view_port);
      CvSize sz = cvSize(view_port[2], view_port[3]);
      CvSize dsz = (dump_image != 0) ? cvGetSize(dump_image) : cvSize(0,0);
      if (dump_image != 0 && (dsz.width != sz.width || dsz.height != sz.height)) {
	cvReleaseImage(&dump_image);
	dump_image = 0;
      }
      if (dump_image == 0) {
	dump_image = cvCreateImage( sz, 8, 3 );
	post("IMAGE %p %lx\n", dump_image, fourcc);
      }
      grab_frame(dump_image);
      if (is_dumping_avi) {
	post("WRITING FRAME %d\n", num_dumped_frames);
	cvWriteToAVI(dumper, dump_image);
      } else if (is_dumping_jpgs) {
	char filename[256];
	sprintf(filename, "%s%04d.jpg", dump_filename, num_dumped_frames);
	int res = cvSaveImage(filename, dump_image);
      }
      num_dumped_frames += 1;
#endif
    }
    num_frames += 1;
    // post("%d\n", num_frames);
    if (num_frames > 0 && (num_frames % (int)(target_fps*speed)) == 0) {
      int secs = num_frames / (target_fps*speed);
      post("T=%02d:%02d\n", secs / 60, secs % 60);
    }
  }
}

void viz_t::start_view3d (int is_picking, int x, int y) {
  glMatrixMode(GL_PROJECTION);
  // glPushMatrix();
  // if (is_picking) glPushMatrix();
  glLoadIdentity();
  if (is_picking) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3]-y), 10.0, 10.0, viewport);
  }
  // using the viewport's aspect ratio means there's no X/Y distortion
  gluPerspective(45.0f, aspect_ratio, 1.0f, 50000.0f); 
  // We want to pull back until the XY plane contains all the 
  // agents (plus a little extra space)
  flo y_angle = M_PI/8; // half of 45 degrees, in radians
  flo fit_zy = (get_height()/2) / tan(y_angle);
  flo x_angle = atan(tan(y_angle)*aspect_ratio);
  flo fit_zx = (get_width()/2) / tan(x_angle);
  // post("ZX=%f, ZY=%f, AR=%f \n", fit_zx, fit_zy, aspect_ratio);
  fit_z = MAX(fit_zx,fit_zy)*1.05;
  glTranslatef( 0, 0, -fit_z); // -125
  // reset_view();  // reset the trackball
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  load_trackball_transformation( );
  load_trackball_zoom_translate( );
}

void viz_t::end_view3d (void) {
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

static void collect_hits (GLint hits, GLuint buffer[], std::vector< int >& picks) {
  GLuint names, *ptr;
  ptr = (GLuint *)buffer;
  for (int i = 0; i < hits; i++) {
    names = *ptr++;
    ptr++; ptr++;
    if (i < MAX_PICKS)
      picks.push_back(*ptr);
    for (int j = 0; j < names; j++)
      ptr++;
  }
}

void viz_t::pick_nodes ( int x, int y, bool is_add ) {
  GLuint select_buf[MAX_PICKS];
  GLint hits;
  GLint viewport[4];
  glSelectBuffer(MAX_PICKS, select_buf); // TODO: N
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(-1);

  start_view3d(1, x, y);
  render(1);
  end_view3d();

  hits = glRenderMode(GL_RENDER);
  std::vector< int > picks;
  collect_hits(hits, select_buf, picks);
  process_picks(picks, is_add);
  glPopName();
}

static char *dup_txt (char *txt) {
  char *str = (char*)malloc(strlen(txt)+1);
  strcpy(str, txt);
  return str;
}

void viz_t::resize (GLsizei new_width, GLsizei new_height) {
  shadow_height = new_height;
  shadow_width  = new_width;
  view_size = vec_t<2>(new_width, new_height);
  glViewport(0, 0, view_size.x, view_size.y);
  aspect_ratio = view_size.x/view_size.y;
  scale = vec_t<2>(new_width / get_width(), new_height / get_height());
  // regenerate_shadow_map();
}

static void c_resize (GLsizei new_width, GLsizei new_height) {
  viz->resize(new_width, new_height);
}

vec_t<2> viz_t::get_mouse_pos ( void ) {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  return vec( mouse_pos.x * (get_width()/viewport[2]) - get_width()/2, 
              (viewport[3]-mouse_pos.y)*(get_height()/viewport[3]) - get_height()/2 );
}

void viz_t::on_passive_mouse_motion( int x, int y ) {
  mouse_pos = vec_t<2>(x, y);
}

static void c_on_passive_mouse_motion( int x, int y ) {
  viz->on_passive_mouse_motion(x, y);
}
void viz_t::on_start_drag(int x, int y){
  drag_old_pos = vec_t<2>(x, y);
  dragging_machine = 1;
  glutSetCursor(GLUT_CURSOR_NONE);
}

void viz_t::on_end_drag(void){
  if (dragging_machine) {
    dragging_machine = 0;
    glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
  }
}

void viz_t::base_handle_drag(int x, int y) {
  if (dragging_machine && picks.size() > 0) {
    handle_drag(vec_t<2>(x, y) - drag_old_pos);
    drag_old_pos = vec_t<2>(x, y);
  }
}

void c_handle_drag(int x, int y) {
  viz->base_handle_drag(x, y);
}

static void print_node (int id) {
  post("ID %d\n", id);
}

void viz_t::on_button_press ( int button, int state, int x, int y ) {
  but_modifiers = glutGetModifiers();
  int shift = (but_modifiers & GLUT_ACTIVE_SHIFT) != 0;
  int control = (but_modifiers & GLUT_ACTIVE_CTRL) != 0;
  // int is_right_click = button == GLUT_LEFT_BUTTON && control;
  int is_right_click = button == GLUT_RIGHT_BUTTON;
  if (state == GLUT_DOWN /* && mouse_button==-1 */) { // no concurrent button ops
    mouse_button = button;
    if (is_right_click) {
      click=1; // we don't yet know whether it's a click or a drag...
    } else if (button == GLUT_LEFT_BUTTON && shift) {
      on_start_drag(x, y); // move machine
    } else if (button == GLUT_LEFT_BUTTON) { // not-shift
      click=1; // we don't yet know whether it's a click or a drag...
    }
  }

  if (state == GLUT_UP) {
    mouse_button = -1;
    // printf("STATE UP SHIFT %d CTRL LEFT %d CLICK %d\n", shift, control, button == GLUT_LEFT_BUTTON, click);
    if (is_right_click && click) {
      pick_nodes(x,y, shift); print_node(picked); 
      click=0;
    } else if (is_right_click) {
      on_right_button_up(x, y);
    } else if (button == GLUT_LEFT_BUTTON && click) {
      pick_nodes(x, y, control); 
      click=0;
    } else if (button == GLUT_LEFT_BUTTON) { // drag
      if (dragging_machine)
	on_end_drag();
      // else
	// on_left_button_up(x, y);
    }
  }
  glutPostRedisplay();
}

void c_on_button_press ( int button, int state, int x, int y ) {
  viz->on_button_press(button, state, x, y);
}

void viz_t::on_mouse_motion( int x, int y ) {
  const int mouse_fac = 3000;
  int shift = (but_modifiers & GLUT_ACTIVE_SHIFT) != 0;
  int control = (but_modifiers & GLUT_ACTIVE_CTRL) != 0;
  // int is_right_click = mouse_button == GLUT_LEFT_BUTTON && control;
  int is_right_click = mouse_button == GLUT_RIGHT_BUTTON;
  if (click) {
    if (MAX(abs(x-mouse_pos.x),abs(y-mouse_pos.y)) > click_fuzz) {
      click=0;
      // start point is old position
      if (is_right_click) {
        on_right_button_down(mouse_pos.x,mouse_pos.y);  // Trackball zoom
      } else if (mouse_button == GLUT_LEFT_BUTTON) {
	// on_joy_move(mouse_fac * (x-mouse_x), mouse_fac * (y-mouse_y));
        on_left_button_down(mouse_pos.x, mouse_pos.y);  // Trackball rotate
      }
    } 
  }
  if (!click) {
    mouse_pos = vec_t<2>(x, y);
    on_mouse_move(x, y);
    base_handle_drag(x, y);
  }
  glutPostRedisplay();
}

void c_on_mouse_motion( int x, int y ) {
  viz->on_mouse_motion(x,y);
}

void viz_t::special_handler( int key, int x, int y ) {
  key_modifiers = glutGetModifiers();
  int shift = (key_modifiers & GLUT_ACTIVE_SHIFT);
  switch (key) {
  case GLUT_KEY_RIGHT: on_joy_move(-MOVE_AMOUNT, 0); break;
  case GLUT_KEY_LEFT: on_joy_move(MOVE_AMOUNT, 0); break;
  case GLUT_KEY_UP: on_joy_move(0, MOVE_AMOUNT); break;
  case GLUT_KEY_DOWN: on_joy_move(0, -MOVE_AMOUNT); break;
  }
  glutPostRedisplay();
}

static void c_special_handler( int key, int x, int y ) {
  viz->special_handler(key, x, y);
}

void viz_t::base_keyboard_handler( unsigned char key, int x, int y ) {
  is_key_hit[key] = 1;
  key_modifiers   = glutGetModifiers();
  switch (key) {
  case 's':
    is_stepping = 1;
    is_step = 1;
    break;
  case 'x':
    is_stepping = 0;
    is_step = 0;
    break;
  case '_':
    write_pic_out();
    break;
  case 'f':
    is_full_screen = !is_full_screen;
    if (is_full_screen)
      glutFullScreen();
    else {
      glutPositionWindow(window_pos.x, window_pos.y);
      glutReshapeWindow(window_size.x, window_size.y);
    }
    break;
    // case ' ':
  case '>':
    seq_inc = 1;
    break;
  case '':
  case '<':
    seq_inc = -1;
    break;
    // case 'z': 
    // reset_view(); 
    // break;
    // case 'z': zoom_out(2.0); break;
    // case 'Z': zoom_in(2.0); break;
    // case '+': vals[0] = int_min(vals[0] + 1, 127); break;
    // case '-': vals[0] = int_max(vals[0] - 1, 0); break;
    //   case '=':
    //     if (is_dumping) {
    //       close_avi_out();
    //     } else {
    //       open_avi_out("./out.avi");
    //     }
    //     break;
  case 'q': is_quit = true; break;

  default: key_hit(key, key_modifiers); break;
  }
  glutPostRedisplay();
}

void viz_t::base_keyboard_up_handler( unsigned char key, int x, int y ) {
  is_key_hit[key] = 0;
  key_release(key, key_modifiers); 
}

static void c_keyboard_handler( unsigned char key, int x, int y ) {
  viz->base_keyboard_handler(key, x, y);
}

static void c_keyboard_up_handler( unsigned char key, int x, int y ) {
  viz->base_keyboard_up_handler(key, x, y);
}

static flo glutStrokeHeight (void *font) {
  return 119.05;
}

double stroke_text_width (void* font, char* text) {
  double l = 0.0;
  for (int i=0; i<strlen(text); i++)
    l += glutStrokeWidth(font, text[i]);
  return l;
}

void draw_rect (vec_t<2> size) {
  flo w = size.x, h = size.y;
  glBegin(GL_LINE_LOOP);
  glVertex2f(-w/2, h/2); glVertex2f( w/2, h/2); 
  glVertex2f( w/2,-h/2); glVertex2f(-w/2,-h/2);
  glEnd();
}

void viz_t::prepare_view (void) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-get_width()/2,get_width()/2,-get_height()/2,get_height()/2);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
}

typedef float M3DMatrix44f[16];
M3DMatrix44f textureMatrix;
// GLfloat texture_matrix[16];
GLfloat factor = 4.0f; // for polygon offset
GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f};
GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f};
GLfloat noLight[]      = { 0.0f, 0.0f, 0.0f, 1.0f};
// GLfloat lightPos[]     = { 100.0f, 300.0f, 100.0f, 1.0f};
// GLfloat lightPos[]     = { 10.0f, 30.0f, 10.0f, 1.0f};
// GLfloat lightPos[]     = { 2.0f, 6.0f, 2.0f, 1.0f};
GLfloat lightPos[]     = { 2.0f, 8.0f, -2.0f, 1.0f};
GLfloat lightAt[]     = { 0.0f, 0.0f, 0.0f };

GLfloat cameraPos[]    = { 100.0f, 150.0f, 200.0f, 1.0f};
GLdouble cameraZoom = 0.3;

void m3dLoadIdentity44(M3DMatrix44f m) {
  // Don't be fooled, this is still column major
  static M3DMatrix44f	identity = { 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f };
  
  memcpy(m, identity, sizeof(M3DMatrix44f));
}

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

void m3dMatrixMultiply44(M3DMatrix44f product, const M3DMatrix44f a, const M3DMatrix44f b ) {
  for (int i = 0; i < 4; i++) {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
}

inline void m3dTranslateMatrix44(M3DMatrix44f m, float x, float y, float z) { 
  m[12] += x; m[13] += y; m[14] += z; 
}

inline void m3dScaleMatrix44(M3DMatrix44f m, float x, float y, float z) { 
  m[0] *= x; m[5] *= y; m[10] *= z; 
}

// Transpose/Invert - Only 4x4 matricies supported
#define TRANSPOSE44(dst, src)            \
{                                        \
    for (int j = 0; j < 4; j++)          \
    {                                    \
        for (int i = 0; i < 4; i++)      \
        {                                \
            dst[(j*4)+i] = src[(i*4)+j]; \
        }                                \
    }                                    \
}
inline void m3dTransposeMatrix44(M3DMatrix44f dst, const M3DMatrix44f src) { 
  TRANSPOSE44(dst, src); 
}

/*
Imath::M44d glToM44d (GLfloat* mat) {
  return Imath::M44d(mat[0], mat[1], mat[2], mat[3], 
                     mat[4], mat[5], mat[6], mat[7], 
                     mat[8], mat[9], mat[10], mat[11], 
                     mat[12], mat[13], mat[14], mat[15]);
}

GLfloat* m44dToGL (GLfloat *res, Imath::M44d mat) {
  for (int i = 0; i < 16; i++)
    res[i] = *mat[i];
  return res;
}
*/

void viz_t::set_light_look_at (vec_t<3> pos) {
  lightAt[0] = pos.x;
  lightAt[1] = pos.y;
  lightAt[2] = pos.z;
}

// Called to regenerate the shadow map
void viz_t::regenerate_shadow_map( void ) {
  GLfloat lightToSceneDistance, nearPlane, fieldOfView;
  GLfloat lightModelview[16], lightProjection[16];
  GLfloat sceneBoundingRadius = 10.0f; // based on objects in scene

  // Save the depth precision for where it's useful
  lightToSceneDistance = sqrt(lightPos[0] * lightPos[0] + 
                              lightPos[1] * lightPos[1] + 
                              lightPos[2] * lightPos[2]);
  nearPlane = lightToSceneDistance - sceneBoundingRadius;
  // Keep the scene filling the depth texture
  fieldOfView = (GLfloat)((180.0 / M_PI) *2.0f * atan(sceneBoundingRadius / lightToSceneDistance));

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fieldOfView, 1.0f, nearPlane, nearPlane + (2.0f * sceneBoundingRadius));
  glGetFloatv(GL_PROJECTION_MATRIX, lightProjection);
  // Switch to light's point of view
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(lightPos[0], lightPos[1], lightPos[2], 
            lightAt[0], lightAt[1], lightAt[2], 0.0f, 1.0f, 0.0f);
  glGetFloatv(GL_MODELVIEW_MATRIX, lightModelview);
  glViewport(0, 0, shadow_width, shadow_height);

  // Clear the depth buffer only
  glClear(GL_DEPTH_BUFFER_BIT);

  // All we care about here is resulting depth values
  glShadeModel(GL_FLAT);
  glDisable(GL_LIGHTING);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_NORMALIZE);
  glColorMask(0, 0, 0, 0);

  // Overcome imprecision
  glEnable(GL_POLYGON_OFFSET_FILL);

  // Draw objects in the scene except base plane
  // which never shadows anything
  render(0);

  // Copy depth values into depth texture
  glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                   0, 0, shadow_width, shadow_height, 0);

  // Restore normal drawing state
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);
  glColorMask(1, 1, 1, 1);
  glDisable(GL_POLYGON_OFFSET_FILL);

  // Set up texture matrix for shadow map projection,
  // which will be rolled into the eye linear
  // texture coordinate generation plane equations

  M3DMatrix44f tempMatrix;
  m3dLoadIdentity44(tempMatrix);
  m3dTranslateMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
  m3dScaleMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
  m3dMatrixMultiply44(textureMatrix, tempMatrix, lightProjection);
  m3dMatrixMultiply44(tempMatrix, textureMatrix, lightModelview);
  // transpose to get the s, t, r, and q rows for plane equations
  m3dTransposeMatrix44(textureMatrix, tempMatrix);

  /*
  Imath::M44d temp_matrix;
  Imath::M44d tex_matrix;
  temp_matrix.makeIdentity();
  temp_matrix.translate(Imath::V3d(0.5f, 0.5f, 0.5f));
  temp_matrix.scale(Imath::V3d(0.5f, 0.5f, 0.5f));
  tex_matrix = temp_matrix * glToM44d(lightProjection);
  temp_matrix = tex_matrix * glToM44d(lightModelview);
  // transpose to get the s, t, r, and q rows for plane equations
  m44dToGL(texture_matrix, temp_matrix.transpose());
  */
}

// This function does any needed initialization on the rendering
// context. 
void viz_t::setup_rc (void) {
  // post("SETTING SHADOWS\n");
  // Make sure required functionality is available!
  /*
  if (!GLEE_VERSION_1_4 && !GLEE_ARB_shadow) {
    fprintf(stderr, "Neither OpenGL 1.4 nor GL_ARB_shadow"
            " extension is available!\n");
    exit(0);
  }
  */

  // Check for optional extensions
  // if (GLEE_ARB_shadow_ambient) {
    is_ambient_shadow_available = GL_TRUE;
    // } else {
    //   fprintf(stderr, "GL_ARB_shadow_ambient extension not available!\n");
    //   fprintf(stderr, "Extra ambient rendering pass will be required.\n\n");
    // }

    // if (GLEE_VERSION_2_0 || GLEE_ARB_texture_non_power_of_two) {
    is_npot_textures_available = GL_TRUE;
    // } else {
    //   fprintf(stderr, "Neither OpenGL 2.0 nor GL_ARB_texture_non_power_of_two extension\n");
    //   fprintf(stderr, "is available!  Shadow map will be lower resolution (lower quality).\n\n");
    // }

  // Black background
    if (is_white_background) {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
    } else
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

  // Hidden surface removal
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  if (is_shadowing)
    glPolygonOffset(factor, 4.0f);

  // Set up some lighting state that never changes
  if (is_lighting) {
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHT0);
  }

  if (is_shadowing) {
    // Set up some texture state that never changes
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
    glGenTextures(1, &shadow_texture_id);
    glBindTexture(GL_TEXTURE_2D, shadow_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    if (is_ambient_shadow_available)
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.5f);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    // regenerate_shadow_map();
  }
}

void viz_t::base_render (void) {
  if (is_process_frame) {
    // if (is_shadowing)
    //   regenerate_shadow_map();
    double tval = set_secs();
    dt = tval - last_tval;
    if (dt == 0)
      tfps = 1;
    else
      tfps = 1 / dt;
    fps       = (1-tdecay)*tfps + tdecay*fps;
    // post("TVAL %f LTVAL %f D %.2f FPS %.2f\n", tval, last_tval, tval-last_tval, (flo)tfps);
    last_tval = tval;
    /*
    double tmp_tval, pdelay, delay;
    tmp_tval  = set_secs();
    pdelay    = (tmp_tval - last_tval);
    delay     = pdelay + rem;
    tfps      = 1 / delay;

    post("PD %.2f TVAL %f LTVAL %f  REM %.2f D %.2f FPS %.2f\n", 
         (flo)pdelay, tmp_tval, last_tval, (flo)rem, (flo)delay, (flo)tfps);

    if (rem == INIT_REM)
      rem = 0.0;
    else
      rem = delay - 1 / target_fps;
    */

    prepare_view();
    start_view3d(0, 0, 0);

    if (is_lighting) {
      // Track light position
      glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

      glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    }

    if (is_shadowing) {
      // Set up shadow comparison
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

      // Set up the eye plane for projecting the shadow map on the scene
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);
      glEnable(GL_TEXTURE_GEN_Q);

      glTexGenfv(GL_S, GL_EYE_PLANE, &textureMatrix[0]);
      glTexGenfv(GL_T, GL_EYE_PLANE, &textureMatrix[4]);
      glTexGenfv(GL_R, GL_EYE_PLANE, &textureMatrix[8]);
      glTexGenfv(GL_Q, GL_EYE_PLANE, &textureMatrix[12]);

      /*
        glTexGenfv(GL_S, GL_EYE_PLANE, &texture_matrix[0]);
        glTexGenfv(GL_T, GL_EYE_PLANE, &texture_matrix[4]);
        glTexGenfv(GL_R, GL_EYE_PLANE, &texture_matrix[8]);
        glTexGenfv(GL_Q, GL_EYE_PLANE, &texture_matrix[12]);
      */
    }

    // glTranslatef(0.0, 0.0, -6.0);
    int is_rendered = render(0);

    if (is_lighting && is_show_lights) {
      glPushMatrix();
      glTranslatef(-lightPos[0], -lightPos[1], -lightPos[2]);
      glColor3f(1,1,1);
      glutSolidCone(0.1, 0.2, 15, 15);
      glPopMatrix();
    }

    if (is_show_shadows) {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glLoadIdentity();
      glEnable(GL_TEXTURE_2D);
      glDisable(GL_LIGHTING);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
      // Show the shadowMap at its actual size relative to window
      glColor4f(1, 1, 1, 0.5);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(-1.0f, -1.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(((GLfloat)shadow_width/(GLfloat)view_size.x)*2.0f-1.0f, 
                 -1.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(((GLfloat)shadow_width/(GLfloat)view_size.x)*2.0f-1.0f, 
                 ((GLfloat)shadow_height/(GLfloat)view_size.y)*2.0f-1.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(-1.0f, 
                 ((GLfloat)shadow_height/(GLfloat)view_size.y)*2.0f-1.0f);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      gluPerspective(45.0f, 1.0f, 1.0f, 1000.0f);
      glMatrixMode(GL_MODELVIEW);
    }
    if (is_shadowing) {
      glDisable(GL_ALPHA_TEST);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glDisable(GL_TEXTURE_GEN_R);
      glDisable(GL_TEXTURE_GEN_Q);
    }

    end_view3d();

    render_frame_monitors();
    glutSwapBuffers();
    if (is_rendered)
      write_avi_out();
  }
}

static void c_render (void) {
  viz->base_render();
}

void viz_t::idle (void) {
  if (is_quit) {
#ifdef IS_AUDIO
    audio_stop();
    Pa_Terminate();
    // audio_terminate();
#endif
    if (is_full_screen) toggle_full_screen();
    glutDestroyWindow(1);
    viz->close(); 
    exit(0); 
  }
  double delay;
  double time = set_secs();
  delay     = (time - last_tval);
  tfps      = 1 / delay;
  double target_delay = 1 / target_fps;
  if (target_fps != -1 && last_tval != -1) {
    if (target_delay > delay) {
      usleep((target_delay - delay)*1000000);
      // post("%d\n", (int)((target_delay - delay)*1000000));
    }
  }

  // last_tval = set_secs();
  // elapsed_time += 1 / real_fps;

  if (is_stepping) {
    if (is_step) {
      is_step = 0;
    } else { 
      usleep(100000);
      return;
    }
  }

  if (!is_audio) {
    // post("EXEC %d %d\n", is_stepping, is_step);
    exec(0);
    ticks += 1;
    // sim_dump(sim, sim->t);
  }
  glutPostRedisplay();

}

static void c_idle (void) {
  viz->idle();
}

#ifdef IS_AUDIO

#define TABLE_SIZE   (200)
typedef struct {
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
} paTestData;

static paTestData test_data;

#define NUM_SECONDS (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

const double SAMPLE_PERIOD = 1.0 / SAMPLE_RATE;

int viz_t::audio_callback
    (const void *input, void *output, unsigned long n_frames, 
     const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status, void* user) {
  if (is_enable_audio) {
  flo *istream = (flo*)input;
  flo *ostream = (flo*)output;
  n_aud_frames = n_frames;
  audio_tick   = time_info->outputBufferDacTime;
  // printf("AUDIO_TICK %f\n", audio_tick);
  if (is_playing_back)
    post("OVERRUN ERROR\n");
  sys_pollmidiqueue();
  is_playing_back = 1;
  for (int j=0; j<N_OUTPUT_CHANNELS; j++) 
    for (int i=0; i<N_DATA; i++) 
      out_samples[j][i] = 0;
  if (istream != NULL) {
    for (int i=0; i<n_frames; i++)
      for (int j=0; j<N_INPUT_CHANNELS; j++) 
	in_samples[j][i] = (Aud)(*istream++);
  }
  // for (int i = 0; i < n_sliders; i++)
  //   if (sliders[i] != NULL) {
  //     // post("VAL %f\n", vals[0] / 127.0);
  //     sliders[i]->val = float_to_fixpoint(vals[i] / 127.0, sliders[i]->width);
  // }
  for (int i = 0; i < n_frames; i++) {
    // if (mic != NULL)
    //   mic->val = in_samples[0][i] * 32767;
    exec(0);
    if (speakers != NULL) {
      for (int j = 0; j < N_OUTPUT_CHANNELS; j++) {
        out_samples[j][i] = speakers[j];
        // printf("OUT %d,%d = %f\n", i, j, out_samples[j][i]);
      }
      //     // sim_store(sim, time->val);
      //     // node_val_of(time, time->val+1);
      audio_tick += SAMPLE_PERIOD;
      // printf("AT%d %f SP = %f\n", i, audio_tick, SAMPLE_PERIOD);
    }
    /*
    *ostream++ = test_data.sine[test_data.left_phase];  // left 
    *ostream++ = test_data.sine[test_data.right_phase]; // right
    test_data.left_phase += 1;
    if( test_data.left_phase >= TABLE_SIZE ) test_data.left_phase -= TABLE_SIZE;
    test_data.right_phase += 3; // higher pitch so we can distinguish left and right.
    if( test_data.right_phase >= TABLE_SIZE ) test_data.right_phase -= TABLE_SIZE;
    */
    for (int j=0; j<N_OUTPUT_CHANNELS; j++) 
      *ostream++ = out_samples[j][i];
  }
  is_playing_back = 0;
  // sim_dump(sim, t++);
  // handle_step_cmds();
  }
  return 0;
}

int sys_audio_callback
    (const void *input, void *output, unsigned long n_frames, 
     const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status, void* user) {
  viz_t* viz = (viz_t*)user;
  return viz->audio_callback(input, output, n_frames, time_info, status, user);
}

static void StreamFinished( void* userData )
{
   paTestData *data = (paTestData *) userData;
   printf( "Stream Completed: %s\n", data->message );
}

void viz_t::audio_init (void) {
  if (is_audio) {
    int err;
    char devname[100];
    speakers = (flo*)MALLOC(N_OUTPUT_CHANNELS*sizeof(flo));
    in_samples = (flo**)MALLOC(N_INPUT_CHANNELS*sizeof(flo*));
    for (int i = 0; i < N_INPUT_CHANNELS; i++) 
      in_samples[i]  = (flo*)MALLOC(N_DATA*sizeof(flo));
    out_samples = (flo**)MALLOC(N_OUTPUT_CHANNELS*sizeof(flo*));
    for (int i = 0; i < N_OUTPUT_CHANNELS; i++) 
      out_samples[i] = (flo*)MALLOC(N_DATA*sizeof(flo));

    // initialise sinusoidal wavetable 
    for(int i=0; i<TABLE_SIZE; i++ ) {
      test_data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. ) * 0.5;
    }
    test_data.left_phase = test_data.right_phase = 0;

    audio_initialize();

    sprintf(devname, "/dev/dsp");
    audio_open_stream(devname, &playback_stream, FRAMES_PER_BUFFER, sys_audio_callback, (void*)this);
    for (size_t i = 0; i < N_SLIDERS; i++)
      sliders.push_back(0.0);
  }
}

void viz_t::audio_start (void) {
  sys_init_midi();
  for (int i = 0; i < 128; i++)
    sys_putmidimess(3, 176 | 0, i, 0);
  audio_start_stream(playback_stream);
  audio_dispatch(playback_stream, is_graphics);
}

void viz_t::audio_stop (void) {
  audio_stop_stream(playback_stream);
  audio_close_stream(playback_stream);
}

#include "port_audio.h"
#include "port_midi.h"
#include "porttime.h"

void inmidi_misc(int portno, int channel, int dat1, int dat2) {
  if (portno == 176 && channel == 0) {
    int n = dat1;
    if (n >= 0 && n < N_SLIDERS) {
      viz->sliders[n] = dat2 / 127.0;
    }
    post("KNOB %d %d %d %d\n", portno, channel, dat1, dat2);
    // SETFXNUM(knob_scalars[dat1]->dst, dat2 / 127.0);
  }
}

void inmidi_noteon(int portno, int channel, int pitch, int velo) {
  if (velo > 0)
    post("NOTEON %d %d %d %d\n", portno, channel, pitch, velo);
}

void inmidi_control(int portno, int channel, int num, int val) {
  post("CONTROL %d %d %d\n", channel, num, val);
}

void pm_set_close_function(void *ptr) {
}

#else

void viz_t::audio_init (void) { }
void viz_t::audio_start (void) { }
void viz_t::audio_stop (void) { }

#endif

static const char *check_cmd_key_value (int i, int argc, const char *argv[]) {
  if (i >= argc) uerror("BAD CMD ARGS MISSING KEY VALUE %s\n", argv[i-1]);
  return argv[i];
}

int viz_t::parse_args (int offset, int argc, const char *argv[]) {
  cmd.clear();
  cmd.assign(argv[0]);
  for (int i = 1; i < argc; i++) {
    cmd = cmd + " " + argv[i];
  }
  int i;
  for (i = offset; i < argc; ) {
    const char *arg = argv[i++];
    
    // post("ARG %s\n", arg);
    if (strcmp(arg, "-step") == 0) {
      post("STEPPING\n");
      is_step = 0;
      is_stepping = 1;
    } else if (strcmp(arg, "-is-audio") == 0) { 
      is_audio = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-is-lighting") == 0) { 
      is_lighting = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-is-shadowing") == 0) { 
      is_shadowing = atoi(check_cmd_key_value(i++, argc, argv));
      is_lighting = is_shadowing;
    } else if (strcmp(arg, "-is-graphics") == 0) { 
      is_graphics = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-is-white-background") == 0) { 
      is_white_background = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-n-steps") == 0) { 
      n_steps = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-dump") == 0) { 
      speed = 1;
      target_fps = 15;
      is_offline = 1;
      strcpy(dump_filename, check_cmd_key_value(i++, argc, argv));
      post("DUMP ARG %s\n", dump_filename);
    } else if (strcmp(arg, "-speed") == 0) { 
      speed = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-1024") == 0) { 
      window_size = vec(1024, 768);
    } else if (strcmp(arg, "-160") == 0) { 
      window_size = vec(160, 120);
    } else if (strcmp(arg, "-320") == 0) { 
      window_size = vec(320, 240);
    } else if (strcmp(arg, "-50") == 0) { 
      window_size = vec(50, 50);
    } else if (strcmp(arg, "-2x1") == 0) { 
      window_size = vec(1200, 600);
    } else if (strcmp(arg, "-is-quit") == 0) { 
      is_quit = true;
    } else if (strcmp(arg, "-is-console") == 0) { 
      is_quit = is_console = true;
    } else if (strcmp(arg, "-720p") == 0) { 
      window_size = vec(1280, 720);
    } else if (strcmp(arg, "-640") == 0) { 
      window_size  = vec(640, 480);
    } else if (strcmp(arg, "-target-fps") == 0) { 
      target_fps = atoi(check_cmd_key_value(i++, argc, argv));
    } else if (strcmp(arg, "-f") == 0) {
      is_full_screen = 1;
    } else
      return i - 1;
  }
  return i;
}

#define NUM_KEYS 128

void viz_t::base_init ( int argc, const char* argv[] ) {
  strcpy(dump_filename,"");
  int sim_args_offset = parse_args(1, argc, argv);
  view_size = vec_t<2>(get_width(),get_height());
  for (int i = 0; i < NUM_KEYS; i++)
    is_key_hit[i] = 0;
  open(sim_args_offset, argc, argv);
  if (is_quit) {
    close();
    return;
  }
  if (is_graphics) {
    mouse_button = -1;
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode (GLUT_ACCUM | GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (window_size.x, window_size.y); 
    glutInitWindowPosition (window_pos.x, window_pos.y);
    int handle = glutCreateWindow (argv[0]);
    glutMouseFunc(c_on_button_press );
    glutMotionFunc(c_on_mouse_motion );
    glutPassiveMotionFunc(c_on_passive_mouse_motion );
    glutDisplayFunc(c_render);
    glutReshapeFunc(c_resize);
    glutIdleFunc(c_idle);
    glutKeyboardFunc(c_keyboard_handler);
    glutKeyboardUpFunc(c_keyboard_up_handler);
    glutSpecialFunc(c_special_handler);
    if (is_full_screen)
      glutFullScreen();

    glClear(GL_ACCUM_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);        // Black Background
    glLineWidth(1);
    glPointSize(1);
    glEnable(GL_DEPTH_TEST);	// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);	// The Type Of Depth Testing (Less Or Equal)
    glClearDepth(1.0f);		// Depth Buffer Setup
    glEnable(GL_LINE_SMOOTH);
    // glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    if (use_textures) {
      glShadeModel (GL_SMOOTH);	// Select Smooth Shading
      glEnable(GL_TEXTURE_2D);
      glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
      // select modulate to mix texture with color for shading
      glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
      // glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);  // Set Texture Gen Mode For S To Sphere Mapping
      // glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);  // Set Texture Gen Mode For T To Sphere Mapping
    }

    if (is_shadowing || is_lighting)
      setup_rc();
    else {
      if (is_white_background) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f );
      } else
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    }
    /*
    glClearColor (0.0f, 0.0f, 0.0f, 0.5f);        // Black Background
    glLineWidth(1);
    glPointSize(1);
    glClear(GL_ACCUM_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    */
  }
  set_base_time();
  if (is_audio) {
    audio_init();
    audio_start();
  } else {
    if (strlen(dump_filename) > 0) {
      char filename[256];
      post("DUMP %s\n", dump_filename);
      sprintf(filename, "./%s", dump_filename);
      open_avi_out(filename);
    } 
    if (is_graphics) {
      glutPostRedisplay();
      glutMainLoop();
    } else
      for (;;)
        viz->exec(0);
  }
}

int do_graphics_loop (void) {
  glutMainLoop();
}

void abort_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
 user_msg.assign("ABORTED");
 string_read_state->is_prompt = false;
 string_read_state->end_completion();
 now_keys = top_keys;
}

void key_not_found_fun (cmd_t* cmd, int key, int modifiers, void* obj) {
 if (key == '' || (key == 'G' && modifiers == GLUT_ACTIVE_CTRL))
   abort_fun(cmd, key, modifiers, obj);
 else {
   user_msg.assign("???");
   post("key not found %c\n", key);
   now_keys = top_keys;
 }
}

cmd_t* key_not_found = new cmd_t("not-found", "not-found", &key_not_found_fun);

void reset_view_fun (cmd_t* cmd, int key, int modifiers, void* obj) { reset_view(); }
cmd_t* reset_view_cmd = new cmd_t("reset-view", "reset-view", &reset_view_fun);
void toggle_full_screen_fun (cmd_t* cmd, int key, int modifiers, void* obj) { viz->toggle_full_screen(); }
cmd_t* toggle_full_screen_cmd
 = new cmd_t("toggle-full-screen", "toggle-full-screen", &toggle_full_screen_fun);
void single_step_fun (cmd_t* cmd, int key, int modifiers, void* obj) { viz->single_step(); }
cmd_t* single_step_cmd
 = new cmd_t("single-step", "single step", &single_step_fun);
void resume_execution_fun (cmd_t* cmd, int key, int modifiers, void* obj) { viz->resume_execution(); }
cmd_t* resume_execution_cmd
 = new cmd_t("resume-execution", "resume execution", &resume_execution_fun);
void quit_fun (cmd_t* cmd, int key, int modifiers, void* obj) { 
  viz->is_quit = true;
}
cmd_t* quit_cmd
 = new cmd_t("quit", "quit", &quit_fun);

int main (int argc, const char *argv[]) {
  viz = new_viz();
  viz->base_init(argc, argv);
}

