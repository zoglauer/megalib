/*
 * MFileEventsSim.h
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


#ifndef __MFileEventsSim__
#define __MFileEventsSim__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MSimEvent.h"
#include "MDGeometryQuest.h"
#include "MBinaryStore.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsSim : public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MFileEventsSim();
  //! Standard constructor with geometry
  MFileEventsSim(MDGeometryQuest* Geometry);
  //! Default destructor
  virtual ~MFileEventsSim();

  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  
  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read, bool IsBinary = false);

  //! Write a sim-specific header
  virtual bool WriteHeader();

  //! Return the next event
  MSimEvent* GetNextEvent(bool Analyze = true);

  //! Add an event to the list
  bool AddEvent(MSimEvent* Event);
  //! Add text to the file
  bool AddText(const MString& Text);

  //! Close the event list after last event (per default add "END")
  virtual bool CloseEventList();

  //! Return the number of simulated events
  //! Attention: This information is only complete after all files have been scanned,
  //! i.e. after GetEvents has been called for all events!
  //! In addition, this is the number for the total file(s), NOT up to the just read event!!
  long GetSimulatedEvents();
  //! Set the simulated events
  void SetSimulatedEvents(long SimulatedEvents) { m_SimulatedEvents = SimulatedEvents; }

  //! Get simulation start are in far field (zero if not used)
  double GetSimulationStartAreaFarField() const { return m_SimulationStartAreaFarField; }

  // protected methods:
 protected:
  //! Perform initializations
  void Init();
  
  //! Return the next event in binary mode
  MSimEvent* GetNextEventBinary(bool Analyze = true);
  //! Return the next event in ASCII mode
  MSimEvent* GetNextEventASCII(bool Analyze = true);
  
  
  //! Parse the special information at the end of file -- add your special parsing in there
  virtual bool ParseFooter(const MString& Line);

  //! Open a file given by the "IN" keyword
  virtual bool OpenIncludeFile(const MString& Line);

  //! Update the observation times using the given event
  void UpdateObservationTimes(MSimEvent* Event);
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry representation
  MDGeometryQuest* m_Geometry;

  //! Number of simulated events
  long m_SimulatedEvents;
  //! True if the simulated events have been read
  bool m_HasSimulatedEvents;

  //! The simulated start area of far field simualtions
  double m_SimulationStartAreaFarField;

  //! The binary stire
  MBinaryStore m_BinaryStore;
  
#ifdef ___CLING___
 public:
  ClassDef(MFileEventsSim, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
