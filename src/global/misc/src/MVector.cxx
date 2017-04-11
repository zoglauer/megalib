/*
 * MVector.cxx
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
// MVector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MVector.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MVector)
#endif


////////////////////////////////////////////////////////////////////////////////


MVector::MVector(double X, double Y, double Z) : m_X(X), m_Y(Y), m_Z(Z) 
{  
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


MVector::MVector(const MVector& V) : m_X(V.m_X), m_Y(V.m_Y), m_Z(V.m_Z) 
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


MVector::~MVector() 
{  
// Itentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


double MVector::operator[] (int i) const 
{
  //! Basic set and get components by index 0..2

  switch(i) {
  case 0:
    return m_X;
  case 1:
    return m_Y;
  case 2:
    return m_Z;
  default:
    merr<<"Bad index ("<<i<<") returning 0"<<endl;
    massert(false);
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


double& MVector::operator[] (int i) 
{
  //! Basic set and get components by index 0..2 as reference!

  switch(i) {
  case 0:
    return m_X;
  case 1:
    return m_Y;
  case 2:
    return m_Z;
  default:
    merr<<"Bad index ("<<i<<") returning &m_X"<<endl;
    massert(false);
  }

  // This is actually very bad... 
  return m_X;
}


////////////////////////////////////////////////////////////////////////////////


bool MVector::operator== (const MVector& V) const 
{
  // Return true if all elements are identical

  if (V.m_X == m_X && V.m_Y == m_Y && V.m_Z == m_Z) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MVector::operator!= (const MVector& V) const 
{
  // Return false if one of the elements is identical

  if (V.m_X != m_X || V.m_Y != m_Y || V.m_Z != m_Z) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MVector::operator<(const MVector& V) const 
{
  // The less operator
  
  if (V.m_X > m_X) return true;
  else if (V.m_X < m_X) return false;
  else {
    if (V.m_Y > m_Y) return true;
    else if (V.m_Y < m_Y) return false;
    else {
      if (V.m_Z > m_Z) return true;
      else return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MVector::SetMagThetaPhi(double Magnitude, double Theta, double Phi) 
{
  // Set in spherical coordinates

  double A = fabs(Magnitude);
  m_X = A*sin(Theta)*cos(Phi);
  m_Y = A*sin(Theta)*sin(Phi);
  m_Z = A*cos(Theta);
}


////////////////////////////////////////////////////////////////////////////////


bool MVector::AreEqual(const MVector& Vector, double Tolerance) const 
{
  // Test if the two vectors are equal wihin a tolerance

  if (fabs(Vector.m_X - m_X) < Tolerance &&
      fabs(Vector.m_Y - m_Y) < Tolerance &&
      fabs(Vector.m_Z - m_Z) < Tolerance) {
    return true;
  }
   
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MVector MVector::Unit() const
{
  // Return a new vector which is unit, i.e. mag=1

  double  Denom = m_X*m_X + m_Y*m_Y + m_Z*m_Z;
  MVector V(m_X, m_Y, m_Z);

  if (Denom > 0.0) {
    return V*(1.0/sqrt(Denom));
  } else {
    return V;
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector& MVector::Unitize()
{
  // Make *this* vector a unit vector

  double mag2 = m_X*m_X + m_Y*m_Y + m_Z*m_Z;
  if (mag2 > 0.0) {
    mag2 = 1.0/sqrt(mag2);
    m_X *= mag2;
    m_Y *= mag2;
    m_Z *= mag2;
  }

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


double MVector::Angle(const MVector& V) const 
{
  // Calculate the angle between two vectors:
  // cos Angle = (v dot w) / (|v| x |w|)

  // Protect against division by zero:
  double Denom = Mag()*V.Mag();
  if (Denom == 0) {
    return 0.0;
  } else {
    double Value = Dot(V)/Denom;
    if (Value >  1.0) Value =  1.0;
    if (Value < -1.0) Value = -1.0;
    return acos(Value);
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MVector::Orthogonal() const 
{
  // Return an orthogonal vector by eliminating the component closest to zero to avoid large rounding errors 
  
  double aX = fabs(m_X);
  double aY = fabs(m_Y);
  double aZ = fabs(m_Z);
  
  if (aX < aY) {
    return aX < aZ ? MVector(0.0, m_Z, -m_Y) : MVector(m_Y, -m_X, 0.0);
  } else {
    return aY < aZ ? MVector(-m_Z, 0.0, m_X) : MVector(m_Y, -m_X, 0.0);
  }
}
      
      
////////////////////////////////////////////////////////////////////////////////


void MVector::RotateX(double Angle) 
{
  // Rotate around x-axis

  double OldY = m_Y;
  // m_X is untouched
  m_Y = cos(Angle)*OldY - sin(Angle)*m_Z;
  m_Z = sin(Angle)*OldY + cos(Angle)*m_Z;
}


////////////////////////////////////////////////////////////////////////////////


void MVector::RotateY(double Angle) 
{
  // Rotate around y-axis

  double OldX = m_X;
  m_X = sin(Angle)*m_Z + cos(Angle)*OldX;
  // m_Y is untouched
  m_Z = cos(Angle)*m_Z - sin(Angle)*OldX;
}


////////////////////////////////////////////////////////////////////////////////


void MVector::RotateReferenceFrame(const MVector& OriginalDir) 
{
  // Rotate to the new reference frame
  
  double a = OriginalDir.m_X;
  double b = OriginalDir.m_Y;
  double c = OriginalDir.m_Z;
  double scalar = a*a + b*b;

  if (scalar != 0) {
    scalar = sqrt(scalar);
    double x = m_X;
    double y = m_Y;
    double z = m_Z;
    m_X = (a*c*x - b*y + a*scalar*z)/scalar;
    m_Y = (b*c*x + a*y + b*scalar*z)/scalar;
    m_Z = (c*c*x -   x + c*scalar*z)/scalar;
  } else if (c < 0.0) {
    // Poles of the sphere 
    m_X = -m_X; 
    m_Z = -m_Z; 
  }
}

////////////////////////////////////////////////////////////////////////////////


void MVector::RotateZ(double Angle) 
{
  // Rotate around z axis

  double OldX = m_X;
  m_X = cos(Angle)*OldX - sin(Angle)*m_Y;
  m_Y = sin(Angle)*OldX + cos(Angle)*m_Y;
  // m_Z is untouched
}


////////////////////////////////////////////////////////////////////////////////


MVector operator+ (const MVector& V, const MVector& W) 
{
  return MVector(V.m_X + W.m_X, V.m_Y + W.m_Y, V.m_Z + W.m_Z);
}


////////////////////////////////////////////////////////////////////////////////


MVector operator- (const MVector& V, const MVector& W) 
{
  return MVector(V.m_X - W.m_X, V.m_Y - W.m_Y, V.m_Z - W.m_Z);
}


////////////////////////////////////////////////////////////////////////////////


MVector operator* (const MVector& V, double S) 
{
  return MVector(S*V.m_X, S*V.m_Y, S*V.m_Z);
}


////////////////////////////////////////////////////////////////////////////////


MVector operator/ (const MVector& V, double S) 
{
  if (S == 0) {
    merr<<"MVector: Division by zero. Returning zero vector!"<<endl;
    return MVector(0.0, 0.0, 0.0);
  }
  return MVector(V.m_X/S, V.m_Y/S, V.m_Z/S);
}


////////////////////////////////////////////////////////////////////////////////


MVector operator* (double S, const MVector& V) 
{
  return MVector(S*V.m_X, S*V.m_Y, S*V.m_Z);
}


////////////////////////////////////////////////////////////////////////////////


double operator* (const MVector& V, const MVector& W) 
{
  return V.Dot(W);
}
  

////////////////////////////////////////////////////////////////////////////////


bool MVector::Coplanar(const MVector& A, const MVector& B, const MVector& C, 
                       double Tolerance) const
{
  // Checks if this vector is in the plane spanned by the other three

  // Determine the normal vector of the plane spanned by ABC:
  MVector Normal = (B-A).Cross(C-A);

  // Now test the other 3 possible planes 
  MVector D = *this;

  MVector TestNormal1 = (C-B).Cross(D-B);
  MVector TestNormal2 = (D-C).Cross(A-C);
  MVector TestNormal3 = (A-D).Cross(B-D);

  double Angle = 0.0;

  Angle = Normal.Angle(TestNormal1);
  if (Angle > c_Pi/2.0) Angle = c_Pi - Angle; 
  if (Angle > Tolerance) {
    mout<<"Not coplanar (1):" <<Angle*c_Deg<<" deg"<<endl;
    return false;
  }
  Angle = Normal.Angle(TestNormal2);
  if (Angle > c_Pi/2.0) Angle = c_Pi - Angle; 
  if (Angle > Tolerance) {
    mout<<"Not coplanar (2):" <<Angle*c_Deg<<" deg"<<endl;
    return false;
  }
  Angle = Normal.Angle(TestNormal3);
  if (Angle > c_Pi/2.0) Angle = c_Pi - Angle; 
  if (Angle > Tolerance) {
    mout<<"Not coplanar (3):" <<Angle*c_Deg<<" deg"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MVector::DistanceToLine(const MVector& A, const MVector& B) const
{
  //! Calculate the distance of a line spanned by the given vectors with this point

  MVector Dir = (B - A).Unit();  
  return ((A - *this) - ((A - *this).Dot(Dir))*Dir).Mag();
}


////////////////////////////////////////////////////////////////////////////////


MString MVector::ToString() const
{
  //! Return a string of the content of this class

  MString S;

  S += "(";
  S += m_X;
  S += ", ";
  S += m_Y;
  S += ", ";
  S += m_Z;
  S += ")";
  
  return S;
}


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const MVector& Vector)
{
  os<<"("<<Vector.X()<<", "<<Vector.Y()<<", "<<Vector.Z()<<")";
  return os;
}


// MVector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
