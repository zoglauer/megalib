/*
 * MEREventId.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventId__
#define __MEREventId__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"
#include "MGeometryRevan.h"
#include "MDDetector.h"

// Forward declarations:
class MRETrack;

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////


class MEREventId : public MERConstruction
{
  // public interface:
 public:
  MEREventId();
  virtual ~MEREventId();

  //! Global parameters used by all electron tracking algorithms
  virtual bool SetParameters(bool SearchMIPS, 
                             bool SearchPairs,  
                             bool SearchComptons,
                             bool SearchPhoto,
                             unsigned int NLayersForVertexSearch,
                             vector<MString> DetectorList);
  virtual bool Analyze(MRawEventIncarnations* List);

  virtual bool PostAnalysis();
  virtual MString ToString(bool CoreOnly = false) const;


  // protected methods:
 protected:
  virtual void CheckForPair(MRERawEvent* RE);
  virtual void CheckForMips(MRERawEvent* RE);
  virtual void CheckForPhoto(MRERawEvent* RE);
  virtual void CheckForCompton(MRERawEvent* RE);

  //virtual MRawEventIncarnations* TrackComptons(MRERawEvent* ER);

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
  //! Search for Compton events
  bool m_SearchCompton;
  //! Search for Single-site events
  bool m_SearchPhoto;
  
  //! The maximum allowed layer jump (1: do not skip one single layer)
//  unsigned int m_ComptonMaxLayerJump;
  //! Reject all ping-pong/curl events
//  bool m_RejectPureAmbiguities;
  //! The best N sequenecs are passed on to the next reconstruction stage
//  unsigned int m_NSequencesToKeep;

  //! maximum number of ambiguities of Compton events before switching to high energy tracking
//  unsigned int m_MaxNAmbiguities; 

  //! If there are multiple possible Compton tracks - consider only those with a maximum number of tracks - i.e. the minimum number of object sin D1
//  bool m_AllowOnlyMinNumberOfRESEsD1;

  //! The number of layers used for vertex search
  unsigned int m_NLayersForVertexSearch;

  //! List of detectors in which the electron tracking is performed
  vector<MDDetector*> m_DetectorList;
  
  double m_TimePhoto;
  double m_TimePairs;
  double m_TimeMips;
  double m_TimeCompton;


#ifdef ___CLING___
 public:
  ClassDef(MEREventId, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
