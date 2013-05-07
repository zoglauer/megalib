/*
 * MPhotoEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPhotoEvent__
#define __MPhotoEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPhotoEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MPhotoEvent();
  virtual ~MPhotoEvent();

  // Initilisations
  bool Assimilate(MPhotoEvent* PhotoEvent);
  bool Assimilate(MPhysicalEvent* Event); 
  bool Assimilate(MVector Position, double Energy, double Weight = 1);
  bool Assimilate(char* LineBuffer);
  virtual bool Stream(fstream& Stream, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  // Basic data:
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  virtual double GetEnergy() const { return m_Energy; }
  virtual double Ei() const { return m_Energy; }
  
  void SetPosition(const MVector& Position) { m_Position = Position; }
  //! Get the position of the event - this is the location of the initial interaction!
  virtual MVector GetPosition() const { return m_Position; }
  
  void SetWeight(const double Weight) { m_Weight = Weight; }
  double GetWeight() const { return m_Weight; }

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
  double m_Energy;           // Deposited Energy
  MVector m_Position;         // Position
  double m_Weight;           // Weighting of this event

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MPhotoEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
