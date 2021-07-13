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


MQuaternion::MQuaternion(void)
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


MQuaternion::MQuaternion(const MQuaternion& Q)
{
  m_W = Q.m_W;
  m_X = Q.m_X;
  m_Y = Q.m_Y;
  m_Z = Q.m_Z;
} 


///////////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(const MRotation& Rotation) 
{
  double Trace = Rotation.GetXX() + Rotation.GetYY() + Rotation.GetZZ();
  if (Trace > 0) {
    double Scale = 2.0 * sqrt(Trace + 1.0);
    m_W = 0.25 * Scale;
    m_X = ( Rotation.GetYZ() - Rotation.GetZY() ) / Scale;
    m_Y = ( Rotation.GetZX() - Rotation.GetXZ() ) / Scale;
    m_Z = ( Rotation.GetXY() - Rotation.GetYX() ) / Scale;
  } else {
    if ( Rotation.GetXX() > Rotation.GetYY() && Rotation.GetXX() > Rotation.GetZZ() ) {
      double Scale = 2.0 * sqrt( 1.0 + Rotation.GetXX() - Rotation.GetYY() - Rotation.GetZZ());
      m_W = (Rotation.GetYZ() - Rotation.GetZY() ) / Scale;
      m_X = 0.25 * Scale;
      m_Y = (Rotation.GetYX() + Rotation.GetXY() ) / Scale;
      m_Z = (Rotation.GetZX() + Rotation.GetXZ() ) / Scale;
    } else if (Rotation.GetYY() > Rotation.GetZZ()) {
      double Scale = 2.0 * sqrt( 1.0 + Rotation.GetYY() - Rotation.GetXX() - Rotation.GetZZ());
      m_W = (Rotation.GetZX() - Rotation.GetXZ() ) / Scale;
      m_X = (Rotation.GetXY() + Rotation.GetYX() ) / Scale;
      m_Y = 0.25 * Scale;
      m_Z = (Rotation.GetZY() + Rotation.GetYZ() ) / Scale;
    } else {
      double Scale = 2.0 * sqrt( 1.0 + Rotation.GetZZ() - Rotation.GetXX() - Rotation.GetYY() );
      m_W = (Rotation.GetXY() - Rotation.GetYX() ) / Scale;
      m_X = (Rotation.GetZX() + Rotation.GetXZ() ) / Scale;
      m_Y = (Rotation.GetZY() + Rotation.GetYZ() ) / Scale;
      m_Z = 0.25 * Scale;
    }
  }
}


//////////////////////////////////////////////////////////////////////


MQuaternion::~MQuaternion()
{
  //Destructor
}


/////////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator = (const MQuaternion& Q)
{
  m_W = Q.m_W;
  m_X = Q.m_X;
  m_Y = Q.m_Y;
  m_Z = Q.m_Z;
  
  return (*this);
}


/////////////////////////////////////////////////////////////////////


//! Addition operator, return a new quaternion created by the addition of Q and this instance
MQuaternion MQuaternion::operator+(const MQuaternion& Q) const
{
  return MQuaternion(m_W + Q.m_W, m_X + Q.m_X, m_Y + Q.m_Y, m_Z + Q.m_Z);
}

 
/////////////////////////////////////////////////////////////////////

 
//! Subtraction operator, return a new quaternion created by the the subtraction of Q from this instance
MQuaternion MQuaternion::operator-(const MQuaternion& Q) const
{
  return MQuaternion(m_W - Q.m_W, m_X - Q.m_X, m_Y - Q.m_Y, m_Z - Q.m_Z);
}


//////////////////////////////////////////////////////////////////////


//! Division operator, return a new quaternion by dividing this quaternion by Q
MQuaternion MQuaternion::operator*(const MQuaternion& Q) const
{
  return MQuaternion(m_W*Q.m_W - m_X*Q.m_X - m_Y*Q.m_Y - m_Z*Q.m_Z, 
                     m_W*Q.m_X + m_X*Q.m_W + m_Y*Q.m_Z - m_Z*Q.m_Y,                          
                     m_W*Q.m_Y + m_Y*Q.m_W + m_Z*Q.m_X - m_X*Q.m_Z,
                     m_W*Q.m_Z + m_Z*Q.m_W + m_X*Q.m_Y - m_Y*Q.m_X);
}

 
/////////////////////////////////////////////////////////////////////


//! Division operator, return a new quaternion by dividing this quaternion by Q
MQuaternion MQuaternion::operator/(const MQuaternion& Q) const
{
  return ((*this) * (Q.GetInverse()));
}


////////////////////////////////////////////////////////////////////


//! Addition operator, add Q to this instance
MQuaternion& MQuaternion::operator+=(const MQuaternion& Q)
{
  m_W += Q.m_W;
  m_X += Q.m_X;
  m_Y += Q.m_Y;
  m_Z += Q.m_Z;

  return (*this);
}


//////////////////////////////////////////////////////////////////


//! Subtraction operator, subtract Q from this instance
MQuaternion& MQuaternion::operator-=(const MQuaternion& Q)
{
  m_W -= Q.m_W;
  m_X -= Q.m_X;
  m_Y -= Q.m_Y;
  m_Z -= Q.m_Z;

  return (*this);
}


/////////////////////////////////////////////////////////////////


//! Multiplication operator - multiply this quaternion by Q
MQuaternion& MQuaternion::operator*=(const MQuaternion& Q)
{
  double w_val = m_W*Q.m_W - m_X*Q.m_X - m_Y*Q.m_Y - m_Z*Q.m_Z;
  double x_val = m_W*Q.m_X + m_X*Q.m_W + m_Y*Q.m_Z - m_Z*Q.m_Y; 
  double y_val = m_W*Q.m_Y + m_Y*Q.m_W + m_Z*Q.m_X - m_X*Q.m_Z;
  double z_val = m_W*Q.m_Z + m_Z*Q.m_W + m_X*Q.m_Y - m_Y*Q.m_X; 
  
  m_W = w_val;
  m_X = x_val;
  m_Y = y_val;
  m_Z = z_val;

  return (*this);
}


//////////////////////////////////////////////////////////////////


//! Division operator - divide this quaternion by Q
MQuaternion& MQuaternion::operator/=(const MQuaternion& Q)
{
  (*this) = (*this)*Q.GetInverse();
  return (*this);
}


/////////////////////////////////////////////////////////////////


//! Return true if the quaternions are not equal
bool MQuaternion::operator!=(const MQuaternion& Q) const
{
  return (m_W != Q.m_W || m_X != Q.m_X || m_Y != Q.m_Y || m_Z != Q.m_Z) ? true : false;
}


//////////////////////////////////////////////////////////////////


//! Return true if the quaternions are  equal
bool MQuaternion::operator==(const MQuaternion& Q) const
{
  return (m_W == Q.m_W && m_X == Q.m_X && m_Y == Q.m_Y && m_Z == Q.m_Z) ? true : false;
}  


//////////////////////////////////////////////////////////////////


//! Get the Norm of the quaternion
double MQuaternion::GetNorm() const
{
  return (m_W*m_W + m_X*m_X + m_Y*m_Y + m_Z*m_Z);  
}


///////////////////////////////////////////////////////////////////


//! Get the magnitude of the quaternion
double MQuaternion::GetMagnitude() const
{
  return sqrt(GetNorm());
}


///////////////////////////////////////////////////////////////////


//! Return a new quaternion by scaling this quaternion with S
MQuaternion MQuaternion::GetScale(const double S) const
{
  return MQuaternion(m_W*S, m_X*S, m_Y*S, m_Z*S);
}


///////////////////////////////////////////////////////////////////


//! Return the inverse of this quaternion
MQuaternion MQuaternion::GetInverse() const
{
  if (GetNorm() == 0) {
    merr<<"Cannot generate the inverse of a quaternion with norm 0"<<endl;
    return MQuaternion();
  }
  
  return GetConjugate().GetScale(1/GetNorm());
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
    return GetScale(1/GetMagnitude());
  } else {
    merr<<"Magnitude of the quaternion is zero -> cannot get unit quaternion"<<endl;
    return MQuaternion();
  }
}


///////////////////////////////////////////////////////////////////


//! Return a rotation matrix
MRotation MQuaternion::GetRotation() const
{  
  double Scale = m_W*m_W + m_X*m_X + m_Y*m_Y + m_Z*m_Z;
  if (Scale == 0) {
    merr<<"Magnitude of the quaternion is zero -> cannot convert it to a rotation "<<endl;
    return MRotation();
  }
  Scale = 1.0/Scale;
  
  return MRotation(1 - 2*Scale*(m_Y*m_Y + m_Z*m_Z), 2*Scale*(m_X*m_Y - m_W*m_Z), 2*Scale*(m_X*m_Z + m_W*m_Y),
                  2*Scale*(m_X*m_Y + m_W*m_Z), 1 - 2*Scale*(m_X*m_X + m_Z*m_Z), 2*Scale*(m_Y*m_Z - m_W*m_X), 
                  2*Scale*(m_X*m_Z - m_W*m_Y), 2*Scale*(m_Y*m_Z + m_W*m_X), 1 - 2*Scale*(m_X*m_X + m_Y*m_Y));
}


////////////////////////////////////////////////////////////////////////////


//! Linear Interpolation between two quaternions, T is the distance between Q1 and Q2 in [0, 1]
MQuaternion MQuaternion::GetLerp(const MQuaternion& Q1, const MQuaternion& Q2, double T) const
{ 
  return (Q1.GetScale(1-T) + Q2.GetScale(T)).GetUnitQuaternion(); 
}


////////////////////////////////////////////////////////////////////////////


//! Spherical linear interpolation between two quaternions, T is the distance between Q1 and Q2 in [0, 1]
MQuaternion MQuaternion::GetSlerp(const MQuaternion& Q1, const MQuaternion& Q2, double T) const
{
  MQuaternion Q3;
  double Dot = GetDotProduct(Q1, Q2);
  if (Dot < 0) {
    Dot = -Dot;
    Q3 = -Q2;
  } else {
    Q3 = Q2;
  }
  
  if (Dot < 0.95) {
    double Angle = acos(Dot);
    return (Q1.GetScale(sin(Angle*(1-T))) + Q3.GetScale(sin(Angle*T))).GetScale(1/sin(Angle));
  } else {
    // Linear interpolation in case of small angles                                                           
    return GetLerp(Q1, Q3, T);
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
