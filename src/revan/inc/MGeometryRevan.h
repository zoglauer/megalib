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


//! Enhanced geometry with revan specific additions
class MGeometryRevan : public MDGeometryQuest
{
  // public interface:
 public:
  //! Standard constructor
  MGeometryRevan();
  //! Default destructor
  virtual ~MGeometryRevan();

  //! Noise a hit consi
  virtual bool NoiseHit(MVector& Pos, double& E);
  //! Return true if the two RESEs are in the same detector and the same layer
  virtual bool AreInSameLayer(MRESE* Orig, MRESE* Test);
  //! Return true if the two RESEs are in the same detector / tracker
  virtual bool AreInSameDetectorVolume(MRESE* RESE1, MRESE* RESE2);
  //! Check if the Test hit is NBelow layers below the Reference hit
  virtual bool IsBelow(MRESE* Reference, MRESE* Test, int NBelow = 1);
  //! Check if the Test hit is NBelow layers below the Reference hit
  virtual bool IsAbove(MRESE* Reference, MRESE* Test, int NAbove = 1);
  //! Return the distance in layers between the reference and test hit
  int GetLayerDistance(MRESE* Reference, MRESE* Test);
  //! Return the distance in cm between the refe
  

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
