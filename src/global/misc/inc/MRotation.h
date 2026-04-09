/*
 * MRotation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRotation__
#define __MRotation__


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
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A 3x3 rotation matrix
//! The component layout is:
//! | XX YX ZX |
//! | XY YY ZY |
//! | XZ YZ ZZ |
class MRotation
{
  
  // public interface:
public:
  //! Default constructor --- creates a unity matrix
  MRotation() : m_XX(1), m_XY(0), m_XZ(0), m_YX(0), m_YY(1), m_YZ(0), m_ZX(0), m_ZY(0), m_ZZ(1) {}
  //! Constructor --- set all elements individually
  MRotation(double XX, double YX, double ZX, double XY, double YY, double ZY, double XZ, double YZ, double ZZ) :
    m_XX(XX), m_XY(XY), m_XZ(XZ), m_YX(YX), m_YY(YY), m_YZ(YZ), m_ZX(ZX), m_ZY(ZY), m_ZZ(ZZ) {} 
  //! Constructor --- rotate unity matrix by an angle (in rad) along a vector
  MRotation(double Angle, const MVector& Vector) { Set(Angle, Vector); }
  //! Copy constructor
  MRotation(const MRotation& R) { Set(R); }
  //! Default destructor
  ~MRotation() {}
  
  //! Set the identity matrix
  void SetIdentity() { m_XX = 1.0; m_YX = 0.0; m_ZX = 0.0; m_XY = 0.0; m_YY = 1.0; m_ZY = 0.0; m_XZ = 0.0; m_YZ = 0.0; m_ZZ = 1.0; }
  //! Set a matrix by rotating the identity matrix by an angle (in rad) along a vector
  void Set(double Angle, const MVector& Vector);
  //! Set the matrix elements
  void Set(double XX, double YX, double ZX, double XY, double YY, double ZY, double XZ, double YZ, double ZZ) {
    m_XX = XX; m_YX = YX; m_ZX = ZX; m_XY = XY; m_YY = YY; m_ZY = ZY; m_XZ = XZ; m_YZ = YZ; m_ZZ = ZZ; 
  }
  //! Set the matrix from another matrix
  void Set(const MRotation& Rotation) {
    m_XX = Rotation.m_XX; m_YX = Rotation.m_YX; m_ZX = Rotation.m_ZX; m_XY = Rotation.m_XY; m_YY = Rotation.m_YY; m_ZY = Rotation.m_ZY; m_XZ = Rotation.m_XZ; m_YZ = Rotation.m_YZ; m_ZZ = Rotation.m_ZZ;
  }
  
  //! Return the top row, left component
  double GetXX() const { return m_XX; }
  //! Set the top row, left component
  void SetXX(double XX) { m_XX = XX; }
  
  //! Return the center row, left component
  double GetXY() const { return m_XY; }
  //! Set the center row, left component
  void SetXY(double XY) { m_XY = XY; }
  
  //! Return the bottom row, left component
  double GetXZ() const { return m_XZ; }
  //! Set the bottom row, left component
  void SetXZ(double XZ) { m_XZ = XZ; }
  
  //! Return the top row, center component
  double GetYX() const { return m_YX; }
  //! Set the top row, center component
  void SetYX(double YX) { m_YX = YX; }
  
  //! Return the center row, center component
  double GetYY() const { return m_YY; }
  //! Set the center row, center component
  void SetYY(double YY) { m_YY = YY; }
  
  //! Return the bottom row, center component
  double GetYZ() const { return m_YZ; }
  //! Set the bottom row, center component
  void SetYZ(double YZ) { m_YZ = YZ; }
  
  //! Return the top row, right component
  double GetZX() const { return m_ZX; }
  //! Set the top row, right component
  void SetZX(double ZX) { m_ZX = ZX; }
  
  //! Return the center row, right component
  double GetZY() const { return m_ZY; }
  //! Set the center row, right component
  void SetZY(double ZY) { m_ZY = ZY; }
  
  //! Return the bottom row, right component
  double GetZZ() const { return m_ZZ; }
  //! Set the bottom row, right component
  void SetZZ(double ZZ) { m_ZZ = ZZ; }
  
  //! Return the x-Vector
  MVector GetX() const { return MVector(m_XX, m_XY, m_XZ); }
  //! Return the y-Vector
  MVector GetY() const { return MVector(m_YX, m_YY, m_YZ); }
  //! Return the z-Vector
  MVector GetZ() const { return MVector(m_ZX, m_ZY, m_ZZ); }
  
  //! Return the polar angle of the x-axis in an unrotated coordinate system
  double GetThetaX() const;
  //! Return the azimuthal angle of the x-axis in an unrotated coordinate system
  double GetPhiX() const;
  //! Return the polar angle of the y-axis in an unrotated coordinate system
  double GetThetaY() const;
  //! Return the azimuthal angle of the y-axis in an unrotated coordinate system
  double GetPhiY() const;
  //! Return the polar angle of the z-axis in an unrotated coordinate system
  double GetThetaZ() const;
  //! Return the azimuthal angle of the z-axis in an unrotated coordinate system
  double GetPhiZ() const;
  
  //! Assignment operator
  MRotation& operator=(const MRotation& Rotation) {
    m_XX = Rotation.m_XX; m_YX = Rotation.m_YX; m_ZX = Rotation.m_ZX; m_XY = Rotation.m_XY; m_YY = Rotation.m_YY; m_ZY = Rotation.m_ZY; m_XZ = Rotation.m_XZ; m_YZ = Rotation.m_YZ; m_ZZ = Rotation.m_ZZ;
    return *this;
  }
  
  //! Multiply this rotation with a scalar
  MRotation& operator*=(double Scalar) {
    m_XX *= Scalar; m_YX *= Scalar; m_ZX *= Scalar; m_XY *= Scalar; m_YY *= Scalar; m_ZY *= Scalar; m_XZ *= Scalar; m_YZ *= Scalar; m_ZZ *= Scalar;
    return *this;
  }
  //! Multiply with another matrix from the right
  MRotation& operator*=(const MRotation& Rotation);
  
  //! Return true if two matrices are identical
  bool operator==(const MRotation& Rotation);

  
  //! Invert the matrix
  const MRotation& Invert();
  //! Return an inverted matrix without touching this one
  MRotation GetInvers() const;
  
  //! Return the determinant (Rule of Sarrus)
  double GetDeterminant() const {
    return m_XX*m_YY*m_ZZ + m_YX*m_ZY*m_XZ + m_ZX*m_XY*m_YZ - m_XZ*m_YY*m_ZX - m_YZ*m_ZY*m_XX - m_ZZ*m_XY*m_YX;
  }
  
  //! Return true if the matrix really is (within tolerances) a rotation matrix
  //! i.e. det A = +1, A*A_inv = I
  bool IsRotation(double Tolerance = 1E-6) const;
  
  //! Perform an additional rotation by an angle around a vector
  MRotation& Rotate(double Angle, const MVector& Vector);

  //! Rotate a vector
  void Rotate(MVector& Vector) {
    double X = m_XX*Vector.m_X + m_YX*Vector.m_Y + m_ZX*Vector.m_Z;
    double Y = m_XY*Vector.m_X + m_YY*Vector.m_Y + m_ZY*Vector.m_Z;
    double Z = m_XZ*Vector.m_X + m_YZ*Vector.m_Y + m_ZZ*Vector.m_Z;
    
    Vector.m_X = X;
    Vector.m_Y = Y;
    Vector.m_Z = Z;
  }

  
  // protected methods:
protected:
  //! Return the determinant of a 2x2 matrix used by Invert
  //! The matrix looks like:   | XX YX |
  //!                          | XY YY |
  double GetSubDeterminant(double XX, double YX, double XY, double YY) const { return XX*YY - XY*YX; }
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! Top row, left
  double m_XX;
  //! Center row, left
  double m_XY;
  //! Bottom row, left
  double m_XZ;
  //! Top row, center
  double m_YX;
  //! Center row, center
  double m_YY;
  //! Bottom row, center
  double m_YZ;
  //! Top row, right
  double m_ZX;
  //! Center row, right
  double m_ZY;
  //! Bottom row, right
  double m_ZZ;
  
};



// Several external functions:

//! Stream a rotation to an output stream
ostream& operator<<(ostream& out, const MRotation& V);

//! Multiply a matrix with a vector from the right
MVector operator*(const MRotation& L, const MVector& R);
//! Multiply two matrices
MRotation operator*(const MRotation& L, const MRotation& R);



#endif


////////////////////////////////////////////////////////////////////////////////
