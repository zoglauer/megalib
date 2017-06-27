/*
 * MDTriggerBasic.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDTriggerBasic__
#define __MDTriggerBasic__


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


class MDTriggerBasic : public MDTrigger
{
  // public interface:
 public:
  //! Standard constructor for a trigger with a name
  MDTriggerBasic(const MString& Name);
  //! Default destructor
  virtual ~MDTriggerBasic();

  //! Return the name of this trigger
  virtual MString GetName() const;

  //! Return true if this trigger is triggering (not vetoing)
  virtual bool IsTriggering(MDDetector* D) const;
  //! Return true if this trigger is vetoing
  virtual bool IsVetoing(MDDetector* D) const;
  
  //! Set if this trigger is a veto trigger
  void SetVeto(const bool IsVetoTrigger) { m_IsVeto = IsVetoTrigger; }
  //! Set if we a triggering by detector (i.e. an whole detector raises the trigger flag, multiple hits in the detector count as one trigger)
  void SetTriggerByDetector(const bool IsTriggerByDetectorFlag) { m_IsTriggerByDetector = IsTriggerByDetectorFlag; }
  //! Set if we a triggering by channel (i.e. an individual channel raises the trigger flag)
  void SetTriggerByChannel(const bool IsTriggerByChannelFlag) { m_IsTriggerByDetector = !IsTriggerByChannelFlag; }

  //! Return, if this is a veto trigger
  bool IsVeto() const { return m_IsVeto; }
  //! Return if we are triggering by detector (i.e. an whole detector raises the trigger flag, multiple hits in the detector count as one trigger)
  bool IsTriggerByDetector() const { return m_IsTriggerByDetector; }
  //! Return if we a triggering by channel (i.e. an individual channel raises the trigger flag)
  bool IsTriggerByChannel() const { return !m_IsTriggerByDetector; }
  //! Return true, if this detector is part of this trigger, excluding vetoes and guard rings
  bool IncludesDetectorAsPositiveTrigger(MDDetector* Detector);

  //! Add this detector type as positive trigger, with the number of hits 
  void SetDetectorType(const int Detectortype, const unsigned int Hits);
  //! Add this detector type as positive trigger with the number of hits
  void SetDetector(MDDetector* Detector, const unsigned int Hits);
  
  //! Add this guard ring detector type as positive trigger, with the number of hits 
  void SetGuardRingDetectorType(const int Detectortype, const unsigned int Hits);
  //! Add this guard ring detector type as positive trigger, with the number of hits 
  void SetGuardRingDetector(MDDetector* Detector, const unsigned int Hits);
  
  //! Validate the trigger setup - required to be called after the setup is complete
  virtual bool Validate();
  
  //! Return the triggering detector types
  vector<int> GetDetectorTypes() { return m_DetectorTypes; }
  //! Return the triggering detectors
  vector<MDDetector*> GetDetectors() { return m_Detectors; }
  //! Return the trigger types (0: Normal detector, 1: GuardRing)
  vector<int> GetTriggerTypes() { return m_Types; }
  
  //! Return true if this trigger applies to the detector
  bool Applies(MDDetector* D) const;

  //! Returns true if the hit could be added 
  bool AddHit(MDVolumeSequence& VS);
  //! Returns true if the hit could be added 
  bool AddGuardRingHit(MDVolumeSequence& VS);

  //! Reset all added hits - it will not change the basic setup of the trigger (type, detector, required hits)
  void Reset();

  //! Return if this trigger has raised a triggered
  bool HasTriggered();
  //! Return if this trigger has raised a veto
  bool HasVetoed();

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
  //! Detector types this trigger consists of
  vector<int> m_DetectorTypes;
  //! Detector this trigger consists of
  vector<MDDetector*> m_Detectors;
  //! The type: 0: Normal detector, 1: GuardRing
  vector<int> m_Types;
  //! Number of hits necessary to raise the trigger
  vector<unsigned int> m_Hits;

  //! Trigger test data storage
  vector<vector<int> > m_TriggerTest;
  //! Volume sequences belonging to these hits...
  vector<vector<MDVolumeSequence> > m_VolumeSequenceTest;

  //! True if this is a veto trigger criteria
  bool m_IsVeto;

  //! True if the "triggers" are counted per detector instead of per channel
  bool m_IsTriggerByDetector;

  // private members:
 private:
  friend ostream& operator<<(ostream& os, const MDTriggerBasic& T);


#ifdef ___CINT___
 public:
  ClassDef(MDTriggerBasic, 0) // Represents a trigger condition...
#endif

};

ostream& operator<<(ostream& os, const MDTriggerBasic& T);

#endif


////////////////////////////////////////////////////////////////////////////////
