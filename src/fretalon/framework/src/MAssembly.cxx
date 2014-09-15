/*
* MAssembly.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAssembly.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
#include <csignal>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGMsgBox.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MString.h"
#include "MTimer.h"
#include "MGUIExpoCombinedViewer.h"
#include "MGUIMainFretalon.h"
#include "MReadOutAssembly.h"
#include "MModule.h"
#include "MModuleLoaderRoa.h"
#include "MModuleTransmitterRealta.h"
#include "MModuleSaver.h"




////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor
    
  m_Interrupt = false;
  m_UseGui = true;
  
  m_Supervisor = new MSupervisor();
  
  m_Supervisor->SetConfigurationFileName(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".fretalon.cfg"));
  
  m_Supervisor->AddAvailableModule(new MModuleLoaderRoa());  
  m_Supervisor->AddAvailableModule(new MModuleTransmitterRealta());
  m_Supervisor->AddAvailableModule(new MModuleSaver());  

  m_Supervisor->Load();
  
  m_Supervisor->SetUIProgramName("Fretalon");
  //m_Supervisor->SetUIPicturePath("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  m_Supervisor->SetUISubTitle("A calibration framework");
  m_Supervisor->SetUILeadAuthor("Andreas Zoglauer");
  m_Supervisor->SetUICoAuthors("and others");
}



////////////////////////////////////////////////////////////////////////////////


MAssembly::~MAssembly()
{
  // standard destructor
  
  delete m_Supervisor;
}


////////////////////////////////////////////////////////////////////////////////


bool MAssembly::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Nuclearizer <options>"<<endl;
  Usage<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.fretalon.cfg is loaded by default"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically start analysis without GUI"<<endl;
  Usage<<"      -m --multithreading:"<<endl;
  Usage<<"             0: false (default), else: true"<<endl;
  Usage<<"      -v --verbosity:"<<endl;
  Usage<<"             Verbosity: 0: Quiet, 1: Errors, 2: Warnings, 3: Info"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  
  // Store some options temporarily:
  MString Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    
    // Single argument
    if (Option == "-c" || Option == "--configuration" ||
        Option == "-m" || Option == "--multithreading") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
  }
  
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      m_Supervisor->Load(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
    } else if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = atoi(argv[++i]);
      cout<<"Command-line parser: Verbosity "<<g_Verbosity<<endl;
    } else if (Option == "--multithreading" || Option == "-m") {
      if (atoi(argv[++i]) != 0) m_Supervisor->UseMultiThreading(true);
      cout<<"Command-line parser: Using multithreading: "<<(atoi(argv[i]) != 0 ? "yes" : "no")<<endl;
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }
  
  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      m_UseGui = false;
      gROOT->SetBatch(true);
      m_Supervisor->Analyze();
      m_Supervisor->Exit();
      return false;
    }
  }
  
  if (m_UseGui == true) {
    m_Supervisor->LaunchUI();
  }
  
  return true;
}


//////////////////////////////////////////////////////////////////////////////////


MAssembly* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C: sent the signal to interrupt, call Ctrl-C again for abort."<<endl;
    g_Prg->SetInterrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! In the beginning Andreas created main and Andreas said "Let there be code!"
//! After many years of coding and debugging, Andreas saw all that he had made, 
//! and it was very good.
int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Catch a user interrupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication* AppNuclearizer = new TApplication("Nuclearizer", 0, 0);

  MAssembly Nuclearizer;
  g_Prg = &Nuclearizer;
  if (Nuclearizer.ParseCommandLine(argc, argv) == false) {
    return 0;
  } else {
    AppNuclearizer->Run();
  }  

  return 0;
}


// MAssembly: the end...
////////////////////////////////////////////////////////////////////////////////
