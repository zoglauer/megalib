/* 
 * CreateTriggerMap.cxx
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
#include "MDTriggerMap.h"


/******************************************************************************/

class CreateTriggerMap
{
public:
  //! Default constructor
  CreateTriggerMap();
  //! Default destructor
  ~CreateTriggerMap();
  
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
CreateTriggerMap::CreateTriggerMap() : m_Interrupt(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
CreateTriggerMap::~CreateTriggerMap()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool CreateTriggerMap::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: CreateTriggerMap <options>"<<endl;
  Usage<<"    General options:"<<endl;
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
    if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
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
bool CreateTriggerMap::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  vector<MDDetector*> Detectors = Geometry->GetDetectorList();
  vector<MString> AllNames;
  
  vector<MDDetector*> Trackers;
  for (MDDetector* D: Detectors) {
    if (D->GetType() == MDDetector::c_Strip2D && D->HasNamedDetectors() == false) {
      Trackers.push_back(D);
      AllNames.push_back(D->GetName());
    }
  }
  
  vector<MDDetector*> Calorimeters;
  for (MDDetector* D: Detectors) {
    if (D->GetType() == MDDetector::c_Calorimeter && D->HasNamedDetectors() == false) {
      Calorimeters.push_back(D);
      AllNames.push_back(D->GetName());
    }
  }
  
  vector<MDDetector*> TrackerGuardRings;
  for (MDDetector* D: Detectors) {
    if (D->GetType() == MDDetector::c_GuardRing && D->HasNamedDetectors() == false) {
      TrackerGuardRings.push_back(D);
      AllNames.push_back(D->GetName());
    }
  }
  
  vector<MDDetector*> ACS;
  for (MDDetector* D: Detectors) {
    if (D->GetType() == MDDetector::c_Scintillator && D->HasNamedDetectors() == false) {
      ACS.push_back(D);
      AllNames.push_back(D->GetName());
    }
  }
  MDDetector* TopACS = Geometry->GetDetector("AntiTop");


  // A trigger map with one hit in the tracker and one in the calorimeter
  // This trigger map is for comparison with the default basic trigger mode
  MDTriggerMap OneSiteTriggers("OneSiteTriggers");
  OneSiteTriggers.SetDetectors(AllNames);
  for (unsigned int i = 0; i < Trackers.size(); ++i) {
    vector<MString> Names = { Trackers[i]->GetName(), Calorimeters[0]->GetName() };
    vector<unsigned int> Hits = { 1, 1 };
    vector<bool> AllowMoreHits = { true, true };
    OneSiteTriggers.SetTrigger(MDTriggerMode::c_VetoableTrigger, Names, Hits, AllowMoreHits);
  }
  OneSiteTriggers.WriteTriggerMap("OneSiteTriggers.trig");
  
  
  // A trigger map which combined all the following ones in one file
  MDTriggerMap Combined("Combined");
  Combined.SetDetectors(AllNames);
  

  // Anything with at least two hits in adjacent tracker layers plus calorimeter triggers
  MDTriggerMap TwoSiteTriggers("TwoSiteTriggers");
  TwoSiteTriggers.SetDetectors(AllNames);
  for (unsigned int i = 0; i < Trackers.size() - 1; ++i) {
    vector<MString> Names = { Trackers[i]->GetName(), Trackers[i+1]->GetName(), Calorimeters[0]->GetName() };
    vector<unsigned int> Hits = { 1, 1, 1 };
    vector<bool> AllowMoreHits = { true, true, true };
    TwoSiteTriggers.SetTrigger(MDTriggerMode::c_VetoableTrigger, Names, Hits, AllowMoreHits);
    Combined.SetTrigger(MDTriggerMode::c_VetoableTrigger, Names, Hits, AllowMoreHits);
  }
  TwoSiteTriggers.WriteTriggerMap("TwoSiteTriggers.trig");
  
  
  // Normal veto: 
  // (1) Anything in any guard ring
  MDTriggerMap GuardRingVeto("GuardRingVeto");
  GuardRingVeto.SetDetectors(AllNames);
  for (unsigned int i = 0; i < TrackerGuardRings.size(); ++i) {
    vector<MString> Names = { TrackerGuardRings[i]->GetName() };
    vector<unsigned int> Hits = { 1 };
    vector<bool> AllowMoreHits = { true };
    GuardRingVeto.SetTrigger(MDTriggerMode::c_Veto, Names, Hits, AllowMoreHits);
    Combined.SetTrigger(MDTriggerMode::c_Veto, Names, Hits, AllowMoreHits);
  }
  GuardRingVeto.WriteTriggerMap("GuardRingVeto.trig");
  
  // (2) Anything in any ACS
  MDTriggerMap ACSVeto("ACSVeto");
  ACSVeto.SetDetectors(AllNames);
  for (unsigned int i = 0; i < ACS.size(); ++i) {
    vector<MString> Names = { ACS[i]->GetName() };
    vector<unsigned int> Hits = { 1 };
    vector<bool> AllowMoreHits = { true };
    ACSVeto.SetTrigger(MDTriggerMode::c_Veto, Names, Hits, AllowMoreHits);
    Combined.SetTrigger(MDTriggerMode::c_Veto, Names, Hits, AllowMoreHits);
  }
  ACSVeto.WriteTriggerMap("ACSVeto.trig");
  
  // If we have tracks which are at least 6 tracks deep and have at least 6 hits in the calorimeter (but have zero hits in the top coincidence or the top tracker layer), they become non vetoable
  // Anything with at least 6 hits in adjacent tracker layers plus 6 hits in th calorimeter triggers
  MDTriggerMap PairTrigger("PairTrigger");
  PairTrigger.SetDetectors(AllNames);
  for (unsigned int i = 1; i < Trackers.size() - 5; ++i) {
    vector<MString> Names = { TopACS->GetName(), Trackers[0]->GetName(), Trackers[i]->GetName(), Trackers[i+1]->GetName(),  Trackers[i+2]->GetName(), Trackers[i+3]->GetName(), Trackers[i+4]->GetName(), Trackers[i+5]->GetName(), Calorimeters[0]->GetName() };
    vector<unsigned int> Hits = { 0, 0, 1, 1, 1, 1, 1, 1, 6 };
    vector<bool> AllowMoreHits = { false, false, true, true, true, true, true, true, true };
    PairTrigger.SetTrigger(MDTriggerMode::c_NonVetoableTrigger, Names, Hits, AllowMoreHits);
    Combined.SetTrigger(MDTriggerMode::c_NonVetoableTrigger, Names, Hits, AllowMoreHits);
  }
  PairTrigger.WriteTriggerMap("PairTrigger.trig");
  // Future: To account for passive material in the tracker

  Combined.WriteTriggerMap("Combined.trig");
  
  
  
  return true;
}


/******************************************************************************/

CreateTriggerMap* g_Prg = 0;
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

  TApplication CreateTriggerMapApp("CreateTriggerMapApp", 0, 0);

  g_Prg = new CreateTriggerMap();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //CreateTriggerMapApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
