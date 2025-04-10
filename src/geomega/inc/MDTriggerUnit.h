/*
 * MDTriggerUnit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDTriggerUnit__
#define __MDTriggerUnit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDVolumeSequence.h"

// Forward declarations:
class MDGeometry;
class MDTrigger;


////////////////////////////////////////////////////////////////////////////////


class MDTriggerUnit
{
  // public interface:
 public:
  //! Standard constructor
  MDTriggerUnit(MDGeometry* Geometry);
  //! Default destructor
  virtual ~MDTriggerUnit();

  //! Reset the stored event data - call before each new event
  void Reset();
  
  //! Validate the trigger unit
  bool Validate() const;
  
  //! Return true if this detector is never triggering
  bool IsNeverTriggering(MDDetector* D) const;
  
  //! Set a flag indicating that vetoes are ignored and transfer it to all triggers
  void IgnoreVetoes(bool IgnoreVetoesFlag);
  //! Set a flag indicating that thresholds are ignored
  void IgnoreThresholds(bool IgnoreThresholdsFlag) { m_IgnoreThresholds = IgnoreThresholdsFlag; }

  //! Set a flag indicating that we always assume to have triggered
  void AlwaysAssumeTrigger(bool AlwaysAssumeTriggerFlag) { m_AlwaysAssumeTrigger = AlwaysAssumeTriggerFlag; }
  //! Set a flag indicating that we always assume to have vetoed
  void AlwaysAssumeVeto(bool AlwaysAssumeVetoFlag) { m_AlwaysAssumeVeto = AlwaysAssumeVetoFlag; }

  //! Return the detectors whose vetos fullfil simple veto rules
  //! (a) it's a veto
  //! (b) it's a basic trigger
  //! (c) it's in a specific detector not a detector type
  //! (d) it's detector type is ACS / Scintillator / Simple
  vector<MDDetector*> GetSimpleVetoDetectors() const;

  //! Add a new hit
  bool AddHit(const MVector& Position, const double& Energy);
  //! Add a new hit
  bool AddHit(const double& Energy, const MDVolumeSequence& VS);
  //! Add a new guard ring hit
  bool AddGuardRingHit(const MVector& Position, const double& Energy);
  //! Add a new guard ring hit
  bool AddGuardRingHit(const double& Energy, const MDVolumeSequence& VS);

  //! Return true if the last event has raised a trigger
  bool HasTriggered();
  //! Return a list of all the vetoes which have been raised
  vector<MString> GetTriggerNameList();
  
  //! Return true if the last event has raised a veto
  bool HasVetoed();
  //! Return a list of all the vetoes which have been raised
  vector<MString> GetVetoNameList();
  

  // protected methods:
 protected:
  //MDTriggerUnit() {};
  //MDTriggerUnit(const MDTriggerUnit& DTriggerUnit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry
  MDGeometry* m_Geometry;

  //! If this DEBUGGING flag is set always assume we had a trigger 
  bool m_AlwaysAssumeTrigger;
  //! If this DEBUGGING flag is set always assume we had a veto
  bool m_AlwaysAssumeVeto;

  //! Ignore veto hits
  bool m_IgnoreVetoes;
  //! Ignore thresholds, i.e. assume they are zero
  bool m_IgnoreThresholds;

  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MDTriggerUnit& T);

#ifdef ___CLING___
 public:
  ClassDef(MDTriggerUnit, 0) // no description
#endif

};

ostream& operator<<(ostream& os, const MDTriggerUnit& T);

#endif


////////////////////////////////////////////////////////////////////////////////
