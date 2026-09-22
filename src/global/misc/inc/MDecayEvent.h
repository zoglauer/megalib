/*
 * MDecayEvent.h
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


#ifndef __MDecayEvent__
#define __MDecayEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"


// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDecayEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MDecayEvent();
  virtual ~MDecayEvent();

  // Initilisations
  bool Assimilate(MDecayEvent* DecayEvent);
  bool Assimilate(MPhysicalEvent* Event); 

  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Parse a single line which is tra-file compatible
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();


  // Basic data:
  double GetBetaEnergy() const { return m_BetaEnergy; }
  MVector GetBetaPosition() const { return m_BetaPosition; }

  void SetPhysicalEvent(const MPhysicalEvent& Event);
  const MPhysicalEvent& GetPhysicalEvent(unsigned int i)const;
  unsigned int GetNPhysicals() const;
  
  MPhysicalEvent* Data(); 

  // Miscellaneous:
  virtual void Reset();
  virtual MString ToString();


  // protected methods:
 protected:


  // private methods:
 private:
 


  // protected members:
 protected:
  // basic data:
  double m_BetaEnergy;
  double m_BetaPosition;

  vector<MPhysicalEvent> m_Events;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MDecayEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
