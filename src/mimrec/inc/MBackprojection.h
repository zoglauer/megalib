/*
 * MBackprojection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojection__
#define __MBackprojection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MProjection.h"
#include "MPhysicalEvent.h"
#include "MResponse.h"
#include "MDGeometryQuest.h"


///////////////////////////////////////////////////////////////////////////////


class MBackprojection : public MProjection
{
  // Public Interface:
 public:
  // The default constructor
  MBackprojection(int CoordinateSystem);
  // The default destructor
  virtual ~MBackprojection();

  //! Choose if you want to use maths approximations
  virtual void SetApproximatedMaths(bool Approximated = true) { m_ApproximatedMaths = Approximated; if (m_Response != 0) m_Response->SetApproximatedMaths(m_ApproximatedMaths); }

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();  
  //virtual bool Backproject(MPhysicalEvent* Event, double* Image) = 0;
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum) = 0;

  //! Set the viewport, i.e. image dimensions
  virtual void SetViewportDimensions(double xMin, double xMax, int xNBins, 
                                     double yMin, double yMax, int yNBins,
                                     double zMin = 0, double zMax = 0, int zNBins = 1,
                                     MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  //! Use absorption for response calculation
  void SetUseAbsorptions(bool Absorptions) { m_UseAbsorptions = Absorptions; }

  //! Set the response
  virtual void SetResponse(MResponse* Response) { m_Response = Response; m_Response->SetApproximatedMaths(m_ApproximatedMaths); }
  //! Get the response
  MResponse* GetResponse() const { return m_Response; }



  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);


  //! Rotate the detector
  virtual void Rotate(double &x, double &y, double &z);

  //! A fast approximation of acos based on Abramowitz and Stegun error < 0.01% if |x| < 0.9998
  float Acos(const float x) {
    if (m_ApproximatedMaths == false) return acos(x);

    if (x < 0.0F) {
      if (x <= -1.0F) return 3.1415926535889793F; 
      return 3.1415926535889793F - sqrt(1.0F + x) * (1.5707288F - x*(-0.2121144F - x*(0.0742610F + x*0.0187293F)));
    }
    if (x >= 1.0F) return 0.0F; 
    return sqrt(1.0F - x) * (1.5707288F + x*(-0.2121144F + x*(0.0742610F - x*0.0187293F)));
  }

  //! A fast optimization of 1/sqrt() after Newton. Error < 0.0005%
  //! Attention: Breaks down at large numbers!
  //! 3x as faster than 1.0/sqrt() on latest Nehalems with gcc -O3 -mtunes=native
  //! Needs to be float since it depends on the IEEE floating point definition
  float InvSqrt(float x) {
    if (m_ApproximatedMaths == false) return 1.0/sqrt(x);

    float xhalf = 0.5f*x;

    // This magic is possible due to the IEEE floating point definition
    int i = *(int*)&x;
    i = 0x5f375a86 - (i>>1);
    x = *(float*)&i;

    // Two Newton stages:
    x = x*(1.5f-xhalf*x*x);
    x = x*(1.5f-xhalf*x*x);

    return x;
  }
  

  //! A cosinus approximation after Abramowitz and Stegun error < 0.0025%
  //! Double's version roughly 3x faster in latest Nehalems with gcc -O3 -mtunes=native
  double Cos(double x) {
    if (m_ApproximatedMaths == false) return cos(x);
   
    static const double pi        = 3.141592653589793;
    
    static const double halfpi    = 0.5 * pi;
    static const double onehalfpi = 1.5 * pi;
    static const double twopi     = 2.0 * pi;
    
    static const double a2 = -0.4999999963;
    static const double a4 = +0.0416666418;
    static const double a6 = -0.0013888387;
    static const double a8 = +0.0000247609;
    static const double a10 = -0.0000002605;
    
    // Get x in rough range:
    while (x < 0.0) x += twopi;
    while (x > twopi) x -= twopi;
    
    // Approximation only valid in 0<=x<=pi/2
    if (x <= halfpi) {
      x *= x;
      return 1 + x*(a2 + x*(a4 + x*(a6 + x*(a8 +x*a10))));
    }
    if (x <= pi) {
      x = (pi-x)*(pi-x);
      return -(1 + x*(a2 + x*(a4 + x*(a6 + x*(a8 +x*a10)))));
    }
    if (x <= onehalfpi) {
      x = (x-pi)*(x-pi);
      return -(1 + x*(a2 + x*(a4 + x*(a6 + x*(a8 +x*a10)))));
    }
    
    x = (twopi-x)*(twopi-x);
    return 1 + x*(a2 + x*(a4 + x*(a6 + x*(a8 +x*a10))));
  }

  //! A cosinus approximation after Abramowitz and Stegun error < 0.0025%
  //! Double's version roughly 5x faster in latest Nehalems with gcc -O3 -mtunes=native
  double Sin(double x) {
    if (m_ApproximatedMaths == false) return sin(x);
  
    static const double pi        = c_Pi;
    
    static const double halfpi    = 0.5 * pi;
    static const double onehalfpi = 1.5 * pi;
    static const double twopi     = 2.0 * pi;
    
    static const double  a2 = -0.1666666664;
    static const double  a4 = +0.0083333315;
    static const double  a6 = -0.0001984090;
    static const double  a8 = +0.0000027526;
    static const double a10 = -0.0000000239;
    
    // Get x in rough range:
    while (x < 0.0f) x += twopi;
    while (x > twopi) x -= twopi;
    
    // Approximation in 0<=x<=pi/2
    double x2;
    if (x <= halfpi) {
      x2 = x*x;
      return x*(1 + x2*(a2 + x2*(a4 + x2*(a6 + x2*(a8 +x2*a10)))));
    }
    if (x <= pi) {
      x = (pi-x);
      x2 = x*x;
      return x*(1 + x2*(a2 + x2*(a4 + x2*(a6 + x2*(a8 +x2*a10)))));
    }
    if ( x <= onehalfpi) {
      x = (x-pi);
      x2 = x*x;
      return -x*(1 + x2*(a2 + x2*(a4 + x2*(a6 + x2*(a8 +x2*a10)))));
    }
    
    x = (twopi-x);
    x2 = x*x;
    
    return -x*(1 + x2*(a2 + x2*(a4 + x2*(a6 + x2*(a8 +x2*a10)))));
  }


  //! Optimized version of calculating an angle
  //! This function is absolutely time critical!
  double Angle(double u, double v, double w, double x, double y, double z) {
    double Nenner = (u*x + v*y + w*z)/sqrt((u*u + v*v + w*w)*(x*x + y*y + z*z));
    
    if (m_ApproximatedMaths == false) {
      if (Nenner > 1.0) return 0.0;
      if (Nenner < -1.0) return c_Pi;
      return acos(Nenner);
    } else {
      // Acos checks if Nenner is within -1 <= x <= 1
      return Acos(Nenner);
    }
  }

  //! Test if a value is not NaN
  bool InRange(double x) {
    if (x < +numeric_limits<double>::max() && 
        x > -numeric_limits<double>::max()) {
      return true; 
    } else {
      return false; 
    }
  }

  // protected members:
 protected:
  //! minimum x (phi) of the viewport
  double m_x1Min;
  //! maximum x (phi) of the viewport
  double m_x1Max;
  //! minimum y (theta) of the viewport
  double m_x2Min; 
  //! maximum y (theta) of the viewport
  double m_x2Max; 
  //! minimum z (radius) of the viewport
  double m_x3Min;
  //! maximum z (radius) of the viewport
  double m_x3Max;

  //! number of bins in x (phi) direction
  int m_x1NBins;                     
  //! number of bins in y (theta) direction
  int m_x2NBins; 
  //! number of bins in z (radius) direction
  int m_x3NBins; 

  //! length of a bin in x (phi) direction
  double m_x1IntervalLength;
  //! length of a bin in y (theta) direction
  double m_x2IntervalLength;
  //! length of a bin in z (radius) direction
  double m_x3IntervalLength;

  //! Total number of bins
  int m_NImageBins; 


  //! The response - Compton as well as pair
  MResponse* m_Response;              

  //! True if absorptions are used during response calculations
  bool m_UseAbsorptions;

  //! The geometry description
  MDGeometryQuest* m_Geometry;




#ifdef ___CINT___
 public:
  ClassDef(MBackprojection, 0)  // abstract base class for backprojections
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
