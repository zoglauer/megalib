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

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The fast math class
class MFastMath
{
  // public interface:
 public:
  //! Default constructor
  MFastMath();
  //! Default destuctor 
  virtual ~MFastMath();

  //! A fast atan (accuracy < 0.00150887 radians)
  static double FastATan(double x) {
    if (x > 1.0) {
      return M_PI_2 - FastArcTan(1.0/x);
    } else if (x < -1.0) {
      return -M_PI_2 - FastArcTan(1.0/x);
    } else {
      return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
    }
  }

  //! A fast a tan2 (accuracy < 0.00150887 radians)
  static double FastATan2(double y, double x) {
    if (x > 0.0) {
      return FastArcTan(y/x);
    } else if (x < 0.0) {
      if (y >= 0.0) {
        return FastArcTan(y/x) + M_PI;
      } else {
        return FastArcTan(y/x) - M_PI;
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
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MFastMath, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
