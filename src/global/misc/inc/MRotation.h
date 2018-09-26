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


/** 
 * A 3x3 rotation matrix
 *
 * The component layout is the following:
 *  | XX YX ZX |
 *  | XY YY ZY |
 *  | XZ YZ ZZ |
 *
 */

class MRotation {
  
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
  //! Set the matric from another matrix
  void Set(const MRotation& R) { 
    m_XX = R.m_XX; m_YX = R.m_YX; m_ZX = R.m_ZX; m_XY = R.m_XY; m_YY = R.m_YY; m_ZY = R.m_ZY; m_XZ = R.m_XZ; m_YZ = R.m_YZ; m_ZZ = R.m_ZZ; 
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
  
  //! Return the polar angle of the x-axis in an unrotated coordiante system 
  double GetThetaX() const;
  //! Return the azimuthal angle of the x-axis in an unrotated coordiante system 
  double GetPhiX() const;
  //! Return the polar angle of the y-axis in an unrotated coordiante system 
  double GetThetaY() const;
  //! Return the azimuthal angle of the y-axis in an unrotated coordiante system 
  double GetPhiY() const;
  //! Return the polar angle of the z-axis in an unrotated coordiante system 
  double GetThetaZ() const;
  //! Return the azimuthal angle of the z-axis in an unrotated coordiante system 
  double GetPhiZ() const;
  
  //! Assignments
  MRotation& operator= (const MRotation& R) { 
    m_XX = R.m_XX; m_YX = R.m_YX; m_ZX = R.m_ZX; m_XY = R.m_XY; m_YY = R.m_YY; m_ZY = R.m_ZY; m_XZ = R.m_XZ; m_YZ = R.m_YZ; m_ZZ = R.m_ZZ; 
    return *this; 
  }
  
  //! Multiply this rotation with a number 
  MRotation& operator*= (double N) { 
    m_XX *= N; m_YX *= N; m_ZX *= N; m_XY *= N; m_YY *= N; m_ZY *= N; m_XZ *= N; m_YZ *= N; m_ZZ *= N; 
    return *this; 
  }
  //! Multiply with another materix from the right:
  MRotation& operator*= (const MRotation& R);
  
  // Check for equality
  bool operator== (const MRotation& R);

  
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
  MRotation& Rotate(const double Angle, const MVector& Vector);

  //! Rotate a vector
  void Rotate(MVector& R) {
    double X = m_XX*R.m_X + m_YX*R.m_Y + m_ZX*R.m_Z;
    double Y = m_XY*R.m_X + m_YY*R.m_Y + m_ZY*R.m_Z;
    double Z = m_XZ*R.m_X + m_YZ*R.m_Y + m_ZZ*R.m_Z;
    
    R.m_X = X;
    R.m_Y = Y;
    R.m_Z = Z;
  }

  
  // protected methods:
protected:
  //! Return the determinant of a 2x2 matrix (used by Invert)
  //! The matrixs looks like:  | XX YX | 
  //!                          | XY YY |
  double GetSubDeterminant(double XX, double YX, double XY, double YY) const { return XX*YY - XY*YX; }
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! top row, left 
  double m_XX;
  //! center row, left 
  double m_XY;
  //! bottom row, left 
  double m_XZ;
  //! top row, center 
  double m_YX;
  //! center row, center 
  double m_YY;
  //! bottom row, center 
  double m_YZ;
  //! top row, right 
  double m_ZX;
  //! center row, right 
  double m_ZY;
  //! bottom row, right 
  double m_ZZ;
  
};



// Several external functions:

// A streamer for the vector
ostream& operator<<(ostream& out, const MRotation& V);

// Multiply with vector from right
MVector operator* (const MRotation& L, const MVector& R);
// Multiply with rotation matrix from right
MRotation operator* (const MRotation& L, const MRotation& R);



#endif


////////////////////////////////////////////////////////////////////////////////
