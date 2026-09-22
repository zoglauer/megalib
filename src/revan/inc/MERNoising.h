/*
 * MERNoising.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
#include "MRawEventIncarnations.h"

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
  
  //! Return the number
  long GetNTriggeredEvents() const { return m_NTriggeredEvents; }
  //! Return the number
  long GetNVetoedEvents() const { return m_NVetoedEvents; }
  //! Return the number
  long GetNNotTriggeredOrVetoedEvents() const { return m_NNotTriggeredOrVetoedEvents; }
  
  //! Return the trigger map
  map<MString, long> GetTriggerMapTriggerNames() const { return m_TriggerMapTriggerNames; }
  //! Return the veto map
  map<MString, long> GetVetoMapTriggerNames() const { return m_VetoMapTriggerNames; }
  //! Return the veto map
  map<MString, long> GetVetoMapVetoNames() const { return m_VetoMapVetoNames; }

  //! Give a summary string containing the trigger statistics
  MString ToString(bool CoreOnly = false) const;

  // protected methods:
 protected:

  // private methods:
 private:
  //! Hide the base classes Analyze
  bool Analyze(MRawEventIncarnations*) { return false; }



  // protected members:
 protected:


  // private members:
 private:
  //! The number of events which have triggered
  long m_NTriggeredEvents;
  //! For triggered events, list the names of all triggered raised
  map<MString, long> m_TriggerMapTriggerNames;
  
  //! The number of events which have triggered
  long m_NVetoedEvents;
  //! For vetoes events, list the names of all triggered raised
  map<MString, long> m_VetoMapTriggerNames;
  //! For vetoed events, list the names of all vetoes raised
  map<MString, long> m_VetoMapVetoNames;

  //! The number of events which have not triggered
  long m_NNotTriggeredOrVetoedEvents;
  

#ifdef ___CLING___
 public:
  ClassDef(MERNoising, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
