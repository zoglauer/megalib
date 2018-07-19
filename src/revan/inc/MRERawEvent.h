/*
 * MRERawEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRERawEvent__
#define __MRERawEvent__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT
#include <MString.h>
#include <TObjArray.h>

// MEGAlib 
#include "MGlobal.h"
#include "MREHit.h"
#include "MRESE.h"
#include "MGeometryRevan.h"
#include "MTime.h"

class MREAM;
class MRawEventIncarnations;
class MPhysicalEvent;

////////////////////////////////////////////////////////////////////////////////


//! The basic event during analysis
class MRERawEvent : public MRESE, public MRotationInterface
{
  // Public Interface:
 public:
  //! Default constructor
  MRERawEvent();
  //! Constructor including geometry
  MRERawEvent(MGeometryRevan* Geo);
  //! Special copy constructor
  MRERawEvent(MRERawEvent* RE);
  //! Default destructor
  virtual ~MRERawEvent();
  
  //! Duplicate this RESE
  MRERawEvent* Duplicate();
  
  //! Parse the text Line which contains hit information from a sim or evta file
  int ParseLine(const char* Line, int Version);
  
  //! Return the ID of the event -- attention GetID() returns the ID of the RESE
  unsigned long GetEventID();
  //! Set the ID of the event -- attention: SetID() sets the ID of the RESE
  void SetEventID(unsigned long ID);
  
  //! DEPRECATED: Return the ID of the event -- ATTENTION: GetID() returns the ID of the RESE
  unsigned long GetEventId() { mdep<<"Please use: GetEventID"<<show; return GetEventID(); }
  //! DEPRECATED: Set the ID of the event -- ATTENTION: SetID() sets the ID of the RESE
  void SetEventId(unsigned long ID) { mdep<<"Please use: SetEventID"<<show; SetEventID(ID); }
  
  //! Return the time of the event -- ATTENTION: GetTime() returns the time of the RESE
  MTime GetEventTime() const { return m_EventTime; }
  //! Set the time of the event -- ATTENTION: GetTime() returns the time of the RESE
  void SetEventTime(MTime Time) { m_EventTime = Time; }
  
  
  int GetEventType();
  void SetEventType(int Type);
  MString GetEventTypeAsString();
  
  
  //! Return the complete energy of this event
  virtual double GetEnergy();
  //! Return the length of the *longest* track
  virtual int GetTrackLength();
  //! Return the number of independent RESEs 
  virtual int GetSequenceLength();

  //! Delete all RESE content
  virtual void DeleteAll(); // TODO: Why is the base class one not OK?
  
  //! Shuffle the RESEs randomly around
  void Shuffle();

  //! Set the geometry of the underlying detector
  void SetGeometry(MGeometryRevan *Geo);
  //! Return the underlying geometry
  MGeometryRevan* GetGeometry();

  // The iterator interface
  
  //! Restart the iterator
  bool ResetIterator(MRESE *RESE = 0);
  //! Return the next RESE in the iterator, nullptr when done
  MRESE* GetNextRESE();;
  
  
  //! Set the start point of the event
  void SetStartPoint(MRESE* RESE);
  //! Return the start point of the event
  MRESE* GetStartPoint();
  
  //! Set the pair vertex
  void SetVertex(MRESE* RESE);
  //! Return the pair vertex
  MRESE* GetVertex();
  
  
  void SetVertexDirection(int Direction);
  int GetVertexDirection();

  MString ToString(bool WithLink = true, int Level = 0);
  MString ToCompactString();
  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  MTime GetCoincidenceWindow() const { return m_CoincidenceWindow; }
  void SetCoincidenceWindow(MTime CoincidenceWindow) { m_CoincidenceWindow = CoincidenceWindow; }


  bool IsDecay() { return m_Decay; }
  void SetDecay(bool Flag = true) { m_Decay = Flag; }

  bool GetExternalBadEventFlag() { return m_ExternalBadEventFlag; }
  MString GetExternalBadEventString() { return m_ExternalBadEventString; }


  void SetPhysicalEvent(MPhysicalEvent* Event);
  MPhysicalEvent* GetPhysicalEvent();


  bool CreateTracks();
  void FindEndpoints();


  //void Evaluate();
  double EvaluateComptonSequence(double ComptonAbsMin);

  bool TestElectronDirection(double E1, double E2);

  //double EvaluatePairs();

  void SetElectronTrack(MRESE* RE);
  void SetPositronTrack(MRESE* RE);

  MRESE* GetElectronTrack();
  MRESE* GetPositronTrack();


  // Interface to the event quality

  
  
  void SetGoodEvent(bool GoodEvent = true);
  bool IsGoodEvent();
  
  bool IsValid();
  
  void SetRejectionReason(int Reason);
  int GetRejectionReason();
  static MString GetRejectionReasonAsString(int r, bool Short = false);
  MString GetRejectionReasonAsString(bool Short = false);
  
  
  //! Return the clustering quality factor
  double GetClusteringQualityFactor() const { return m_ClusteringQualityFactor; }

  //! Return the track quality factor
  double GetTrackQualityFactor() const { return m_TrackQualityFactor; }

  //! Return the Compton quality factor of the best sequence:
  double GetComptonQualityFactor() const { return m_ComptonQualityFactor1; }
  //! Return the Compton quality factor of the best sequence:
  double GetComptonQualityFactor1() const { return m_ComptonQualityFactor1; }
  //! Return the Compton quality factor of the second best sequence:
  double GetComptonQualityFactor2() const { return m_ComptonQualityFactor2; }

  //! Return the pair quality factor
  double GetPairQualityFactor() const { return m_PairQualityFactor; }

  //! Set the Clustering quality factor of the best and second best sequence:
  void SetClusteringQualityFactor(double ClusteringQualityFactor);
  //! Set the Compton quality factor of the best and second best sequence:
  void SetComptonQualityFactors(double ComptonQualityFactor1, double ComptonQualityFactor2);
  //! Set the (overall) Compton (!) track quality factor
  void SetTrackQualityFactor(double TrackQualityFactor);
  //! Set the pair quality factor
  void SetPairQualityFactor(double PairQualityFactor);


  // Interface to the additional measurements - 
  // hopefully much more intelligent than the old RESE interface...

  //! Return the number of available additional measurements
  unsigned int GetNREAMs() const;
  //! Return the REAM at position a
  MREAM* GetREAMAt(unsigned int a);
  //! Start iterator for REAM container
  vector<MREAM*>::iterator GetREAMBegin() { return m_Measurements.begin(); }
  //! End iterator for REAM container
  vector<MREAM*>::iterator GetREAMEnd() { return m_Measurements.end(); }
  //! Delete a REAM, iterator points to previous REAM!
  void DeleteREAM(vector<MREAM*>::iterator& Iter);
  //! Remove and return a REAM, iterator points to previous REAM!
  MREAM* RemoveREAM(vector<MREAM*>::iterator& Iter);

  
  // protected methods:
 protected:
  void Init();

  // private methods:
 private:



  // public constants:
 public:
  // The event types:
   
  // TODO: Need to be moved in own class and be identical to what is in MPhysicalEvent
  static const int c_UnknownEvent;
  static const int c_ComptonEvent;
  static const int c_PairEvent;
  static const int c_MipEvent;
  static const int c_ShowerEvent;
  static const int c_PhotoEvent;

  static const int c_RejectionNone                               =  0;
  static const int c_RejectionTooManyHitsCSR                     =  1;
  static const int c_RejectionD1Only                             =  2;
  static const int c_RejectionD2Only                             =  3;
  static const int c_RejectionD3Only                             =  4;
  static const int c_RejectionD4Only                             =  5;
  static const int c_RejectionD5Only                             =  6;
  static const int c_RejectionD6Only                             =  7;
  static const int c_RejectionD7Only                             =  8;
  static const int c_RejectionD8Only                             =  9;
  static const int c_RejectionOneTrackOnly                       = 10;
  static const int c_RejectionTwoTracksOnly                      = 11;
  static const int c_RejectionTrackNotValid                      = 12;
  static const int c_RejectionSequenceBad                        = 13;
  static const int c_RejectionTooManyHits                        = 14;
  static const int c_RejectionEventStartNotD1                    = 15;
  static const int c_RejectionEventStartUndecided                = 16;
  static const int c_RejectionElectronDirectionBad               = 17;
  static const int c_RejectionCSRThreshold                       = 18;
  static const int c_RejectionCSRNoGoodCombination               = 19;
  static const int c_RejectionComptelTypeEvent                   = 20;
  static const int c_RejectionComptelTypeKinematicsBad           = 21;
  static const int c_RejectionSingleSiteEvent                    = 22;
  static const int c_RejectionNoHits                             = 23;
  static const int c_RejectionTotalEnergyOutOfLimits             = 24;
  static const int c_RejectionLeverArmOutOfLimits                = 25;
  static const int c_RejectionEventIdOutOfLimits                 = 26;
  static const int c_RejectionNotFromObject                      = 27;
  static const int c_RejectionTooManyUndecidedTrackElements      = 28;
  static const int c_RejectionExternalBadEventFlag               = 29;
  static const int c_RejectionEventClusteringTooManyHits         = 30;
  static const int c_RejectionTooManyEventIncarnations           = 31;
  static const int c_RejectionEventClusteringUnresolvedHits      = 32;
  static const int c_RejectionEventClusteringNoOrigins           = 33;
  static const int c_RejectionEventClusteringEnergyOutOfBounds   = 34;
  
  static const double c_NoQualityFactor;
  static const double c_NoScore;

  // protected members:
 protected:
  //! The start point of the event
  MRESE* m_Start;

  //! Pointer to geometry
  MGeometryRevan* m_Geo;
  //! The physical representation of the event - zero if there isn't (yet) any
  MPhysicalEvent* m_Event;
  //! True if event has been identified as decay
  bool m_Decay;      

  //! List of all additional measurements:
  vector<MREAM*> m_Measurements;

  // private members:
 private:
  //! True if this is a good event
  bool m_GoodEvent;

  //! The time this event happened
  MTime m_EventTime;
  //! The numerical ID
  unsigned long m_EventID;
  //! The type (Compton, etc.)
  int m_EventType;
  //! The rejection reason, if any
  int m_RejectionReason;

  //! External bad event flag has been raised
  bool m_ExternalBadEventFlag;
  //! External bad event String
  MString m_ExternalBadEventString;

  //! Comments in the event
  vector<MString> m_Comments;
  
  //! Time walk between D1 and D2, if any
  int m_TimeWalk;

  //! QF of the clustering
  double m_ClusteringQualityFactor;
  
  //! QF of the best Compton sequence
  double m_ComptonQualityFactor1;
  //! QF of the second best Compton sequence
  double m_ComptonQualityFactor2;

  //! QF of the pair event
  double m_PairQualityFactor;

  //! QF of the tracks
  double m_TrackQualityFactor;
  

  //! iterator RESE (current)
  MRESE* m_RESEIt_c;        
  //! iterator RESE (previous)
  MRESE* m_RESEIt_p;    
  
  //! the vertex of the pair
  MRESE* m_Vertex;
  //! direction of the vertex (up/down)
  int m_VertexDirection;
  
  //! the electron track of the pair
  MRESE* m_ElectronTrack; 
  //! the positron track of the pair
  MRESE* m_PositronTrack; 

  //! The coincidence window - time between the first and the last interaction
  MTime m_CoincidenceWindow;

#ifdef ___CLING___
 public:
  ClassDef(MRERawEvent, 0) // a raw event
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
