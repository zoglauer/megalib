/*
 * MReadOutSequence.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutSequence__
#define __MReadOutSequence__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MReadOut.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A read-out sequence consisting of all the data collected during this sequence
//! such as time, channel IDs and data, etc.
class MReadOutSequence
{
  // public interface:
 public:
  //! Default constructor 
  MReadOutSequence();
  //! Default destructor
  ~MReadOutSequence();

  //! Remove all content
  void Clear();
  
  //! Set the ID of this sequence
  void SetID(unsigned long ID) { m_ID = ID; }
  //! Get the ID of this sequence
  unsigned long GetID() const { return m_ID; }
  
  //! Set the time
  void SetTime(const MTime& Time) { m_Time = Time; }
  //! Get the time
  MTime GetTime() const { return m_Time; }
  
  //! Set the clock
  void SetClock(unsigned long Clock) { m_Clock = Clock; }
  //! Get the time
  unsigned long GetClock() const { return m_Clock; }
  
  //! Add a new read out
  void AddReadOut(const MReadOut& RO) { m_ReadOuts.push_back(RO); }
  //! Get the number of available read outs
  unsigned int GetNumberOfReadOuts() const { return m_ReadOuts.size(); }
  //! Find a read out with the given read-out element and return its position
  //! Return g_UnsignedIntNotDefined in case it is not found
  unsigned int FindReadOut(const MReadOutElement& ROE) const;
  //! Get a specific read-outs
  const MReadOut& GetReadOut(unsigned int R) const;
  
  //! Dump a string
  virtual MString ToString() const;
  
  
  // protected methods:
 protected:
  //MReadOutSequence() {};
  //MReadOutSequence(const MReadOutSequence& ReadOutSequence) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! All the read outs
  vector<MReadOut> m_ReadOuts;

  //! The ID
  unsigned long m_ID;
  //! The time
  MTime m_Time;
  //! A hardware clock
  unsigned long m_Clock;


#ifdef ___CINT___
 public:
  ClassDef(MReadOutSequence, 0) // no description
#endif

};

//! Streamify the read-out sequence
ostream& operator<<(ostream& os, const MReadOutSequence& R);

#endif


////////////////////////////////////////////////////////////////////////////////
