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


//! A photo-effect or single-site event
class MPhotoEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  //! Standard consructor
  MPhotoEvent();
  //! Standrad destructor
  virtual ~MPhotoEvent();

  // Initilizations

  //! Assimilate from a similar event
  bool Assimilate(MPhotoEvent* PhotoEvent);
  //! Assimilate from a physical event
  bool Assimilate(MPhysicalEvent* Event);
  //! Assimilate from the basic data
  bool Assimilate(MVector Position, double Energy, double Weight = 1);
  //! Assimilate from a single line of data
  bool Assimilate(char* LineBuffer);

  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Parse a single line which is tra-file compatible
  virtual int ParseLine(const char* Line, bool Fast = false);

  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  // Basic data:

  //! Set the total energy of the event
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  //! Get the total energy of the event
  virtual double GetEnergy() const { return m_Energy; }
  
  //! Set the position of the event
  void SetPosition(const MVector& Position) { m_Position = Position; }
  //! Get the position of the event
  virtual MVector GetPosition() const { return m_Position; }
  
  //! For some imaging tasks we need to weigh the event, set that weight
  void SetWeight(const double Weight) { m_Weight = Weight; }
  //! For some imaging tasks we need to weigh the event, get that weight
  double GetWeight() const { return m_Weight; }

  //! ToDo: Make osolencent
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
  //! The deposited energy
  double m_Energy;
  //! The intercation position
  MVector m_Position;
  //! The weighting of the event (needed for some imaging tasks)
  double m_Weight;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MPhotoEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
