/*
 * MResponseEventQuality.cxx
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
// MResponseEventQuality
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseEventQuality.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MTimer.h"
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseEventQuality)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseEventQuality::MResponseEventQuality()
{
  // Intentionally left empty - call Initialize for initialization
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseEventQuality::~MResponseEventQuality()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseEventQuality::Initialize()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  int AngleBinWidth = 5;
  int AngleBins = 4*c_Pi*c_Deg*c_Deg / AngleBinWidth / AngleBinWidth;

  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  AxisEnergyMeasured.SetLinear(1, 0, 2000);

  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(180/AngleBinWidth, 0, 180);
  
  MResponseMatrixAxisSpheric AxisScatteredGammaRayCoordinates("#psi [deg]", "#chi [deg]");
  AxisScatteredGammaRayCoordinates.SetFISBELByNumberOfBins(AngleBins);
  
  m_GoodQuality.SetName("Event quality -- good events");
  m_GoodQuality.AddAxis(AxisEnergyMeasured);
  m_GoodQuality.AddAxis(AxisPhi);
  m_GoodQuality.AddAxis(AxisScatteredGammaRayCoordinates);
  if (m_SiReader != nullptr) {
    m_GoodQuality.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   
  
  m_BadQuality.SetName("Event quality -- good events");
  m_BadQuality.AddAxis(AxisEnergyMeasured);
  m_BadQuality.AddAxis(AxisPhi);
  m_BadQuality.AddAxis(AxisScatteredGammaRayCoordinates);
  if (m_SiReader != nullptr) {
    m_BadQuality.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseEventQuality::Analyze() 
{ 
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require a successful reconstruction 
  MRawEventIncarnationList* REList = m_ReReader->GetRawEventList();
  if (REList->HasOnlyOptimumEvents() == false) {
    return true;
  }
    
  // ... leading to an event
  MPhysicalEvent* Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
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
  //double Lambda = IdealOriginDir.Phi()*c_Deg;
  //while (Lambda < -180) Lambda += 360.0;
  //while (Lambda > +180) Lambda -= 360.0;
  double Nu = IdealOriginDir.Theta()*c_Deg;
  //double EnergyInitial = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
  
  // And fill the matrices
  if (Nu < 60) {
    m_GoodQuality.Add( vector<double>{ EnergyMeasured, Phi, Psi, Chi } );
  } else if (Nu > 80) {
    m_BadQuality.Add( vector<double>{ EnergyMeasured, Phi, Psi, Chi } );
  }
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseEventQuality::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseEventQuality::Save()
{
  MTimer T;
  
  m_GoodQuality.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_GoodQuality.Write(m_ResponseName + ".quality.good" + m_Suffix, true);
  
  m_BadQuality.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_BadQuality.Write(m_ResponseName + ".quality.bad" + m_Suffix, true);
  
  cout<<"Time spent saving data: "<<T.GetElapsed()<<" seconds"<<endl;
  
  return true;
}


// MResponseEventQuality.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
