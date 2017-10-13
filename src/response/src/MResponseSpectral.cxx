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
  
  m_EnergyUnderflow = 1;
  m_EnergyMinimum = 10;
  m_EnergyMaximum = 20000;
  m_EnergyOverflow = 100000;
  m_EnergyNumberOfBins = 500;
  m_EnergyNumberOfSkyBins = 413;
  m_ARMCut = 5;
  m_ARMCutNumberOfSkyBins = 413;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseSpectral::~MResponseSpectral()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseSpectral::Description()
{
  return MString("Spectral response (before event reconstruction, mimrec without & with event selections, ARM cut response)");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseSpectral::Options()
{
  ostringstream out;
  out<<"             emin:            minimum energy (default: 10 keV)"<<endl;
  out<<"             emax:            maximum energy (default: 20,000 keV)"<<endl;
  out<<"             ebins:           number of energy bins between min and max energy (default: 500)"<<endl;
  out<<"             eunder:          underflow bin minimum (default: 1 keV)"<<endl;
  out<<"             eover:           overflow bin maximum (default: 100,000 keV)"<<endl;
  out<<"             eskybins:        sky bins (default: 413 - 10^2 deg bins)"<<endl;
  out<<"             armcut:          ARM cut radius (default: 5 deg)"<<endl;
  out<<"             armcutskybins:   sky bins for ARM cut response (default: 413 - 10^2 deg bins)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseSpectral::ParseOptions(const MString& Options)
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
      m_EnergyNumberOfBins = stod(Value);
    } else if (Split2[i][0] == "eunder") {
      m_EnergyUnderflow = stod(Value);
    } else if (Split2[i][0] == "eover") {
      m_EnergyOverflow = stod(Value);
    } else if (Split2[i][0] == "eskybins") {
      m_EnergyNumberOfSkyBins = stod(Value);
    } else if (Split2[i][0] == "armcut") {
      m_ARMCut = stod(Value);
    } else if (Split2[i][0] == "armcutskybins") {
      m_ARMCutNumberOfSkyBins = stod(Value);
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_EnergyUnderflow <= 0 || m_EnergyMinimum <= 0 || m_EnergyMaximum <= 0 || m_EnergyOverflow <= 0) {
    mout<<"Error: All energy values must be positive (larger than zero)"<<endl;
    return false;    
  }
  if (m_EnergyUnderflow > m_EnergyMinimum) {
    mout<<"Error: The underflow energy can not be larger than the minimum energy"<<endl;
    return false;       
  }
  if (m_EnergyMinimum >= m_EnergyMaximum) {
    mout<<"Error: The minimum energy must be smaller than the maximum energy"<<endl;
    return false;       
  }
  if (m_EnergyMaximum > m_EnergyOverflow) {
    mout<<"Error: The overflow energy must be larger than the maximum energy"<<endl;
    return false;       
  }
  if (m_EnergyNumberOfBins <= 0) {
    mout<<"Error: You need at least one energy bin"<<endl;
    return false;       
  }
  if (m_EnergyNumberOfSkyBins <= 0) {
    mout<<"Error: You need at least one sky bin"<<endl;
    return false;       
  }
  if (m_ARMCut <= 0) {
    mout<<"Error: You need a positive value for the ARM cut"<<endl;
    return false;       
  }
  if (m_ARMCutNumberOfSkyBins <= 0) {
    mout<<"Error: You need at least one sky bin for the ARM cut response"<<endl;
    return false;       
  } 
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for spectral response:"<<endl;
  mout<<"  Minimum energy:        "<<m_EnergyMinimum<<endl;
  mout<<"  Maximum energy:        "<<m_EnergyMaximum<<endl;
  mout<<"  Number of bins energy: "<<m_EnergyNumberOfBins<<endl;
  mout<<"  Underflow minimum:     "<<m_EnergyUnderflow<<endl;
  mout<<"  Overflow maximum:      "<<m_EnergyOverflow<<endl;
  mout<<"  Sky bins energy:       "<<m_EnergyNumberOfSkyBins<<endl;
  mout<<"  ARM cut:               "<<m_ARMCut<<endl;
  mout<<"  ARM cut sky bins:      "<<m_ARMCutNumberOfSkyBins<<endl;
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseSpectral::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  
  MResponseMatrixAxis Ideal("ideal energy [keV]");
  Ideal.SetLogarithmic(m_EnergyNumberOfBins, m_EnergyMinimum, m_EnergyMaximum, m_EnergyUnderflow, m_EnergyOverflow);
  
  MResponseMatrixAxisSpheric Origin("Theta (detector coordinates) [deg]", "Phi (detector coordinates) [deg]");
  Origin.SetFISBEL(m_EnergyNumberOfSkyBins);
  
  MResponseMatrixAxis Measured("measured energy [keV]");
  Measured.SetLogarithmic(m_EnergyNumberOfBins, m_EnergyMinimum, m_EnergyMaximum, m_EnergyUnderflow, m_EnergyOverflow);
  
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
  RatioIdeal.SetLinear(m_EnergyNumberOfBins, m_EnergyMinimum, m_EnergyMaximum);

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

  
  MResponseMatrixAxisSpheric ARMCut("ARM cut center - theta (detector coordinates) [deg]", "ARM cut center - phi (detector coordinates) [deg]");
  ARMCut.SetFISBEL(m_ARMCutNumberOfSkyBins);
  
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
