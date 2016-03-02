/* 
 * EventListAnalyzer.cxx
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

/******************************************************************************/

class EventListAnalyzer
{
public:
  //! Default constructor
  EventListAnalyzer();
  //! Default destructor
  ~EventListAnalyzer();
  
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
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
EventListAnalyzer::EventListAnalyzer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  //gROOT->SetBatch(true);
}


/******************************************************************************
 * Default destructor
 */
EventListAnalyzer::~EventListAnalyzer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool EventListAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: EventListAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   tra file name"<<endl;
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
    cout<<"Error: Need a tracked events file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".tra") == false) {
    cout<<"Error: Need a tracked events file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool EventListAnalyzer::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  MFileEventsTra* Reader = new MFileEventsTra();
  if (Reader->Open(m_FileName) == false) {
    mout<<"Unable to open file "<<m_FileName<<". Aborting!"<<endl;
  }
  Reader->ShowProgress();

  TH1D* Hist = new TH1D("Relative scatter direction", "Relative scatter direction", 9, 0, 180);
  Hist->SetXTitle("Angle [deg]");
  Hist->SetYTitle("cts");
  Hist->SetMinimum(0);

  MPhysicalEvent* Event;
  MComptonEvent* ComptonEvent;
  while ((Event = Reader->GetNextEvent()) != 0) { 
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;
    
    if (Event->Ei() < 500) {
      delete Event;
      continue;
    }
    
    if (Event->GetType() == MPhysicalEvent::c_Compton) {
      ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

      for (unsigned int c = 0; c < ComptonEvent->GetNComments(); ++c) {
        if (ComptonEvent->GetComment(c).BeginsWith("Absorber") == true) {
          // ToDo: we should be able to do some sanity checks if we got the correct first hit 
          //       since the possible positions are restricted by the start position, and the scatterer 
          
          // Scatter direction of the upper gamma ray
          MTokenizer T;
          T.Analyze(ComptonEvent->GetComment(c));
          MVector UpperScatterDir(T.GetTokenAtAsDouble(1), T.GetTokenAtAsDouble(2), 0);
          
          MVector LowerScatterDir = (ComptonEvent->C2() - ComptonEvent->C1());
          LowerScatterDir[2] = 0;
          
          Hist->Fill(UpperScatterDir.Angle(LowerScatterDir)*c_Deg);
          
          /*
          // The origin of the gammas is at 0, 0, 0, so the direction of the inital gamma ray is:
          MVector LowerInitialDir = ComptonEvent->C1();
         
          // The angle we are interested in is the angle between the lower scattered gamma ray and 
          // the plane spanned by the direction of the initial lower gamma ray the scatter direction 
          // of the upper gamma ray
          
          double Angle = (ComptonEvent->C2() - ComptonEvent->C1()).Angle(LowerInitialDir.Cross(UpperScatterDir));
          Hist->Fill(Angle*c_Deg);
          */
        }
      }
    }

    delete Event;
  }
  
  TCanvas* Canvas = new TCanvas();
  Canvas->cd();
  Hist->Draw();
  Canvas->Update();

  // Some cleanup
  delete Reader;
  delete Geometry;

  
  return true;
}


/******************************************************************************/

EventListAnalyzer* g_Prg = 0;
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

  TApplication EventListAnalyzerApp("EventListAnalyzerApp", 0, 0);

  g_Prg = new EventListAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->IsBatch() == false) {
    EventListAnalyzerApp.Run();
  }

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
