/* 
 * NuStarShield.cxx
 *
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
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

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <vector>
#include <algorithm>
#include <limits>
#include <iomanip>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TPolyLine3D.h>
#include <THStack.h>
#include <TPad.h>
#include <TLatex.h>

// MEGAlib
#include "MAssert.h"
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"

/******************************************************************************/

/******************************************************************************/

class EnergyPair : public pair<double, double>
{
public:
  EnergyPair(double Energy, int Rate, TString Material) { 
    m_Energy = Energy; m_Rate = Rate; m_Material = Material;
  }

  bool operator<(EnergyPair& Pair) {
    if (m_Rate > Pair.m_Rate) return true;
    return false;
  }

  double m_Energy;
  int m_Rate;
  TString m_Material;
};

/******************************************************************************/

class EnergyPairList
{
public:
  EnergyPairList() : m_Threshold(10), m_Limit(1), m_Accuracy(0.01) {}

  void SetThreshold(double Threshold) { m_Threshold = Threshold; }
  void SetLimit(int Limit) { m_Limit = Limit; }

  void Add(double Energy, TString Material) {
    if (Energy < m_Threshold) return;

    for (unsigned int i = 0; i < m_List.size(); ++i) {
      if (fabs(m_List[i].m_Energy - Energy) < m_Accuracy && m_List[i].m_Material == Material) {
        m_List[i].m_Rate += 1;
        return;
      }
    }
    m_List.push_back(EnergyPair(Energy, 1, Material));
  }

  unsigned int NEntries() const { return m_List.size(); }

  string Dump() {

    vector<EnergyPair> List = m_List;;

    // Count entries:
    double NEntries = 0;
    for (unsigned int i = 0; i < List.size(); ++i) {
      NEntries += List[i].m_Rate;
    }
    
    // Sort:
    sort(List.begin(), List.end());

    // Dump to string
    ostringstream out;
    out.setf(ios_base::fixed, ios_base::floatfield);
    out.precision(4);
    for (unsigned int i = 0; i < List.size(); ++i) {
      if (List[i].m_Rate > m_Limit) {
        out<<"DE "<<setw(12)<<List[i].m_Energy<<"  1.5   // N="
           <<setw(8)<<List[i].m_Rate<<" ("
           <<List[i].m_Material<<")"<<endl;
      }
    }
    return out.str();
  }

protected:
  vector<EnergyPair> m_List;
  double m_Threshold;
  int m_Limit;
  double m_Accuracy;
};


/******************************************************************************/


class NuStarShield
{
public:
  /// Default constructor
  NuStarShield();
  /// Default destructor
  ~NuStarShield();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  TString GetText(TString FileName);
  TString SafeString(TString String);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;
  /// Geometry
  TString m_GeometryFileName;
  /// Background file names:
  vector<TString> m_FileNames;
  /// Background time
  vector<double> m_Times;
  /// Background time
  double m_VetoThreshold;
  /// Output file
  TString m_OutputPrefix;
  ///
  double m_DetThresholdMin;
  double m_DetThresholdMax;
  ///
  double m_SpectrumMin;
  double m_SpectrumMax;
  int m_SpectrumBins;

  /// 
  double m_EnergyInterval;

  ///
  double m_Area;

  ///
  double m_Theta;
  double m_Phi;

  /// Create acceleration files
  bool m_Accelerate;
  /// Create Line-file
  bool m_StoreLines;

  ///
  MVector m_Pointing;
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
NuStarShield::NuStarShield() : m_Interrupt(false), 
                               m_VetoThreshold(-1), 
                               m_OutputPrefix("NuStarRates"), 
                               m_DetThresholdMin(5), 
                               m_DetThresholdMax(80),
                               m_SpectrumMin(5), 
                               m_SpectrumMax(80),
                               m_SpectrumBins(75),
                               m_Area(5.6064),
                               m_Theta(0.0),
                               m_Phi(0.0),
                               m_Accelerate(false),
                               m_StoreLines(false)
{
  gStyle->SetPalette(1, 0);
  //gROOT->SetBatch(true);

  m_EnergyInterval = m_DetThresholdMax - m_DetThresholdMin;
  m_Pointing.SetMagThetaPhi(1.0, m_Theta*c_Rad, m_Phi*c_Rad);
}


/******************************************************************************
 * Default destructor
 */
NuStarShield::~NuStarShield()
{
  // Intentionally left blanck
}


/******************************************************************************
 * Parse the command line
 */
bool NuStarShield::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: NuStarShield <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -b:   background file name + simulation time"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -v:   veto threshold"<<endl;
  Usage<<"         -d:   detector threshold (min, max)"<<endl;
  Usage<<"         -s:   spectral parameters (min, max, bins)"<<endl;
  Usage<<"         -a:   detector area"<<endl;
  Usage<<"         -p:   pointing"<<endl;
  Usage<<"         -l:   store lines"<<endl;
  Usage<<"         -o:   output file for rates"<<endl;
  Usage<<"         --accelerate:   generate accelertion sim file (i.e. generate sim with only events with detector hits)"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// First check if each option has sufficient arguments:
		// Single argument
    if (Option == "-g" || Option == "-v" || Option == "-o") {
			if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
				cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
			}
		} 
		// Multiple arguments template
		else if (Option == "-b" || Option == "-p") {
			if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
				cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
				cout<<Usage.str()<<endl;
				return false;
			}
		}
  }

  // Check for the output file to start logging: 
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// Then fulfill the options:
    if (Option == "-o") {
      m_OutputPrefix = argv[++i];
			cout<<"Accepting output file name: "<<m_OutputPrefix<<endl;
    }
  }

  mlog.Connect(m_OutputPrefix + ".log");

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
		Option = argv[i];

		// Then fulfill the options:
    if (Option == "-b") {
      m_FileNames.push_back(argv[++i]);
      if (MFile::Exists(m_FileNames.back()) == false) {
        mlog<<"Error: File does not exist: \""<<m_FileNames.back()<<"\"!"<<endl;
        return false;        
      }
      m_Times.push_back(atof(argv[++i]));
			mlog<<"Accepting background file name: "<<m_FileNames.back()<<" and time "<<m_Times.back()<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
			mlog<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else if (Option == "-v") {
      m_VetoThreshold = atof(argv[++i]);
			mlog<<"Accepting veto threshold: "<<m_VetoThreshold<<endl;
    } else if (Option == "-d") {
      m_DetThresholdMin = atof(argv[++i]);
      m_DetThresholdMax = atof(argv[++i]);
			mlog<<"Accepting detector thresholds: "<<m_DetThresholdMin<<", "<<m_DetThresholdMax<<endl;
    } else if (Option == "-s") {
      m_SpectrumMin = atof(argv[++i]);
      m_SpectrumMax = atof(argv[++i]);
      m_SpectrumBins = atoi(argv[++i]);
			mlog<<"Accepting spectral parameters: "<<m_SpectrumMin<<", "<<m_SpectrumMax<<", "<<m_SpectrumBins<<endl;
    } else if (Option == "-p") {
      m_Theta = atof(argv[++i]);
      m_Phi = atof(argv[++i]);
			mlog<<"Accepting pointing: "<<m_Theta<<" "<<m_Phi<<endl;
    } else if (Option == "-a") {
      m_Area = atof(argv[++i]);
			mlog<<"Accepting detector area: "<<m_Area<<endl;
    } else if (Option == "-l") {
      m_StoreLines = true;
			mlog<<"Storing line data"<<endl;
    } else if (Option == "-o") {
      ++i;
		} else {
			mlog<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
			mlog<<Usage.str()<<endl;
			return false;
		}
  }

  if (m_VetoThreshold == -1) {
    mlog<<"Error: Need a veto threshold !"<<endl;
    mlog<<Usage.str()<<endl;
    return false;    
  }

  if (m_GeometryFileName == "") {
    mlog<<"Error: Need a geometry file!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;    
  }

  if (m_FileNames.size() == 0) {
    mlog<<"Error: Need at least one background file!"<<endl;
    mlog<<Usage.str()<<endl;
    return false;    
  }

  m_Pointing.SetMagThetaPhi(1.0, m_Theta*c_Rad, m_Phi*c_Rad);

  // Check if prefix exists, if yes, append date
  if (MFile::Exists(m_OutputPrefix + ".thres") == true) {
    MTime Now;
    m_OutputPrefix += TString(".") + Now.GetShortString();
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */

bool MHistCompare(TH1D* H1, TH1D* H2) {
  return (H1->Integral() < H2->Integral());
}

bool NuStarShield::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();

  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    mlog<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
  } else {
    mlog<<"Loading of geometry "<<Geometry->GetName()<<" failed!!"<<endl;
    return false;
  }  

  // List of pairs:
  EnergyPairList List;
  List.SetLimit(0);
  List.SetThreshold(0);

  vector<double> Thresholds;
  Thresholds.push_back(0);
  Thresholds.push_back(5);
  Thresholds.push_back(10);
  Thresholds.push_back(15);
  Thresholds.push_back(25);
  Thresholds.push_back(50);
  Thresholds.push_back(75);
  Thresholds.push_back(100);
  Thresholds.push_back(125);
  Thresholds.push_back(150);
  Thresholds.push_back(200);
  Thresholds.push_back(250);
  Thresholds.push_back(500);
  Thresholds.push_back(1000);
  Thresholds.push_back(2000);
  Thresholds.push_back(5000);
  Thresholds.push_back(10000);
  Thresholds.push_back(20000);
  Thresholds.push_back(50000);
  Thresholds.push_back(100000);
  Thresholds.push_back(1e50);

  vector<int> ShieldCountsByThreshold(Thresholds.size(), 0);
  vector<int> DetectorCountsByThreshold(Thresholds.size(), 0);

  const int eNBins = m_SpectrumBins;
  double eMin = m_SpectrumMin;
  double eMax = m_SpectrumMax;

  // Log:
//   if (eMin <= 0) eMin = 1E-9;
//   eMin = log(eMin);
//   eMax = log(eMax);
//   float eDist = (eMax-eMin)/(eNBins);
//   double* eBins = new double[eNBins+1];
//   for (int i = 0; i < eNBins+1; ++i) {
//     eBins[i] = exp(eMin+i*eDist);
//   }

  // Lin:
  float eDist = (eMax-eMin)/(eNBins);
  double* eBins = new double[eNBins+1];
  for (int i = 0; i < eNBins+1; ++i) {
    eBins[i] = eMin+i*eDist;
  }

  const int rNBins = 40;
  double rMin = 5;
  double rMax = 50000;

  if (rMin <= 0) rMin = 1E-9;
  rMin = log(rMin);
  rMax = log(rMax);
  float rDist = (rMax-rMin)/(rNBins);
  double* rBins = new double[rNBins+1];
  for (int i = 0; i < rNBins+1; ++i) {
    rBins[i] = exp(rMin+i*rDist);
  }

  TH1D* AperturePhotonsTotal = new TH1D("AperturePhotonsSpectrum", "Aperture photons spectrum", eNBins, eBins);
  AperturePhotonsTotal->SetStats(false);
  AperturePhotonsTotal->SetFillColor(8);
  AperturePhotonsTotal->SetXTitle("Deposited Energy [keV]");
  AperturePhotonsTotal->SetYTitle("cts/sec/keV/cm^{2}");

  TH1D* AperturePhotonsZenithAngle = new TH1D("AperturePhotonsZenithAngle", "Aperture photons zenith angle", 180, 0, 180);
  AperturePhotonsZenithAngle->SetStats(false);
  AperturePhotonsZenithAngle->SetFillColor(8);
  AperturePhotonsZenithAngle->SetXTitle("Azimuthal angle [deg]");
  AperturePhotonsZenithAngle->SetYTitle("cts/sr");

  TH1D* BackgroundDetectorPassiveShieldTotal = new TH1D("DetectorBackgroundSpectrumPassiveShield)", "Detector background spectrum (passive shield)", eNBins, eBins);
  BackgroundDetectorPassiveShieldTotal->SetStats(false);
  BackgroundDetectorPassiveShieldTotal->SetFillColor(8);
  BackgroundDetectorPassiveShieldTotal->SetXTitle("Deposited Energy [keV]");
  BackgroundDetectorPassiveShieldTotal->SetYTitle("cts/sec/keV/cm^{2}");
  
  TH1D* BackgroundDetectorActiveShieldTotal = new TH1D("DetectorBackgroundSpectrum", "Detector background spectrum", eNBins, eBins);
  BackgroundDetectorActiveShieldTotal->SetStats(false);
  BackgroundDetectorActiveShieldTotal->SetFillColor(8);
  BackgroundDetectorActiveShieldTotal->SetXTitle("Deposited Energy [keV]");
  BackgroundDetectorActiveShieldTotal->SetYTitle("cts/sec/keV/cm^{2}");
  
  TH1D* ZenitAngleDetectorActiveShieldTotal = new TH1D("OriginalZenithAngle", "Original zenith angle", 60, 0 , 180);
  ZenitAngleDetectorActiveShieldTotal->SetStats(false);
  ZenitAngleDetectorActiveShieldTotal->SetFillColor(8);
  ZenitAngleDetectorActiveShieldTotal->SetXTitle("Azimuthal angle [deg]");
  ZenitAngleDetectorActiveShieldTotal->SetYTitle("cts/sr");
  
  TH1D* BackgroundShieldTotal = new TH1D("ShieldBackgroundSpectrum", "Shield background spectrum", eNBins, eBins);
  BackgroundShieldTotal->SetStats(false);
  BackgroundShieldTotal->SetFillColor(8);
  BackgroundShieldTotal->SetXTitle("Deposited Energy [keV]");
  BackgroundShieldTotal->SetYTitle("cts/sec/keV/cm^{2}");
  
  TH1D* CollimatorFirstTotal = new TH1D("ShieldBackgroundSpectrum", "Shield background spectrum", eNBins, eBins);
  CollimatorFirstTotal->SetStats(false);
  CollimatorFirstTotal->SetFillColor(8);
  CollimatorFirstTotal->SetXTitle("Deposited Energy [keV]");
  CollimatorFirstTotal->SetYTitle("cts/sec/keV/cm^{2}");
  
  double ShieldCountsTotalRate = 0;
  double ShieldCountsAboveThresholdTotalRate = 0;
  double DetectorCountsActiveShieldTotalRate = 0;
  double DetectorCountsPassiveShieldTotalRate = 0;
  double AperturePhotonRate = 0;  

  double ShieldCountsTotalRateError = 0;
  double ShieldCountsAboveThresholdTotalRateError = 0;
  double DetectorCountsActiveShieldTotalRateError = 0;
  double DetectorCountsPassiveShieldTotalRateError = 0;

  ofstream fthres;
  fthres.open((m_OutputPrefix + ".thres").Data());

  ofstream fout;
  fout.open((m_OutputPrefix + ".csv").Data());

  fout<<"Type;Sim time [s];Shield counts [cts];Shield rate [cts/s];Shield Counts above veto threshold [cts];Shield rate above veto threshold [cts/s];Detector counts passive shield [cts];Detector rate passive shield [cts/s];Detector counts active shield [cts];Detector rate active shield [cts/s]"<<endl;
    
  TCanvas* GeoView = new TCanvas("Geometry", "Geometry", 400, 800);
  GeoView->cd();
  Geometry->DrawGeometry(GeoView);
  GeoView->Update();

  vector<TH1D*> BackgroundDetectorActiveShieldList;
  vector<TH1D*> ZenitAngleDetectorActiveShieldList;
  vector<TH1D*> BackgroundDetectorPassiveShieldList;
  vector<TH1D*> BackgroundShieldList;

  for (unsigned int i = 0; i < m_FileNames.size(); ++i) {

    MFileEventsSim SiReader(Geometry);
    if (SiReader.Open(m_FileNames[i]) == false) {
      mlog<<"Unable to open sim file!"<<endl; 
      return false;
    }
    SiReader.ShowProgress();
    
    int ShieldCounts = 0;
    int ShieldCountsAboveThreshold = 0;
    int DetectorCountsActiveShield = 0;
    int DetectorCountsPassiveShield = 0;
    int AperturePhotonCounts = 0;
    int CollimatorFirstCounts = 0;

    TString ID(" - ");
    //ID += i;
    ID += m_FileNames[i];

    TH1D* BackgroundDetectorPassiveShield = new TH1D(SafeString(TString("Detector background spectrum (passive shield)") + ID), 
                                                     TString("Detector background spectrum (passive shield)") + ID, eNBins, eBins);
    BackgroundDetectorPassiveShield->SetStats(false);
    BackgroundDetectorPassiveShield->SetXTitle("Deposited Energy [keV]");
    BackgroundDetectorPassiveShield->SetYTitle("cts/keV/sec/cm^{2}");
    BackgroundDetectorPassiveShieldList.push_back(BackgroundDetectorPassiveShield);

    TH1D* BackgroundDetectorActiveShield = new TH1D(SafeString(TString("Detector") + ID), 
                                                    TString("Detector") + ID, eNBins, eBins);
    BackgroundDetectorActiveShield->SetStats(false);
    BackgroundDetectorActiveShield->SetXTitle("Deposited Energy [keV]");
    BackgroundDetectorActiveShield->SetYTitle("cts/keV/sec/cm^{2}");
    BackgroundDetectorActiveShieldList.push_back(BackgroundDetectorActiveShield);

    TH1D* ZenitAngleDetectorActiveShield = new TH1D(SafeString(TString("Original zenith angle") + ID), 
                                                    TString("Original zenith angle") + ID, 60, 0, 180);
    ZenitAngleDetectorActiveShield->SetStats(false);
    ZenitAngleDetectorActiveShield->SetXTitle("Azimuthal angle [deg]");
    ZenitAngleDetectorActiveShield->SetYTitle("cts/sr");
    ZenitAngleDetectorActiveShieldList.push_back(ZenitAngleDetectorActiveShield);

    TH1D* BackgroundShield = new TH1D(SafeString(TString("Shield background spectrum") + ID), 
                                      TString("Shield background spectrum") + ID, eNBins, eBins);
    BackgroundShield->SetStats(false);
    BackgroundShield->SetXTitle("Deposited Energy [keV]");
    BackgroundShield->SetYTitle("cts/keV/sec/cm^{2}");
    BackgroundShieldList.push_back(BackgroundShield);

    TH1D* AperturePhotons = new TH1D(SafeString(TString("Aperture photon spectrum") + ID), 
                                     TString("Aperture photon spectrum") + ID, eNBins, eBins);
    AperturePhotons->SetStats(false);
    AperturePhotons->SetXTitle("Deposited Energy [keV]");
    AperturePhotons->SetYTitle("cts/keV/sec/cm^{2}");

    TH2D* EnergyResponse = new TH2D(SafeString(TString("Energy Response") + ID), 
                                    TString("Energy Response") + ID, rNBins, rBins, rNBins, rBins);
    AperturePhotons->SetStats(false);
    AperturePhotons->SetXTitle("Initial Energy [keV]");
    AperturePhotons->SetXTitle("Deposited Energy [keV]");
    AperturePhotons->SetYTitle("cts");

    MSimEvent* SiEvent = 0;
    while ((SiEvent = SiReader.GetNextEvent(false)) != 0) {

      // Check ACS:
      bool HasVetoed = false;
      double ACSDeposit = 0;
      double ACSSmallestDeposit = numeric_limits<double>::max();
      for (unsigned int h = 0; h < SiEvent->GetNHTs(); ++h) {
        if (SiEvent->GetHTAt(h)->GetDetector() == 4) {
          ACSDeposit += SiEvent->GetHTAt(h)->GetEnergy();
          BackgroundShield->Fill(SiEvent->GetHTAt(h)->GetEnergy());
          if (ACSDeposit == 0) {
            ShieldCounts++;
          }
          if (SiEvent->GetHTAt(h)->GetEnergy() < ACSSmallestDeposit) {
            ACSSmallestDeposit = SiEvent->GetHTAt(h)->GetEnergy();
          }
          if (SiEvent->GetHTAt(h)->GetEnergy() >= m_VetoThreshold) {
            HasVetoed = true;
          }
        }
      }
      if (HasVetoed == true) {
        ShieldCountsAboveThreshold++;
      }
      if (ACSSmallestDeposit == numeric_limits<double>::max()) {
        ACSSmallestDeposit = 0;
      }


      for (unsigned int i = 0; i < Thresholds.size(); ++i) {
        if (ACSSmallestDeposit > Thresholds[i]) {
          ShieldCountsByThreshold[i]++;
        }
      }
  
      // Detector only count hits within the threshold:
      double Deposit = 0;
      for (unsigned int h = 0; h < SiEvent->GetNHTs(); ++h) {
        if (SiEvent->GetHTAt(h)->GetDetector() == 1 && 
            SiEvent->GetHTAt(h)->GetEnergy() > m_DetThresholdMin) {
          Deposit += SiEvent->GetHTAt(h)->GetEnergy();
        }
      }
      bool DepositWithinEnergyRange = false;
      if (Deposit >= m_DetThresholdMin && Deposit <= m_DetThresholdMax) {
        DepositWithinEnergyRange = true;
      }


      // Don't add PHOTON hits, which have their first (triggered) interaction in active material
      bool IsApperaturPhoton = false;
      if (SiEvent->GetNIAs() > 1) {
        // It has to be a photon and the first interaction has to be in CZT material:
        if (SiEvent->GetIAAt(0)->GetParticleNumber() == 1 && SiEvent->GetIAAt(0)->GetType() == "INIT") {
          // Detector only count hits within the threshold:
          if (DepositWithinEnergyRange == true && HasVetoed == false) {
            // Check if we went through passive material:

            MVector IntersectionPosition = SiEvent->GetIAAt(1)->GetPosition();

            MDVolumeSequence* VS = 0;
            for (unsigned int h = 0; h < SiEvent->GetNHTs(); ++h) {
              if (SiEvent->GetHTAt(h)->GetDetector() == 1 && 
                  SiEvent->GetHTAt(h)->GetEnergy() > m_DetThresholdMin) {
                VS = SiEvent->GetHTAt(h)->GetVolumeSequence();
              }
            }
            if (VS == 0) {
              merr<<"Something bad is wrong - we should have a volume sequence at this point!"<<endl;
            } else {
              MVector PlaneNormal = m_Pointing;
              PlaneNormal = VS->GetRotationInFirstVolume(VS->GetSensitiveVolume())*PlaneNormal;
              MVector PlanePosition = VS->GetPositionInFirstVolume(MVector(0, 0, 0), VS->GetSensitiveVolume());

              MVector LineDirection = SiEvent->GetIAAt(0)->GetDirection();
              MVector LinePosition = SiEvent->GetIAAt(0)->GetPosition();
              
              if (PlaneNormal.Dot(LineDirection) != 0) {
                double Lambda = PlaneNormal.Dot(PlanePosition-LinePosition)/PlaneNormal.Dot(LineDirection);
                IntersectionPosition = LinePosition + Lambda*LineDirection;
              }
            }

            map<MDMaterial*, double> Lengths = Geometry->GetPathLengths(SiEvent->GetIAAt(0)->GetPosition(), IntersectionPosition);
            map<MDMaterial*, double>::iterator Iter;
    
            IsApperaturPhoton = true;
            bool FoundCZT = false;
            for (Iter = Lengths.begin(); Iter != Lengths.end(); ++Iter) {
              // The only allowed volumes are vaccum and CZT:
              //mlog<<(*Iter).first->GetName()<<"!"<<(*Iter).second<<endl;
              if ((*Iter).first->GetName() != "vacuum" && (*Iter).first->GetName() != "czt" && (*Iter).first->GetName() != "Be") {
                IsApperaturPhoton = false;
                break;
              }
              if ((*Iter).first->GetName() == "czt") {
                FoundCZT = true;
                break;
              }
            }
            if (FoundCZT == false) {
              IsApperaturPhoton = false;
            }
          }
        }
      }

      // Now check if the first scatter is in the collimator:
      bool IsFirstInteractionCollimator = false;
      if (SiEvent->GetNIAs() > 1) {

        MDVolumeSequence* VS = 0;
        VS = Geometry->GetVolumeSequencePointer(SiEvent->GetIAAt(1)->GetPosition());

        if (VS->GetDeepestVolume()->GetName() == "COL1a" ||
            VS->GetDeepestVolume()->GetName() == "COL1b" ||
            VS->GetDeepestVolume()->GetName() == "COL1c" ||
            VS->GetDeepestVolume()->GetName() == "COL2a" ||
            VS->GetDeepestVolume()->GetName() == "COL2b" ||
            VS->GetDeepestVolume()->GetName() == "COL2c" ||
            VS->GetDeepestVolume()->GetName() == "COL3a" ||
            VS->GetDeepestVolume()->GetName() == "COL3b" ||
            VS->GetDeepestVolume()->GetName() == "COL3c") {
          mlog<<"First col... "<<SiEvent->GetEventNumber()<<endl;
          IsFirstInteractionCollimator = true;
        }
        delete VS;
      }

      if (IsApperaturPhoton == true) {
        AperturePhotonCounts++;
        AperturePhotons->Fill(Deposit);
        AperturePhotonsZenithAngle->Fill(SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg);
        if (SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg > 5.5) {
          mlog<<"Is aparature photon: "<<SiEvent->GetEventNumber()<<" "<<SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg<<endl;
        }
      } else {
        
        if (DepositWithinEnergyRange == true) {
          if (HasVetoed == false) {
            BackgroundDetectorActiveShield->Fill(Deposit); 
            ZenitAngleDetectorActiveShield->Fill(SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg);
            if (SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg < 5.5) {
              mlog<<"No aparture but small angle: "<<SiEvent->GetEventNumber()<<" "<<SiEvent->GetIAAt(0)->GetDirection().Angle(-m_Pointing)*c_Deg<<endl;
            }
            DetectorCountsActiveShield++;
            EnergyResponse->Fill(SiEvent->GetIAAt(0)->GetEnergy(), Deposit);
            List.Add(Deposit, Geometry->GetMaterial(SiEvent->GetIAAt(0)->GetPosition())->GetName());

            if (IsFirstInteractionCollimator == true) {
              CollimatorFirstCounts++;
              CollimatorFirstTotal->Fill(Deposit);          
              mlog<<"CollimatorFirst: "<<SiEvent->GetEventNumber()<<endl;
            }
          } 
          BackgroundDetectorPassiveShield->Fill(Deposit);          
          DetectorCountsPassiveShield++;

          for (unsigned int i = 0; i < Thresholds.size(); ++i) {
            if (ACSSmallestDeposit < Thresholds[i]) {
              DetectorCountsByThreshold[i]++;
            }
          }          
        }
        
        // Draw if we have a not vetoed hit above the threshold
        if (DepositWithinEnergyRange == true && HasVetoed == false) {
          mlog<<"Drawing: "<<SiEvent->GetEventNumber()<<" E="<<Deposit<<endl;
          GeoView->cd();
          TPolyLine3D* l = new TPolyLine3D();
          l->SetNextPoint(SiEvent->GetIAAt(0)->GetPosition().X(), 
                          SiEvent->GetIAAt(0)->GetPosition().Y(),
                          SiEvent->GetIAAt(0)->GetPosition().Z());
          for (unsigned int a = 1; a < SiEvent->GetNIAs(); ++a) {
            l->SetNextPoint(SiEvent->GetIAAt(a)->GetPosition().X(), 
                            SiEvent->GetIAAt(a)->GetPosition().Y(),
                            SiEvent->GetIAAt(a)->GetPosition().Z());
          }
          l->Draw();
        }
      }
        
      delete SiEvent;
    }

    // Normalize: cts/sec/keV/cm2 and add to total
    for (int b = 1; b <= BackgroundDetectorPassiveShield->GetNbinsX(); ++b) {
      BackgroundDetectorPassiveShield->SetBinContent(b, BackgroundDetectorPassiveShield->GetBinContent(b)/BackgroundDetectorPassiveShield->GetBinWidth(b)/m_Times[i]/m_Area);
      BackgroundDetectorPassiveShieldTotal->SetBinContent(b, BackgroundDetectorPassiveShield->GetBinContent(b)+
                                                          BackgroundDetectorPassiveShieldTotal->GetBinContent(b));
    }


    for (int b = 1; b <= BackgroundDetectorActiveShield->GetNbinsX(); ++b) {
      BackgroundDetectorActiveShield->SetBinContent(b, BackgroundDetectorActiveShield->GetBinContent(b)/BackgroundDetectorActiveShield->GetBinWidth(b)/m_Times[i]/m_Area);
      BackgroundDetectorActiveShieldTotal->SetBinContent(b, BackgroundDetectorActiveShield->GetBinContent(b)+
                                                         BackgroundDetectorActiveShieldTotal->GetBinContent(b));
    }
    for (int b = 1; b <= ZenitAngleDetectorActiveShield->GetNbinsX(); ++b) {
//       double Area = 2*c_Pi*(cos(ZenitAngleDetectorActiveShield->GetBinLowEdge(b)*c_Rad) - 
//                             cos(ZenitAngleDetectorActiveShield->GetBinLowEdge(b)*c_Rad + ZenitAngleDetectorActiveShield->GetBinWidth(b)*c_Rad));
      double Area = 1;
      ZenitAngleDetectorActiveShield->SetBinContent(b, ZenitAngleDetectorActiveShield->GetBinContent(b)/Area/m_Times[i]);
      ZenitAngleDetectorActiveShieldTotal->SetBinContent(b, ZenitAngleDetectorActiveShield->GetBinContent(b)+
                                                         ZenitAngleDetectorActiveShieldTotal->GetBinContent(b));
    }
    for (int b = 1; b <= BackgroundShield->GetNbinsX(); ++b) {
      BackgroundShield->SetBinContent(b, BackgroundShield->GetBinContent(b)/BackgroundShield->GetBinWidth(b)/m_Times[i]/m_Area);
      BackgroundShieldTotal->SetBinContent(b, BackgroundShield->GetBinContent(b)+
                                           BackgroundShieldTotal->GetBinContent(b));
    }
    for (int b = 1; b <= CollimatorFirstTotal->GetNbinsX(); ++b) {
      CollimatorFirstTotal->SetBinContent(b, CollimatorFirstTotal->GetBinContent(b)/CollimatorFirstTotal->GetBinWidth(b)/m_Times[i]/m_Area);
    }
    for (int b = 1; b <= AperturePhotons->GetNbinsX(); ++b) {
      AperturePhotons->SetBinContent(b, AperturePhotons->GetBinContent(b)/AperturePhotons->GetBinWidth(b)/m_Times[i]/m_Area);
      AperturePhotonsTotal->SetBinContent(b, AperturePhotons->GetBinContent(b)+
                                                          AperturePhotonsTotal->GetBinContent(b));
    }
    
    
    SiReader.Close();
    
    //     TCanvas* BackgroundDetectorPassiveShieldCanvas = new TCanvas();
    //     BackgroundDetectorPassiveShieldCanvas->cd();
    //     BackgroundDetectorPassiveShieldCanvas->SetLogx();
    //     BackgroundDetectorPassiveShieldCanvas->SetLogy();
    //     BackgroundDetectorPassiveShield->Draw();
    //     BackgroundDetectorPassiveShieldCanvas->Update();
    
    TCanvas* BackgroundDetectorActiveShieldCanvas = new TCanvas("BackgroundActiveShield");
    BackgroundDetectorActiveShieldCanvas->cd();
    BackgroundDetectorActiveShieldCanvas->SetLogx();
    BackgroundDetectorActiveShieldCanvas->SetLogy();
    BackgroundDetectorActiveShield->Draw();
    BackgroundDetectorActiveShieldCanvas->Update();
    BackgroundDetectorActiveShieldCanvas->SaveAs(m_OutputPrefix + "_" + GetText(m_FileNames[i]).ReplaceAll(" ", "_") + "_BackgroundDetectorActiveShield.C");

    TCanvas* EnergyResponseCanvas = new TCanvas("EnergyResponse");
    EnergyResponseCanvas->cd();
    EnergyResponseCanvas->SetLogx();
    EnergyResponseCanvas->SetLogy();
    EnergyResponse->Draw("colz");
    EnergyResponseCanvas->Update();
    EnergyResponseCanvas->SaveAs(m_OutputPrefix + "_" + GetText(m_FileNames[i]).ReplaceAll(" ", "_") + "_EnergyResponse.C");

    ShieldCountsTotalRate += ShieldCounts/m_Times[i]/m_EnergyInterval/m_Area;
    ShieldCountsAboveThresholdTotalRate += ShieldCountsAboveThreshold/m_Times[i]/m_EnergyInterval/m_Area;
    DetectorCountsActiveShieldTotalRate += DetectorCountsActiveShield/m_Times[i]/m_EnergyInterval/m_Area;
    DetectorCountsPassiveShieldTotalRate += DetectorCountsPassiveShield/m_Times[i]/m_EnergyInterval/m_Area;
    AperturePhotonRate += AperturePhotonCounts/m_Times[i]/m_EnergyInterval/m_Area;

    ShieldCountsTotalRateError += ShieldCounts/m_Times[i]/m_Times[i]/m_EnergyInterval/m_Area;
    ShieldCountsAboveThresholdTotalRateError += ShieldCountsAboveThreshold/m_Times[i]/m_Times[i]/m_EnergyInterval/m_Area;
    DetectorCountsActiveShieldTotalRateError += DetectorCountsActiveShield/m_Times[i]/m_Times[i]/m_EnergyInterval/m_Area;
    DetectorCountsPassiveShieldTotalRateError += DetectorCountsPassiveShield/m_Times[i]/m_Times[i]/m_EnergyInterval/m_Area;

    ostringstream out;
    out<<endl;
    out<<"Summary file "<<m_FileNames[i]<<endl;
    out<<endl;
    out<<"Count rate shield:                                "<<ShieldCounts<<" cts -> "<<ShieldCounts/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/sec"<<endl;
    out<<"Count rate shield above threshold:                "<<ShieldCountsAboveThreshold<<" cts -> "<<ShieldCountsAboveThreshold/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/sec"<<endl;
    out<<"Detector rate passive shield ("<<m_DetThresholdMin<<"-"<<m_DetThresholdMax<<" keV): "<<DetectorCountsPassiveShield<<" cts -> "<<DetectorCountsPassiveShield/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/cm2"<<endl;
    out<<"Detector rate active shield ("<<m_DetThresholdMin<<"-"<<m_DetThresholdMax<<" keV):  "<<DetectorCountsActiveShield<<" cts -> "<<DetectorCountsActiveShield/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/cm2"<<endl;
    out<<"Count rate apperature:                             "<<AperturePhotonCounts<<" cts -> "<<AperturePhotonCounts/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/sec"<<endl;
    out<<"Count collimator first:                           "<<CollimatorFirstCounts<<" cts -> "<<CollimatorFirstCounts/m_Times[i]/m_EnergyInterval/m_Area<<" cts/sec/keV/sec"<<endl;
    out<<endl;
    mlog<<out.str()<<endl;

    fout<<GetText(m_FileNames[i])<<";"<<m_Times[i]<<";"<<ShieldCounts<<";"<<ShieldCounts/m_Times[i]/m_EnergyInterval/m_Area<<";"
        <<ShieldCountsAboveThreshold<<";"<<ShieldCountsAboveThreshold/m_Times[i]/m_EnergyInterval/m_Area<<";"
        <<DetectorCountsPassiveShield<<";"<<DetectorCountsPassiveShield/m_Times[i]/m_EnergyInterval/m_Area<<";"
        <<DetectorCountsActiveShield<<";"<<DetectorCountsActiveShield/m_Times[i]/m_EnergyInterval/m_Area<<endl;
    fthres<<GetText(m_FileNames[i])<<endl;
    fthres<<endl;    
    fthres<<out.str()<<endl;
    fthres<<endl;
    for (unsigned int i = 0; i < Thresholds.size(); ++i) {
      fthres<<Thresholds[i]<<"  "<<ShieldCountsByThreshold[i]<<"  "<<DetectorCountsByThreshold[i]<<endl;
    }
    fthres<<endl;
  }
   
  // Show all total canvases:
 
  TCanvas* BackgroundDetectorPassiveShieldTotalCanvas = new TCanvas(SafeString("Detector background spectrum (passive shield)"));
  BackgroundDetectorPassiveShieldTotalCanvas->cd();
  BackgroundDetectorPassiveShieldTotalCanvas->SetLogx();
  BackgroundDetectorPassiveShieldTotalCanvas->SetLogy();
  BackgroundDetectorPassiveShieldTotal->Draw();
  BackgroundDetectorPassiveShieldTotalCanvas->Update();
  BackgroundDetectorPassiveShieldTotalCanvas->SaveAs(SafeString(m_OutputPrefix + "_BackgroundDetectorPassiveShieldTotal.C"));
    
  TCanvas* BackgroundDetectorActiveShieldTotalCanvas = new TCanvas(SafeString("Detector background spectrum"));
  BackgroundDetectorActiveShieldTotalCanvas->cd();
  BackgroundDetectorActiveShieldTotalCanvas->SetLogx();
  BackgroundDetectorActiveShieldTotalCanvas->SetLogy();
  BackgroundDetectorActiveShieldTotal->Draw();
  BackgroundDetectorActiveShieldTotalCanvas->Update();
  BackgroundDetectorActiveShieldTotalCanvas->SaveAs(SafeString(m_OutputPrefix + "_BackgroundDetectorActiveShieldTotal.C"));
    
//   TCanvas* ZenitAngleDetectorActiveShieldTotalCanvas = new TCanvas("Zenith angle distribution");
//   ZenitAngleDetectorActiveShieldTotalCanvas->cd();
//   ZenitAngleDetectorActiveShieldTotal->Draw();
//   ZenitAngleDetectorActiveShieldTotalCanvas->Update();
//   ZenitAngleDetectorActiveShieldTotalCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetectorActiveShieldTotal.gif");
//   ZenitAngleDetectorActiveShieldTotalCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetectorActiveShieldTotal.eps");
//   ZenitAngleDetectorActiveShieldTotalCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetectorActiveShieldTotal.root");
  
  TCanvas* BackgroundShieldTotalCanvas = new TCanvas();
  BackgroundShieldTotalCanvas->cd();
  BackgroundShieldTotalCanvas->SetLogx();
  BackgroundShieldTotalCanvas->SetLogy();
  BackgroundShieldTotal->Draw();
  BackgroundShieldTotalCanvas->Update();
  BackgroundShieldTotalCanvas->SaveAs(SafeString(m_OutputPrefix + "_BackgroundShieldTotalCanvas.C"));
    
  TCanvas* AperturePhotonsTotalCanvas = new TCanvas("Aperature photon spectrum");
  AperturePhotonsTotalCanvas->cd();
  AperturePhotonsTotalCanvas->SetLogx();
  AperturePhotonsTotalCanvas->SetLogy();
  AperturePhotonsTotal->Draw();
  AperturePhotonsTotalCanvas->Update();
  AperturePhotonsTotalCanvas->SaveAs(SafeString(m_OutputPrefix + "_AperaturePhotonSpectrum.C"));
  
  TCanvas* CollimatorFirstTotalCanvas = new TCanvas();
  CollimatorFirstTotalCanvas->cd();
  CollimatorFirstTotalCanvas->SetLogx();
  CollimatorFirstTotalCanvas->SetLogy();
  CollimatorFirstTotal->Draw();
  CollimatorFirstTotalCanvas->Update();
  CollimatorFirstTotalCanvas->SaveAs(SafeString(m_OutputPrefix + "_CollimatorFirstTotalCanvas.C"));


  // Aperature photons zenith angle:
  for (int b = 1; b <= AperturePhotonsZenithAngle->GetNbinsX(); ++b) {
    double Area = 
      2*c_Pi*(cos(AperturePhotonsZenithAngle->GetBinLowEdge(b)*c_Rad) - 
              cos(AperturePhotonsZenithAngle->GetBinLowEdge(b)*c_Rad + AperturePhotonsZenithAngle->GetBinWidth(b)*c_Rad));
    AperturePhotonsZenithAngle->SetBinContent(b, AperturePhotonsZenithAngle->GetBinContent(b)/Area);
  }
    
  TCanvas* AperturePhotonsZenithAngleCanvas = new TCanvas("Aperature photon zenith angle distribution");
  AperturePhotonsZenithAngleCanvas->cd();
  AperturePhotonsZenithAngle->Draw();
  AperturePhotonsZenithAngleCanvas->Update();
  AperturePhotonsZenithAngleCanvas->SaveAs(SafeString(m_OutputPrefix + "_AperaturePhotonZenithAnlge.C"));



  // Stack of detector background:

  int Color = 2;
  double Position = 0.2;
  double PositionIncrement = 0.1;

  THStack* BackgroundDetectorActiveShieldListStack = new THStack("BackgroundDetectorActiveShieldListStack","");
  TCanvas* BackgroundDetectorActiveShieldListStackCanvas = new TCanvas("BackgroundDetectorActiveShieldListStackCanvas", "BackgroundDetectorActiveShieldListStackCanvas", 1000, 500);
  BackgroundDetectorActiveShieldListStackCanvas->SetFillColor(0);
  BackgroundDetectorActiveShieldListStackCanvas->SetFrameBorderSize(0);
  BackgroundDetectorActiveShieldListStackCanvas->SetFrameBorderMode(0);
  BackgroundDetectorActiveShieldListStackCanvas->SetBorderSize(0);
  BackgroundDetectorActiveShieldListStackCanvas->SetBorderMode(0);


  TPad* BackgroundDetectorActiveShieldListStackHistPad =
    new TPad("pad1", "pad1", 0.00, 0.00, 0.6, 1.0);
  BackgroundDetectorActiveShieldListStackHistPad->SetFillColor(0);
  BackgroundDetectorActiveShieldListStackHistPad->SetFrameBorderSize(0);
  BackgroundDetectorActiveShieldListStackHistPad->SetFrameBorderMode(0);
  BackgroundDetectorActiveShieldListStackHistPad->SetBorderSize(0);
  BackgroundDetectorActiveShieldListStackHistPad->SetBorderMode(0);

  TPad* BackgroundDetectorActiveShieldListStackTextPad =
    new TPad("pad1", "pad1", 0.6, 0.0, 1.0, 1.0);
  BackgroundDetectorActiveShieldListStackTextPad->SetFillColor(0);
  BackgroundDetectorActiveShieldListStackTextPad->SetFrameBorderSize(0);
  BackgroundDetectorActiveShieldListStackTextPad->SetFrameBorderMode(0);
  BackgroundDetectorActiveShieldListStackTextPad->SetBorderSize(0);
  BackgroundDetectorActiveShieldListStackTextPad->SetBorderMode(0);

  BackgroundDetectorActiveShieldListStackHistPad->Draw();
  BackgroundDetectorActiveShieldListStackTextPad->Draw();

  BackgroundDetectorActiveShieldListStackTextPad->cd();
  sort(BackgroundDetectorActiveShieldList.begin(), BackgroundDetectorActiveShieldList.end(), MHistCompare);
  int DrawnBackgroundShieldList = 0;
  for (unsigned int i = 0; i < BackgroundShieldList.size(); ++i) {
    if (BackgroundDetectorActiveShieldList[i]->Integral() <= 0) continue;
    
    TString Name(BackgroundDetectorActiveShieldList[i]->GetName());
    TLatex* text = 0;
    text = new TLatex(0.05,Position, GetText(Name));

    if (text != 0) {
      text->SetTextSize(0.05);
      text->SetTextColor(Color);
      text->SetLineWidth(2);
      text->Draw();            
      Position += PositionIncrement;
    }


    BackgroundDetectorActiveShieldList[i]->SetFillColor(Color++);
    BackgroundDetectorActiveShieldListStack->Add(BackgroundDetectorActiveShieldList[i]);
    DrawnBackgroundShieldList++;
  }
  
  if (DrawnBackgroundShieldList != 0) {
    BackgroundDetectorActiveShieldListStackHistPad->cd();
    BackgroundDetectorActiveShieldListStackHistPad->SetLogx();
    BackgroundDetectorActiveShieldListStackHistPad->SetLogy();
    BackgroundDetectorActiveShieldListStack->Draw();
    // BackgroundDetectorActiveShieldListStack->SetMinimum(0.001);
    BackgroundDetectorActiveShieldListStack->SetTitle("Total Background Rate of the CZT Detectors with Active Shield");
    BackgroundDetectorActiveShieldListStack->GetXaxis()->SetMoreLogLabels();
    BackgroundDetectorActiveShieldListStack->GetXaxis()->SetTitle("Deposited Energy [keV]");
    BackgroundDetectorActiveShieldListStack->GetXaxis()->SetTitleOffset(1.2);
    BackgroundDetectorActiveShieldListStack->GetYaxis()->SetTitle("cts/sec/keV/cm^{2}");
    BackgroundDetectorActiveShieldListStack->GetYaxis()->SetTitleOffset(1.2);
    BackgroundDetectorActiveShieldListStack->Draw();
    
    BackgroundDetectorActiveShieldListStackCanvas->Modified();
    BackgroundDetectorActiveShieldListStackCanvas->Update();
    //BackgroundDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_TotalBackgroundRate.gif");
    //BackgroundDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_TotalBackgroundRate.eps");
    BackgroundDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_TotalBackgroundRate.root");
  }


  // Stack of zenith angles:

  Color = 2;
  Position = 0.2;
  PositionIncrement = 0.1;

  THStack* ZenitAngleDetectorActiveShieldListStack = new THStack("ZenitAngleDetectorActiveShieldListStack","");
  TCanvas* ZenitAngleDetectorActiveShieldListStackCanvas = new TCanvas("ZenitAngleDetectorActiveShieldListStackCanvas", "ZenitAngleDetectorActiveShieldListStackCanvas", 1000, 500);
  ZenitAngleDetectorActiveShieldListStackCanvas->SetFillColor(0);
  ZenitAngleDetectorActiveShieldListStackCanvas->SetFrameBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackCanvas->SetFrameBorderMode(0);
  ZenitAngleDetectorActiveShieldListStackCanvas->SetBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackCanvas->SetBorderMode(0);


  TPad* ZenitAngleDetectorActiveShieldListStackHistPad =
    new TPad("pad1", "pad1", 0.00, 0.00, 0.6, 1.0);
  ZenitAngleDetectorActiveShieldListStackHistPad->SetFillColor(0);
  ZenitAngleDetectorActiveShieldListStackHistPad->SetFrameBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackHistPad->SetFrameBorderMode(0);
  ZenitAngleDetectorActiveShieldListStackHistPad->SetBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackHistPad->SetBorderMode(0);

  TPad* ZenitAngleDetectorActiveShieldListStackTextPad =
    new TPad("pad1", "pad1", 0.6, 0.0, 1.0, 1.0);
  ZenitAngleDetectorActiveShieldListStackTextPad->SetFillColor(0);
  ZenitAngleDetectorActiveShieldListStackTextPad->SetFrameBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackTextPad->SetFrameBorderMode(0);
  ZenitAngleDetectorActiveShieldListStackTextPad->SetBorderSize(0);
  ZenitAngleDetectorActiveShieldListStackTextPad->SetBorderMode(0);

  ZenitAngleDetectorActiveShieldListStackHistPad->Draw();
  ZenitAngleDetectorActiveShieldListStackTextPad->Draw();

  ZenitAngleDetectorActiveShieldListStackTextPad->cd();
  sort(ZenitAngleDetectorActiveShieldList.begin(), ZenitAngleDetectorActiveShieldList.end(), MHistCompare);

  int DrawnZenitAngleDetectorActiveShieldList = 0;
  for (unsigned int i = 0; i < ZenitAngleDetectorActiveShieldList.size(); ++i) {
    if (ZenitAngleDetectorActiveShieldList[i]->Integral() <= 0) continue;

    TString Name(ZenitAngleDetectorActiveShieldList[i]->GetName());
    TLatex* text = 0;
    text = new TLatex(0.05,Position, GetText(Name));

    if (text != 0) {
      text->SetTextSize(0.05);
      text->SetTextColor(Color);
      text->SetLineWidth(2);
      text->Draw();            
      Position += PositionIncrement;
    }


    ZenitAngleDetectorActiveShieldList[i]->SetFillColor(Color++);
    ZenitAngleDetectorActiveShieldListStack->Add(ZenitAngleDetectorActiveShieldList[i]);
    DrawnZenitAngleDetectorActiveShieldList++;
  }
  
  if (DrawnZenitAngleDetectorActiveShieldList > 0) {
    ZenitAngleDetectorActiveShieldListStackHistPad->cd();
    ZenitAngleDetectorActiveShieldListStack->Draw();
    // ZenitAngleDetectorActiveShieldListStack->SetMinimum(0.001);
    ZenitAngleDetectorActiveShieldListStack->SetTitle("Zenith angle distribution");
    ZenitAngleDetectorActiveShieldListStack->GetXaxis()->SetMoreLogLabels();
    ZenitAngleDetectorActiveShieldListStack->GetXaxis()->SetTitle("Angle [deg]");
    ZenitAngleDetectorActiveShieldListStack->GetXaxis()->SetTitleOffset(1.2);
    ZenitAngleDetectorActiveShieldListStack->GetYaxis()->SetTitle("cts/sec");
    ZenitAngleDetectorActiveShieldListStack->GetYaxis()->SetTitleOffset(1.2);
    ZenitAngleDetectorActiveShieldListStack->Draw();
    
    ZenitAngleDetectorActiveShieldListStackCanvas->Modified();
    ZenitAngleDetectorActiveShieldListStackCanvas->Update();
    //ZenitAngleDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetector.gif");
    //ZenitAngleDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetector.eps");
    ZenitAngleDetectorActiveShieldListStackCanvas->SaveAs(m_OutputPrefix + "_ZenitAngleDetector.root");
  }

  // Dump final statistics:

  ShieldCountsTotalRateError = sqrt(ShieldCountsTotalRateError);
  ShieldCountsAboveThresholdTotalRateError = sqrt(ShieldCountsAboveThresholdTotalRateError);
  DetectorCountsActiveShieldTotalRateError = sqrt(DetectorCountsActiveShieldTotalRateError);
  DetectorCountsPassiveShieldTotalRateError = sqrt(DetectorCountsPassiveShieldTotalRateError);

  mlog<<endl;
  mlog<<"Final summary"<<endl;
  mlog<<endl;
  mlog<<"Count rate shield:                                "
      <<ShieldCountsTotalRate<<" +- "<<ShieldCountsTotalRateError<<" cts/sec"<<endl;
  mlog<<"Count rate shield above threshold:                "
      <<ShieldCountsAboveThresholdTotalRate<<" +- "<<ShieldCountsAboveThresholdTotalRateError<<"  cts/sec/keV/cm2"<<endl;
  mlog<<"Detector rate passive shield ("<<m_DetThresholdMin<<"-"<<m_DetThresholdMax<<" keV): "
      <<DetectorCountsPassiveShieldTotalRate<<" +- "<<DetectorCountsPassiveShieldTotalRateError<<"  cts/sec/keV/cm2"<<endl;
  mlog<<"Detector rate active shield ("<<m_DetThresholdMin<<"-"<<m_DetThresholdMax<<" keV):  "
      <<DetectorCountsActiveShieldTotalRate<<" +- "<<DetectorCountsActiveShieldTotalRateError<<"  cts/sec/keV/cm2"<<endl;

  
  fout.close();

  if (m_StoreLines == true) {
    string Text = List.Dump();

    ofstream flines;
    flines.open((m_OutputPrefix + ".lines").Data());
    flines<<Text<<endl;
    flines.close();
  }

  return true;
}

TString NuStarShield::SafeString(TString Name)
{
  Name.ReplaceAll(" ", "_");
  Name.ReplaceAll("\\", "_");
  Name.ReplaceAll("/", "_");
  Name.ReplaceAll("-", "_");
  Name.ReplaceAll(".", "_");
  Name.ReplaceAll("+", "_");
  Name.ReplaceAll("-", "_");
  Name.ReplaceAll("(", "_");
  Name.ReplaceAll(")", "_");

  return Name;
}

TString NuStarShield::GetText(TString Name)
{
  if (Name.Contains("CosmicProtons.prompt")) {
   return "Cosmic protons prompt component";
  } else if (Name.Contains("CosmicProtons.activation")) {
   return "Cosmic protons delayed component";
  } else if (Name.Contains("AlbedoNeutrons.activation")) {
   return "Albedo neutrons delayed component";
  } else if (Name.Contains("AlbedoNeutrons.prompt")) {
   return "Albedo neutrons prompt component";
  } else if (Name.Contains("CosmicPhotons")) {
    return "Cosmic photons";
  } else if (Name.Contains("AlbedoPhotons")) {
    return "Albedo photons";
  } else {
    return Name;
  }
}


/******************************************************************************/

NuStarShield* g_Prg = 0;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
  
  if (g_Prg != 0) {
    g_Prg->Interrupt();
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  //void (*handler)(int);
	//handler = CatchSignal;
  //(void) signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication NuStarShieldApp("NuStarShieldApp", 0, 0);

  g_Prg = new NuStarShield();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  NuStarShieldApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
