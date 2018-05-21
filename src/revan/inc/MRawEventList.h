/*
 * MRawEventList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRawEventList__
#define __MRawEventList__


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
class MRawEventList
{
  // Public Interface:
 public:
  //! Default constructor
  MRawEventList();
  //! Standard constructor with geometry
  MRawEventList(MGeometryRevan* Geometry);
  //! Default destructor
  virtual ~MRawEventList();
  
  //! Set the geometry
  void SetGeometry(MGeometryRevan* Geometry);

  //! Set the initial raw event
  void SetInitialRawEvent(MRERawEvent* RE);
  MRERawEvent* GetInitialRawEvent();

  bool HasOptimumEvent();
  MRERawEvent* GetOptimumEvent();
  void SetOptimumEvent(MRERawEvent* OptimumEvent);

  bool HasBestTry();
  MRERawEvent* GetBestTryEvent();
  void SetBestTryEvent(MRERawEvent* BestTryEvent);

  MString ToString(bool WithLink = true, int Level = 0);

  int GetNRawEvents();
  MRERawEvent* GetRawEventAt(int i);
  void SetRawEventAt(MRERawEvent* RE, int i);
  void AddRawEvent(MRERawEvent *RE);

  //! Just remove don't delete the event
  void RemoveRawEvent(MRERawEvent* RE);
  //! Remove and delete the raw event
  void DeleteRawEvent(MRERawEvent* RE);
  //! Remove and delete all raw events
  void DeleteAll();

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
  vector<MRERawEvent*> m_RawEventList;

  //! Link to the optimum event
  MRERawEvent* m_OptimumEvent;
  //! Link to th best try, if there is an optimum, use this
  MRERawEvent* m_BestTryEvent;
  //! The inital event
  MRERawEvent* m_InitialEvent;

  int m_EventCounter;


#ifdef ___CLING___
 public:
  ClassDef(MRawEventList, 0) // a list of raw events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
