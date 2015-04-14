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
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"
#include "MResponseMatrixO7.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO1.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseImagingBinnedMode)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseImagingBinnedMode::MResponseImagingBinnedMode()
{
  // Construct an instance of MResponseImagingBinnedMode
}


////////////////////////////////////////////////////////////////////////////////


MResponseImagingBinnedMode::~MResponseImagingBinnedMode()
{
  // Delete this instance of MResponseImagingBinnedMode
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImagingBinnedMode::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;
  
  cout<<"Generating binned mode imaging pdfs"<<endl;

  // Bin 1: Number of simulated events
  // Bin 2: Number of triggered
  vector<float> AxisNormalization;
  AxisNormalization.push_back(-0.5);
  AxisNormalization.push_back(0.5);
  AxisNormalization.push_back(1.5);

  int AngleBinWidth = 5;

  vector<float> AxisEnergyMeasured = CreateEquiDist(0, 2000, 1);
  vector<float> AxisPhi = CreateEquiDist(0, 180, 180/AngleBinWidth);
  vector<float> AxisChi = CreateEquiDist(-180, 180, 360/AngleBinWidth);
  vector<float> AxisPsi = CreateEquiDist(0, 180, 180/AngleBinWidth);

  vector<float> AxisEnergyInitial = CreateEquiDist(0, 2000, 1);
  vector<float> AxisLambda        = CreateEquiDist(-180, 180, 360/AngleBinWidth);
  vector<float> AxisNy            = CreateEquiDist(0, 180, 180/AngleBinWidth);

  vector<float> AxisEnergyElectron = CreateEquiDist(0, 2000, 200);
  vector<float> AxisEnergyTotal = CreateEquiDist(0, 2000, 200);

  vector<float> AxisEnergyGammaIdealIA = CreateEquiDist(0, 2000, 20);

  vector<float> AxisEnergyGamma = CreateEquiDist(0, 2000, 200);
  vector<float> AxisEnergyGammaIdeal = CreateEquiDist(0, 2000, 200);


  MResponseMatrixO7 Response("Binned response 7D", AxisLambda, AxisNy, AxisEnergyInitial, AxisPhi, AxisChi, AxisPsi, AxisEnergyMeasured);
  Response.SetAxisNames("#lambda [deg]", 
                        "#nu [deg]",
                        "Initial energy [keV]",
                        "#phi [deg]", 
                        "#chi [deg]", 
                        "#psi [deg]",
                        "Measured Energy [keV]");

  MResponseMatrixO1 Normalization("Normalization", AxisNormalization);

  MResponseMatrixO3 Exposure("Exposure", AxisLambda, AxisNy, AxisEnergyInitial);
  Exposure.SetAxisNames("#lambda [deg]", 
                        "#nu [deg]",
                        "Initial energy [keV]");

  MResponseMatrixO2 KleinNishinaResponse("ElectronEnergyResponse", AxisPhi, AxisEnergyInitial);
  KleinNishinaResponse.SetAxisNames("#varphi [deg]", "Total energy [keV]");

  MResponseMatrixO2 ElectronEnergyResponse("ElectronEnergyResponse", AxisEnergyElectron, AxisEnergyTotal);
  ElectronEnergyResponse.SetAxisNames("Measured electron energy [keV]", "Total energy [keV]");

  MResponseMatrixO3 ScatteredPhotonDir("Exposure", AxisChi, AxisPsi, AxisEnergyGammaIdealIA);
  ScatteredPhotonDir.SetAxisNames("#chi [deg]", 
                        "#psi [deg]",
                        "Ideal gamma energy [keV]");

  MResponseMatrixO2 GammaEnergyResponse("GammaEnergyResponse", AxisEnergyGamma, AxisEnergyGammaIdeal);
  ElectronEnergyResponse.SetAxisNames("Measured gamma energy [keV]", "Real gamma energy [keV]");

  int NSimulatedEvents = 0;

  double Phi;
  double Chi;
  double Psi;
  double EnergyElectron;
  double EnergyGamma;

  double Lambda;
  double EnergyInitial;
  double PhiGeo;
  double EnergyElectronInitial;

  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    NSimulatedEvents = m_SiEvent->GetSimulationEventID();
    REList = m_ReReader->GetRawEventList();
    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event, true) == true) {
          if (Event->GetType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;

            TMatrix Rotation = Compton->GetDetectorRotationMatrix();

            if (Compton->IsKinematicsOK() == false) continue;
            Phi = Compton->Phi()*c_Deg;
            MVector Dg = -Compton->Dg();
            Dg = Rotation*Dg;
            Chi = Dg.Phi()*c_Deg;
            while (Chi < -180) Chi += 360.0;
            while (Chi > +180) Chi -= 360.0;
            Psi = Dg.Theta()*c_Deg;
            EnergyElectron = Compton->Ee();
            EnergyGamma = Compton->Eg();

            //cout<<Phi<<":"<<Chi<<":"<<Psi<<endl;

            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = -m_SiEvent->GetIAAt(0)->GetSecondaryDirection();
              IdealOriginDir = Rotation*IdealOriginDir;
              Lambda = IdealOriginDir.Phi()*c_Deg;
              while (Lambda < -180) Lambda += 360.0;
              while (Lambda > +180) Lambda -= 360.0;
              EnergyInitial = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
              PhiGeo = IdealOriginDir.Angle(Dg)*c_Deg;

              //cout<<"PhiGeo: "<<PhiGeo<<endl;
              
              if (cos(PhiGeo*c_Rad)*EnergyInitial-EnergyInitial-c_E0 == 0) {
                continue;
              } else {
                EnergyElectronInitial = (EnergyInitial*EnergyInitial*(cos(PhiGeo*c_Rad)-1))/(cos(PhiGeo*c_Rad)*EnergyInitial-EnergyInitial-c_E0);
              }

              Normalization.Add(1);

              Response.Add(IdealOriginDir.Phi()*c_Deg,
                           IdealOriginDir.Theta()*c_Deg,
                           EnergyInitial,
                           Phi,
                           Chi,
                           Psi,
                           EnergyElectron+EnergyGamma);
              
              // Stimmt nicht ganz: Auch solche die keine zweite WW machen gehoren da rein:
              Exposure.Add(IdealOriginDir.Phi()*c_Deg,
                           IdealOriginDir.Theta()*c_Deg,
                           EnergyInitial);

              // ok 
              KleinNishinaResponse.Add(PhiGeo, EnergyInitial);

              // ok
              ElectronEnergyResponse.Add(EnergyElectron, EnergyElectronInitial);

              // ok
              ScatteredPhotonDir.Add(Chi, Psi, EnergyInitial - EnergyElectronInitial);

              // ok
              GammaEnergyResponse.Add(EnergyGamma, EnergyInitial - EnergyElectronInitial);

              ++Counter;
              if (Counter % m_SaveAfter == 0) {
                cout<<"Saving intermediate results..."<<endl;
                Normalization.SetBinContent(0, NSimulatedEvents);
                Normalization.Write(m_ResponseName + ".Normalization" + m_Suffix, true);
                Response.Write(m_ResponseName + ".Response" + m_Suffix, true);
                Exposure.Write(m_ResponseName + ".Exposure" + m_Suffix, true);
                KleinNishinaResponse.Write(m_ResponseName + ".KleinNishina" + m_Suffix, true);
                ElectronEnergyResponse.Write(m_ResponseName + ".ElectronEnergyResponse" + m_Suffix, true);
                ScatteredPhotonDir.Write(m_ResponseName + ".ScatteredPhotonDir" + m_Suffix, true);
                GammaEnergyResponse.Write(m_ResponseName + ".GammaEnergyResponse" + m_Suffix, true);
              }
            }
          }
        }
      }    
    }
  }  

  Normalization.SetBinContent(0, NSimulatedEvents);
  Normalization.Write(m_ResponseName + ".Normalization" + m_Suffix, true);
  Response.Write(m_ResponseName + ".Response" + m_Suffix, true);
  Exposure.Write(m_ResponseName + ".Exposure" + m_Suffix, true);
  KleinNishinaResponse.Write(m_ResponseName + ".KleinNishina" + m_Suffix, true);
  ElectronEnergyResponse.Write(m_ResponseName + ".ElectronEnergyResponse" + m_Suffix, true);
  ScatteredPhotonDir.Write(m_ResponseName + ".ScatteredPhotonDir" + m_Suffix, true);
  GammaEnergyResponse.Write(m_ResponseName + ".GammaEnergyResponse" + m_Suffix, true);

  return true;
}


// MResponseImagingBinnedMode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
