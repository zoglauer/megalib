/*
 * MResponseImagingARM.cxx
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
// MResponseImagingARM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingARM.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseImagingARM)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingARM::MResponseImagingARM()
{
  m_ResponseNameSuffix = "imagingarm";
  m_OnlyINITRequired = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingARM::~MResponseImagingARM()
{
  // Nothing to delete
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingARM::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  vector<float> PhiDiffAxis;

  /*
  vector<float> Axis; // = CreateLogDist(0.1, 180, 20);
  for (float x = 0.25; x < 7.9; x += 0.25) Axis.push_back(x);
  for (float x = 8.0; x < 14.9; x += 0.5) Axis.push_back(x);
  for (float x = 15.0; x <= 29.9; x += 1) Axis.push_back(x);
  for (float x = 30.0; x <= 89.9; x += 2) Axis.push_back(x);
  for (float x = 90.0; x <= 179.9; x += 3) Axis.push_back(x);
  Axis.push_back(180.0);

  // Add Inverted:
  for (unsigned int b = Axis.size()-1; b < Axis.size(); --b) {
    PhiDiffAxis.push_back(-Axis[b]);
  }
  PhiDiffAxis.push_back(0);
  for (unsigned int b = 0; b < Axis.size(); ++b) {
    PhiDiffAxis.push_back(Axis[b]);
  }
  */

  double PhiMin = 0.3;
  double PhiNBins = 80;
  
  PhiNBins /= 2;
  
  double PhiStart = 0.5*PhiMin;
  double PhiStop = 180;
  
  double PhiDistance = (PhiStop - PhiStart) - PhiMin*PhiNBins;
  
  double lPhiStart = log(PhiStart);
  double lPhiEnd = log(PhiStart + PhiDistance);
  double lPhiDist = (lPhiEnd - lPhiStart) / PhiNBins;
  
  for (unsigned int b = PhiNBins; b <= PhiNBins; --b) {
    PhiDiffAxis.push_back(-exp(lPhiStart + b*lPhiDist) - b*PhiMin);
  }
  for (unsigned int b = 0; b <= PhiNBins; ++b) {
    PhiDiffAxis.push_back(exp(lPhiStart + b*lPhiDist) + b*PhiMin);
  }
  
  
  vector<float> EnergyAxis = CreateThresholdedLogDist(100, 10000, 10, 25);

  vector<float> PhiAxis = CreateEquiDist(0, 180, 9);
  
  vector<float> DistanceAxis;
  DistanceAxis.push_back(0);
  DistanceAxis.push_back(0.19);
  DistanceAxis.push_back(0.39);
  DistanceAxis.push_back(0.69);
  DistanceAxis.push_back(0.99);
  DistanceAxis.push_back(1.49);
  DistanceAxis.push_back(1.99);
  DistanceAxis.push_back(2.99);
  DistanceAxis.push_back(4.99);
  DistanceAxis.push_back(7.99);
  DistanceAxis.push_back(19.99);
  DistanceAxis.push_back(99.99);
  
  vector<float> InteractionsAxis;
  InteractionsAxis.push_back(1.5);
  InteractionsAxis.push_back(2.5);
  InteractionsAxis.push_back(9.5);
  
  m_Arm.SetName("Angular resolution (all energies)");
  m_Arm.SetAxis(PhiDiffAxis, PhiAxis, EnergyAxis, DistanceAxis, InteractionsAxis);
  m_Arm.SetAxisNames("ARM #phi_{meas} - #phi_{real} [deg]", "Measured Compton-scatter angle [deg]", "Measured energy [keV]", "Measured interaction distance [cm]", "Number of interactions: 2 or 3+ site events");
  if (m_SiReader != nullptr) {
    m_Arm.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   
  
  m_ArmPhotoPeak.SetName("Angular resolution (photo peak)");
  m_ArmPhotoPeak.SetAxis(PhiDiffAxis, PhiAxis, EnergyAxis, DistanceAxis, InteractionsAxis);
  m_ArmPhotoPeak.SetAxisNames("ARM #phi_{meas} - #phi_{real} [deg]", "Measured Compton-scatter angle [deg]", "Measured energy [keV]", "Measured interaction distance [cm]", "Number of interactions: 2 or 3+ site events");
  if (m_SiReader != nullptr) {
    m_ArmPhotoPeak.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   
  
  vector<float> AxisEpsilonDiff = CreateEquiDist(0, 180, 36);
  vector<float> AxisEpsilonReal = CreateEquiDist(0, 180, 1);
  vector<float> AxisEe = CreateLogDist(100, 20000, 20);
  
  m_Spd.SetName("Shape along cone for tracked events");
  m_Spd.SetAxis(AxisEpsilonDiff, AxisEpsilonReal, AxisEe);
  m_Spd.SetAxisNames("SPD #epsilon_{meas} - #epsilon_{real} [deg]", "Measure electron scatter angle [deg]", "Measured electron energy [keV]");
  
  m_SpdPhotoPeak.SetName("Shape along cone for tracked events (photo peak)");
  m_SpdPhotoPeak.SetAxis(AxisEpsilonDiff, AxisEpsilonReal, AxisEe);
  m_SpdPhotoPeak.SetAxisNames("SPD #epsilon_{meas} - #epsilon_{real} [deg]", "Measure electron scatter angle [deg]", "Measured electron energy [keV]");
  

  
  m_NMatchedEvents = 0;
  m_NOptimumEvents = 0;
  m_NQualifiedComptonEvents = 0;
  m_NPhotoPeakEvents = 0;
  
  m_NQualifiedComptonEventsWithTrack = 0;
  m_NPhotoPeakEventsWithTrack = 0;
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseImagingARM::Analyze() 
{ 
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  ++m_NMatchedEvents;
  
  MRawEventIncarnationList* REList = m_ReReader->GetRawEventList();
  
  if (REList->HasOnlyOptimumEvents() == true) {
    MRERawEvent* RE = REList->GetOptimumEvents()[0];
    MPhysicalEvent* Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
    if (Event != 0) {
      ++m_NOptimumEvents;
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          MComptonEvent* Compton = (MComptonEvent*) Event;
          
          if (Compton->IsKinematicsOK() == false) return true;
          ++m_NQualifiedComptonEvents;
          
          // Now get the ideal origin:
          if (m_SiEvent->GetNIAs() > 0) {
            MVector IdealOrigin = m_SiEvent->GetIAAt(0)->GetPosition();
            
            // Phi response:
            double PhiDiff = Compton->GetARMGamma(IdealOrigin)*c_Deg;
            
            //
            m_Arm.Add(PhiDiff, Compton->Phi()*c_Deg, Compton->Ei(), Compton->LeverArm(), Compton->SequenceLength());
            
            double IdealEnergy = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
            
            if (IdealEnergy >= RE->GetEnergy() - 3*RE->GetEnergyResolution() &&
              IdealEnergy <= RE->GetEnergy() + 3*RE->GetEnergyResolution()) {
              ++m_NPhotoPeakEvents;
              m_ArmPhotoPeak.Add(PhiDiff, Compton->Phi()*c_Deg, Compton->Ei(), Compton->LeverArm(), Compton->SequenceLength());
            }
            
            if (Compton->HasTrack() == true) {
              double EpsilonDiff = Compton->GetSPDElectron(IdealOrigin)*c_Deg;
              m_Spd.Add(EpsilonDiff, Compton->Epsilon()*c_Deg, Compton->Ee());
              m_NQualifiedComptonEventsWithTrack++;
              if (IdealEnergy >= RE->GetEnergy() - 3*RE->GetEnergyResolution() &&
                IdealEnergy <= RE->GetEnergy() + 3*RE->GetEnergyResolution()) {
                ++m_NPhotoPeakEventsWithTrack;
                m_SpdPhotoPeak.Add(EpsilonDiff, Compton->Epsilon()*c_Deg, Compton->Ee());
              }
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
bool MResponseImagingARM::Finalize() 
{ 
  cout<<"Statistics: "<<endl;
  cout<<"# matched events:            "<<m_NMatchedEvents<<endl;
  cout<<"# optimum events:            "<<m_NOptimumEvents<<endl;
  cout<<"# qualified Compton events:  "<<m_NQualifiedComptonEvents<<endl;
  cout<<"# photo peak events:         "<<m_NPhotoPeakEvents<<endl;

  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingARM::Save()
{
  MResponseBuilder::Save(); 
  
  m_Arm.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Arm.Write(GetFilePrefix() + ".allenergies" + m_Suffix, true);
  
  m_ArmPhotoPeak.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ArmPhotoPeak.Write(GetFilePrefix() + ".photopeak" + m_Suffix, true);
  
  m_Spd.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Spd.Write(GetFilePrefix() + ".spd.allenergies" + m_Suffix, true);
  
  m_SpdPhotoPeak.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_SpdPhotoPeak.Write(GetFilePrefix() + "spd.photopeak" + m_Suffix, true);
  
  return true;
}


// MResponseImagingARM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
