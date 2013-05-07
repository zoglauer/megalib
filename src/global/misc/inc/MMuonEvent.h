/*
 * MMuonEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  virtual bool Stream(fstream& Stream, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();


  // Basic data:
  void SetEnergy(double Energy) { m_Energy = Energy; }
  virtual double GetEnergy() { return m_Energy; }
  
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


#ifdef ___CINT___
 public:
  ClassDef(MMuonEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
