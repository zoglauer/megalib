/*
 * MRealTimeEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRealTimeEvent__
#define __MRealTimeEvent__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MRERawEvent.h"
#include "MPhysicalEvent.h"
#include "MBPData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MRealTimeEvent
{
  // public interface:
 public:
  MRealTimeEvent();
  virtual ~MRealTimeEvent();

  //! Set the ID of this event
  void SetID(unsigned int ID) { m_ID = ID; }
  //! Get the ID of this event
  unsigned int GetID() { return m_ID; }

  //! Set the time of this event
  void SetTime(MTime Time) { m_Time = Time; }
  //! Get the time of this event
  MTime GetTime() { return m_Time; }
  
  //! Set if the event is initialized
  void IsInitialized(bool IsInitalized) { m_IsInitialized = IsInitalized; }
  //! Return if the is initialized
  bool IsInitialized() { return m_IsInitialized; }
  
  //! Set if the coincidence has been tested
  void IsCoincident(bool IsCoincident) { m_IsCoincident = IsCoincident; }
  //! Return if the coincidence has been tested
  bool IsCoincident() { return m_IsCoincident; }
  
  //! Set if the event is reconstructed
  void IsReconstructed(bool IsInitalized) { m_IsReconstructed = IsInitalized; }
  //! Return if the event is reconstructed
  bool IsReconstructed() { return m_IsReconstructed; }
  
  //! Set if the event is imaged
  void IsImaged(bool IsImaged) { m_IsImaged = IsImaged; }
  //! Return if the event is imaged
  bool IsImaged() { return m_IsImaged; }
  
  //! Set if the event is dropped from the analysis
  void IsDropped(bool IsDropped) { m_IsDropped = IsDropped; }
  //! Return if the event is dropped from the analysis
  bool IsDropped() { return m_IsDropped; }
  
  //! Set if the event is merged with another one during coincidence seach -- these events should ignored in later analysis!
  void IsMerged(bool IsMerged) { m_IsMerged = IsMerged; }
  //! Return if the event is merged with another one during coincidence seach -- these events should ignored in later analysis!
  bool IsMerged() { return m_IsMerged; }
  
  
  //! Set the initial raw event
  void SetInitialRawEvent(MRERawEvent* InitialRawEvent) { m_InitialRawEvent = InitialRawEvent; }
  //! Get the initial raw event
  MRERawEvent* GetInitialRawEvent() { return m_InitialRawEvent; }
  
  //! Set the coincident raw event
  void SetCoincidentRawEvent(MRERawEvent* CoincidentRawEvent) { m_CoincidentRawEvent = CoincidentRawEvent; }
  //! Get the coincident raw event
  MRERawEvent* GetCoincidentRawEvent() { return m_CoincidentRawEvent; }
  
  //! Set the physical event
  void SetPhysicalEvent(MPhysicalEvent* PhysicalEvent) { m_PhysicalEvent = PhysicalEvent; }
  //! Get the physical event
  MPhysicalEvent* GetPhysicalEvent() { return m_PhysicalEvent; }
    
  //! Set the backprojection
  void SetBackprojection(MBPData* Backprojection) { m_Backprojection = Backprojection; }
  //! Get the backprojection
  MBPData* GetBackprojection() { return m_Backprojection; }

  
  // protected methods:
 protected:
  //MRealTimeEvent() {};
  //MRealTimeEvent(const MRealTimeEvent& RealTimeEvent) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The unique ID of this event
  unsigned int m_ID;
  //! The time of this event
  MTime m_Time; 
  
  //! True if initial raw event is available 
  bool m_IsInitialized;
  //! True if the coincidence search has been performed
  bool m_IsCoincident;
  //! True if event reconstruction has been performed
  bool m_IsReconstructed;
  //! True if backprojection has been performed
  bool m_IsImaged;

  //! True if event has been dropped from analysis
  bool m_IsDropped;
  //! True if event has been merged with another event during coincidence search
  bool m_IsMerged;
  
  //! The initial raw event (before coincidence and event reconstruction)
  MRERawEvent* m_InitialRawEvent;
  //! The initial raw event (before coincidence and event reconstruction)
  MRERawEvent* m_CoincidentRawEvent;
  //! The physical event after event reconstruction
  MPhysicalEvent* m_PhysicalEvent;
  //! The backprojection of the event
  MBPData* m_Backprojection;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MRealTimeEvent, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
