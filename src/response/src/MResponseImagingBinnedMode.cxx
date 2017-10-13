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
  
  m_AngleBinWidth = 5;
  m_EnergyNBins = 1;
  m_EnergyMinimum = 10;
  m_EnergyMaximum = 2000;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingBinnedMode::~MResponseImagingBinnedMode()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseImagingBinnedMode::Description()
{
  return MString("Binned imaging response");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseImagingBinnedMode::Options()
{
  ostringstream out;
  out<<"             anglebinwidth:   the width of a aky bin at the equator (default: 5 deg)"<<endl;
  out<<"             emin:            minimum energy (default: 10 keV)"<<endl;
  out<<"             emax:            maximum energy (default: 2,000 keV)"<<endl;
  out<<"             ebins:           number of energy bins between min and max energy (default: 1)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseImagingBinnedMode::ParseOptions(const MString& Options)
{
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("=")); 
  }
  
  // Basic sanity check and to lower for all options
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i].size() == 0) {
      mout<<"Error: Empty option in string "<<Options<<endl;
      return false;
    }    
    if (Split2[i].size() == 1) {
      mout<<"Error: Option has no value: "<<Split2[i][0]<<endl;
      return false;
    }
    if (Split2[i].size() > 2) {
      mout<<"Error: Option has more than one value or you used the wrong separator (not \":\"): "<<Split1[i]<<endl;
      return false;
    }
    Split2[i][0].ToLowerInPlace();
  }
  
  // Parse
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    string Value = Split2[i][1].Data();
    
    if (Split2[i][0] == "emin") {
      m_EnergyMinimum = stod(Value);
    } else if (Split2[i][0] == "emax") {
      m_EnergyMaximum = stod(Value);
    } else if (Split2[i][0] == "ebins") {
      m_EnergyNBins = stod(Value);
    } else if (Split2[i][0] == "anglebinwidth") {
      m_AngleBinWidth = stod(Value);
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_EnergyMinimum <= 0 || m_EnergyMaximum <= 0) {
    mout<<"Error: All energy values must be positive (larger than zero)"<<endl;
    return false;    
  }
  if (m_EnergyMinimum >= m_EnergyMaximum) {
    mout<<"Error: The minimum energy must be smaller than the maximum energy"<<endl;
    return false;       
  }
  if (m_EnergyNBins <= 0) {
    mout<<"Error: You need at least one energy bin"<<endl;
    return false;       
  }
  if (m_AngleBinWidth <= 0) {
    mout<<"Error: You need at give a positive with of the sky bins at the equator"<<endl;
    return false;       
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for spectral response:"<<endl;
  mout<<"  Minimum energy:                "<<m_EnergyMinimum<<endl;
  mout<<"  Maximum energy:                "<<m_EnergyMaximum<<endl;
  mout<<"  Number of bins energy:         "<<m_EnergyNBins<<endl;
  mout<<"  Width of sky bins at equator:  "<<m_AngleBinWidth<<endl;

  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingBinnedMode::Initialize()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  int AngleBins = 4*c_Pi*c_Deg*c_Deg / m_AngleBinWidth / m_AngleBinWidth;

  MResponseMatrixAxis AxisEnergyInitial("Initial energy [keV]");
  AxisEnergyInitial.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
  
  MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");
  AxisSkyCoordinates.SetFISBEL(AngleBins);

  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  AxisEnergyMeasured.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);

  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(180/m_AngleBinWidth, 0, 180);
  
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

  m_EnergyResponse.SetName("Energy response");
  m_EnergyResponse.AddAxis(AxisEnergyInitial);
  m_EnergyResponse.AddAxis(AxisSkyCoordinates);
  m_EnergyResponse.AddAxis(AxisEnergyMeasured);
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
  m_ImagingResponse.Add( vector<double>{ EnergyInitial, Nu, Lambda, EnergyMeasured, Phi, Psi, Chi } );
  m_Exposure.Add( vector<double>{ EnergyInitial, Nu, Lambda } );
  m_EnergyResponse.Add( vector<double>{ EnergyInitial, Nu, Lambda, EnergyMeasured } );
            
  //cout<<"Added: "<<Event->GetId()<<":"<<Phi<<":"<<Psi<<":"<<Chi<<endl;
  
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
