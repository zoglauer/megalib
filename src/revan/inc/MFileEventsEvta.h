/*
 * MFileEventsEvta.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsEvta__
#define __MFileEventsEvta__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MRERawEvent.h"
#include "MGeometryRevan.h"
#include "MERNoising.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsEvta : public MFileEvents
{
  // public interface:
 public:
  MFileEventsEvta(MGeometryRevan* Geo);
  virtual ~MFileEventsEvta();

  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);
  virtual bool Close();

  MRERawEvent* GetNextEvent();

  //! Get the noising ER
  MERNoising* GetERNoising() { return m_Noising; }

  //! Save the OI information
  void SaveOI(bool SaveOI = true) { m_SaveOI = SaveOI; }

  // protected methods:
 protected:
  //! Update the observation times using the given event
  void UpdateObservationTimes(MRERawEvent* Event);
  

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MGeometryRevan* m_Geometry;
  MString m_GeometryFileName;

  int m_EventId;
  bool m_IsSimulation;
  bool m_IsFirstEvent;

  //! Save the OI information
  bool m_SaveOI;
  
  //! The ER responsible for noising the data
  MERNoising* m_Noising;
  
  static const int c_NoId;


#ifdef ___CINT___
 public:
  ClassDef(MFileEventsEvta, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
