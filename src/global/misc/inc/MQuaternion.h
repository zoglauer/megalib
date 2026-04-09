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
  //! Copy constructor
  MQuaternion(const MQuaternion& Quaternion);
  //! Default destructor
  virtual ~MQuaternion();
  
  //! Assignment operator
  MQuaternion& operator=(const MQuaternion& Quaternion);

  //! Addition operator, return a new quaternion created by adding Quaternion to this instance
  MQuaternion operator+(const MQuaternion& Quaternion) const;
  //! Subtraction operator, return a new quaternion created by subtracting Quaternion from this instance
  MQuaternion operator-(const MQuaternion& Quaternion) const;
  //! Multiplication operator, return a new quaternion created by multiplying this quaternion with Quaternion
  MQuaternion operator*(const MQuaternion& Quaternion) const;
  //! Division operator, return a new quaternion created by dividing this quaternion by Quaternion
  MQuaternion operator/(const MQuaternion& Quaternion) const;
  
  //! Addition operator, add Quaternion to this instance
  MQuaternion& operator+=(const MQuaternion& Quaternion);
  //! Subtraction operator, subtract Quaternion from this instance
  MQuaternion& operator-=(const MQuaternion& Quaternion);
  //! Multiplication operator, multiply this quaternion by Quaternion
  MQuaternion& operator*=(const MQuaternion& Quaternion);
  //! Division operator, divide this quaternion by Quaternion
  MQuaternion& operator/=(const MQuaternion& Quaternion);
  

  //! Return true if the quaternions are not equal
  bool operator!=(const MQuaternion& Quaternion) const;
  //! Return true if the quaternions are equal
  bool operator==(const MQuaternion& Quaternion) const;
  
  //! Return a new quaternion, which is the negative of this one
  MQuaternion operator-() const { return MQuaternion(-m_W, -m_X, -m_Y, -m_Z); }
  
  //! Return the norm of the quaternion
  double GetNorm() const;
  //! Return the magnitude of the quaternion
  double GetMagnitude() const;
  //! Return a new quaternion by scaling this quaternion with Scalar
  MQuaternion GetScale(const double Scalar) const;
  //! Return the inverse of this quaternion
  MQuaternion GetInverse() const;
  //! Return the conjugate of this quaternion
  MQuaternion GetConjugate() const;
  //! Return the unit quaternion of this quaternion
  MQuaternion GetUnitQuaternion() const;
  //! Return a rotation matrix
  MRotation GetRotation() const;

  
  //! Return the dot product between Quaternion1 and Quaternion2
  double GetDotProduct(const MQuaternion& Quaternion1, const MQuaternion& Quaternion2) const
  {
    return Quaternion1.m_W * Quaternion2.m_W + Quaternion1.m_X * Quaternion2.m_X +
           Quaternion1.m_Y * Quaternion2.m_Y + Quaternion1.m_Z * Quaternion2.m_Z;
  }
  
  //! Return the linear interpolation between Quaternion1 and Quaternion2, with T in [0, 1]
  MQuaternion GetLerp(const MQuaternion& Quaternion1, const MQuaternion& Quaternion2, const double T) const;
  //! Return the spherical linear interpolation between Quaternion1 and Quaternion2, with T in [0, 1]
  MQuaternion GetSlerp(const MQuaternion& Quaternion1, const MQuaternion& Quaternion2, const double T) const;
 
  //! Get the W component of the quaternion
  double GetW() const { return m_W; }
  //! Get the X component of the quaternion
  double GetX() const { return m_X; }
  //! Get the Y component of the quaternion
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

//! Stream a quaternion to an output stream
std::ostream& operator<<(std::ostream& os, const MQuaternion& Q);


#endif
