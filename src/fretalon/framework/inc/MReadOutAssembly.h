/*
 * MReadOutAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutAssembly__
#define __MReadOutAssembly__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MReadOutAssembly
{
  // public interface:
 public:
  //! Default constructor
  MReadOutAssembly();
  //! Default destructor
  virtual ~MReadOutAssembly();

  //! Reset all data
  void Clear();
  
  //! Set the ID of this event
  void SetID(unsigned int ID) { m_ID = ID; }
  //! Return the ID of this event
  unsigned int GetID() const { return m_ID; }

  //! Set and get the Time of this event
  void SetTime(MTime Time) { m_Time = Time; }
  //! Return the time this event has been obtained
  MTime GetTime() const { return m_Time; }

  //! Returns true if none of the "bad" or "incomplete" falgs has been set
  bool IsGood() const { return true; }
  //! Returns true if any of the "bad" or "incomplete" falgs has been set
  bool IsBad() const { return !IsGood(); }

  //! Set the filtered-out flag
  void SetFilteredOut(bool Flag = true) { m_FilteredOut = Flag; }
  //! Get the filgtered-out flag
  bool IsFilteredOut() const { return m_FilteredOut; }

  //! Set a specific analysis progress
  void SetAnalysisProgress(uint64_t Progress) { m_AnalysisProgress |= Progress; }
  //! Check if we have a certain progress
  bool HasAnalysisProgress(uint64_t Progress) const { return (m_AnalysisProgress & Progress) == Progress ? true : false; }
  //! Return the analysis progress flag
  uint64_t GetAnalysisProgress() const { return m_AnalysisProgress; }

  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);
  
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S);


  // protected methods:
 protected:

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! ID of this event
  unsigned int m_ID;

  //! The time of this event
  MTime m_Time;

  //! The analysis progress 
  uint64_t m_AnalysisProgress;
  
  //! True if event has been filtered out
  bool m_FilteredOut;

  
  
#ifdef ___CINT___
 public:
  ClassDef(MReadOutAssembly, 0) // no description
#endif

};

#endif


///////////////////////////////////////////////////////////////////////////////
