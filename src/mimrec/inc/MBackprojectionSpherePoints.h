/*
 * MBackprojectionSpherePoints.h
 *
 * Copyright (C) 1999-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionSpherePoints__
#define __MBackprojectionSpherePoints__


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


class MBackprojectionSpherePoints : public MBackprojectionSphere
{
  // Public Interface:
 public:
  MBackprojectionSpherePoints();
  ~MBackprojectionSpherePoints();

  void PrepareBackprojection();
  bool Backproject(MPhysicalEvent *Event, double *Image);
  bool Backproject(MPhysicalEvent *Event, 
		     double *Image, double& Limit, int& NBelowLimit);

  bool Assimilate(MPhysicalEvent *Event);

  // protected methods:
 protected:
  bool BackprojectionCompton(double *Image, double& Limit, int& NBelowLimit);
  bool BackprojectionPair(double *Image, double& Limit, int& NBelowLimit);

  // private methods:
 private:


  // protected members:
 protected:
  double *m_AreaBin;    // Area of each bin (we only need them for each theta...)


  // private members:
 private:
  double *m_xBin;       // x-Position of the center of the bin 
  double *m_yBin;       // y-Position of the center of the bin
  double *m_zBin;       // z-Position of the center of the bin
  


#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionSpherePoints, 0) // Area-backprojections on a sphere 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
