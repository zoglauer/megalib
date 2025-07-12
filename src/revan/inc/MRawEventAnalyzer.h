/*
 * MRawEventAnalyzer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRawEventAnalyzer__
#define __MRawEventAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MRERawEvent.h"
#include "MRawEventIncarnations.h"
#include "MRawEventIncarnationList.h"
#include "MGeometryRevan.h"
#include "MFileEventsEvta.h"
#include "MFileEventsTra.h"
#include "MSettingsEventReconstruction.h"
#include "MERCSRTMVAMethods.h"

// Forward declarations:
class MDGeometryQuest;
class MERCoincidence;
class MEREventClusterizer;
class MERHitClusterizer;
class MEREventType;
class MERTrack;
class MERCSR;
class MERDecay;

////////////////////////////////////////////////////////////////////////////////

//! How to use guide:
//!
//! First create the analyzer:
//! > MRawEventAnalyzer Analyzer;
//! Then set the event mode (default is "Add")
class MRawEventAnalyzer
{
  // Public Interface:
 public:
  //! Default constructor
  MRawEventAnalyzer();
  //! Default destructor
  virtual ~MRawEventAnalyzer();

  //! Set the geometry
  void SetGeometry(MGeometryRevan* Geometry);

  //! Set all options from a settings file
  void SetSettings(MSettingsEventReconstruction* Setting);

  //! If events are read from file set it here
  bool SetInputModeFile(MString Filename);
  //! If events are written to a file, set it with this function
  bool SetOutputModeFile(MString Filename);

  //! Do not use any GUI functions
  void SetBatch(bool IsBatch) { m_IsBatch = IsBatch; }

  //! Call this function just before the event reconstruction starts
  bool PreAnalysis();

  //! In case the events are not read from file,
  //! add an event to the back of the temporary event store.
  //! The raw event will be deleted by this class!
  bool AddRawEvent(MRERawEvent* RE);

  //! In case the events are not read from file,
  //! create the event from a string containing ALL event data
  bool AddRawEvent(const MString& RE, bool NeedsNoising = true, int Version = 25);

  //! Analyze one event
  //! The event can then be retrieved via GetOptimumEvent() or GetBestTryEvent()
  //! Return codes:
  //! c_AnalysisSucess
  //! c_AnalysisCoincidenceWindowWait
  //! c_AnalysisNoEventsInStore
  //! c_AnalysisNoEventsLeftInFile
  unsigned int AnalyzeEvent();

  //! Save the OI
  void SetSaveOI(bool SaveOI);

  // The return codes of AnalyzeEvent()
  enum c_AnalysisReturn {
    c_AnalysisSucess,
    c_AnalysisCoincidenceWindowWait,
    c_AnalysisEventClusteringFailed,
    c_AnalysisNoEventsInStore,
    c_AnalysisNoEventsLeftInFile,
    c_AnalysisSavingEventFailed,
    c_AnalysisUndefinedError
  };

  //! Return the next initial raw event from file
  //! Return zero if the input mode is not file or if the ened of the file has been reached
  MRERawEvent* GetNextInitialRawEventFromFile();


  // The results:

  //! The returns the best found event
  vector<MRERawEvent*> GetOptimumEvents();
  //! Return the single best event -- if we have multiple events it will just return the first one
  MRERawEvent* GetSingleOptimumEvent();

  //! A best solution was not found - but we still have a best try
  vector<MRERawEvent*> GetBestTryEvents();  
  //! Return the single best try event, if we have multiple return just the first one
  MRERawEvent* GetSingleBestTryEvent();  

  //! Return the initial raw events - we always have one, but in the analysis it is easier when we get it in a list too
  vector<MRERawEvent*> GetInitialRawEvents();
  //! Return the initial raw event
  MRERawEvent* GetInitialRawEvent();
  
  //! return a list of all possible events after the given event reconstrcution
  MRawEventIncarnationList* GetRawEventList() { return m_RawEvents; }


  //! Optional: Call this function at the end of the event reconstruction to gather some statistics
  bool PostAnalysis();

  //! If multiple raw event analyzer have been started, e.g. for multi threading, the analysis statistics can be joinded with this function
  void JoinStatistics(const MRawEventAnalyzer& A);


  // Interface for the basic reconstruction algorithms

  enum c_CoincidenceAlgo {
    c_CoincidenceAlgoNone,
    c_CoincidenceAlgoWindow };
  
  //! Set the coincidence algorithm. One of: c_CoincidenceAlgoNone, c_CoincidenceAlgoWindow
  void SetCoincidenceAlgorithm(int ID) { m_CoincidenceAlgorithm = ID; }

  enum c_EventClusteringAlgo {
    c_EventClusteringAlgoNone,
    c_EventClusteringAlgoTMVA,
    c_EventClusteringAlgoDistance };
  
  //! Set the coincidence algorithm. One of: c_EventClusteringAlgoNone, c_EventClusteringAlgoDistance, c_EventClusteringAlgoTMVA
  void SetEventClusteringAlgorithm(int ID) { m_EventClusteringAlgorithm = ID; }

  enum c_HitClusteringAlgo {
    c_HitClusteringAlgoNone,
    c_HitClusteringAlgoDistance,
    c_HitClusteringAlgoAdjacent,
    c_HitClusteringAlgoPDF };

  //! Set the clustering algorithm: One of c_ClusteringAlgoNone, c_ClusteringAlgoDistance, c_ClusteringAlgoAdjacent;
  void SetHitClusteringAlgorithm(int ID) { m_HitClusteringAlgorithm = ID; }

  enum c_EventTypeAlgo {
    c_EventTypeDefault,
    c_EventTypeExternal };

  //! Set the Event type identification algorithm
  void SetEventTypeAlgorithm(int ID) { m_EventTypeAlgorithm = ID; }

  enum c_TrackingAlgo {
    c_TrackingAlgoNone,
    c_TrackingAlgoModifiedPearson,
    c_TrackingAlgoChiSquare,
    c_TrackingAlgoGas,
    c_TrackingAlgoDirectional,
    c_TrackingAlgoBayesian,
    c_TrackingAlgoRank,
    c_TrackingAlgoPearson,
    c_TrackingAlgoKalman2D,
    c_TrackingAlgoKalman3D,
    c_TrackingAlgoFirstLayer
  };

  //! Set the electron tracking algorithm: One of c_TrackingAlgoNone, c_TrackingAlgoModifiedPearson, c_TrackingAlgoPearson,
  //! c_TrackingAlgoChiSquare, c_TrackingAlgoGas, c_TrackingAlgoDirectional, c_TrackingAlgoBayesian, c_TrackingAlgoRank
  void SetTrackingAlgorithm(int ID) { m_TrackingAlgorithm = ID; }

  enum c_PairAlgo {
    c_PairDefault };

  //! Set the pair reconstruction algorithm: One of c_PairDefault, c_PairKalman3D or c_PairKalman2D
  void SetPairAlgorithm(int ID) { m_PairAlgorithm = ID; }

  enum c_CSRAlgo {
    c_CSRAlgoNone,
    c_CSRAlgoFoM,
    c_CSRAlgoFoME,
    c_CSRAlgoFoMToF,
    c_CSRAlgoFoMToFAndE,
    c_CSRAlgoBayesian,
    c_CSRAlgoTMVA };

  //! Set the Compton photon tracking algorithm: One of c_CSRAlgoNone, c_CSRAlgoFoM, c_CSRAlgoFoME, c_CSRAlgoFoMToF, c_CSRAlgoBayesian
  void SetCSRAlgorithm(int ID) { m_CSRAlgorithm = ID; }

  enum c_DecayAlgo {
    c_DecayAlgoNone,
    c_DecayAlgoStandard };

  //! Set the radioactive decay detection algorithm: One of c_DecayAlgoNone, c_DecayAlgoStandard
  void SetDecayAlgorithm(int ID) { m_DecayAlgorithm = ID; }


  // Section: Interface to set all reconstruction options:
  
  // Options coincidence:
  void SetCoincidenceWindow(double CoincidenceWindow) { m_CoincidenceWindow = CoincidenceWindow; }

  // Options for event clustering
  void SetEventClusteringDistanceCutOff(double DistanceCutOff) { m_EventClusteringDistanceCutOff = DistanceCutOff; }
  
  void SetEventClusteringTMVAFileName(MString FileName) { m_EventClusteringTMVAFileName = FileName; }
  void SetEventClusteringTMVAMethods(MERCSRTMVAMethods Methods) { m_EventClusteringTMVAMethods = Methods; }
  
  
  
  // Options hit clustering:
  void SetStandardClusterizerMinDistanceD1(double MinDistance) { m_StandardClusterizerMinDistanceD1 = MinDistance; }
  void SetStandardClusterizerMinDistanceD2(double MinDistance) { m_StandardClusterizerMinDistanceD2 = MinDistance; }
  void SetStandardClusterizerMinDistanceD3(double MinDistance) { m_StandardClusterizerMinDistanceD3 = MinDistance; }
  void SetStandardClusterizerMinDistanceD4(double MinDistance) { m_StandardClusterizerMinDistanceD4 = MinDistance; }
  void SetStandardClusterizerMinDistanceD5(double MinDistance) { m_StandardClusterizerMinDistanceD5 = MinDistance; }
  void SetStandardClusterizerMinDistanceD6(double MinDistance) { m_StandardClusterizerMinDistanceD6 = MinDistance; }
  void SetStandardClusterizerMinDistanceD7(double MinDistance) { m_StandardClusterizerMinDistanceD7 = MinDistance; }
  void SetStandardClusterizerMinDistanceD8(double MinDistance) { m_StandardClusterizerMinDistanceD8 = MinDistance; }
  void SetStandardClusterizerCenterIsReference(bool CenterIsReference) { m_StandardClusterizerCenterIsReference = CenterIsReference; }
  void SetAdjacentLevel(const int Value) { m_AdjacentLevel = Value; }
  void SetAdjacentSigma(const double Value) { m_AdjacentSigma = Value; }
  void SetPDFClusterizer(MString BaseFileName) { m_PDFClusterizerBaseFileName = BaseFileName; }

  // Event type identification:
  void SetSearchPairTracks(bool Search) { m_SearchPairTracks = Search; }
  void SetSearchMIPTracks(bool Search) { m_SearchMIPTracks = Search; }
  void SetSearchPhotoEvent(bool Search) { m_SearchPhotoEvent = Search; }
  void SetSearchComptonTracks(bool Search) { m_SearchComptonTracks = Search; }
  void SetNLayersForVertexSearch(unsigned int NLayersForVertexSearch) { m_NLayersForVertexSearch = NLayersForVertexSearch; }
  void SetEventTypeFileName(MString etpFileName) { m_EventTypeFileName = etpFileName; }

  // Options electron tracking:
  void SetDoTracking(bool Do) { m_DoTracking = Do; if (Do == false) m_TrackingAlgorithm = c_TrackingAlgoNone; }
  void SetKeepAllComptonTracks(bool Keep) { m_KeepAllComptonTracks = Keep; }
  void SetAssumeTrackTopBottom(bool Flag) { m_AssumeTrackTopBottom = Flag; }
  void SetBETFileName(MString FileName) { m_BETFileName = FileName; }
  void SetMaxComptonJump(unsigned int Jump) { m_MaxComptonJump = Jump; }
  void SetNTrackSequencesToKeep(unsigned int Jump) { m_NTrackSequencesToKeep = Jump; }
  void SetRejectPurelyAmbiguousTrackSequences(bool Flag) { m_RejectPurelyAmbiguousTrackSequences = Flag; }
  void SetElectronTrackingDetectorList(vector<MString> DetectorList) { m_ElectronTrackingDetectorList = DetectorList; }

  // Options Kalman Filter
  void SetSigmaHitPos(double SigmaHitPos) { m_SigmaHitPos = SigmaHitPos; }

  // Option Compton tracking:
  void SetClassicUndecidedHandling(int Flag) { m_ClassicUndecidedHandling = Flag; }
  void SetAssumeD1First(bool Flag) { m_AssumeD1First = Flag; }
  void SetUseComptelTypeEvents(bool Flag) { m_UseComptelTypeEvents = Flag; }
  void SetGuaranteeStartD1(bool Flag) { m_GuaranteeStartD1 = Flag; }
  void SetRejectOneDetectorTypeOnlyEvents(bool Flag) { m_RejectOneDetectorTypeOnlyEvents = Flag; }

  void SetCSRThresholdMin(double Value) { m_CSRThresholdMin = Value; }
  void SetCSRThresholdMax(double Value) { m_CSRThresholdMax = Value; }

  void SetCSRMaxNHits(int Value) { m_CSRMaxNHits = Value; }
  void SetCSROnlyCreateSequences(bool Flag) { m_CSROnlyCreateSequences = Flag; }
  void SetOriginObjectsFileName(MString Name) { m_OriginObjectsFileName = Name; }

  void SetBCTFileName(MString FileName) { m_BCTFileName = FileName; }
  
  void SetCSRTMVAFileName(MString FileName) { m_CSRTMVAFileName = FileName; }
  void SetCSRTMVAMethods(MERCSRTMVAMethods Methods) { m_CSRTMVAMethods = Methods; }
  

  void SetFocalSpotCenter(MVector FocalSpotCenter) { m_FocalSpotCenter = FocalSpotCenter; }
  void SetLensCenter(MVector LensCenter) { m_LensCenter = LensCenter; }

  // general options:
  void SetTotalEnergyMax(double Value) { m_TotalEnergyMax = Value; }
  void SetTotalEnergyMin(double Value) { m_TotalEnergyMin = Value; }

  void SetLeverArmMax(double Value) { m_LeverArmMax = Value; }
  void SetLeverArmMin(double Value) { m_LeverArmMin = Value; }

  void SetEventIdMax(double Value) { m_EventIdMax = Value; }
  void SetEventIdMin(double Value) { m_EventIdMin = Value; }

  void SetRejectAllBadEvents(bool Flag) { m_RejectAllBadEvents = Flag; }

  // Decay options
  void SetDecayFileName(MString FileName) { m_DecayFileName = FileName; }

  void AddDecayEnergy(double Energy, double Error) { m_DecayEnergy.push_back(Energy); m_DecayEnergyError.push_back(Error); }







  // protected methods:
 protected:

  void AnalyzeTracks();
  void AnalyzeClusters();

  void AddRejectionReason(MRERawEvent* RE);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The sim/evta file reader
  MFileEventsEvta* m_Reader;
  //! Save the OI information
  bool m_SaveOI;

  //! The tra file writer
  MFileEventsTra* m_PhysFile;

  //! Intermediate store of the events after reading
  MRawEventIncarnations* m_EventStore;

  //! List of all events in all possible incarnations
  MRawEventIncarnationList* m_RawEvents;  

  //! Coincidence search
  MERCoincidence* m_Coincidence;
  //! Event clustering
  MEREventClusterizer* m_EventClusterizer;
  //! Hit clustering
  MERHitClusterizer* m_HitClusterizer;
  //! Event type identification
  MEREventType* m_EventType;
  //! Electron tracking
  MERTrack* m_Tracker;
  //! Pair reconstruction
//  MERPair* m_Pair;
  //! Compton sequence reconstruction
  MERCSR* m_CSR;
  //! Decay search
  MERDecay* m_Decay;
  //! Provides functionality for noising simulation data
  //! This one is independent of the one used in MFileEventsEvta
  MERNoising* m_Noising;

  MString m_Filename;
  MString m_FilenameOut;

  MGeometryRevan* m_Geometry;

  bool m_MoreEventsAvailable;

  MRERawEvent* m_InitialRawEvent;

  vector<int> m_Rejections;

  int m_NEvents;
  int m_NPassedEventSelection;
  int m_NGoodEvents;

  int m_NPhotoEvents;
  int m_NComptonEvents;
  int m_NPairEvents;
  int m_NMuonEvents;
  int m_NDecayEvents;
  int m_NPETEvents;
  int m_NMultiEvents;
  int m_NUnidentifiableEvents;

  // Reconstruction options:
  int m_CoincidenceAlgorithm;
  int m_EventClusteringAlgorithm;
  int m_HitClusteringAlgorithm;
  int m_EventTypeAlgorithm;
  int m_TrackingAlgorithm;
  int m_PairAlgorithm;
  int m_CSRAlgorithm;
  int m_DecayAlgorithm;

  // Coincidence
  double m_CoincidenceWindow;
  
  // Event clustering
  double m_EventClusteringDistanceCutOff;
  MString m_EventClusteringTMVAFileName;
  MERCSRTMVAMethods m_EventClusteringTMVAMethods;
  
  // Hit lustering
  double m_StandardClusterizerMinDistanceD1;
  double m_StandardClusterizerMinDistanceD2;
  double m_StandardClusterizerMinDistanceD3;
  double m_StandardClusterizerMinDistanceD4;
  double m_StandardClusterizerMinDistanceD5;
  double m_StandardClusterizerMinDistanceD6;
  double m_StandardClusterizerMinDistanceD7;
  double m_StandardClusterizerMinDistanceD8;
  double m_StandardClusterizerCenterIsReference;

  int m_AdjacentLevel;
  double m_AdjacentSigma;

  MString m_PDFClusterizerBaseFileName;

  // Event type identification
  bool m_SearchPhotoEvent;
  bool m_SearchPairTracks;
  bool m_SearchMIPTracks;
  bool m_SearchComptonTracks;
  unsigned int m_NLayersForVertexSearch;
  MString m_EventTypeFileName;
  
  // Electron tracking:
  bool m_DoTracking;
  bool m_KeepAllComptonTracks;
  bool m_AssumeTrackTopBottom;
  MString m_BETFileName;

  unsigned int m_MaxComptonJump;
  unsigned int m_NTrackSequencesToKeep;
  bool m_RejectPurelyAmbiguousTrackSequences;

  vector<MString> m_ElectronTrackingDetectorList;

  //Kalman Filter:
  double m_SigmaHitPos;

  // Compton tracking:
  bool m_AssumeD1First;
  bool m_UseComptelTypeEvents;
  int m_ClassicUndecidedHandling;
  bool m_GuaranteeStartD1;
  bool m_RejectOneDetectorTypeOnlyEvents;
  bool m_CSROnlyCreateSequences;

  double m_CSRThresholdMin;
  double m_CSRThresholdMax;

  int m_CSRMaxNHits;

  MString m_OriginObjectsFileName;
  MString m_BCTFileName;

  MString m_CSRTMVAFileName;
  MERCSRTMVAMethods m_CSRTMVAMethods;
  
  MVector m_LensCenter;
  MVector m_FocalSpotCenter;

  // Decay options:
  vector<double> m_DecayEnergy;
  vector<double> m_DecayEnergyError;
  MString m_DecayFileName;

  // General options:
  double m_TotalEnergyMin;
  double m_TotalEnergyMax;

  double m_LeverArmMin;
  double m_LeverArmMax;

  double m_EventIdMin;
  double m_EventIdMax;

  bool m_RejectAllBadEvents;

  // Misc:
  MGeometryRevan* m_OriginGeometry;

  double m_TimeLoad;
  double m_TimeEventClusterize;
  double m_TimeHitClusterize;
  double m_TimeEventType;
  double m_TimeTrack;
  double m_TimeCSR;
  double m_TimeFinalize;

  bool m_IsBatch;

#ifdef ___CLING___
 public:
  ClassDef(MRawEventAnalyzer, 0) // this class manages the analysis of events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
