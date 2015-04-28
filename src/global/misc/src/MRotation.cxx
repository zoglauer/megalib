/*
 * MRotation.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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


////////////////////////////////////////////////////////////////////////////////
//
// MRotation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRotation.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRotation)
#endif


////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
 * Return true if the matrix really is (within tolerances) a rotation matrix
 * i.e. det A = +1, A*A_inv = I
 */
bool MRotation::IsRotation(double Tolerance) const
{
  double Det = GetDeterminant();
  if (fabs(Det) - 1 > Tolerance) return false;
  
  MRotation Inv = GetInvers();
  MRotation Unity = Inv*(*this);
  
  if (fabs(Unity.GetXX() - 1) > Tolerance || 
      fabs(Unity.GetYY() - 1) > Tolerance || 
      fabs(Unity.GetZZ() - 1) > Tolerance) return false;
  
  if (fabs(Unity.GetYX()) > Tolerance || 
      fabs(Unity.GetZX()) > Tolerance || 
      fabs(Unity.GetXY()) > Tolerance || 
      fabs(Unity.GetZY()) > Tolerance || 
      fabs(Unity.GetXZ()) > Tolerance || 
      fabs(Unity.GetYZ()) > Tolerance) return false;
  
  return true;
}
    
    
/******************************************************************************
 * Invert the matrix
 */
const MRotation& MRotation::Invert()
{
  (*this) = GetInvers();
  return (*this);
}


/******************************************************************************
 * Return an inverted matrix without touching this one
 */
MRotation MRotation::GetInvers() const
{
  MRotation New;
  
  double Det = GetDeterminant();
  if (Det == 0) {
    cerr<<"Matrix cannot be inverted, determinant is zero! Returning identity matrix!"<<endl;
  } else {
    New.SetXX(GetSubDeterminant(m_YY, m_ZY, m_YZ, m_ZZ));
    New.SetYX(GetSubDeterminant(m_ZX, m_YX, m_ZZ, m_YZ));
    New.SetZX(GetSubDeterminant(m_YX, m_ZX, m_YY, m_ZY));
    New.SetXY(GetSubDeterminant(m_ZY, m_XY, m_ZZ, m_XZ));
    New.SetYY(GetSubDeterminant(m_XX, m_ZX, m_XZ, m_ZZ));
    New.SetZY(GetSubDeterminant(m_ZX, m_XX, m_ZY, m_XY));
    New.SetXZ(GetSubDeterminant(m_XY, m_YY, m_XZ, m_YZ));
    New.SetYZ(GetSubDeterminant(m_YX, m_XX, m_YZ, m_XZ));
    New.SetZZ(GetSubDeterminant(m_XX, m_YX, m_XY, m_YY));
    New *= Det;
  }
  
  return New;
} 


/******************************************************************************
 * Perform an additional rotation by an angle (in rad) along a vector
 */
void MRotation::Set(const double Angle, const MVector& Vector)
{
  double Length = Vector.Mag();
  
  if (Angle == 0.0 || Length == 0.0) {
    SetIdentity();
    return;
  }

  double sina = sin(Angle);
  double cosa = cos(Angle);
  
  double x = Vector.GetX()/Length;
  double y = Vector.GetY()/Length;
  double z = Vector.GetZ()/Length;

  m_XX = cosa + (1-cosa)*x*x;
  m_XY = (1-cosa)*x*y + z*sina;
  m_XZ = (1-cosa)*x*z - y*sina;
  m_YX = (1-cosa)*x*y - z*sina;
  m_YY = cosa + (1-cosa)*y*y;
  m_YZ = (1-cosa)*y*z + x*sina;
  m_ZX = (1-cosa)*x*z + y*sina;
  m_ZY = (1-cosa)*y*z - x*sina;
  m_ZZ = cosa + (1-cosa)*z*z;
}


/******************************************************************************
 * Perform an additional rotation by an angle around a vector
 */
MRotation& MRotation::Rotate(const double Angle, const MVector& Vector)
{
  MRotation R(Angle, Vector);
   
  (*this) = R*(*this); 
  
  return *this;
}

 
/******************************************************************************
 * Multiply with another materix from the right:
 */
MRotation& MRotation::operator*= (const MRotation& R)
{
  MRotation New;

  New.SetXX(m_XX*R.GetXX() + m_YX*R.GetXY() + m_ZX*R.GetXZ());
  New.SetYX(m_XX*R.GetYX() + m_YX*R.GetYY() + m_ZX*R.GetYZ());
  New.SetZX(m_XX*R.GetZX() + m_YX*R.GetZY() + m_ZX*R.GetZZ());

  New.SetXY(m_XY*R.GetXX() + m_YY*R.GetXY() + m_ZY*R.GetXZ());
  New.SetYY(m_XY*R.GetYX() + m_YY*R.GetYY() + m_ZY*R.GetYZ());
  New.SetZY(m_XY*R.GetZX() + m_YY*R.GetZY() + m_ZY*R.GetZZ());

  New.SetXZ(m_XZ*R.GetXX() + m_YZ*R.GetXY() + m_ZZ*R.GetXZ());
  New.SetYZ(m_XZ*R.GetYX() + m_YZ*R.GetYY() + m_ZZ*R.GetYZ());
  New.SetZZ(m_XZ*R.GetZX() + m_YZ*R.GetZY() + m_ZZ*R.GetZZ());
  
  Set(New);
  
  return *this;
}


/******************************************************************************
 * Check for equality
 */
bool MRotation::operator== (const MRotation& R)
{
  if (m_XX == R.m_XX && m_YX == R.m_YX && m_ZX == R.m_ZX && m_XY == R.m_XY && m_YY == R.m_YY && m_ZY == R.m_ZY && m_XZ == R.m_XZ && m_YZ == R.m_YZ && m_ZZ == R.m_ZZ) {
    return true;
  } else {
    return false;
  }
}

/******************************************************************************
 * A streamer for the rotation class
 */
ostream& operator<<(ostream& out, const MRotation& V)
{
  out<<"("<<V.GetXX()<<"/"<<V.GetYX()<<"/"<<V.GetZX()<<", "<<V.GetXY()<<"/"<<V.GetYY()<<"/"<<V.GetZY()<<", "<<V.GetXZ()<<"/"<<V.GetYZ()<<"/"<<V.GetZZ()<<")";
  return out;
}


/******************************************************************************
 * Multiply with vector from right
 */
MVector operator* (const MRotation& L, const MVector& R)
{
  MVector New;
  
  New.SetX(L.GetXX()*R.GetX() + L.GetYX()*R.GetY() + L.GetZX()*R.GetZ());
  New.SetY(L.GetXY()*R.GetX() + L.GetYY()*R.GetY() + L.GetZY()*R.GetZ());
  New.SetZ(L.GetXZ()*R.GetX() + L.GetYZ()*R.GetY() + L.GetZZ()*R.GetZ());

  return New;
}

 
/******************************************************************************
 * Multiply with matrix from right
 */
MRotation operator* (const MRotation& L, const MRotation& R)
{
  MRotation New;
  
  New.SetXX(L.GetXX()*R.GetXX() + L.GetYX()*R.GetXY() + L.GetZX()*R.GetXZ());
  New.SetYX(L.GetXX()*R.GetYX() + L.GetYX()*R.GetYY() + L.GetZX()*R.GetYZ());
  New.SetZX(L.GetXX()*R.GetZX() + L.GetYX()*R.GetZY() + L.GetZX()*R.GetZZ());

  New.SetXY(L.GetXY()*R.GetXX() + L.GetYY()*R.GetXY() + L.GetZY()*R.GetXZ());
  New.SetYY(L.GetXY()*R.GetYX() + L.GetYY()*R.GetYY() + L.GetZY()*R.GetYZ());
  New.SetZY(L.GetXY()*R.GetZX() + L.GetYY()*R.GetZY() + L.GetZY()*R.GetZZ());

  New.SetXZ(L.GetXZ()*R.GetXX() + L.GetYZ()*R.GetXY() + L.GetZZ()*R.GetXZ());
  New.SetYZ(L.GetXZ()*R.GetYX() + L.GetYZ()*R.GetYY() + L.GetZZ()*R.GetYZ());
  New.SetZZ(L.GetXZ()*R.GetZX() + L.GetYZ()*R.GetZY() + L.GetZZ()*R.GetZZ());
  
  return New;
}


// MRotation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
