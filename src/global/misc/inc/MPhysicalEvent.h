/*
 * MPhysicalEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPhysicalEvent__
#define __MPhysicalEvent__


////////////////////////////////////////////////////////////////////////////////

// Standard libs::
#include <fstream>
#include <string>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TRotMatrix.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MTime.h"
#include "MVector.h"
#include "MStreams.h"
#include "MFile.h"
#include "MRotationInterface.h"
#include "MPhysicalEventHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Definition of all events type in a single place
// The namespace wrapper allow for easy import in other classes
namespace n_EventTypeWrapper {
  enum c_EventTypeEnum {
    c_UnknownEvent = -1,
    c_ComptonEvent = 0,
    c_PairEvent = 1,
    c_MuonEvent,
    c_ShowerEvent,
    c_PhotoEvent,
    c_DecayEvent,
    c_PETEvent,
    c_MultiEvent,
    c_UnidentifiableEvent = 100
  };
};
using namespace n_EventTypeWrapper;

class MPhysicalEvent : public MRotationInterface
{
  // public interface:
 public:
  MPhysicalEvent();
  virtual ~MPhysicalEvent();

  //! Assimilate this event --- only works if the types are identical
  virtual bool Assimilate(MPhysicalEvent* E);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();
  //! Stream the content to the given file-stream
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
  //! Parse the content of the stream
  virtual int ParseLine(const char* Line, bool Fast = false);
  static int ParseET(const char* type);
  //! Parse the content of the stream
  virtual bool ParseDelayed(bool Fast = false);

  //! Validate the event and calculate all high level data...
  virtual bool Validate();

  virtual void Reset();

  //! Return the type of this event
  int GetType() const { return m_EventType; }
  //! Return the type of this event
  //[[deprecated("Use GetType() instead")]]
  int GetEventType() const { return m_EventType; }
  //! Return the type of this event as string 
  MString GetTypeString() const;
  static MString GetTypeStringCode(int evtype);
  //! Set event type probability
  void SetTypeProbability(double evtTypeProbability)
        { m_EventTypeProbability = evtTypeProbability; }
  //! Get event type probability
  double GetTypeProbability() { return m_EventTypeProbability; }
  
  void SetTime(const MTime Time) { m_Time = Time; }
  MTime GetTime() const { return m_Time; }

  void SetId(const long Id) { m_Id = Id; }
  long GetId() const { return m_Id; };

  void SetTimeWalk(const int TimeWalk) { m_TimeWalk = TimeWalk; }
  int GetTimeWalk() const { return m_TimeWalk; }

  //! Get the total energy of the event
  virtual double GetEnergy() const;
  virtual double Ei() const { return GetEnergy(); }
  //! Get the position of the event - this is the location of the initial interaction!
  virtual MVector GetPosition() const;
  //! Get the origin direction of the event - if it has none return g_VectorNotDefined
  //! In detector coordinates - this is the reverse travel direction!
  virtual MVector GetOrigin() const;

  virtual bool IsGoodEvent() const { return m_IsGoodEvent; }
  virtual bool AllHitsGood() const { return m_AllHitsGood; }
  virtual void SetAllHitsGood(bool Flag) { m_AllHitsGood = Flag; }

  //! Set a flag indicating that this event originates from a decay
  void SetDecay(const bool Flag) { m_Decay = Flag; }
  //! Return true if this event originated from a decay
  bool IsDecay() const { return m_Decay; }

  //! Set a flag indicating that this event is bad
  void SetBad(const bool Flag, const MString BadString = "") { m_Bad = Flag; if (Flag == false) m_BadString = ""; else m_BadString = BadString; }
  //! Return true the if this event is bad
  bool IsBad() const { return m_Bad; }
  //! Return a string indicating why this event is bad
  MString GetBadString() const { return m_BadString; }

  //! Add a comment
  void AddComment(MString& Comment) { m_Comments.push_back(Comment); }
  //! Get the number of comments
  unsigned int GetNComments() const { return m_Comments.size(); }
  //! Get the specific comment -- throws MExceptionIndexOutOfBounds otherwise
  MString GetComment(unsigned int i) const;
  //! Clear comments
  void ClearComments() { m_Comments.clear(); }
  
  //! Add a hit
  void AddHit(const MPhysicalEventHit& Hit) { m_Hits.push_back(Hit); }
  //! Return the number of hits
  unsigned int GetNHits() const { return m_Hits.size(); }
  //! Get a specific hit -- throws MExceptionIndexOutOfBounds otherwise
  const MPhysicalEventHit& GetHit(unsigned int i) const;
  
  //! Set the OI information
  void SetOIInformation(const MVector Position, const MVector Direction, const MVector Polarization, const double Energy) { m_OIPosition = Position; m_OIDirection = Direction; m_OIPolarization = Polarization, m_OIEnergy = Energy; }
  //! Get the OI position information
  MVector GetOIPosition() const { return m_OIPosition; }
  //! Get the OI direction information
  MVector GetOIDirection() const { return m_OIDirection; }
  //! Get the OI polarization information
  MVector GetOIPolarization() const { return m_OIPolarization; }
  //! Get the OI energy information
  double GetOIEnergy() const { return m_OIEnergy; }

  //! Convert content to a descriptive string of the event
  virtual MString ToString() const;



  // protected methods:
 protected:


  // private methods:
 private:


  // public members:
 public:


  // protected members:
 protected:
  //! Type of the event 
  int m_EventType;
  double m_EventTypeProbability;

  //! The time this event occurred
  MTime m_Time;

  //! The event ID
  long m_Id;

  //! The hits
  vector<MPhysicalEventHit> m_Hits;
  
  //! True if this event has been passed all tests..
  bool m_IsGoodEvent; 
  //! To be removed...
  bool m_AllHitsGood;

  //! True if this event has a decay flag
  bool m_Decay;
  //! True if this event has be qualified as bad event
  bool m_Bad;
  //! String giving the reason this event is qualified as bad
  MString m_BadString;

  //! A set of comments store with the event
  vector<MString> m_Comments;
  
  //! The time walk between D1 and D2 in the MEGA detector
  int m_TimeWalk;

  //! OI Position (the meaning is a secret...)
  MVector m_OIPosition;
  //! OI Direction (the meaning is a secret...)
  MVector m_OIDirection;
  //! OI Polarization (the meaning is a secret...)
  MVector m_OIPolarization;
  //! OI energy (the meaning is a secret...)
  double m_OIEnergy;

  //! Store the read lines for delayed parsing
  vector<MString> m_Lines; 

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MPhysicalEvent, 1)   // base class for compton and pair events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
