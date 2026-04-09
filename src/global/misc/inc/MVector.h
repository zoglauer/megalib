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

// MEGAlib libs:
#include "MString.h"
#include "MFastMath.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Three-dimensional vector class
class MVector 
{
public:
  //! Standard constructor
  MVector(double X = 0.0, double Y = 0.0, double Z = 0.0);
  //! Copy constructor
  MVector(const MVector& V);
  //! Default destructor
  virtual ~MVector();

  // Section 1: Get and Set operations --- those are the most time critical part of MEGAlib:
  //! Reset all vector components to zero
  void Clear() { m_X = 0.0; m_Y = 0.0; m_Z = 0.0; }

  //! Return true if this is the null vector
  bool IsNull() const { return (m_X == 0 && m_Y == 0 && m_Z == 0) ? true : false; } 
  
  //! Return the x component
  double X() const { return m_X; }
  //! Return the y component
  double Y() const { return m_Y; }
  //! Return the z component
  double Z() const { return m_Z; }

  //! Return the x component
  double GetX() const { return m_X; }
  //! Return the y component
  double GetY() const { return m_Y; }
  //! Return the z component
  double GetZ() const { return m_Z; }

  //! Set all components from another vector
  void Set(const MVector& V) { m_X = V.m_X; m_Y = V.m_Y; m_Z = V.m_Z; }
  //! Set the x component
  void SetX(double X) { m_X = X; }
  //! Set the y component
  void SetY(double Y) { m_Y = Y; }
  //! Set the z component
  void SetZ(double Z) { m_Z = Z; }
  //! Set all components directly
  void SetXYZ(double X, double Y, double Z) { m_X = X; m_Y = Y; m_Z = Z; }
  //! Set the vector from spherical coordinates
  void SetMagThetaPhi(double Magnitude, double Theta, double Phi);

  //! Set theta while keeping radius and phi
  void SetTheta(double Theta) {
    double Radius = Mag();
    double PhiValue = Phi();
    m_X = Radius * sin(Theta) * cos(PhiValue);
    m_Y = Radius * sin(Theta) * sin(PhiValue);
    m_Z = Radius * cos(Theta);
  }
  
  //! Set phi while keeping radius and theta
  void SetPhi(double PhiValue) {
    double Radius = Mag();
    double ThetaValue = Theta();
    m_X = Radius * sin(ThetaValue) * cos(PhiValue);
    m_Y = Radius * sin(ThetaValue) * sin(PhiValue);
    m_Z = Radius * cos(ThetaValue);
  }
  
  //! Set the radius while keeping theta and phi
  void SetMag(double Radius) {
    double PhiValue = Phi();
    double ThetaValue = Theta();
    m_X = Radius * sin(ThetaValue) * cos(PhiValue);
    m_Y = Radius * sin(ThetaValue) * sin(PhiValue);
    m_Z = Radius * cos(ThetaValue);
  }
  
  //! Basic set and get components by index 0..2
  double operator[](int i) const;
  //! Basic set and get components by index 0..2
  double& operator[](int i);


  // Section 2: Operations
 
  //! Assignment operator 
  MVector& operator=(const MVector& V) { m_X = V.m_X; m_Y = V.m_Y; m_Z = V.m_Z; return *this; }
  
  //! Return true if all elements are identical
  bool operator==(const MVector&) const;
  //! Equality operator with a tolerance
  bool AreEqual(const MVector& Vector, double Tolerance = 1E-6) const;

  //! Return false if one of the elements is identical
  bool operator!=(const MVector&) const;

  //! Less operator
  //! This one is intended for sorting purposes
  //! If x is smaller return true, if x is equal check y, if y is equal check z
  bool operator<(const MVector&) const;
  
  //! At least one component of V is smaller
  bool AtLeastOneSmaller(const MVector& V) const;
  //! At least one component of V is larger
  bool AtLeastOneLarger(const MVector& V) const;
  
  //! All components of V are smaller
  bool AllSmaller(const MVector& V) const;
  //! All components of V are larger
  bool AllLarger(const MVector& V) const;

  //! Add W to this vector
  MVector& operator+=(const MVector& W) { m_X += W.m_X; m_Y += W.m_Y; m_Z += W.m_Z; return *this; }
  //! Add W to this vector
  void Add(const MVector& W) { m_X += W.m_X; m_Y += W.m_Y; m_Z += W.m_Z; }
  
  //! Subtract V from this vector
  MVector& operator-=(const MVector& V) { m_X -= V.m_X; m_Y -= V.m_Y; m_Z -= V.m_Z; return *this; }
  //! Subtract V from this vector
  void Subtract(const MVector& V) { m_X -= V.m_X; m_Y -= V.m_Y; m_Z -= V.m_Z; }
  
  //! Returns a new vector, which is the negative of this one
  MVector operator-() const { return MVector(-m_X, -m_Y, -m_Z); }

  //! Scalar multiplication to this vector
  MVector& operator*=(double Scale) { m_X *= Scale; m_Y *= Scale; m_Z *= Scale; return *this; };
  //! Scalar multiplication to this vector
  void Multiply(double Scale) { m_X *= Scale; m_Y *= Scale; m_Z *= Scale; };
  

  // Section 3: Basic Functionality

  //! Return a new vector which is unit
  MVector Unit() const;

  //! Make *this* vector a unit vector(2,2)
  MVector& Unitize();

  //! Make all elements positive
  MVector Abs() const { return MVector(fabs(m_X), fabs(m_Y), fabs(m_Z)); }

  //! Return the azimuth angle of the vector in spherical coordinates 
  double Phi() const { return (m_X == 0.0 && m_Y == 0.0) ? 0.0 : atan2(m_Y, m_X); }

  //! Return the azimuth angle of the vector in spherical coordinates 
  //! Attention: accuracy is significantly reduced (~0.001 rad), do not use for further computations!
  double PhiApproximateMaths() const { return (m_X == 0.0 && m_Y == 0.0) ? 0.0 : MFastMath::atan2(m_Y, m_X); }

  //! Return the azimuth angle of the vector in spherical coordinates 
  double Theta() const { return (m_X == 0.0 && m_Y == 0.0 && m_Z == 0.0) ? 0.0 : atan2(sqrt(m_X * m_X + m_Y * m_Y), m_Z); }

  //! Return the azimuth angle of the vector in spherical coordinates
  //! Attention: accuracy is significantly reduced (~0.001 rad), do not use for further computations!
  double ThetaApproximateMaths() const { return (m_X == 0.0 && m_Y == 0.0 && m_Z == 0.0) ? 0.0 : MFastMath::atan2(sqrt(m_X * m_X + m_Y * m_Y), m_Z); }

  // Return the square of the magnitude of this vector
  double Mag2() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z; }

  // Return the magnitude of this vector
  double Mag() const { return sqrt(m_X * m_X + m_Y * m_Y + m_Z * m_Z); }

  // Return an orthogonal vector, by eliminating the component closest to zero to avoid large rounding errors 
  MVector Orthogonal() const;
  

  // Section 4: Advanced mathematical operations:

  //! Scalar product
  double Dot(const MVector& V) const { return m_X * V.m_X + m_Y * V.m_Y + m_Z * V.m_Z; }

  //! Cross product
  MVector Cross(const MVector& V) const { return MVector(m_Y * V.m_Z - V.m_Y * m_Z, m_Z * V.m_X - V.m_Z * m_X, m_X * V.m_Y - V.m_X * m_Y); }

  //! Angle between two Vectors
  double Angle(const MVector& V) const;



  // Section 5: Rotations:

  //! Rotate around x-axis
  void RotateX(double Angle); 
  //! Rotate around y-axis
  void RotateY(double Angle); 
  //! Rotate around z-axis
  void RotateZ(double Angle); 
  //! Rotate around V
  void RotateReferenceFrame(const MVector& V);
  //! Rotate vector with axis Axis by Angle
  void RotateAroundVector(const MVector& Axis, double Angle);
  
  
  //! Do a rotation 
  //MVector& operator*= (const MRotation& Rot);
  
  // Section 6: Additional high-level functions

  //! Checks if this vector is in the plane spanned by the other three vectors
  //! TODO: Change to AreCoplanar(...)
  bool Coplanar(const MVector& A, const MVector& B, const MVector& C, double Tolerance = 1E-6) const;

  //! Check if the vectors are orthogonal with a tolerance
  bool IsOrthogonal(const MVector& Vector, double Tolerance = 1E-6) const;
  
  //! Calculate the distance of a line spanned by the given vectors with this point
  double DistanceToLine(const MVector& A, const MVector& B) const;
  
  //! Return a string of the content of this class
  MString ToString() const;

  // private - not private - for speed

  //! The x component
  double m_X;
  //! The y component
  double m_Y;
  //! The z component
  double m_Z;

#ifdef ___CLING___
 public:
  ClassDef(MVector, 0) // no description
#endif
};

// Section XXX: Globally defined operators

// Very important ;-)
std::ostream& operator<<(std::ostream& os, const MVector& Vector);

//! Addition
MVector operator+(const MVector& V, const MVector& W);
//! Subtraction
MVector operator-(const MVector& V, const MVector& W);
//! Dot-product multiplication
double operator*(const MVector& V, const MVector& W);
//! Multiplication with scalar from right
MVector operator*(const MVector& V, double S);
//! Divide with scalar from right
MVector operator/(const MVector& V, double S);
//! Multiplication with scalar from left
MVector operator*(double S, const MVector& V);



#endif


////////////////////////////////////////////////////////////////////////////////
