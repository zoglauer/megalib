/*
 * MFastMath.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFastMath__
#define __MFastMath__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>
#include <cstring>
using namespace std;

// ROOT libs:

// MEGAlib libs:

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The fast math class
class MFastMath
{
  // public interface:
 public:
  //! Default constructor
  MFastMath() {}
  //! Default destuctor 
  virtual ~MFastMath() {}

  //! A fast atan (accuracy < 0.00150887 radians)
  static double atan(double x) {
    if (x > 1.0) {
      return M_PI_2 - MFastMath::atan(1.0/x);
    } else if (x < -1.0) {
      return -M_PI_2 - MFastMath::atan(1.0/x);
    } else {
      return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
    }
  }

  //! A fast a tan2 (accuracy < 0.00150887 radians)
  static double atan2(double y, double x) {
    if (x > 0.0) {
      return MFastMath::atan(y/x);
    } else if (x < 0.0) {
      if (y >= 0.0) {
        return MFastMath::atan(y/x) + M_PI;
      } else {
        return MFastMath::atan(y/x) - M_PI;
      }
    } else {
      if (y > 0.0) {
        return M_PI_2;
      } else if (y < 0.0) {
        return -M_PI_2;
      } else {
        return 0.0;
      }
    }
  }
  
  //! A fast approximation of acos based on Abramowitz and Stegun error < 0.01% if |x| < 0.9998
  static float acos(const float x) {
    if (x < 0.0F) {
      if (x <= -1.0F) return 3.1415926535889793F; 
      return 3.1415926535889793F - sqrt(1.0F + x) * (1.5707288F - x*(-0.2121144F - x*(0.0742610F + x*0.0187293F)));
    }
    if (x >= 1.0F) return 0.0F; 
    return sqrt(1.0F - x) * (1.5707288F + x*(-0.2121144F + x*(0.0742610F - x*0.0187293F)));
  }

  //! A fast optimization of 1/sqrt() after John Carmack / Newton. Error < 0.0005%
  //! Attention: Breaks down at large numbers!
  //! 3x as faster than 1.0/sqrt() on Haswell CPU with gcc -O3 -mtune=native
  //! Needs to be float since it depends on the IEEE floating point definition
  static float invsqrt(float x) {
    float xhalf = 0.5f*x;

    // This magic is possible due to the IEEE floating point definition
    uint32_t i;
    memcpy(&i, &x, sizeof(float));
    i = 0x5f375a86 - (i>>1);
    memcpy(&x, &i, sizeof(float));

    // Two Newton stages:
    x = x*(1.5f-xhalf*x*x);
    x = x*(1.5f-xhalf*x*x);

    return x;
  }
  

  //! A cosinus approximation after Abramowitz and Stegun error < 0.0025%
  //! Double's version roughly 3x faster in latest Nehalems with gcc -O3 -mtunes=native
  static double cos(double x) {
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
  static double sin(double x) {
    static const double pi        = M_PI;
    
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
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MFastMath, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
