/* 
 * AtmosphereImager.cxx
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

class AtmosphereImager
{
public:
  //! Default constructor
  AtmosphereImager();
  //! Default destructor
  ~AtmosphereImager();
  
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
AtmosphereImager::AtmosphereImager() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
AtmosphereImager::~AtmosphereImager()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool AtmosphereImager::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: AtmosphereImager <options>"<<endl;
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

  if (m_FileName.EndsWith(".sim") == false) {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool AtmosphereImager::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
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

  // Create some histograms here ...
  TH1D* Hist = new TH1D("Angular distance", "AngularDistance", 180, 0, 180);
  Hist->SetXTitle("Distance between incidence and original direction [deg]");
  Hist->SetYTitle("cts");
  
  // Create some histograms here ...
  TH1D* ARMHist = new TH1D("ARM", "ARM", 361, -180, 180);
  ARMHist->SetXTitle("ARM [deg]");
  ARMHist->SetYTitle("cts");
  
  int NFirst = 0;
  int NTotal = 0;
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;

    if (Event->GetNIAs() >= 2) {
      NTotal++; 
      if (Event->GetIAAt(1)->GetProcess() == "ENTR") {
        NFirst++; 
      }
    }
    
    // Do your analysis:
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MSimIA* IA = Event->GetIAAt(i);
      if (IA->GetProcess() == "ENTR") { 
        double Angle = Event->GetIAAt(0)->GetSecondaryDirection().Angle(Event->GetIAAt(i)->GetMotherDirection());
        Hist->Fill(Angle*c_Deg, 1);
        
        // Only use photon from > 60 deg from zenith
        //if (Event->GetIAAt(i)->GetMotherDirection().Angle(MVector(0, 0, -1))*c_Deg > 70) continue;
        
        // Create a fake Compton event according to Klein-Nishina:
        double Phi = MComptonEvent::GetRandomPhi(Event->GetIAAt(i)->GetMotherEnergy());
        // Create a fake direction
        double Chi = 2*c_Pi * gRandom->Rndm();
        MVector Dg(sin(Phi)*cos(Chi), sin(Phi)*sin(Chi), cos(Phi));
        Dg.RotateReferenceFrame(Event->GetIAAt(i)->GetMotherDirection());
        // Phi Uncertainty:
        Phi += gRandom->Gaus(0.0, 8.0/2.35*c_Rad);
        // Calculate the ARM
        double ARM = Event->GetIAAt(0)->GetSecondaryDirection().Angle(Dg) - Phi;
        ARMHist->Fill(ARM*c_Deg);
      }
    }
    
    // Never forget to delete the event
    delete Event;
  }

  // Some cleanup
  delete Reader;
  delete Geometry;

  /*
  // Normalize by sr:
  for (int bx = 1; bx <= Hist->GetNbinsX(); ++bx) {
    double Area = 2*c_Pi*(cos(Hist->GetXaxis()->GetBinLowEdge(bx)*c_Rad) - cos(Hist->GetXaxis()->GetBinUpEdge(bx)*c_Rad));
    Hist->SetBinContent(bx, Hist->GetBinContent(bx)/Area); 
  }
  */
  
  // Show your histogram here
  TCanvas* C = new TCanvas();
  C->cd();
  Hist->Draw();
  C->Update();

  // Show your histogram here
  TCanvas* ARMCanvas = new TCanvas();
  ARMCanvas->cd();
  ARMHist->Draw();
  ARMCanvas->Update();

  cout<<"First IA: "<<NFirst<<" vs. total: "<<NTotal<<endl;
  
  return true;
}


/******************************************************************************/

AtmosphereImager* g_Prg = 0;
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

  TApplication AtmosphereImagerApp("AtmosphereImagerApp", 0, 0);

  g_Prg = new AtmosphereImager();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  AtmosphereImagerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
