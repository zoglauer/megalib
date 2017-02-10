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


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseSpectral)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseSpectral::MResponseSpectral()
{
  // Intentionally left empty - call Initialize for initialization
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
  Origin.SetFISBEL(500);
  
  MResponseMatrixAxis Measured("measured energy [keV]");
  Measured.SetLogarithmic(500, 10, 20000, 1, 100000);
  
  m_EnergyBeforeER.SetName("Energy response (before event reconstruction)");
  m_EnergyBeforeER.AddAxis(Ideal);
  m_EnergyBeforeER.AddAxis(Origin);
  m_EnergyBeforeER.AddAxis(Measured);

  m_EnergyUnselected.SetName("Energy response (mimrec - no event selections)");
  m_EnergyUnselected.AddAxis(Ideal);
  m_EnergyUnselected.AddAxis(Origin);
  m_EnergyUnselected.AddAxis(Measured);

  m_EnergySelected.SetName("Energy response (mimrec - with event selections)");
  m_EnergySelected.AddAxis(Ideal);
  m_EnergySelected.AddAxis(Origin);
  m_EnergySelected.AddAxis(Measured);

  
  MResponseMatrixAxis RatioIdeal("ideal energy [keV]");
  RatioIdeal.SetLinear(400, 10, 10000);

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

  m_EnergyRatioUnselected.SetName("Energy ratio (no event selections)");
  m_EnergyRatioUnselected.AddAxis(RatioIdeal);
  m_EnergyRatioUnselected.AddAxis(Origin);
  m_EnergyRatioUnselected.AddAxis(RatioMeasuredIdeal);
  
  m_EnergyRatioSelected.SetName("Energy ratio (with event selections)");
  m_EnergyRatioSelected.AddAxis(RatioIdeal);
  m_EnergyRatioSelected.AddAxis(Origin);
  m_EnergyRatioSelected.AddAxis(RatioMeasuredIdeal);
  
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
    m_EnergyBeforeER.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, RE->GetEnergy() });
    m_EnergyRatioBeforeER.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, RE->GetEnergy() / SimStartEnergy });
  }
    
  
  if (REList->HasOptimumEvent() == true) {
    MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
    if (Event != nullptr) {
      m_EnergyUnselected.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() });
      m_EnergyRatioUnselected.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() / SimStartEnergy });
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        // TODO: We might need to do an ARM cut?
        m_EnergySelected.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() });
        m_EnergyRatioSelected.Add({ SimStartEnergy, SimStartTheta, SimStartPhi, Event->Ei() / SimStartEnergy });
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
  m_EnergyBeforeER.Write(m_ResponseName + ".energy.beforeeventreconstruction" + m_Suffix, true);
  m_EnergyUnselected.Write(m_ResponseName + ".energy.mimrecunselected" + m_Suffix, true);
  m_EnergySelected.Write(m_ResponseName + ".energy.mimrecselected" + m_Suffix, true);
  m_EnergyRatioBeforeER.Write(m_ResponseName + ".energyratio.beforeeventreconstruction" + m_Suffix, true);
  m_EnergyRatioUnselected.Write(m_ResponseName + ".energyratio.mimrecunselected" + m_Suffix, true);
  m_EnergyRatioSelected.Write(m_ResponseName + ".energyratio.mimrecselected" + m_Suffix, true);

  return true;
}


// MResponseSpectral.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
