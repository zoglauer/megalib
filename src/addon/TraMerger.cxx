/* 
 * TraAnalyzer.cxx
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
#include <vector>
#include <limits>
#include <map>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <MString.h>
#include <TList.h>
#include <TSystem.h>
#include <TSystemFile.h>
#include <TSystemDirectory.h>

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

class TraAnalyzer
{
public:
  //! Default constructor
  TraAnalyzer();
  //! Default destructor
  ~TraAnalyzer();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze whatever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Tra file name prefix
  MString m_FileNamePrefix;
  //! Geometry file name
  MString m_GeometryFileName;
  //! Output file name
  MString m_OutputFileName;
  //! Maximum number of files
  unsigned int m_MaxFiles;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
TraAnalyzer::TraAnalyzer() : m_Interrupt(false)
{
  m_MaxFiles = numeric_limits<unsigned int>::max();
}


/******************************************************************************
 * Default destructor
 */
TraAnalyzer::~TraAnalyzer()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool TraAnalyzer::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: TraAnalyzer <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -p:   tra file name prefix"<<endl;
  Usage<<"         -o:   output file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -m:   maximum number of files"<<endl;
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
      m_FileNamePrefix = argv[++i];
      cout<<"Accepting file name prefix: "<<m_FileNamePrefix<<endl;
    } else if (Option == "-o") {
      m_OutputFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutputFileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-m") {
      m_MaxFiles = atoi(argv[++i]);
      cout<<"Accepting maximum number of files: "<<m_MaxFiles<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileNamePrefix == "") {
    cout<<"Error: Need a tra file name prefix!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_OutputFileName == "") {
    cout<<"Error: Need an output file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool TraAnalyzer::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  bool InformationTransfered = false;
  MFileEventsTra* Writer = new MFileEventsTra();
  if (Writer->Open(m_OutputFileName, MFile::c_Write) == false) {
    cout<<"Unable to open output file!"<<endl;
    return false;
  }


  // Find files:
  cout<<"Retrieving files..."<<endl;
  TSystemDirectory D(".", gSystem->pwd());
  TList* Files = D.GetListOfFiles();
  if (Files == nullptr) {
    mout<<"ERROR: Did not find or couldn't get any files!"<<endl;
    return false;
  }


  // Sort them
  cout<<"Sorting and eliminating not matching files (current total: "<<Files->LastIndex()<<" files)"<<endl;
  vector<MString> FileNames;
                  
  for (const auto&& Object: *Files) {
    MString Name = Object->GetName();
    if (Name.BeginsWith(m_FileNamePrefix) == false) continue;
    // cout<<"Checking "<<Name<<" for prefix \""<<Prefix<<"\" and suffix \""<<Type<<"\" (+ .gz)... ";
    if (Name.EndsWith(".tra.gz") == true || Name.EndsWith(".tra") == true) {
      FileNames.push_back(Name);
    }
  }
  if (FileNames.size() > 0) {
    cout<<"Considering "<<FileNames.size()<<" files..."<<endl;
  } else {
    cout<<"No files found to join..."<<endl;
    return false;
  }

  // The vector with the events
  vector<MPhysicalEvent*> Events;

  unsigned int FileCounter = 0;
  for (MString FileName: FileNames) {
    if (++FileCounter > m_MaxFiles) break;
    
    MFileEventsTra* Reader = new MFileEventsTra();
    if (Reader->Open(FileName) == false) {
      mout<<"Unable to open file "<<FileName<<". Aborting!"<<endl;
      return false;
    }
    if (InformationTransfered == false) {
      Writer->TransferInformation(Reader);
      InformationTransfered = true;
    }

    MPhysicalEvent* Event = nullptr;
    while ((Event = Reader->GetNextEvent()) != nullptr) { 
      Events.push_back(Event);    
      // NO: delete Event;
    }

    Reader->Close();
    delete Reader;
  }

  // Now sort them:
  cout<<"Starting with sort..."<<endl;
  sort(Events.begin(), Events.end(), [](MPhysicalEvent* A, MPhysicalEvent* B) { return A->GetTime() < B->GetTime(); });

  // And save:
  cout<<"Saving"<<endl;
  Writer->WriteHeader();
  for (MPhysicalEvent* Event: Events) {
    Writer->AddEvent(Event);
  }
  Writer->WriteFooter();
  Writer->Close();

  // Some cleanup
  delete Writer;
  delete Geometry;

  
  return true;
}


/******************************************************************************/

TraAnalyzer* g_Prg = 0;
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

  TApplication TraAnalyzerApp("TraAnalyzerApp", 0, 0);

  g_Prg = new TraAnalyzer();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  if (gROOT->IsBatch() == false) {
    TraAnalyzerApp.Run();
  }

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
