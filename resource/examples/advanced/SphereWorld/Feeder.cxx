/* 
 * SimAnalyzer.cxx
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
#include <TH2.h>
#include <TCanvas.h>
#include <TRandom.h>

// MEGAlib
#include "Base.hxx"
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MBinnerFISBEL.h"
#include "MString.h"

/******************************************************************************/

class SimAnalyzer
{
public:
  //! Default constructor
  SimAnalyzer();
  //! Default destructor
  ~SimAnalyzer();
  
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
SimAnalyzer::SimAnalyzer() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
  gROOT->SetBatch(true);
  gRandom->SetSeed(0);
}


/******************************************************************************
 * Default destructor
 */
SimAnalyzer::~SimAnalyzer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool SimAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: SimAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
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
bool SimAnalyzer::Analyze()
{
  Processor P;
  P.Initialize(662);  
  
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  //TH1D* ScatterDir = new TH1D("Scatter", "Scatter", 90, -180, 360);
  //MVector Test(816.327, 0, 2000);
  //TH2D* SphereDir = new TH2D("Sphere", "Sphere", 45, -180, 180, 22, 0, 180);
  
  //MBinnerFISBEL m_Binner;
  //m_Binner.Create(2048);
  //vector<double> Data(2048);
  
  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  if (Reader->Open(m_FileName) == false) {
    cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl; 
    return false;
  }
  Reader->ShowProgress();
    
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;
    
    /*
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      if (Event->GetIAAt(i)->GetProcess() == "ENTR" && Event->GetIAAt(i)->GetMotherParticleID() == 1) {

        if ((Event->GetIAAt(i)->GetPosition() - Test).Mag() < 200.1) {
          MVector Dir = Event->GetIAAt(i)->GetMotherDirection();
          double Energy = Event->GetIAAt(i)->GetMotherEnergy();
          if (Energy > 196.438 && Energy < 217.116) {
            cout<<"Adding (ID="<<Event->GetID()<<"): "<<Dir.Phi()*c_Deg<<":"<<Dir.Theta()*c_Deg<<":"<<Event->GetIAAt(i)->GetMotherEnergy()<<endl;
            SphereDir->Fill(Dir.Phi()*c_Deg, Dir.Theta()*c_Deg);
            Dir *= -1;
            Data[m_Binner.FindBin(Dir.Theta(), Dir.Phi())] += 1;
          }
        }
      }
      
    }
    
    if (Event->GetNIAs() > 2 && Event->GetIAAt(1)->GetProcess() == "COMP") {
      double Angle = MVector(0, 1, 0).Angle(Event->GetIAAt(1)->GetMotherDirection())*c_Deg; 
      ScatterDir->Fill(Angle);
    }
    */
    
    P.Add(Event);
    
    // Never forget to delete the event
    delete Event;
  }

  // Some cleanup
  delete Reader;
  delete Geometry;
  
  /*
  TCanvas* C = new TCanvas();
  C->cd();
  SphereDir->Draw("colz");
  C->Update();
  
  m_Binner.View(Data);
  
  cout<<"Drawn"<<endl;
  */
  
  return true;
}


/******************************************************************************/

SimAnalyzer* g_Prg = 0;
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

  TApplication SimAnalyzerApp("SimAnalyzerApp", 0, 0);

  g_Prg = new SimAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  SimAnalyzerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
