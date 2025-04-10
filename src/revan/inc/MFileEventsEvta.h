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
#include "MBinaryStore.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsEvta : public MFileEvents
{
  // public interface:
 public:
  MFileEventsEvta(MGeometryRevan* Geo);
  virtual ~MFileEventsEvta();

  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read, bool IsBinary = false);
  virtual bool Close();

  MRERawEvent* GetNextEvent();
  MRERawEvent* GetNextEventASCII();
  MRERawEvent* GetNextEventBinary();
  
  //! Get the noising ER
  MERNoising* GetERNoising() { return m_Noising; }

  //! Save the OI information
  void SaveOI(bool SaveOI = true) { m_SaveOI = SaveOI; }

  //! Return the number of simulated events
  //! Attention: This information is only complete after all files have been scanned,
  //! i.e. after GetEvents has been called for all events!
  //! In addition, this is the number for the total file(s), NOT up to the just read event!!
  virtual long GetSimulatedEvents();

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

  long m_EventId;
  bool m_IsSimulation;
  bool m_IsFirstEvent;

  //! Reached the binary section in binary files
  bool m_ReachedBinarySection;
  //! Store for the temporary binary data
  MBinaryStore m_BinaryStore;
  
  //! Save the OI information
  bool m_SaveOI;
  
  //! The ER responsible for noising the data
  MERNoising* m_Noising;
  
  static const long c_NoId;


#ifdef ___CLING___
 public:
  ClassDef(MFileEventsEvta, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
