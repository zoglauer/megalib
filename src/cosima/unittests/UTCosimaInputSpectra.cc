/*
 * UTCosimaInputSpectra.cxx
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


// Cosima:

// MEGAlib:
#include "MGlobal.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MSimEvent.h"
#include "MSimIA.h"

// Geant4:

// ROOT:
#include "TH1.h"
#include "TF1.h"
#include "TApplication.h"
#include "TCanvas.h"

// Standard lib:
#include <sstream>
#include <fstream>
#include <future>
#include <cmath>
using namespace std;

/******************************************************************************/

class UTCosimaInputSpectra
{
  // public interface:
public:
  /// Default constructor
  UTCosimaInputSpectra();
  /// Default destructor
  virtual ~UTCosimaInputSpectra() {};

  /// Run all tests
  bool Run();

  /// Run a single tests
  bool RunSingle();

  // protected methods:
protected:
  bool CreateSourceFile(MString FileNamePrefix, MString Spectrum);
  TH1D* CreateSpectrum(MString FileNamePrefix, unsigned int NumberOfBins, double EMin, double EMax);

  double TestPowerLaw();
  double TestBrokenPowerLaw();
  double TestCutOffPowerLaw();


  // protected members:
protected:


  // private members:
private:
  /// The mass model file name
  MString m_MassModelFileName;
  /// The number of triggers
  unsigned int m_NumberOfTriggers;
  /// Single mode
  bool m_SingleMode;
};


/******************************************************************************
 * Default constructor
 */
UTCosimaInputSpectra::UTCosimaInputSpectra()
{
  m_MassModelFileName = "$(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup";
  m_NumberOfTriggers = 200000;
  m_SingleMode = false;
}


/******************************************************************************
 * Run all tests
 */
bool UTCosimaInputSpectra::RunSingle()
{
  m_SingleMode = true;
  
  cout<<"CutOffPowerLaw: "<<TestCutOffPowerLaw()<<endl;
  
  return true;
}


/******************************************************************************
 * Run all tests
 */
bool UTCosimaInputSpectra::Run()
{
  future<double> TestPowerLawResult = async(launch::async, &UTCosimaInputSpectra::TestPowerLaw, this);
  future<double> TestCutOffPowerLawResult = async(launch::async, &UTCosimaInputSpectra::TestCutOffPowerLaw, this);
  
  bool AllPassed = true;

  double TestPowerLawSigma = TestPowerLawResult.get();
  double TestCutOffPowerLawSigma = TestCutOffPowerLawResult.get();

  if (TestPowerLawSigma < 0) {
    cout<<"Failed: Power law test did not finish"<<endl;
    AllPassed = false;
  } else if (TestPowerLawSigma >= 3) {
    cout<<"Failed: Power law significance: "<<TestPowerLawSigma<<" sigma"<<endl;
    AllPassed = false;
  } else {
    cout<<"Passed: Power law significance: "<<TestPowerLawSigma<<" sigma"<<endl;
  }

  if (TestCutOffPowerLawSigma < 0) {
    cout<<"Failed: Cutoff power law test did not finish"<<endl;
    AllPassed = false;
  } else if (TestCutOffPowerLawSigma >= 3) {
    cout<<"Failed: Cutoff power  law significance: "<<TestCutOffPowerLawSigma<<" sigma"<<endl;
    AllPassed = false;
  } else {
    cout<<"Passed: Cutoff power  law significance: "<<TestCutOffPowerLawSigma<<" sigma"<<endl;
  }
  
  if (AllPassed == true) {
    cout<<"Passed: All passed"<<endl;
  }
  
  return true;
}


/******************************************************************************
 * Create a source file
 */
bool UTCosimaInputSpectra::CreateSourceFile(MString FileNamePrefix, MString Spectrum)
{
  MString FileName = FileNamePrefix + ".source";
  ofstream fout;
  fout.open(FileName);
  if (fout.is_open() == false) {
    cout<<"Unable to open "<<FileNamePrefix<<endl;
    return false;
  }
  fout<<"# Unit test source file"<<endl;
  fout<<endl;
  fout<<"Version 1"<<endl;
  fout<<endl;
  fout<<"# Geometry"<<endl;
  fout<<"Geometry        "<<m_MassModelFileName<<endl;
  fout<<""<<endl;
  fout<<"# Physics list"<<endl;
  fout<<"PhysicsListEM               LivermorePol"<<endl;
  fout<<endl;
  fout<<"# Output formats"<<endl;
  fout<<"StoreSimulationInfo         init"<<endl;
  fout<<endl;
  fout<<"# Run & source parameters"<<endl;
  fout<<"Run UnitTest"<<endl;
  fout<<"UnitTest.FileName               "<<FileNamePrefix<<endl;
  fout<<"UnitTest.Triggers               "<<m_NumberOfTriggers<<endl;
  fout<<endl;
  fout<<"UnitTest.Source UnitTestSource"<<endl;
  fout<<"UnitTestSource.ParticleType           1"<<endl;
  fout<<"UnitTestSource.Beam                   PointSource 0 0 0"<<endl;
  fout<<"UnitTestSource.Spectrum               "<<Spectrum<<endl;
  fout<<"UnitTestSource.Flux                   1"<<endl;
  fout<<endl;
  fout.close();
  
  return true;
}


/******************************************************************************
 * Create a spectrum
 */
TH1D* UTCosimaInputSpectra::CreateSpectrum(MString FileNamePrefix, unsigned int Bins, double EMin, double EMax)
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_MassModelFileName) == true) {
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<m_MassModelFileName<<" - Aborting!"<<endl;
    return nullptr;
  }  

  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  if (Reader->Open(FileNamePrefix + ".inc1.id1.sim.gz") == false) {
    cout<<"Unable to open sim file "<<(FileNamePrefix + ".id1.ic1.sim.gz")<<" - Aborting!"<<endl; 
    return nullptr;
  }

  // Create some histograms here ...
  TH1D* Spectrum = new TH1D(FileNamePrefix + "Spectrum", FileNamePrefix + "Spectrum", Bins, EMin, EMax);
  Spectrum->SetXTitle("Energy [keV]");
  Spectrum->SetYTitle("cts");
  
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    if (Event->GetNIAs() >= 1) {
      double Energy = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (std::isfinite(Energy)) {
        Spectrum->Fill(Energy);
      } else {
        cout<<"Found Inf or NaN"<<endl;
      }
    }
    delete Event;
  }

  //delete Reader;
  //delete Geometry;
  
  return Spectrum;
}


/******************************************************************************
 * Power law test
 */
double UTCosimaInputSpectra::TestPowerLaw()
{
  cout<<"Started test power law"<<endl;
  
  int EMin = 10;
  int EMax = 10000;
  double PhotonIndex = 2;
  
  MString FileNamePrefix = "TestPowerLaw";
  if (CreateSourceFile(FileNamePrefix, MString("PowerLaw") + " " + EMin + " " + EMax + " " + PhotonIndex) == false) return false;
  
  if (system(MString("rm -f ") + FileNamePrefix + ".inc1.id1.sim.gz") == -1) {
    return -1;
  }
  if (system(MString("cosima -v 0 -z ") + FileNamePrefix + ".source &> /dev/null") == -1) {
    return -1;
  }

  TH1D* Spectrum = CreateSpectrum(FileNamePrefix, 100, EMin, EMax);
  if (Spectrum == nullptr) return -1;
  
  TF1* Fit = new TF1(FileNamePrefix + "Fit","[0]*pow(x,-[1])", EMin, EMax);
  Fit->SetParameters(100, 2.5);
  
  Spectrum->Fit(Fit, "IMN");
  
  double FittedPhotonIndex = Fit->GetParameter(1);
  double FittedPhotonIndexError = Fit->GetParError(1);

  delete Fit;
  delete Spectrum;

  return fabs(PhotonIndex - FittedPhotonIndex) / FittedPhotonIndexError;
}


/******************************************************************************
 * Run all tests
 */
double UTCosimaInputSpectra::TestBrokenPowerLaw()
{
  /*
  int EMin = 10;
  int EMax = 10000;
  double EBreak 100;
  double PhotonIndex1 = 0.5
  double PhotonIndex2 = 2;
  
  MString FileNamePrefix = "TestBrokenPowerLaw";
  if (CreateSourceFile(FileNamePrefix, MString("BrokenPowerLaw") + " " + EMin + " " + EMax + " " + EBreak + " " + PhotonIndex1 + " " + PhotonIndex2) == false) return false;
  
  system(MString("rm -f ") + FileNamePrefix + ".inc1.id1.sim.gz");
  system(MString("mwait -p=cosima; cosima -v 0 -z ") + FileNamePrefix + ".source"); // &> /dev/null");

  TH1D* Spectrum = CreateSpectrum(FileNamePrefix, 100, EMin, EMax);
  if (Spectrum == nullptr) return -1;
  
  TF1* Fit = new TF1(FileNamePrefix + "Fit","[0]*pow(x,-[1])*exp(-x/[2])", EMin, EMax);
  Fit->SetParameters(10000, 2.5, 200);
  
  Spectrum->Fit(Fit, "IMN");
  
  double FittedPhotonIndex = Fit->GetParameter(1);
  double FittedPhotonIndexError = Fit->GetParError(1);
  
  double FittedCutOff = Fit->GetParameter(2);
  double FittedCutOffError = Fit->GetParError(2);

  delete Fit;
  delete Spectrum;

  return max(fabs(PhotonIndex - FittedPhotonIndex) / FittedPhotonIndexError, fabs(CutOff - FittedCutOff) / FittedCutOffError);
  */
  return -1;
}


/******************************************************************************
 * Run all tests
 */
double UTCosimaInputSpectra::TestCutOffPowerLaw()
{
  cout<<"Started cut off power law"<<endl;

  int EMin = 10;
  int EMax = 10000;
  double PhotonIndex = 0.5;
  double CutOff = 1000;
  
  MString FileNamePrefix = "TestCutOffPowerLaw";
  if (CreateSourceFile(FileNamePrefix, MString("CutOffPowerLaw") + " " + EMin + " " + EMax + " " + PhotonIndex + " " + CutOff) == false) return false;
  
  if (system(MString("rm -f ") + FileNamePrefix + ".inc1.id1.sim.gz") == -1) {
    return -1;
  }
  if (system(MString("cosima -v 0 -z ") + FileNamePrefix + ".source &> /dev/null") == -1) {
    return -1;
  }
    

  TH1D* Spectrum = CreateSpectrum(FileNamePrefix, 100, EMin, EMax);
  if (Spectrum == nullptr) return -1;

  TF1* Fit = new TF1(FileNamePrefix + "Fit","[0]*pow(x,-[1])*exp(-x/[2])", EMin, EMax);
  Fit->SetParameters(10000, 2.5, 200);

  Spectrum->Fit(Fit, "IMN");
  
  if (m_SingleMode == true) {
    TCanvas* C = new TCanvas();
    C->cd();
    Spectrum->DrawCopy();
    Fit->DrawCopy("SAME");
    C->Update();
  }

  double FittedPhotonIndex = Fit->GetParameter(1);
  double FittedPhotonIndexError = Fit->GetParError(1);
  
  double FittedCutOff = Fit->GetParameter(2);
  double FittedCutOffError = Fit->GetParError(2);

  delete Fit;
  delete Spectrum;
  
  return max(fabs(PhotonIndex - FittedPhotonIndex) / FittedPhotonIndexError, fabs(CutOff - FittedCutOff) / FittedCutOffError);
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Initialize ROOT:
  TApplication ROOT("ROOT", 0, 0);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("TestSpectra", "unit test the cosima input spectra");
  __merr.SetHeader("COSIMA-ERROR:");

  UTCosimaInputSpectra Test;
  Test.Run();
  
  //Test.RunSingle();
  //ROOT.Run();

  return 0;
}


/*
 * UTCosimaInputSpectra.cc: the end...
 ******************************************************************************/
