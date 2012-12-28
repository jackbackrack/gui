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
//	  Department of Computing Science, University of Copenhagen
//    Universitetsparken 1
//    DK-2100 Copenhagen
//    Denmark
//
////////////////////////////////////////////////////////////////////////////////
#if defined(MSVC)
# pragma once
#endif

#ifndef OPENTISSUE_TRACKBALL_TRACKBALL_H
#define OPENTISSUE_TRACKBALL_TRACKBALL_H

	typedef double Real;
	typedef Real Vector[3];
	typedef Real Quaternion[4];
	typedef Real Transform[4][4];
	typedef Real glTransform[16];


/**
 *
 */
	class GenericTrackball 
	{
	public:
	    GenericTrackball(const Real radius = 1);
	    virtual ~GenericTrackball(void);
	    
	    virtual void Reset(void);

	    Real Radius(void) const;

	    const Vector& Pa();
	    const Vector& Pc();

	    Real  Angle() const;
	    const Vector& Axis();

	    virtual void BeginDrag(const Real x, const Real y) = 0;
	    virtual void Drag(const Real x, const Real y) = 0;
	    virtual void EndDrag(const Real x, const Real y) = 0;

	    /* const */ Transform&   Transformation(void);
	    const glTransform& GLtransformation(void);

	protected:
	    Real radius;   ///< TBD

	    Vector Panchor;  ///< TBD
	    Vector Pcurrent; ///< TBD

	    Real   CurrentAngle ; ///< TBD
	    Vector CurrentAxis;   ///< TBD

	    Transform   AnchorTransformation;         ///< TBD
	    Transform   IncrementalTransformation;    ///< TBD
	    Transform   CurrentTransformation;        ///< TBD
	    glTransform GLCurrentTransformation;      ///< TBD

	    // Operations on type Transform

	    void Identity(Transform& transformation) const;
	    void Copy(/* const */ Transform& src, Transform& dst) const;
	    void Transpose(Transform& trans) const;
	    void Multiply(/* const */ Transform& trans1, /* const */ Transform& trans2,
			  Transform& transformation) const;

	    // Operations on type Vector

	    Real Length(const Vector& vec) const;
	    void Copy(const Vector& src, Vector& dst) const;
	    Real Dot(const Vector& vec1, const Vector& vec2) const;
	    void Cross(const Vector& vec1, const Vector& vec2, 
		       Vector& crossprod) const;
	    void ComputeTransform(const Real angle, const Vector& axis, 
				  Transform& transformation);

	    // Operations on type Quaternion

	    Real Length(const Quaternion& quat) const;
	    void Copy(const Quaternion& src, Quaternion& dst) const;
	    void Negate(Quaternion& quat) const;
	    void Conjugate(Quaternion& quat) const;
	    void Inverse(Quaternion& quat) const;
	    void Multiply(const Quaternion& quat1, const Quaternion& quat2,
			  Quaternion& quaternion) const;
	    void ComputeTransform(const Quaternion& quat, 
				  Transform& transformation);
;

	private:
	    virtual void ComputeIncrementalTransformation(const Vector& Panchor,
							  const Vector& Pcurrent,
							  Transform& transformation) = 0;
	};/*End of class GenericTrackball*/

class Trackball : public GenericTrackball
{
public:
    Trackball(const Real radius = 1);
    virtual ~Trackball();

    virtual void Reset(void);

    void BeginDrag(const Real x, const Real y);
    void Drag(const Real x, const Real y);
    void EndDrag(const Real x, const Real y);

protected:
    
private:
    void ProjectOntoSurface(Vector& P);
    virtual void ComputeIncrementalTransformation(const Vector& Panchor,
						  const Vector& Pcurrent,
						  Transform& transformation);

};/*End of class Trackball*/

// extern "C" {
  extern void load_trackball_transformation( void ); // model coords
  extern void load_trackball_zoom_translate( void ); // projection coords
  extern void on_left_button_down (int x, int y);
  extern void on_left_button_up (int x, int y);
  extern void on_right_button_down (int x, int y);
  extern void on_right_button_up (int x, int y);
  extern void on_mouse_move (int x, int y);
  extern void on_joy_move (int x, int y);
  extern void reset_view ( void );
  extern void get_view_size ( double* width, double* height );
// extern void post(char* string, ...);

// }

#endif // OPENTISSUE_TRACKBALL_BELL_H

