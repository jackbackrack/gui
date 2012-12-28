#ifndef __IS_GLUT__
#define __IS_GLUT__

#ifdef MACOSX
#include <GLUT/glut.h> 
#else
#include <GL/glut.h> // Linux, Windows
#endif
#define GL_BGR_EXT  0x80E0
#define GL_BGRA_EXT 0x80E1

#endif
