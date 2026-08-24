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
#include <cstdlib>
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
#include "MFile.h"
#include "MGUIExpoCombinedViewer.h"
#include "MGUIMainFretalon.h"
#include "MReadOutAssembly.h"
#include "MModule.h"
#include "MModuleLoaderRoa.h"
#include "MModuleTransmitterRealta.h"
#include "MModuleSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor
    
  m_Interrupt = false;
  m_UseGui = true;

  m_Supervisor = MSupervisor::GetSupervisor();
  
  MString ConfigurationFileName = "~/.fretalon.cfg";
  MFile::ExpandFileName(ConfigurationFileName);
  m_Supervisor->SetConfigurationFileName(ConfigurationFileName);
  
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
}


////////////////////////////////////////////////////////////////////////////////


bool MAssembly::ParseCommandLine(int argc, char** argv, bool& ReadyForUILoop)
{
  ReadyForUILoop = false;

  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Nuclearizer <options>"<<endl;
  Usage<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.fretalon.cfg is loaded by default"<<endl;
  Usage<<"      -C --change-configuration <pattern>:"<<endl;
  Usage<<"             Replace any value in the configuration file (-C can be used multiple times)"<<endl;
  Usage<<"             E.g. to replace the geometry file name, one would set pattern to:"<<endl;
  Usage<<"             -C GeometryFileName=/path/to/some.geo.setup"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically start analysis without GUI"<<endl;
  Usage<<"      -m --multithreading:"<<endl;
  Usage<<"             0: false (default), else: true"<<endl;
  Usage<<"      -v --verbosity:"<<endl;
  Usage<<"             Verbosity: 0: Quiet, 1: Errors, 2: Warnings, 3: Info, 4: Chatty, 5: Extreme"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  
  // Store some options temporarily:
  MString Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      mout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // Single argument
    if (Option == "-c" || Option == "--configuration" ||
        Option == "-C" || Option == "--change-configuration" ||
        Option == "-m" || Option == "--multithreading" ||
        Option == "-v" || Option == "--verbosity") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        mout<<"Error: Option "<<Option<<" needs a second argument!"<<endl;
        mout<<Usage.str()<<endl;
        return true;
      }

      int ParsedValue = 0;
      if (Option == "-m" || Option == "--multithreading" ||
          Option == "-v" || Option == "--verbosity") {
        if (MString(argv[i+1]).Is<int>() == false) {
          mout<<"Error: Option "<<Option<<" needs a plain integer second argument!"<<endl;
          mout<<Usage.str()<<endl;
          return true;
        }
        ParsedValue = MString(argv[i+1]).ToInt();
      }
      // Check verbosity range
      if ((Option == "-v" || Option == "--verbosity") && (ParsedValue < c_Quiet || ParsedValue > c_Extreme)) {
        mout<<"Error: Option "<<Option<<" needs a verbosity value between "<<c_Quiet<<" and "<<c_Extreme<<"!"<<endl;
        mout<<Usage.str()<<endl;
        return true;
      }
      ++i; // We have one option here to skip
    } else if (Option == "-a" || Option == "--auto") {
      // No argument to skip
    } else {
      mout<<"Error: Unknown option "<<Option<<endl;
      mout<<Usage.str()<<endl;
      return true;
    }
  }

  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      if (m_Supervisor->Load(argv[++i]) == false) {
        mout<<"Error: Configuration file "<<argv[i]<<" could not be fully loaded"<<endl;
        return true;
      }
      mout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
    } else if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = MString(argv[++i]).ToInt();
      mout<<"Command-line parser: Verbosity "<<g_Verbosity<<endl;
    } else if (Option == "--multithreading" || Option == "-m") {
      bool UseMultiThreading = (MString(argv[++i]).ToInt() != 0);
      m_Supervisor->UseMultiThreading(UseMultiThreading);
      mout<<"Command-line parser: Using multithreading: "<<(UseMultiThreading == true ? "yes" : "no")<<endl;
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }

  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--change-configuration" || Option == "-C") {
      if (m_Supervisor->ChangeConfiguration(argv[++i]) == false) {
        mout<<"Error: Command-line parser: Unable to change this configuration value: "<<argv[i]<<endl;
      } else {
        mout<<"Command-line parser: Changing this configuration value: "<<argv[i]<<endl;
      }
    }
  }

  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      m_UseGui = false;
      gROOT->SetBatch(true);
      const bool AnalysisFailed = (m_Supervisor->Analyze() == false);
      m_Supervisor->Exit();
      return AnalysisFailed;
    }
  }

  if (m_UseGui == true) {
    if (m_Supervisor->LaunchUI() == false) {
      return true;
    }
  }

  ReadyForUILoop = true;
  return false;
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
  if (MGlobal::Initialize() == false) {
    return 1;
  }

  TApplication* AppNuclearizer = new TApplication("Nuclearizer", 0, 0);
  // Make sure TApplication::Terminate() returns here
  AppNuclearizer->SetReturnFromRun(true);

  MAssembly Nuclearizer;
  g_Prg = &Nuclearizer;
  bool ReadyForUILoop = false;
  const bool HadError = Nuclearizer.ParseCommandLine(argc, argv, ReadyForUILoop);
  if (HadError == false && ReadyForUILoop == true) {
    AppNuclearizer->Run();
  }

  return HadError == true ? 1 : 0;
}


// MAssembly: the end...
////////////////////////////////////////////////////////////////////////////////
