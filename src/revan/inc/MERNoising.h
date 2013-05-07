/*
 * MERNoising.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERNoising__
#define __MERNoising__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MERConstruction.h"
#include "MRawEventList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERNoising : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERNoising();
  //! Default destructor
  virtual ~MERNoising();

  //! some initlizations before the analysis:
  virtual bool PreAnalysis();

  //! Perform the analysis, i.e. in this case do noising and triggering
  virtual bool Analyze(MRERawEvent* Event);

  //! Add the statistics of the given MERNoising to this one
  void AddStatistics(MERNoising* Noising);
  
  //! Return the trigger map
  map<MString, int> GetTriggerMap() const { return m_TriggerMap; }
  //! Return the veto map
  map<MString, int> GetVetoMap() const { return m_VetoMap; }
  //! Return the number
  int GetNNotTriggeredEvents() const { return m_NNotTriggeredEvents; }

  //! Give a summary string containing the trigger statistics
  MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Which trigger was "triggered" how often
  map<MString, int> m_TriggerMap;
  //! Which trigger was "triggered" how often
  map<MString, int> m_VetoMap;
  //! The number of events which have not triggered
  int m_NNotTriggeredEvents;
  

#ifdef ___CINT___
 public:
  ClassDef(MERNoising, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
