//**********************************************************
//* quaternion.h                                           *
//*                                                        *
//* Implementaion for a generalized quaternion class       *   
//*                                                        *
//* Written 1.25.00 by Angela Bennett and implemented in   *
//* MEGAlib by Carolyn Kierans	                           *
//**********************************************************


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

class MQuaternion
{

 public:
  
  //! default constructor - a new quaternion with all parts equal to zero
  MQuaternion(void);
  //! constructor creates a new quaternion based on the elements passed in
  MQuaternion(double wi, double xi, double yi, double zi);
  //! constructor creates a new quaternion based on the 4D vector elements passed in
  MQuaternion(double v[4]);
  //! Copy Constructor
  MQuaternion(const MQuaternion& q); 
  //! Default Destructor
  virtual ~MQuaternion();
  //! = operator - when called on quaternion q2 sets q2 to be an object of  q3 
  MQuaternion operator = (const MQuaternion& q);
  //! + operator - when called on quaternion q2 adds q1 + q2 and returns the sum in a new quaternion
  MQuaternion operator + (const MQuaternion& q);
  //! - operator - when called on q1 subtracts q1 - q2 and returns the difference as a new quaternion
  MQuaternion operator - (const MQuaternion& q);
  //! * operator - when called on a quaternion q2, multiplies q2 *q1  and returns the product in a new quaternion 
  MQuaternion operator * (const MQuaternion& q);
  //! / operator - divide q1 by q2 and returns the quotient as q1 
  MQuaternion operator / (MQuaternion& q);
  //! += operator - when called on quaternion q3 adds q1 and q3 and returns the sum as q3 
  MQuaternion& operator += (const MQuaternion& q);
  //! -= operator - when called on quaternion q3, subtracts q1 from q3 and returns the difference as q3
  MQuaternion& operator -= (const MQuaternion& q);
  //! *= operator - when called on quaternion q3, multiplies q3 by q1 and returns the product as q3
  MQuaternion& operator *= (const MQuaternion& q);
  //! /= operator - when called on quaternion q3, divides q3 by q1 and returns the quotient as q3
  MQuaternion& operator /= (MQuaternion& q);
  //! << operator - prints out a quaternion by it's components
  friend inline ostream& operator << (ostream& output, const MQuaternion& q)
    {
      output << "[" << q.w << ", " << "(" << q.x << ", " << q.y << ", " << q.z << ")]";
      return output; 
    }
  //! != operator - determines if q1 and q2 and equal
  bool operator != (const MQuaternion& q);
  //! == operator - determines if q1 and q2 and equal
  bool operator == (const MQuaternion& q);  
  //! negative of quaternion
  MQuaternion operator -() { return MQuaternion(-w, -x, -y, -z); }
  //! norm - when called on a quaternion object q, returns the norm of q
  double norm();
  //! magnitude - when called on a quaternion object q, returns the magnitude q (sqrt of norm)
  double magnitude();
  //! scale - returns the original quaternion with each part, w,x,y,z, multiplied by some scalar s
  MQuaternion scale(double s);
  //! inverse - when called on a quaternion object q, returns the inverse of q
  MQuaternion inverse();
  //! conjugate - when called on a quaternion object q, returns the conjugate of q
  MQuaternion conjugate();
  //! UnitQuaternion - when called on quaterion q, takes q and returns the unit quaternion of q (each element divided by magntidue)
  MQuaternion UnitQuaternion();
  //! Vector rotataion - when given a  3D vector, v, rotates v by the quaternion
  void QuatRotation(double v[3]);
  //! Calcuate Quaternion from Rotation Matrix
  MQuaternion CalculateQuaternionFromMatrix(MRotation& a);
  // Calcualte Rotation Matrix from Quaternion
  MRotation QuaternionToMatrix();
  //! Dot product between two Quaternions
  double dot(MQuaternion& q1, MQuaternion& q2) { return q1.w*q2.w + q1.x*q2.x + q1.y*q2.y + q1.z*q2.z; }
  //! Linear Interpolation
  MQuaternion Lerp(MQuaternion& q1, MQuaternion& q2, double t) { return (q1.scale(1-t) + q2.scale(t)).UnitQuaternion(); }
  //! Spherical linear interpolation
  MQuaternion Slerp(MQuaternion& q1, MQuaternion& q2, double t);
 
  //! Get the W component of MQuaternion
  double GetW() const { return w ;}
  //! Get the X component of MQuaternion
  double GetX() const { return x; }
  //! Get the Y component of MQuaternion
  double GetY() const { return y; }
  //! Get the Z component of MQuaternion
  double GetZ() const { return z; }

	
 private:
  // [w, (x, y, z)]
  double w, x, y, z;
  

};

#endif

