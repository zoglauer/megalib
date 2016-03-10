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

  //! Simulation file name - polarized
  MString m_PolarizedFileName;
  //! Simulation file name - unpolarized
  MString m_UnpolarizedFileName;
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
  Usage<<"         -p:   tra file name - polarized data"<<endl;
  Usage<<"         -u:   tra file name - unpolarized data (optional)"<<endl;
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
    if (Option == "-p") {
      m_PolarizedFileName = argv[++i];
      cout<<"Accepting file name: "<<m_PolarizedFileName<<endl;
    } else if (Option == "-u") {
      m_UnpolarizedFileName = argv[++i];
      cout<<"Accepting file name: "<<m_UnpolarizedFileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_PolarizedFileName == "") {
    cout<<"Error: Need a tracked events file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_PolarizedFileName.EndsWith(".tra") == false) {
    cout<<"Error: Need a tracked events file name, not a "<<m_PolarizedFileName<<" file "<<endl;
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

  MFileEventsTra* PolarizedReader = new MFileEventsTra();
  if (PolarizedReader->Open(m_PolarizedFileName) == false) {
    mout<<"Unable to open file "<<m_PolarizedFileName<<". Aborting!"<<endl;
  }
  PolarizedReader->ShowProgress();

  int NBins = 7;
  TH1D* HistPol = new TH1D("Relative scatter direction - polarized", "Relative scatter direction - polarized", NBins, 0, 180);
  HistPol->SetXTitle("Angle [deg]");
  HistPol->SetYTitle("cts");
  HistPol->SetMinimum(0);

  MPhysicalEvent* Event;
  MComptonEvent* ComptonEvent;
  while ((Event = PolarizedReader->GetNextEvent()) != 0) { 
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
          
          HistPol->Fill(UpperScatterDir.Angle(LowerScatterDir)*c_Deg);
          
        }
      }
    }

    delete Event;
  }
  // Some cleanup
  delete PolarizedReader;
 
  TCanvas* Canvas = new TCanvas();
  Canvas->cd();
  HistPol->DrawCopy();
  Canvas->Update();


  if (m_UnpolarizedFileName != "") {
    TH1D* HistUnpol = new TH1D("Relative scatter direction - unpolarized", "Relative scatter direction - unpolarized", NBins, 0, 180);
    HistUnpol->SetXTitle("Angle [deg]");
    HistUnpol->SetYTitle("cts");
    HistUnpol->SetMinimum(0);

    MFileEventsTra* UnpolarizedReader = new MFileEventsTra();
    if (UnpolarizedReader->Open(m_UnpolarizedFileName) == false) {
      mout<<"Unable to open file "<<m_UnpolarizedFileName<<". Aborting!"<<endl;
    }
    UnpolarizedReader->ShowProgress();

    MPhysicalEvent* Event;
    MComptonEvent* ComptonEvent;
    while ((Event = UnpolarizedReader->GetNextEvent()) != 0) { 
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
          
            HistUnpol->Fill(UpperScatterDir.Angle(LowerScatterDir)*c_Deg);
          
          }
        }
      }

      delete Event;
    }
    delete UnpolarizedReader;
  
    TH1D* HistCorrected = new TH1D("Corrected", "Corrected", NBins, 0, 180);
    HistCorrected->SetXTitle("Angle [deg]");
    HistCorrected->SetYTitle("cts - corrected");

    HistPol->Scale(1.0/HistPol->Integral());
    HistUnpol->Scale(1.0/HistUnpol->Integral());

    for (int b = 1; b <= NBins; ++b) {
      HistCorrected->SetBinContent(b, HistPol->GetBinContent(b)/HistUnpol->GetBinContent(b));
    }
    HistCorrected->Scale(1.0/HistCorrected->GetMaximum());
    HistCorrected->SetMinimum(0);

    TCanvas* CanvasCorr = new TCanvas();
    CanvasCorr->cd();
    HistCorrected->DrawCopy();
    CanvasCorr->Update();
  }

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
