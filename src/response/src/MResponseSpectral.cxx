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
  // Intentionally left empty 
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseSpectral::Initialize() 
{ 
  if (MResponseBuilder::Initialize() == false) return false;
  
  
  vector<float> AxisEnergy2;
  AxisEnergy2 = CreateLogDist(10, 20000, 1000, 1, 100000);

  m_EnergyBeforeER.SetName("Energy (before event reconstruction)");
  m_EnergyBeforeER.SetAxis(AxisEnergy2, AxisEnergy2);
  m_EnergyBeforeER.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

  m_EnergyUnselected.SetName("Energy (no event selections)");
  m_EnergyUnselected.SetAxis(AxisEnergy2, AxisEnergy2);
  m_EnergyUnselected.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

  m_EnergySelected.SetName("Energy (with event selections)");
  m_EnergySelected.SetAxis(AxisEnergy2, AxisEnergy2);
  m_EnergySelected.SetAxisNames("ideal energy [keV]", "measured energy [keV]");

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
  
  
  m_EnergyRatioBeforeER.SetName("Energy ratio (before event reconstruction)");
  m_EnergyRatioBeforeER.SetAxis(AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  m_EnergyRatioBeforeER.SetAxisNames("ideal energy [keV]", "measured energy / ideal energy");

  m_EnergyRatioUnselected.SetName("Energy ratio (no event selections)");
  m_EnergyRatioUnselected.SetAxis(AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  m_EnergyRatioUnselected.SetAxisNames("ideal energy [keV]" , "measured energy / ideal energy");

  m_EnergyRatioSelected.SetName("Energy ratio (with event selections)");
  m_EnergyRatioSelected.SetAxis(AxisRelativeIdealEnergy, AxisRelativeMeasuredEnergy);
  m_EnergyRatioSelected.SetAxisNames("ideal energy [keV]", "measured energy / ideal energy");
   
  
  return true; 
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze th events (all if in file mode, one if in event-by-event mode)
bool MResponseSpectral::Analyze() 
{ 
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  MRawEventList* REList = m_ReReader->GetRawEventList();
  MRERawEvent* RE = REList->GetInitialRawEvent();
  if (RE != nullptr) {
    m_EnergyBeforeER.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), RE->GetEnergy());
    m_EnergyRatioBeforeER.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), RE->GetEnergy() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
  }
    
  if (REList->HasOptimumEvent() == true) {
    MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
    if (Event != nullptr) {
      m_EnergyUnselected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei());
      m_EnergyRatioUnselected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        m_EnergySelected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei());
        m_EnergyRatioSelected.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Event->Ei() / m_SiEvent->GetIAAt(0)->GetSecondaryEnergy());
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


bool MResponseSpectral::Save()
{
  // Create the multiple Compton response

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
