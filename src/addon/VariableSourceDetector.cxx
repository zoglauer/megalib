/* 
 * MVariableSourceDetectionTester.cxx
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
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MFileEventsTra.h"
#include "MSettingsMimrec.h"
#include "MVariableSourceDetector.h"


/******************************************************************************/

class MVariableSourceDetectionTester
{
public:
  //! Default constructor
  MVariableSourceDetectionTester();
  //! Default destructor
  ~MVariableSourceDetectionTester();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze whatever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Simulation file name
  MString m_FileName;
  //! Geometry file name
  MString m_GeometryFileName;
  //! Mimrec configuration file name
  MString m_MimrecConfigurationFileName;
  //! History file name
  MString m_HistoryFileName;
};


/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
MVariableSourceDetectionTester::MVariableSourceDetectionTester() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  //gROOT->SetBatch(true);
}


/******************************************************************************
 * Default destructor
 */
MVariableSourceDetectionTester::~MVariableSourceDetectionTester()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool MVariableSourceDetectionTester::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: MVariableSourceDetectionTester <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -c:   mimrec configuration file name"<<endl;
  Usage<<"         -h:   history file name "<<endl;
  Usage<<"               IMPORTANT: the history must have been made with the same configuration file to work!"<<endl;
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
    } else if (Option == "-c") {
      m_MimrecConfigurationFileName = argv[++i];
      cout<<"Accepting mimrec configuration file name: "<<m_MimrecConfigurationFileName<<endl;
    } else if (Option == "-h") {
      m_HistoryFileName = argv[++i];
      cout<<"Accepting history file name: "<<m_HistoryFileName<<endl;
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

  if (m_MimrecConfigurationFileName == "") {
    cout<<"Error: Need a mimrec configuration file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".tra") == false && m_FileName.EndsWith(".tra.gz") == false) {
    cout<<"Error: Need a tra file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool MVariableSourceDetectionTester::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  MSettingsMimrec* Settings = new MSettingsMimrec(false);
  if (Settings->Read(m_MimrecConfigurationFileName) == false) {
    mout<<"Unable to open file "<<m_MimrecConfigurationFileName<<". Aborting!"<<endl;
    return false;
  }

  MVariableSourceDetector* Detector = new MVariableSourceDetector(dynamic_cast<MSettingsImaging*>(Settings), dynamic_cast<MSettingsEventSelections*>(Settings), Geometry);
  
  if (m_HistoryFileName.IsEmpty() == false) {
    Detector->ReadPeakHeightHistory(m_HistoryFileName);
  }
  
  MFileEventsTra* Reader = new MFileEventsTra();
  if (Reader->Open(m_FileName) == false) {
    mout<<"Unable to open file "<<m_FileName<<". Aborting!"<<endl;
    return false;
  }
  Reader->ShowProgress();

  TH1D* Hist = new TH1D("Energy of Compton events", "Energy of Compton Events", 100, 0, 1000);

  MPhysicalEvent* Event;
  MComptonEvent* ComptonEvent;
  while ((Event = Reader->GetNextEvent()) != 0) { 
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;
    
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

      Hist->Fill(ComptonEvent->Ei());
      
      Detector->BackprojectEvent(Event);
    } else {
      delete Event;
    }
  }
  delete Reader;
  
  
  TCanvas* Canvas = new TCanvas();
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();

  Detector->Search();
  
  
  // Some cleanup
  delete Geometry;

  
  return true;
}


/******************************************************************************/

MVariableSourceDetectionTester* g_Prg = 0;
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

  TApplication MVariableSourceDetectionTesterApp("MVariableSourceDetectionTesterApp", 0, 0);

  g_Prg = new MVariableSourceDetectionTester();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->IsBatch() == false) {
    MVariableSourceDetectionTesterApp.Run();
  }

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
