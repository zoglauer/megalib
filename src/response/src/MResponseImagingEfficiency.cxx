/*
 * MResponseImagingEfficiency.cxx
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
// MResponseImagingEfficiency
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingEfficiency.h"

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
ClassImp(MResponseImagingEfficiency)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseImagingEfficiency::MResponseImagingEfficiency()
{
  // Construct an instance of MResponseImagingEfficiency
}


////////////////////////////////////////////////////////////////////////////////


MResponseImagingEfficiency::~MResponseImagingEfficiency()
{
  // Delete this instance of MResponseImagingEfficiency
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImagingEfficiency::SetMimrecConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_MimrecCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImagingEfficiency::SetRevanConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImagingEfficiency::OpenSimulationFile()
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


bool MResponseImagingEfficiency::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  cout<<"Generating efficiency"<<endl;

  vector<float> AxisPhi = CreateEquiDist(-180.0, 180.0, 4*360);
  vector<float> AxisTheta = CreateEquiDist(0.0, 180.0, 4*180);

  MResponseMatrixO2 Efficiency("Efficiency", AxisPhi, AxisTheta);
  Efficiency.SetAxisNames("#phi [deg]", "#theta [deg]");


  MVector IdealOrigin;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;
  
  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();

    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event, true) == true) {
          if (Event->GetType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;

            if (Compton->IsKinematicsOK() == false) continue;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOrigin = m_SiEvent->GetIAAt(0)->GetPosition();

              // Phi response:
              Efficiency.Add(IdealOrigin.Phi()*c_Deg, IdealOrigin.Theta()*c_Deg);
           }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      Efficiency.Write(m_ResponseName + ".efficiency" + m_Suffix, true);
    }
  }  

  Efficiency.Write(m_ResponseName + ".efficiency" + m_Suffix, true);

  return true;
}


// MResponseImagingEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
