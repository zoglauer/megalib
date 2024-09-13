/*
 * UTARMFitter.cxx
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
#include "MFitFunctions.h"
#include "MARMFitter.h"
#include "MInterface.h"

// Geant4:

// ROOT:
#include "TH1.h"
#include "TF1.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TRandom.h"

// Standard lib:
#include <sstream>
#include <fstream>
#include <future>
#include <cmath>
#include <chrono>
#include <thread>
using namespace std;


//! Unit test class for the ARM fitter
class UTARMFitter
{
  // public interface:
public:
  //! Default constructor
  UTARMFitter();
  //! Default destructor
  virtual ~UTARMFitter() {};

  //! Run all tests
  bool Run();

  //! Run a single tests
  bool RunSingle();

  // protected methods:
protected:
  //! Test single fitting
  double TestSingleFit();
  //! Test bootstrapping
  double TestBootstrapping();



  // protected members:
protected:


  // private members:
private:
  //! Single mode
  bool m_SingleMode;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
UTARMFitter::UTARMFitter()
{
  //m_MassModelFileName = "$(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup";
  //m_NumberOfTriggers = 100000;
  m_SingleMode = false;

  //gROOT->SetBatch(true);
  gRandom->SetSeed(0);
}


////////////////////////////////////////////////////////////////////////////////


//! Run a single tests
bool UTARMFitter::RunSingle()
{
  m_SingleMode = true;
  
  cout<<"Test single fit: "<<TestSingleFit()<<endl;
  //cout<<"Test single fit: "<<TestSingleFit()<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTARMFitter::Run()
{
  //cout<<"Test single fit: "<<TestSingleFit()<<endl;
  cout<<"Test bootstrapping: "<<TestBootstrapping()<<endl;



  /*
  future<double> TestPowerLawResult = async(launch::async, &UTARMFitter::TestPowerLaw, this);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  future<double> TestCutOffPowerLawResult = async(launch::async, &UTARMFitter::TestCutOffPowerLaw, this);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  future<double> TestComptonization = async(launch::async, &UTARMFitter::TestComptonization, this);
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  bool AllPassed = true;

  double TestPowerLawSigma = TestPowerLawResult.get();
  double TestCutOffPowerLawSigma = TestCutOffPowerLawResult.get();
  double TestComptonizationSigma = TestComptonization.get();

  if (TestPowerLawSigma < 0) {
    cout<<"Failed: Power law test did not finish"<<endl;
    AllPassed = false;
  } else if (TestPowerLawSigma >= 3 || std::isnan(TestPowerLawSigma) == true) {
    cout<<"Failed: Power law significance: "<<TestPowerLawSigma<<" sigma"<<endl;
    AllPassed = false;
  } else {
    cout<<"Passed: Power law significance: "<<TestPowerLawSigma<<" sigma"<<endl;
  }

  if (TestCutOffPowerLawSigma < 0) {
    cout<<"Failed: Cutoff power law test did not finish"<<endl;
    AllPassed = false;
  } else if (TestCutOffPowerLawSigma >= 3 || std::isnan(TestCutOffPowerLawSigma) == true) {
    cout<<"Failed: Cutoff power law significance: "<<TestCutOffPowerLawSigma<<" sigma"<<endl;
    AllPassed = false;
  } else {
    cout<<"Passed: Cutoff power law significance: "<<TestCutOffPowerLawSigma<<" sigma"<<endl;
  }

  if (TestComptonizationSigma < 0) {
    cout<<"Failed: Comptonization test did not finish"<<endl;
    AllPassed = false;
  } else if (TestComptonizationSigma >= 3 || std::isnan(TestComptonizationSigma) == true) {
    cout<<"Failed: Comptonization significance: "<<TestComptonizationSigma<<" sigma"<<endl;
    AllPassed = false;
  } else {
    cout<<"Passed: Comptonization significance: "<<TestComptonizationSigma<<" sigma"<<endl;
  }

  if (AllPassed == true) {
    cout<<"Passed: All passed"<<endl;
  }
  */
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a single fit
double UTARMFitter::TestSingleFit()
{
  cout<<"Started test single fit"<<endl;

  // Basics
  constexpr unsigned int Counts = 200;
  constexpr unsigned int NumberOfBins = 201;
  constexpr double MaxARM = 20;

  vector<double> ARMValues;
  ARMValues.reserve(Counts);

  // Create the ARM data
  TF1* Sampler = new TF1("DoubleLorentzAsymGausArm", DoubleLorentzAsymGausArm, -MaxARM, MaxARM, 9);
  Sampler->SetBit(kCanDelete);
  Sampler->SetParNames("Offset",
                       "Mean",
                       "Lorentz Width 1",
                       "Lorentz Height 1",
                       "Lorentz Width 2",
                       "Lorentz Height 2",
                       "Gaus Height",
                       "Gaus Sigma 1",
                       "Gaus Sigma 2");
  //Sampler->SetParameters(100, -1.0, 1.0, 200, 3.0, 300, 500, 1.5, 4.5);
  Sampler->SetParameters(0, 0.0, 1.0, 0, 3.0, 0, 500, 2.5, 2.5);

  for (unsigned int i = 0; i < Counts; ++i) {
    ARMValues.push_back(Sampler->GetRandom(-MaxARM, MaxARM));
  }

  delete Sampler;

  // Set up the fitter and fit
  MARMFitter Fitter;
  Fitter.SetNumberOfBins(NumberOfBins);
  Fitter.SetMaximumARMValue(MaxARM);

  for (unsigned int i = 0; i < Counts; ++i) {
    Fitter.AddARMValue(ARMValues[i]);
  }

  Fitter.Fit(1);

  if (Fitter.WasFittingSuccessful() == true) {
    cout<<"FWHM: "<<Fitter.GetAverageFWHM()<<endl;
  }

  TCanvas* FitterCanvas = new TCanvas();
  FitterCanvas->SetTitle("ARM of Compton cone");
  FitterCanvas->cd();
  Fitter.Draw();
  FitterCanvas->Modified();
  FitterCanvas->Update();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a single fit
double UTARMFitter::TestBootstrapping()
{
  cout<<"Started test bootstrapping"<<endl;

  // Basics
  constexpr unsigned int Counts = 100000;
  constexpr unsigned int NumberOfBins = 101;
  constexpr double MaxARM = 10;

  vector<double> ARMValues;
  ARMValues.reserve(Counts);

  // Create the ARM data
  TF1* Sampler = new TF1("DoubleLorentzAsymGausArm", DoubleLorentzAsymGausArm, -180, 180, 9);
  Sampler->SetBit(kCanDelete);
  Sampler->SetParNames("Offset",
                       "Mean",
                       "Lorentz Width 1",
                       "Lorentz Height 1",
                       "Lorentz Width 2",
                       "Lorentz Height 2",
                       "Gaus Height",
                       "Gaus Sigma 1",
                       "Gaus Sigma 2");
  Sampler->SetParameters(0, 0.0, 2.0, 0, 3.0, 0, 500, 2.5, 2.5);

  for (unsigned int i = 0; i < Counts; ++i) {
    ARMValues.push_back(Sampler->GetRandom(-180, 180));
  }

  double OriginalFWHM = MInterface::GetFWHM(Sampler, -MaxARM, MaxARM);
  cout<<"Original FWHM: "<<OriginalFWHM<<endl;

  delete Sampler;

  // Set up the fitter and fit
  MARMFitter Fitter;
  Fitter.SetNumberOfBins(NumberOfBins);
  Fitter.SetMaximumARMValue(MaxARM);
  //Fitter.SetFitFunction(MARMFitFunctionID::c_AsymmetricGaussLorentzLorentz);
  Fitter.SetFitFunction(MARMFitFunctionID::c_Gauss);
  Fitter.UseOptimizedBinning(true);
  Fitter.UseBinnedFitting(true);

  for (unsigned int i = 0; i < Counts; ++i) {
    Fitter.AddARMValue(ARMValues[i]);
  }

  Fitter.Fit(1);

  if (Fitter.WasFittingSuccessful() == true) {
    cout<<"Average FWHM: "<<Fitter.GetAverageFWHM()<<endl;
    cout<<"FWHM Uncertainty: "<<Fitter.GetAverageFWHMUncertainty()<<endl;
    cout<<"Sigma difference: "<<fabs(OriginalFWHM - Fitter.GetAverageFWHM())/Fitter.GetAverageFWHMUncertainty()<<endl;

    vector<double> FWHMes = Fitter.GetBootstrappedFWHMSamples();
    if (FWHMes.size() > 0) {

      // Use std::min_element to find the minimum value
      auto MinValueIter = std::min_element(FWHMes.begin(), FWHMes.end());
      double MinValue = *MinValueIter;
      auto MaxValueIter = std::max_element(FWHMes.begin(), FWHMes.end());
      double MaxValue = *MaxValueIter;

      TH1D* H = new TH1D("H", "H", 100, MinValue, MaxValue + 0.5);
      for (auto F: FWHMes) {
        H->Fill(F);
      }

      TCanvas* C = new TCanvas();
      C->cd();
      H->Draw();
      C->Update();
    }

    TCanvas* FitterCanvas = new TCanvas();
    FitterCanvas->SetTitle("ARM of Compton cone");
    FitterCanvas->cd();
    Fitter.Draw();
    FitterCanvas->Modified();
    FitterCanvas->Update();

    cout<<Fitter.ToString()<<endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Initialize ROOT:
  TApplication ROOT("ROOT", 0, 0);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize("ARMFitter", "unit test the ARM fitting class");

  UTARMFitter Test;
  //Test.Run();
  
  Test.Run();
  ROOT.Run();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
