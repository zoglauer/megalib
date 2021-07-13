/* 
 * SimCombiner.cxx
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
#include <MString.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

class SimCombiner
{
public:
  /// Default constructor
  SimCombiner();
  /// Default destructor
  ~SimCombiner();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze whatever needs to be analyzed...
  bool Analyze();
  /// Analyze the event, return true if it has to be writen to file
  bool AnalyzeEvent(MSimEvent& Event);
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  /// Show spectra per detector
  bool Spectra();

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  /// Simulation file name
  vector<MString> m_FileNames;
  /// Output file name
  MString m_OutputFileName;
  /// Geometry file name
  MString m_GeoFileName;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
SimCombiner::SimCombiner() : m_Interrupt(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
SimCombiner::~SimCombiner()
{
  m_OutputFileName = "Combined.sim";
}


/******************************************************************************
 * Parse the command line
 */
bool SimCombiner::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Description:"<<endl;
  Usage<<"    This program combines multiple sim files into one."<<endl;
  Usage<<"    The files are *not* concatenated, it is assumes that the events happened concurrently."<<endl;
  Usage<<"    Example: Your background consists of multiple componenets, you simulated them separately,"<<endl;
  Usage<<"             and now want to combined them into one file."<<endl;
  Usage<<"    The combination stops when the file file with the shortest observation time runs out of time."<<endl;
  Usage<<endl;
  Usage<<"  Attention:"<<endl;
  Usage<<"    The arrival times are re-poissonized!"<<endl;
  Usage<<"    You cannot use this program if you have moving parts in your simulation!"<<endl;
  Usage<<endl;
  Usage<<"  Usage: SimCombiner <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -b <file name>:   background file name (should appear multiple times!)"<<endl;
  Usage<<"         -o <file name>:   output simulation file name"<<endl;
  Usage<<"         -g <file name>:   geometry file name"<<endl;
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
    if (Option == "-f" || Option == "-o" || Option == "-b") {
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
    if (Option == "-f" || Option == "-b") {
      m_FileNames.push_back(argv[++i]);
      if (MFile::Exists(m_FileNames.back()) == false) {
        cout<<"Error: The simulation file does not exist!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      cout<<"Accepting file name: "<<m_FileNames.back()<<endl;
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      if (MFile::Exists(m_GeoFileName) == false) {
        cout<<"Error: The geometry file does not exist!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
      cout<<"Accepting file name: "<<m_GeoFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileNames.size() == 0) {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeoFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }  

  for (unsigned int f = 0; f < m_FileNames.size(); ++f) {
    if (m_FileNames[f].EndsWith(".sim") == false) {
      cout<<"Error: Need a simulation file name, not a "<<m_FileNames[f]<<" file "<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool SimCombiner::Analyze()
{
  if (m_Interrupt == true) return false;

  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();

  if (Geometry->ScanSetupFile(m_GeoFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
  } else {
    cout<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  unsigned int NFiles = m_FileNames.size();
  
  vector<MFileEventsSim*> Readers(NFiles);
  vector<double> ObservationTimes(NFiles);
  vector<long> Events(NFiles);
  vector<double> NextEventTime(NFiles);
  
  for (unsigned int f = 0; f < NFiles; ++f) {
    MFileEventsSim* SiReader = new MFileEventsSim(Geometry);
    if (SiReader->Open(m_FileNames[f]) == false) {
      cout<<"Unable to open sim file: "<<m_FileNames[f]<<endl; 
      return false;
    }
    SiReader->ShowProgress();
    Events[f] = SiReader->GetNEvents(false);
    ObservationTimes[f] = SiReader->GetObservationTime().GetAsSeconds();
    NextEventTime[f] = gRandom->Exp(ObservationTimes[f]/Events[f]);
    cout<<"Triggered events: "<<Events[f]<<" --- Observation time: "<<ObservationTimes[f]<<endl;
    SiReader->Rewind();
    Readers[f] = SiReader;
  }

  // Open output file:
  MFileEventsSim* SiWriter = new MFileEventsSim(Geometry);
  if (SiWriter->Open(m_OutputFileName, MFile::c_Write) == false) {
    cout<<"Unable to open output file: "<<m_OutputFileName<<endl;
    return false;
  }
   
  SiWriter->SetGeometryFileName(m_GeoFileName);
  SiWriter->SetVersion(25);
  SiWriter->WriteHeader();

  MSimEvent* SiEvent = 0;
  
  unsigned int Next = 0;
  double NextTime = NextEventTime[Next];
  for (unsigned int f = 0; f < NFiles; ++f) {
    if (NextEventTime[f] < NextTime) {
      Next = f;  
      NextTime = NextEventTime[Next];
    }
  }
  
  
  while (true) {
    if ((SiEvent = Readers[Next]->GetNextEvent(false)) == 0) {
      cout<<m_FileNames[Next]<<" is out of events!"<<endl;
      break;
    }
    SiEvent->SetTime(NextTime);
    SiWriter->AddEvent(SiEvent);
    SiWriter->SetObservationTime(NextTime);
    delete SiEvent;
    NextEventTime[Next] += gRandom->Exp(ObservationTimes[Next]/Events[Next]);
    NextTime = NextEventTime[Next];
    for (unsigned int f = 0; f < NFiles; ++f) {
      if (NextEventTime[f] < NextTime) {
        Next = f;  
        NextTime = NextEventTime[Next];
      }
    }
  }

  SiWriter->CloseEventList();
  SiWriter->Close();
  delete SiWriter;
  for (unsigned int f = 0; f < NFiles; ++f) {
    Readers[f]->Close();
    delete Readers[f];
  }
  delete Geometry;
  
  return true;
}


/******************************************************************************/

SimCombiner* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  //void (*handler)(int);
  //handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication SimCombinerApp("SimCombinerApp", 0, 0);

  g_Prg = new SimCombiner();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //SimCombinerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
