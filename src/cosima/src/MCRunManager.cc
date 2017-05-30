/*
 * MCRunManager.cxx
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
#include "MCRunManager.hh"
#include "MCPrimaryGeneratorAction.hh"
#include "MCEventAction.hh"
#include "MCSteppingAction.hh"
#include "MCDetectorConstruction.hh"
#include "MCCrossSections.hh"

// Megalib:
#include "MStreams.h"

// Standard lib:
#include <limits>
using namespace std;

/******************************************************************************/


MCRunManager* MCRunManager::s_RunManager = 0;


/******************************************************************************
 * Accesss to the singleton class
 */
MCRunManager* MCRunManager::GetMCRunManager()
{ 
  return s_RunManager; 
}

/******************************************************************************
 * Constructor
 */
MCRunManager::MCRunManager(MCParameterFile& RunParameters) : 
  G4RunManager(), m_RunParameters(RunParameters)
{
  // Error handling if this in constructed twice is done in the abse class
  s_RunManager = this;
}


/******************************************************************************
 * Default destructor
 */
MCRunManager::~MCRunManager()
{
  // Intentionally left blank
}


/******************************************************************************
 * Do some initializations for the next run
 */
bool MCRunManager::PrepareNextRun()
{
  // Prepare for the new run
  if (GetEventAction()->NextRun() == false) {
    return false;
  }

  m_RunParameters.GetCurrentRun().Initialize();

  return true;
}


/******************************************************************************
 * Activate the beam and work on all activators and runs included in the file
 */
void MCRunManager::StartBeam()
{
  // Test if just absorption files have to be created:
  if (m_RunParameters.CreateCrossSectionFiles() == true) {
    MCCrossSections XSections;
    XSections.CreateCrossSectionFiles(m_RunParameters.GetCrossSectionFileDirectory());
    return;
  }

  for (unsigned int a = 0; a < m_RunParameters.GetNActivators(); ++a) {
    mout<<endl;
    mout<<"Starting activator \""<<m_RunParameters.GetActivator(a).GetName()<<"\":"<<endl;
    mout<<endl;

    m_RunParameters.GetActivator(a).LoadCountsFiles();
    m_RunParameters.GetActivator(a).CalculateEquilibriumRates();
    m_RunParameters.GetActivator(a).SaveOutputFile();
  }

  for (unsigned int r = 0; r < m_RunParameters.GetNRuns(); ++r) {
    mout<<endl;
    mout<<"Starting run \""<<m_RunParameters.GetCurrentRun().GetName()<<"\":"<<endl;
    mout<<endl;

    if (PrepareNextRun() == false) {
      break;
    }

    // This takes care of the initilizations
    G4RunManager::BeamOn(0);

    MTimer RunTimer;
    RunTimer.Start();

    // Execute the new run:
    while (true) {
      G4RunManager::BeamOn(2000000000);
      if (m_RunParameters.GetCurrentRun().CheckStopConditions() == true) {
        break;
      }
      if (GetEventAction()->GetInterrupt() == true) {
        break; 
      }
      mout<<endl;
      mout<<"Restarting Geant4 since we almost reached the maximum number of events Geant4 can process..."<<endl;
      mout<<endl;
    }
    RunTimer.Pause();

    m_RunParameters.GetCurrentRun().DumpRunStatistics(RunTimer.GetElapsed());
    m_RunParameters.GetCurrentRun().SaveIsotopeStore();

    if (m_RunParameters.GetCurrentRun().CheckStopConditions() == false) {
      cout<<endl;
      cout<<"!!!!!!"<<endl;
      cout<<" Run "<<m_RunParameters.GetCurrentRun().GetName()<<" ended prematurely, i.e. without reaching its stop condition!"<<endl;
      cout<<"!!!!!!"<<endl;
      cout<<endl;
      cout<<endl;
    }

    // Switch to next run:
    m_RunParameters.NextRun();
  }
}


/******************************************************************************
 * Abort all the action, store the generated radioactive particles, etc.
 */
void MCRunManager::AbortRun(G4bool softAbort) 
{
  G4RunManager::AbortRun(softAbort);
}


/******************************************************************************
 * Return a list of the runs
 */
vector<MCRun>& MCRunManager::GetRuns()
{
  return m_RunParameters.GetRunList();
}


/******************************************************************************
 * Return the event generator
 */
MCPrimaryGeneratorAction* MCRunManager::GetPrimaryGeneratorAction()
{
  return (MCPrimaryGeneratorAction*) GetUserPrimaryGeneratorAction();
}


/******************************************************************************
 * Return the stepping action
 */
MCSteppingAction* MCRunManager::GetSteppingAction()
{
  return (MCSteppingAction*) GetUserSteppingAction();
}


/******************************************************************************
 * Return the tracking action
 */
MCTrackingAction* MCRunManager::GetTrackingAction()
{
  return (MCTrackingAction*) GetUserTrackingAction();
}


/******************************************************************************
 * Return the event action
 */
MCEventAction* MCRunManager::GetEventAction()
{
  return (MCEventAction*) GetUserEventAction();
}


/******************************************************************************
 * Return the physics list
 */
MCPhysicsList* MCRunManager::GetPhysicsList()
{
  return (MCPhysicsList*) GetUserPhysicsList();
}


/******************************************************************************
 * Return the detector constructor
 */
MCDetectorConstruction* MCRunManager::GetDetectorConstruction()
{
  return (MCDetectorConstruction*) GetUserDetectorConstruction();
}


/*
 * MCRunManager.cc: the end...
 ******************************************************************************/
