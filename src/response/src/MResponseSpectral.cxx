/*
 * MResponseSpectral.cxx
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
// MResponseSpectral
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseSpectral.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseSpectral)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseSpectral::MResponseSpectral()
{
  // Construct an instance of MResponseSpectral
}


////////////////////////////////////////////////////////////////////////////////


MResponseSpectral::~MResponseSpectral()
{
  // Delete this instance of MResponseSpectral
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseSpectral::OpenFiles()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) {
    merr<<"Unable to open simulation file \""<<m_SimulationFileName<<"\" in revan"<<endl; 
    return false;
  }

  MSettingsRevan RevanCfg(false);
  if (RevanCfg.Read(m_RevanCfgFileName) == false) {
    merr<<"Unable to open revan configuration file \""<<m_RevanCfgFileName<<"\""<<endl; 
    return false;
  }
  m_ReReader->SetSettings(&RevanCfg);

  MSettingsMimrec MimrecCfg(false);
  if (MimrecCfg.Read(m_MimrecCfgFileName) == false) {
    merr<<"Unable to open mimrec configuration file \""<<m_MimrecCfgFileName<<"\""<<endl; 
    return false;
  }
  m_MimrecEventSelector.SetSettings(&MimrecCfg);

  if (m_ReReader->PreAnalysis() == false) {
    merr<<"Unable to initialize event reconstruction."<<endl;     
    return false;
  }
  
  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) {
    merr<<"Unable to open simulation file \""<<m_SimulationFileName<<"\" in sivan"<<endl; 
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseSpectral::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenFiles() == false) return false;

  cout<<"Generating spectral response"<<endl;

  vector<float> AxisEnergy2;
  AxisEnergy2 = CreateLogDist(10, 20000, 1000, 1, 100000);

  MResponseMatrixO2 EnergyBeforeER("Energy (before event reconstruction)", AxisEnergy2, AxisEnergy2);
  EnergyBeforeER.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

  MResponseMatrixO2 EnergyUnselected("Energy (no event selections)", AxisEnergy2, AxisEnergy2);
  EnergyUnselected.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

  MResponseMatrixO2 EnergySelected("Energy (with event selections)", AxisEnergy2, AxisEnergy2);
  EnergySelected.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

  vector<float> AxisRelativeIdealEnergy;
  AxisRelativeIdealEnergy = CreateLogDist(10, 10000, 400);

  vector<float> AxisRelativeMeasuredEnergy;
  AxisRelativeMeasuredEnergy = CreateEquiDist(0, 1.2, 9 + 100*6);
  // 9 + N*6:
  // The goal is to have one bin exactly centered around 1.0
  // The above statement gives a boarder pixel at 0, 0.4, 0.8, 1.2
  // and a center pixel at 0.2 0.6 and 1.0
  // The minimum number of pixels I need to achieve this is 9
  // To get more pixel, I have 6 positions to fill them in therefore 100 * 6
  // 
  // Pictogram:
  //
  //     ----- ----- ----- ----- ----- ----- ----- ----- ----- 
  //    |     |     |     |     |     |     |     |     |     |
  //     ----- ----- ----- ----- ----- ----- ----- ----- ----- 
  //    ^        ^        ^        ^        ^        ^        ^
  //   0.0      0.2      0.4      0.6      0.8      1.0      1.2
  //          ^     ^           ^     ^           ^     ^
  //        Fill   Fill       Fill   Fill       Fill   Fill
  
  
  MResponseMatrixO2 EnergyRatioBeforeER("Energy ratio (before event reconstruction)", AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  EnergyRatioBeforeER.SetAxisNames("ideal energy [keV]", "measured energy / ideal energy");

  MResponseMatrixO2 EnergyRatioUnselected("Energy ratio (no event selections)", AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  EnergyRatioUnselected.SetAxisNames("ideal energy [keV]", "measured energy / ideal energy");

  MResponseMatrixO2 EnergyRatioSelected("Energy ratio (with event selections)", AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  EnergyRatioSelected.SetAxisNames("ideal energy [keV]", "measured energy / ideal energy");
  
  
  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();
    MRERawEvent* RE = REList->GetInitialRawEvent();
    if (RE != nullptr) {
      EnergyBeforeER.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), RE->GetEnergy());
      EnergyRatioBeforeER.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), RE->GetEnergy() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
    }
    
    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != nullptr) {
        EnergyUnselected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei());
        EnergyRatioUnselected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
        if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
          EnergySelected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei());
          EnergyRatioSelected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
        }
      }
    }
    
    if (++Counter % m_SaveAfter == 0) {
      EnergyBeforeER.Write(m_ResponseName + ".energy.beforeeventreconstruction" + m_Suffix, true);
      EnergyUnselected.Write(m_ResponseName + ".energy.mimrecunselected" + m_Suffix, true);
      EnergySelected.Write(m_ResponseName + ".energy.mimrecselected" + m_Suffix, true);
      EnergyRatioBeforeER.Write(m_ResponseName + ".energyratio.beforeeventreconstruction" + m_Suffix, true);
      EnergyRatioUnselected.Write(m_ResponseName + ".energyratio.mimrecunselected" + m_Suffix, true);
      EnergyRatioSelected.Write(m_ResponseName + ".energyratio.mimrecselected" + m_Suffix, true);
    }
  }  

  EnergyBeforeER.Write(m_ResponseName + ".energy.beforeeventreconstruction" + m_Suffix, true);
  EnergyUnselected.Write(m_ResponseName + ".energy.mimrecunselected" + m_Suffix, true);
  EnergySelected.Write(m_ResponseName + ".energy.mimrecselected" + m_Suffix, true);
  EnergyRatioBeforeER.Write(m_ResponseName + ".energyratio.beforeeventreconstruction" + m_Suffix, true);
  EnergyRatioUnselected.Write(m_ResponseName + ".energyratio.mimrecunselected" + m_Suffix, true);
  EnergyRatioSelected.Write(m_ResponseName + ".energyratio.mimrecselected" + m_Suffix, true);

  return true;
}


// MResponseSpectral.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
