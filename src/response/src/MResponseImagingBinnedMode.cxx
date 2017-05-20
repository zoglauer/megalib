/*
 * MResponseImagingBinnedMode.cxx
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
// MResponseImagingBinnedMode
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingBinnedMode.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseImagingBinnedMode)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingBinnedMode::MResponseImagingBinnedMode()
{
  m_ResponseNameSuffix = "binnedimaging";
  m_OnlyINITRequired = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingBinnedMode::~MResponseImagingBinnedMode()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingBinnedMode::Initialize()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  int AngleBinWidth = 5;
  int AngleBins = 4*c_Pi*c_Deg*c_Deg / AngleBinWidth / AngleBinWidth;

  MResponseMatrixAxis AxisEnergyInitial("Initial energy [keV]");
  AxisEnergyInitial.SetLinear(1, 0, 2000);
  
  MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");
  AxisSkyCoordinates.SetFISBEL(AngleBins);
  
  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  AxisEnergyMeasured.SetLinear(1, 0, 2000);

  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(180/AngleBinWidth, 0, 180);
  
  MResponseMatrixAxisSpheric AxisScatteredGammaRayCoordinates("#psi [deg]", "#chi [deg]");
  AxisScatteredGammaRayCoordinates.SetFISBEL(AngleBins);
  
  m_ImagingResponse.SetName("7D imaging response");
  m_ImagingResponse.AddAxis(AxisEnergyInitial);
  m_ImagingResponse.AddAxis(AxisSkyCoordinates);
  m_ImagingResponse.AddAxis(AxisEnergyMeasured);
  m_ImagingResponse.AddAxis(AxisPhi);
  m_ImagingResponse.AddAxis(AxisScatteredGammaRayCoordinates);
   if (m_SiReader != nullptr) {
    m_ImagingResponse.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   
  
  m_Exposure.SetName("Exposure");
  m_Exposure.AddAxis(AxisEnergyInitial);
  m_Exposure.AddAxis(AxisSkyCoordinates);
  if (m_SiReader != nullptr) {
    m_Exposure.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  MResponseMatrixAxis AxisEnergyInitialFine("Initial energy [keV]");
  AxisEnergyInitialFine.SetLogarithmic(500, 1, 20000);
  
  MResponseMatrixAxis AxisEnergyMeasuredFine("Measured energy [keV]");
  AxisEnergyMeasuredFine.SetLogarithmic(200, 1, 2000);

  m_EnergyResponse.SetName("Energy response");
  m_EnergyResponse.AddAxis(AxisEnergyInitialFine);
  m_EnergyResponse.AddAxis(AxisSkyCoordinates);
  m_EnergyResponse.AddAxis(AxisEnergyMeasuredFine);
  if (m_SiReader != nullptr) {
    m_EnergyResponse.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseImagingBinnedMode::Analyze() 
{ 
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require a successful reconstruction 
  MRawEventList* REList = m_ReReader->GetRawEventList();
  if (REList->HasOptimumEvent() == false) {
    return true;
  }
    
  // ... leading to an event
  MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
  if (Event == nullptr) {
    return true;
  }
  
  // ... which needs to be a Compton event
  if (Event->GetType() != MPhysicalEvent::c_Compton) {
    return true;
  }
  
  // ... that passed the event selections
  if (m_MimrecEventSelector.IsQualifiedEvent(Event) == false) {
    return true; 
  }
  
  // ... and has a good kinematics
  MComptonEvent* Compton = (MComptonEvent*) Event;
  if (Compton->IsKinematicsOK() == false) {
    return true;
  }
  
  // Get the data space information
  MRotation Rotation = Compton->GetDetectorRotationMatrix();

  double Phi = Compton->Phi()*c_Deg;
  MVector Dg = -Compton->Dg();
  Dg = Rotation*Dg;
  double Chi = Dg.Phi()*c_Deg;
  while (Chi < -180) Chi += 360.0;
  while (Chi > +180) Chi -= 360.0;
  double Psi = Dg.Theta()*c_Deg;
  double EnergyMeasured = Compton->Ei();
  
  // Now get the origin information
  MVector IdealOriginDir = -m_SiEvent->GetIAAt(0)->GetSecondaryDirection();
  IdealOriginDir = Rotation*IdealOriginDir;
  double Lambda = IdealOriginDir.Phi()*c_Deg;
  while (Lambda < -180) Lambda += 360.0;
  while (Lambda > +180) Lambda -= 360.0;
  double Nu = IdealOriginDir.Theta()*c_Deg;
  double EnergyInitial = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
  
  
  // And fill the matrices
  m_ImagingResponse.Add( { EnergyInitial, Nu, Lambda, EnergyMeasured, Phi, Psi, Chi } );
  m_Exposure.Add( { EnergyInitial, Nu, Lambda } );
  m_EnergyResponse.Add( { EnergyInitial, Nu, Lambda, EnergyMeasured } );
            
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseImagingBinnedMode::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingBinnedMode::Save()
{
  MResponseBuilder::Save(); 
  
  m_ImagingResponse.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ImagingResponse.Write(GetFilePrefix() + ".imagingresponse" + m_Suffix, true);
  
  m_Exposure.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Exposure.Write(GetFilePrefix() + ".exposure" + m_Suffix, true);
  
  m_EnergyResponse.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyResponse.Write(GetFilePrefix() + ".energyresponse" + m_Suffix, true);
  
  return true;
}


// MResponseImagingBinnedMode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
