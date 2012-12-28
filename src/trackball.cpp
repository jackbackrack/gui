////////////////////////////////////////////////////////////////////////////////
//
// OpenTissue, A toolbox for physical based simulation and animation.
// Copyright (C) 2003 Department of Computer Science, University of Copenhagen
//
// This file is part of OpenTissue.
//
// OpenTissue is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// any later version.
//
// OpenTissue is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenTissue; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Please send remarks, questions and bug reports to OpenTissue@diku.dk,
// or write to:
//
//    Att: Kenny Erleben and Jon Sporring
//    Department of Computing Science, University of Copenhagen
//    Universitetsparken 1
//    DK-2100 Copenhagen
//    Denmark
//
////////////////////////////////////////////////////////////////////////////////

#if(_MSC_VER >= 1200)
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#endif

// #include <iostream>
// #include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cmath>
#include "trackball.h"
#include "vec.h"

    /**
     *
     * @param radius
     */
    GenericTrackball::GenericTrackball(const Real radius) : radius(radius)
    {};


    /**
     *
     */
    GenericTrackball::~GenericTrackball(void)
    {};
 

    /**
     *
     */
    void GenericTrackball::Reset(void)
    {
      // Initialize private variables

      for (int i = 0; i < 3; ++i)
      {
        this->Panchor[i]     = 0;
        this->Pcurrent[i]    = 0;
	this->CurrentAxis[i] = 0;
      }
      this->CurrentAngle     = 0;

      // Initialize Anchor, Incremental, and CurrentTransformation.

      this->Identity(this->AnchorTransformation);
      this->Identity(this->IncrementalTransformation);
      this->Identity(this->CurrentTransformation);
    }


    /**
     *
     * @return
     */
    Real GenericTrackball::Radius(void) const
    {
      return this->radius;
    }


    const Vector& GenericTrackball::Pa()
    {
	Real lengthPa = this->Length(this->Panchor);
	if (lengthPa == 0.0) {
	  // std::cout << "|Pa()| == 0" << std::endl;
	    //exit(-1);
	}
	else {
	    for (int i = 0; i < 3; ++i) this->Panchor[i] /= lengthPa;
	}

	return this->Panchor;
    }


    const Vector& GenericTrackball::Pc()
    {
	Real lengthPc = this->Length(this->Pcurrent);
	if (lengthPc == 0.0) {
	  // std::cout << "|Pc()| == 0" << std::endl;
	    //exit(-1);
	}
	else {
	    for (int i = 0; i < 3; ++i) this->Pcurrent[i] /= lengthPc;
	}

	return this->Pcurrent;
    }


    Real GenericTrackball::Angle() const
    {
      return this->CurrentAngle;
    }


    const Vector& GenericTrackball::Axis()
    {
	Real lengthAxis = this->Length(this->CurrentAxis);
	if (lengthAxis == 0.0) {
	  // std::cout << "GenericTrackball::|Axis()| == 0" << std::endl;
	    //exit(-1);
	}
	else {
	    for (int i = 0; i < 3; ++i) this->CurrentAxis[i] /= lengthAxis;
	}

	return this->CurrentAxis;
    }


    /**
     *
     * @return 
     */
  /* const */ Transform& GenericTrackball::Transformation(void)
    {
      // Compute the the rotation from Panchor to Pcurrent, i.e.
      // the rotation form (Xanchor, Yanchor, Zanchor) to
      // (Xcurrent, Ycurrent, Zcurrent) along a great circle.
      // Multiply the IncrementalTransformation and the AnchorTransformation
      // to get the CurrentTransformation.

      this->Multiply(this->IncrementalTransformation, this->AnchorTransformation,
                     this->CurrentTransformation);

      return this->CurrentTransformation;
    }

    /**
     *
     * @return 
     */
    const glTransform& GenericTrackball::GLtransformation(void)
    {
      // Compute the the rotation from Panchor to Pcurrent, i.e.
      // the rotation form (Xanchor, Yanchor, Zanchor) to
      // (Xcurrent, Ycurrent, Zcurrent) along a great circle.
      // Multiply the IncrementalTransformation and the AnchorTransformation
      // to get the CurrentTransformation.

      this->Multiply(this->IncrementalTransformation, this->AnchorTransformation,
                     this->CurrentTransformation);

      int glindex = 0;
      for (unsigned int col = 0; col < 4; ++col)
      {
        for (unsigned int row = 0; row < 4; ++row)
        {
          this->GLCurrentTransformation[glindex++] = this->CurrentTransformation[row][col];
        }
      }
      return this->GLCurrentTransformation;
    }


    // Protected Members
    /**
     *
     * @param transformation
     */
    void GenericTrackball::Identity(Transform& transformation) const
    {
      for (int i = 0; i < 4; ++i)
      {
        for (int j = 0; j < 4; ++j)
        {
          transformation[i][j] = 0;
        }
      }
      for (int k = 0; k < 4; ++k)
      {
        transformation[k][k] = 1;
      }
    }

    /**
     *
     * @param src
     * @param dst
     */
  void GenericTrackball::Copy(/* const */ Transform& src, Transform& dst) const
    {
      for (int i = 0; i < 4; ++i)
      {
        for (int j = 0; j < 4; ++j)
        {
          dst[i][j] = src[i][j];
        }
      }
    }


    /**
     *
     *
     * @param trans
     */
    void GenericTrackball::Transpose(Transform& trans) const
    {
      Transform tmp;
      for (int i = 0; i < 4; ++i)
      {
        for (int j = 0; j < 4; ++j)
        {
          tmp[i][j] = trans[j][i];
        }
      }
      this->Copy(tmp, trans);
    }

    /**
     *
     * @param trans1
     * @param trans2
     * @param transformation
     *
     */
    void GenericTrackball::Multiply(/* const */ Transform& trans1,
                                    /* const */ Transform& trans2,
                                    Transform& transformation) const
    {
      // This is an ordinary matrix multiplication
      for (int i = 0; i < 4; ++i)
      {
        for (int j = 0; j < 4; ++j)
        {
          Real sum = 0;
          for (int k = 0; k < 4; ++k)
          {
            sum += trans1[i][k] * trans2[k][j];
          }
          transformation[i][j] = sum;
        }
      }
    }


    Real GenericTrackball::Length(const Vector& vec) const
    {
      Real length = 0;
      for (int i = 0; i < 3; ++i) length += vec[i] * vec[i];

      return sqrt(length);
    }


    void GenericTrackball::Copy(const Vector& src, Vector& dst) const
    {
      for (int i = 0; i < 3; ++i) dst[i] = src[i];
    }


    Real GenericTrackball::Dot(const Vector& vec1, const Vector& vec2) const
    {
      Real dotprod(0);
      for (int i = 0; i < 3; ++i) dotprod += vec1[i] * vec2[i];

      return dotprod;
    }


    void GenericTrackball::Cross(const Vector& vec1, const Vector& vec2,
                                 Vector& crossprod) const
    {
      crossprod[0] = vec1[1] * vec2[2] - vec2[1] * vec1[2];
      crossprod[1] = vec2[0] * vec1[2] - vec1[0] * vec2[2];
      crossprod[2] = vec1[0] * vec2[1] - vec2[0] * vec1[1];
    }


    void GenericTrackball::ComputeTransform(const Real    angle,
                                            const Vector& axis,
                                            Transform& transformation)
    {
      Real sinTheta = sin(angle);
      Real cosTheta = cos(angle);

      Real lengthU = this->Length(axis);
      if (lengthU == 0)
      {
        this->Identity(transformation);
        return;
      }
      Real Ux = axis[0] / lengthU;
      Real Uy = axis[1] / lengthU;
      Real Uz = axis[2] / lengthU;

      transformation[0][0] = Ux * Ux + (1 - Ux * Ux) * cosTheta;
      transformation[0][1] = Ux * Uy * (1 - cosTheta) - Uz * sinTheta;
      transformation[0][2] = Ux * Uz * (1 - cosTheta) + Uy * sinTheta;
      transformation[0][3] = 0;

      transformation[1][0] = Ux * Uy * (1 - cosTheta) + Uz * sinTheta;
      transformation[1][1] = Uy * Uy + (1 - Uy * Uy) * cosTheta;
      transformation[1][2] = Uy * Uz * (1 - cosTheta) - Ux * sinTheta;
      transformation[1][3] = 0;

      transformation[2][0] = Ux * Uz * (1 - cosTheta) - Uy * sinTheta;
      transformation[2][1] = Uy * Uz * (1 - cosTheta) + Ux * sinTheta;
      transformation[2][2] = Uz * Uz + (1 - Uz * Uz) * cosTheta;
      transformation[2][3] = 0;

      transformation[3][0] = 0;
      transformation[3][1] = 0;
      transformation[3][2] = 0;
      transformation[3][3] = 1;

      this->CurrentAngle = angle;

      Vector Axis;
      Axis[0] = Ux;
      Axis[1] = Uy;
      Axis[2] = Uz;
      this->Copy(Axis, this->CurrentAxis);
    }


    Real GenericTrackball::Length(const Quaternion& quat) const
    {
      Real length = 0;
      for (int i = 0; i < 4; ++i) length += quat[i] * quat[i];

      return sqrt(length);
    }


    void GenericTrackball::Copy(const Quaternion& src,
                                Quaternion& quaternion) const
    {
      for (int i = 0; i < 4; ++i) quaternion[i] = src[i];
    }


    void GenericTrackball::Negate(Quaternion& quat) const
    {
      for (int i = 0; i < 4; ++i) quat[i] = -quat[i];
    }


    void GenericTrackball::Conjugate(Quaternion& quat) const
    {
      for (int i = 1; i < 4; ++i) quat[i] = -quat[i];
    }


    void GenericTrackball::Inverse(Quaternion& quat) const
    {
      // qinv = conjugate(q) / length(q)^2

      Real length  = this->Length(quat);
      Real length2 = length * length;

      Quaternion conjugate;
      this->Copy(quat, conjugate);
      this->Conjugate(conjugate);

      Quaternion inverse;
      for (int i = 0; i < 4; ++i) inverse[i] = conjugate[i] / length2;
      this->Copy(inverse, quat);
    }


    void GenericTrackball::Multiply(const Quaternion& quat1,
                                    const Quaternion& quat2,
                                    Quaternion& quaternion) const
    {
      // (s1 * s2 - v1 * v2, s1 * v2 + s2 * v1  v1 x v2)

      Real dotprod = 0;
      for (int i = 1; i < 4; ++i) dotprod += quat1[i] * quat2[i];

      Real Xcrossprod = quat1[2] * quat2[3] - quat2[2] * quat1[3];
      Real Ycrossprod = quat2[1] * quat1[3] - quat1[1] * quat2[3];
      Real Zcrossprod = quat1[1] * quat2[2] - quat2[1] * quat1[2];

      quaternion[0] = quat1[0] * quat2[0] - dotprod;
      quaternion[1] = quat1[0] * quat2[1] + quat2[0] * quat1[1] + Xcrossprod;
      quaternion[2] = quat1[0] * quat2[2] + quat2[0] * quat1[2] + Ycrossprod;
      quaternion[3] = quat1[0] * quat2[3] + quat2[0] * quat1[3] + Zcrossprod;
    }


    void GenericTrackball::ComputeTransform(const Quaternion& quat,
                                            Transform& transformation)
    {
      int i;
      Real W = quat[0];
      Real X = quat[1];
      Real Y = quat[2];
      Real Z = quat[3];

      if ( sqrt(X*X + Y*Y + Z*Z) == 0)
      {
        this->Identity(transformation);
        return;
      }


      transformation[0][0] = 1 - 2 * Y * Y - 2 * Z * Z;
      transformation[0][1] = 2 * X * Y - 2 * W * Z;
      transformation[0][2] = 2 * X * Z + 2 * W * Y;
      transformation[0][3] = 0;

      transformation[1][0] = 2 * X * Y + 2 * W * Z;
      transformation[1][1] = 1 - 2 * X * X - 2 * Z * Z;
      transformation[1][2] = 2 * Y * Z - 2 * W * X;
      transformation[1][3] = 0;

      transformation[2][0] = 2 * X * Z - 2 * W * Y;
      transformation[2][1] = 2 * Y * Z + 2 * W * X;
      transformation[2][2] = 1 - 2 * X * X - 2 * Y * Y;
      transformation[2][3] = 0;

      transformation[3][0] = 0;
      transformation[3][1] = 0;
      transformation[3][2] = 0;
      transformation[3][3] = 1;

      Vector Axis;
      for (i = 0; i < 3; ++i) Axis[i] = quat[i+1];
      Real lengthAxis = this->Length(Axis);
      for (i = 0; i < 3; ++i) Axis[i] /= lengthAxis;
      this->Copy(Axis, this->CurrentAxis);

      this->CurrentAngle = 2 * atan2(lengthAxis, quat[0]);
    }

/**
 *
 * @param radius
 */
Trackball::Trackball(const Real radius) : GenericTrackball(radius)
{
    this->Reset();
}


/**
 *
 */
Trackball::~Trackball()
{}


/**
 *
 */
void Trackball::Reset()
{
    // Initialize private variables

    this->GenericTrackball::Reset();
    this->ProjectOntoSurface(this->Panchor);
    this->ProjectOntoSurface(this->Pcurrent);
}


/**
 *
 * @param x
 * @param y
 */
void Trackball::BeginDrag(const Real x, const Real y)
{
    this->CurrentAngle   = 0;
    this->CurrentAxis[0] = 0;
    this->CurrentAxis[1] = 0;
    this->CurrentAxis[2] = 0;

    this->Copy(this->CurrentTransformation, this->AnchorTransformation);
    this->Identity(this->IncrementalTransformation);
    this->Identity(this->CurrentTransformation);

    // Initialize the private variables 
    // (Xanchor, Yanchor, Zanchor) and (Xcurrent, Ycurrent, Zcurrent)

    this->Panchor[0]  = x;
    this->Panchor[1]  = y;
    this->Panchor[2]  = 0;
    this->ProjectOntoSurface(this->Panchor);
    this->Pcurrent[0] = x;
    this->Pcurrent[1] = y;
    this->Pcurrent[2] = 0;
    this->ProjectOntoSurface(this->Pcurrent);
}


/**
 *
 * @param x
 * @param y
 */
void Trackball::Drag(const Real x, const Real y)
{
    // Update the private variables (Xcurrent, Ycurrent, Zcurrent)

    this->Pcurrent[0] = x;
    this->Pcurrent[1] = y;
    this->Pcurrent[2] = 0;
    this->ProjectOntoSurface(this->Pcurrent);

    this->ComputeIncrementalTransformation(this->Panchor, this->Pcurrent,
					   this->IncrementalTransformation);
}


/**
 *
 * @param x
 * @param y
 */
void Trackball::EndDrag(const Real x, const Real y)
{
    // Update the private variable (Xcurrent, Ycurrent, Zcurrent)

    this->Pcurrent[0] = x;
    this->Pcurrent[1] = y;
    this->Pcurrent[2] = 0;
    this->ProjectOntoSurface(this->Pcurrent);

    this->ComputeIncrementalTransformation(this->Panchor, this->Pcurrent,
					   this->IncrementalTransformation);
}


/**
 *
 * @param x
 * @paran y
 * @param z
 */
void Trackball::ProjectOntoSurface(Vector& P)
{
    Real radius2 = this->radius * this->radius;
    Real length2 = P[0] * P[0] + P[1] * P[1];

    if (length2 <= radius2 / 2.0)
	P[2] = sqrt(radius2 - length2);
    else 
    {
	P[2] = radius2 / (2 * sqrt(length2));
	
	Real length = sqrt(length2 + P[2] * P[2]);
	P[0] /= length;
	P[1] /= length;
	P[2] /= length;
    }
    Real lengthP = this->Length(P);
    for (int i = 0; i < 3; ++i) P[i] /= lengthP;
}


/**
 *
 *
 * @param Xanchor
 * @param Yanchor
 * @param Zanchor
 * @param Xcurrent
 * @param Ycurrent
 * @param Zcurrent
 * @param transformation
 */
void Trackball::ComputeIncrementalTransformation(const Vector& Panchor,
						 const Vector& Pcurrent,
						 Transform& transformation)
{
#if 1
    Real lengthPa = this->Length(Panchor);
    Real lengthPc = this->Length(Pcurrent);
    Vector Pa;
    Vector Pc;
    for (int i = 0; i < 3; ++i) {
	Pa[i] = Panchor[i]  / lengthPa;
	Pc[i] = Pcurrent[i] / lengthPc;
    }

    Vector Axis;
    this->Cross(Panchor, Pcurrent, Axis);
    Real lengthAxis = this->Length(Axis);
    Real Ux = Axis[0];
    Real Uy = Axis[1];
    Real Uz = Axis[2];
    if (lengthAxis != 0.0)
    {
      Ux /= lengthAxis;
      Uy /= lengthAxis;
      Uz /= lengthAxis;
    }
    this->CurrentAxis[0] = Ux;
    this->CurrentAxis[1] = Uy;
    this->CurrentAxis[2] = Uz;
 
    Real Theta = atan2(lengthAxis, this->Dot(Panchor, Pcurrent));
    this->CurrentAngle = Theta;

    Real sinTheta = sin(Theta);
    Real cosTheta = cos(Theta);

    transformation[0][0] = Ux * Ux + (1 - Ux * Ux) * cosTheta;
    transformation[0][1] = Ux * Uy * (1 - cosTheta) - Uz * sinTheta;
    transformation[0][2] = Ux * Uz * (1 - cosTheta) + Uy * sinTheta;
    transformation[0][3] = 0;

    transformation[1][0] = Ux * Uy * (1 - cosTheta) + Uz * sinTheta;
    transformation[1][1] = Uy * Uy + (1 - Uy * Uy) * cosTheta;
    transformation[1][2] = Uy * Uz * (1 - cosTheta) - Ux * sinTheta;
    transformation[1][3] = 0;

    transformation[2][0] = Ux * Uz * (1 - cosTheta) - Uy * sinTheta;
    transformation[2][1] = Uy * Uz * (1 - cosTheta) + Ux * sinTheta;
    transformation[2][2] = Uz * Uz + (1 - Uz * Uz) * cosTheta;
    transformation[2][3] = 0;

    transformation[3][0] = 0;
    transformation[3][1] = 0;
    transformation[3][2] = 0;
    transformation[3][3] = 1;
#else
#ifdef DEBUG
    std::cout << "Panchor  = (" << Panchor  << ")   " 
	 << "|Panchor| = " << this->Length(Panchor) << std::endl;
    std::cout << "Pcurrent = (" << Pcurrent << ")   " 
	 << "|Pcurrent| = " << this->Length(Pcurrent)<< std::endl;
#endif

    Real lengthPa = this->Length(Panchor);
    Real lengthPc = this->Length(Pcurrent);
    Quaternion Qanchor;
    Quaternion Qcurrent;
    Qanchor[0] = Qcurrent[0] = 0;
    for (int i = 0; i < 3; ++i) {
	Qanchor[i+1]  = Panchor[i]  / lengthPa;
	Qcurrent[i+1] = Pcurrent[i] / lengthPc;
    }

#ifdef DEBUG
    std::cout << "Qanchor   = (" << Qanchor << ")   "
	 << "|Qanchor|   = " << this->Length(Qanchor) << std::endl;
    std::cout << "Qcurrent  = (" << Qcurrent << ")   "
	 << "|Qcurrent|  = " << this->Length(Qcurrent) << std::endl;
#endif

    Quaternion Qrotation;
    this->Multiply(Qcurrent, Qanchor, Qrotation);
    this->Negate(Qrotation);

#ifdef DEBUG
    std::cout << "Qrotation = (" << Qrotation << ")   "
	 << "|Qrotation| = " << this->Length(Qrotation) << std::endl;
#endif

    this->ComputeTransform(Qrotation, transformation);

    // std::cout << transformation << std::endl;
#endif
}

#include <math.h>
#ifdef _WIN32
#include <windows.h>
#include <gl\gl.h>
#else
#include <unistd.h>
#include <stdlib.h>
#ifdef MACOSX
#include <OpenGL/gl.h> 
#else
#include <GL/gl.h>  // Linux
#endif
#endif

Real      Radius  = 0.8;
Real      Xcenter = 0.0;
Real      Ycenter = 0.0;
Real      Zcenter = 0.0;
float     zoom_factor = 1.0;

GenericTrackball* TB = new Trackball(Radius);

#define NONE   0
#define ZOOM   1
#define ROTATE 2

typedef struct { float x; float y; float z; } Vec3f;

static Vec3f last_point = { 0.0, 0.0, 0.0 };
static int   movement   = NONE;

static const float m_ROTSCALE = 90.0;
static const float m_ZOOMSCALE = 0.008;
static const float m_MOVESCALE = 0.0001;

static Vec3f vec3f(float xx, float yy, float zz) {
  Vec3f v; v.x=xx; v.y=yy; v.z=zz; return v;
}

extern float view_width, view_height;

void NormalizeCoordinates(Real& x, Real& y) {
  double width, height;
  get_view_size( &width, &height);
  x = 2.0 * x / width  - 1.0;
  if (x < -1.0) x = -1.0;
  if (x >  1.0) x =  1.0;

  y = -(2.0 * y / height - 1.0);
  if (y < -1.0) y = -1.0;
  if (y >  1.0) y =  1.0;
}

void load_trackball_transformation( void ) {
  Transform& Tmp = TB->Transformation();
  int glindex = 0;
  GLdouble glmatrix[16];
    
  glLoadIdentity();

  for (unsigned int col = 0; col < 4; ++col) {
    for (unsigned int row = 0; row < 4; ++row) {
      glmatrix[glindex++] = Tmp[row][col];
    }
  }
  glLoadMatrixd(glmatrix);

}

// Loads the non-rotational parts of the trackball positioning
// Added by jsmb 5/13/06
void load_trackball_zoom_translate( void ) {
  glScalef( zoom_factor, zoom_factor, zoom_factor );
  glTranslatef( Xcenter, Ycenter, 0.0 );
}

void on_left_button_down (int x, int y) {
  Real Xnorm = x; Real Ynorm = y;
  NormalizeCoordinates(Xnorm, Ynorm);
  movement = ROTATE;
  TB->BeginDrag(Xnorm, Ynorm);
}

void on_left_button_up (int x, int y) {
  Real Xnorm = x; Real Ynorm = y;
  NormalizeCoordinates(Xnorm, Ynorm);
  TB->EndDrag(Xnorm, Ynorm);
  movement = NONE;
}

void on_right_button_down (int x, int y) {
  movement = ZOOM;
  last_point = vec3f(x, y, 0);
}

void on_right_button_up (int x, int y) {
  movement = NONE;
}

void on_mouse_move (int x, int y) {
  float pixel_diff;
  Vec3f cur_point;

  switch (movement) {
  case ROTATE :  { // Left-mouse button is being held down
    Real Xnorm = x;
    Real Ynorm = y;
    NormalizeCoordinates(Xnorm, Ynorm);
    TB->Drag(Xnorm, Ynorm);
    break;
  }
  case ZOOM :  { // Right-mouse button is being held down
    //
    // Zoom into or away from the scene based upon how far the mouse moved in the x-direction.
    //   This implementation does this by scaling the eye-space.
    //   This should be the first operation performed by the GL_PROJECTION matrix.
    //   1. Calculate the signed distance
    //       a. movement to the left is negative (zoom out).
    //       b. movement to the right is positive (zoom in).
    //   2. Calculate a scale factor for the scene s = 1 + a*dx
    //   3. Call glScalef to have the scale be the first transformation.
    // 
    // post("ZOOM %d %f %f\n", x, last_point.x, zoom_factor);
    pixel_diff  = x - last_point.x; 
    zoom_factor *= 1.0 + pixel_diff * m_ZOOMSCALE;
    //
    // Set the current point, so the lastPoint will be saved properly below.
    //
    // glMatrixMode( GL_PROJECTION );
    // glScalef( zoom_factor, zoom_factor, zoom_factor );
    // glMatrixMode( GL_MODELVIEW );
    cur_point.x = (float) x;  cur_point.y = (float) y;  cur_point.z = (float)0;
    break;
  }
  default: return;
  }
  //
  // Save the location of the current point for the next movement. 
  //
  last_point = cur_point;
	
}

void reset_view ( void ) {
  TB->Reset();
  Xcenter=Ycenter=Zcenter=0.0; // rezoom and recenter as well
  zoom_factor=1.0;
}  

float get_zoom ( void ) {
  return zoom_factor;
}
float set_zoom ( float zoom ) {
  return zoom_factor = zoom;
}
vec_t<3> get_translation ( void ) {
  return vec(Xcenter, Ycenter, Zcenter);
}
vec_t<3> set_translation ( vec_t<3> pos ) {
  Xcenter = pos.x;
  Ycenter = pos.y;
  Zcenter = pos.z;
  return pos;
}
void zoom_zoom ( float fac ) {
  zoom_factor *= fac;
}

static Vec3f last_joy_point = { 0.0, 0.0, 0.0 };

void on_joy_move (int x, int y) {
  float pixel_diff_x, move_amount_x;
  float pixel_diff_y, move_amount_y;
  Vec3f cur_joy_point;

  move_amount_x = x * m_MOVESCALE;
  move_amount_y = -y * m_MOVESCALE;
  glMatrixMode( GL_PROJECTION );
  glTranslatef( move_amount_x, move_amount_y, 0.0 );
  Xcenter += move_amount_x;  Ycenter += move_amount_y;
  glMatrixMode( GL_MODELVIEW );
  last_joy_point = cur_joy_point;
}
