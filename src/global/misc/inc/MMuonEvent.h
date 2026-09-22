/*
 * MMuonEvent.h
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


#ifndef __MMuonEvent__
#define __MMuonEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MMuonEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MMuonEvent();
  virtual ~MMuonEvent();

  // Initilisations
  bool Assimilate(MMuonEvent *CED);
  bool Assimilate(MPhysicalEvent *Event); 
  bool Assimilate(MVector Direction, MVector CenterOfGravity, double Energy);
  bool Assimilate(char *LineBuffer);

  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Parse a single line which is tra-file compatible
  virtual int ParseLine(const char* Line, bool Fast = false);

  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();


  // Basic data:
  void SetEnergy(double Energy) { m_Energy = Energy; }
  virtual double GetEnergy() const { return m_Energy; }
  
  void SetDirection(MVector Direction) { m_Direction = Direction; }
  MVector GetDirection() { return m_Direction; }
  
  void SetCenterOfGravity(MVector CenterOfGravity) { m_CenterOfGravity = CenterOfGravity; }
  MVector GetCenterOfGravity() { return m_CenterOfGravity; }
  
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
  // basic data:
  double m_Energy;        // Deposited Energy
  MVector m_Direction;     // Direction of the muon
  MVector m_CenterOfGravity;  // Center Of Gravity of the muon-track
 

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MMuonEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
