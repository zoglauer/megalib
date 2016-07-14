/*
 * MResponseEarthHorizon.cxx
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
// MResponseEarthHorizon
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEarthHorizon.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseEarthHorizon)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseEarthHorizon::MResponseEarthHorizon()
{
  // Construct an instance of MResponseEarthHorizon
}


////////////////////////////////////////////////////////////////////////////////


MResponseEarthHorizon::~MResponseEarthHorizon()
{
  // Delete this instance of MResponseEarthHorizon
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseEarthHorizon::OpenFiles()
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


bool MResponseEarthHorizon::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenFiles() == false) return false;

  cout<<"Generating earth horizon pdf"<<endl;

  const double MaxESigma = 100;
  const double MaxARM = 5;

  // Earth horizon cut:
  vector<float> AxisEHCEpsilonDiff;
  AxisEHCEpsilonDiff = CreateEquiDist(0, 180, 36);

  vector<float> AxisEHCElectronEnergy;
  AxisEHCElectronEnergy = CreateLogDist(100, 5000, 20, 1);

  vector<float> Add = CreateLogDist(7000, 100000, 4); 
  for (unsigned int i = 0; i < Add.size(); ++i) {
    AxisEHCElectronEnergy.push_back(Add[i]);
  }

  MResponseMatrixO2 EHC("EHC", AxisEHCEpsilonDiff, AxisEHCElectronEnergy);
  EHC.SetAxisNames("SPD [#circ]", "Measured electron energy [keV]");

  double Spd;
  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();

    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
          if (Event->GetType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;
            //Compton->Calculate();
            if (Compton->IsKinematicsOK() == false) continue;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = m_SiEvent->GetIAAt(0)->GetPosition();
              
              // Only take completely absorbed events within ARM
              if (fabs(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy() - Compton->Ei()) < MaxESigma*Compton->dEi() &&
                  Compton->GetARMGamma(IdealOriginDir)*c_Deg < MaxARM) {
                // Epsilon/EHC response:
                if (Compton->HasTrack() == true) {
                  Spd = Compton->GetSPDElectron(IdealOriginDir)*c_Deg;
                  EHC.Add(Spd, Compton->Ee());
                }
              }
            }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      EHC.Write(m_ResponseName + ".compton.ehc" + m_Suffix, true);
    }
  }  

  EHC.Write(m_ResponseName + ".compton.ehc" + m_Suffix, true);

  return true;
}


// MResponseEarthHorizon.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
