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


//! The class of trigger: Basic or Universal
enum class MDTriggerType : int { 
  c_Unknown = 0, c_Basic = 1, c_Universal = 2
};


////////////////////////////////////////////////////////////////////////////////


//! The Type of trigger: VetoableTrigger, UnvetoableTrigger, Veto
enum class MDTriggerMode : int { 
  c_Unknown = 0, c_VetoableTrigger = 1, c_NonVetoableTrigger = 2, c_Veto = 3
};


////////////////////////////////////////////////////////////////////////////////


//! The base class for a trigger
class MDTrigger
{
  // public interface:
 public:
  //! Standard constructor for a trigger with a name
  MDTrigger(const MString& Name);
  //! Default destructor
  virtual ~MDTrigger();

  //! Return the name of this trigger
  MString GetName() const { return m_Name; }
  
  //! Return the trigger class
  MDTriggerType GetType() const { return m_TriggerType; }
  
  //! Validate the trigger setup - required to be called after the setup is complete
  virtual bool Validate() { return false; }
  
  //! Ignore all vetoes -- for pretriggering in the detector effects engine, usually set via the trigger unit
  virtual void IgnoreVetoes(bool Ignore) { m_IgnoreVetoes = Ignore; }
  
  //! Return true if this trigger applies to the detector
  virtual bool Applies(MDDetector*) const { return false; }
  //! Return true if this trigger is triggering (not vetoing)
  virtual bool IsTriggering(MDDetector*) const { return false; }
  //! Return true if this trigger is vetoing
  virtual bool IsVetoing(MDDetector*) const { return false; }
  
  //! Returns true if the hit could be added 
  virtual bool AddHit(MDVolumeSequence&) { return false; }
  //! Returns true if the hit could be added 
  virtual bool AddGuardRingHit(MDVolumeSequence&) { return false; }

  //! Reset all added hits - it will not change the basic setup of the trigger (type, detector, required hits)
  virtual void Reset() { }

  //! Return if this one has raised a trigger
  virtual bool HasTriggered() { return false; }
  //! Return if this one has raised a trigger which cannot be vetoed
  virtual bool HasNonVetoablyTriggered() { return false; }
  //! Return if thsi one has raised a veto
  virtual bool HasVetoed() { return false; }

  //! Return as Geomega description
  virtual MString GetGeomega() const { return "GetGeomega must be defined in derived class"; }
  //! Return as a string
  virtual MString ToString() const { return "ToString must be defined in derived class"; }
  
  static const int c_Detector;
  static const int c_GuardRing;
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:

  //! Name of the trigger 
  MString m_Name;
  
  //! Type of the trigger
  MDTriggerType m_TriggerType;
    
  //! Ignore all vetoes
  bool m_IgnoreVetoes;
  
  
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
