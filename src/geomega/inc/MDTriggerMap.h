/*
 * MDTriggerMap.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDTriggerMap__
#define __MDTriggerMap__


////////////////////////////////////////////////////////////////////////////////


// Standard libs::
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MDVolumeSequence.h"
#include "MDTrigger.h"

// Forward declarations:
class MDDetector;

////////////////////////////////////////////////////////////////////////////////


//! Class representing a universal trigger map
class MDTriggerMap : public MDTrigger
{
  // public interface:
 public:
  //! Standard constructor for a trigger with a name
  MDTriggerMap(const MString& Name);
  //! Default destructor
  virtual ~MDTriggerMap();

  //! Read the triggers for a trigger map
  bool ReadTriggerMap(MString FileName);
  //! Write the trigger map to a file
  bool WriteTriggerMap(MString FileName);
  
  //! Set the detector list
  bool SetDetectors(vector<MString> Detectors);
  //! Get the detector list
  vector<MString> GetDetectors() const { return m_Detectors; }
  
  //! Set a trigger, veto, etc.
  //! Long version: Hits & AllowMoreHits must have the same number of elements as we have detectors and be the same way sorted
  bool SetTrigger(MDTriggerMode Mode, vector<unsigned int> Hits, vector<bool> AllowMoreHits);
  
  //! Set a trigger, veto, etc.
  //! Short version: Hits and AllowMoreHits correspond to the given detectors and not the one stored in the class
  //! Hits is zero for all not set detectors
  //! AllMoreHits is true for all not set detectors
  bool SetTrigger(MDTriggerMode Mode, vector<MString> Detectors, vector<unsigned int> Hits, vector<bool> AllowMoreHits);
  
  //! Return true if this trigger is triggering (not vetoing)
  virtual bool IsTriggering(MDDetector* D) const;
  //! Return true if this trigger is vetoing
  virtual bool IsVetoing(MDDetector* D) const;
  
  //! Validate the trigger setup - required to be called after the setup is complete
  virtual bool Validate();

  //! Return true if this trigger applies to the detector
  virtual bool Applies(MDDetector* D) const;

  //! Returns true if the hit could be added 
  virtual bool AddHit(MDVolumeSequence& VS);
  //! Returns true if the hit could be added 
  virtual bool AddGuardRingHit(MDVolumeSequence& VS);

  //! Reset all added hits - it will not change the basic setup of the trigger (type, detector, required hits)
  virtual void Reset();

  //! Return if this one has raised a trigger
  virtual bool HasTriggered();
  //! Return if this one has raised a trigger which cannot be vetoes
  virtual bool HasNonVetoablyTriggered();
  //! Return if this one has raised a veto
  virtual bool HasVetoed();

  //! Return as Geomega description
  virtual MString GetGeomega() const;
  //! Return as a string
  virtual MString ToString() const;


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! List of the detectors
  vector<MString> m_Detectors;
  //! List of trigger types
  vector<MDTriggerMode> m_TriggerModes;
  //! List of vectors of required hits
  vector<vector<unsigned int>> m_Hits;
  //! List of vectors indicating if more than the given amount of hits is acceptable
  vector<vector<bool>> m_AllowMoreHits;
  
  //! The hits to be tested for trigger
  vector<unsigned int> m_TriggerTest;
  

  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MDTriggerMap& T);


#ifdef ___CINT___
 public:
  ClassDef(MDTriggerMap, 0) // Represents a trigger condition...
#endif

};

ostream& operator<<(ostream& os, const MDTriggerMap& T);

#endif


////////////////////////////////////////////////////////////////////////////////
