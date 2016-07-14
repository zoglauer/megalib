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


bool MResponseImagingEfficiency::OpenFiles()
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

  if (OpenFiles() == false) return false;

  cout<<"Generating efficiency"<<endl;

  vector<float> AxisPhi = CreateEquiDist(-180.0, 180.0, 360);
  vector<float> AxisTheta = CreateEquiDist(0.0, 180.0, 180);

  MResponseMatrixO2 Efficiency1("Efficiency_rot90yaxis", AxisPhi, AxisTheta);
  Efficiency1.SetAxisNames("#phi [deg]", "#theta [deg]");
  Efficiency1.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());

  // A default rotation of the origin 
  MRotation Rotation1(90.0 * c_Rad, MVector(0, 1, 0));
  
  MResponseMatrixO2 Efficiency2("Efficiency_rot90zaxis_rot90yaxis", AxisPhi, AxisTheta);
  Efficiency2.SetAxisNames("#phi [deg]", "#theta [deg]");
  Efficiency2.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());

  // A default rotation of the origin 
  MRotation Rotation2(90.0 * c_Rad, MVector(0, 0, 1));
  Rotation2 = MRotation(90.0 * c_Rad, MVector(0, 1, 0));
  
  
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

              MVector IdealOrigin1 = Rotation1 * IdealOrigin;
              Efficiency1.Add(IdealOrigin1.Phi()*c_Deg, IdealOrigin1.Theta()*c_Deg);

              MVector IdealOrigin2 = Rotation2 * IdealOrigin;
              Efficiency2.Add(IdealOrigin2.Phi()*c_Deg, IdealOrigin2.Theta()*c_Deg);
           }
          }
        }
      }    
    }
    if (++Counter % m_SaveAfter == 0) {
      Efficiency1.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
      Efficiency2.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
      Efficiency1.Write(m_ResponseName + ".efficiency.90y" + m_Suffix, true);
      Efficiency2.Write(m_ResponseName + ".efficiency.90z.90y" + m_Suffix, true);
    }
  }  

  Efficiency1.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  Efficiency2.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  Efficiency1.Write(m_ResponseName + ".efficiency.90y" + m_Suffix, true);
  Efficiency2.Write(m_ResponseName + ".efficiency.90z.90y" + m_Suffix, true);

  return true;
}


// MResponseImagingEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
