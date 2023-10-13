/*
 * MCMain.cxx
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


// Cosima:
#include "MCMain.hh"
#include "MCDetectorConstruction.hh"
#include "MCGeometryConverter.hh"
#include "MCPhysicsList.hh"
#include "MCPrimaryGeneratorAction.hh"
#include "MCSteppingAction.hh"
#include "MCTrackingAction.hh"
#include "MCEventAction.hh"
#include "MCRunManager.hh"

// Geant4:
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4EmCalculator.hh"
#include "G4ParticleTypes.hh"
#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#endif

// ROOT:
#include "TRandom.h"

// MEGAlib:
#include "MGlobal.h"
#include "MStreams.h"

// Standard lib
#include <ctime>
#include <sstream>
using namespace std;


/******************************************************************************
 * Default constructor
 */
MCMain::MCMain()
{
  m_RunManager = nullptr;
  m_VisManager = nullptr;
  m_Session = nullptr;
  m_UI = nullptr;

  m_HasMacro = false;
  m_Interactive = false;
  m_UseDebug = false;
  m_Zip = true;

  m_Verbosity = 1;

  m_ParallelID = 0;
  m_IncarnationID = 0;
  
  // At least under Linux this should work...
  m_Seed = (long) time(0);

  // If empty do not convert...
  m_ConvertFileName = "";
}


/******************************************************************************
 * Default destructor
 */
MCMain::~MCMain()
{
  delete m_Session;
#ifdef G4VIS_USE
  delete m_VisManager;
#endif
  delete m_RunManager;
}


/******************************************************************************
 * Initialize the program
 */
bool MCMain::Initialize()
{
  // Construct the default run manager
  m_RunManager = new MCRunManager(m_RunParameters);

  vector<MCRun>& Runs = m_RunManager->GetRuns();
  for (unsigned int r = 0; r < Runs.size(); ++r) {
    Runs[r].SetZip(m_Zip);
    Runs[r].SetStoreBinary(m_RunParameters.StoreBinary());
    Runs[r].SetIncarnationID(m_IncarnationID);
    Runs[r].SetParallelID(m_ParallelID);
    Runs[r].CheckIncarnationID();
  }

  // set mandatory initialization classes
  m_RunManager->SetUserInitialization(new MCPhysicsList(m_RunParameters));

  m_RunManager->SetUserAction(new MCEventAction(m_RunParameters, m_Zip, m_Seed));


  // Set geometry
  MCDetectorConstruction* DC = new MCDetectorConstruction(m_RunParameters);
  if (DC->Initialize() == false) {
    mout<<"ERROR: Unable to initalize detector"<<endl;
    return false;
  }
  m_RunManager->SetUserInitialization(DC);

  
  // The sanity checking, if we have all volumes, can only be done here:
  for (unsigned int r = 0; r < Runs.size(); ++r) {
    for (unsigned int s = 0; s < Runs[r].GetNSources(); ++s) {
      MCSource* S = Runs[r].GetSource(s);
      if (S->GetVolume() != "") {
        if (DC->HasVolume(S->GetVolume()) == false) {
          mout<<"ERROR: Unable to find required volume "<<S->GetVolume()<<endl;
          return false;
        }
      }
    }
  }
  
  
  if (m_ConvertFileName != "") {
    MCGeometryConverter* Converter = new MCGeometryConverter();
    Converter->Convert(m_ConvertFileName.GetString());
    mout<<"Geometry converted. Aborting program. Ignore following error messages."<<endl;
    return false;
  }

  // Primaries
  m_RunManager->SetUserAction(new MCPrimaryGeneratorAction(m_RunParameters));

#ifdef G4VIS_USE
  // set visualization manager
  m_VisManager = new G4VisExecutive;
  m_VisManager->Initialize();
#endif

  // activate interactive mode
  m_Session = new G4UIterminal(0, false);

  // Initialize G4 kernel
  m_RunManager->Initialize();

  // set mandatory user action class
  m_RunManager->SetUserAction(new MCSteppingAction(m_RunParameters));
  m_RunManager->SetUserAction(new MCTrackingAction());

  // get the pointer to the m_UI manager and set verbosities
  m_UI = G4UImanager::GetUIpointer();

  return true;
}


/******************************************************************************
 * Set an event relegator function (i.e. your own event handler)
 */
void MCMain::SetEventRelegator(void (Relegator)(MSimEvent*))
{
  if (m_RunManager != 0) {
    MCEventAction* Action = MCRunManager::GetMCRunManager()->GetEventAction();
    if (Action != 0) {
      Action->SetRelegator(Relegator);
      return;
    }
  }
  merr<<"Unable to set event relegator!"<<show;
}


/******************************************************************************
 * Execute the macro or parameter file
 */
bool MCMain::Execute()
{
  if (m_Verbosity == 3) {
    m_UI->ApplyCommand("/control/verbose 0");
    m_UI->ApplyCommand("/run/verbose 0");
    m_UI->ApplyCommand("/event/verbose 1");
    m_UI->ApplyCommand("/geometry/verbose 1");
    m_UI->ApplyCommand("/tracking/verbose 1");
    m_UI->ApplyCommand("/particle/property/verbose 0");
    m_UI->ApplyCommand("/particle/process/verbose 0");
    m_UI->ApplyCommand("/gps/verbose 0");
    m_UI->ApplyCommand("/process/verbose 0");
  } else if  (m_Verbosity == 4) {
    m_UI->ApplyCommand("/control/verbose 0");
    m_UI->ApplyCommand("/run/verbose 0");
    m_UI->ApplyCommand("/event/verbose 2");
    m_UI->ApplyCommand("/geometry/verbose 1");
    m_UI->ApplyCommand("/tracking/verbose 2");
    m_UI->ApplyCommand("/particle/property/verbose 0");
    m_UI->ApplyCommand("/particle/process/verbose 0");
    m_UI->ApplyCommand("/gps/verbose 0");
    m_UI->ApplyCommand("/process/verbose 2");
  } else if  (m_Verbosity >= 5) {
    m_UI->ApplyCommand("/control/verbose 0");
    m_UI->ApplyCommand("/run/verbose 0");
    m_UI->ApplyCommand("/event/verbose 3");
    m_UI->ApplyCommand("/geometry/verbose 1");
    m_UI->ApplyCommand("/tracking/verbose 3");
    m_UI->ApplyCommand("/particle/property/verbose 0");
    m_UI->ApplyCommand("/particle/process/verbose 0");
    m_UI->ApplyCommand("/gps/verbose 0");
    m_UI->ApplyCommand("/process/verbose 3");
  }

  if (m_Interactive == true) {
    if (m_HasMacro == true) {
      mout<<"Execute macro...!"<<endl;
      MString Command = "/control/execute ";
      MString FileName = m_MacroFileName;
      m_RunManager->PrepareNextRun();
      m_UI->ApplyCommand(Command + FileName);
    }
    m_Session->SessionStart();
  } else {
    m_RunManager->StartBeam();
  }

  return true;
}


/******************************************************************************
 * Parse the command line
 */
unsigned int MCMain::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Cosima <options> <parameter file>"<<endl;
  Usage<<"    Mandatory option:"<<endl;
  Usage<<"         parameter file name (type: *.source)"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -i:   start interactive mode"<<endl;
  Usage<<"         -c:   convert to MEGAlib (file name)"<<endl;
  Usage<<"         -s:   seed (integer > 0)"<<endl;
  Usage<<"         -m:   macro file name (type: *.mac)"<<endl;
  Usage<<"         -u:   do not gzip *.sim files (default is to gzip them)"<<endl;
  Usage<<"         -z:   Not used: gzip *.sim files (already default, use -u to not zip them)"<<endl;
  //Usage<<"         -p:   parallel ID (used by mcosima)"<<endl;
  //Usage<<"         -f:   incarnation ID (used by mcosima)"<<endl;
  //Usage<<"         -t:   unique tag ID (used by mcosima)"<<endl;
  Usage<<"         -v:   verbosity (0: Basic Geant, 1: Standard, 2: Debug, 3: All Geant)"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;


  int i = 0;
  int RestrictedRun = -1;
  string Option, SubOption;

  // Check for help and verbosity
  for (i = 1; i < argc; ++i) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-v" || Option == "-p") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        mout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return 2;
      }
    } 

    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return 1;
    } else if (Option == "-v") {
      m_Verbosity = atoi(argv[++i]);
      if (m_Verbosity > 5) {
        mout<<"Maximum verbosity is 5!"<<endl;
        m_Verbosity = 5;
      }
      mout<<"Using verbosity "<<m_Verbosity<<endl;
      if (m_Verbosity < 0) m_Verbosity = 0;
    }
  }

  if (m_Verbosity == 0) {
    mout.Enable(false);
    mlog.Enable(false);
    mgui.Enable(false);
    mdebug.Enable(false);
  } else if (m_Verbosity >= 2) {
    if (g_Verbosity < 2) g_Verbosity = 2;
    m_UseDebug = true;
    mout<<"Enabling debug mode!"<<endl;
  }

  // Now parse the command line options:
  for (i = 1; i < argc; ++i) {
    Option = argv[i];

    // Check for last option:
    if (Option[0] != '-' && i+1 == argc) {
      m_ParameterFileName = Option;
      mout<<"Using parameter file "<<m_ParameterFileName<<endl;
      continue;
    }

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-m" || Option == "-s" || Option == "-c") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        mout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return 2;
      }
    } 


    // Then fulfill the options:
    if (Option == "-h") {
      // Already handled
    } else if (Option == "-c") {
      m_ConvertFileName = argv[++i];
    } else if (Option == "-m") {
      m_MacroFileName = argv[++i];
      mout<<"Accepting macro file name: "<<m_MacroFileName<<endl;
      m_Interactive = true;
      m_HasMacro = true;
    } else if (Option == "-z") {
      m_Zip = true;
      mout<<"All simulation files are going to be gzip'ed!"<<endl;
    } else if (Option == "-u") {
      m_Zip = false;
      mout<<"Simulation files are NOT going to be gzip'ed!"<<endl;
    } else if (Option == "-s") {
      m_Seed = atol(argv[++i]);
      if (m_Seed <= 0) {
        mout<<"Error: The seed must be larger than zero."<<endl;
        return 2;
      }
      mout<<"Setting the seed to "<<m_Seed<<endl;
    } else if (Option == "-r") {
      RestrictedRun = atoi(argv[++i]);
      mout<<"Restricting to run "<<RestrictedRun<<endl;
    } else if (Option == "-p") {
      m_ParallelID = atoi(argv[++i]);
      mout<<"Setting the ID for parallel simulations to "<<m_ParallelID<<endl;
    } else if (Option == "-t") {
      // Ignore the tag, it is only used to identify the remotely started processes
      ++i;
    } else if (Option == "-f") {
      m_IncarnationID = atoi(argv[++i]);
      mout<<"Setting the ID of the simulation file incarnation to "<<m_IncarnationID<<endl;
    } else if (Option == "-i") {
      mout<<"Starting interactive mode!"<<endl;
      m_Interactive = true;
    } else if (Option == "-v") {
      ++i;
      // Already handled...
    } else {
      mout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return 2;
    }
  } // command line options

  // Error checking:
  if (m_ParameterFileName == "") {
    m_ParameterFileName = g_MEGAlibPath + "/resource/examples/cosima/source/Run.source";
    mout<<"Error: No parameter file name given!"<<endl;
    mout<<"--> Assuming we are in test condition..."<<endl;
    mout<<"--> Using default parameter file from "<<m_ParameterFileName
        <<"    and enabling debugging"<<endl;
    m_Seed = 0;
  } 

  if (m_RunParameters.Open(m_ParameterFileName) == false) {
    mout<<"Error: Unable to parse parameter file \""<<m_ParameterFileName<<"\" correctly!"<<endl;
    return 2;
  }
  
  
  if (RestrictedRun >= 0) {
    m_RunParameters.RestrictToRun(RestrictedRun);
  }

  // Set the initial seed - Geant4 & ROOT !
  CLHEP::HepRandom::setTheSeed(m_Seed);
  gRandom->SetSeed(m_Seed);

  return 0;
}


/******************************************************************************
 * Interrupt the run at the end of the next event
 */
bool MCMain::Interrupt()
{
  if (m_RunManager != 0) {
    ((MCEventAction*) m_RunManager->GetUserEventAction())->Interrupt();
    return true;
  } else {
    return false;
  }
}


/*
 * MCMain.cc: the end...
 ******************************************************************************/
