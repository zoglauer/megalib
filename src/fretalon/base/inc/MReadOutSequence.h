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
#include "MSimIA.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A read-out sequence consisting of all the data collected during this sequence
//! such as time, channel IDs and data, etc.
//! This contains just the basic data - for adding user derived or additional data
//! use the derived class MReadOutAssembly
class MReadOutSequence
{
  // public interface:
 public:
  //! Default constructor 
  MReadOutSequence();
  //! Default destructor
  virtual ~MReadOutSequence();

  //! Remove all content
  virtual void Clear();

  //! Parse some content from a line - returns true if the line was handled - irrelevant if successfu
  virtual bool Parse(MString& Line, int Version = 1);

  
  //! Set the ID of this sequence
  void SetID(unsigned long ID) { m_ID = ID; }
  //! Get the ID of this sequence
  unsigned long GetID() const { return m_ID; }
  
  //! Set the time
  void SetTime(const MTime& Time) { m_Time = Time; }
  //! Get the time
  MTime GetTime() const { return m_Time; }
  
  //! Add a new read out
  void AddReadOut(const MReadOut& RO) { m_ReadOuts.push_back(RO); }
  //! Get the number of available read outs
  unsigned int GetNumberOfReadOuts() const { return m_ReadOuts.size(); }
  //! Find a read out with the given read-out element and return its position
  //! Return g_UnsignedIntNotDefined in case it is not found
  unsigned int FindReadOut(const MReadOutElement& ROE) const;
  //! Remove a read out - does do nothing if the index is not found
  void RemoveReadOut(unsigned int i);
  //! Get a specific read-outs
  const MReadOut& GetReadOut(unsigned int R) const;
  
  //! Return true if all read-out elements are of the same type
  bool HasIdenticalReadOutElementTypes() const;
  
  //! Return the number of simulation interactions
  unsigned int GetNSimIAs() const { return m_SimIAs.size(); }
  //! Return simulation hit i
  const MSimIA& GetSimIA(unsigned int i) const;

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
  //! The ID
  unsigned long m_ID;
  //! The time
  MTime m_Time;

  //! All the read outs
  vector<MReadOut> m_ReadOuts;
  //! All the (optional) simulation interaction informations
  vector<MSimIA> m_SimIAs;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MReadOutSequence, 0) // no description
#endif

};

//! Streamify the read-out sequence
ostream& operator<<(ostream& os, const MReadOutSequence& R);

#endif


////////////////////////////////////////////////////////////////////////////////
