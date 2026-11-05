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
#include "UTParameterFileShared.h"

// MEGAlib:
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MGlobal.h"
#include "MProcessManager.h"
#include "MSimEvent.h"
#include "MSimIA.h"
#include "MSystem.h"

// ROOT:
#include "TApplication.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TH1.h"
#include "TROOT.h"

// Standard lib:
#include <cmath>
#include <cstdlib>
#include <future>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the spectrum keywords of the cosima source files: every keyword and alias is parsed into
//! the expected spectral type and parameters, and the spectra which cosima simulates follow the input spectra
class UTCosimaInputSpectra : public UTParameterFileTest
{
public:
  //! Default constructor
  UTCosimaInputSpectra() : UTParameterFileTest("UTCosimaInputSpectra") {}
  //! Default destructor
  virtual ~UTCosimaInputSpectra() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One spectrum which is simulated with cosima and fitted
  struct SimulationCase {
    //! Name for the output
    MString m_Name;
    //! File name prefix of the source, log, and sim file, and name of the private directory of the run
    MString m_Prefix;
    //! The Spectrum keyword value
    MString m_Spectrum;
    //! The random seed of cosima
    unsigned int m_Seed;
    //! The energy range in keV
    double m_EMin;
    double m_EMax;
    //! The fit function and the start values of its parameters (parameter 0 is the normalization and is set automatically)
    //! The x-axis of the fit is u = ln(E/keV): the function is dN/du = E*dN/dE, the bins are uniform in u (see ReadSpectrum())
    MString m_Formula;
    vector<double> m_Start;
    //! The parameters (index in the fit function, true value) which have to be reproduced
    vector<pair<unsigned int, double>> m_Truth;
  };

  //! Return the spectra which are simulated
  vector<SimulationCase> GetSimulationCases() const;
  //! Write the source file of a simulation into its directory
  bool WriteSimulationSource(const MString& Directory, const SimulationCase& Case) const;
  //! Run cosima on the source file in the directory (external program, can run in parallel with others) and return its wait status (0: exited normally with 0)
  int RunCosima(const MString& Directory, const SimulationCase& Case) const;
  //! Fill the energies of the first interactions of the sim file into a histogram with uniform bins in ln(E/keV), return nullptr if the file cannot be read
  //! Count the events without a finite energy
  TH1D* ReadSpectrum(const MString& Directory, const SimulationCase& Case, unsigned int& NonFiniteEnergies);
  //! Read the simulated spectrum, fit it, and evaluate the deviations from the true spectrum (ROOT: main thread only)
  bool EvaluateSimulatedSpectrum(const MString& Directory, const SimulationCase& Case);

  //! Test that every spectrum keyword and alias is parsed into the expected spectral type and parameters
  bool TestSpectrumKeywords();
  //! Test that the spectra simulated by cosima follow the input spectra
  bool TestSimulatedSpectra();

  //! The number of triggers of each simulation
  static const unsigned int c_NumberOfTriggers = 100000;
  //! The number of bins of the fitted spectra
  static const unsigned int c_NumberOfBins = 50;
  //! The allowed deviation of the fitted parameters and of the deviance in standard deviations: 5 sigma (3e-7 per normal deviation)
  //! keeps the chance of a false failure negligible
  static constexpr double c_NumberOfSigmas = 5.0;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputSpectra::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", PrepareFiles()) && Passed;
  if (Passed == false) {
    Summarize();
    return Passed;
  }

  Passed = TestSpectrumKeywords() && Passed;
  Passed = TestSimulatedSpectra() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputSpectra::TestSpectrumKeywords()
{
  bool Passed = true;

  const MString SpectrumFile = WriteSource("Spectrum.dat", "IP LIN\nDP 10 1\nDP 100 0.1\n");
  const MString Data = GetExampleDirectory();

  //! One spectrum: the name for the output, the Spectrum keyword line, the expected spectral type, and the expected parameters (energies in the Geant4 energy unit)
  struct SpectrumCase {
    MString m_Name;
    MString m_Line;
    int m_Type;
    vector<double> m_Parameters;
  };
  vector<SpectrumCase> Cases = {
    { "Mono", "Mono 511", MCSource::c_Monoenergetic, { 511*keV } },
    { "Mono alias", "M 511", MCSource::c_Monoenergetic, { 511*keV } },
    { "Linear", "Linear 10 100", MCSource::c_Linear, { 10*keV, 100*keV } },
    { "Linear alias", "L 10 100", MCSource::c_Linear, { 10*keV, 100*keV } },
    { "PowerLaw", "PowerLaw 10 1000 2", MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2 } },
    { "PowerLaw alias", "PL 10 1000 2", MCSource::c_PowerLaw, { 10*keV, 1000*keV, 2 } },
    { "BrokenPowerLaw", "BrokenPowerLaw 10 1000 100 1.5 2.5", MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 } },
    { "BrokenPowerLaw alias", "BPL 10 1000 100 1.5 2.5", MCSource::c_BrokenPowerLaw, { 10*keV, 1000*keV, 100*keV, 1.5, 2.5 } },
    { "CutOffPowerLaw", "CutOffPowerLaw 10 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 500*keV } },
    { "CutOffPowerLaw alias", "CPL 10 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 500*keV } },
    { "Comptonized", "Comptonized 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV } },
    { "Comptonization alias", "Comptonization 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV } },
    { "Comp alias", "Comp 10 1000 -1.2 200", MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.2, 200*keV } },
    { "Gaussian", "Gaussian 100 10 3", MCSource::c_Gaussian, { 100*keV, 10*keV, 3 } },
    { "Gaussian alias", "G 100 10 3", MCSource::c_Gaussian, { 100*keV, 10*keV, 3 } },
    { "ThermalBremsstrahlung", "ThermalBremsstrahlung 10 1000 50", MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 50*keV } },
    { "ThermalBremsstrahlung alias", "TB 10 1000 50", MCSource::c_ThermalBremsstrahlung, { 10*keV, 1000*keV, 50*keV } },
    { "BlackBody", "BlackBody 10 1000 50", MCSource::c_BlackBody, { 10*keV, 1000*keV, 50*keV } },
    { "BlackBody alias", "BB 10 1000 50", MCSource::c_BlackBody, { 10*keV, 1000*keV, 50*keV } },
    { "BandFunction", "BandFunction 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV } },
    { "Band alias", "Band 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV } },
    { "BF alias", "BF 10 1000 -1 -2.5 200", MCSource::c_BandFunction, { 10*keV, 1000*keV, -1, -2.5, 200*keV } },
    { "File", "File " + SpectrumFile, MCSource::c_FileDifferentialFlux, {} },
    { "File alias", "F " + SpectrumFile, MCSource::c_FileDifferentialFlux, {} }
  };
  if (Data.IsEmpty() == false) {
    Cases.push_back({ "NormalizedEnergyBeamFluxFunction", "NormalizedEnergyBeamFluxFunction", MCSource::c_NormalizedEnergyBeamFluxFunction, {} });
  } else {
    mout<<"MEGALIB is not set: the normalized energy beam flux function is not tested"<<endl;
  }

  for (const SpectrumCase& C: Cases) {
    // The normalized energy beam flux function needs its beam and no flux, all others a point source with a flux
    MString Lines = "S.ParticleType 1\n";
    if (C.m_Type == MCSource::c_NormalizedEnergyBeamFluxFunction) {
      Lines += "S.Beam FarFieldNormalizedEnergyBeamFluxFunction " + Data + "/3DSpherical.dat\n";
    } else {
      Lines += "S.Beam PointSource 0 0 0\nS.Flux 1\n";
    }
    Lines += "S.Spectrum " + C.m_Line + "\n";

    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", C.m_Name, "The spectrum line is accepted by the parser", Open(P, OneSource(Lines))) && Passed;
    MCSource* S = GetS(P);
    Passed = EvaluateTrue("GetSource()", C.m_Name, "The source S exists after parsing", S != nullptr) && Passed;
    if (S == nullptr) {
      continue;
    }
    Passed = Evaluate("GetSpectralType()", C.m_Name, "The spectral type is the one of the keyword", S->GetSpectralType(), C.m_Type) && Passed;
    for (unsigned int p = 0; p < C.m_Parameters.size(); ++p) {
      // The parameters are read from text and multiplied by their unit: exact to the rounding of a double
      const double Expected = C.m_Parameters[p];
      Passed = EvaluateNear("GetEnergyParameter()", C.m_Name + " parameter " + (p+1), "The energy parameter is stored in its unit", S->GetEnergyParameter(p+1), Expected, 1e-12*max(1.0, fabs(Expected))) && Passed;
    }
  }

  // The deprecated numeric spectral type remains accepted for old source files
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "SpectralType", "The deprecated numeric spectral type is accepted", Open(P, OneSource(MString("S.ParticleType 1\nS.Beam PointSource 0 0 0\nS.SpectralType ") + MCSource::c_PowerLaw + "\n"))) && Passed;
    MCSource* S = GetS(P);
    Passed = EvaluateTrue("GetSource()", "SpectralType", "The source S exists after parsing", S != nullptr) && Passed;
    if (S != nullptr) {
      Passed = Evaluate("GetSpectralType()", "SpectralType", "The spectral type is the one of the numeric code", S->GetSpectralType(), MCSource::c_PowerLaw) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


vector<UTCosimaInputSpectra::SimulationCase> UTCosimaInputSpectra::GetSimulationCases() const
{
  vector<SimulationCase> Cases;
  // Power law: dN/dE = N*E^-index, i.e. dN/du = N*E^(1-index)
  Cases.push_back({ "Power law", "PowerLaw", "PowerLaw 10 10000 2", 101, 10, 10000, "[0]*exp((1-[1])*x)", { 1.0, 2.5 }, { { 1, 2.0 } } });
  // Cut-off power law: dN/dE = N*E^-index*exp(-E/cutoff)
  Cases.push_back({ "Cut-off power law", "CutOffPowerLaw", "CutOffPowerLaw 10 10000 0.5 1000", 202, 10, 10000, "[0]*exp((1-[1])*x)*exp(-exp(x)/[2])", { 1.0, 1.0, 500.0 }, { { 1, 0.5 }, { 2, 1000.0 } } });
  // Comptonized: dN/dE = N*E^alpha*exp(-(alpha + 2)*E/Epeak)
  Cases.push_back({ "Comptonized", "Comptonized", "Comptonized 10 10000 -1.5 200", 303, 10, 10000, "[0]*exp(([1]+1)*x)*exp(-([1]+2)*exp(x)/[2])", { 1.0, -1.4, 220.0 }, { { 1, -1.5 }, { 2, 200.0 } } });
  return Cases;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputSpectra::WriteSimulationSource(const MString& Directory, const SimulationCase& Case) const
{
  MString Source;
  Source += "# Unit test source file\n";
  Source += "Version 1\n";
  Source += "Geometry $(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup\n";
  Source += "PhysicsListEM LivermorePol\n";
  Source += "StoreSimulationInfo init\n";
  Source += "Run UnitTest\n";
  Source += "UnitTest.FileName " + Case.m_Prefix + "\n";
  Source += MString("UnitTest.Triggers ") + c_NumberOfTriggers + "\n";
  Source += "UnitTest.Source UnitTestSource\n";
  Source += "UnitTestSource.ParticleType 1\n";
  Source += "UnitTestSource.Beam PointSource 0 0 0\n";
  Source += "UnitTestSource.Spectrum " + Case.m_Spectrum + "\n";
  Source += "UnitTestSource.Flux 1\n";
  return WriteTextFile(Directory + "/" + Case.m_Prefix + ".source", Source);
}


////////////////////////////////////////////////////////////////////////////////


int UTCosimaInputSpectra::RunCosima(const MString& Directory, const SimulationCase& Case) const
{
  // Each run has its own directory, seed, and log file; the time limit turns a hang into a failure
  // The run stays in the process group of the test: the test driver stops it with the test if the test times out
  return MProcessManager::Run("cosima", MString("-v 0 -s ") + Case.m_Seed + " -z " + MSystem::GetShellQuoted(Case.m_Prefix + ".source"), Directory + "/" + Case.m_Prefix + ".log", Directory, 100);
}


////////////////////////////////////////////////////////////////////////////////


TH1D* UTCosimaInputSpectra::ReadSpectrum(const MString& Directory, const SimulationCase& Case, unsigned int& NonFiniteEnergies)
{
  NonFiniteEnergies = 0;

  // The geometry and the reader print a lot
  SilenceOutput();
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  const bool GeometryLoaded = Geometry->ScanSetupFile("$(MEGALIB)/src/cosima/unittests/GiganticSphere.geo.setup");
  RestoreOutput();
  if (GeometryLoaded == false) {
    merr<<"Unable to load the geometry GiganticSphere.geo.setup"<<endl;
    delete Geometry;
    return nullptr;
  }

  const MString SimFile = Directory + "/" + Case.m_Prefix + ".inc1.id1.sim.gz";
  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  SilenceOutput();
  const bool Opened = Reader->Open(SimFile);
  RestoreOutput();
  if (Opened == false) {
    merr<<"Unable to open the sim file "<<SimFile<<endl;
    delete Reader;
    delete Geometry;
    return nullptr;
  }

  // Uniform bins in u = ln(E/keV): the spectra fall by orders of magnitude, linear bins would leave the high-energy bins almost empty,
  // and ROOT compares the bin content with the average function value of the bin, which is only right for equal bin widths
  TH1D* Spectrum = new TH1D(Case.m_Prefix + "Spectrum", Case.m_Prefix + "Spectrum", c_NumberOfBins, log(Case.m_EMin), log(Case.m_EMax));
  Spectrum->SetDirectory(nullptr);

  SilenceOutput();
  MSimEvent* Event = nullptr;
  while ((Event = Reader->GetNextEvent()) != nullptr) {
    if (Event->GetNIAs() >= 1) {
      const double Energy = Event->GetIAAt(0)->GetSecondaryEnergy();
      if (std::isfinite(Energy) == true) {
        Spectrum->Fill(log(Energy));
      } else {
        ++NonFiniteEnergies;
      }
    }
    delete Event;
  }
  Reader->Close();
  RestoreOutput();

  delete Reader;
  delete Geometry;

  return Spectrum;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputSpectra::EvaluateSimulatedSpectrum(const MString& Directory, const SimulationCase& Case)
{
  bool Passed = true;

  unsigned int NonFiniteEnergies = 0;
  TH1D* Spectrum = ReadSpectrum(Directory, Case, NonFiniteEnergies);
  Passed = EvaluateTrue("ReadSpectrum()", Case.m_Name, "The sim file of the simulation can be read", Spectrum != nullptr) && Passed;
  if (Spectrum == nullptr) {
    return Passed;
  }
  Passed = Evaluate("ReadSpectrum()", Case.m_Name, "No start energy is infinite or NaN", NonFiniteEnergies, 0U) && Passed;
  // Every triggered event has its start energy in the sim file: the number of entries is the number of triggers
  Passed = Evaluate("ReadSpectrum()", Case.m_Name, "The spectrum contains one start energy per trigger", static_cast<unsigned int>(Spectrum->GetEntries()), c_NumberOfTriggers) && Passed;

  // The counts of the bins are Poisson distributed, thus a Poisson likelihood fit (option L, function integrated over the bins: option I) is used:
  // a chi-square fit with the errors sqrt(N) is biased and has wrong errors in bins with few counts. For a correct model the deviations of the
  // fitted parameters from the true values in units of the fit error are standard normal distributed, and the Poisson deviance of the fit follows
  // a chi-square distribution with mean ndf and variance 2*ndf
  const double ULow = log(Case.m_EMin);
  const double UHigh = log(Case.m_EMax);
  const double BinWidth = (UHigh - ULow)/c_NumberOfBins;
  TF1* Fit = new TF1(Case.m_Prefix + "Fit", Case.m_Formula, ULow, UHigh);
  for (unsigned int p = 1; p < Case.m_Start.size(); ++p) {
    Fit->SetParameter(p, Case.m_Start[p]);
  }
  // The normalization (parameter 0): the bin content is compared with the average of the function in the bin, thus the sum of the averages is the number of entries
  Fit->SetParameter(0, 1.0);
  Fit->SetParameter(0, Spectrum->GetEntries()/(Fit->Integral(ULow, UHigh)/BinWidth));

  SilenceOutput();
  TFitResultPtr Result = Spectrum->Fit(Fit, "LIMNSQ");
  RestoreOutput();
  const bool FitValid = Result.Get() != nullptr && Result->IsValid() == true;
  Passed = EvaluateTrue("Fit()", Case.m_Name, "The Poisson likelihood fit of the spectrum converges", FitValid) && Passed;
  if (FitValid == true) {
    for (const pair<unsigned int, double>& T: Case.m_Truth) {
      Passed = EvaluateNear("Fit()", Case.m_Name + " parameter " + T.first, "The fitted parameter agrees with the input spectrum within 5 standard errors",
                            Fit->GetParameter(T.first), T.second, c_NumberOfSigmas*Fit->GetParError(T.first)) && Passed;
    }

    // The Poisson deviance of the fit (counts n, expectation mu per bin): 2*(mu - n + n*ln(n/mu))
    double Deviance = 0.0;
    for (int b = 1; b <= Spectrum->GetNbinsX(); ++b) {
      const double n = Spectrum->GetBinContent(b);
      const double mu = Fit->Integral(Spectrum->GetXaxis()->GetBinLowEdge(b), Spectrum->GetXaxis()->GetBinUpEdge(b))/BinWidth;
      if (mu > 0) {
        Deviance += 2*(mu - n);
        if (n > 0) {
          Deviance += 2*n*log(n/mu);
        }
      }
    }
    const double NDF = Spectrum->GetNbinsX() - Case.m_Start.size();
    Passed = EvaluateNear("Fit()", Case.m_Name + " deviance", "The Poisson deviance of the fit agrees with the degrees of freedom within 5 sigma", Deviance, NDF, c_NumberOfSigmas*sqrt(2*NDF)) && Passed;
  }

  delete Fit;
  delete Spectrum;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaInputSpectra::TestSimulatedSpectra()
{
  bool Passed = true;

  const vector<SimulationCase> Cases = GetSimulationCases();

  // Each simulation gets its own directory below the private temporary root
  vector<MString> Directories;
  for (const SimulationCase& C: Cases) {
    Directories.push_back(GetTemporaryDirectoryName(C.m_Prefix));
    Passed = EvaluateTrue("PrepareTemporaryDirectory()", C.m_Name, "The directory of the simulation can be created", PrepareTemporaryDirectory(C.m_Prefix)) && Passed;
    Passed = EvaluateTrue("WriteSimulationSource()", C.m_Name, "The source file of the simulation can be written", WriteSimulationSource(Directories.back(), C)) && Passed;
  }
  if (Passed == false) {
    return Passed;
  }

  // The simulations are external programs and run in parallel; all ROOT work is done afterwards in the main thread
  vector<future<int>> Simulations;
  for (unsigned int c = 0; c < Cases.size(); ++c) {
    Simulations.push_back(async(launch::async, [this, &Directories, &Cases, c]() { return RunCosima(Directories[c], Cases[c]); }));
  }
  vector<int> Status;
  for (unsigned int c = 0; c < Cases.size(); ++c) {
    Status.push_back(Simulations[c].get());
  }

  for (unsigned int c = 0; c < Cases.size(); ++c) {
    Passed = EvaluateTrue("RunCosima()", Cases[c].m_Name, "Cosima exits normally within the time limit", Status[c] == 0) && Passed;
    Passed = EvaluateSimulatedSpectrum(Directories[c], Cases[c]) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // ROOT is needed for the histograms and fits, without any graphics
  TApplication ROOT("ROOT", 0, 0);
  gROOT->SetBatch(true);

  // The geometry and the sim file reader need the MEGAlib globals
  MGlobal::Initialize("UTCosimaInputSpectra", "unit test of the cosima input spectra");

  UTCosimaInputSpectra Test;
  return Test.Run() == true ? 0 : 1;
}


/*
 * UTCosimaInputSpectra.cc: the end...
 ******************************************************************************/
