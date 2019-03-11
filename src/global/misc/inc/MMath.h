/*
 * MMath.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MMath__
#define __MMath__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MMath
{
  // public interface:
 public:
  MMath();
  virtual ~MMath();

  static void GalacticToSpheric(double &LatIsTheta, double &LongIsPhi);
  static void SphericToGalactic(double &ThetaIsLat, double &PhiIsLong);
  static void SphericToCartesean(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ);
  static void CarteseanToSpheric(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ);

  //! Perform a Thomson Tau test
  //! Return for each value if it is an outlier
  vector<bool> ModifiedThomsonTauTest(vector<double> Values, double Alpha = 0.05);
  
  
  double Gauss(const double& x, const double& mean = 0, const double& sigma = 1);

  bool InRange(double x);

  double AngleBetweenTwoVectors(const double& u, const double& v, const double& w, 
      const double& x, const double& y, const double& z);


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
  ClassDef(MMath, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
