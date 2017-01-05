/*
 * MERTrack.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrack__
#define __MERTrack__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventList.h"
#include "MGeometryRevan.h"
#include "MDDetector.h"

// Forward declarations:
class MRETrack;

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////


class MERTrack : public MERConstruction
{
  // public interface:
 public:
  MERTrack();
  virtual ~MERTrack();

  //! Global parameters used by all electron tracking algorithms
  virtual bool SetParameters(bool SearchMIPS, 
                             bool SearchPairs,  
                             bool SearchComptons, 
                             unsigned int MaxLayerJump, 
                             unsigned int NSequencesToKeep, 
                             bool RejectPureAmbiguities,
                             unsigned int NLayersForVertexSearch,
                             vector<MString> DetectorList);
  virtual bool Analyze(MRawEventList* List);

  virtual bool PostAnalysis();
  virtual MString ToString(bool CoreOnly = false) const;


  // protected methods:
 protected:
  virtual bool EvaluateTracks(MRERawEvent* ER);
  virtual bool EvaluateTrack(MRETrack* Track);
  virtual bool EvaluatePairs(MRERawEvent* ER);
  
  //! Eliminate track segements with a too strong deviation from the rest
  virtual void EliminatePairDeviations(MRERawEvent* RE, MRETrack* Track);

  virtual MRawEventList* CheckForPair(MRERawEvent* RE);
  virtual void TrackPairs(MRERawEvent* ER);
  virtual void CheckForMips(MRERawEvent* RE);

  virtual MRawEventList* TrackComptons(MRERawEvent* ER);

  virtual void SortByTrackQualityFactor(MRawEventList* List);

  //! Return true, if the RESE happened in a detector in our list
  bool IsInTracker(MRESE* R);

  // private methods:
 private:



  // protected members:
 protected:
  //! True, if we should look for minimum ionizing particles (muons)
  bool m_SearchMIPs;
  //! True, if we should look for electron/positron-pair creation events
  bool m_SearchPairs;
  //! Search for Compton tracks
  bool m_SearchComptons;
  
  //! The maximum allowed layer jump (1: do not skip one single layer)
  unsigned int m_ComptonMaxLayerJump;
  //! Reject all ping-pong/curl events
  bool m_RejectPureAmbiguities;
  //! The best N sequenecs are passed on to the next reconstruction stage
  unsigned int m_NSequencesToKeep;

  //! maximum number of ambiguities of Compton events before switching to high energy tracking
  unsigned int m_MaxNAmbiguities; 

  //! If there are multiple possible Compton tracks - consider only those with a maximum number of tracks - i.e. the minimum number of object sin D1
  bool m_AllowOnlyMinNumberOfRESEsD1;

  //! The number of layers used for vertex search
  unsigned int m_NLayersForVertexSearch;

  //! List of detectors in which the electron tracking is performed
  vector<MDDetector*> m_DetectorList;
  
  double m_TimePairs;
  double m_TimeMips;
  double m_TimeComptonSequences;
  double m_TimeComptonDirections;


#ifdef ___CINT___
 public:
  ClassDef(MERTrack, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
