/*
 * MCEventAction.cxx
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
#include "MStreams.h"
#include "MCCommon.hh"
#include "MCEventAction.hh"
#include "MCRunManager.hh"
#include "MCDetectorConstruction.hh"
#include "MCSteppingAction.hh"
#include "MCRun.hh"

// MEGAlib:
#include "MAssert.h"
#include "MFile.h"
#include "MDDetector.h"
#include "MSimEvent.h"
#include "MSimIA.h"
#include "MTime.h"
#include "MDGeometry.h"

// Geant4:
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "Randomize.hh"

// Standard lib:
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;

// Root:
#include "TSystem.h"

/******************************************************************************/

const int MCEventAction::c_InvalidCollID = -987;

/******************************************************************************
 * Default constructor
 */
MCEventAction::MCEventAction(MCParameterFile& RunParameters, const bool Zip, 
                             const long Seed) : 
  m_RunParameters(RunParameters),
  m_Zip(Zip)
{
  m_OutFile = 0;
  m_OutFileName = "";

  m_StoreSimulationInfo = RunParameters.StoreSimulationInfo();
  m_StoreSimulationInfoVersion = RunParameters.StoreSimulationInfoVersion();
  m_StoreCalibrated = RunParameters.StoreCalibrated();
  m_StoreOnlyTriggeredEvents = RunParameters.StoreOnlyTriggeredEvents();
  m_StoreOneHitPerEvent = RunParameters.StoreOneHitPerEvent();

  if (RunParameters.StoreScientific() == true) {
    m_StoreScientificPrecision = RunParameters.StoreScientificPrecision();
  } else {
    m_StoreScientificPrecision = 0;
  }

  m_Seed = Seed;

  m_TotalTime = 0;
  m_TimerStarted = false;

  m_Interrupt = false;

  m_Event = new MSimEvent();

  m_SaveEvents = false;
  m_TransmitEvents = false;
}


/******************************************************************************
 * Default destructor - only closes all open files
 */
MCEventAction::~MCEventAction()
{
  delete m_Event;

  if (m_SaveEvents == true) {
    if (m_OutFile != 0) {
      m_OutFile->close();
    }
    m_OutFile = 0;
    GzipSimFile();
  }
}


/******************************************************************************
 * Open all files and reset the instance
 */
bool MCEventAction::NextRun()
{
  Reset();

  m_Event->SetGeometry(MCRunManager::GetMCRunManager()->GetDetectorConstruction()->GetGeometry());

  m_TriggerUnit = MCRunManager::GetMCRunManager()->GetDetectorConstruction()->GetGeometry()->GetTriggerUnit();
  massert(m_TriggerUnit);

  // Open the file output
  G4String BaseName = m_RunParameters.GetCurrentRun().GetFileName().Data();
  G4String Name = BaseName; 

  if (Name != "") {
    m_SaveEvents = true;
    
    if (m_OutFile != 0) {
      m_OutFile->close();
      m_OutFile = 0;
      GzipSimFile();
    }
    m_OutFile = new fstream();
    m_FileNumber = 1;
    m_Incarnation = m_RunParameters.GetCurrentRun().GetIncarnationID();
    m_ParallelID = m_RunParameters.GetCurrentRun().GetParallelID();

    ostringstream FileName;
    if (m_ParallelID == 0) {
      FileName<<BaseName<<".inc"<<m_Incarnation<<".id1.sim";
    } else {
      FileName<<BaseName<<".p"<<m_ParallelID<<".inc"<<m_Incarnation<<".id1.sim";
    }
    m_OutFileName = FileName.str().c_str();

    m_OutFile->open(m_OutFileName.c_str(), ios_base::out);

    if (m_OutFile->is_open() == false) {
      mout<<"Can't open file!"<<endl;
      return false;
    }

    WriteFileHeader(0.0);
  } else {
    m_SaveEvents = false;
  }
  
  // Open the TCP/IP putput 
  if (m_RunParameters.GetCurrentRun().GetTcpIpHostName() != "") {
    m_TransmitEvents = true;

    if (m_Transceiver.IsConnected() == true) {
      if (m_Transceiver.Disconnect(true, 60.0) == false) {
        // Transceiver gives alreday warning message
        return false;
      }
    }
  
    m_Transceiver.SetName("Cosima");
    m_Transceiver.SetHost(m_RunParameters.GetCurrentRun().GetTcpIpHostName());
    m_Transceiver.SetPort(m_RunParameters.GetCurrentRun().GetTcpIpPort());

    if (m_Transceiver.Connect(true, 60) == false) {
      return false;
    }
  } else {
    m_TransmitEvents = false;
  }


  // Some final initializations
  m_TotalTime = 0;
  m_TimerStarted = false;

  m_Interrupt = false;

  return true;
}


/******************************************************************************
 * Write the file header of the sim file
 */
bool MCEventAction::WriteFileHeader(double SimulationStartTime)
{
  (*m_OutFile)<<"# Simulation data file"<<endl;
  (*m_OutFile)<<"# Created by Cosima and "<<G4RunManager::GetRunManager()->GetVersionString()<<endl;
  (*m_OutFile)<<endl;
  (*m_OutFile)<<"Type       SIM"<<endl;
  (*m_OutFile)<<"Version    "<<m_StoreSimulationInfoVersion<<endl;
  (*m_OutFile)<<"Geometry   "<<m_RunParameters.GetGeometryFileName()<<endl;
  (*m_OutFile)<<endl;
  MTime Now;
  (*m_OutFile)<<"Date       "<<Now.GetSQLString()<<endl;
  (*m_OutFile)<<"MEGAlib    "<<g_VersionString<<endl;
  (*m_OutFile)<<endl;
  (*m_OutFile)<<"Seed       "<<m_Seed<<endl;
  (*m_OutFile)<<endl;
//   if (m_RunParameters.GetCurrentRun().GetNSources() == 1) {
//     (*m_OutFile)<<"FX "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetFlux()<<endl; 
//   } else {
//     (*m_OutFile)<<"# The FX keyword only appears when you have only one source"<<endl;
//   }
  bool ValidStartArea = true;
  for (unsigned int s = 0; s < m_RunParameters.GetCurrentRun().GetNSources(); ++s) {
    if (m_RunParameters.GetCurrentRun().GetSource(s)->GetCoordinateSystem() != MCSource::c_FarField) {
      ValidStartArea = false;
    }
  }
  if (ValidStartArea == true && m_RunParameters.GetCurrentRun().GetNSources() == 1) {
    (*m_OutFile)<<"SimulationStartAreaFarField "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetStartAreaAverageArea()/cm/cm<<endl; 
  } else {
    (*m_OutFile)<<"# The SimulationStartAreaFarField keyword is only meaningfull if you have only far field sources using a surrounding sphere"<<endl;
    (*m_OutFile)<<"SimulationStartAreaFarField 0.0"<<endl;
  }
  if (m_RunParameters.GetCurrentRun().GetNSources() > 1) {
    (*m_OutFile)<<"BeamType Multiple"<<endl;
    (*m_OutFile)<<"SpectralType Multiple"<<endl;
  } else {
    (*m_OutFile)<<"BeamType "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetBeamTypeAsString()<<endl;
    (*m_OutFile)<<"SpectralType "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetSpectralTypeAsString()<<endl;
  }
  
  (*m_OutFile)<<endl;
  (*m_OutFile)<<"TB "<<SimulationStartTime<<endl; 
  (*m_OutFile)<<endl; 

  return true;
}
  

/******************************************************************************
 * Set the collection ID of the hits
 */
void MCEventAction::SetCollectionName(G4String Name, int Type)
{
  if (Type == MDDetector::c_Strip2D) {
    m_2DStripCollNames.push_back(Name);
  } else if (Type == MDDetector::c_Calorimeter) {
    m_CalorimeterCollNames.push_back(Name);
  } else if (Type == MDDetector::c_Strip3D) {
    m_3DStripCollNames.push_back(Name);
  } else if (Type == MDDetector::c_Voxel3D) {
    m_Voxel3DCollNames.push_back(Name);
  } else if (Type == MDDetector::c_ACS) {
    m_ScintillatorCollNames.push_back(Name);
  } else if (Type == MDDetector::c_DriftChamber) {
    m_DriftChamberCollNames.push_back(Name);
  } else if (Type == MDDetector::c_AngerCamera) {
    m_AngerCameraCollNames.push_back(Name);
  } else {
    merr<<"Unknown detector type: "<<Type<<endl;
  }
}


/******************************************************************************
 * Pack the simulation file in gzip format and close it
 */
bool MCEventAction::GzipSimFile()
{
  if (m_OutFile == 0 && m_Zip == true && m_OutFileName != "") {
    mout<<"Zipping simulation file... please stand by..."<<endl;
    string Cmd = "gzip ";
    Cmd += m_OutFileName;
    Cmd += " &";
    system(Cmd.c_str());
    // The system call always returns 0
  }

  return true;
}


/******************************************************************************
 * Things to do before the start of a new event (reset last event data)
 */
void MCEventAction::BeginOfEventAction(const G4Event* Event)
{
  mdebug<<"Starting event "<<Event->GetEventID();
  mdebug<< "... Please stand by..."<<endl;

  if (m_TimerStarted == false) {
    m_Timer.Start();
    m_TimerStarted = true;
  }
}


/******************************************************************************
 * Reset all per event data
 */
void MCEventAction::Reset()
{
  m_PassiveMaterialMap.clear();

  // Deletes all passed pointers!
  m_Event->Reset();
}


/******************************************************************************
 * Add an IA information to the current event
 */
void MCEventAction::AddIA(G4String ProcessID, 
                          int InteractionID,
                          int OriginID,
                          int DetectorID,
                          double Time,
                          G4ThreeVector Pos,
                          int MotherParticleID,
                          G4ThreeVector NewDir,
                          G4ThreeVector NewPol,
                          double NewKin,
                          int SecondaryParticleID,
                          G4ThreeVector SecDir,
                          G4ThreeVector SecPol,
                          double SecKin)
{
  MSimIA* IA = new MSimIA();
  IA->SetProcess(ProcessID);
  IA->SetDetectorType(DetectorID);
  IA->SetID(InteractionID);
  IA->SetOriginID(OriginID);
  IA->SetTime(Time);
  IA->SetPosition(MVector(Pos[0]/cm, Pos[1]/cm, Pos[2]/cm));
  IA->SetMotherParticleID(MotherParticleID);
  IA->SetMotherDirection(MVector(NewDir[0], NewDir[1], NewDir[2]));
  IA->SetMotherPolarisation(MVector(NewPol[0], NewPol[1], NewPol[2]));
  IA->SetMotherEnergy(NewKin/keV);
  IA->SetSecondaryParticleID(SecondaryParticleID);
  IA->SetSecondaryDirection(MVector(SecDir[0], SecDir[1], SecDir[2]));
  IA->SetSecondaryPolarisation(MVector(SecPol[0], SecPol[1], SecPol[2]));
  IA->SetSecondaryEnergy(SecKin/keV);

  // The event takes over and deletes the IA
  m_Event->AddIA(IA);
}


/******************************************************************************
 * Add a deposit in passive material to the event data
 */
void MCEventAction::AddDepositPassiveMaterial(double Energy, 
                                              string MaterialName)
{
  m_PassiveMaterialMap[MaterialName] += Energy;
}


/******************************************************************************
 * Test for trigger and store all events
 */
void MCEventAction::EndOfEventAction(const G4Event* Event)
{
  // Let's store the events...
  G4int h;
  G4String Text;
  
  MCRun& Run = m_RunParameters.GetCurrentRun();

  // make a list of all collections:
  G4SDManager* SDMan = G4SDManager::GetSDMpointer();
  
  vector<MC2DStripHitsCollection*> TwoDStripColl;
  vector<MCCalorBarHitsCollection*> CalorimeterColl;
  vector<MC2DStripHitsCollection*> ThreeDStripColl;
  vector<MCVoxel3DHitsCollection*> Voxel3DColl;
  vector<MCScintillatorHitsCollection*> ScintillatorColl;
  vector<MCDriftChamberHitsCollection*> DriftChamberColl;
  vector<MCAngerCameraHitsCollection*> AngerCameraColl;

  int ID = 0;
  for (unsigned int i = 0; i < m_2DStripCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_2DStripCollNames[i]);
    TwoDStripColl.push_back((MC2DStripHitsCollection*) Event
                            ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_CalorimeterCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_CalorimeterCollNames[i]);
    CalorimeterColl.push_back((MCCalorBarHitsCollection*) Event
                                ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_3DStripCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_3DStripCollNames[i]);
    ThreeDStripColl.push_back((MC2DStripHitsCollection*) Event
                            ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_Voxel3DCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_Voxel3DCollNames[i]);
    Voxel3DColl.push_back((MCVoxel3DHitsCollection*) Event
                            ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_ScintillatorCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_ScintillatorCollNames[i]);
    ScintillatorColl.push_back((MCScintillatorHitsCollection*) Event
                                 ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_DriftChamberCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_DriftChamberCollNames[i]);
    DriftChamberColl.push_back((MCDriftChamberHitsCollection*) Event
                                 ->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_AngerCameraCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_AngerCameraCollNames[i]);
    AngerCameraColl.push_back((MCAngerCameraHitsCollection*) Event
                              ->GetHCofThisEvent()->GetHC(ID));
  }


  // Sum up the total energy deposit in sensitive detectors:
  double SensitiveEnergy = 0;
  for (unsigned int i = 0; i < TwoDStripColl.size(); ++i) {
    for (h = 0; h < TwoDStripColl[i]->entries(); ++h) {
      SensitiveEnergy += (*TwoDStripColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < CalorimeterColl.size(); ++i) {
    for (h = 0; h < CalorimeterColl[i]->entries(); ++h) {
      SensitiveEnergy += (*CalorimeterColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < ThreeDStripColl.size(); ++i) {
    for (h = 0; h < ThreeDStripColl[i]->entries(); ++h) {
      SensitiveEnergy += (*ThreeDStripColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < Voxel3DColl.size(); ++i) {
    for (h = 0; h < Voxel3DColl[i]->entries(); ++h) {
      SensitiveEnergy += (*Voxel3DColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < ScintillatorColl.size(); ++i) {
    for (h = 0; h < ScintillatorColl[i]->entries(); ++h) {
      SensitiveEnergy += (*ScintillatorColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < DriftChamberColl.size(); ++i) {
    for (h = 0; h < DriftChamberColl[i]->entries(); ++h) {
      SensitiveEnergy += (*DriftChamberColl[i])[h]->GetEnergy();
    }
  }
  for (unsigned int i = 0; i < AngerCameraColl.size(); ++i) {
    for (h = 0; h < AngerCameraColl[i]->entries(); ++h) {
      SensitiveEnergy += (*AngerCameraColl[i])[h]->GetEnergy();
    }
  }

  mdebug<<"Energy deposits in sensitive material: "<<SensitiveEnergy<<endl;

  // Section: test the trigger conditions:
  bool HasTriggered = false;
  if (m_TriggerUnit != 0) {
    m_TriggerUnit->Reset();
    
    for (unsigned int i = 0; i < TwoDStripColl.size(); ++i) {
      for (h = 0; h < TwoDStripColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*TwoDStripColl[i])[h]->GetPosition().getX(), (*TwoDStripColl[i])[h]->GetPosition().getY(), (*TwoDStripColl[i])[h]->GetPosition().getZ())/cm, (*TwoDStripColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < CalorimeterColl.size(); ++i) {
      for (h = 0; h < CalorimeterColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*CalorimeterColl[i])[h]->GetPosition().getX(), (*CalorimeterColl[i])[h]->GetPosition().getY(), (*CalorimeterColl[i])[h]->GetPosition().getZ())/cm, (*CalorimeterColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < ThreeDStripColl.size(); ++i) {
      for (h = 0; h < ThreeDStripColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*ThreeDStripColl[i])[h]->GetPosition().getX(), (*ThreeDStripColl[i])[h]->GetPosition().getY(), (*ThreeDStripColl[i])[h]->GetPosition().getZ())/cm, (*ThreeDStripColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < Voxel3DColl.size(); ++i) {
      for (h = 0; h < Voxel3DColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*Voxel3DColl[i])[h]->GetPosition().getX(), (*Voxel3DColl[i])[h]->GetPosition().getY(), (*Voxel3DColl[i])[h]->GetPosition().getZ())/cm, (*Voxel3DColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < ScintillatorColl.size(); ++i) {
      for (h = 0; h < ScintillatorColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*ScintillatorColl[i])[h]->GetPosition().getX(), (*ScintillatorColl[i])[h]->GetPosition().getY(), (*ScintillatorColl[i])[h]->GetPosition().getZ())/cm, (*ScintillatorColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < DriftChamberColl.size(); ++i) {
      for (h = 0; h < DriftChamberColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*DriftChamberColl[i])[h]->GetPosition().getX(), (*DriftChamberColl[i])[h]->GetPosition().getY(), (*DriftChamberColl[i])[h]->GetPosition().getZ())/cm, (*DriftChamberColl[i])[h]->GetEnergy()/keV);
      }
    }
    for (unsigned int i = 0; i < AngerCameraColl.size(); ++i) {
      for (h = 0; h < AngerCameraColl[i]->entries(); ++h) {
        m_TriggerUnit->AddHit(MVector((*AngerCameraColl[i])[h]->GetPosition().getX(), (*AngerCameraColl[i])[h]->GetPosition().getY(), (*AngerCameraColl[i])[h]->GetPosition().getZ())/cm, (*AngerCameraColl[i])[h]->GetEnergy()/keV);
      }
    }

    if (m_TriggerUnit->HasTriggered() == true) {
      HasTriggered = true;
    }
  }


  if (HasTriggered == false && m_StoreOnlyTriggeredEvents == true) {
    mdebug<<"Event has not triggered!"<<endl;
  } else {
    for (unsigned int i = 0; i < TwoDStripColl.size(); ++i) {
      mdebug<<"Writing 2D strip hits..."<<endl;
      TwoDStripColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < CalorimeterColl.size(); ++i) {
      mdebug<<"Writing calorimeter hits..."<<endl;
      CalorimeterColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < ThreeDStripColl.size(); ++i) {
      mdebug<<"Writing 3D strip hits..."<<endl;
      ThreeDStripColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < Voxel3DColl.size(); ++i) {
      mdebug<<"Writing voxel 3D hits..."<<endl;
      Voxel3DColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < ScintillatorColl.size(); ++i) {
      mdebug<<"Writing scintillator hits..."<<endl;
      ScintillatorColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < DriftChamberColl.size(); ++i) {
      mdebug<<"Writing scintillator hits..."<<endl;
      DriftChamberColl[i]->PrintAllHits();
    }
    for (unsigned int i = 0; i < AngerCameraColl.size(); ++i) {
      mdebug<<"Writing voxel 3D hits..."<<endl;
      AngerCameraColl[i]->PrintAllHits();
    }

    // Section: Store events:
    Run.AddTriggeredEvent();

    m_Event->SetID(Run.GetNTriggeredEvents());
    m_Event->SetSimulationEventID(Event->GetEventID()+1);
    m_Event->SetTime(Run.GetSimulatedTime()/s);

    map<string, double>::iterator Iter; 
    for (Iter = m_PassiveMaterialMap.begin(); 
         Iter != m_PassiveMaterialMap.end(); 
         ++Iter) {
      if (Iter->second > 1*eV) {
        MSimPM* PM = new MSimPM(Iter->first, Iter->second/keV);
        m_Event->AddPM(PM);
      }
    }

    mout<<"Storing event "<<Run.GetNTriggeredEvents()
        <<" of "<<Event->GetEventID()+1<<" at t_obs="<<Run.GetSimulatedTime()/s<<"s ... Please stand by... "<<flush;

    // (b) Store the positions and energies or store Strips and counts...
    if (m_StoreCalibrated == true) {
      for (unsigned int i = 0; i < TwoDStripColl.size(); ++i) {
        for (h = 0; h < TwoDStripColl[i]->entries(); ++h) {
          m_Event->AddGR((*TwoDStripColl[i])[h]->GetGuardringCalibrated());
          m_Event->AddHT((*TwoDStripColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < CalorimeterColl.size(); ++i) {
        for (h = 0; h < CalorimeterColl[i]->entries(); ++h) {
          m_Event->AddHT((*CalorimeterColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < ThreeDStripColl.size(); ++i) {
        for (h = 0; h < ThreeDStripColl[i]->entries(); ++h) {
          m_Event->AddGR((*ThreeDStripColl[i])[h]->GetGuardringCalibrated());
          m_Event->AddHT((*ThreeDStripColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < Voxel3DColl.size(); ++i) {
        for (h = 0; h < Voxel3DColl[i]->entries(); ++h) {
          m_Event->AddGR((*Voxel3DColl[i])[h]->GetGuardringCalibrated());
          m_Event->AddHT((*Voxel3DColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < ScintillatorColl.size(); ++i) {
        for (h = 0; h < ScintillatorColl[i]->entries(); ++h) {
          m_Event->AddHT((*ScintillatorColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < DriftChamberColl.size(); ++i) {
        for (h = 0; h < DriftChamberColl[i]->entries(); ++h) {
          m_Event->AddGR((*DriftChamberColl[i])[h]->GetGuardringCalibrated());
          m_Event->AddHT((*DriftChamberColl[i])[h]->GetCalibrated());
        }
      }
      for (unsigned int i = 0; i < AngerCameraColl.size(); ++i) {
        for (h = 0; h < AngerCameraColl[i]->entries(); ++h) {
          m_Event->AddHT((*AngerCameraColl[i])[h]->GetCalibrated());
        }
      }
    } else {
      mout<<"Storing uncalibrated data is no longer supported..."<<endl;
    }
    
    mout<<"Done"<<endl;

    if (m_StoreOneHitPerEvent == true && m_Event->GetNHTs() > 1) {
      vector<MSimEvent*> E = m_Event->CreateSingleHitEvents();
      for (unsigned int e = 0; e < E.size(); ++e) {
        if (e > 0) {
          Run.AddTriggeredEvent();
          E[e]->SetID(Run.GetNTriggeredEvents());
        }

        // Save and transmit know if we should do it
        SaveEventToFile(E[e]);
        TransmitEvent(E[e]);
        
        delete E[e];
      }
    } else {
      // Save and transmit know if we should do it
      SaveEventToFile(m_Event);
      TransmitEvent(m_Event);
    }

    // Check if we are reaching the maximum file size: 
    if (m_SaveEvents == true) {
      streampos Length;
      Length = m_OutFile->tellp(); // position at end is length
      if (Length > 0.95*numeric_limits<streamsize>::max()) {
        mout<<"Current file length ("<<Length
            <<") exceeds maximum file length ("<<0.95*numeric_limits<streamsize>::max()<<")!"<<endl;

        ostringstream FileName;
        if (m_ParallelID == 0) {
          FileName<<Run.GetFileName()<<".inc"<<m_Incarnation<<".id"<<(++m_FileNumber)<<".sim";
        } else {
          FileName<<Run.GetFileName()<<".p"<<m_ParallelID<<".inc"<<m_Incarnation<<".id"<<(++m_FileNumber)<<".sim";
        }

        mout<<"Opening new file: "<<FileName.str().c_str()<<endl;
      
        (*m_OutFile)<<endl; 
        (*m_OutFile)<<"NF "<<FileName.str()<<endl;
        (*m_OutFile)<<endl; 
        (*m_OutFile)<<"EN"<<endl; 
        (*m_OutFile)<<endl; 
        (*m_OutFile)<<"TE "<<Run.GetSimulatedTime()/s<<endl; 
        (*m_OutFile)<<"TS "<<Run.GetNSimulatedEvents()<<endl; 
        m_OutFile->close();
        m_OutFile = 0;
        GzipSimFile();
        m_OutFile = new fstream();
        m_OutFile->open(FileName.str().c_str(), ios_base::out);
        m_OutFileName = FileName.str().c_str();
      
        if (m_OutFile->is_open() == false) {
          mout<<"Can't open file!"<<endl;
        }

        (*m_OutFile)<<"# Continued file..."<<endl;
        (*m_OutFile)<<endl;
        WriteFileHeader(Run.GetSimulatedTime()/s);
      }
    }
  }

  Reset();

  m_TotalTime += m_Timer.ElapsedTime();
  m_TimerStarted = false;

  if (m_Interrupt == true || Run.CheckStopConditions() == true) {
    if (m_SaveEvents == true) {
      (*m_OutFile)<<endl; 
      (*m_OutFile)<<"EN"<<endl; 
      (*m_OutFile)<<endl; 
      (*m_OutFile)<<"TE "<<Run.GetSimulatedTime()/s<<endl; 
      (*m_OutFile)<<"TS "<<Run.GetNSimulatedEvents()<<endl; 
      m_OutFile->close();
      m_OutFile = 0;
      GzipSimFile();
    }
    if (m_TransmitEvents == true) {
      if (m_Transceiver.GetNStringsToSend() > 0) {
        MTimer Wait;
        while (m_Transceiver.GetNStringsToSend() > 0 && Wait.GetElapsed() < 60) {
          gSystem->Sleep(50);
        }
      }
    }
    Run.Stop();
  }
}



/******************************************************************************
 * Save the event to file (only saves the event if we really want to)
 */
bool MCEventAction::SaveEventToFile(MSimEvent* Event)
{
  if (m_SaveEvents == true) {
    if (m_OutFile->is_open() == false) return false;
    (*m_OutFile)<<Event->ToSimString(m_StoreSimulationInfo, m_StoreScientificPrecision, m_StoreSimulationInfoVersion);
  }
  
  return true;
}


/******************************************************************************
 * Transmit event via TCP/IP (only transmit it if we have an open transceiver)
 */
bool MCEventAction::TransmitEvent(MSimEvent* Event)
{
  if (m_TransmitEvents == true) {
    m_Transceiver.Send(Event->ToSimString(m_StoreSimulationInfo, m_StoreScientificPrecision, m_StoreSimulationInfoVersion));
    if (m_Transceiver.IsConnected() == false) {
      mout<<"Tranceiver not connected, queuing event..."<<endl;
    }
  }
  return true;
}

  /*
 * MCEventAction.cc: the end...
 ******************************************************************************/

