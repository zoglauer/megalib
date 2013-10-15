/*
 * MVector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MVector__
#define __MVector__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <cmath>
using namespace std;

// Root libs:
#include <TMatrix.h>
#include <TRotation.h>

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MVector 
{
public:
  //! Standard constructor
  MVector(double X = 0.0, double Y = 0.0, double Z = 0.0);
  //! Copy constructor
  MVector(const MVector& V);
  // Destructor
  virtual ~MVector();

  // Section 1: Get and Set operations --- those are the most time critical part of MEGAlib:
  void Clear() { m_X = 0.0; m_Y = 0.0; m_Z = 0.0; }

  double X() const { return m_X; }
  double Y() const { return m_Y; }
  double Z() const { return m_Z; }

  double GetX() const { return m_X; }
  double GetY() const { return m_Y; }
  double GetZ() const { return m_Z; }

  void Set(const MVector& V) { m_X = V.m_X; m_Y = V.m_Y; m_Z = V.m_Z; }
  void SetX(double x) { m_X = x; }
  void SetY(double y) { m_Y = y; }
  void SetZ(double z) { m_Z = z; }
  void SetXYZ(double x, double y, double z) { m_X = x; m_Y = y; m_Z = z; }
  void SetMagThetaPhi(double mag, double theta, double phi);

  //! Set theta while keeping radius and phi
  void SetTheta(double t) {
    double r = Mag();
    double p = Phi();
    m_X = r*sin(t)*cos(p);
    m_Y = r*sin(t)*sin(p);
    m_Z = r*cos(t);
  }

  //! Set phi while keeping radius and theta
  void SetPhi(double p) {
    double r = Mag();
    double t = Theta();
    m_X = r*sin(t)*cos(p);
    m_Y = r*sin(t)*sin(p);
    m_Z = r*cos(t);
  }

  //! Basic set and get components by index 0..2
  double operator[] (int i) const;
  double& operator [] (int i);


  // Section 2: Operations
 
  //! Assignment operator 
  MVector& operator= (const MVector& V) { m_X = V.m_X; m_Y = V.m_Y; m_Z = V.m_Z; return *this; }
  
  //! Return true if all elements are identical
  bool operator==(const MVector&) const;
  //! Equality operator with a tolerance
  bool AreEqual(const MVector& Vector, double Tolerance = 1E-6) const;

  //! Return false if one of the elements is identical
  bool operator!=(const MVector&) const;

  // Special operator required by Cosima... (basically don't use it for anything else..)
  bool operator<(const MVector&) const;

  // Add W to this vector
  MVector& operator+=(const MVector& W) { m_X += W.m_X; m_Y += W.m_Y; m_Z += W.m_Z; return *this; }

  // Subtract V from this vector
  MVector& operator-=(const MVector& V) { m_X -= V.m_X; m_Y -= V.m_Y; m_Z -= V.m_Z; return *this; }

  //! Returns a new vector, which is the negative of this one
  MVector operator-() const { return MVector(-m_X, -m_Y, -m_Z); }

  //! Scalar multiplication to this vector
  MVector& operator*= (double s) { m_X *= s; m_Y *= s; m_Z *= s; return *this; };


  // Section 3: Basic Functionality

  //! Return a new vector which is unit
  MVector Unit() const;

  //! Make *this* vector a unit vector
  MVector& Unitize();

  //! Make all elements positive
  MVector Abs() const { return MVector(fabs(m_X), fabs(m_Y), fabs(m_Z)); }

  //! Return the azimuth angle of the vector in spherical coordinates 
  double Phi() const { return (m_X == 0.0 && m_Y == 0.0) ? 0.0 : atan2(m_Y,m_X); }

  //! Return he azimuth angle of the vector in spherical coordinates 
  double Theta() const { return (m_X == 0.0 && m_Y == 0.0 && m_Z == 0.0) ? 0.0 : atan2(sqrt(m_X*m_X + m_Y*m_Y),m_Z); }

  // Return the square of the magnitude of this vector
  double Mag2() const { return m_X*m_X + m_Y*m_Y + m_Z*m_Z; }

  // Return the magnitude of this vector
  double Mag() const { return sqrt(m_X*m_X + m_Y*m_Y + m_Z*m_Z); }



  // Section 4: Advanced mathematical operations:

  //! Scalar product
  double Dot(const MVector& V) const { return m_X*V.m_X + m_Y*V.m_Y + m_Z*V.m_Z; }

  //! Cross product
  MVector Cross(const MVector& V) const { return MVector(m_Y*V.m_Z - V.m_Y*m_Z, m_Z*V.m_X - V.m_Z*m_X, m_X*V.m_Y - V.m_X*m_Y); }

  //! Angle between two Vectors
  double Angle(const MVector& V) const;



  // Section 5: Rotations:

  //! Rotate around x-axis
  void RotateX(double Angle); 
  //! Rotate around y-axis
  void RotateY(double Angle); 
  //! Rotate around z-axis
  void RotateZ(double Angle); 
	//! Rotate around Uz
	void RotateReferenceFrame(const MVector& V);

	
  //! Do a rotation via a TRotation --- needed by geomega (--> define simplified TRotation later if needed)
  MVector& operator *= (const TRotation & Rot);


  // Section 6: Additional high-level functions

  //! Checks if this vector is in the plane spanned by the other three vectors
  bool Coplanar(const MVector& A, const MVector& B, const MVector& C, double Tolerance = 1E-6);

  //! Calculate the distance of a line spanned by the given vectors with this point
  double DistanceToLine(const MVector& A, const MVector& B);
  
  //! Return a string of the content of this class
  MString ToString();

  // private - not private - for speed

  double m_X, m_Y, m_Z;
  // The components...

#ifdef ___CINT___
 public:
  ClassDef(MVector, 0) // no description
#endif
};

// Section XXX: Globally defined operators

// Very important ;-)
std::ostream& operator<<(std::ostream& os, const MVector& Vector);

// Addition
MVector operator+ (const MVector& V, const MVector& W);
// Subtraction
MVector operator- (const MVector& V, const MVector& W);
// Dot-product multiplication
double operator* (const MVector& V, const MVector& W);
// Multiplication with scalar from right
MVector operator* (const MVector& V, double S);
// Divide with scalar from right
MVector operator/ (const MVector& V, double S);
// Multiplication with scalar from left
MVector operator* (double S, const MVector& V);
//! Allow rotations with ROOT matrices --> replace by own rotation class...
MVector operator* (const TMatrix& M, const MVector& V);







#endif


////////////////////////////////////////////////////////////////////////////////
