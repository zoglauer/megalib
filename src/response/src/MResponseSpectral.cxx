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
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"
#include "MCoordinateSystem.h"
#include "MComptonEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseSpectral)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseSpectral::MResponseSpectral()
{
  m_ResponseNameSuffix = "energy";
  m_OnlyINITRequired = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseSpectral::~MResponseSpectral()
{
  // Nothing to delete
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseSpectral::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  
  MResponseMatrixAxis Ideal("ideal energy [keV]");
  Ideal.SetLogarithmic(500, 10, 20000, 1, 100000);
  
  MResponseMatrixAxisSpheric Origin("Theta (detector coordinates) [deg]", "Phi (detector coordinates) [deg]");
  Origin.SetFISBEL(413); // 100 deg^2
  
  MResponseMatrixAxis Measured("measured energy [keV]");
  Measured.SetLogarithmic(500, 10, 20000, 1, 100000);
  
  m_EnergyBeforeER.SetName("Energy response (before event reconstruction)");
  m_EnergyBeforeER.AddAxis(Ideal);
  m_EnergyBeforeER.AddAxis(Origin);
  m_EnergyBeforeER.AddAxis(Measured);
  if (m_SiReader != nullptr) {
    m_EnergyBeforeER.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_EnergyUnselected.SetName("Energy response (mimrec - no event selections)");
  m_EnergyUnselected.AddAxis(Ideal);
  m_EnergyUnselected.AddAxis(Origin);
  m_EnergyUnselected.AddAxis(Measured);
  if (m_SiReader != nullptr) {
    m_EnergyUnselected.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_EnergySelected.SetName("Energy response (mimrec - with event selections)");
  m_EnergySelected.AddAxis(Ideal);
  m_EnergySelected.AddAxis(Origin);
  m_EnergySelected.AddAxis(Measured);
  if (m_SiReader != nullptr) {
    m_EnergySelected.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  
  MResponseMatrixAxis RatioIdeal("ideal energy [keV]");
  RatioIdeal.SetLinear(500, 10, 10000);

  MResponseMatrixAxis RatioMeasuredIdeal("measured energy / ideal energy");
  RatioMeasuredIdeal.SetLinear(9 + 100*6, 0, 1.2);
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
  
  
  m_EnergyRatioBeforeER.SetName("Energy ratio (before event reconstruction)");
  m_EnergyRatioBeforeER.AddAxis(RatioIdeal);
  m_EnergyRatioBeforeER.AddAxis(Origin);
  m_EnergyRatioBeforeER.AddAxis(RatioMeasuredIdeal);
  if (m_SiReader != nullptr) {
    m_EnergyRatioBeforeER.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_EnergyRatioUnselected.SetName("Energy ratio (no event selections)");
  m_EnergyRatioUnselected.AddAxis(RatioIdeal);
  m_EnergyRatioUnselected.AddAxis(Origin);
  m_EnergyRatioUnselected.AddAxis(RatioMeasuredIdeal);
  if (m_SiReader != nullptr) {
    m_EnergyRatioUnselected.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_EnergyRatioSelected.SetName("Energy ratio (with event selections)");
  m_EnergyRatioSelected.AddAxis(RatioIdeal);
  m_EnergyRatioSelected.AddAxis(Origin);
  m_EnergyRatioSelected.AddAxis(RatioMeasuredIdeal);
  if (m_SiReader != nullptr) {
    m_EnergyRatioSelected.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_ARMCut = 5;
  
  MResponseMatrixAxisSpheric ARMCut("ARM cut center - theta (detector coordinates) [deg]", "ARM cut center - phi (detector coordinates) [deg]");
  ARMCut.SetFISBEL(413);
  
  m_EnergySelectedARMCut.SetName(MString("Energy response with ARM cut of ") + m_ARMCut + " deg radius around given position");
  m_EnergySelectedARMCut.AddAxis(Ideal);
  m_EnergySelectedARMCut.AddAxis(Measured);
  m_EnergySelectedARMCut.AddAxis(ARMCut);
  if (m_SiReader != nullptr) {
    m_EnergySelectedARMCut.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }
  
  m_BinCenters = ARMCut.GetAllBinCenters();
  for (unsigned int i = 0; i < m_BinCenters.size(); ++i) m_BinCenters[i].SetMag(1E+20);
  
  return true; 
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseSpectral::Analyze() 
{ 
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  MRawEventList* REList = m_ReReader->GetRawEventList();
  MRERawEvent* RE = REList->GetInitialRawEvent();
  
  double SimStartEnergy = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
  double SimStartTheta = (-m_SiEvent->GetIAAt(0)->GetSecondaryDirection()).Theta()*c_Deg;
  double SimStartPhi = (-m_SiEvent->GetIAAt(0)->GetSecondaryDirection()).Phi()*c_Deg;
  while (SimStartPhi < 0) SimStartPhi += 360;  
  
  if (RE != nullptr) {
    m_EnergyBeforeER.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, RE->GetEnergy() });
    m_EnergyRatioBeforeER.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, RE->GetEnergy() / SimStartEnergy });
  }
    
  
  if (REList->HasOptimumEvent() == true) {
    MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
    if (Event != nullptr) {
      m_EnergyUnselected.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() });
      m_EnergyRatioUnselected.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() / SimStartEnergy });
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        // TODO: We might need to do an ARM cut?
        m_EnergySelected.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() });
        m_EnergyRatioSelected.Add(vector<double>{ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() / SimStartEnergy });
      
        // ARM cut for Compton events
        MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Event);
        if (Compton != nullptr && Compton->IsKinematicsOK() == true) {
          for (unsigned int i = 0; i < m_BinCenters.size(); ++i) {
            double ARM = Compton->GetARMGamma(m_BinCenters[i], MCoordinateSystem::c_Spheric)*c_Deg;
            //cout<<m_BinCenters[i].Theta()*c_Deg<<":"<<m_BinCenters[i].Phi()*c_Deg<<endl;
            if (ARM <= m_ARMCut) {
              double Theta = m_BinCenters[i].Theta()*c_Deg;
              double Phi = m_BinCenters[i].Phi()*c_Deg;
              while (Phi < 0) Phi += 360;  
              m_EnergySelectedARMCut.Add(vector<double>{SimStartEnergy, Event->Ei(), Theta, Phi });
            }
          }
        }
      }
    }
  }
  
  return true; 
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseSpectral::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseSpectral::Save()
{
  MResponseBuilder::Save(); 

  
  m_EnergyBeforeER.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyBeforeER.Write(GetFilePrefix() + ".beforeeventreconstruction" + m_Suffix, true);
  
  m_EnergyUnselected.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyUnselected.Write(GetFilePrefix() + ".mimrecunselected" + m_Suffix, true);
  
  m_EnergySelected.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergySelected.Write(GetFilePrefix() + ".mimrecselected" + m_Suffix, true);
  
  m_EnergyRatioBeforeER.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyRatioBeforeER.Write(GetFilePrefix() + ".ratio.beforeeventreconstruction" + m_Suffix, true);
  
  m_EnergyRatioUnselected.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyRatioUnselected.Write(GetFilePrefix() + ".ratio.mimrecunselected" + m_Suffix, true);
  
  m_EnergyRatioSelected.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyRatioSelected.Write(GetFilePrefix() + ".ratio.mimrecselected" + m_Suffix, true);
  
  m_EnergySelectedARMCut.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergySelectedARMCut.Write(GetFilePrefix() + ".armcut" + m_Suffix, true);
  
  return true;
}


// MResponseSpectral.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
