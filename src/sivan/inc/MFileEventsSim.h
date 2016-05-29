/*
 * MFileEventsSim.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEventsSim__
#define __MFileEventsSim__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MSimEvent.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEventsSim : public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MFileEventsSim();
  //! Standard constructor with geometry
  MFileEventsSim(MDGeometryQuest* Geometry);
  //! Default destructor
  virtual ~MFileEventsSim();

  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geo) { m_Geo = Geo; }
  
  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);

  //! Return the next event
  MSimEvent* GetNextEvent(bool Analyze = true);

  //! Add an event to the list
  bool AddEvent(MSimEvent* Event);
  //! Add text to the file
  bool AddText(const MString& Text);

  //! Close the event list after last event (per default add "END")
  virtual bool CloseEventList();

  //! Return the number of simulated events
  //! Attention: This information is only complete after all files have been scanned,
  //! i.e. after GetEvents has been called for all events!
  //! In addition, this is the number for the total file(s), NOT up to the just read event!!
  long GetSimulatedEvents();
  //! Set the simulated events
  void SetSimulatedEvents(long SimulatedEvents) { m_SimulatedEvents = SimulatedEvents; }

  //! Get simulation start are in far field (zero if not used)
  double GetSimulationStartAreaFarField() const { return m_SimulationStartAreaFarField; }

  // protected methods:
 protected:
  //! Perform initializations
  void Init();
  
  //! Parse the special information at the end of file -- add your special parsing in there
  virtual bool ParseFooter(const MString& Line);

  //! Open a file given by the "IN" keyword
  virtual bool OpenIncludeFile(const MString& Line);

  //! Update the observation times using the given event
  void UpdateObservationTimes(MSimEvent* Event);
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry representation
  MDGeometryQuest* m_Geo;

  //! ID of NEXT event
  long m_EventId;
  //! True if the current event is the first event
  bool m_IsFirstEvent;

  //! Number of simulated events
  long m_SimulatedEvents;
  //! True if the simulated events have been read
  bool m_HasSimulatedEvents;

  //! The simulated start area of far field simualtions
  double m_SimulationStartAreaFarField;

#ifdef ___CINT___
 public:
  ClassDef(MFileEventsSim, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
