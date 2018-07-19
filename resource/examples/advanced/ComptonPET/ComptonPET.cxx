/* 
 * ComptonPET.cxx
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
#include "MPhotoEvent.h"
#include "MComptonEvent.h"
#include "MPETEvent.h"
#include "MMultiEvent.h"
#include "MFileEventsTra.h"


/******************************************************************************/

class ComptonPET
{
public:
  //! Default constructor
  ComptonPET();
  //! Default destructor
  ~ComptonPET();
  
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
ComptonPET::ComptonPET() : m_Interrupt(false)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
ComptonPET::~ComptonPET()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool ComptonPET::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ComptonPET <options>"<<endl;
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
bool ComptonPET::Analyze()
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

  MFileEventsTra* Writer = new MFileEventsTra();
  Writer->Open("Out.tra", MFile::c_Write);
  
  // Create some histograms here ...
  TH1D* NumberOfComptonInteractionsOfPrimary = new TH1D("N", "Number of Compton interactions of primary particle in active detector", 20, 0.5, 20.5);
  NumberOfComptonInteractionsOfPrimary->SetXTitle("Number of interactions");
  NumberOfComptonInteractionsOfPrimary->SetYTitle("cts");
  
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;

    // First get all the gamma-ray INIT's and the ANNI's
    vector<MSimIA*> Starts;
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MSimIA* IA = Event->GetIAAt(i);
      if (IA->GetProcess() == "INIT" && IA->GetSecondaryParticleID() == 1) {
        Starts.push_back(IA);
      } else if (IA->GetProcess() == "ANNI" && IA->GetSecondaryParticleID() == 1) {
        Starts.push_back(IA);
      }
    }
    
    // Now check if at least one has a good sequence
    vector<MPhysicalEvent*> Events;
    for (MSimIA* S: Starts) {
      // Find the IAs with this one as origin
      bool Found = false;
      for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
        MSimIA* First = Event->GetIAAt(i);
        if (First->GetOriginID() == S->GetID() && First->GetDetector() != 0) {
          if (First->GetProcess() == "COMP") {
            if (i+1 < Event->GetNIAs()) {
              MSimIA* Second = Event->GetIAAt(i+1);
              if (Second->GetOriginID() == S->GetID()) {
                MComptonEvent* E = new MComptonEvent();
                E->SetId(Event->GetID());
                E->SetTime(Event->GetTime());
                E->SetC1(First->GetPosition());
                E->SetEe(S->GetSecondaryEnergy() - First->GetMotherEnergy());
                E->SetEg(First->GetMotherEnergy());
                E->SetC2(Second->GetPosition());
                if (E->Validate() == false) {
                  delete E; 
                } else {
                  Events.push_back(E);
                }
                Found = true;
              }
            } 
          } else if (First->GetProcess() == "PHOT") {
            MPhotoEvent* P = new MPhotoEvent();
            P->SetId(Event->GetID());
            P->SetTime(Event->GetTime());
            P->SetEnergy(S->GetSecondaryEnergy());
            P->SetPosition(First->GetPosition());
            P->Validate();
            Events.push_back(P);
            Found = true;
          }
        }
        if (Found == true) break;
      }
    }
    
    // Let's check what we have:
    cout<<Events.size()<<" events"<<endl;
    for (auto E: Events) {
      cout<<E->GetTypeString()<<": "<<E->Ei()<<endl;
    }
    
    int NCompton = 0;
    int NPET = 0;
    
    // Check if we have PET event to add
    vector<MPhysicalEvent*> NotAnnis;
    vector<MPhysicalEvent*> Annis;
    for (auto E: Events) {
      if (fabs(511 - E->Ei()) < 1) {
        Annis.push_back(E);
      } else {
        NotAnnis.push_back(E);
        ++NCompton;
      }
    }
    

    MPETEvent* P = nullptr;
    if (Annis.size() == 2) {
      P = new MPETEvent();
      P->SetId(Event->GetID());
      P->SetTime(Event->GetTime());
      P->SetEnergy1(Annis[0]->GetEnergy());
      P->SetEnergy2(Annis[1]->GetEnergy());
      P->SetPosition1(Annis[0]->GetPosition());
      P->SetPosition2(Annis[1]->GetPosition());
      NotAnnis.push_back(P);
      delete Annis[0];
      delete Annis[1];
      ++NPET;
    } else {
      for (auto E: Annis) {
        NotAnnis.push_back(E);
        ++NCompton;
      }
    }
    
    if (NPET == 1 && NCompton == 0) {
      Writer->AddEvent(P);
      delete P;
    } else if (NPET == 0 && NCompton == 1) {
      Writer->AddEvent(NotAnnis[0]);
      delete NotAnnis[0];
    } else if (NPET + NCompton > 1) {
      // Create the multi event:
      MMultiEvent* M = new MMultiEvent();
      M->SetId(Event->GetID());
      M->SetTime(Event->GetTime());
      for (auto E: NotAnnis) {
        M->Add(E);
      }
      
      MString CC("Events: ");
      CC += NotAnnis.size();
      M->AddComment(CC);
      if (M->GetNumberOfEvents() > 0) {
        Writer->AddEvent(M);
      }
      delete M;      
    }
    
    
    // Never forget to delete the event
    delete Event;
  }

  // Some cleanup
  delete Reader;
  delete Geometry;

  Writer->Close();
  
  // Show your histogram here
  TCanvas* C = new TCanvas();
  C->cd();
  NumberOfComptonInteractionsOfPrimary->Draw();
  C->Update();
  
  return true;
}


/******************************************************************************/

ComptonPET* g_Prg = 0;
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

  TApplication ComptonPETApp("ComptonPETApp", 0, 0);

  g_Prg = new ComptonPET();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ComptonPETApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
