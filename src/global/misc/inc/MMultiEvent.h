/*
 * MMultiEvent.h
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


#ifndef __MMultiEvent__
#define __MMultiEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! One event constisting of multiple sub events
//! This happens for exampple if you have Na22 decay and measure 3 gamma rays
class MMultiEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  //! Default constructor
  MMultiEvent();
  //! Default destructor
  virtual ~MMultiEvent();


  //! Copy the data of the given event into this one
  bool Assimilate(MMultiEvent* MultiEvent);
  //! Copy the data of the given event into this one
  bool Assimilate(MPhysicalEvent* Event); 

  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Stream the content to the given file-stream
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  //! Parse a single line from the file
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Parse the content of the stream
  virtual bool ParseDelayed(bool Fast = false);
  
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  //! Add event
  //! Attention: This event will be owned and destroyed by this class
  //! This if you want to keep your own controlled event, add a duplicate 
  void Add(MPhysicalEvent* Event) { m_Events.push_back(Event); }
  
  //! Return the number of events
  unsigned int GetNumberOfEvents() const { return m_Events.size(); }
  //! Get a specific event
  MPhysicalEvent* GetEvent(unsigned int i);
  
  
  //! Return the total energy
  virtual double GetEnergy() const { double Ei = 0.0; for (auto E: m_Events) Ei += E->GetEnergy(); return Ei; }
  
  //! Should be "cast", but... that's bad design...
  MPhysicalEvent* Data(); 
  
  //! Reset the event data to default values
  virtual void Reset();
  //! Create a string with the data of the event
  virtual MString ToString() const;


  // protected methods:
 protected:


  // private methods:
 private:
 


  // protected members:
 protected:
  //! Store the individual events
  vector<MPhysicalEvent*> m_Events;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MMultiEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
