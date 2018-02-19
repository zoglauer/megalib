/*
 * MRawEventAnalyzer.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MRawEventAnalyzer.cxx
//
//
// This class analyzes raw events.
//
// During the analysis of one event, different ordering possibilties are stored 
// in the m_RawEvents member. After loading the event it contains only one
// expression of the event: only single hits. After the track analysis also the 
// possibilties for different tracks in the one raw event are stored as 
// different expressions in the m_RawEvents member.
//
// After all analysis is done the most likely expression is the chosen one. 
//
//
////////////////////////////////////////////////////////////////////////////////

// Standard lib
#include <iomanip>
#include <limits>
using namespace std;

// ROOT lib
#include <TSystem.h>


// MEGAlib
#include "MRawEventAnalyzer.h"
#include "MRERawEvent.h"
#include "MGUIProgressBar.h"
#include "MStreams.h"
#include "MAssert.h"
#include "MTimer.h"
#include "MERCoincidence.h"
#include "MERClusterize.h"
#include "MERTrack.h"
#include "MERTrackPearson.h"
#include "MERTrackRank.h"
#include "MERTrackBayesian.h"
#include "MERTrackGas.h"
#include "MERTrackDirectional.h"
#include "MERTrackChiSquare.h"
#include "MERCSRChiSquare.h"
#include "MERCSRToF.h"
#include "MERCSREnergyRecovery.h"
#include "MERCSRToFWithEnergyRecovery.h"
#include "MERCSRBayesian.h"
#include "MERCSRTMVA.h"
#include "MERDecay.h"


#ifdef ___CLING___
ClassImp(MRawEventAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////

// DONT'T CHANGE THE NUMBERS!!

const int MRawEventAnalyzer::c_CoincidenceAlgoNone   = 0;
const int MRawEventAnalyzer::c_CoincidenceAlgoWindow = 1;

const int MRawEventAnalyzer::c_ClusteringAlgoNone     = 0;
const int MRawEventAnalyzer::c_ClusteringAlgoDistance = 1;
const int MRawEventAnalyzer::c_ClusteringAlgoAdjacent = 2;
const int MRawEventAnalyzer::c_ClusteringAlgoPDF      = 3;

const int MRawEventAnalyzer::c_TrackingAlgoNone            = 0;
const int MRawEventAnalyzer::c_TrackingAlgoModifiedPearson = 1;
const int MRawEventAnalyzer::c_TrackingAlgoChiSquare       = 2;
const int MRawEventAnalyzer::c_TrackingAlgoGas             = 3;
const int MRawEventAnalyzer::c_TrackingAlgoDirectional     = 4;
const int MRawEventAnalyzer::c_TrackingAlgoBayesian        = 5;
const int MRawEventAnalyzer::c_TrackingAlgoRank            = 6;
const int MRawEventAnalyzer::c_TrackingAlgoPearson         = 7;

const int MRawEventAnalyzer::c_CSRAlgoNone          = 0;
const int MRawEventAnalyzer::c_CSRAlgoFoM           = 1;
const int MRawEventAnalyzer::c_CSRAlgoFoME          = 2;
const int MRawEventAnalyzer::c_CSRAlgoFoMToF        = 3;
const int MRawEventAnalyzer::c_CSRAlgoBayesian      = 4;
const int MRawEventAnalyzer::c_CSRAlgoTMVA = 5;
const int MRawEventAnalyzer::c_CSRAlgoFoMToFAndE    = 6;

const int MRawEventAnalyzer::c_DecayAlgoNone     = 0;
const int MRawEventAnalyzer::c_DecayAlgoStandard = 1;

// DONT'T CHANGE THE NUMBERS!!

const unsigned int MRawEventAnalyzer::c_AnalysisSucess                 = 0;
const unsigned int MRawEventAnalyzer::c_AnalysisCoincidenceWindowWait  = 1;
const unsigned int MRawEventAnalyzer::c_AnalysisNoEventsInStore        = 2;
const unsigned int MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile     = 3;
const unsigned int MRawEventAnalyzer::c_AnalysisSavingEventFailed      = 4;
const unsigned int MRawEventAnalyzer::c_AnalysisUndefinedError         = 5;



////////////////////////////////////////////////////////////////////////////////


MRawEventAnalyzer::MRawEventAnalyzer()
{
  // default constructor

  m_Filename = "";
  m_Reader = nullptr;
  m_SaveOI = false;
  
  m_FilenameOut = "";
  m_PhysFile = nullptr;

  m_Geometry = nullptr; 
  m_RawEvents = new MRawEventIncarnations(0);

  m_MoreEventsAvailable = true;

  m_InitialRawEvent = nullptr;

  m_NEvents = 0;
  m_NPassedEventSelection = 0;
  m_NGoodEvents = 0;
  m_NPhotoEvents = 0;
  m_NComptonEvents = 0;
  m_NPairEvents = 0;
  m_NMuonEvents = 0;
  m_NDecayEvents = 0;
  m_NUnidentifiableEvents = 0;

  m_Rejections.resize(30, 0);

  m_CoincidenceAlgorithm = c_CoincidenceAlgoNone;
  m_ClusteringAlgorithm = c_ClusteringAlgoAdjacent;
  m_TrackingAlgorithm = c_TrackingAlgoNone;
  m_CSRAlgorithm = c_CSRAlgoFoM;
  m_DecayAlgorithm = c_DecayAlgoNone;

  m_StandardClusterizerMinDistanceD1 = 0.05;
  m_StandardClusterizerMinDistanceD2 = 1.1;
  m_StandardClusterizerMinDistanceD3 = 0.19;
  m_StandardClusterizerMinDistanceD4 = 0.0;
  m_StandardClusterizerMinDistanceD5 = 0.19;
  m_StandardClusterizerMinDistanceD6 = 0.19;
  m_StandardClusterizerMinDistanceD7 = 0.19;
  m_StandardClusterizerMinDistanceD8 = 0.19;
  m_StandardClusterizerCenterIsReference = false;

  m_AdjacentLevel = 2;
  m_AdjacentSigma = 0.0;

  m_DoTracking = true;
  m_SearchPairTracks = true;
  m_SearchMIPTracks = false;
  m_SearchComptonTracks = true;
  m_KeepAllComptonTracks = false;
  m_AssumeTrackTopBottom = false;

  m_MaxComptonJump = 2;
  m_NTrackSequencesToKeep = 1;
  m_RejectPurelyAmbiguousTrackSequences = false;
  m_NLayersForVertexSearch = 4;

  m_AssumeD1First = false;
  m_ClassicUndecidedHandling = false;
  m_UseComptelTypeEvents = true;
  m_GuaranteeStartD1 = true;
  m_RejectOneDetectorTypeOnlyEvents = true;
  
  m_CSRThresholdMin = 0;
  m_CSRThresholdMax = 1;
  m_CSRMaxNHits = 4;

  m_CSROnlyCreateSequences = false;

  m_OriginGeometry = nullptr;

  m_OriginObjectsFileName = "";

  m_TotalEnergyMin = 0;
  m_TotalEnergyMax = numeric_limits<double>::max();

  m_LeverArmMin = 0;
  m_LeverArmMax = numeric_limits<double>::max();

  m_EventIdMin = -1;
  m_EventIdMax = -1;

  m_RejectAllBadEvents = true;

  m_TimeLoad = 0;
  m_TimeClusterize = 0;
  m_TimeTrack = 0;
  m_TimeCSR = 0;
  m_TimeFinalize = 0;

  m_IsBatch = false;

  m_Coincidence = nullptr;
  m_Clusterizer = nullptr;
  m_Tracker = nullptr;
  m_CSR = nullptr;
  m_Decay = nullptr;
  m_Noising = nullptr;

  m_EventStore = new MRawEventIncarnations();
}


////////////////////////////////////////////////////////////////////////////////


MRawEventAnalyzer::~MRawEventAnalyzer()
{
  // default destructor

  delete m_OriginGeometry;

  delete m_Reader;
  delete m_PhysFile;

  m_RawEvents->DeleteAll();
  delete m_RawEvents;

  delete m_InitialRawEvent;

  delete m_EventStore;
  
  delete m_Coincidence;
  delete m_Clusterizer;
  delete m_Tracker;
  delete m_CSR;
  delete m_Decay;
  delete m_Noising;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzer::SetGeometry(MGeometryRevan* Geometry)
{
  // Set the geometry
  
  massert(Geometry != nullptr);
  m_Geometry = Geometry;
  
  // The noising is based on the geometry data, thus set here
  delete m_Noising;
  m_Noising = new MERNoising();
  m_Noising->SetGeometry(m_Geometry);
  m_Noising->PreAnalysis();
}

 
////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzer::SetSettings(MSettingsEventReconstruction* S)
{
  // Transfer all necessary data to the analyzer...

  SetCoincidenceAlgorithm(S->GetCoincidenceAlgorithm()); 
  SetClusteringAlgorithm(S->GetClusteringAlgorithm()); 
  SetTrackingAlgorithm(S->GetTrackingAlgorithm()); 
  SetCSRAlgorithm(S->GetCSRAlgorithm());
  SetDecayAlgorithm(S->GetDecayAlgorithm());

  // coincidence
  SetCoincidenceWindow(S->GetCoincidenceWindow());

  // clustering
  SetStandardClusterizerMinDistanceD1(S->GetStandardClusterizerMinDistanceD1());
  SetStandardClusterizerMinDistanceD2(S->GetStandardClusterizerMinDistanceD2());
  SetStandardClusterizerMinDistanceD3(S->GetStandardClusterizerMinDistanceD3());
  SetStandardClusterizerMinDistanceD4(S->GetStandardClusterizerMinDistanceD4());
  SetStandardClusterizerMinDistanceD5(S->GetStandardClusterizerMinDistanceD5());
  SetStandardClusterizerMinDistanceD6(S->GetStandardClusterizerMinDistanceD6());
  SetStandardClusterizerMinDistanceD7(S->GetStandardClusterizerMinDistanceD7());
  SetStandardClusterizerMinDistanceD8(S->GetStandardClusterizerMinDistanceD8());
  SetStandardClusterizerCenterIsReference(S->GetStandardClusterizerCenterIsReference());
   
  SetAdjacentLevel(S->GetAdjacentLevel());
  SetAdjacentSigma(S->GetAdjacentSigma());

  SetPDFClusterizer(S->GetPDFClusterizerBaseFileName());
  
  // electron tracking
  SetSearchPairTracks(S->GetSearchPairs());
  SetSearchMIPTracks(S->GetSearchMIPs());
  SetSearchComptonTracks(S->GetSearchComptons());

  SetBETFileName(S->GetBayesianElectronFileName());

  SetMaxComptonJump(S->GetMaxComptonJump());
  SetNTrackSequencesToKeep(S->GetNTrackSequencesToKeep());
  SetRejectPurelyAmbiguousTrackSequences(S->GetRejectPurelyAmbiguousTrackSequences());
  SetNLayersForVertexSearch(S->GetNLayersForVertexSearch());
  
  SetElectronTrackingDetectorList(S->GetElectronTrackingDetectors());
  
  // compton tracking:
  SetAssumeD1First(S->GetAssumeD1First());
  SetClassicUndecidedHandling(S->GetClassicUndecidedHandling());
  SetUseComptelTypeEvents(S->GetUseComptelTypeEvents());
  SetGuaranteeStartD1(S->GetGuaranteeStartD1());

  SetRejectOneDetectorTypeOnlyEvents(S->GetRejectOneDetectorTypeOnlyEvents());

  SetCSRThresholdMin(S->GetCSRThresholdMin());
  SetCSRThresholdMax(S->GetCSRThresholdMax());

  SetCSRMaxNHits(S->GetCSRMaxNHits());

  SetOriginObjectsFileName(S->GetOriginObjectsFileName());
  
  SetBCTFileName(S->GetBayesianComptonFileName());
  
  SetTMVAFileName(S->GetTMVAFileName());
  SetTMVAMethods(S->GetTMVAMethods());
  
  SetLensCenter(S->GetLensCenter());
  SetFocalSpotCenter(S->GetFocalSpotCenter());

  // decay:
  SetDecayFileName(S->GetDecayFileName());

  // global options:
  SetTotalEnergyMax(S->GetTotalEnergyMax());
  SetTotalEnergyMin(S->GetTotalEnergyMin());

  SetLeverArmMax(S->GetLeverArmMax());
  SetLeverArmMin(S->GetLeverArmMin());

  SetEventIdMax(S->GetEventIdMax());
  SetEventIdMin(S->GetEventIdMin());

  SetRejectAllBadEvents(S->GetRejectAllBadEvents());
  
  SetSaveOI(S->GetSaveOI());
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzer::SetSaveOI(bool SaveOI) 
{ 
  //! Save the OI 

  m_SaveOI = SaveOI; 
  if (m_Reader != nullptr) {
    m_Reader->SaveOI(m_SaveOI); 
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::SetInputModeFile(MString Filename)
{
  delete m_Reader;
  m_Reader = nullptr;
  
  if (MFile::Exists(Filename) == false) {
    mout<<"MRawEventAnalyzer: Input file: \""<<Filename<<"\" does not exist!"<<endl;
    return false;
  }
  m_Filename = Filename;
  
  m_Reader = new MFileEventsEvta(m_Geometry);
  if (m_Reader->Open(m_Filename) == false) {
    mout<<"MRawEventAnalyzer: Unable to open input file \""<<m_Filename<<"\""<<endl;
    delete m_Reader;
    m_Reader = nullptr;
    return false;
  }
  m_Reader->SaveOI(m_SaveOI);
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::SetOutputModeFile(MString Filename)
{
  delete m_PhysFile;
  m_PhysFile = nullptr;
  
  if (Filename == "") {
    mout<<"MRawEventAnalyzer: No output file name given!"<<endl;
    return false;
  }
  m_FilenameOut = Filename;
  
  m_PhysFile = new MFileEventsTra();
  if (m_PhysFile->Open(m_FilenameOut, MFile::c_Write) == false) {
    mout<<"MRawEventAnalyzer: Unable to open output file \""<<m_FilenameOut<<"\""<<endl;
    delete m_PhysFile;
    m_PhysFile = nullptr;
    return false;
  }

  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::AddRawEvent(const MString& String, bool NeedsNoising, int Version)
{
  // Add a raw event to the store
  
  if (m_Reader != nullptr) {
    merr<<"You can only add events if you don't read them from file!"<<show;
    return false;
  }
  
  vector<MString> Lines = String.Tokenize("\n");

  MRESE::ResetIDCounter();
  MRERawEvent* RE = new MRERawEvent(m_Geometry);
  
  for (MString L: Lines) {
    if (RE->ParseLine(L, Version) == 1) {
      delete RE;
      cout<<"Parsing of line failed: "<<L<<endl;
      return false; 
    }
  }

  if (NeedsNoising == true) {
    m_Noising->Analyze(RE);
  }
  m_EventStore->AddRawEvent(RE);
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::AddRawEvent(MRERawEvent* RE)
{
  // Add a raw event to the store
  
  if (m_Reader != nullptr) {
    merr<<"You can only add events if you don't read them from file!"<<show;
    return false;
  }
  
  m_EventStore->AddRawEvent(RE);
  
  return true;
}

 
////////////////////////////////////////////////////////////////////////////////


unsigned int MRawEventAnalyzer::AnalyzeEvent()
{
  // A event timer...
  MTimer Timer;
  
  // this flag indicates that we have no more events in file, and the coincidence search has to "clear the store"
  bool ClearStore = false;
  
  // A temporary raw event...
  MRERawEvent* RE = nullptr;
  
  // Read events if we have reader
  if (m_Reader != nullptr) {
    RE = m_Reader->GetNextEvent();
    if (RE == nullptr) { // No more events left in file
      ClearStore = true;
      if (m_EventStore->GetNRawEvents() == 0) {
        return c_AnalysisNoEventsLeftInFile;
      }
    } else {
      m_EventStore->AddRawEvent(RE);
    }
    
    // Event timing...
    m_TimeLoad += Timer.ElapsedTime();
    Timer.Start();
  }
  
  if (m_EventStore->GetNRawEvents() == 0) {
    return c_AnalysisNoEventsInStore;
  }
  
  // We have events, we can start the analysis:
  
  // Section A: Coincidence search
  if (m_CoincidenceAlgorithm != c_CoincidenceAlgoNone) {
    if (m_Coincidence == nullptr) {
      merr<<"Coincidence pointer is zero. You changed the event reconstruction setup without calling PreAnalysis()!"<<show;
      return c_AnalysisUndefinedError;
    }
    
    RE = nullptr;
    
    // Search for coincidences
    if (ClearStore == true || m_EventStore->GetNRawEvents() >= 2) {
      RE = m_Coincidence->Search(m_EventStore, ClearStore);
    }
    
    // When does this case happen???
    if (RE == nullptr && m_EventStore->GetNRawEvents() == 0) {
      return c_AnalysisNoEventsInStore;
    }
    
    if (RE == nullptr) {
      return c_AnalysisCoincidenceWindowWait;
    }
    
    // Handle coincidences in the same voxel due to coincidence search - this should only happen in the simulation...
    if (RE != nullptr && m_CoincidenceAlgorithm != c_CoincidenceAlgoNone) {
      for (int r1 = 0; r1 < RE->GetNRESEs(); ++r1) {
        for (int r2 = r1+1; r2 < RE->GetNRESEs(); ++r2) {
          if (RE->GetRESEAt(r1)->GetPosition().AreEqual(RE->GetRESEAt(r2)->GetPosition(), 0.0000001) == true) {
            cout<<"Event "<<RE->GetEventID()<<"(t="<<RE->GetEventTime()<<"): Coincidence search lead to two interactions in same voxel within one event: "<<endl;
            cout<<RE->GetRESEAt(r1)->ToString();
            cout<<RE->GetRESEAt(r2)->ToString();
            cout<<"Merging hits..."<<endl;
            RE->GetRESEAt(r1)->SetEnergy(RE->GetRESEAt(r1)->GetEnergy() + RE->GetRESEAt(r2)->GetEnergy());
            RE->DeleteRESEAndCompress(RE->GetRESEAt(r2));
            // Restart:
            r2 = r1+1;
          }
        }
      }
    }
  } // no coincidence search
  else {
    RE = m_EventStore->GetRawEventAt(0);
    m_EventStore->RemoveRawEvent(RE);
  }
  
  if (RE == nullptr) {
    merr<<"We don't have a raw event..."<<fatal;
    return c_AnalysisUndefinedError;
  }
  
  mdebug<<endl;
  mdebug<<endl;
  mdebug<<endl;
  mdebug<<"ER - Event: "<<RE->GetEventID()<<endl;
  mdebug<<endl;

  
  // Store the inital coincident event:
  if (m_InitialRawEvent != nullptr) {
    delete m_InitialRawEvent;
    m_InitialRawEvent = nullptr;
  }
  m_InitialRawEvent = RE->Duplicate();
  

  // Set the initial event and clean the remainders of the last event
  m_RawEvents->SetInitialRawEvent(RE);
  
  
  // Check for event selections:
  bool SelectionsPassed = true;
  if (SelectionsPassed == true && 
      RE->GetExternalBadEventFlag() == true &&
      m_RejectAllBadEvents == true) {
    mdebug<<"ER - Selection: External bad event flag raised: "<<RE->GetExternalBadEventString()<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionExternalBadEventFlag);
    SelectionsPassed = false;
  }

  if (SelectionsPassed == true && 
      ((RE->GetEventID() < m_EventIdMin && m_EventIdMin >= 0) || 
       (RE->GetEventID() > m_EventIdMax && m_EventIdMax >= 0))) {
    mdebug<<"ER - Selection: Event ID out of limits: "<<RE->GetEventID()
        <<" is not within ["<<m_EventIdMin<<", "<<m_EventIdMax<<"]"<<endl;
    RE->SetRejectionReason(MRERawEvent::c_RejectionEventIdOutOfLimits);
    if (RE->GetEventID() > m_EventIdMax && m_EventIdMax >= 0) {
      mout<<"ER - Event ID above limit!"<<endl;
      return c_AnalysisUndefinedError;
    }
    SelectionsPassed = false;      
  }
  
  if (SelectionsPassed == true && 
      (RE->GetEnergy() < m_TotalEnergyMin || RE->GetEnergy() > m_TotalEnergyMax)) {
    mdebug<<"ER - Selection: Total energy out of limits: "<<RE->GetEnergy()
        <<" keV is not within ["<<m_TotalEnergyMin<<", "<<m_TotalEnergyMax<<"] keV"<<endl; 
    RE->SetRejectionReason(MRERawEvent::c_RejectionTotalEnergyOutOfLimits);
    SelectionsPassed = false;
  }

  
  
  // Analyze the event
  if (SelectionsPassed == true) {

    // Section B: Clusterizing:
    Timer.Start();

    if (SelectionsPassed == true && m_ClusteringAlgorithm > c_ClusteringAlgoNone) {
      if (m_Clusterizer == nullptr) {
        merr<<"Clusterizer pointer is zero. You changed the event reconstruction setup without calling PreAnalysis()!"<<show;
        return c_AnalysisUndefinedError;
      }
      
      m_Clusterizer->Analyze(m_RawEvents);
      // Since we have exactly one event, it is automatically the best event:
      if (m_RawEvents->GetNRawEvents() == 1) {
        //m_RawEvents->SetBestEvent(m_RawEvents->GetRawEventAt(0));
      } else {
        merr<<"ER - We should have only one good event here..."<<endl;
      }
    }
    m_RawEvents->SetBestTryEvent(m_RawEvents->GetRawEventAt(0));

    m_TimeClusterize += Timer.ElapsedTime();

    
    // Section C: Tracking:
    Timer.Start();
    if (SelectionsPassed == true && m_TrackingAlgorithm > c_TrackingAlgoNone) {
      if (m_Tracker == nullptr) {
        merr<<"Tracker pointer is zero. You changed the event reconstruction setup without calling PreAnalysis()!"<<show;
        return c_AnalysisUndefinedError;
      }
      m_Tracker->Analyze(m_RawEvents);
    } else {
      mdebug<<"I am not doing Tracking!"<<endl;
    }
    m_TimeTrack += Timer.ElapsedTime();
    Timer.Start();


    // Now we have possibly found: showers, MIPS and of course pairs
    if (SelectionsPassed == true && m_RawEvents->HasOptimumEvent() == false) {

      // Check lever arm selection:
      for (int rw = 0; rw < m_RawEvents->GetNRawEvents(); ++rw) {
        MRERawEvent* RW = m_RawEvents->GetRawEventAt(rw);
        for (int r = 0; r < RW->GetNRESEs(); ++r) {
          for (int s = r+1; s < RW->GetNRESEs(); ++s) {
            if ((RW->GetRESEAt(r)->GetPosition() - RW->GetRESEAt(s)->GetPosition()).Mag() < m_LeverArmMin ||
                (RW->GetRESEAt(r)->GetPosition() - RW->GetRESEAt(s)->GetPosition()).Mag() > m_LeverArmMax) {
              mdebug<<"ER - Selection: Lever arm out of limits: "<<(RW->GetRESEAt(r)->GetPosition() - RW->GetRESEAt(s)->GetPosition()).Mag()
                  <<" cm is not within ["<<m_LeverArmMin
                  <<", "<<m_LeverArmMax<<"] cm"<<endl;
              RW->SetRejectionReason(MRERawEvent::c_RejectionLeverArmOutOfLimits);
              SelectionsPassed = false;
              break;
            }
          }
          if (SelectionsPassed == false) break;
        }
      }

      // Section D: Compton sequence reconstruction     
      Timer.Start();
      if (SelectionsPassed == true && m_RawEvents->HasOptimumEvent() == false && m_CSRAlgorithm > c_CSRAlgoNone) {
        if (m_CSR == nullptr) {
          merr<<"CSR pointer is zero. You changed the event reconstruction setup without calling PreAnalysis()!"<<show;
          return c_AnalysisUndefinedError;
        }
        // so Comptons are next
        m_CSR->Analyze(m_RawEvents);
      } else {
        mdebug<<"I am not doing CSR!"<<endl;
      }

      if (SelectionsPassed == true && m_DecayAlgorithm > c_DecayAlgoNone) {
        m_Decay->Analyze(m_RawEvents);
      } else {
        mdebug<<"I am not doing Decay!"<<endl;
      }
      m_TimeCSR += Timer.ElapsedTime();
    }


    // Final stuff 
    Timer.Start();
  
    if (m_RawEvents->HasOptimumEvent() == true) {
      mdebug<<"ER - Optimum event found!"<<endl;
    } else {
      mdebug<<"ER - NO optimum event found!"<<endl;
    }

    // If we have a good event, then write it
    if (m_RawEvents->HasOptimumEvent() == true && m_PhysFile != nullptr) {
      mdebug<<m_RawEvents->GetOptimumEvent()->ToString()<<endl;
      // Write the event:
      MPhysicalEvent* Event = m_RawEvents->GetOptimumEvent()->GetPhysicalEvent();
   
      if (m_PhysFile != nullptr) {
        if (m_PhysFile->AddEvent(Event) == false) {
          merr<<"Saving the event failed!"<<show;
          return c_AnalysisSavingEventFailed;
        }
      }
      mdebug<<Event->ToString()<<endl;
      m_NGoodEvents++;
      if (Event->GetType() == MPhysicalEvent::c_Photo) {
        mdebug<<"ER - Good photo event..."<<endl;
        m_NPhotoEvents++;
      } else if (Event->GetType() == MPhysicalEvent::c_Compton) {
        if (m_RawEvents->GetOptimumEvent()->GetPhysicalEvent()->IsDecay() == true) {
          mdebug<<"ER - Compton event - probably decay..."<<endl;
          m_NDecayEvents++;
        } else {
          mdebug<<"ER - Good Compton event..."<<endl;
        }
        m_NComptonEvents++;
      } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
        mdebug<<"ER - Good pair event..."<<endl;
        m_NPairEvents++;
      } else if (Event->GetType() == MPhysicalEvent::c_Muon) {
        mdebug<<"ER - Good muon event..."<<endl;
        m_NMuonEvents++;
      } else if (Event->GetType() == MPhysicalEvent::c_Unidentifiable) {
        mdebug<<"ER - Undefinable event..."<<endl;
        m_NUnidentifiableEvents++;
      } else {
        merr<<"ER - Unhandled event type: "<<Event->GetType()<<endl;
      }
    } else {
      MRERawEvent* BestTry = nullptr;
      if (m_RawEvents->GetBestTryEvent() != nullptr) {
        BestTry = m_RawEvents->GetBestTryEvent();
      } else if (m_RawEvents->GetNRawEvents() > 0) {
        mdebug<<"ER - Multiple raw events and no best one ... grummel ... Grabbing the last one for rejection reason statistics ..."<<endl;
        BestTry = m_RawEvents->GetRawEventAt(m_RawEvents->GetNRawEvents()-1);
      }

      if (BestTry != nullptr) {
        mdebug<<"ER - Second best event structure..."<<endl;
        mdebug<<BestTry->ToString()<<endl;
        while (m_Rejections.size() < (unsigned int) BestTry->GetRejectionReason()+1) {
          m_Rejections.push_back(0);
        }
        m_Rejections[(unsigned int) BestTry->GetRejectionReason()]++;
        mdebug<<"ER - Rejection: "
            <<MRERawEvent::GetRejectionReasonAsString(BestTry->GetRejectionReason())<<endl;
      
        MPhysicalEvent* Event = BestTry->GetPhysicalEvent();
        if (m_PhysFile != nullptr) {
          if (m_PhysFile->AddEvent(Event) == false) {
            merr<<"Saving the event failed!"<<show;
            return c_AnalysisSavingEventFailed;
          }
        }
        mdebug<<BestTry->GetPhysicalEvent()->ToString()<<endl;
      }
    }
  }

  m_NEvents++;
  if (SelectionsPassed == true) {
    m_NPassedEventSelection++;  
  }

  m_TimeFinalize += Timer.ElapsedTime();
  
  
  return c_AnalysisSucess;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventAnalyzer::GetNextInitialRawEventFromFile()
{
  // Return the next initial raw event
  // This loops over all events
  // If the return value == nullptr then no more events are available

  if (m_Reader == nullptr) return nullptr;

  return m_Reader->GetNextEvent();
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzer::AddRejectionReason(MRERawEvent* RE)
{
  while (m_Rejections.size() < (unsigned int) RE->GetRejectionReason()+1) {
    m_Rejections.push_back(0);
  }
  m_Rejections[(unsigned int) RE->GetRejectionReason()]++;
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventAnalyzer::GetOptimumEvent()
{
  // Return the last raw event or 0 if it is not valid

  return m_RawEvents->GetOptimumEvent();
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventAnalyzer::GetBestTryEvent()
{
  // Return the last raw event, before abort or 0 if it is not valid

  return m_RawEvents->GetBestTryEvent();
}


////////////////////////////////////////////////////////////////////////////////


MRERawEvent* MRawEventAnalyzer::GetInitialRawEvent()
{
  // Return the initial raw event

  return m_InitialRawEvent;
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzer::JoinStatistics(const MRawEventAnalyzer& A)
{
  m_NEvents += A.m_NEvents;
  m_NPassedEventSelection += A.m_NPassedEventSelection;
  m_NGoodEvents += A.m_NGoodEvents;
  
  m_NPhotoEvents += A.m_NPhotoEvents;
  m_NComptonEvents += A.m_NComptonEvents;
  m_NPairEvents += A.m_NPairEvents;
  m_NMuonEvents += A.m_NMuonEvents;
  m_NDecayEvents += A.m_NDecayEvents;
  m_NUnidentifiableEvents += A.m_NUnidentifiableEvents;


  for (unsigned int r = 0; r < m_Rejections.size(); ++r) {
    m_Rejections[r] += A.m_Rejections[r];
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::PostAnalysis()
{
  // No more events available
  
  if (m_PhysFile != nullptr) {
    m_PhysFile->SetObservationTime(m_Reader->GetObservationTime());
  } 
  
  
  ostringstream out;

  out<<endl;
  out<<"---------------------------------------------------------------------------"<<endl;
  out<<endl;

  // Store all options:
  out<<"# Tracked events file options"<<endl;
  MTime T;
  T.Now();
  out<<"# Date "<<T.GetSQLString()<<endl;
  out<<"# OriginalFile "<<m_Filename<<endl;
  out<<"# GeometryFile "<<m_Geometry->GetFileName()<<endl;
  if (m_Clusterizer != nullptr) {
    out<<"# "<<endl;
    out<<m_Clusterizer->ToString();
  }
  if (m_Tracker != nullptr) {
    out<<"# "<<endl;
    out<<m_Tracker->ToString();
  }
  if (m_CSR != nullptr) {
    out<<"# "<<endl;
    out<<m_CSR->ToString();
  }
  if (m_Decay != nullptr) {
    out<<"# "<<endl;
    out<<m_Decay->ToString();
  }

  int Width = 6;
  int WidthPercent = 7;
  int Precision = 3;
  
  out<<endl;
  out<<"----------------------------------------------------------------------------"<<endl;
  if (m_Reader != nullptr) {
    out<<endl;
    out<<m_Reader->GetERNoising()->ToString();
    out<<endl;
    out<<"----------------------------------------------------------------------------"<<endl;
    m_Reader->Close();
  }
  out<<endl;
  out<<"Event statistics for all triggered (!) events:"<<endl;
  out<<fixed;
  if (m_NEvents > 0) {
    out<<"  Number of events ....................................... "
        <<setw(Width)<<m_NEvents<<" (100.000%)"<<endl;
    out<<"  Number of events, which passed event selections ........ "
       <<setw(Width)<<m_NPassedEventSelection<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NEvents == 0) ? 0 : 100.0*m_NPassedEventSelection/m_NEvents)<<"%)"<<setprecision(6)<<endl;
    out<<"  Reconstructable events ................................. "
       <<setw(Width)<<m_NGoodEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NEvents == 0) ? 0 : 100.0*m_NGoodEvents/m_NEvents)<<"%)"<<setprecision(6)<<endl;

    out<<"       Single-site  ...................................... "
       <<setw(Width)<<m_NPhotoEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NGoodEvents == 0) ? 0 : 100.0*m_NPhotoEvents/m_NGoodEvents)<<"%)"<<setprecision(6)<<endl;
    out<<"       Compton  .......................................... "
       <<setw(Width)<<m_NComptonEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NGoodEvents == 0) ? 0 : 100.0*m_NComptonEvents/m_NGoodEvents)<<"%)"<<setprecision(6)<<endl;
    out<<"           Decay  ........................................ "
       <<setw(Width)<<m_NDecayEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NComptonEvents == 0) ? 0 : 100.0*m_NDecayEvents/m_NComptonEvents)<<"%)"<<setprecision(6)<<endl;
    out<<"       Pair  ............................................. "
       <<setw(Width)<<m_NPairEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NGoodEvents == 0) ? 0 : 100.0*m_NPairEvents/m_NGoodEvents)<<"%)"<<setprecision(6)<<endl;
    out<<"       Muon  ............................................. "
       <<setw(Width)<<m_NMuonEvents<<" ("<<setw(WidthPercent)<<setprecision(Precision)
       <<((m_NGoodEvents == 0) ? 0 : 100.0*m_NMuonEvents/m_NGoodEvents)<<"%)"<<setprecision(6)<<endl;
    out<<endl;

    out<<"Rejection reasons for not reconstructable events:"<<endl;
    MString Reason;
    int Total = 0;
    for (unsigned int r = 0; r < m_Rejections.size(); ++r) {
      if (m_Rejections[r] > 0) {
        Reason = MRERawEvent::GetRejectionReasonAsString(int(r));
        out<<"  "<<MRERawEvent::GetRejectionReasonAsString(int(r))<<" ";
        for (int s = Reason.Length(); s < 55; ++s) out<<".";
        out<<" "<<setw(Width)<<m_Rejections[r]<<endl;
        Total += m_Rejections[r];
      }
    }
    out<<"    Total ................................................ "<<setw(Width)<<Total<<endl;
  } else {
    out<<endl;
    out<<"  No events available"<<endl;
  }
  out<<endl;
  out<<"----------------------------------------------------------------------------"<<endl;
  out<<endl;


  if (m_PhysFile != nullptr) {
    m_PhysFile->CloseEventList();
    m_PhysFile->AddFooter(out.str().c_str());
    m_PhysFile->Close();
  }

  mout<<out.str().c_str()<<endl;

  /*
  mout<<"General timings:"<<endl;
  mout<<"Timer Load     "<<m_TimeLoad<<endl;
  mout<<"Timer Cluster  "<<m_TimeClusterize<<endl;
  mout<<"Timer Track    "<<m_TimeTrack<<endl;
  mout<<"Timer CSR      "<<m_TimeCSR<<endl;
  mout<<"Timer Finalize "<<m_TimeFinalize<<endl;
  mout<<endl;
  */
  
  if (m_Clusterizer != nullptr) {
    m_Clusterizer->PostAnalysis();
  }
  if (m_Tracker != nullptr) {
    m_Tracker->PostAnalysis();
  }
  if (m_CSR != nullptr) {
    m_CSR->PostAnalysis();
  }
  if (m_Decay != nullptr) {
    m_Decay->PostAnalysis();
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MRawEventAnalyzer::PreAnalysis()
{
  // 

  bool Return = false;

  m_NEvents = 0;
  m_NGoodEvents = 0;
  
  if (m_OriginObjectsFileName != "") {
    if (MFile::FileExists(m_OriginObjectsFileName) == true) {
      delete m_OriginGeometry;
      m_OriginGeometry = new MGeometryRevan();
      if (m_OriginGeometry->ScanSetupFile(m_OriginObjectsFileName) == true) {
        mout<<"Geometry "<<m_OriginGeometry->GetName()<<" loaded!"<<endl;
      } else {
        mout<<"MRawEventAnalyzer: Loading of geometry "<<m_OriginObjectsFileName<<" failed!!"<<endl;
        delete m_OriginGeometry;
        m_OriginGeometry = nullptr;
        Return = false;
      }
    } else {
      mout<<"MRawEventAnalyzer: Loading of geometry "<<m_OriginObjectsFileName<<" failed!!"<<endl;
      delete m_OriginGeometry;
      m_OriginGeometry = nullptr;
      Return = false;
    }
  }


  if (m_Filename != "") {
    if (m_Reader != nullptr && m_Reader->IsOpen() == true) {
      m_Reader->ShowProgress(!m_IsBatch);
      if (m_PhysFile != nullptr) {
        if (m_PhysFile->IsOpen() == true) {
          Return = true;
        } else {
          Return = false;
        }
      } else {
        Return = true;
      }
    }
  } else {
    Return = true;
  }

  if (Return == true) {
    if (m_Geometry == nullptr) {
      cout<<"MRawEventAnalyzer: A geometry is required!"<<endl;
      Return = false;
    }
  }
  
  
  // Coincidence
  if (Return == true) {
    delete m_Coincidence;
    m_Coincidence = new MERCoincidence();
    if (m_CoincidenceAlgorithm == c_CoincidenceAlgoWindow) {
      m_Coincidence->SetCoincidenceWindow(m_CoincidenceWindow);
    } else if (m_CoincidenceAlgorithm == c_CoincidenceAlgoNone) {
      // Nothing
    } else {
      merr<<"Unknown coincidence algorithm: "<<m_CoincidenceAlgorithm<<endl;
      Return = false;
    }
  }

  
  // Clusterining
  if (Return == true) {
    delete m_Clusterizer;
    m_Clusterizer = nullptr;
    if (m_ClusteringAlgorithm == c_ClusteringAlgoDistance) {
      m_Clusterizer = new MERClusterize();
      if (m_Clusterizer->SetParameters(m_StandardClusterizerMinDistanceD1, 
                                       m_StandardClusterizerMinDistanceD2,
                                       m_StandardClusterizerMinDistanceD3,
                                       m_StandardClusterizerMinDistanceD4,
                                       m_StandardClusterizerMinDistanceD5,
                                       m_StandardClusterizerMinDistanceD6,
                                       m_StandardClusterizerMinDistanceD7,
                                       m_StandardClusterizerMinDistanceD8,
                                       m_StandardClusterizerCenterIsReference) == false) {
        Return = false;
      }
    } else if (m_ClusteringAlgorithm == c_ClusteringAlgoPDF) {
      m_Clusterizer = new MERClusterize();
      if (m_Clusterizer->SetParameters(m_PDFClusterizerBaseFileName) == false) {
        Return = false;
      }
    } else if (m_ClusteringAlgorithm == c_ClusteringAlgoAdjacent) {
      m_Clusterizer = new MERClusterize();
      if (m_Clusterizer->SetParameters(m_AdjacentLevel, m_AdjacentSigma) == false) {
        Return = false;
      }
    } else if (m_ClusteringAlgorithm == c_ClusteringAlgoNone) {
      // Nothing
    } else {
      merr<<"Unknown clustering algorithm: "<<m_ClusteringAlgorithm<<endl;
      Return = false;
    }

    // Compton sequence reconstruction
    delete m_CSR;
    m_CSR = nullptr;
    if (m_CSRAlgorithm == c_CSRAlgoFoM) {
      m_CSR = new MERCSRChiSquare();
      if (dynamic_cast<MERCSRChiSquare*>(m_CSR)->SetParameters(m_Geometry, 
                                                               m_CSRThresholdMin, 
                                                               m_CSRThresholdMax, 
                                                               m_CSRMaxNHits, 
                                                               m_GuaranteeStartD1, 
                                                               m_CSROnlyCreateSequences, 
                                                               m_UseComptelTypeEvents, 
                                                               m_ClassicUndecidedHandling, 
                                                               m_RejectOneDetectorTypeOnlyEvents, 
                                                               m_OriginGeometry) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoFoME) {
      m_CSR = new MERCSREnergyRecovery();
      if (dynamic_cast<MERCSREnergyRecovery*>(m_CSR)->SetParameters(m_Geometry, 
                                 m_CSRThresholdMin, 
                                 m_CSRThresholdMax, 
                                 m_CSRMaxNHits, 
                                 m_GuaranteeStartD1, 
                                 m_CSROnlyCreateSequences) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoFoMToF) {
      m_CSR = new MERCSRToF();
      if (dynamic_cast<MERCSRToF*>(m_CSR)->SetParameters(m_Geometry, 
                                    m_CSRThresholdMin, 
                                    m_CSRThresholdMax, 
                                    m_CSRMaxNHits, 
                                    m_GuaranteeStartD1, 
                                    m_CSROnlyCreateSequences) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoFoMToFAndE) {
      m_CSR = new MERCSRToFWithEnergyRecovery();
      if (dynamic_cast<MERCSRToFWithEnergyRecovery*>(m_CSR)->SetParameters(m_Geometry, 
                                    m_CSRThresholdMin, 
                                    m_CSRThresholdMax, 
                                    m_CSRMaxNHits, 
                                    m_GuaranteeStartD1, 
                                    m_CSROnlyCreateSequences) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoBayesian) {
      m_CSR = new MERCSRBayesian();
      if (dynamic_cast<MERCSRBayesian*>(m_CSR)->SetParameters(m_BCTFileName, 
                                      m_Geometry, 
                                      m_CSRThresholdMin, 
                                      m_CSRThresholdMax, 
                                      m_CSRMaxNHits, 
                                      m_GuaranteeStartD1, 
                                      m_CSROnlyCreateSequences) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoTMVA) {
      m_CSR = new MERCSRTMVA();
      if (dynamic_cast<MERCSRTMVA*>(m_CSR)->SetParameters(m_TMVAFileName, 
        m_TMVAMethods,
        m_Geometry, 
        m_CSRThresholdMin, 
        m_CSRThresholdMax, 
        m_CSRMaxNHits, 
        m_GuaranteeStartD1, 
        m_CSROnlyCreateSequences) == false) {
        Return = false;
      }
    } else if (m_CSRAlgorithm == c_CSRAlgoNone) {
      // Nothing
    } else {
      merr<<"Unknown compton sequence reocnstruction algorithm: "<<m_CSRAlgorithm<<endl;
      Return = false;
    }

    
    // Electron tracking
    delete m_Tracker;
    m_Tracker = nullptr;
    if (m_TrackingAlgorithm == c_TrackingAlgoModifiedPearson) {
      m_Tracker = new MERTrack();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoPearson) {
      m_Tracker = new MERTrackPearson();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoRank) {
      m_Tracker = new MERTrackRank();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoChiSquare) {
      m_Tracker = new MERTrackChiSquare();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoGas) {
      m_Tracker = new MERTrackGas();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoDirectional) {
      m_Tracker = new MERTrackDirectional();
    } else if (m_TrackingAlgorithm == c_TrackingAlgoBayesian) {
      m_Tracker = new MERTrackBayesian();
      if (dynamic_cast<MERTrackBayesian*>(m_Tracker)->
          SetSpecialParameters(m_BETFileName) == false) {
        Return = false;
      }
    } else if (m_TrackingAlgorithm == c_TrackingAlgoNone) {
      // Nothing
    } else {
      merr<<"Unknown electron tracking algorithm: "<<m_TrackingAlgorithm<<endl;
      Return = false;
    }

    if (m_Tracker != nullptr) {
      m_Tracker->SetGeometry(m_Geometry);
      m_Tracker->SetParameters(m_SearchMIPTracks, 
                               m_SearchPairTracks, 
                               m_SearchComptonTracks, 
                               m_MaxComptonJump, 
                               m_NTrackSequencesToKeep, 
                               m_RejectPurelyAmbiguousTrackSequences, 
                               m_NLayersForVertexSearch,
                               m_ElectronTrackingDetectorList);
    }

    
    // Initialize the Decay searcher:
    delete m_Decay;
    m_Decay = nullptr;
    if (m_DecayAlgorithm == c_DecayAlgoStandard) {
      m_Decay = new MERDecay();
      if (m_Decay->SetParameters(m_DecayFileName, m_DecayEnergy, m_DecayEnergyError) == false) {
        Return = false;
      }
    } else if (m_DecayAlgorithm == c_DecayAlgoNone) {
      // Nothing
    } else {
      merr<<"Unknown decay detection algorithm: "<<m_DecayAlgorithm<<endl;
      Return = false;
    }
  }

  if (Return == false) {
    mout<<"MRawEventAnalyzer: Pre-analysis failed!"<<endl;
  } else {
    if (m_PhysFile != nullptr) {
      m_PhysFile->SetVersion(1);
      m_PhysFile->SetGeometryFileName(m_Geometry->GetFileName());
      m_PhysFile->WriteHeader();
    }
  }

  return Return;
}


// MRawEventAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
