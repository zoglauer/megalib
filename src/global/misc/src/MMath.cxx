/*
 * MMath.cxx
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
// MMath
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMath.h"

// Standard libs:
#include <float.h>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MMath)
#endif


////////////////////////////////////////////////////////////////////////////////


MMath::MMath()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MMath::~MMath()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MMath::GalacticToSpheric(double &theta, double &phi)
{
  // --> Bogenmass verwenden!
  // Transform galactic coordinates to spherical (theta and phi are in degree)

  theta = -theta + 90;
  
  phi *= -1;
  //if (phi <= -180) phi += 360.0;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::SphericToGalactic(double &theta, double &phi)
{
  // --> Bogenmass verwenden!
  // Transform Spheric coordinates to Galactic (theta and phi are in degree)

  theta = -theta + 90;
  
  phi *= -1;
  //if (phi < 0) phi += 360.0;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::SphericToCartesean(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ)
{
  // Transform spheric coordinates (theta, phi in deg) into Cartesean coordinates

  double x, y, z;
  x = RadiusIsZ * sin(ThetaIsX*c_Rad) * cos(PhiIsY*c_Rad);
  y = RadiusIsZ * sin(ThetaIsX*c_Rad) * sin(PhiIsY*c_Rad); 
  z = RadiusIsZ * cos(ThetaIsX*c_Rad);

  ThetaIsX = x;
  PhiIsY = y;
  RadiusIsZ = z;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::CarteseanToSpheric(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ)
{
  // Transform spheric coordinates (theta, phi in deg) into Cartesean coordinates

  double t, p, r;
  // Now transform:
  r =  sqrt(ThetaIsX*ThetaIsX + PhiIsY*PhiIsY + RadiusIsZ*RadiusIsZ);

  if (RadiusIsZ == 0) {
    t = c_Pi/2.0;
  } else { 
    t = atan(sqrt(ThetaIsX*ThetaIsX + PhiIsY*PhiIsY)/RadiusIsZ);
    if (RadiusIsZ < 0) t += c_Pi;
  }

  p =  atan(PhiIsY/ThetaIsX); 
  if (ThetaIsX < 0) {
    if (PhiIsY > 0) p += c_Pi;
    if (PhiIsY < 0) p -= c_Pi;
  }
  if (ThetaIsX == 0 && PhiIsY > 0) p = c_Pi/2.0;
  if (ThetaIsX == 0 && PhiIsY < 0) p = -c_Pi/2.0;

  ThetaIsX = t;
  PhiIsY = p;
  RadiusIsZ = r;
}


////////////////////////////////////////////////////////////////////////////////


double MMath::Gauss(const double& x, const double& mean, const double& sigma)
{
  // Calculate a gaussian function with mean and sigma
  // It is slightly different from the original ROOT routine, containing
  // the normalization factor: 1/(sigma*c_Sqrt2Pi)

  if (sigma == 0) return 1.e30;
  double arg = (x-mean)/sigma;
  return 1/(sigma*c_Sqrt2Pi)*exp(-0.5*arg*arg);
} 


////////////////////////////////////////////////////////////////////////////////


bool MMath::InRange(double x)
{ 
  //  

  if (x < DBL_MAX && x > -DBL_MAX) {
    return true; 
  } else {
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////



double MMath::AngleBetweenTwoVectors(const double& u, const double& v, const double& w, 
				      const double& x, const double& y, const double& z)
{
  // Sollte niemand verwenden!!!!!!!!!

  double Nenner = sqrt((u*u + v*v + w*w)*(x*x + y*y + z*z));

  if (Nenner <= 0.0) {
    return 0.0;
  } else {
    Nenner = (u*x + v*y + w*z)/Nenner;
    if (Nenner > 1.0) Nenner = 1.0;
    if (Nenner < -1.0) Nenner = -1.0;
    return acos(Nenner);
  }
}


// MMath.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
