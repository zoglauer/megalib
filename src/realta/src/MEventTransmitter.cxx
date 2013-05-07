/* 
 * MEventTransmitter.cxx
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

// Standard
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdlib>
#include <csignal>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TObjString.h>

// MEGAlib
#include "MGlobal.h"
#include "MTimer.h"
#include "MTransceiverTcpIp.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MGeometryRevan.h"
#include "MFileEventsEvta.h"
#include "MRERawEvent.h"


/******************************************************************************/

class MEventTransmitter
{
public:
  /// Default constructor
  MEventTransmitter();
  /// Default destructor
  ~MEventTransmitter();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// The geometry file name
  MString m_GeoFileName;
  /// The simulation file name
  vector<MString> m_SimFileNames;
  /// The Port
  int m_Port;
  /// The name of the host
  MString m_Host;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
MEventTransmitter::MEventTransmitter() : m_Interrupt(false), m_Port(9090), m_Host("localhost")
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
MEventTransmitter::~MEventTransmitter()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool MEventTransmitter::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: MEventTransmitter <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -p:   port"<<endl;
  Usage<<"         -d:   destination host"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-f") {
      m_SimFileNames.push_back(argv[++i]);
      cout<<"Accepting simulation file name: "<<m_SimFileNames.back()<<endl;
    } else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeoFileName<<endl;
    } else if (Option == "-p") {
      m_Port = atoi(argv[++i]);
      cout<<"Accepting port: "<<m_Port<<endl;
    } else if (Option == "-d") {
      m_Host = argv[++i];
      cout<<"Accepting host: "<<m_Host<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool MEventTransmitter::Analyze()
{   
  // Load geometry:
  MGeometryRevan* Geometry = new MGeometryRevan();

  if (Geometry->ScanSetupFile(m_GeoFileName) == true) {
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  // Load the event file
  vector<MFileEvents*> Readers;
  for (unsigned int s = 0; s < m_SimFileNames.size(); ++s) {
    MFileEvents* Reader;
    if (m_SimFileNames[s].EndsWith("sim")) {
      Reader = new MFileEventsSim(Geometry);
    } else if (m_SimFileNames[s].EndsWith("evta")) {
      Reader = new MFileEventsEvta(Geometry);
    } else {
      cout<<"Error: File must either have sim or evta suffix: "<<m_SimFileNames[s]<<endl;
      return false;
    }
    if (Reader->Open(m_SimFileNames[s]) == false) {
      cout<<"Error: Unable to open sim file!"<<endl; 
      return false;
    }
    Readers.push_back(Reader);
  }
  double ObservationTime = 0.0;
  
  // Set up the transceiver and connect:
  MTransceiverTcpIp* Transceiver = new MTransceiverTcpIp("A transceiver", m_Host, m_Port, MTransceiverTcpIp::c_ModeRawEventList);
  Transceiver->Connect();

  MTimer Wait;
  bool WaitTimerFirst = true;
  do {
    gSystem->Sleep(1000);
    if (WaitTimerFirst == false) {
      cout<<"\r\r"<<flush;
    } else {
      cout<<endl;
      WaitTimerFirst = false;
    }
    cout<<"Waiting for connection with remote host \""<<m_Host<<":"<<m_Port<<"\" since "<<fixed<<setprecision(1)<<Wait.GetElapsed()<<" seconds..."<<flush;
  } while (Transceiver->IsConnected() == false && m_Interrupt == false);
  cout<<endl;

  if (Transceiver->IsConnected() == true) {
    cout<<"Connection established with \""<<m_Host<<":"<<m_Port<<"\""<<endl;
  }
  
  // Send the events
  int ID = 0;
  MTime Previuos;
  MTime Leap;
  
  unsigned int ReaderID = 0;
  MSimEvent* SiEvent;
  MRERawEvent* ReEvent;
  MTimer Timer;
  bool IsFirst = true;
  MTime FirstTime;
  while (m_Interrupt == false) {
    unsigned int BatchSize = 10 + gRandom->Integer(10);
    MString SendString;
    for (unsigned int b = 0; b < BatchSize; ++b) {
      if (dynamic_cast<MFileEventsSim*>(Readers[ReaderID]) != 0) {
        SiEvent = dynamic_cast<MFileEventsSim*>(Readers[ReaderID])->GetNextEvent(false);
        if (SiEvent == 0) {
          Readers[ReaderID]->Rewind();
          ObservationTime += Readers[ReaderID]->GetObservationTime();
          ReaderID++;
          if (ReaderID >= Readers.size()) ReaderID = 0; 
          cout<<"Switching to file: "<<Readers[ReaderID]->GetFileName()<<endl;
          SiEvent = dynamic_cast<MFileEventsSim*>(Readers[ReaderID])->GetNextEvent(false);
          if (SiEvent == 0) {
            cout<<"No new events after rewinding! Aborting..."<<endl;
            return false;
          }        
        }
        ++ID;
        SiEvent->SetID(ID);
        SiEvent->SetTime(SiEvent->GetTime() + ObservationTime);
        double TimeDifference = SiEvent->GetTime() - Timer.GetElapsed();
        if (TimeDifference > 0) gSystem->Sleep(int(1000*TimeDifference));
    
        SendString += SiEvent->ToSimString(MSimEvent::c_StoreSimulationInfoNone, 6);
      
        delete SiEvent;
      } else if (dynamic_cast<MFileEventsEvta*>(Readers[ReaderID]) != 0) {
        ReEvent = dynamic_cast<MFileEventsEvta*>(Readers[ReaderID])->GetNextEvent();
        if (ReEvent == 0) {
          Readers[ReaderID]->Rewind();
          ObservationTime += Readers[ReaderID]->GetObservationTime();
          ReaderID++;
          if (ReaderID >= Readers.size()) ReaderID = 0; 
          cout<<"Switching to file: "<<Readers[ReaderID]->GetFileName()<<endl;
          ReEvent = dynamic_cast<MFileEventsEvta*>(Readers[ReaderID])->GetNextEvent();
          if (ReEvent == 0) {
            cout<<"No new events after rewinding! Aborting..."<<endl;
            return false;
          }        
        }
        ++ID;
        if (IsFirst == true) {
          FirstTime = ReEvent->GetEventTime();
          IsFirst = false;
        }
        ReEvent->SetEventId(ID);
        ReEvent->SetEventTime(ReEvent->GetEventTime() + ObservationTime - FirstTime);
        
        double TimeDifference = ReEvent->GetEventTime().GetAsSeconds() - Timer.GetElapsed();
        if (TimeDifference > 0 && TimeDifference < 2) {
          gSystem->Sleep(int(1000*TimeDifference));
        }
    
        SendString += ReEvent->ToEvtaString(6, 26);
      
        delete ReEvent;
      } else {
        cout<<"No reader found!"<<endl;
      }
    }

    SendString += "EN\n";
    
    if (Transceiver->IsConnected() == false) {
      Wait.Reset();
      WaitTimerFirst = true;
      do {
        gSystem->Sleep(1000);
        if (WaitTimerFirst == false) {
          cout<<"\r\r"<<flush;
        } else {
          cout<<endl;
          WaitTimerFirst = false;
        }
        cout<<"Waiting for reconnection with remote host \""<<m_Host<<":"<<m_Port<<"\" since "<<fixed<<setprecision(1)<<Wait.GetElapsed()<<" seconds..."<<flush;
      } while (Transceiver->IsConnected() == false && m_Interrupt == false);
      cout<<endl;
    }
     
    Transceiver->Send(SendString);

    cout<<"\r"<<"Sent "<<ID<<" events...  "<<flush;
  }
 
  delete Transceiver;

  return true;
}


/******************************************************************************/

MEventTransmitter* g_Prg = 0;
int g_NInterruptCatches = 1;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Setup the interrupt catcher
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("MEventTransmitter", "an event transmitter");

  g_Prg = new MEventTransmitter();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    delete g_Prg;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    delete g_Prg;
    return -2;
  } 

  delete g_Prg;

  cout<<"Program exited normally!"<<endl;

  return 0;
}


/*
 * Cosima: the end...
 ******************************************************************************/
