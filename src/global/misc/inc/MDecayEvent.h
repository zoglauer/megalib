/*
 * MDecayEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
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
