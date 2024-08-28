/*
 * MREStripHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREStripHit__
#define __MREStripHit__


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


class MREStripHit : public MRESE
{
  // Public Interface:
 public:
  //! Default constructor
  MREStripHit();
  //! "Copy" constructor
  MREStripHit(MREStripHit* Hit);
  //! Default destructor
  virtual ~MREStripHit();

  //! Parse a line
  bool ParseLine(MString HitString, int Version = 0);
  
  double ComputeMinDistance(MRESE* RESE);
  MVector ComputeMinDistanceVector(MRESE* RESE);
  bool AreAdjacent(MRESE* R, double Sigma = 3, int Level = 1) { return false; }
  
  bool AreStripsAdjacent(MREStripHit* SH);
  
  
  void AddRESE(MRESE* RESE);

  MString ToString(bool WithLink = true, int Level = 0);
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  MREStripHit* Duplicate();

  //! Return true if the resolutions have been set externally and cannot be changed
  bool HasFixedResolutions() const { return m_FixedResolutions; } 
  
  //! Update the volume sequence
  bool UpdateVolumeSequence(MDGeometryQuest* Geometry);
  //! Retrieve the resolutions from the geometry
  bool RetrieveResolutions(MDGeometryQuest* Geometry);
  
  //! True if this is an X strip (otherwise it is an y strip)
  bool IsXStrip() const { return m_IsXStrip; }
  //! True if this is an X strip (otherwise it is an y strip)
  void IsXStrip(bool Flag) { m_IsXStrip = Flag; }
  
  //! Return the strip ID
  int GetStripID() const { return m_StripID; }
  //! Set the strip ID
  void SetStripID(int StripID) { m_StripID = StripID; }
  
  //! Return the detector ID
  long GetDetectorID() const { return m_DetectorID; }
  //! Set the dectorte ID
  void SetDetectorID(long DetectorID) { m_DetectorID = DetectorID; }
  
  //! Return the global non-strip position
  double GetNonStripPosition() const { return m_NonStripPosition; }
  //! Set the global non-strip position
  void SetNonStripPosition(double NonStripPosition) { m_NonStripPosition = NonStripPosition; }
  
  //! Return the global depth position
  double GetDepthPosition() const { return m_DepthPosition; }
  //! Set the global depth position
  void SetDepthPosition(double DepthPosition) { m_DepthPosition = DepthPosition; }
  
  
  // TODO: Delete!
  
  //! Noise the positions, energies and times (!!make sure to do this only for simulations and  only once!!)
  bool Noise(MDGeometryQuest* Geometry);
  //! Split the hit (due to e.g. charge sharinh)
  MDGridPointCollection Grid(MDGeometryQuest* Geometry);
  

  // private members:
 private:
  //! Flag indicating the resolutions have been set externally
  bool m_FixedResolutions;
  //! 
  bool m_IsXStrip;
  //!
  int m_StripID;
  //!
  long m_DetectorID;
  //! 
  double m_NonStripPosition;
  //!
  double m_DepthPosition;
  //! 
  
#ifdef ___CLING___
 public:
  ClassDef(MREStripHit, 0) // elementary hit
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
