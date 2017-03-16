/*
 * MResponseImagingEfficiencyNearField.cxx
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
// MResponseImagingEfficiencyNearField
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingEfficiencyNearField.h"

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


#ifdef ___CINT___
ClassImp(MResponseImagingEfficiencyNearField)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingEfficiencyNearField::MResponseImagingEfficiencyNearField()
{
  m_ResponseNameSuffix = "efficiencynearfield";
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingEfficiencyNearField::~MResponseImagingEfficiencyNearField()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingEfficiencyNearField::Initialize()
{
  if (MResponseBuilder::Initialize() == false) return false;

  vector<float> AxisX = CreateEquiDist(-10, 10, 15);
  vector<float> AxisY = CreateEquiDist(-10, 10, 15);
  vector<float> AxisZ = CreateEquiDist(8, 18, 15);
  vector<float> AxisEnergy = CreateEquiDist(1, 10000, 1);

  m_Efficiency.SetName("EfficiencyNearField");
  m_Efficiency.SetAxis(AxisX, AxisY, AxisZ, AxisEnergy);
  m_Efficiency.SetAxisNames("x [cm]", "y [cm]", "z [cm]", "Energy [keV]");
  if (m_SiReader != nullptr) {
    m_Efficiency.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseImagingEfficiencyNearField::Analyze()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;


  MRawEventList* REList = m_ReReader->GetRawEventList();

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
            m_Efficiency.Add(IdealOrigin.X(), IdealOrigin.Y(), IdealOrigin.Z(), m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
          }
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseImagingEfficiencyNearField::Finalize()
{
  return MResponseBuilder::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingEfficiencyNearField::Save()
{
  MResponseBuilder::Save();

  m_Efficiency.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Efficiency.Write(GetFilePrefix() +  m_Suffix, true);

  return true;
}


// MResponseImagingEfficiencyNearField.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
