/*
 * MUnidentifiableEvent.h
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


#ifndef __MUnidentifiableEvent__
#define __MUnidentifiableEvent__


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


class MUnidentifiableEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MUnidentifiableEvent();
  virtual ~MUnidentifiableEvent();

  // Initilisations
  bool Assimilate(MUnidentifiableEvent* UnidentifiableEvent);
  bool Assimilate(MPhysicalEvent *Event);

  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Parse a single line from the file
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  // Basic data:
  virtual double GetEnergy() const { return m_Energy; }
  virtual void SetEnergy(const double Energy) { m_Energy = Energy; }
  
  MPhysicalEvent* Data(); 

  // Miscellaneous:
  virtual void Reset();
  virtual MString ToString() const;


  // protected methods:
 protected:


  // private methods:
 private:
 


  // protected members:
 protected:
  //! Total deposited energy during this event
  double m_Energy;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MUnidentifiableEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
