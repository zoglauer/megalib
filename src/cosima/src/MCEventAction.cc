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
#include "G4SystemOfUnits.hh"
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
MCEventAction::MCEventAction(MCParameterFile& RunParameters, const bool Zip, const long Seed) : 
  m_RunParameters(RunParameters), m_Zip(Zip)
{
  m_OutFileName = "";

  m_StoreSimulationInfo = RunParameters.StoreSimulationInfo();
  m_StoreSimulationInfoVersion = RunParameters.StoreSimulationInfoVersion();
  m_StoreCalibrated = RunParameters.StoreCalibrated();
  m_StoreOneHitPerEvent = RunParameters.StoreOneHitPerEvent();
  m_StoreMinimumEnergy = RunParameters.StoreMinimumEnergy();
  
  m_PreTriggerMode = RunParameters.GetPreTriggerMode();
  
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
  m_RelegateEvents = false;
}


/******************************************************************************
 * Default destructor - only closes all open files
 */
MCEventAction::~MCEventAction()
{
  delete m_Event;
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
    
    if (m_OutFile.IsOpen()) {
      m_OutFile.Close();
    }
    m_FileNumber = 1;
    m_Incarnation = m_RunParameters.GetCurrentRun().GetIncarnationID();
    m_ParallelID = m_RunParameters.GetCurrentRun().GetParallelID();

    ostringstream FileName;
    if (m_ParallelID == 0) {
      FileName<<BaseName<<".inc"<<m_Incarnation<<".id1.sim";
    } else {
      FileName<<BaseName<<".p"<<m_ParallelID<<".inc"<<m_Incarnation<<".id1.sim";
    }
    if (m_Zip == true) {
      FileName<<".gz";
    }
    m_OutFileName = FileName.str().c_str();

    m_OutFile.Open(m_OutFileName, MFile::c_Write);

    if (m_OutFile.IsOpen() == false) {
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
        // Transceiver gives already warning message
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
  ostringstream Out;
  Out<<"# Simulation data file"<<endl;
  Out<<"# Created by Cosima and "<<G4RunManager::GetRunManager()->GetVersionString()<<endl;
  Out<<endl;
  Out<<"Type       SIM"<<endl;
  Out<<"Version    "<<m_StoreSimulationInfoVersion<<endl;
  Out<<"Geometry   "<<m_RunParameters.GetGeometryFileName()<<endl;
  Out<<endl;
  MTime Now;
  Out<<"Date       "<<Now.GetSQLString()<<endl;
  Out<<"MEGAlib    "<<g_VersionString<<endl;
  Out<<endl;
  Out<<"Seed       "<<m_Seed<<endl;
  Out<<endl;
//   if (m_RunParameters.GetCurrentRun().GetNSources() == 1) {
//     Out<<"FX "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetFlux()<<endl; 
//   } else {
//     Out<<"# The FX keyword only appears when you have only one source"<<endl;
//   }
  bool ValidStartArea = true;
  for (unsigned int so = 0; so < m_RunParameters.GetCurrentRun().GetNSources(); ++so) {
    if (m_RunParameters.GetCurrentRun().GetSource(so)->GetCoordinateSystem() != MCSource::c_FarField) {
      ValidStartArea = false;
    }
  }
    
  if (ValidStartArea == true) {
    Out<<"SimulationStartAreaFarField "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetStartAreaAverageArea()/cm/cm<<endl; 
  } else {
    Out<<"# The SimulationStartAreaFarField keyword is only meaningfull if you have only far field sources using a surrounding sphere"<<endl;
    Out<<"SimulationStartAreaFarField 0.0"<<endl;
  }

  bool SameBeamType = true;
  bool SameSpectralType = true;
  for (unsigned int source = 1; source < m_RunParameters.GetCurrentRun().GetNSources(); ++source) {
    if (m_RunParameters.GetCurrentRun().GetSource(source)->GetCoordinateSystem() != m_RunParameters.GetCurrentRun().GetSource(0)->GetCoordinateSystem() ||
        m_RunParameters.GetCurrentRun().GetSource(source)->GetBeamType() != m_RunParameters.GetCurrentRun().GetSource(0)->GetBeamType()) {
      SameBeamType = false;
    }
    if (m_RunParameters.GetCurrentRun().GetSource(source)->GetSpectralType() != m_RunParameters.GetCurrentRun().GetSource(0)->GetSpectralType()) {
      SameSpectralType = false;
    }
  }
  
  if (SameBeamType == true) {
    Out<<"BeamType "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetBeamAsString()<<endl;
  } else {
    Out<<"BeamType Multiple"<<endl;
  }
  
  if (SameSpectralType == true) {
    Out<<"SpectralType "<<m_RunParameters.GetCurrentRun().GetSource(0)->GetSpectralAsString()<<endl;
  } else {
    Out<<"SpectralType Multiple"<<endl;
  }
  
  Out<<endl;
  Out<<"TB "<<SimulationStartTime<<endl; 
  Out<<endl; 

  m_OutFile.Write(Out);
  
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
 * Set the function to which we relegate the events
 */
void MCEventAction::SetRelegator(void (Relegator)(MSimEvent*))
{
  m_RelegateEvents = true;
  //m_SaveEvents = false;
  //m_TransmitEvents = false;
  
  m_Relegator = Relegator;
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
 * Set the Galactic pointing --- this is in degree as is does not translate well into radians (e.g. -90 lat)
 */
void MCEventAction::SetGalacticPointing(double XLat, double XLong, double ZLat, double ZLong)
{
  m_Event->SetGalacticPointingXAxis(XLong*c_Deg, XLat*c_Deg);
  m_Event->SetGalacticPointingZAxis(ZLong*c_Deg, ZLat*c_Deg);
}


/******************************************************************************
 * Set the Detector orientation --- this is in degree for congruence reasons with SetGalacticPointing
 */
void MCEventAction::SetDetectorPointing(double XTheta, double XPhi, double ZTheta, double ZPhi)
{
  m_Event->SetDetectorPointingXAxis(XPhi*c_Deg, XTheta*c_Deg);
  m_Event->SetDetectorPointingZAxis(ZPhi*c_Deg, ZTheta*c_Deg);
 
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

  // Make a list of all collections:
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
    TwoDStripColl.push_back((MC2DStripHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_CalorimeterCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_CalorimeterCollNames[i]);
    CalorimeterColl.push_back((MCCalorBarHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_3DStripCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_3DStripCollNames[i]);
    ThreeDStripColl.push_back((MC2DStripHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_Voxel3DCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_Voxel3DCollNames[i]);
    Voxel3DColl.push_back((MCVoxel3DHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_ScintillatorCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_ScintillatorCollNames[i]);
    ScintillatorColl.push_back((MCScintillatorHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_DriftChamberCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_DriftChamberCollNames[i]);
    DriftChamberColl.push_back((MCDriftChamberHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
  }
  for (unsigned int i = 0; i < m_AngerCameraCollNames.size(); ++i) {
    ID = SDMan->GetCollectionID(m_AngerCameraCollNames[i]);
    AngerCameraColl.push_back((MCAngerCameraHitsCollection*) Event->GetHCofThisEvent()->GetHC(ID));
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

  bool StoreEvent = false;
  if (m_PreTriggerMode == MCParameterFile::c_PreTriggerEverything) {
    StoreEvent = true;
  } else if (m_PreTriggerMode == MCParameterFile::c_PreTriggerEveryEventWithHits) {
    if (SensitiveEnergy > 0) {
      StoreEvent = true;
    }
  } else if (m_PreTriggerMode == MCParameterFile::c_PreTriggerFull) {
    if (HasTriggered == true) {
      StoreEvent = true;
    }
  } else {
    mout<<"ERROR: Unknown pre-trigger mode, assuming full pretrigger."<<endl;
    if (HasTriggered == true) {
      StoreEvent = true;
    }
  }
    
    
  if (StoreEvent == true) {
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


    if (m_StoreOneHitPerEvent == true && m_Event->GetNHTs() > 1) {
      vector<MSimEvent*> E = m_Event->CreateSingleHitEvents();
      for (unsigned int e = 0; e < E.size(); ++e) {
        if (e > 0) {
          Run.AddTriggeredEvent();
          E[e]->SetID(Run.GetNTriggeredEvents());
        }

        if (E[e]->GetTotalEnergyDepositBeforeNoising() > m_StoreMinimumEnergy) {
          // Save and transmit know if we should do it
          SaveEventToFile(E[e]);
          TransmitEvent(E[e]);
          if (m_RelegateEvents == true) {
            m_Relegator(E[e]); 
          }
        }
        
        delete E[e];
      }
    } else {
      // Save and transmit know if we should do it
      if (m_Event->GetTotalEnergyDepositBeforeNoising() > m_StoreMinimumEnergy) {
        mout<<"Storing event "<<Run.GetNTriggeredEvents()<<" of "<<Event->GetEventID()+1<<" at t_obs="<<Run.GetSimulatedTime()/s<<"s"<<endl;
        SaveEventToFile(m_Event);
        TransmitEvent(m_Event);
        if (m_RelegateEvents == true) {
          m_Relegator(m_Event); 
        }
      }
    }

    // Check if we are reaching the maximum file size: 
    if (m_SaveEvents == true) {
      streampos Length = m_OutFile.GetFileLength();
      if (Length > m_OutFile.GetMaxFileLength()) {
        mout<<"Current file length ("<<Length
            <<") exceeds maximum (safe) file length ("<<m_OutFile.GetMaxFileLength()<<")!"<<endl;

        ostringstream FileName;
        if (m_ParallelID == 0) {
          FileName<<Run.GetFileName()<<".inc"<<m_Incarnation<<".id"<<(++m_FileNumber)<<".sim";
        } else {
          FileName<<Run.GetFileName()<<".p"<<m_ParallelID<<".inc"<<m_Incarnation<<".id"<<(++m_FileNumber)<<".sim";
        }
        if (m_Zip == true) {
          FileName<<".gz";
        }

        mout<<"Opening new file: "<<FileName.str().c_str()<<endl;
      
        ostringstream Out;
        Out<<endl; 
        Out<<"NF "<<FileName.str()<<endl;
        Out<<endl; 
        Out<<"EN"<<endl; 
        Out<<endl; 
        Out<<"TE "<<Run.GetSimulatedTime()/s<<endl; 
        Out<<"TS "<<Run.GetNSimulatedEvents()<<endl; 
        m_OutFile.Write(Out);
        m_OutFile.Close();

        m_OutFile.Open(FileName.str(), MFile::c_Write);
        m_OutFileName = FileName.str().c_str();
      
        if (m_OutFile.IsOpen() == false) {
          mout<<"Can't open file!"<<endl;
        }

        m_OutFile.Write("# Continued file...\n");
        WriteFileHeader(Run.GetSimulatedTime()/s);
      }
    }
  }

  Reset();

  m_TotalTime += m_Timer.ElapsedTime();
  m_TimerStarted = false;

  if (m_Interrupt == true || Run.CheckStopConditions() == true) {
    if (m_SaveEvents == true) {
      ostringstream Out;
      Out<<endl; 
      Out<<"EN"<<endl; 
      Out<<endl; 
      Out<<"TE "<<Run.GetSimulatedTime()/s<<endl; 
      Out<<"TS "<<Run.GetNSimulatedEvents()<<endl;
      m_OutFile.Write(Out);
      m_OutFile.Close();
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
    if (m_OutFile.IsOpen() == false) return false;
    m_OutFile.Write(Event->ToSimString(m_StoreSimulationInfo, m_StoreScientificPrecision, m_StoreSimulationInfoVersion));
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

