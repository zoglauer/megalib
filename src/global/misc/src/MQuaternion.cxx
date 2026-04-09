/*
 * MQuaternion.cxx
 *
 *
 * Copyright (C) by Carolyn Kierans & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header
#include "MQuaternion.h"


// Standard lib:
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstddef>
#include <cstring>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MSystem.h"



///////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MQuaternion)
#endif


///////////////////////////////////////////////////////////////


MQuaternion::MQuaternion()
{
  m_X = 0.0;
  m_Y = 0.0;
  m_Z = 0.0;
  m_W = 0.0;
}


///////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(const double W, const double X, const double Y, const double Z)
{
  m_W = W;
  m_X = X;
  m_Y = Y;
  m_Z = Z;
}


///////////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(const MQuaternion& Quaternion)
{
  m_W = Quaternion.m_W;
  m_X = Quaternion.m_X;
  m_Y = Quaternion.m_Y;
  m_Z = Quaternion.m_Z;
} 


///////////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(const MRotation& Rotation) 
{
  double Trace = Rotation.GetXX() + Rotation.GetYY() + Rotation.GetZZ();
  if (Trace > 0) {
    double Scale = 2.0 * sqrt(Trace + 1.0);
    m_W = 0.25 * Scale;
    m_X = (Rotation.GetYZ() - Rotation.GetZY()) / Scale;
    m_Y = (Rotation.GetZX() - Rotation.GetXZ()) / Scale;
    m_Z = (Rotation.GetXY() - Rotation.GetYX()) / Scale;
  } else {
    if (Rotation.GetXX() > Rotation.GetYY() && Rotation.GetXX() > Rotation.GetZZ()) {
      double Scale = 2.0 * sqrt(1.0 + Rotation.GetXX() - Rotation.GetYY() - Rotation.GetZZ());
      m_W = (Rotation.GetYZ() - Rotation.GetZY()) / Scale;
      m_X = 0.25 * Scale;
      m_Y = (Rotation.GetYX() + Rotation.GetXY()) / Scale;
      m_Z = (Rotation.GetZX() + Rotation.GetXZ()) / Scale;
    } else if (Rotation.GetYY() > Rotation.GetZZ()) {
      double Scale = 2.0 * sqrt(1.0 + Rotation.GetYY() - Rotation.GetXX() - Rotation.GetZZ());
      m_W = (Rotation.GetZX() - Rotation.GetXZ()) / Scale;
      m_X = (Rotation.GetXY() + Rotation.GetYX()) / Scale;
      m_Y = 0.25 * Scale;
      m_Z = (Rotation.GetZY() + Rotation.GetYZ()) / Scale;
    } else {
      double Scale = 2.0 * sqrt(1.0 + Rotation.GetZZ() - Rotation.GetXX() - Rotation.GetYY());
      m_W = (Rotation.GetXY() - Rotation.GetYX()) / Scale;
      m_X = (Rotation.GetZX() + Rotation.GetXZ()) / Scale;
      m_Y = (Rotation.GetZY() + Rotation.GetYZ()) / Scale;
      m_Z = 0.25 * Scale;
    }
  }
}


//////////////////////////////////////////////////////////////////////


MQuaternion::~MQuaternion()
{
  // Destructor
}


/////////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator=(const MQuaternion& Quaternion)
{
  m_W = Quaternion.m_W;
  m_X = Quaternion.m_X;
  m_Y = Quaternion.m_Y;
  m_Z = Quaternion.m_Z;
  
  return *this;
}


/////////////////////////////////////////////////////////////////////


//! Addition operator, return a new quaternion created by the addition of Quaternion and this instance
MQuaternion MQuaternion::operator+(const MQuaternion& Quaternion) const
{
  return MQuaternion(m_W + Quaternion.m_W, m_X + Quaternion.m_X, m_Y + Quaternion.m_Y, m_Z + Quaternion.m_Z);
}

 
/////////////////////////////////////////////////////////////////////

 
//! Subtraction operator, return a new quaternion created by the subtraction of Quaternion from this instance
MQuaternion MQuaternion::operator-(const MQuaternion& Quaternion) const
{
  return MQuaternion(m_W - Quaternion.m_W, m_X - Quaternion.m_X, m_Y - Quaternion.m_Y, m_Z - Quaternion.m_Z);
}


//////////////////////////////////////////////////////////////////////


//! Multiplication operator, return a new quaternion by multiplying this quaternion with Quaternion
MQuaternion MQuaternion::operator*(const MQuaternion& Quaternion) const
{
  return MQuaternion(m_W * Quaternion.m_W - m_X * Quaternion.m_X - m_Y * Quaternion.m_Y - m_Z * Quaternion.m_Z,
                     m_W * Quaternion.m_X + m_X * Quaternion.m_W + m_Y * Quaternion.m_Z - m_Z * Quaternion.m_Y,
                     m_W * Quaternion.m_Y + m_Y * Quaternion.m_W + m_Z * Quaternion.m_X - m_X * Quaternion.m_Z,
                     m_W * Quaternion.m_Z + m_Z * Quaternion.m_W + m_X * Quaternion.m_Y - m_Y * Quaternion.m_X);
}

 
/////////////////////////////////////////////////////////////////////


//! Division operator, return a new quaternion by dividing this quaternion by Quaternion
MQuaternion MQuaternion::operator/(const MQuaternion& Quaternion) const
{
  return (*this) * Quaternion.GetInverse();
}


////////////////////////////////////////////////////////////////////


//! Addition operator, add Quaternion to this instance
MQuaternion& MQuaternion::operator+=(const MQuaternion& Quaternion)
{
  m_W += Quaternion.m_W;
  m_X += Quaternion.m_X;
  m_Y += Quaternion.m_Y;
  m_Z += Quaternion.m_Z;

  return *this;
}


//////////////////////////////////////////////////////////////////


//! Subtraction operator, subtract Quaternion from this instance
MQuaternion& MQuaternion::operator-=(const MQuaternion& Quaternion)
{
  m_W -= Quaternion.m_W;
  m_X -= Quaternion.m_X;
  m_Y -= Quaternion.m_Y;
  m_Z -= Quaternion.m_Z;

  return *this;
}


/////////////////////////////////////////////////////////////////


//! Multiplication operator - multiply this quaternion by Quaternion
MQuaternion& MQuaternion::operator*=(const MQuaternion& Quaternion)
{
  double WValue = m_W * Quaternion.m_W - m_X * Quaternion.m_X - m_Y * Quaternion.m_Y - m_Z * Quaternion.m_Z;
  double XValue = m_W * Quaternion.m_X + m_X * Quaternion.m_W + m_Y * Quaternion.m_Z - m_Z * Quaternion.m_Y;
  double YValue = m_W * Quaternion.m_Y + m_Y * Quaternion.m_W + m_Z * Quaternion.m_X - m_X * Quaternion.m_Z;
  double ZValue = m_W * Quaternion.m_Z + m_Z * Quaternion.m_W + m_X * Quaternion.m_Y - m_Y * Quaternion.m_X;
  
  m_W = WValue;
  m_X = XValue;
  m_Y = YValue;
  m_Z = ZValue;

  return *this;
}


//////////////////////////////////////////////////////////////////


//! Division operator - divide this quaternion by Quaternion
MQuaternion& MQuaternion::operator/=(const MQuaternion& Quaternion)
{
  (*this) = (*this) * Quaternion.GetInverse();
  return *this;
}


/////////////////////////////////////////////////////////////////


//! Return true if the quaternions are not equal
bool MQuaternion::operator!=(const MQuaternion& Quaternion) const
{
  return (m_W != Quaternion.m_W || m_X != Quaternion.m_X ||
          m_Y != Quaternion.m_Y || m_Z != Quaternion.m_Z) ? true : false;
}


//////////////////////////////////////////////////////////////////


//! Return true if the quaternions are equal
bool MQuaternion::operator==(const MQuaternion& Quaternion) const
{
  return (m_W == Quaternion.m_W && m_X == Quaternion.m_X &&
          m_Y == Quaternion.m_Y && m_Z == Quaternion.m_Z) ? true : false;
}  


//////////////////////////////////////////////////////////////////


//! Get the Norm of the quaternion
double MQuaternion::GetNorm() const
{
  return m_W * m_W + m_X * m_X + m_Y * m_Y + m_Z * m_Z;
}


///////////////////////////////////////////////////////////////////


//! Get the magnitude of the quaternion
double MQuaternion::GetMagnitude() const
{
  return sqrt(GetNorm());
}


///////////////////////////////////////////////////////////////////


//! Return a new quaternion by scaling this quaternion with Scalar
MQuaternion MQuaternion::GetScale(const double Scalar) const
{
  return MQuaternion(m_W * Scalar, m_X * Scalar, m_Y * Scalar, m_Z * Scalar);
}


///////////////////////////////////////////////////////////////////


//! Return the inverse of this quaternion
MQuaternion MQuaternion::GetInverse() const
{
  if (GetNorm() == 0) {
    merr<<"Cannot generate the inverse of a quaternion with norm 0"<<endl;
    return MQuaternion();
  }
  
  return GetConjugate().GetScale(1 / GetNorm());
}


///////////////////////////////////////////////////////////////////


//! Return the conjugate of this quaternion
MQuaternion MQuaternion::GetConjugate() const
{
  return MQuaternion(m_W, -m_X, -m_Y, -m_Z);
}


///////////////////////////////////////////////////////////////////


//! Return the unit quaternion of this quaternion
MQuaternion MQuaternion::GetUnitQuaternion() const
{
  if (GetMagnitude() > 0) {
    return GetScale(1 / GetMagnitude());
  } else {
    merr<<"Magnitude of the quaternion is zero -> cannot get unit quaternion"<<endl;
    return MQuaternion();
  }
}


///////////////////////////////////////////////////////////////////


//! Return a rotation matrix
MRotation MQuaternion::GetRotation() const
{  
  double Scale = m_W * m_W + m_X * m_X + m_Y * m_Y + m_Z * m_Z;
  if (Scale == 0) {
    merr<<"Magnitude of the quaternion is zero -> cannot convert it to a rotation "<<endl;
    return MRotation();
  }
  Scale = 1.0 / Scale;
  
  return MRotation(1 - 2 * Scale * (m_Y * m_Y + m_Z * m_Z), 2 * Scale * (m_X * m_Y - m_W * m_Z), 2 * Scale * (m_X * m_Z + m_W * m_Y),
                   2 * Scale * (m_X * m_Y + m_W * m_Z), 1 - 2 * Scale * (m_X * m_X + m_Z * m_Z), 2 * Scale * (m_Y * m_Z - m_W * m_X),
                   2 * Scale * (m_X * m_Z - m_W * m_Y), 2 * Scale * (m_Y * m_Z + m_W * m_X), 1 - 2 * Scale * (m_X * m_X + m_Y * m_Y));
}


////////////////////////////////////////////////////////////////////////////


//! Linear interpolation between two quaternions, T is the distance between Quaternion1 and Quaternion2 in [0, 1]
MQuaternion MQuaternion::GetLerp(const MQuaternion& Quaternion1, const MQuaternion& Quaternion2, double T) const
{ 
  return (Quaternion1.GetScale(1 - T) + Quaternion2.GetScale(T)).GetUnitQuaternion(); 
}


////////////////////////////////////////////////////////////////////////////


//! Spherical linear interpolation between two quaternions, T is the distance between Quaternion1 and Quaternion2 in [0, 1]
MQuaternion MQuaternion::GetSlerp(const MQuaternion& Quaternion1, const MQuaternion& Quaternion2, double T) const
{
  MQuaternion Quaternion3;
  double Dot = GetDotProduct(Quaternion1, Quaternion2);
  if (Dot < 0) {
    Dot = -Dot;
    Quaternion3 = -Quaternion2;
  } else {
    Quaternion3 = Quaternion2;
  }
  
  if (Dot < 0.95) {
    double Angle = acos(Dot);
    return (Quaternion1.GetScale(sin(Angle * (1 - T))) + Quaternion3.GetScale(sin(Angle * T))).GetScale(1 / sin(Angle));
  } else {
    // Linear interpolation in case of small angles
    return GetLerp(Quaternion1, Quaternion3, T);
  }
}


////////////////////////////////////////////////////////////////////////////


//! Quaternion IO
ostream& operator<<(ostream& out, const MQuaternion& Q)
{
  out<<"["<<Q.GetW()<<", "<<"("<<Q.GetX()<<", "<<Q.GetY()<<", "<<Q.GetZ()<<")]";
  return out; 
}


// MQuaternion.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
