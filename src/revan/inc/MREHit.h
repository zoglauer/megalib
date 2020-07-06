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
  //! Default constructor
  MREHit();
  //! "Copy" constructor
  MREHit(MREHit* Hit);
  //! Default destructor
  virtual ~MREHit();

  //! Parse a line
  bool ParseLine(MString HitString, int Version = 0);
  
  double ComputeMinDistance(MRESE *RESE);
  MVector ComputeMinDistanceVector(MRESE *RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1);

  void AddRESE(MRESE *RESE);

  MString ToString(bool WithLink = true, int Level = 0);
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  MREHit* Duplicate();

  //! Return true if the resolutions have been set externally and cannot be changed
  bool HasFixedResolutions() const { return m_FixedResolutions; } 
  
  //! Update the volume sequence
  bool UpdateVolumeSequence(MDGeometryQuest* Geometry);
  //! Retrieve the resolutions from the geometry
  bool RetrieveResolutions(MDGeometryQuest* Geometry);
  
  // TODO: Delete!
  
  //! Noise the positions, energies and times (!!make sure to do this only for simulations and  only once!!)
  bool Noise(MDGeometryQuest* Geometry);
  //! Split the hit (due to e.g. charge sharing)
  MDGridPointCollection Grid(MDGeometryQuest* Geometry);
  

  // private members:
 private:
  //! Flag indicating the resolutions have been set externally
  bool m_FixedResolutions;

  
#ifdef ___CLING___
 public:
  ClassDef(MREHit, 0) // elementary hit
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
