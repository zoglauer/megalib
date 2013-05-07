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

  //! Reset the stored event data - call befoire each new event
  void Reset();

  //! Set a flag indicating that vetoes are ignored
  void IgnoreVetoes(bool IgnoreVetoes) { m_IgnoreVetoes = IgnoreVetoes; }
  //! Set a flag indicating that thresholds are ignored
  void IgnoreThresholds(bool IgnoreThresholds) { m_IgnoreThresholds = IgnoreThresholds; }

  //! Set a flag indicating that we always assume to have triggered
  void AlwaysAssumeTrigger(bool AlwaysAssumeTrigger) { m_AlwaysAssumeTrigger = AlwaysAssumeTrigger; }
  //! Set a flag indicating that we always assume to have vetoed
  void AlwaysAssumeVeto(bool AlwaysAssumeVeto) { m_AlwaysAssumeVeto = AlwaysAssumeVeto; }

  //! Add a new hit
  bool AddHit(const MVector& Position, const double& Energy);
  //! Add a new hit
  bool AddHit(const double& Energy, MDVolumeSequence& VS);
  //! Add a new guard ring hit
  bool AddGuardringHit(const MVector& Position, const double& Energy);
  //! Add a new guard ring hit
  bool AddGuardringHit(const double& Energy, MDVolumeSequence& VS);

  //! Return true if the last event has raised a trigger
  bool HasTriggered();
  //! If the last event has raised a trigger, return its name 
  MString GetNameTrigger();
  //! Return true if the last event has raised a veto
  bool HasVetoed();
  //! If the last event has raised a veto, return its name 
  MString GetNameVeto();


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

#ifdef ___CINT___
 public:
  ClassDef(MDTriggerUnit, 0) // no description
#endif

};

ostream& operator<<(ostream& os, const MDTriggerUnit& T);

#endif


////////////////////////////////////////////////////////////////////////////////
