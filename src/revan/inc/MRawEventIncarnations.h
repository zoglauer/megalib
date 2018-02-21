/*
 * MRawEventIncarnations.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRawEventIncarnations__
#define __MRawEventIncarnations__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs

// MEGAlib libs:
#include "MGlobal.h"
#include "MRERawEvent.h"
#include "MGeometryRevan.h"


////////////////////////////////////////////////////////////////////////////////


//! This list contains all possible incarnations (reconstruction possibilities) of one single event
class MRawEventIncarnations
{
  // Public Interface:
 public:
  //! Default constructor: you must set the geometry later
  MRawEventIncarnations();
  //! Standard constructor with geometry
  MRawEventIncarnations(MGeometryRevan* Geometry);
  //! Default destructor --- it does not delete anything, if you want to delete any objects, call DeleteAll before
  virtual ~MRawEventIncarnations();
  
  //! Set the geometry (mandatory)
  void SetGeometry(MGeometryRevan* Geometry);

  //! Set the initial raw event
  void SetInitialRawEvent(MRERawEvent* RE);
  //! Return the initial raw event
  MRERawEvent* GetInitialRawEvent();

  //! Return true if we have a best event
  bool HasOptimumEvent();
  //! Return the best event or a nullptr
  MRERawEvent* GetOptimumEvent();
  //! Set the best event
  void SetOptimumEvent(MRERawEvent* OptimumEvent);

  //! Return if we have a best try event - if we have also an optimum event, return that one instead!
  bool HasBestTryEvent();
  //! Get the best try event - if we have also an optimum event, return that one instead, if we have none, use the first raw event instead
  MRERawEvent* GetBestTryEvent();
  //! Set the best try event
  void SetBestTryEvent(MRERawEvent* BestTryEvent);

  //! Get the number of raw events stored here
  int GetNRawEvents();
  //! Get a specific raw event or a nullptr if the index is our of range
  MRERawEvent* GetRawEventAt(int i);
  //! Set (and possibly replace a raw event)
  void SetRawEventAt(MRERawEvent* RE, int i);
  //! Add a raw event to the event of the list
  void AddRawEvent(MRERawEvent *RE);

  //! Just remove don't delete the event
  void RemoveRawEvent(MRERawEvent* RE);
  //! Remove and delete the raw event
  void DeleteRawEvent(MRERawEvent* RE);
  //! Remove and delete all raw events
  void DeleteAll();
  
  //! Return true if any of the raw events is valid
  bool IsAnyEventValid() const;
  
  //! Return the content as a string
  MString ToString(bool WithLink = true, int Level = 0);
  
  void SortByTrackQualityFactor(bool GoodAreHigh = true);

  // protected methods:
 protected:
  void Init();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry
  MGeometryRevan* m_Geometry;
  //! The list of raw events
  vector<MRERawEvent*> m_Incarnations;

  //! Link to the optimum event
  MRERawEvent* m_OptimumEvent;
  //! Link to th best try, if there is an optimum, use this
  MRERawEvent* m_BestTryEvent;
  //! The inital event
  MRERawEvent* m_InitialEvent;


#ifdef ___CLING___
 public:
  ClassDef(MRawEventIncarnations, 0) // a list of raw events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
