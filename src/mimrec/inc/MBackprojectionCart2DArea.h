/*
 * MBackprojectionCart2DArea.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBackprojectionCart2DArea__
#define __MBackprojectionCart2DArea__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojectionCart2D.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionCart2DArea : public MBackprojectionCart2D
{
  // public interface:
 public:
  MBackprojectionCart2DArea();
  ~MBackprojectionCart2DArea();

  void PrepareBackprojection();
  bool Backproject(MPhysicalEvent *Event, double *Image) { return false; }
  virtual bool Backproject(MPhysicalEvent* Event, double *Image, 
                             double &Limit, int &NAboveLimit);
  bool BackprojectionCompton(double *Image, double& Limit, int& NAboveLimit);
  bool BackprojectionPair(double *Image, double& Limit, int& NAboveLimit);
  bool Assimilate(MPhysicalEvent* Event);

  double Angle(double a, double b, double c, double x, double y, double z);

  // protected methods:
 protected:


  // private methods:
 private:

  // protected members:
 protected:
  double *m_xBinCenter;  // Center of the bins (x) x are the image coordinates not the detector coordinates!
  double *m_yBinCenter;  // Center of the bins (y)
  double *m_zBinCenter;  // Center of bins z


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MBackprojectionCart2DArea, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
