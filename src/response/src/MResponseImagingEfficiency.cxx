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
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO4.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseImagingEfficiency)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingEfficiency::MResponseImagingEfficiency()
{
  m_ResponseNameSuffix = "efficiency";
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingEfficiency::~MResponseImagingEfficiency()
{
  // Nothing to delete
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingEfficiency::Initialize() 
{ 
  if (MResponseBuilder::Initialize() == false) return false;
  
  vector<float> AxisPhi = CreateEquiDist(-180.0, 180.0, 360);
  vector<float> AxisTheta = CreateEquiDist(0.0, 180.0, 180);

  m_Efficiency1.SetName("Efficiency_rot90yaxis");
  m_Efficiency1.SetAxis(AxisPhi, AxisTheta);
  m_Efficiency1.SetAxisNames("#phi [deg]", "#theta [deg]");
  if (m_SiReader != nullptr) {
    m_Efficiency1.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }

  // A default rotation of the origin 
  m_Rotation1 = MRotation(90.0 * c_Rad, MVector(0, 1, 0));
  
  m_Efficiency2.SetName("Efficiency_rot90zaxis_rot90yaxis");
  m_Efficiency2.SetAxis(AxisPhi, AxisTheta);
  m_Efficiency2.SetAxisNames("#phi [deg]", "#theta [deg]");
  if (m_SiReader != nullptr) {
    m_Efficiency2.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }

  // A default rotation of the origin 
  m_Rotation2 = MRotation(90.0 * c_Rad, MVector(0, 0, 1));

  // Detection efficiency detector
  vector<float> AxisComptonScatterAngle = CreateEquiDist(0.0, 180.0, 36);
  vector<float> AxisSecondScatterAngleFromZenith = CreateEquiDist(0.0, 180.0, 36);
  vector<float> AxisFirstInteractionDistance = CreateEquiDist(0.2, 10, 20, 0, 30);
  vector<float> AxisEi = CreateLogDist(100, 10000, 15);

  m_DetectionEfficiency.SetName("DetectionEfficiency");
  m_DetectionEfficiency.SetAxis(AxisComptonScatterAngle, AxisSecondScatterAngleFromZenith, AxisFirstInteractionDistance, AxisEi);
  m_DetectionEfficiency.SetAxisNames("Compton scatter angle [deg]", 
                                     "Scattered gamma on-axis direction [deg]",
                                     "Interaction distance [cm]", 
                                     "Energy [keV]");  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseImagingEfficiency::Analyze() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  MRawEventIncarnations* REList = m_ReReader->GetRawEventList();
  
  if (REList->HasOptimumEvent() == true) {
    MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
    if (Event != 0) {
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          MComptonEvent* Compton = (MComptonEvent*) Event;
          
          if (Compton->IsKinematicsOK() == false) return true;
          
          // Now get the ideal origin:
          if (m_SiEvent->GetNIAs() > 0) {
            MVector IdealOrigin = m_SiEvent->GetIAAt(0)->GetPosition();
            
            MVector IdealOrigin1 = m_Rotation1 * IdealOrigin;
            m_Efficiency1.Add(IdealOrigin1.Phi()*c_Deg, IdealOrigin1.Theta()*c_Deg);
            
            MVector IdealOrigin2 = m_Rotation2 * IdealOrigin;
            m_Efficiency2.Add(IdealOrigin2.Phi()*c_Deg, IdealOrigin2.Theta()*c_Deg);
            
            m_DetectionEfficiency.Add(Compton->Phi()*c_Deg, Compton->Dg().Angle(MVector(0, 0, -1)), Compton->MinLeverArm(), Compton->Ei());
            
          }
        }
      }
    }    
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseImagingEfficiency::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingEfficiency::Save()
{
  MResponseBuilder::Save(); 

  m_Efficiency1.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Efficiency2.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_DetectionEfficiency.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Efficiency1.Write(GetFilePrefix() + ".90y" + m_Suffix, true);
  m_Efficiency2.Write(GetFilePrefix() + ".90z.90y" + m_Suffix, true);
  m_DetectionEfficiency.Write(GetFilePrefix() + ".detection" + m_Suffix, true);

  return true;
}

// MResponseImagingEfficiency.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
