/*
 * MDTrigger.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDTrigger__
#define __MDTrigger__


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

// Forward declarations:
class MDDetector;

////////////////////////////////////////////////////////////////////////////////


class MDTrigger
{
  // public interface:
 public:
  MDTrigger(const MString& Name);
  virtual ~MDTrigger();

  virtual MString GetName() const;

  void Reset();

  void SetVeto(const bool IsVeto) { m_IsVeto = IsVeto; }
  void SetTriggerByDetector(const bool IsTriggerByDetector) { m_IsTriggerByDetector = IsTriggerByDetector; }
  void SetTriggerByChannel(const bool IsTriggerByChannel) { m_IsTriggerByDetector = !IsTriggerByChannel; }

  bool IsVeto() const { return m_IsVeto; }
  bool IsTriggerByDetector() const { return m_IsTriggerByDetector; }
  bool IsTriggerByChannel() const { return !m_IsTriggerByDetector; }
  //! Return true, if this detector is part of this trigger, excluding vetoes and guard rings
  bool IncludesDetectorAsPositiveTrigger(MDDetector* Detector);


  void SetDetectorType(const int Detectortype, const unsigned int Hits);
  void SetDetector(MDDetector* Detector, const unsigned int Hits);

  void SetGuardringDetectorType(const int Detectortype, const unsigned int Hits);
  void SetGuardringDetector(MDDetector* Detector, const unsigned int Hits);

  //! Returns true if the hit could be added 
  bool AddHit(MDVolumeSequence& VS);
  //! Returns true if the hit could be added 
  bool AddGuardringHit(MDVolumeSequence& VS);

  bool HasTriggered();
  bool HasVetoed();

  virtual bool Validate();

  virtual MString GetGeomega();
  virtual MString GetGeant3(int i);
  virtual MString GetMGeant(int i);
  virtual MString ToString();

  static const int c_Detector;
  static const int c_Guardring;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of the trigger 
  MString m_Name;

  //! Detector types this trigger consists of
  vector<int> m_DetectorTypes;
  //! Detector this trigger consists of
  vector<MDDetector*> m_Detectors;
  //! The type: 0: Normal detector, 1: Guardring
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
  friend ostream& operator<<(ostream& os, const MDTrigger& T);


#ifdef ___CINT___
 public:
  ClassDef(MDTrigger, 0) // Represents a trigger condition...
#endif

};

ostream& operator<<(ostream& os, const MDTrigger& T);

#endif


////////////////////////////////////////////////////////////////////////////////
