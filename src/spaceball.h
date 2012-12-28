//////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010, Jonathan Bachrach, all rights reserved. //
// See LICENSING file for details.                                  //
//////////////////////////////////////////////////////////////////////
#include "vec.h"
extern void on_left_button_down (int x, int y);
extern void on_left_button_up (int x, int y);
extern void on_right_button_down (int x, int y);
extern void on_right_button_up (int x, int y);
extern void on_mouse_move (int x, int y);
extern float get_zoom ( void );
extern float set_zoom ( float fac );
extern vec_t<3> get_translation ( void );
extern vec_t<3> set_translation ( vec_t<3> pos );
