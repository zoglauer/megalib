/*
 * MRealTimeAnalyzer.cxx
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
// MRealTimeAnalyzer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRealTimeAnalyzer.h"

// Standard libs:
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

// ROOT libs:
#include "TObjString.h"
#include "TObjArray.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MTimer.h"
#include "MTime.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MGeometryRevan.h"
#include "MRERawEvent.h"
#include "MREHit.h"
#include "MRawEventAnalyzer.h"
#include "MERCSRChiSquare.h"
#include "MBPData.h"
#include "MImage.h"
#include "MImageSpheric.h"
#include "MImageGalactic.h"
#include "MImage2D.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MMuonEvent.h"
#include "MUnidentifiableEvent.h"
#include "MSpectralAnalyzer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRealTimeAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


void* StartTransmissionThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneTransmissionLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartCoincidenceThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneCoincidenceLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartReconstructionThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneReconstructionLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartImagingThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneImagingLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartHistogrammingThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneHistogrammingLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartIdentificationThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneIdentificationLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void* StartCleanUpThread(void* Analysis)
{
  ((MRealTimeAnalyzer *) Analysis)->OneCleanUpLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MRealTimeAnalyzer::m_ThreadId = 0;


////////////////////////////////////////////////////////////////////////////////


MRealTimeAnalyzer::MRealTimeAnalyzer()
{
  // Construct an instance of MRealTimeAnalyzer
  
  m_NEvents = 0;
  m_MaxNEvents = 1000000;
 
  m_AccumulationTime = 100;
  
  m_GeometryFileName = "$(MEGALIB)/resource/examples/geomega/mpesatellitebaseline/SatelliteWithACS.geo.setup";
  
  m_TransmissionThread = 0;
  m_IsTransmissionThreadRunning = false;
  m_TransmissionThreadCpuUsage = 0.0;
  m_TransmissionThreadLastEventID = 0;
  
  m_CoincidenceThread = 0;
  m_IsCoincidenceThreadRunning = false;
  m_CoincidenceThreadCpuUsage = 0.0;
  m_CoincidenceThreadLastEventID = 0;
  
  m_ReconstructionThread = 0;
  m_IsReconstructionThreadRunning = false;
  m_ReconstructionThreadCpuUsage = 0.0;
  m_ReconstructionThreadLastEventID = 0;
  
  m_ImagingThread = 0;
  m_IsImagingThreadRunning = false;
  m_ImagingThreadCpuUsage = 0.0;
  m_ImagingThreadLastEventID = 0;

  m_HistogrammingThread = 0;
  m_IsHistogrammingThreadRunning = false;
  m_HistogrammingThreadCpuUsage = 0.0;
  m_HistogrammingThreadLastEventID = 0;
  m_HistogrammingThreadFirstEventID = 0;

  m_IdentificationThread = 0;
  m_IsIdentificationThreadRunning = false;
  m_IdentificationThreadCpuUsage = 0.0;
  m_IdentificationThreadLastEventID = 0;
  m_IdentificationThreadFirstEventID = 0;

  m_CleanUpThread = 0;
  m_IsCleanUpThreadRunning = false;
  m_CleanUpThreadCpuUsage = 0.0;
  m_CleanUpThreadLastEventID = 0;

  m_StopThreads = false;
  m_IsAnalysisRunning = false;
  m_IsInitializing = false;
  m_IsConnected = false;
  m_DoDisconnect = false;
  
  m_Spectrum = nullptr;
  m_CountRate = nullptr;
  m_Image = nullptr;
  
  m_Settings = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MRealTimeAnalyzer::~MRealTimeAnalyzer()
{
  // Delete this instance of MRealTimeAnalyzer
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::SetAccumulationTime(double AccumulationTime) 
{ 
  //! Set the accumulation time
  
  m_AccumulationTimeMutex.Lock();
  m_AccumulationTime = AccumulationTime; 
  m_AccumulationTimeMutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::SetSettings(MSettingsRealta* Settings) 
{ 
  //! Set all the user definable settings

  m_Settings = Settings; 
  SetAccumulationTime(m_Settings->GetAccumulationTime()); 
}

////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::StartAnalysis()
{
  // Starts the multithreaded TransceiverLoop

  if (m_Settings == 0) { 
    merr<<"You have to set the user settings first!"<<show;
    return;
  }

  cout<<"Powering up analysis sequence"<<endl;

  m_IsInitializing = true;
  m_StopThreads = false;

  m_IsTransmissionThreadRunning = false;
  m_IsCoincidenceThreadRunning = false;
  m_IsReconstructionThreadRunning = false;
  m_IsImagingThreadRunning = false;
  m_IsHistogrammingThreadRunning = false;
  m_IsIdentificationThreadRunning = false;
  m_IsCleanUpThreadRunning = false;
  
  m_TransmissionGeometry = nullptr;
  m_CoincidenceGeometry = nullptr;
  m_ReconstructionGeometry = nullptr;
  m_ImagingGeometry = nullptr;
  m_IdentificationGeometry = nullptr;
  
  if (m_TransmissionThread == 0) {
    m_ThreadId++;
    MString Name = "TransmissionThread ";
    Name += m_ThreadId;

    m_TransmissionThread = 
      new TThread(Name, 
                  (void(*) (void *)) &StartTransmissionThread, 
                  (void*) this);
    m_TransmissionThread->SetPriority(TThread::kHighPriority);
    m_TransmissionThread->Run();
    
    while (m_IsTransmissionThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents(); // We cannot have GUI events during start up...
    }
  }
  
  if (m_CoincidenceThread == 0) {
    m_ThreadId++;
    MString Name  = "CoincidenceThread ";
    Name += m_ThreadId;

    m_CoincidenceThread = 
      new TThread(Name, 
                  (void(*) (void *)) &StartCoincidenceThread, 
                  (void*) this);
    m_CoincidenceThread->SetPriority(TThread::kHighPriority);
    m_CoincidenceThread->Run();
    
    while (m_IsCoincidenceThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  } 

  if (m_ReconstructionThread == 0) {
    m_ThreadId++;
    MString Name  = "ReconstructionThread ";
    Name += m_ThreadId;

    m_ReconstructionThread = 
      new TThread(Name, 
                  (void(*) (void *)) &StartReconstructionThread, 
                  (void*) this);
    m_ReconstructionThread->SetPriority(TThread::kHighPriority);
    m_ReconstructionThread->Run();
    
    while (m_IsReconstructionThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  } 

  if (m_ImagingThread == 0) {
    m_ThreadId++;
    MString Name  = "ImagingThread ";
    Name += m_ThreadId;

    m_ImagingThread = 
      new TThread(Name, 
                  (void(*) (void *)) &StartImagingThread, 
                  (void*) this);
    m_ImagingThread->SetPriority(TThread::kHighPriority);
    m_ImagingThread->Run();
    
    while (m_IsImagingThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  } 

  if (m_HistogrammingThread == 0) {
    m_ThreadId++;
    MString Name  = "HistogrammingThread ";
    Name += m_ThreadId;

    m_HistogrammingThread =
      new TThread(Name,
                  (void(*) (void *)) &StartHistogrammingThread,
                  (void*) this);
    m_HistogrammingThread->SetPriority(TThread::kHighPriority);
    m_HistogrammingThread->Run();

    while (m_IsHistogrammingThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  }

  if (m_IdentificationThread == 0) {
    m_ThreadId++;
    MString Name  = "IdentificationThread ";
    Name += m_ThreadId;

    m_IdentificationThread =
      new TThread(Name,
                  (void(*) (void *)) &StartIdentificationThread,
                  (void*) this);
    m_IdentificationThread->SetPriority(TThread::kHighPriority);
    m_IdentificationThread->Run();

    while (m_IsIdentificationThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  }

  if (m_CleanUpThread == 0) {
    m_ThreadId++;
    MString Name  = "CleanUpThread ";
    Name += m_ThreadId;

    m_CleanUpThread =
      new TThread(Name,
                  (void(*) (void *)) &StartCleanUpThread,
                  (void*) this);
    m_CleanUpThread->SetPriority(TThread::kHighPriority);
    m_CleanUpThread->Run();

    while (m_IsCleanUpThreadRunning == false) {
      gSystem->Sleep(10);
      //gSystem->ProcessEvents();
    }
  }
  
  m_IsAnalysisRunning = true;
  m_IsInitializing = false;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::StopAnalysis()
{
  if (m_IsInitializing == true) {
    cout<<"Cannot stop until the initalization is finished! --- Try again later..."<<endl;
    return;
  }

  cout<<"Starting shutdown sequence..."<<endl;
  
  m_StopThreads = true;
  
  while (m_IsInitializing == true ||
         m_IsTransmissionThreadRunning == true || 
         m_IsCoincidenceThreadRunning == true || 
         m_IsReconstructionThreadRunning == true || 
         m_IsImagingThreadRunning == true || 
         m_IsHistogrammingThreadRunning == true ||
         m_IsIdentificationThreadRunning == true ||
         m_IsCleanUpThreadRunning == true) {
    gSystem->Sleep(200);
    if (m_IsInitializing == true) cout<<"Waiting for initalization to finish"<<endl;
    if (m_IsTransmissionThreadRunning == true) cout<<"Waiting for Transmission thread"<<endl;
    if (m_IsCoincidenceThreadRunning == true) cout<<"Waiting for Coincidence thread"<<endl;
    if (m_IsReconstructionThreadRunning == true) cout<<"Waiting for reconstruction thread"<<endl;
    if (m_IsImagingThreadRunning == true) cout<<"Waiting for imaging thread"<<endl;
    if (m_IsHistogrammingThreadRunning == true) cout<<"Waiting for histogramming thread"<<endl;
    if (m_IsIdentificationThreadRunning == true) cout<<"Waiting for identification thread"<<endl;
    if (m_IsCleanUpThreadRunning == true) cout<<"Waiting for cleanup thread"<<endl;
 }
  
  if (m_TransmissionThread != 0) m_TransmissionThread->Kill();
  m_TransmissionThread = 0;
  m_IsTransmissionThreadRunning = false;
  delete m_TransmissionGeometry;
  
  if (m_CoincidenceThread != 0) m_CoincidenceThread->Kill();
  m_CoincidenceThread = 0;
  m_IsCoincidenceThreadRunning = false;
  delete m_CoincidenceGeometry;
  
  if (m_ReconstructionThread != 0) m_ReconstructionThread->Kill();
  m_ReconstructionThread = 0;
  m_IsReconstructionThreadRunning = false;
  delete m_ReconstructionGeometry;
  
  if (m_ImagingThread != 0) m_ImagingThread->Kill();
  m_ImagingThread = 0;
  m_IsImagingThreadRunning = false;
  delete m_ImagingGeometry;

  if (m_HistogrammingThread != 0) m_HistogrammingThread->Kill();
  m_HistogrammingThread = 0;
  m_IsHistogrammingThreadRunning = false;

  if (m_IdentificationThread != 0) m_IdentificationThread->Kill();
  m_IdentificationThread = 0;
  m_IsIdentificationThreadRunning = false;
  delete m_IdentificationGeometry;

  if (m_CleanUpThread != 0) m_CleanUpThread->Kill();
  m_CleanUpThread = 0;
  m_IsCleanUpThreadRunning = false;

  m_IsAnalysisRunning = false;
    
  cout<<"All threads stopped safely!"<<endl;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MRealTimeAnalyzer::Connect()
{
  //! Connect to the other machine

  m_DoDisconnect = false;
  if (m_IsAnalysisRunning == false) {
    StartAnalysis();
  }

  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MRealTimeAnalyzer::Disconnect()
{
  //! Disconnect from the other machine

  m_DoDisconnect = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MRealTimeAnalyzer::Reset()
{
  //! Reset the analys
  
  m_DoDisconnect = false;

  StopAnalysis();

  for (list<MRealTimeEvent*>::iterator I = m_Events.begin(); I != m_Events.end(); ++I) {
    delete (*I);
  }
  m_Events.clear();
  
  StartAnalysis();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneTransmissionLoop()
{
  //! The transmission loop

  // Load geometry:
  m_TransmissionGeometry = new MGeometryRevan();

  if (m_TransmissionGeometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_TransmissionGeometry->GetName()<<" failed!!"<<endl;
    return;
  }  

    
  MTransceiverTcpIp Transceiver("Realta", m_Settings->GetHostName(), m_Settings->GetPort(), m_Settings->GetTransceiverMode());
  Transceiver.SetVerbosity(2);
  Transceiver.RequestClient(true);
  
  // The geometry initialization is not reentrant, this we say it's running only here
  cout<<"Reading and initialization thread started..."<<endl<<flush;
  m_IsTransmissionThreadRunning = true;
  
  Transceiver.Connect();
  do {
    gSystem->Sleep(250);
    cout<<"Waiting for connection to remote host..."<<endl;
  } while (Transceiver.IsConnected() == false && m_StopThreads == false);

  if (Transceiver.IsConnected() == true) {
    m_IsConnected = true;
  }
  
  bool SaveEvents = false;
  ofstream fout;
  if (m_Settings->GetAccumulationFileName().IsEmpty() == false) {
    MString FileName = m_Settings->GetAccumulationFileName();
    MFile::ExpandFileName(FileName);
    if (m_Settings->GetAccumulationFileNameAddDateAndTime() == true) {
      size_t Pos = FileName.Last('.');
      if (Pos != MString::npos) {
        MString Suffix = FileName.GetSubString(Pos, FileName.Length() - Pos);
        FileName = FileName.GetSubString(0, Pos+1);
        MTime Now;
        FileName += Now.GetShortString();
        FileName += Suffix; 
      } else {
        MTime Now;
        FileName += ".";
        FileName += Now.GetShortString();
      }
    }
    fout.open(FileName);
    if (fout.is_open() == true) SaveEvents = true;
  }
  
  list<MRealTimeEvent*>::reverse_iterator InitIter;
  double InitializationCutOff = 1.0; // sec
  
  MTimer NapTimer;
  double NapAmount = 1000;
  double NapTime = 0;
  
  MTimer UpdateTimer;

  bool NoisingDefined = false;
  MERNoising* Noising = 0;
  
  // The maximum number of allowed time jumps before we reset
  int MaxNContinuousTimeJumps = 100;
  int NContinuousTimeJumps = 0;
  
  bool RestartInitilization = false; 
  
  // The infinite loop
  while (true) {
   
    // Check if we want a disconnect
    if (Transceiver.IsConnected() == true && m_DoDisconnect == true) {
      Transceiver.Disconnect();
      while (Transceiver.IsConnected() == true && m_StopThreads == false) {
        //cout<<"Waiting for DIS-connection to remote host..."<<endl;
        gSystem->Sleep(int(NapAmount));
        NapTime += NapAmount/1000.0;        
      }
      m_IsConnected = false;
      //cout<<"Disconnected!"<<endl;
      if (m_StopThreads == true) break;
    }
    
    // First make sure we are connected
    while (Transceiver.IsConnected() == false && m_DoDisconnect == false) {
      Transceiver.Connect();
      //cout<<"Waiting for connection to remote host..."<<endl;

      m_IsConnected = false;
      gSystem->Sleep(int(NapAmount));
      NapTime += NapAmount/1000.0;
      
      if (NapTimer.GetElapsed() > 5.0) {
        m_TransmissionThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_TransmissionThreadCpuUsage < 0.0) m_TransmissionThreadCpuUsage = 0.0;
        if (m_TransmissionThreadCpuUsage > 1.0) m_TransmissionThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
      if (m_StopThreads == true) break;    
    }
    if (Transceiver.IsConnected() == true) {
      m_IsConnected = true;
    }
    if (m_StopThreads == true) break;
    
    // Get the next event
    MString Message;
    if (Transceiver.Receive(Message) == false) {
      gSystem->Sleep(int(NapAmount));
      NapTime += NapAmount/1000.0;
    } else {
      // Begin: we have events
      
      // Split message into lines
      vector<MString> Tokens = Message.Tokenize("\n");
      // cout<<"Message size: "<<Message.Length()<<endl;
      //if (Tokens.size() > 0) cout<<"Last message: "<<Tokens.back()<<endl;

      // Save the events directly to file -- remove all EN's
      if (SaveEvents == true) {
        for (unsigned int i = 0; i < Tokens.size(); ++i) {
          if (Tokens[i] != "EN") {
            fout<<Tokens[i]<<endl;
          }
        }
      }

      // Before we parse anything we check on the noising status:
      if (NoisingDefined == false) { // 0 == Unknown
        for (unsigned int i = 0; i < Tokens.size(); ++i) {
          if (Tokens[i].BeginsWith("HT") == true) {
            if (Tokens[i].BeginsWith("HTsim") == true) {
              Noising = new MERNoising();
              Noising->SetGeometry(m_TransmissionGeometry);
              Noising->PreAnalysis();
              cout<<"Activate noising of input events"<<endl;
            } else {
              cout<<"NO noising of input events"<<endl;
            }
            NoisingDefined = true;
            break;
          }
        }
      }
     
      // One string can contain multiple events, thus split
      unsigned int StartIndex = 0;
      for (unsigned int i = 0; i < Tokens.size(); ++i) {
        if (Tokens[i] == "SE") {
          StartIndex = i+1;
          break;
        }
      }
      while (StartIndex < Tokens.size()) {
        MRERawEvent* RE = new MRERawEvent(m_TransmissionGeometry); 
        for (unsigned int i = StartIndex; i < Tokens.size(); ++i) {
          if (Tokens[i] == "SE" || Tokens[i] == "EN" ) {
            StartIndex = i+1;
            break;
          } else {
            RE->ParseLine(Tokens[i], 25);
            StartIndex = i+1;
          }
        }

        if (Noising != 0) {
          Noising->Analyze(RE);
          if (RE->GetNRESEs() == 0) {
            delete RE;
            continue;
          }
        }
        
        // If we have events, check the time timing 
        if (m_Events.empty() == false && (
              RE->GetEventTime() > m_Events.front()->GetTime() + 5*InitializationCutOff ||
              RE->GetEventTime() < (*InitIter)->GetTime())
            ) {
          cout<<"Time jump for event "<<RE->GetEventID()<<": T="<<RE->GetEventTime()<<" vs. T="<<m_Events.front()->GetTime()<<" in the front of our list and T="<<(*InitIter)->GetTime()<<" of last initialized event"<<endl;
          ++NContinuousTimeJumps;
          if (NContinuousTimeJumps < MaxNContinuousTimeJumps) {
            cout<<"Only "<<NContinuousTimeJumps<<" continuous time jumps: Ignoring event!"<<endl;
            delete RE;
            continue;
          } else {
            RestartInitilization = true; 
          }
        } else {
          NContinuousTimeJumps = 0; 
        }

        //cout<<"Received event "<<RE->GetEventID()<<" with obs time "<<RE->GetEventTime()<<" seconds."<<endl;
      
        MRealTimeEvent* Event = new MRealTimeEvent();
        Event->SetID(RE->GetEventID());
        Event->SetTime(RE->GetEventTime());
        Event->SetInitialRawEvent(RE);
  
        // Search for the correct position to add it:
        if (m_Events.empty() == true) {
          m_Events.push_front(Event);
          InitIter = m_Events.rbegin();
        } else {
          // Sometimes we might have to restart the initialization:
          if (RestartInitilization == false) {
          
            bool AtInitilizationLimit = false;
            list<MRealTimeEvent*>::iterator EventIter = m_Events.begin();
            while (EventIter != m_Events.end() && (*EventIter)->GetTime() > Event->GetTime()) {
              //cout<<"Checking: "<<(*EventIter)->GetTime()<<" vs. "<<Event->GetTime()<<endl;
              if ((*EventIter)->IsInitialized() == true) {
                //cout<<(*EventIter)->GetTime()<<" already init"<<endl;
                AtInitilizationLimit = true;
                break;
              }
              EventIter++;       
            }
            if (AtInitilizationLimit == true) {
              cout<<"Error: Event "<<Event->GetID()<<" has been dropped because it arrived out of sync and too late!"<<endl;
              cout<<"       Youngest event time on file: "<<(*EventIter)->GetTime()<<" sec ("<<(*EventIter)->GetID()<<") vs. this events time : "<<Event->GetTime()<<" sec ("<<Event->GetID()<<")"<<endl;
              cout<<"ATTENTION: We should never ever reach this fall back code!"<<endl;
              delete Event;
              Event = 0;
            } else {
              if (EventIter == m_Events.end()) {
                m_Events.push_back(Event);
              } else {
                // insert before!
                m_Events.insert(EventIter, Event);
              }
            }
          }
          // Restart initialization
          else {
            cout<<"Restarting event initialization!"<<endl;
            // Add the event
            m_Events.push_front(Event);
            // and initialize all up to this event:
            while ((*InitIter) != m_Events.front()) {
              (*InitIter)->IsInitialized(true);
              InitIter++;
            }
             
            RestartInitilization = false;
          }
      
          if (Event != 0) { // we deleted it previously...
            m_TransmissionThreadLastEventID = Event->GetID();
          }
      
          // Finally we announce that the event can be used:
          MTime Front = m_Events.front()->GetTime();
          //cout<<"Front ID: "<<m_Events.front()->GetID()<<" is init: "<<((m_Events.front()->IsInitialized() == true) ? "true" : "false")<<":"<<(*InitIter)->GetID()<<endl;
          while ((Front - (*InitIter)->GetTime()).GetAsSeconds() > InitializationCutOff) {
            (*InitIter)->IsInitialized(true);
            if ((*InitIter) == m_Events.front()) break; // Make sure we never go beyond the first event!
            InitIter++;
          }
        } // event list not empty        
      } // current event
    } // end we have events!

    
    if (NapTimer.GetElapsed() > 5.0) {
      m_TransmissionThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_TransmissionThreadCpuUsage < 0.0) m_TransmissionThreadCpuUsage = 0.0;
      if (m_TransmissionThreadCpuUsage > 1.0) m_TransmissionThreadCpuUsage = 1.0;
      NapTimer.Reset();
      NapTime = 0.0;
    }
    if (m_StopThreads == true) break;
  }
  
  Transceiver.Disconnect(true, 120);
  
  // Save the events directly to file:
  if (SaveEvents == true) {
    fout<<"EN"<<endl;
    fout.close();  
  }

  m_IsConnected = false;
  
  m_IsTransmissionThreadRunning = false;
  
  cout<<"Transmission thread finished!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneCoincidenceLoop()
{
  // Do the coincidence search...

  bool DoCoincidence = m_Settings->GetDoCoincidence();
  
  // Load geometry:
  m_CoincidenceGeometry = new MGeometryRevan();

  if (m_CoincidenceGeometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_CoincidenceGeometry->GetName()<<" failed!!"<<endl;
    return;
  }  

  // Initialize the raw event analyzer for HEMI:
  MRawEventAnalyzer* RawEventAnalyzer = new MRawEventAnalyzer();
  RawEventAnalyzer->SetGeometry(m_CoincidenceGeometry);
  RawEventAnalyzer->SetSettings(m_Settings);
  
  RawEventAnalyzer->SetClusteringAlgorithm(MRawEventAnalyzer::c_ClusteringAlgoNone);
  RawEventAnalyzer->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);
  RawEventAnalyzer->SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
  RawEventAnalyzer->SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);

  
  if (RawEventAnalyzer->PreAnalysis() == false) {
    cout<<"Preanalysis failed!"<<endl;  
    return;
  }
      
  // The geometry initialization is not reentrant, this we say it's running only here
  cout<<"Coincidence thread started..."<<endl<<flush;
  m_IsCoincidenceThreadRunning = true;

  
  list<MRealTimeEvent*>::reverse_iterator EventIter;
  while (m_Events.empty() && m_StopThreads == false) {
    gSystem->Sleep(10);
  }
  EventIter = m_Events.rbegin();
  
  MTimer NapTimer;
  double NapAmount = 100;
  double NapTime = 0;
  
  MRealTimeEvent* Event = 0;
  while (m_StopThreads == false) {
    Event = (*EventIter);
     
    while (Event->IsInitialized() == false && m_StopThreads == false) {
      //cout<<"Reconstruction thread: Waiting for initialization...."<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_CoincidenceThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_CoincidenceThreadCpuUsage < 0.0) m_CoincidenceThreadCpuUsage = 0.0;
        if (m_CoincidenceThreadCpuUsage > 1.0) m_CoincidenceThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (DoCoincidence == false) {
      Event->SetCoincidentRawEvent(new MRERawEvent(Event->GetInitialRawEvent()));  
      Event->IsCoincident(true);
      m_CoincidenceThreadLastEventID = Event->GetID();
    }
    
    if (DoCoincidence == true && Event->IsCoincident() == false) {
      MRERawEvent* RawEvent = new MRERawEvent(Event->GetInitialRawEvent());
      // --> it will be deleted by the RawEventAnalyzer!!
      
      // Reconstruct
      RawEventAnalyzer->AddRawEvent(RawEvent);
      unsigned int ReturnCode = MRawEventAnalyzer::c_AnalysisSucess;
      
      while (ReturnCode == MRawEventAnalyzer::c_AnalysisSucess) {
        ReturnCode = RawEventAnalyzer->AnalyzeEvent();
    
        if (ReturnCode == MRawEventAnalyzer::c_AnalysisSucess) {
       
          MRERawEvent* BestRawEvent = 0;
          if (RawEventAnalyzer->GetOptimumEvent() != 0) {
            BestRawEvent = RawEventAnalyzer->GetOptimumEvent();
          } else if (RawEventAnalyzer->GetBestTryEvent() != 0) {
            BestRawEvent = RawEventAnalyzer->GetBestTryEvent();
          }
          if (BestRawEvent != 0) {
            // Find the event to which it belongs:
            list<MRealTimeEvent*>::reverse_iterator SearchIter = EventIter;
            bool Found = false;
            do {
              //cout<<"S: "<<(*SearchIter)->GetID()<<" - B: "<<BestRawEvent->GetEventID()<<endl;
              if ((*SearchIter)->IsCoincident() == true) {
                //cout<<"Coincidence flag found: "<<(*SearchIter)->GetID()<<endl;
                if (Found == false) {
                  cout<<"Realta's coincidence search (ID: "<<Event->GetID()<<"): Why do we have coincident event without finding the corresponding event??"<<endl; 
                }
                break;
              }
              if ((*SearchIter)->GetID() == BestRawEvent->GetEventID()) {
                //cout<<"Found: "<<(*SearchIter)->GetID()<<endl;
                Found = true;
                (*SearchIter)->SetCoincidentRawEvent(new MRERawEvent(BestRawEvent));
                (*SearchIter)->IsCoincident(true);
              } else if (Found == true) {
                //cout<<"Also coincident & merged: "<<(*SearchIter)->GetID()<<endl;
                (*SearchIter)->IsCoincident(true);
                (*SearchIter)->IsMerged(true);
                (*SearchIter)->IsDropped(true);
              }
              
              if (SearchIter == m_Events.rbegin()) {
                if (Found == false) {
                  cout<<"Realta's coincidence search (ID: "<<Event->GetID()<<"): Why didn't we find the original event this coincident event belongs to??"<<endl;   
                }
                break;
              }
              SearchIter--;
            }  
            while (true);
            
          }
        } // while (ReturnCode == MRawEventAnalyzer::c_AnalysisSucess) {
          
        if (ReturnCode == MRawEventAnalyzer::c_AnalysisUndefinedError) {
          cout<<"Realta's coincidence search (ID: "<<Event->GetID()<<"): An undefined error occurred?"<<endl;  
        }
          
      } // IsCoincident() == false
      
      m_CoincidenceThreadLastEventID = Event->GetID();
    }
        
    while (Event == m_Events.front() && m_StopThreads == false) {
      //cout<<"Coincidence thread: Waiting for new events...."<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_CoincidenceThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_CoincidenceThreadCpuUsage < 0.0) m_CoincidenceThreadCpuUsage = 0.0;
        if (m_CoincidenceThreadCpuUsage > 1.0) m_CoincidenceThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (NapTimer.GetElapsed() > 5.0) {
      m_CoincidenceThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_CoincidenceThreadCpuUsage < 0.0) m_CoincidenceThreadCpuUsage = 0.0;
      if (m_CoincidenceThreadCpuUsage > 1.0) m_CoincidenceThreadCpuUsage = 1.0;
      NapTimer.Reset();
      NapTime = 0.0;
    }

    EventIter++;
  }
  
   
  m_IsCoincidenceThreadRunning = false;
    
  cout<<"Coincidence thread finished!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneReconstructionLoop()
{
  // Do the event reconstruction...


  // Load geometry:
  m_ReconstructionGeometry = new MGeometryRevan();

  if (m_ReconstructionGeometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_ReconstructionGeometry->GetName()<<" failed!!"<<endl;
    return;
  }  

  MFile File;
  bool SaveEvents = false;
  if (m_Settings->GetAccumulationFileName().IsEmpty() == false) {
    MString FileName = m_Settings->GetAccumulationFileName();
    MFile::ExpandFileName(FileName);
    FileName += ".tra";
    if (m_Settings->GetAccumulationFileNameAddDateAndTime() == true) {
      size_t Pos = FileName.Last('.');
      if (Pos != MString::npos) {
        MString Suffix = FileName.GetSubString(Pos, FileName.Length() - Pos);
        FileName = FileName.GetSubString(0, Pos+1);
        MTime Now;
        FileName += Now.GetShortString();
        FileName += Suffix; 
      } else {
        MTime Now;
        FileName += ".";
        FileName += Now.GetShortString();
      }
    }
    File.Open(FileName, MFile::c_Write);
    if (File.IsOpen() == true) SaveEvents = true;
  }


  // Initialize the raw event analyzer for HEMI:
  MRawEventAnalyzer* RawEventAnalyzer = new MRawEventAnalyzer();
  RawEventAnalyzer->SetGeometry(m_ReconstructionGeometry);
  RawEventAnalyzer->SetSettings(m_Settings);
  
  RawEventAnalyzer->SetClusteringAlgorithm(MRawEventAnalyzer::c_ClusteringAlgoNone);
  
  if (RawEventAnalyzer->PreAnalysis() == false) {
    cout<<"Preanalysis failed!"<<endl;  
    return;
  }
      
  // The geometry initialization is not reentrant, this we say it's running only here
  cout<<"Reconstruction thread started..."<<endl<<flush;
  m_IsReconstructionThreadRunning = true;

  
  list<MRealTimeEvent*>::reverse_iterator EventIter;
  while (m_Events.empty() && m_StopThreads == false) {
    gSystem->Sleep(10);
  }
  EventIter = m_Events.rbegin();
  
  MTimer NapTimer;
  double NapAmount = 100;
  double NapTime = 0;

  double EventSkip = 0.0;
  
  MRealTimeEvent* Event = 0;
  while (m_StopThreads == false) {
    Event = (*EventIter);
 
    while (Event->IsCoincident() == false && m_StopThreads == false) {
      //cout<<"Reconstruction thread: Waiting for coincident event at position "<<Event->GetID()<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_ReconstructionThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_ReconstructionThreadCpuUsage < 0.0) m_ReconstructionThreadCpuUsage = 0.0;
        if (m_ReconstructionThreadCpuUsage > 1.0) m_ReconstructionThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (EventSkip > 0) {
      if (gRandom->Rndm() < EventSkip) {
        Event->IsDropped(true);
      }
    }
  
    if (Event->IsReconstructed() == false && Event->IsDropped() == false) {
      MRERawEvent* RawEvent = new MRERawEvent(Event->GetCoincidentRawEvent());
      // --> it is deleted by the RawEventAnalyzer
      
      // Reconstruct
      RawEventAnalyzer->AddRawEvent(RawEvent);
      unsigned int ReturnCode = RawEventAnalyzer->AnalyzeEvent();
    
      if (ReturnCode == MRawEventAnalyzer::c_AnalysisSucess) {
       
        MRERawEvent* BestRawEvent = 0;
        if (RawEventAnalyzer->GetOptimumEvent() != 0) {
          BestRawEvent = RawEventAnalyzer->GetOptimumEvent();
        } else if (RawEventAnalyzer->GetBestTryEvent() != 0) {
          BestRawEvent = RawEventAnalyzer->GetBestTryEvent();
        }
        if (BestRawEvent != 0) {
          MPhysicalEvent* P = BestRawEvent->GetPhysicalEvent();
          //cout<<"Physical event type: "<<P->GetType()<<endl;

          if (dynamic_cast<MComptonEvent*>(P) != 0) {
            MComptonEvent* E = new MComptonEvent();
            E->Assimilate(dynamic_cast<MComptonEvent*>(P));
            Event->SetPhysicalEvent(E);
          } else if (dynamic_cast<MPhotoEvent*>(P) != 0) {
            MPhotoEvent* E = new MPhotoEvent();
            E->Assimilate(dynamic_cast<MPhotoEvent*>(P));
            Event->SetPhysicalEvent(E);
          } else if (dynamic_cast<MPairEvent*>(P) != 0) {
            MPairEvent* E = new MPairEvent();
            E->Assimilate(dynamic_cast<MPairEvent*>(P));
            Event->SetPhysicalEvent(E);
          } else if (dynamic_cast<MUnidentifiableEvent*>(P) != 0) {
            MUnidentifiableEvent* E = new MUnidentifiableEvent();
            E->Assimilate(dynamic_cast<MUnidentifiableEvent*>(P));
            Event->SetPhysicalEvent(E);
          } else if (dynamic_cast<MMuonEvent*>(P) != 0) {
            MMuonEvent* E = new MMuonEvent();
            E->Assimilate(dynamic_cast<MMuonEvent*>(P));
            Event->SetPhysicalEvent(E);
          } else {
            cout<<"Error: Unknown event type!"<<endl; 
          }
        }
      }
      
      m_ReconstructionThreadLastEventID = Event->GetID();
      
      Event->IsReconstructed(true);
    }
    if (Event->IsDropped() == true || Event->IsMerged() == true) {
      if (SaveEvents == true) {
        cout<<"Saving..."<<endl;
        if (Event->GetPhysicalEvent() != 0) {
          Event->GetPhysicalEvent()->Stream(File, 1, false);
          File.Flush();
        }
      }
      Event->IsReconstructed(true);      
    }
    
    while (Event == m_Events.front() && m_StopThreads == false) {
      //cout<<"Reconstruction thread: Waiting for new events...."<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 2.0) {
        m_ReconstructionThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_ReconstructionThreadCpuUsage < 0.0) m_ReconstructionThreadCpuUsage = 0.0;
        if (m_ReconstructionThreadCpuUsage > 1.0) m_ReconstructionThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (NapTimer.GetElapsed() > 2.0) {
      m_ReconstructionThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_ReconstructionThreadCpuUsage < 0.0) m_ReconstructionThreadCpuUsage = 0.0;
      if (m_ReconstructionThreadCpuUsage > 1.0) m_ReconstructionThreadCpuUsage = 1.0;
      if (m_ReconstructionThreadCpuUsage == 1.0) {
        EventSkip += 0.1;
        cout<<"New Reconstruction event skip: "<<EventSkip<<endl;
      }
      if (m_ReconstructionThreadCpuUsage < 0.9 && EventSkip != 0.0) {
        EventSkip -= 0.1;
        if (EventSkip < 0) EventSkip = 0.0;
        cout<<"New Reconstruction event skip: "<<EventSkip<<endl;
      }
      NapTimer.Reset();
      NapTime = 0.0;
    }

    EventIter++;
  }
  
  if (SaveEvents == true) {
    File.Close();
  }
  
  delete RawEventAnalyzer;
  
  m_IsReconstructionThreadRunning = false;
    
  cout<<"Reconstruction thread finished!"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


MImagerExternallyManaged* MRealTimeAnalyzer::InitializeImager()
{
  
  // Load geometry:
  m_ImagingGeometry = new MDGeometryQuest();

  if (m_ImagingGeometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_ImagingGeometry->GetName()<<" failed!!"<<endl;
    return 0;
  }  

  MImagerExternallyManaged* Imager = 
    new MImagerExternallyManaged(m_Settings->GetCoordinateSystem());
  Imager->SetGeometry(m_ImagingGeometry);
  Imager->UseGUI(false);  
    
  // Maths:
  Imager->SetApproximatedMaths(m_Settings->GetApproximatedMaths());
    
  // Set the dimensions of the image
  if (m_Settings->GetCoordinateSystem() == MProjection::c_Spheric) {
    Imager->SetViewport(m_Settings->GetPhiMin()*c_Rad, 
                          m_Settings->GetPhiMax()*c_Rad, 
                          m_Settings->GetBinsPhi(),
                          m_Settings->GetThetaMin()*c_Rad,
                          m_Settings->GetThetaMax()*c_Rad,
                          m_Settings->GetBinsTheta(),
                          c_FarAway/10, 
                          c_FarAway, 
                          1, 
                          m_Settings->GetImageRotationXAxis(), 
                          m_Settings->GetImageRotationZAxis());
  } else if (m_Settings->GetCoordinateSystem() == MProjection::c_Galactic) {
    Imager->SetViewport(m_Settings->GetGalLongitudeMin()*c_Rad, 
                          m_Settings->GetGalLongitudeMax()*c_Rad, 
                          m_Settings->GetBinsGalLongitude(),
                          (m_Settings->GetGalLatitudeMin()+90)*c_Rad,
                          (m_Settings->GetGalLatitudeMax()+90)*c_Rad,
                          m_Settings->GetBinsGalLatitude(),
                          c_FarAway/10, 
                          c_FarAway, 
                          1);
  } else if (m_Settings->GetCoordinateSystem() == MProjection::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MProjection::c_Cartesian3D){
    Imager->SetViewport(m_Settings->GetXMin(), 
                          m_Settings->GetXMax(), 
                          m_Settings->GetBinsX(),
                          m_Settings->GetYMin(), 
                          m_Settings->GetYMax(), 
                          m_Settings->GetBinsY(),
                          m_Settings->GetZMin(), 
                          m_Settings->GetZMax(), 
                          m_Settings->GetBinsZ());
  } else {
    merr<<"Unknown coordinate system ID: "<<m_Settings->GetCoordinateSystem()<<fatal;
  }
    
  // Set the draw modes
  Imager->SetDrawMode(m_Settings->GetImageDrawMode());
  Imager->SetPalette(m_Settings->GetImagePalette());
  Imager->SetSourceCatalog(m_Settings->GetImageSourceCatalog());
 
  Imager->SetAnimationMode(MImager::c_AnimateNothing); 

    
  // Set the response type:
  if (m_Settings->GetResponseType() == 0) {
    Imager->SetResponseGaussian(m_Settings->GetFitParameterComptonTransSphere(), 
                                m_Settings->GetFitParameterComptonLongSphere(),
                                m_Settings->GetFitParameterPair(),
                                m_Settings->GetGauss1DCutOff(),
                                m_Settings->GetUseAbsorptions());
  } else if (m_Settings->GetResponseType() == 1) {
    Imager->SetResponseGaussianByUncertainties();
  } else if (m_Settings->GetResponseType() == 2) {
    Imager->SetResponseEnergyLeakage(m_Settings->GetFitParameterComptonTransSphere(), 
                                       m_Settings->GetFitParameterComptonLongSphere());
     
  } else if (m_Settings->GetResponseType() == 3) {
    if (Imager->SetResponsePRM(m_Settings->GetImagingResponseComptonTransversalFileName(),
                                 m_Settings->GetImagingResponseComptonLongitudinalFileName(),
                                 m_Settings->GetImagingResponsePairRadialFileName()) == false) {
      mgui<<"Cannot set PRM response! Aborting imaging!"<<error;
      delete Imager;
      return 0;
    }
  } else {
    merr<<"Unknown response type: "<<m_Settings->GetResponseType()<<show;
    delete Imager;
    return 0;
  }

  // A new event selector:
  MEventSelector S;
  S.SetSettings(m_Settings);
  Imager->SetEventSelector(S);

  // Memory management... needs clean up...
  Imager->SetMemoryManagment(m_Settings->GetRAM(),
                             m_Settings->GetSwap(),
                             m_Settings->GetMemoryExhausted(),
                             m_Settings->GetBytes());

  if (m_Settings->GetLHAlgorithm() == MLMLAlgorithms::c_ClassicEM) {
    Imager->SetDeconvolutionAlgorithmClassicEM();
  } else if (m_Settings->GetLHAlgorithm() == MLMLAlgorithms::c_OSEM) {
    Imager->SetDeconvolutionAlgorithmOSEM(m_Settings->GetOSEMSubSets());
  } else {
    merr<<"Unknown deconvolution algorithm. Using classic EM."<<error;
    Imager->SetDeconvolutionAlgorithmClassicEM();
  }

  if (m_Settings->GetLHStopCriteria() == 0) {
    Imager->SetStopCriterionByIterations(m_Settings->GetNIterations());
  } else {
    merr<<"Unknown stop criterion. Stopping after 0 iterations."<<error;
    Imager->SetStopCriterionByIterations(0);
  }
  
  Imager->Initialize();
  
  return Imager;
}
  

////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneImagingLoop()
{
  // Do the event reconstruction...
  

  MImagerExternallyManaged* Imager = InitializeImager();
    
  // The geometry initialization is not reentrant, this we say it's running only here
  cout<<"Imaging thread started..."<<endl<<flush;
  m_IsImagingThreadRunning = true;

  // Wait until we have an event
  list<MRealTimeEvent*>::reverse_iterator EventIter;
  while (m_Events.empty() && m_StopThreads == false) {
    gSystem->Sleep(10);
  }
  EventIter = m_Events.rbegin();
 
    
  MTimer NapTimer;
  double NapAmount = 100.0;
  double NapTime = 0.0;
  
  double EventSkip = 0.0;

  MRealTimeEvent* Event = 0;
  while (m_StopThreads == false) {
    Event = (*EventIter);
 
    // Wait until the event is reconstructed...
    while (Event->IsReconstructed() == false && m_StopThreads == false) {
      //cout<<"Imaging thread: Waiting for reconstruction of event "<<Event->GetID()<<"...."<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_ImagingThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_ImagingThreadCpuUsage < 0.0) m_ImagingThreadCpuUsage = 0.0;
        if (m_ImagingThreadCpuUsage > 1.0) m_ImagingThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (EventSkip > 0) {
      if (gRandom->Rndm() < EventSkip) {
        Event->IsDropped(true);
      }
    }

    // Do the imaging if we have a good reconstructed event
    if (Event->IsImaged() == false && Event->IsDropped() == false && Event->GetPhysicalEvent() != 0) {
      //cout<<"Imaging; "<<Event->GetID()<<endl;
      //cout<<Event->GetPhysicalEvent()->ToString()<<endl;
      
      MBPData* Data = Imager->CalculateResponseSlice(Event->GetPhysicalEvent());
      if (Data != 0) {
        Event->SetBackprojection(Data);
      }
      
      m_ImagingThreadLastEventID = Event->GetID();
      
      Event->IsImaged(true);
    }
    if (Event->IsDropped() == true) {
      Event->IsReconstructed(true);      
    }

    // Wait until we have another event
    while (Event == m_Events.front() && m_StopThreads == false) {
      //cout<<"Imaging thread: Waiting for new events...."<<endl;
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 2.0) {
        m_ImagingThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_ImagingThreadCpuUsage < 0.0) m_ImagingThreadCpuUsage = 0.0;
        if (m_ImagingThreadCpuUsage > 1.0) m_ImagingThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
    }
    if (m_StopThreads == true) break;

    if (NapTimer.GetElapsed() > 2.0) {
      m_ImagingThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_ImagingThreadCpuUsage < 0.0) m_ImagingThreadCpuUsage = 0.0;
      if (m_ImagingThreadCpuUsage > 1.0) m_ImagingThreadCpuUsage = 1.0;
      if (m_ImagingThreadCpuUsage == 1.0) {
        EventSkip += 0.1;
        cout<<"New imaging event skip: "<<EventSkip<<endl;
      }
      if (m_ImagingThreadCpuUsage < 0.9 && EventSkip != 0.0) {
        EventSkip -= 0.1;
        if (EventSkip < 0) EventSkip = 0.0;
        cout<<"New imaging event skip: "<<EventSkip<<endl;
      }
      NapTimer.Reset();
      NapTime = 0.0;
    }

    EventIter++;
  }

  delete Imager;
  
  m_IsImagingThreadRunning = false;
 
  cout<<"Imaging thread finished"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneHistogrammingLoop()
{
  m_AccumulationTimeMutex.Lock();
  TH1D* CountRate = new TH1D("CountRates", "", m_Settings->GetBinsCountRate(), 0, m_AccumulationTime);
  m_AccumulationTimeMutex.UnLock();
  CountRate->SetXTitle("Passed Time in seconds");
  CountRate->SetYTitle("cts/sec");
  CountRate->SetMinimum(0);
  CountRate->SetLabelSize(0.06f, "XY");
  CountRate->SetTitleSize(0.06f, "XY");
  CountRate->SetFillColor(kViolet+6);
  CountRate->SetLineColor(kViolet+10);

  // Make it atomic...
  // no delete for the time being...
  m_CountRate = CountRate;

  TH1D* InternalCountRate = new TH1D(*CountRate);

  double Emin = m_Settings->GetMinimumSpectrum();
  double Emax = m_Settings->GetMaximumSpectrum();

  TH1D* Spectrum = new TH1D("Spectrum", "Spectrum", m_Settings->GetBinsSpectrum(), 
                            m_Settings->GetMinimumSpectrum(), m_Settings->GetMaximumSpectrum());
  Spectrum->SetXTitle("Energy in keV");
  Spectrum->SetYTitle("cts/sec/keV");
  Spectrum->SetMinimum(0);
  Spectrum->SetFillColor(kViolet+6);
  Spectrum->SetLineColor(kViolet+10);

  // Set it in one step...
  // no delete for the time being...
  m_Spectrum = Spectrum;

  m_SpectrumMin.clear();
  m_SpectrumMin.push_back(m_Settings->GetFirstEnergyRangeMin());
  m_SpectrumMin.push_back(m_Settings->GetSecondEnergyRangeMin());
  m_SpectrumMin.push_back(m_Settings->GetThirdEnergyRangeMin());
  m_SpectrumMin.push_back(m_Settings->GetFourthEnergyRangeMin());
  
  m_SpectrumMax.clear();
  m_SpectrumMax.push_back(m_Settings->GetFirstEnergyRangeMax());
  m_SpectrumMax.push_back(m_Settings->GetSecondEnergyRangeMax());
  m_SpectrumMax.push_back(m_Settings->GetThirdEnergyRangeMax());
  m_SpectrumMax.push_back(m_Settings->GetFourthEnergyRangeMax());
  
  TH1D* InternalSpectrum = new TH1D(*Spectrum);

  MImagerExternallyManaged* Imager = InitializeImager();

  MEventSelector Selector;
  Selector.SetSettings(m_Settings);

  cout<<"Histogramming thread started..."<<endl<<flush;
  m_IsHistogrammingThreadRunning = true;

  // Wait until we have an event
  while (m_StopThreads == false && m_Events.size() < 2) {
    gSystem->Sleep(10);
  }
  // Wait until we have a first reconstructed event
  // The event horizon is the newest reconstructed event before the cut-off time
  list<MRealTimeEvent*>::reverse_iterator EventHorizon = m_Events.rbegin();
  while (m_StopThreads == false && (*EventHorizon)->IsReconstructed() == false)  {
    gSystem->Sleep(10);
  }

  double EventHorizonTime = 0.0;
  double LatestTime = 0.0;

  MTimer NapTimer;
  double NapAmount = 100.0;
  double NapTime = 0.0;

  MTimer UpdateTimer;
  while (m_StopThreads == false) {

    double AccumulationTime = m_AccumulationTime;

    // We always sleep 2 seconds:
    UpdateTimer.Reset();
    while (UpdateTimer.GetElapsed() < 2.0 && m_StopThreads == false) {
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_HistogrammingThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_HistogrammingThreadCpuUsage < 0.0) m_HistogrammingThreadCpuUsage = 0.0;
        if (m_HistogrammingThreadCpuUsage > 1.0) m_HistogrammingThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
      //cout<<"Nap timer: "<<UpdateTimer.GetElapsed()<<endl;
    }
    UpdateTimer.Reset();
    if (m_StopThreads == true) break;

    //cout<<"Passed nap timer..."<<endl;

    if (m_Events.empty() == true) {
      cout<<"No events to show..."<<endl;
      continue;
    }

    //cout<<"Histogramming..."<<endl;

    //cout<<"Going to event horizon from "<<(*EventHorizon)->GetID()<<", t="<<(*EventHorizon)->GetTime()<<endl;
    do {
      list<MRealTimeEvent*>::reverse_iterator Next = EventHorizon;
      ++Next;
      if (Next == m_Events.rend()) break;
      if ((*Next)->IsReconstructed() == false) break;
      ++EventHorizon;
    } while (true);
    //cout<<"New event horizon: ID="<<(*EventHorizon)->GetID()<<", t="<<(*EventHorizon)->GetTime()<<endl;

    
    // Check if we really have more new data:
    if (m_HistogrammingThreadLastEventID == (*EventHorizon)->GetID() && EventHorizon != m_Events.rbegin()) {
      //cout<<"Continue: No new data"<<endl;
      continue;
    }


    InternalCountRate->Reset();
    InternalCountRate->GetXaxis()->Set(InternalCountRate->GetXaxis()->GetNbins(), 0, AccumulationTime);
    InternalSpectrum->Reset();


    vector<MBPData*> Backprojections;

    list<MRealTimeEvent*>::iterator Event = EventHorizon.base();
    EventHorizonTime = (*EventHorizon)->GetTime().GetAsDouble();

    int NEventsCountRate = 0;
    int NEventsSpectrum = 0;
    int NBackprojections = 0;
    while (Event != m_Events.end() && EventHorizonTime - (*Event)->GetTime().GetAsDouble() < AccumulationTime && EventHorizonTime - (*Event)->GetTime().GetAsDouble() >= 0) {
      if (m_StopThreads == true) break;
      LatestTime = (*Event)->GetTime().GetAsDouble();
      // The count rate histogram is always filled 
      InternalCountRate->Fill(EventHorizonTime - (*Event)->GetTime().GetAsDouble());
      ++NEventsCountRate;

      // The spectrum is filled as soon as we are within the energy window and 
      if ((*Event)->IsCoincident() == true && (*Event)->IsMerged() == false && (*Event)->GetCoincidentRawEvent() != 0) {
        if ((*Event)->GetCoincidentRawEvent()->GetEnergy() >= Emin && (*Event)->GetCoincidentRawEvent()->GetEnergy() <= Emax) {
          InternalSpectrum->Fill((*Event)->GetCoincidentRawEvent()->GetEnergy());
          ++NEventsSpectrum;
        }
      }

      // The image is only filled if *all* event selections are fullfilled
      if ((*Event)->GetPhysicalEvent() != 0 && Selector.IsQualifiedEvent((*Event)->GetPhysicalEvent(), false) == true) {
        // And fill...
        if ((*Event)->GetBackprojection() != 0) {
          Backprojections.push_back((*Event)->GetBackprojection());
          ++NBackprojections;
        }
      }
      Event++;
    }
    //cout<<"Events in histogramming: "<<NEvents<<endl;

    // Normalize the histograms
    for (int b = 1; b <= InternalCountRate->GetXaxis()->GetNbins(); ++b) {
      InternalCountRate->SetBinContent(b, InternalCountRate->GetBinContent(b)/InternalCountRate->GetBinWidth(b));
    }
    if (EventHorizonTime - LatestTime > 0) {
      for (int b = 1; b <= InternalSpectrum->GetXaxis()->GetNbins(); ++b) {
        //cout<<m_Spectrum->GetBinContent(b)<<":"<<m_Spectrum->GetBinWidth(b)<<":"<<(StartTime - EndTime)<<endl;
        InternalSpectrum->SetBinContent(b, InternalSpectrum->GetBinContent(b)/InternalSpectrum->GetBinWidth(b)/(EventHorizonTime - LatestTime));
      }
    } else {
      cout<<"Error: Start time - end time = "<<EventHorizonTime - LatestTime<<endl;
    }

    if (m_StopThreads == true) break;

    // The deconvolution part:
     vector<MImage*> Images = Imager->Deconvolve(Backprojections);


    // Now update everything:
    if (Event != m_Events.end()) {
      m_HistogrammingThreadFirstEventID = (*Event)->GetID();
    } else {
      m_HistogrammingThreadFirstEventID = m_Events.back()->GetID();
    }
    m_HistogrammingThreadLastEventID = (*EventHorizon)->GetID();

    m_CountRate->Reset();
    m_CountRate->GetXaxis()->Set(InternalCountRate->GetXaxis()->GetNbins(), 0, AccumulationTime);
    for (int b = 1; b <= InternalCountRate->GetXaxis()->GetNbins(); ++b) {
      m_CountRate->SetBinContent(b, InternalCountRate->GetBinContent(b));
    }

    m_Spectrum->Reset();
    for (int b = 1; b <= InternalSpectrum->GetXaxis()->GetNbins(); ++b) {
      m_Spectrum->SetBinContent(b, InternalSpectrum->GetBinContent(b));
    }

    // m_Image = 0; 
    // delete m_Image;
    //m_Image = dynamic_cast<MImageSpheric*>(Images.back());
    m_Image = shared_ptr<MImage>(Images.back());
    if (Images.size() > 1) {
      for (unsigned int i = 0; i < Images.size() - 1; ++i) {
        delete Images[i];
      }
    }

    ostringstream si;
    si.precision(2);
    si.setf(ios::fixed, ios::floatfield);
    si<<"Image from "<<LatestTime<<" sec to "<<EventHorizonTime<<" sec with "<<NBackprojections<<" events";
    m_Image->SetTitle(si.str().c_str());
    
    ostringstream ss;
    ss.precision(2);
    ss.setf(ios::fixed, ios::floatfield);
    ss<<"Spectrum from "<<LatestTime<<" sec to "<<EventHorizonTime<<" sec with "<<NEventsSpectrum<<" events";
    m_Spectrum->SetTitle(ss.str().c_str());
 
    /*
    m_Image->SetTitle("Backprojections");
    m_Spectrum->SetTitle("Spectrum");
    */    

    if (NapTimer.GetElapsed() > 5.0) {
      m_HistogrammingThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_HistogrammingThreadCpuUsage < 0.0) m_HistogrammingThreadCpuUsage = 0.0;
      if (m_HistogrammingThreadCpuUsage > 1.0) m_HistogrammingThreadCpuUsage = 1.0;
      NapTimer.Reset();
      NapTime = 0.0;
    }
    
    if (m_StopThreads == true) break;
  }

  delete Imager;
  
  m_IsHistogrammingThreadRunning = false;

  cout<<"Histogramming thread finished"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


vector<MQualifiedIsotope> MRealTimeAnalyzer::GetIsotopes() 
{
  //! Get a COPY of the isotope list

  vector<MQualifiedIsotope> Isotopes;

  m_IsotopeMutex.Lock();
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    Isotopes.push_back(m_Isotopes[i]);  
  }
  m_IsotopeMutex.UnLock();
  
  return Isotopes;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneIdentificationLoop()
{
  // First set up the geometry
  m_IdentificationGeometry = new MDGeometryQuest();
  if (m_IdentificationGeometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_IdentificationGeometry->GetName()<<" failed!!"<<endl;
    return;
  }

  bool DoIdentification = m_Settings->GetDoIdentification();

  // Calculate the energy bins:
  double Emin = m_Settings->GetFirstEnergyRangeMin();
  double Emax = m_Settings->GetFirstEnergyRangeMax();

  if (m_Settings->GetSecondEnergyRangeMin() != 0 || m_Settings->GetSecondEnergyRangeMax() != 0) {
    if (m_Settings->GetSecondEnergyRangeMin() < Emin) Emin = m_Settings->GetSecondEnergyRangeMin();
    if (m_Settings->GetSecondEnergyRangeMax() > Emax) Emax = m_Settings->GetSecondEnergyRangeMax();    
  }

  if (m_Settings->GetThirdEnergyRangeMin() != 0 || m_Settings->GetThirdEnergyRangeMax() != 0) {
    if (m_Settings->GetThirdEnergyRangeMin() < Emin) Emin = m_Settings->GetThirdEnergyRangeMin();
    if (m_Settings->GetThirdEnergyRangeMax() > Emax) Emax = m_Settings->GetThirdEnergyRangeMax();    
  }

  if (m_Settings->GetFourthEnergyRangeMin() != 0 || m_Settings->GetFourthEnergyRangeMax() != 0) {
    if (m_Settings->GetFourthEnergyRangeMin() < Emin) Emin = m_Settings->GetFourthEnergyRangeMin();
    if (m_Settings->GetFourthEnergyRangeMax() > Emax) Emax = m_Settings->GetFourthEnergyRangeMax();    
  }

  // Then set up the spectral analyzer
  MSpectralAnalyzer S;
  S.SetBatch(true);
  
  // Fill the initial histogram:
  S.SetGeometry(m_IdentificationGeometry);
  S.SetSpectrum(1000, Emin, Emax, 2);

  // Peak search parameters:
  S.SetSignaltoNoiseRatio(m_Settings->GetSpectralSignaltoNoiseRatio());
  S.SetPoissonLimit(m_Settings->GetSpectralPoissonLimit());

  // Isotope Selection
  S.SetIsotopeFileName(m_Settings->GetSpectralIsotopeFileName());
  S.SetEnergyRange(m_Settings->GetSpectralEnergyRange());

  
  cout<<"Identification thread started..."<<endl<<flush;
  m_IsIdentificationThreadRunning = true;

  // Wait until we have an event
  while (m_StopThreads == false && m_Events.size() < 2) {
    gSystem->Sleep(10);
  }
  // Wait until we have a first reconstructed event
  // The event horizon is the newest reconstructed event before the cut-off time
  list<MRealTimeEvent*>::reverse_iterator EventHorizon = m_Events.rbegin();
  while (m_StopThreads == false && (*EventHorizon)->IsReconstructed() == false)  {
    gSystem->Sleep(10);
  }
  
  double EventHorizonTime = 0.0;

  MTimer NapTimer;
  double NapAmount = 100.0;
  double NapTime = 0.0;

  MTimer UpdateTimer;
  
  while (m_StopThreads == false) {
    //cout<<"Main idnetification loop start"<<endl;
    
    double AccumulationTime = m_AccumulationTime;

    // We always sleep 2 seconds:
    UpdateTimer.Reset();
    while (UpdateTimer.GetElapsed() < 1.0 && m_StopThreads == false) {
      gSystem->Sleep((unsigned int) NapAmount);
      NapTime += NapAmount/1000.0;
      if (NapTimer.GetElapsed() > 5.0) {
        m_IdentificationThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
        if (m_IdentificationThreadCpuUsage < 0.0) m_IdentificationThreadCpuUsage = 0.0;
        if (m_IdentificationThreadCpuUsage > 1.0) m_IdentificationThreadCpuUsage = 1.0;
        NapTimer.Reset();
        NapTime = 0.0;
      }
      //cout<<"Nap timer: "<<UpdateTimer.GetElapsed()<<endl;
    }
    UpdateTimer.Reset();
    if (m_StopThreads == true) break;

    //cout<<"Passed nap timer..."<<endl;

    if (m_Events.empty() == true) {
      cout<<"No events to show..."<<endl;
      continue;
    }

    //cout<<"Identification..."<<endl;

    //cout<<"Going to event horizon from ID="<<(*EventHorizon)->GetID()<<", t="<<(*EventHorizon)->GetTime()<<endl;
    do {
      list<MRealTimeEvent*>::reverse_iterator Next = EventHorizon;
      ++Next;
      if (Next == m_Events.rend()) break;
      if ((*Next)->IsReconstructed() == false) break;
      ++EventHorizon;
    } while (true);
    //cout<<"At event horizon: ID="<<(*EventHorizon)->GetID()<<", t="<<(*EventHorizon)->GetTime()<<", recon:"<<((*EventHorizon)->IsReconstructed() ? "true" : "false")<<endl;

    
    // Check if we really have more new data:
    if (m_IdentificationThreadLastEventID == (*EventHorizon)->GetID() && EventHorizon != m_Events.rbegin()) {
      //cout<<"Continue: No new data"<<endl;
      continue;
    }

    // Reset the spectrum:
    if (DoIdentification == true) {
      S.Reset();
      S.SetSpectrum(1000, Emin, Emax, 2);
    }

    list<MRealTimeEvent*>::iterator Event = EventHorizon.base();
    EventHorizonTime = (*EventHorizon)->GetTime().GetAsDouble();

    int NEvents = 0;
    while (Event != m_Events.end() && EventHorizonTime - (*Event)->GetTime().GetAsDouble() < AccumulationTime) {
      // And fill...
      if ((*Event)->IsCoincident() == true && (*Event)->IsMerged() == false && (*Event)->GetCoincidentRawEvent() != 0) {
        S.FillSpectrum((*Event)->GetCoincidentRawEvent()->GetEnergy());
        ++NEvents;
      }

      Event++;
    }
    
   
    // Now update everything:
    if (Event != m_Events.end()) {
      m_IdentificationThreadFirstEventID = (*Event)->GetID();
    } else {
      m_IdentificationThreadFirstEventID = m_Events.back()->GetID();
    }
    m_IdentificationThreadLastEventID = (*EventHorizon)->GetID();

    if (m_StopThreads == true) break;

    if (DoIdentification == true && S.FindIsotopes() == true) {
      m_IsotopeMutex.Lock();
      m_Isotopes.clear();
      m_Isotopes = S.GetIsotopes();
      m_IsotopeMutex.UnLock();
    }

    
    if (m_StopThreads == true) break;
  }
  m_IsotopeMutex.Lock();
  m_Isotopes.clear();
  m_IsotopeMutex.UnLock();

  m_IsIdentificationThreadRunning = false;

  cout<<"Identification thread finished"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MRealTimeAnalyzer::OneCleanUpLoop()
{
  // Do the clean up

  cout<<"Cleanup thread started..."<<endl<<flush;
  m_IsCleanUpThreadRunning = true;

  list<MRealTimeEvent*>::reverse_iterator EventIter;
  while (m_Events.empty() && m_StopThreads == false) {
    gSystem->Sleep(10);
  }
  EventIter = m_Events.rbegin();
  
  MTimer NapTimer;
  double NapAmount = 100;
  double NapTime = 0;
  
  MRealTimeEvent* Event = 0;
  while (m_StopThreads == false) {
    
    list<MRealTimeEvent*>::iterator DeleteIter = m_Events.end();
    EventIter = m_Events.rbegin();
    while (EventIter != m_Events.rend()) {
      unsigned int ID = (*EventIter)->GetID();
      // Attention: This is not atomic, but shouldn't cause any too severe problems...
      if (ID >= m_TransmissionThreadLastEventID || 
          ID >= m_CoincidenceThreadLastEventID ||   
          ID >= m_ReconstructionThreadLastEventID || 
          ID >= m_ImagingThreadLastEventID || 
          ID >= m_HistogrammingThreadFirstEventID ||
          ID >= m_IdentificationThreadFirstEventID) {
        break;
      }
      DeleteIter = --(EventIter.base()); // One cannot erase from a reverse iterator, this is the trick how to do it
      ++EventIter;
    }
    
    if (DeleteIter != m_Events.end()) {
      m_CleanUpThreadLastEventID = (*DeleteIter)->GetID();
      while (DeleteIter != m_Events.end()) {
        Event = *DeleteIter;
        //cout<<"Deleting:"<<Event->GetID()<<" Boundaries: T="<<m_TransmissionThreadLastEventID<<" C="<<m_CoincidenceThreadLastEventID<<" R="<<m_ReconstructionThreadLastEventID<<" I="<<m_ImagingThreadLastEventID<<" ID="<<m_IdentificationThreadFirstEventID<<" H="<<m_HistogrammingThreadFirstEventID<<" C="<<m_CleanUpThreadLastEventID<<endl;
        DeleteIter = m_Events.erase(DeleteIter); 
        delete Event;
      }
    }
 
    //cout<<"CleanUp thread: Waiting for more events falling off the cliff..."<<endl;
    if (m_StopThreads == true) break;
    gSystem->Sleep((unsigned int) NapAmount);
    NapTime += NapAmount/1000.0;
     if (NapTimer.GetElapsed() > 5.0) {
      m_CleanUpThreadCpuUsage = (1.0 - NapTime/NapTimer.GetElapsed());
      if (m_CleanUpThreadCpuUsage < 0.0) m_CleanUpThreadCpuUsage = 0.0;
      if (m_CleanUpThreadCpuUsage > 1.0) m_CleanUpThreadCpuUsage = 1.0;
      NapTimer.Reset();
      NapTime = 0.0;
    }
    if (m_StopThreads == true) break;
  }
      
  m_IsCleanUpThreadRunning = false;
    
  cout<<"Cleanup thread finished!"<<endl;
}





// MRealTimeAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
