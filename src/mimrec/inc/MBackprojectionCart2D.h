/*
 * MBackprojectionCart2D.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionCart2D__
#define __MBackprojectionCart2D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionCart2D : public MBackprojection
{
  // public interface:
 public:
  MBackprojectionCart2D();
  ~MBackprojectionCart2D();

  void PrepareBackprojection();
  virtual bool Backproject(MPhysicalEvent* Event, double *Image, 
                             double &Limit, double &NAboveLimit);
  bool Assimilate(MPhysicalEvent* Event);

  double DistanceConecirclePoint(double x, double y, double z);

  virtual void SetViewportDimensions(double xMin, double xMax, int xNBins, 
                                     double yMin, double yMax, int yNBins, 
                                     double zMin = 0, double zMax = 0, int zNBins = 1,
                                     MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

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
  ClassDef(MBackprojectionCart2D, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
