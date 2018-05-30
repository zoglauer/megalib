/*
 * MRawEventIncarnationList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRawEventIncarnationList__
#define __MRawEventIncarnationList__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs

// MEGAlib libs:
#include "MGlobal.h"
#include "MRERawEvent.h"
#include "MRawEventIncarnations.h"
#include "MGeometryRevan.h"
#include "MPhysicalEvent.h"
#include "MMultiEvent.h"
#include "MPETEvent.h"


////////////////////////////////////////////////////////////////////////////////


//! A collection of events with their diffent incarnations
class MRawEventIncarnationList
{
  // Public Interface:
 public:
  //! Default constructor: you must set the geometry later
   MRawEventIncarnationList();
  //! Standard constructor with geometry
   MRawEventIncarnationList(MGeometryRevan* Geometry);
  //! Default destructor --- it does not delete anything, if you want to delete any objects, call DeleteAll before
   virtual ~MRawEventIncarnationList();
  
  //! Set the geometry (mandatory)
  void SetGeometry(MGeometryRevan* Geometry) { m_Geometry = Geometry; }


  //! Return true if we have ONLY a best events for all raw events
  bool HasOnlyOptimumEvents();
  //! Return the best event for all events - may contain nullptr if we do not have only optimum events 
  vector<MRERawEvent*> GetOptimumEvents();
  //! Return the optimum physical event -- if this event contains more than one good event we will return a Multi-event event
  MPhysicalEvent* GetOptimumPhysicalEvent();
  
  //! Return if all events have at least a best try event (or better)
  bool HasBestTryEvents();
  //! Get the best try events - if it has an optimum event return that, if it has no best try event return the first raw event, if it has nothing return nullptr. 
  vector<MRERawEvent*> GetBestTryEvents();
  //! Return the best try physical event -- if this event contains more than one event we will return a Multi-event event
  MPhysicalEvent* GetBestTryPhysicalEvent();
  

  //! Get the number of raw event incarnations stored here
  unsigned int Size() { return m_Collection.size(); }
  //! Get a specific raw event incarnation or a nullptr if the index is our of range
  MRawEventIncarnations* Get(unsigned int i);
  //! Add a raw event incrantaion
  void Add(MRawEventIncarnations* REI) { m_Collection.push_back(REI); }

  //! Just remove don't delete the event incarnations
  void Remove(MRawEventIncarnations* REI);
  //! Remove and delete the raw event incarnations
  void Delete(MRawEventIncarnations* REI);
  //! Remove all raw events incarnations
  void RemoveAll();
  //! Remove and delete all raw events incarnations
  void DeleteAll();
  
  //! Return true if any of the raw events is valid
  bool IsAnyEventValid() const;
  
  //! Return the content as a string
  MString ToString(bool WithLink = true, int Level = 0);


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
  vector<MRawEventIncarnations*> m_Collection;



#ifdef ___CLING___
 public:
   ClassDef(MRawEventIncarnationList, 0) // a list of raw event incarnations
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
