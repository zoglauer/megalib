/* 
 * DistanceOptimizerForEventAnalyzer.cxx
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

/******************************************************************************/

class DistanceOptimizerForEventAnalyzer
{
public:
  //! Default constructor
  DistanceOptimizerForEventAnalyzer();
  //! Default destructor
  ~DistanceOptimizerForEventAnalyzer();
  
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
  
  //! Minimum energy
  double m_MinimumEnergy;
  
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
DistanceOptimizerForEventAnalyzer::DistanceOptimizerForEventAnalyzer() : m_Interrupt(false)
{
  m_MinimumEnergy = 0;
}


/******************************************************************************
 * Default destructor
 */
DistanceOptimizerForEventAnalyzer::~DistanceOptimizerForEventAnalyzer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool DistanceOptimizerForEventAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: DistanceOptimizerForEventAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -m:   minimum energy"<<endl;
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
    } else if (Option == "-m") {
      m_MinimumEnergy = atof(argv[++i]);
      cout<<"Accepting minimum energy: "<<m_MinimumEnergy<<endl;
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

  if (m_FileName.EndsWith(".sim") == false && m_FileName.EndsWith(".sim.gz") == false) {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool DistanceOptimizerForEventAnalyzer::Analyze()
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
  TH1D* MaximumDistanceSingleEvent = new TH1D("MaximumDistanceSingleEvent", "MaximumDistanceSingleEvent", 100, 0, 100);
  MaximumDistanceSingleEvent->SetXTitle("Maximum Distance [cm]");
  MaximumDistanceSingleEvent->SetYTitle("cts");
  
  TH1D* MinimumDistanceBetweenEvents = new TH1D("MinimumDistanceBetweenEvents", "MinimumDistanceBetweenEvents", 100, 0, 100);
  MinimumDistanceBetweenEvents->SetXTitle("Maximum Distance [cm]");
  MinimumDistanceBetweenEvents->SetYTitle("cts");
  MinimumDistanceBetweenEvents->SetLineColor(kGreen);
  
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;

    // Do your analysis
    

    if (Event->GetTotalEnergyDeposit() > m_MinimumEnergy) {
      
      vector<vector<MSimHT*>> AllEvents;
      for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
        MSimIA* IA = Event->GetIAAt(i);
        // Look for all inits
        if (IA->GetProcess() == "INIT") {
          // Find all hits originating from this process
          vector<MSimHT*> HTs = Event->GetHTListIncludingDescendents(IA->GetID());
          // and calculate the alrgest distance between those - if there are multiples
          if (HTs.size() > 0) {
            double LargestDistance = 0;
            for (unsigned int a = 0; a < HTs.size(); ++a) {
              for (unsigned int b = a+1; b < HTs.size(); ++b) {
                if ((HTs[a]->GetPosition() - HTs[b]->GetPosition()).Mag() > LargestDistance) {
                  LargestDistance = (HTs[a]->GetPosition() - HTs[b]->GetPosition()).Mag();
                } 
              }
            }
            MaximumDistanceSingleEvent->Fill(LargestDistance);
          }
          AllEvents.push_back(HTs);
        }
      }
      
      // Get the smallest distance between the events 
      for (unsigned int e = 0; e < AllEvents.size(); ++e) {
        for (unsigned int f = e+1; f < AllEvents.size(); ++f) {
          double SmallestDistance = numeric_limits<double>::max();
          for (unsigned int a = 0; a < AllEvents[e].size(); ++a) {
            for (unsigned int b = a+1; b < AllEvents[f].size(); ++b) {
              if ((AllEvents[e][a]->GetPosition() - AllEvents[f][b]->GetPosition()).Mag() < SmallestDistance) {
                SmallestDistance = (AllEvents[e][a]->GetPosition() - AllEvents[f][b]->GetPosition()).Mag();
              } 
            }
          }
          MinimumDistanceBetweenEvents->Fill(SmallestDistance);
        }
      }
    }
    
    // Never forget to delete the event
    delete Event;
  }

  // Some cleanup
  delete Reader;
  delete Geometry;

  // Show your histogram here
  TCanvas* C = new TCanvas();
  C->cd();
  MaximumDistanceSingleEvent->Draw();
  MinimumDistanceBetweenEvents->Draw("SAME");
  C->Update();
  
  return true;
}


/******************************************************************************/

DistanceOptimizerForEventAnalyzer* g_Prg = 0;
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

  TApplication DistanceOptimizerForEventAnalyzerApp("DistanceOptimizerForEventAnalyzerApp", 0, 0);

  g_Prg = new DistanceOptimizerForEventAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  DistanceOptimizerForEventAnalyzerApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
