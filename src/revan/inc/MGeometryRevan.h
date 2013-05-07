/*
 * MGeometryRevan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGeometryRevan__
#define __MGeometryRevan__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"
#include "MRESE.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGeometryRevan : public MDGeometryQuest
{
  // public interface:
 public:
  MGeometryRevan();
  virtual ~MGeometryRevan();

  virtual bool NoiseHit(MVector& Pos, double& E);
  virtual bool AreInSameLayer(MRESE* Orig, MRESE* Test);
  virtual bool AreInSameDetectorVolume(MRESE* RESE1, MRESE* RESE2);
  virtual bool IsBelow(MRESE* Orig, MRESE* Test, int NBelow = 1);
  virtual bool IsAbove(MRESE* Orig, MRESE* Test, int NAbove = 1);
  int GetLayerDistance(MRESE* Orig, MRESE* Test);

  static const int c_DifferentTracker;

  // protected methods:
 protected:
  //MGeometryRevan() {};
  //MGeometryRevan(const MGeometryRevan& GeometryRevan) {};

  virtual bool AreInSameTracker(MRESE *Orig, MRESE *Test);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_LayerDistance;

#ifdef ___CINT___
 public:
  ClassDef(MGeometryRevan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
