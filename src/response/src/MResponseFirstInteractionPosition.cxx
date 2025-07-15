/*
 * MResponseFirstInteractionPosition.cxx
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
// MResponseFirstInteractionPosition
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseFirstInteractionPosition.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseFirstInteractionPosition)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseFirstInteractionPosition::MResponseFirstInteractionPosition()
{
  // Construct an instance of MResponseFirstInteractionPosition
}


////////////////////////////////////////////////////////////////////////////////


MResponseFirstInteractionPosition::~MResponseFirstInteractionPosition()
{
  // Delete this instance of MResponseFirstInteractionPosition
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseFirstInteractionPosition::OpenFiles()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;

  MSettingsRevan RevanCfg(false);
  RevanCfg.Read(m_RevanCfgFileName);
  m_ReReader->SetSettings(&RevanCfg);

  MSettingsMimrec MimrecCfg(false);
  MimrecCfg.Read(m_MimrecCfgFileName);
  m_MimrecEventSelector.SetSettings(&MimrecCfg);

  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseFirstInteractionPosition::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenFiles() == false) return false;

  cout<<"Generating first interaction position pdf"<<endl;


  vector<float> AxisFirstInteractionDistance;
  AxisFirstInteractionDistance = CreateEquiDist(-0.3, 0.3, 60);

  vector<float> EnergyBins;
  EnergyBins.push_back(0);
  //EnergyBins.push_back(1000);
  EnergyBins.push_back(2000);
  EnergyBins.push_back(5000);

  MResponseMatrixO3 InteractionDistance("InteractionDistance", AxisFirstInteractionDistance, AxisFirstInteractionDistance, EnergyBins);
  InteractionDistance.SetAxisNames("Distance x_{meas} - x_{real} [cm]", 
                                   "Distance y_{meas} - y_{real} [cm]",
                                   "Energy electron [keV]");

  MResponseMatrixO3 ElectronDirection("ElectronDirection", AxisFirstInteractionDistance, AxisFirstInteractionDistance, EnergyBins);
  ElectronDirection.SetAxisNames("Distance [cm]", 
                                 "Distance [cm]",
                                 "Energy electron [keV]");


  MVector IdealOriginPos;

  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    MRawEventIncarnationList* REList = m_ReReader->GetRawEventList();
    

    if (REList->HasOnlyOptimumEvents() == true) {
      Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
      if (Event != nullptr) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
          if (Event->GetType() == c_ComptonEvent) {
            Compton = (MComptonEvent*) Event;

            if (Compton->IsKinematicsOK() == false) continue;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 1) {
              IdealOriginPos = m_SiEvent->GetIAAt(0)->GetPosition();
              
              InteractionDistance.Add(Compton->C1().X() - IdealOriginPos.X(), Compton->C1().Y() - IdealOriginPos.Y(), Compton->Ee());

              MVector Distance = Compton->C1() - IdealOriginPos;
              //Distance = m_SiEvent->GetIAAt(1)->GetSecondaryDirection();
              Distance.SetZ(0);
              MVector ComptonDir = Compton->C2() - Compton->C1();
              ComptonDir.SetZ(0);
              double Angle = ComptonDir.Angle(Distance);

              if (gRandom->Rndm() > 0.5) {
                ElectronDirection.Add(Distance.Mag()*sin(Angle), Distance.Mag()*cos(Angle), Compton->Ee());
              } else {
                ElectronDirection.Add(-Distance.Mag()*sin(Angle), Distance.Mag()*cos(Angle), Compton->Ee());
              }
            }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      InteractionDistance.Write(m_ResponseName + ".iadistance" + m_Suffix, true);
      ElectronDirection.Write(m_ResponseName + ".iadirection" + m_Suffix, true);
    }
  }  

  InteractionDistance.Write(m_ResponseName + ".iadistance" + m_Suffix, true);
  ElectronDirection.Write(m_ResponseName + ".iadirection" + m_Suffix, true);

  return true;
}


// MResponseFirstInteractionPosition.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
