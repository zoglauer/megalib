/*
 * MResponseImaging.cxx
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
// MResponseImaging
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImaging.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TH2.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseImaging)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseImaging::MResponseImaging()
{
  // Construct an instance of MResponseImaging
}


////////////////////////////////////////////////////////////////////////////////


MResponseImaging::~MResponseImaging()
{
  // Delete this instance of MResponseImaging
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImaging::SetMimrecConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_MimrecCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImaging::SetRevanConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseImaging::OpenSimulationFile()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;

  MSettingsRevan RevanCfg(false);
  RevanCfg.Read(m_RevanCfgFileName);
  m_ReReader->SetSettings(&RevanCfg);

  MSettingsMimrec MimrecCfg(false);
  MimrecCfg.Read(m_MimrecCfgFileName);
  m_MimrecEventSelector.SetSettings(&MimrecCfg);

  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseImaging::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  cout<<"Generating imaging pdf"<<endl;

  vector<float> AxisPhiDiff;

  vector<float> Axis; // = CreateLogDist(0.1, 180, 20);
  Axis.push_back(0.5);
  Axis.push_back(1.0);
  Axis.push_back(1.5);
  Axis.push_back(2.0);
  Axis.push_back(2.5);
  Axis.push_back(3.0);
  Axis.push_back(4.0);
  Axis.push_back(5.0);
  Axis.push_back(6.0);
  Axis.push_back(8.0);
  Axis.push_back(10.0);
  Axis.push_back(15.0);
  Axis.push_back(20.0);
  Axis.push_back(30.0);
  Axis.push_back(40.0);
  Axis.push_back(60.0);
  Axis.push_back(90.0);
  Axis.push_back(180.0);

  // Add Inverted:
  for (unsigned int b = Axis.size()-1; b < Axis.size(); --b) {
    AxisPhiDiff.push_back(-Axis[b]);
  }
  AxisPhiDiff.push_back(0);
  for (unsigned int b = 0; b < Axis.size(); ++b) {
    AxisPhiDiff.push_back(Axis[b]);
  }

  // Simple Phi_Real/Phi_Meas-Response:
  vector<float> AxisPhiReal = CreateEquiDist(0, 180, 18);
  vector<float> AxisEi; // = CreateLogDist(500, 30000, 5);
  AxisEi.push_back(0);
  AxisEi.push_back(10000);
  AxisEi.push_back(30000);
  AxisEi.push_back(100000);

  MResponseMatrixO3 Phi("Phi", AxisPhiDiff, AxisPhiReal, AxisEi);
  Phi.SetAxisNames("Phi_{meas}-Phi_{real} [deg]", 
                   "Phi_{real} [deg]", 
                   "E_{i} [keV]");


  vector<float> AxisEpsilonDiff = CreateLogDist(0.1, 180, 30, 0);
  vector<float> AxisEpsilonReal = CreateEquiDist(0, 180, 1);
  vector<float> AxisEe = CreateLogDist(100, 20000, 20);

  MResponseMatrixO3 Epsilon("Epsilon", AxisEpsilonDiff, AxisEpsilonReal, AxisEe);
  Epsilon.SetAxisNames("Epsilon_{meas}-Epsilon_{real} [deg]", 
                       "Epsilon_{real} [deg]", 
                       "E_{e} [keV]");


  vector<float> AxisEnergy2;
  AxisEnergy2 = CreateLogDist(100, 20000, 100, 1, 100000);

  MResponseMatrixO2 Energy("Energy", AxisEnergy2, AxisEnergy2);
  Energy.SetAxisNames("ideal energy [keV]", 
                      "measured energy [keV]");

  vector<float> AxisFirstInteractionDistance;
  AxisFirstInteractionDistance = CreateEquiDist(0, 10, 10);

  MResponseMatrixO2 InteractionDistance("InteractionDistance", AxisFirstInteractionDistance, AxisFirstInteractionDistance);
  InteractionDistance.SetAxisNames("distance x_{meas} - x_{real} [cm]", 
                                   "distance y_{meas} - y_{real} [cm]");


  // efficiency corrected response
  vector<float> AxisPhiStart = CreateEquiDist(-180, 180, 36);
  vector<float> AxisThetaStart = CreateEquiDist(0, 180, 18);
  
  MResponseMatrixO2 Efficiency("EffectiveArea", AxisPhiStart, AxisThetaStart);
  Efficiency.SetAxisNames("Phi [deg]", "Theta [deg]");
  
  // Make StartDirNorm TH2D containing the normalization
  TH2D* StartDirNorm = new TH2D("Norm", "Norm", (int) AxisPhiStart.size()-1, &AxisPhiStart[0], (int) AxisThetaStart.size()-1, &AxisThetaStart[0]);
  for (int bx = 1; bx <= StartDirNorm->GetNbinsX(); ++bx) { 
    for (int by = 1; by <= StartDirNorm->GetNbinsY(); ++by) { 
      double Area = (StartDirNorm->GetXaxis()->GetBinUpEdge(bx) - StartDirNorm->GetXaxis()->GetBinLowEdge(bx))*c_Rad *
        (cos(StartDirNorm->GetYaxis()->GetBinLowEdge(by)*c_Rad) - cos(StartDirNorm->GetYaxis()->GetBinUpEdge(by)*c_Rad));
        cout<<"Area: "<<Area<<endl;
      StartDirNorm->SetBinContent(bx, by, 1.0/Area);  
    }
  }
  
  
  
  double PhiMeas;
  double PhiDiff;
  double Spd;
  MVector IdealOriginDir;

  MRawEventList* REList = 0;
  MPhysicalEvent* Event = 0;
  MComptonEvent* Compton = 0;

  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {
    REList = m_ReReader->GetRawEventList();

    if (REList->HasOptimumEvent() == true) {
      Event = REList->GetOptimumEvent()->GetPhysicalEvent();
      if (Event != 0) {
        if (m_MimrecEventSelector.IsQualifiedEvent(Event, false) == true) {
          if (Event->GetType() == MPhysicalEvent::c_Compton) {
            Compton = (MComptonEvent*) Event;

            if (Compton->IsKinematicsOK() == false) continue;
            PhiMeas = Compton->Phi()*c_Deg;
            
            // Now get the ideal origin:
            if (m_SiEvent->GetNIAs() > 0) {
              IdealOriginDir = m_SiEvent->GetIAAt(0)->GetPosition();
              
              // Phi response:
              PhiDiff = Compton->GetARMGamma(IdealOriginDir)*c_Deg;
              Phi.Add(PhiDiff, PhiMeas, Compton->Ei());
              
              // Epsilon response:
              if (Compton->HasTrack() == true) {
                Spd = Compton->GetSPDElectron(IdealOriginDir)*c_Deg;
                Epsilon.Add(Spd, 1, Compton->Ee());
              }
              
              // Energy response:
              Energy.Add(m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Compton->Ei());

              // normalized by steradian, scale according to binning??
              Efficiency.Add(IdealOriginDir.Phi()*c_Deg,
                                   IdealOriginDir.Theta()*c_Deg, 
                                   StartDirNorm->GetBinContent(StartDirNorm->GetXaxis()->FindBin(IdealOriginDir.Phi()*c_Deg), StartDirNorm->GetYaxis()->FindBin(IdealOriginDir.Theta()*c_Deg)));
              
              /*  // Steradian per bin normalization...
               delete [] m_AreaBin;
               m_AreaBin = new double[m_x2NBins];
               double TotalArea = 0.0;
               for (int x2 = 0; x2 < m_x2NBins; x2++) { // theta
               m_AreaBin[x2] = fabs(m_x1IntervalLength * (cos(m_x2Min + (x2+1)*m_x2IntervalLength) - cos(m_x2Min + x2*m_x2IntervalLength)));
               TotalArea += m_AreaBin[x2];
               m_AreaBin[x2] = m_AreaBin[x2];
               }
               mout<<"Image Area: "<<TotalArea*m_x1NBins<<" sr"<<endl;
               mout<<"Average Area: "<<TotalArea/m_x2NBins<<endl;
               */
              
              
              InteractionDistance.Add(Compton->C1().X() - IdealOriginDir.X(), Compton->C1().Y() - IdealOriginDir.Y());

              ++Counter;
            }
          }
        }
      }    
    }
    if (Counter % m_SaveAfter == 0) {
      Phi.Write(m_ResponseName + ".phi" + m_Suffix, true);
      Epsilon.Write(m_ResponseName + ".epsilon" + m_Suffix, true);
      Energy.Write(m_ResponseName + ".energy" + m_Suffix, true);
      InteractionDistance.Write(m_ResponseName + ".iadistance" + m_Suffix, true);
    }
  }  
  // Finally scale 
  if (m_ReaderFinished == true) {
    double Area = m_SiReader->GetSimulationStartAreaFarField();
    long Events = m_SiReader->GetSimulatedEvents();
    
    cout<<"Area: "<<Area<<"  Events: "<<Events<<endl;
    
    // A_eff = Area * [(Measured events in bin)/(Area in bin)] / [(All started events)/(4pi)]
    
    Efficiency *= Area/Events * 4*c_Pi;
    Efficiency.Write(m_ResponseName + ".effectivearea" + m_Suffix, true);
  } else {
    Efficiency.Write(m_ResponseName + ".effective_area_is_wrong_since_you_didnt_go_through_the_whole_sim_file" + m_Suffix, true); 
  }
  
  Phi.Write(m_ResponseName + ".phi" + m_Suffix, true);
  Epsilon.Write(m_ResponseName + ".epsilon" + m_Suffix, true);
  Energy.Write(m_ResponseName + ".energy" + m_Suffix, true);  
  InteractionDistance.Write(m_ResponseName + ".iadistance" + m_Suffix, true);

  cout<<"We had "<<Counter<<" good events"<<endl;
  
  return true;
}


// MResponseImaging.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
