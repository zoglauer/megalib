/*
 * MQuaternion.h
 *
 * Copyright (C) by Carolyn Kierans & Andreas Zoglauer
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 * Inspired by the quaternion implementation of Angela Bennett:
 * http://www.ncsa.illinois.edu/People/kindr/emtc/quaternions/quaternion.h
 * 
 */


#ifndef __MQuaternion__
#define __MQuaternion__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <iostream>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRotation.h"

// Forward declarations:


///////////////////////////////////////////////////////////////////////////////////


//! A quaternion 
class MQuaternion
{
  public:
  //! Default constructor - a new quaternion with all elements equal zero
  MQuaternion();
  //! Standard constructor - quaternion initialized by elements
  MQuaternion(const double W, const double X, const double Y, const double Z);
  //! Construct a quaternion for a rotation matrix
  MQuaternion(const MRotation& Rotation);
  //! Copy Constructor
  MQuaternion(const MQuaternion& Q); 
  //! Default Destructor
  virtual ~MQuaternion();
  
  //! Assignment operator
  MQuaternion& operator=(const MQuaternion& Q);

  //! Addition operator, return a new quaternion created by the addition of Q and this instance
  MQuaternion operator+(const MQuaternion& Q) const;
  //! Subtraction operator, return a new quaternion created by the the subtraction of Q from this instance
  MQuaternion operator-(const MQuaternion& Q) const;
  //! Division operator, return a new quaternion by dividing this quaternion by Q
  MQuaternion operator*(const MQuaternion& Q) const;
  //! Division operator, return a new quaternion by dividing this quaternion by Q
  MQuaternion operator/(const MQuaternion& Q) const;
  
  //! Addition operator, add Q to this instance
  MQuaternion& operator+=(const MQuaternion& Q);
  //! Subtraction operator, subtract Q from this instance
  MQuaternion& operator-=(const MQuaternion& Q);
  //! Multiplication operator - multiply this quaternion by Q
  MQuaternion& operator*=(const MQuaternion& Q);
  //! Division operator - divide this quaternion by Q
  MQuaternion& operator/=(const MQuaternion& Q);
  

  //! Return true if the quaternions are not equal
  bool operator != (const MQuaternion& Q) const;
  //! Return true if the quaternions are equal
  bool operator == (const MQuaternion& Q) const;
  
  //! Returns a new vector, which is the negative of this one
  MQuaternion operator-() const { return MQuaternion(-m_W, -m_X, -m_Y, -m_Z); }
  
  //! Get the Norm of the quaternion
  double GetNorm() const;
  //! Get the magnitude of the quaternion
  double GetMagnitude() const;
  //! Return a new quaternion by scaling this quaternion with S
  MQuaternion GetScale(const double S) const;
  //! Return the inverse of this quaternion
  MQuaternion GetInverse() const;
  //! Return the conjugate of this quaternion
  MQuaternion GetConjugate() const;
  //! Return the unit quaternion of this quaternion
  MQuaternion GetUnitQuaternion() const;
  //! Return a rotation matrix
  MRotation GetRotation() const;

  
  //! Dot product between Q1 and Q2
  double GetDotProduct(const MQuaternion& Q1, const MQuaternion& Q2) const { return Q1.m_W*Q2.m_W + Q1.m_X*Q2.m_X + Q1.m_Y*Q2.m_Y + Q1.m_Z*Q2.m_Z; }
  
  //! Linear Interpolation between two quaternions, T is the distance between Q1 and Q2 in [0, 1]
  MQuaternion GetLerp(const MQuaternion& Q1, const MQuaternion& Q2, const double T) const;
  //! Spherical linear interpolation between two quaternions, T is the distance between Q1 and Q2 in [0, 1]
  MQuaternion GetSlerp(const MQuaternion& Q1, const MQuaternion& Q2, const double T) const;
 
  //! Get the W component of the quaternion
  double GetW() const { return m_W ;}
  //! Get the X component of the quaternion
  double GetX() const { return m_X; }
  //! Get the Y component of th quaternion
  double GetY() const { return m_Y; }
  //! Get the Z component of the quaternion
  double GetZ() const { return m_Z; }

	
 private:
  //! Angle component of the quaternion
  double m_W;
  //! X component of the quaternion
  double m_X;
  //! Y component of the quaternion
  double m_Y;
  //! Z component of the quaternion
  double m_Z;
  

};

// IO
std::ostream& operator<<(std::ostream& os, const MQuaternion& Q);


#endif

