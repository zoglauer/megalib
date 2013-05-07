/*
 * MDACS.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDACS__
#define __MDACS__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDDetector.h"
#include "MDVolume.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDACS : public MDDetector
{
  // public interface:
 public:
  MDACS(MString String = "");
  MDACS(const MDACS& A);
  virtual ~MDACS();

  virtual MDDetector* Clone();

  virtual bool Validate();

  virtual void SetDetectorVolume(MDVolume* Volume);

  virtual void Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const;
  virtual vector<MDGridPoint> Discretize(const MVector& Pos, 
                                         const double& Energy, 
                                         const double& Time,
                                         MDVolume* Volume) const;
  //! Return the Grid point of this position
  virtual MDGridPoint GetGridPoint(const MVector& Pos) const;
  //! Return a position in detector volume coordinates
  virtual MVector GetPositionInDetectorVolume(const unsigned int xGrid, 
                                              const unsigned int yGrid,
                                              const unsigned int zGrid,
                                              const MVector PositionInGrid,
                                              const unsigned int Type,
                                              MDVolume* Volume);
  virtual MVector GetPositionResolution(const MVector& Pos, const double Energy) const;

  bool IsVeto(const MVector& Pos, const double Energy) const;

  virtual MString GetGeomega() const;
  virtual MString GetGeant3() const ;
  virtual MString GetMGeant() const ;
  virtual MString GetGeant3Divisions() const { return ""; }
  virtual MString GetMGeantDivisions() const { return ""; }
  virtual MString ToString() const;


  // protected methods:
 protected:

  //! The grid is only created if it is actually used
  virtual void CreateBlockedTriggerChannelsGrid();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MDACS, 0) // a basic ACS detector
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
