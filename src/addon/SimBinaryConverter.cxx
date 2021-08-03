/* 
 * SimBinaryConverter.cxx
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
#include <string>
#include <sstream>
#include <csignal>
#include <map>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MString.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MBinaryStore.h"

/******************************************************************************/

class SimBinaryConverter
{
public:
  /// Default constructor
  SimBinaryConverter();
  /// Default destructor
  ~SimBinaryConverter();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Analyze the event, return true if it has to be writen to file
  bool AnalyzeEvent(MSimEvent* Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  MString m_FileName;
  /// Output file name
  MString m_OutputFileName;
  /// Output file name
  MString m_OutputSuffix;
  /// Geometry file name
  MString m_GeometryFileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
SimBinaryConverter::SimBinaryConverter() : m_Interrupt(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
SimBinaryConverter::~SimBinaryConverter()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool SimBinaryConverter::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: SimBinaryConverter <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -o:   output simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
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
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".bin.sim") == true) {
    if (m_OutputFileName == "") {
      m_OutputFileName = m_FileName;
      m_OutputFileName.ReplaceAtEndInPlace(".bin.sim", ".sim");
      m_OutputSuffix = ".sim";
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    }    
  } else if (m_FileName.EndsWith(".bin.sim.gz") == true) {
    if (m_OutputFileName == "") {
      m_OutputFileName = m_FileName;
      m_OutputFileName.ReplaceAtEndInPlace(".bin.sim.gz", ".sim.gz");
      m_OutputSuffix = ".sim.gz";
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    }    
  } else if (m_FileName.EndsWith(".sim") == true) {
    if (m_OutputFileName == "") {
      m_OutputFileName = m_FileName;
      m_OutputFileName.ReplaceAtEndInPlace(".sim", ".bin.sim");
      m_OutputSuffix = ".bin.sim";
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    }    
  } else if (m_FileName.EndsWith(".sim.gz") == true) {
    if (m_OutputFileName == "") {
      m_OutputFileName = m_FileName;
      m_OutputFileName.ReplaceAtEndInPlace(".sim.gz", ".bin.sim.gz");
      m_OutputSuffix = ".bin.sim.gz";
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    }    
  } else {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool SimBinaryConverter::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  if (Reader->Open(m_FileName) == false) {
    cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl; 
    return false;
  }
  Reader->ShowProgress();

  while (MFile::Exists(m_OutputFileName) == true) {
    int ID = m_OutputFileName.Extract(".id", m_OutputSuffix).ToInt();
    ++ID;
    m_OutputFileName = m_OutputFileName.GetSubString(0,  m_OutputFileName.Index(".id") + 3) + ID + m_OutputSuffix;
  }

  // Open output file:
  bool Binary = !(Reader->IsBinary());
  MFileEventsSim* Writer = new MFileEventsSim(Geometry);
  if (Writer->Open(m_OutputFileName, MFile::c_Write, Binary) == false) {
    cout<<"Unable to open output file!"<<endl;
    return false;
  }
   
  Writer->SetGeometryFileName(m_GeometryFileName);
  Writer->SetVersion(101);
  Writer->WriteHeader();

  MSimEvent* Event = 0;
  
  while ((Event = Reader->GetNextEvent(false)) != 0) {
    if (m_Interrupt == true) return false;
    if (AnalyzeEvent(Event) == true) {
      /*
      if (Binary == true) {
        MBinaryStore Store;
        Event->ToBinary(Store, MSimEvent::c_StoreSimulationInfoAll, true);
      
        MString S = Event->ToSimString();
        BytesBinary += Store.GetArraySize();
        BytesString += S.Length();
      
        MSimEvent* E2 = new MSimEvent();
        E2->SetGeometry(Geometry);
        try {
          E2->ParseBinary(Store);
        } catch (...) {
          cout<<"Unable to parse an event"<<endl;
          delete Event;
          break;
        }
        
        cout<<"Orig: "<<endl<<S<<endl;
        cout<<"Conv: "<<endl<<E2->ToSimString()<<endl;
      } else {
        cout<<"Orig: "<<endl<<Event->ToSimString()<<endl;
      }
      */
      Writer->AddEvent(Event);      
    }
    delete Event;
  }

  cout<<"Observation time: "<<Reader->GetObservationTime()<<" sec  --  simulated events: "<<Reader->GetSimulatedEvents()<<endl;

  Reader->Close();
  delete Reader;
  
  Writer->SetObservationTime(Reader->GetObservationTime());
  Writer->SetSimulatedEvents(Reader->GetSimulatedEvents());
  Writer->CloseEventList();
  Writer->Close();
  delete Writer;
  
  delete Geometry;

  return true;
}


/******************************************************************************
 * Analyze the event, return true if it has to be writen to file
 */
bool SimBinaryConverter::AnalyzeEvent(MSimEvent* Event)
{
  // Add your code here
  // Return true if the event should be written to file

  // Example:
  // if (Event.GetVeto() == true) return false;

  return true;
}


/******************************************************************************/

SimBinaryConverter* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the analysis..."<<endl;
    if (g_Prg != 0) {
      g_Prg->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication SimBinaryConverterApp("SimBinaryConverterApp", 0, 0);

  g_Prg = new SimBinaryConverter();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //SimBinaryConverterApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
