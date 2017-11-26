//*********************************************************
//* quaternion.c++                                        *
//*                                                       *
//* Implementaion for a generalized quaternion class      *   
//*                                                       *
//* Written 1.25.00 by Angela Bennett and implemented in  *
//* MEGAlib by Carolyn Kierans 	                          *
//*********************************************************

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


#ifdef ___CINT___
ClassImp(MQuaternion)
#endif


///////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(void)
{
  // Construct an instance of MQuaternion
  x = 0.0;
  y = 0.0;
  z = 0.0;
  w = 0.0;
}


///////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(double wi, double xi, double yi, double zi)
{
  // Construct an instance of MQuaternion with parametes : x, y, z, w elements of the quaternion
  w = wi;
  x = xi;
  y = yi;
  z = zi;
}


/////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(double v[4])
{
  //Construct an instance of MQuaternion with parameters : vector/array of four elements
  w = v[0];
  x = v[1];
  y = v[2];
  z = v[3];
}


///////////////////////////////////////////////////////////////////


MQuaternion::MQuaternion(const MQuaternion& q)
{
  //Copy Constructor
  w = q.w;
  x = q.x;
  y = q.y;
  z = q.z;
} 


//////////////////////////////////////////////////////////////////////


MQuaternion::~MQuaternion()
{
  //Destructor
}

/////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::operator = (const MQuaternion& q)
{
  // = operator
  w = q.w;
  x = q.x;
  y = q.y;
  z = q.z;
  
  return (*this);
}


/////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::operator + (const MQuaternion& q)
{
  //+ operator
  return MQuaternion(w+q.w, x+q.x, y+q.y, z+q.z);
}

 
/////////////////////////////////////////////////////////////////////

 
MQuaternion MQuaternion::operator - (const MQuaternion& q)
{
  // - operator
  return MQuaternion(w-q.w, x-q.x, y-q.y, z-q.z);
}


//////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::operator * (const MQuaternion& q)
{
  // * operator
  return MQuaternion(
   w*q.w - x*q.x - y*q.y - z*q.z, 
   w*q.x + x*q.w + y*q.z - z*q.y,                          
   w*q.y + y*q.w + z*q.x - x*q.z,
   w*q.z + z*q.w + x*q.y - y*q.x);
}

 
/////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::operator / (MQuaternion& q)
{
  // / operator
  return ((*this) * (q.inverse()));
}


////////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator += (const MQuaternion& q)
{
  // += operator
  w += q.w;
  x += q.x;
  y += q.y;
  z += q.z;

  return (*this);
}


//////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator -= (const MQuaternion& q)
{
  // -= operator
  w -= q.w;
  x -= q.x;
  y -= q.y;
  z -= q.z;

  return (*this);
}


/////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator *= (const MQuaternion& q)
{

  // *= operator
  double w_val = w*q.w - x*q.x - y*q.y - z*q.z;
  double x_val = w*q.x + x*q.w + y*q.z - z*q.y; 
  double y_val = w*q.y + y*q.w + z*q.x - x*q.z;
  double z_val = w*q.z + z*q.w + x*q.y - y*q.x; 
  
  w = w_val;
  x = x_val;
  y = y_val;
  z = z_val;

  return (*this);
}


//////////////////////////////////////////////////////////////////


MQuaternion& MQuaternion::operator /= (MQuaternion& q)
{
  // /= operator
  (*this) = (*this)*q.inverse();
  return (*this);
}


/////////////////////////////////////////////////////////////////


bool MQuaternion::operator != (const MQuaternion& q)
{
  // != operator
  return (w!=q.w || x!=q.x || y!=q.y || z!=q.z) ? true : false;
}


//////////////////////////////////////////////////////////////////


bool MQuaternion::operator == (const MQuaternion& q)
{
  // ==  operator
  return (w==q.w && x==q.x && y==q.y && z==q.z) ? true : false;
}  


//////////////////////////////////////////////////////////////////

double MQuaternion::norm()
{
  //Returns the Norm of MQuaternion as double
  return (w*w + x*x + y*y + z*z);  
}


///////////////////////////////////////////////////////////////////


double MQuaternion::magnitude()
{
  //Returns Magnitude of MQuaternion as double
  return sqrt(norm());
}


///////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::scale(double  s)
{
  //Scale each component of MQuaternion by double s
  return MQuaternion(w*s, x*s, y*s, z*s);
}


///////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::inverse()
{
  //Returns inverse of MQuaternion
  return conjugate().scale(1/norm());
}


///////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::conjugate()
{
  //Returns Conjugate of MQuaternion
  return MQuaternion(w, -x, -y, -z);
}


///////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::UnitQuaternion()
{
  //Returns Normalized MQuaternion
  return (*this).scale(1/(*this).magnitude());
}


///////////////////////////////////////////////////////////////////


void MQuaternion::QuatRotation(double v[3])
{
  //Rotates 3D Vector V by this quaternion
  MQuaternion qv(0, v[0], v[1], v[2]);
  MQuaternion qm = (*this) * qv * (*this).inverse();
  
  v[0] = qm.x;
  v[1] = qm.y;
  v[2] = qm.z;  
}



//////////////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::CalculateQuaternionFromMatrix(MRotation& a) 
{
	//MQuaternion q;
        double trace = a.GetXX() + a.GetYY() + a.GetZZ();
        if( trace > 0 ) {
                double s = 2.0 * sqrt(trace+ 1.0);
                w = 0.25 * s;
                x = ( a.GetYZ() - a.GetZY() ) / s;
                y = ( a.GetZX() - a.GetXZ() ) / s;
                z = ( a.GetXY() - a.GetYX() ) / s;
        } else {
                if ( a.GetXX() > a.GetYY() && a.GetXX() > a.GetZZ() ) {
                        double s = 2.0 * sqrt( 1.0 + a.GetXX() - a.GetYY() - a.GetZZ());
                        w = (a.GetYZ() - a.GetZY() ) / s;
                        x = 0.25 * s;
                        y = (a.GetYX() + a.GetXY() ) / s;
                        z = (a.GetZX() + a.GetXZ() ) / s;
                } else if (a.GetYY() > a.GetZZ()) {
                        double s = 2.0 * sqrt( 1.0 + a.GetYY() - a.GetXX() - a.GetZZ());
                        w = (a.GetZX() - a.GetXZ() ) / s;
                        x = (a.GetXY() + a.GetYX() ) / s;
                        y = 0.25 * s;
                        z = (a.GetZY() + a.GetYZ() ) / s;
                } else {
                        double s = 2.0 * sqrt( 1.0 + a.GetZZ() - a.GetXX() - a.GetYY() );
                        w = (a.GetXY() - a.GetYX() ) / s;
                        x = (a.GetZX() + a.GetXZ() ) / s;
                        y = (a.GetZY() + a.GetYZ() ) / s;
                        z = 0.25 * s;
                }
        }
	return MQuaternion(w, x, y, z);;
}


///////////////////////////////////////////////////////////////////////////


MRotation MQuaternion::QuaternionToMatrix() {

	double s = w*w + x*x + y*y + z*z;
        s = 1./s;
	MRotation R( 1 - 2*s*(y*y + z*z), 2*s*(x*y - w*z), 2*s*(x*z + w*y),
		2*s*(x*y + w*z), 1 - 2*s*(x*x + z*z), 2*s*(y*z - w*x), 
		2*s*(x*z - w*y), 2*s*(y*z + w*x), 1 - 2*s*(x*x + y*y));

        return R;
}


////////////////////////////////////////////////////////////////////////////


MQuaternion MQuaternion::Slerp(MQuaternion& q1, MQuaternion& q2, double t)
{
	MQuaternion q3;
	double dot = MQuaternion::dot(q1, q2);
	if (dot < 0) {
		dot = -dot;
		q3 = -q2;
	} else q3 = q2;
	if (dot < 0.95) {
		double angle = acos(dot);
		return (q1.scale(sin(angle*(1-t))) + q3.scale(sin(angle*t))).scale(1/sin(angle));

	} else // if the angle is small, use linear interpolation                                                               

	return Lerp(q1,q3,t);
}

// MQuaternion.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
