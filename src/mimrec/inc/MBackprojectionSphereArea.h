/*
 * MBackprojectionSphereArea.h
 *
 * Copyright (C) 1999-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionSphereArea__
#define __MBackprojectionSphereArea__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojectionSphere.h"
#include "MPhysicalEvent.h"


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionSphereArea : public MBackprojectionSphere
{
  // Public Interface:
 public:
  MBackprojectionSphereArea();
  ~MBackprojectionSphereArea();

  void PrepareBackprojection();
  bool Backproject(MPhysicalEvent* Event, double* Image);
  bool Backproject(MPhysicalEvent* Event, double* Image, 
                   double& Limit, int& NBelowLimit);

  bool Assimilate(MPhysicalEvent *Event);

  // protected methods:
 protected:
  bool BackprojectionCompton(double* Image, double& Limit, int& NBelowLimit);
  bool BackprojectionPair(double* Image, double& Limit, int& NBelowLimit);
  bool BackprojectionPhoto(double* Image, double& Limit, int& NBelowLimit);

  // private methods:
 private:
  double Angle(double a, double b, double c, double x, double y, double z);
  double GaussLongitudinal(double LogitudinalAngle);
  double GaussTransversal(double TransversalAngle);

  // protected members:
 protected:
  double* m_AreaBin;    // Area of each bin (we only need them for each theta...)


  // private members:
 private:
  double* m_xBin;       // x-Position of the center of the bin 
  double* m_yBin;       // y-Position of the center of the bin
  double* m_zBin;       // z-Position of the center of the bin


  double m_InvSquareDist;


#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionSphereArea, 0) // Area-backprojections on a sphere 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
