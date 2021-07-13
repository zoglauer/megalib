/*
 * MUnidentifiableEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
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
