/*
 * UTCosimaInputSpectra.cc
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


// Cosima:
#include "MCParameterFile.hh"
#include "MCSource.hh"

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
#include "TFitResult.h"
#include "TPad.h"
#include "TApplication.h"
#include "TCanvas.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <sstream>
#include <fstream>
#include <future>
#include <cmath>
#include <chrono>
#include <thread>
#include <filesystem>
#include <cstdlib>
#include <string>
#include <sys/wait.h>
using namespace std;

//! Expose the parsed source for parser-level spectrum checks.
class UTCosimaParameterFile : public MCParameterFile
{
public:
  using MCParameterFile::GetSource;
};

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
  //! One spectrum which is simulated with cosima and fitted
  struct SpectrumCase {
    //! Name for the output
    MString m_Name;
    //! File name prefix of the source and sim file
    MString m_Prefix;
    //! The Spectrum keyword value
    MString m_Spectrum;
    //! The energy range in keV
    double m_EMin;
    double m_EMax;
    //! The fit function and the start values of its parameters (parameter 0 is the normalization and is set automatically).
    //! The x-axis of the fit is u = ln(E/keV): the function is dN/du = E*dN/dE, the bins are uniform in u (see CreateSpectrum())
    MString m_Formula;
    vector<double> m_Start;
    //! The parameters (index in the fit function, true value) which have to be reproduced
    vector<pair<unsigned int, double>> m_Truth;
  };

  //! Return the spectra which are tested
  vector<SpectrumCase> GetCases() const;
  bool CreateSourceFile(MString FileNamePrefix, MString Spectrum);
  //! Simulate the spectrum with cosima (external program: can run in parallel with others)
  bool RunCosima(const SpectrumCase& Case);
  TH1D* CreateSpectrum(MString FileNamePrefix, unsigned int NumberOfBins, double EMin, double EMax);
  //! Fit the simulated spectrum and return the significance of the deviation from the true spectrum, or a negative value if the analysis failed (ROOT: main thread only!)
  double Analyze(const SpectrumCase& Case);
  bool TestSpectrumInputs();

  //! The maximum significance in sigma: see Analyze()
  static constexpr double c_MaximumSigma = 5.0;


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
  m_NumberOfTriggers = 100000;
  m_SingleMode = false;

  gROOT->SetBatch(true);
}


/******************************************************************************
 * Run a single test
 */
bool UTCosimaInputSpectra::RunSingle()
{
  m_SingleMode = true;

  const SpectrumCase Case = GetCases()[1];
  if (RunCosima(Case) == false) return false;
  double Sigma = Analyze(Case);
  cout<<Case.m_Name<<": "<<Sigma<<endl;
  return Sigma >= 0.0 && std::isfinite(Sigma) && Sigma < c_MaximumSigma;
}


/******************************************************************************
 * The spectra which are tested
 */
vector<UTCosimaInputSpectra::SpectrumCase> UTCosimaInputSpectra::GetCases() const
{
  vector<SpectrumCase> Cases;
  // Power law: dN/dE = N*E^-index, i.e. dN/du = N*E^(1-index)
  Cases.push_back({ "Power law", "TestPowerLaw", "PowerLaw 10 10000 2", 10, 10000, "[0]*exp((1-[1])*x)", { 1.0, 2.5 }, { { 1, 2.0 } } });
  // Cut-off power law: dN/dE = N*E^-index*exp(-E/cutoff)
  Cases.push_back({ "Cutoff power law", "TestCutOffPowerLaw", "CutOffPowerLaw 10 10000 0.5 1000", 10, 10000, "[0]*exp((1-[1])*x)*exp(-exp(x)/[2])", { 1.0, 1.0, 500.0 }, { { 1, 0.5 }, { 2, 1000.0 } } });
  // Comptonized: dN/dE = N*E^alpha*exp(-(alpha + 2)*E/Epeak)
  Cases.push_back({ "Comptonization", "TestComptonization", "Comptonized 10 10000 -1.5 200", 10, 10000, "[0]*exp(([1]+1)*x)*exp(-([1]+2)*exp(x)/[2])", { 1.0, -1.4, 220.0 }, { { 1, -1.5 }, { 2, 200.0 } } });
  return Cases;
}


/******************************************************************************
 * Run all tests
 */
bool UTCosimaInputSpectra::Run()
{
  bool AllPassed = TestSpectrumInputs();

  // The simulations are external programs, they run in parallel. All ROOT work (histograms, fits) is done afterwards in the main thread, since ROOT is not thread safe
  const vector<SpectrumCase> Cases = GetCases();
  vector<future<bool>> Simulations;
  for (const SpectrumCase& C: Cases) {
    Simulations.push_back(async(launch::async, [this, C]() { return RunCosima(C); }));
    // Cosima seeds its random numbers from the time: keep the starts apart
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
  }

  for (unsigned int c = 0; c < Cases.size(); ++c) {
    const bool Simulated = Simulations[c].get();
    const double Sigma = (Simulated == true) ? Analyze(Cases[c]) : -1.0;
    if (Sigma < 0) {
      cout<<"Failed: "<<Cases[c].m_Name<<" test did not finish"<<endl;
      AllPassed = false;
    } else if (Sigma >= c_MaximumSigma || std::isnan(Sigma) == true) {
      cout<<"Failed: "<<Cases[c].m_Name<<" significance: "<<Sigma<<" sigma"<<endl;
      AllPassed = false;
    } else {
      cout<<"Passed: "<<Cases[c].m_Name<<" significance: "<<Sigma<<" sigma"<<endl;
    }
  }

  if (AllPassed == true) {
    cout<<"Passed: All passed"<<endl;
  }

  return AllPassed;
}


/******************************************************************************
 * Check that every spectrum keyword accepted by MCParameterFile is parsed and
 * stored as the expected type and parameters. Distribution shape is tested
 * separately by the simulation/fit tests below.
 */
bool UTCosimaInputSpectra::TestSpectrumInputs()
{
  const char* Root = getenv("MEGALIB");
  if (Root == nullptr) {
    cerr<<"Failed: MEGALIB is not set; cannot test spectrum parser inputs"<<endl;
    return false;
  }

  const MString Geometry = MString(Root) + "/src/cosima/unittests/GiganticSphere.geo.setup";
  const MString BeamFile = MString(Root) + "/resource/examples/cosima/source/3DSpherical.dat";
  const auto Stamp = chrono::high_resolution_clock::now().time_since_epoch().count();
  const filesystem::path Directory = filesystem::temp_directory_path() / ("UTCosimaInputSpectra_" + to_string(Stamp));
  error_code Error;
  if (filesystem::create_directories(Directory, Error) == false || Error) {
    cerr<<"Failed: unable to create spectrum parser test directory: "<<Directory<<endl;
    return false;
  }

  const filesystem::path SpectrumFile = Directory / "Spectrum.dat";
  {
    ofstream Out(SpectrumFile);
    Out<<"IP LIN\nDP 10 1\nDP 100 0.1\n";
    if (Out.good() == false) {
      cerr<<"Failed: unable to write differential-flux fixture"<<endl;
      filesystem::remove_all(Directory, Error);
      return false;
    }
  }

  struct SpectrumCase {
    const char* Name;
    MString Input;
    int ExpectedType;
    vector<double> ExpectedParameters;
    bool NormalizedBeam;
  };
  const vector<SpectrumCase> Cases = {
    { "Mono", "Mono 511", MCSource::c_Monoenergetic, { 511*keV }, false },
    { "Mono alias", "M 511", MCSource::c_Monoenergetic, { 511*keV }, false },
    { "Linear", "Linear 10 100", MCSource::c_Linear, { 10*keV, 100*keV }, false },
    { "Linear alias", "L 10 100", MCSource::c_Linear, { 10*keV, 100*keV }, false },
    { "PowerLaw", "PowerLaw 10 1000 2", MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2 }, false },
    { "PowerLaw alias", "PL 10 1000 2", MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2 }, false },
    { "BrokenPowerLaw", "BrokenPowerLaw 10 1000 100 1.5 2.5", MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 }, false },
    { "BrokenPowerLaw alias", "BPL 10 1000 100 1.5 2.5", MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 }, false },
    { "CutOffPowerLaw", "CutOffPowerLaw 10 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 500*keV }, false },
    { "CutOffPowerLaw alias", "CPL 10 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 500*keV }, false },
    { "Comptonized", "Comptonized 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV }, false },
    { "Comptonization alias", "Comptonization 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV }, false },
    { "Comptonized alias", "Comp 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV }, false },
    { "Gaussian", "Gaussian 100 10 3", MCSource::c_Gaussian, { 100*keV, 10*keV, 3 }, false },
    { "Gaussian alias", "G 100 10 3", MCSource::c_Gaussian, { 100*keV, 10*keV, 3 }, false },
    { "ThermalBremsstrahlung", "ThermalBremsstrahlung 10 1000 50", MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 50*keV }, false },
    { "ThermalBremsstrahlung alias", "TB 10 1000 50", MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 50*keV }, false },
    { "BlackBody", "BlackBody 10 1000 50", MCSource::c_BlackBody, { 10*keV, 1000*keV, 50*keV }, false },
    { "BlackBody alias", "BB 10 1000 50", MCSource::c_BlackBody, { 10*keV, 1000*keV, 50*keV }, false },
    { "BandFunction", "BandFunction 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV }, false },
    { "Band alias", "Band 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV }, false },
    { "BandFunction alias", "BF 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV }, false },
    { "FileDifferentialFlux", MString("File ") + SpectrumFile.string().c_str(), MCSource::c_FileDifferentialFlux, {}, false },
    { "File alias", MString("F ") + SpectrumFile.string().c_str(), MCSource::c_FileDifferentialFlux, {}, false },
    { "NormalizedEnergyBeamFluxFunction", "NormalizedEnergyBeamFluxFunction", MCSource::c_NormalizedEnergyBeamFluxFunction, {}, true }
  };

  bool Passed = true;
  unsigned int CaseNumber = 0;
  for (const SpectrumCase& C: Cases) {
    UTCosimaParameterFile Parameters;
    MString Beam = C.NormalizedBeam ? MString("FarFieldNormalizedEnergyBeamFluxFunction ") + BeamFile : "PointSource 0 0 0";
    MString Content = MString("Version 1\nGeometry ") + Geometry + "\nRun R\nR.FileName Test\nR.Time 1\nR.Source S\nS.ParticleType 1\nS.Beam " + Beam + "\nS.Spectrum " + C.Input + "\n";
    const filesystem::path SourceFile = Directory / ("Case" + to_string(CaseNumber++) + ".source");
    {
      ofstream Out(SourceFile);
      Out<<Content;
      if (Out.good() == false) {
        cerr<<"Failed: unable to write parser fixture for "<<C.Name<<endl;
        Passed = false;
        continue;
      }
    }

    if (Parameters.Open(SourceFile.string().c_str()) == false) {
      cerr<<"Failed: parser rejected spectrum input "<<C.Name<<": "<<C.Input<<endl;
      Passed = false;
      continue;
    }
    MCSource* Source = Parameters.GetSource("S");
    if (Source == nullptr || Source->GetSpectralType() != C.ExpectedType) {
      cerr<<"Failed: parser stored the wrong spectral type for "<<C.Name<<endl;
      Passed = false;
      continue;
    }
    for (unsigned int P = 0; P < C.ExpectedParameters.size(); ++P) {
      const double Actual = Source->GetEnergyParameter(P+1);
      const double Expected = C.ExpectedParameters[P];
      if (fabs(Actual - Expected) > max(1e-12*fabs(Expected), 1e-12*keV)) {
        cerr<<"Failed: parser stored the wrong parameter "<<P+1<<" for "<<C.Name<<endl;
        Passed = false;
      }
    }
  }

  // Deprecated numeric input remains accepted for legacy source files.
  {
    UTCosimaParameterFile Parameters;
    const filesystem::path SourceFile = Directory / "Legacy.source";
    ofstream Out(SourceFile);
    Out<<"Version 1\nGeometry "<<Geometry<<"\nRun R\nR.FileName Test\nR.Time 1\nR.Source S\nS.ParticleType 1\nS.Beam PointSource 0 0 0\nS.SpectralType "<<MCSource::c_PowerLaw<<"\n";
    Out.close();
    if (Parameters.Open(SourceFile.string().c_str()) == false || Parameters.GetSource("S") == nullptr || Parameters.GetSource("S")->GetSpectralType() != MCSource::c_PowerLaw) {
      cerr<<"Failed: legacy numeric SpectralType input was not accepted"<<endl;
      Passed = false;
    }
  }

  filesystem::remove_all(Directory, Error);
  if (Passed) cout<<"Passed: all spectrum input forms parse with the expected type and parameters"<<endl;
  return Passed;
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
 * Create a spectrum with logarithmically spaced bins: the spectra fall by orders of magnitude, linear bins leave the high-energy bins (almost) empty
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
    cout<<"Unable to open sim file "<<(FileNamePrefix + ".inc1.id1.sim.gz")<<" - Aborting!"<<endl; 
    return nullptr;
  }

  // Uniform bins in u = ln(E): ROOT compares the bin content with the (average) function value of the bin, which is only right for equal bin widths
  TH1D* Spectrum = new TH1D(FileNamePrefix + "Spectrum", FileNamePrefix + "Spectrum", Bins, log(EMin), log(EMax));
  Spectrum->SetXTitle("ln(Energy/keV)");
  Spectrum->SetYTitle("cts");
  
  MSimEvent* Event = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    if (Event->GetNIAs() >= 1) {
      double Energy = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (std::isfinite(Energy)) {
        Spectrum->Fill(log(Energy));
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
 * Simulate a spectrum with cosima
 */
bool UTCosimaInputSpectra::RunCosima(const SpectrumCase& Case)
{
  if (CreateSourceFile(Case.m_Prefix, Case.m_Spectrum) == false) return false;

  if (system(MString("rm -f ") + Case.m_Prefix + ".inc1.id1.sim.gz") == -1) {
    return false;
  }
  int Status = system(MString("cosima -v 0 -z ") + Case.m_Prefix + ".source > " + Case.m_Prefix + ".log 2>&1");
  return Status != -1 && WIFEXITED(Status) == true && WEXITSTATUS(Status) == 0;
}


/******************************************************************************
 * Fit a simulated spectrum:
 * The counts of the bins are Poisson distributed, thus a Poisson likelihood fit (option L, function integrated over the bins: option I) is used - a chi-square fit
 * with the errors sqrt(N) is biased and has wrong errors in bins with few counts. For a correct model the deviations of the fitted parameters from the
 * true values in units of the fit error are standard normal distributed, and the Poisson deviance of the fit follows a chi-square distribution.
 * The returned significance is the maximum of the deviations of the parameters in sigma and the deviation of the deviance from its expectation
 * in sigma (one sided: (deviance - ndf)/sqrt(2*ndf)). The threshold of 5 sigma (3e-7 per normal deviation) keeps the chance of a false failure of the test
 * negligible without relying on a random seed.
 */
double UTCosimaInputSpectra::Analyze(const SpectrumCase& Case)
{
  const unsigned int Bins = 50;
  TH1D* Spectrum = CreateSpectrum(Case.m_Prefix, Bins, Case.m_EMin, Case.m_EMax);
  if (Spectrum == nullptr) return -1;
  if (Spectrum->GetEntries() < 0.9*m_NumberOfTriggers) {
    cout<<"Too few entries in the spectrum: "<<Spectrum->GetEntries()<<endl;
    delete Spectrum;
    return -1;
  }

  const double ULow = log(Case.m_EMin);
  const double UHigh = log(Case.m_EMax);
  const double BinWidth = (UHigh - ULow)/Bins;
  TF1* Fit = new TF1(Case.m_Prefix + "Fit", Case.m_Formula, ULow, UHigh);
  for (unsigned int p = 1; p < Case.m_Start.size(); ++p) Fit->SetParameter(p, Case.m_Start[p]);
  // The normalization (parameter 0): the bin content is compared with the average of the function in the bin, thus the sum of the averages is the number of entries
  Fit->SetParameter(0, 1.0);
  Fit->SetParameter(0, Spectrum->GetEntries()/(Fit->Integral(ULow, UHigh)/BinWidth));

  TFitResultPtr Result = Spectrum->Fit(Fit, "LIMNS");
  if (Result.Get() == nullptr || Result->IsValid() == false) {
    cout<<"The fit of "<<Case.m_Name<<" failed"<<endl;
    delete Fit;
    delete Spectrum;
    return -1;
  }

  double Significance = 0.0;
  for (const pair<unsigned int, double>& T: Case.m_Truth) {
    const double Deviation = fabs(T.second - Fit->GetParameter(T.first))/Fit->GetParError(T.first);
    cout<<"  "<<Case.m_Name<<": parameter "<<T.first<<" = "<<Fit->GetParameter(T.first)<<" +- "<<Fit->GetParError(T.first)<<" (true: "<<T.second<<", "<<Deviation<<" sigma)"<<endl;
    Significance = max(Significance, Deviation);
  }

  // The Poisson deviance of the fit (counts n, expectation mu per bin): 2*(mu - n + n*ln(n/mu))
  double Deviance = 0.0;
  for (int b = 1; b <= Spectrum->GetNbinsX(); ++b) {
    const double n = Spectrum->GetBinContent(b);
    const double mu = Fit->Integral(Spectrum->GetXaxis()->GetBinLowEdge(b), Spectrum->GetXaxis()->GetBinUpEdge(b))/BinWidth;
    if (mu > 0) Deviance += 2*(mu - n + ((n > 0) ? n*log(n/mu) : 0.0));
  }
  const double NDF = Spectrum->GetNbinsX() - Case.m_Start.size();
  const double GoodnessSigma = (Deviance - NDF)/sqrt(2*NDF);
  cout<<"  "<<Case.m_Name<<": Poisson deviance "<<Deviance<<" for "<<NDF<<" degrees of freedom ("<<GoodnessSigma<<" sigma)"<<endl;
  Significance = max(Significance, GoodnessSigma);

  if (m_SingleMode == true) {
    TCanvas* C = new TCanvas();
    C->cd();
    gPad->SetLogy();
    Spectrum->DrawCopy();
    Fit->DrawCopy("SAME");
    C->Update();
  }

  delete Fit;
  delete Spectrum;

  return Significance;
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

  // The simulations write their files into the current directory: use a private one and remove it afterwards
  const filesystem::path OriginalDirectory = filesystem::current_path();
  const filesystem::path Directory = filesystem::temp_directory_path() / ("UTCosimaInputSpectraRun_" + to_string(chrono::high_resolution_clock::now().time_since_epoch().count()));
  error_code Error;
  if (filesystem::create_directories(Directory, Error) == false || Error) {
    cerr<<"Failed: unable to create the working directory "<<Directory<<endl;
    return 1;
  }
  filesystem::current_path(Directory);

  UTCosimaInputSpectra Test;
  bool Passed = Test.Run();

  filesystem::current_path(OriginalDirectory);
  filesystem::remove_all(Directory, Error);
  
  //Test.RunSingle();
  //ROOT.Run();

  return Passed ? 0 : 1;
}


/*
 * UTCosimaInputSpectra.cc: the end...
 ******************************************************************************/
