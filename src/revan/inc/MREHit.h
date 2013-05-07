/*
 * MREHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREHit__
#define __MREHit__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:
class MString;
class MVector;
class MDGeometryQuest;

////////////////////////////////////////////////////////////////////////////////


class MREHit : public MRESE
{
  // Public Interface:
 public:
  MREHit();
  MREHit(MVector Pos, double Energy, double Time, int Detector, 
         MVector PosRes, double EnergyRes, double TimeRes);
  MREHit(MString HitString, int Version = 0);
  MREHit(MREHit* Hit);
  virtual ~MREHit();

  double ComputeMinDistance(MRESE *RESE);
  MVector ComputeMinDistanceVector(MRESE *RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);

  void AddRESE(MRESE *RESE);

  MString ToString(bool WithLink = true, int Level = 0);
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  MREHit* Duplicate();

  //! Retrieve the resolutions from the geometry
  bool RetrieveResolutions(MDGeometryQuest* Geometry);
  //! Noise the positions, energies and times (!!make sure to do this only for simulations and  only once!!)
  bool Noise(MDGeometryQuest* Geometry);

  // private members:
 private:
  
#ifdef ___CINT___
 public:
  ClassDef(MREHit, 0) // elementary hit
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
