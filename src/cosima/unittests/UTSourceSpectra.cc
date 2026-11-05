/*
 * UTSourceSpectra.cc
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
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MUnitTest.h"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the energy generation of MCSource: every spectral type is sampled with a fixed seed
//! and compared with an independently integrated reference distribution (Kolmogorov-Smirnov)
class UTSourceSpectra : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceSpectra() : UTSourceTestBase("UTSourceSpectra") {}
  //! Default destructor
  virtual ~UTSourceSpectra() {}

  //! Run all tests
  virtual bool Run();

private:
  //! The number of samples per distribution
  static const unsigned int c_Samples = 20000;

  //! Test all analytic spectra against their reference distribution
  bool TestAnalyticSpectra();
  //! Test the spectrum from a file
  bool TestFileSpectrum();
  //! Test the interpolation modes of file spectra: generated distribution and mean energy
  bool TestFileInterpolation();
  //! Test the simple spectra: mono, activation, unimplemented
  bool TestSimpleSpectra();
  //! Test the Gaussian, including its cut-off
  bool TestGaussian();
  //! Test that the Gaussian never generates energies below zero
  bool TestGaussianPositive();
  //! Test the failure path of the loops with a maximum number of iterations
  bool TestLoopLimits();
  //! Test that the generation is reproducible with the seed
  bool TestReproducibility();

  //! Return the Kolmogorov-Smirnov distance between the samples and the density on [A, B]
  static double KSDistance(vector<double> Samples, const function<double(double)>& Density, double A, double B)
  {
    sort(Samples.begin(), Samples.end());
    // Cumulative distribution on a fine grid:
    const unsigned int Grid = 40000;
    vector<double> Cumulative(Grid + 1, 0.0);
    const double h = (B - A)/Grid;
    for (unsigned int i = 1; i <= Grid; ++i) {
      Cumulative[i] = Cumulative[i-1] + 0.5*h*(Density(A + (i-1)*h) + Density(A + i*h));
    }
    auto CDF = [&](double X) {
      if (X <= A) return 0.0;
      if (X >= B) return 1.0;
      const double Position = (X - A)/h;
      const unsigned int i = min((unsigned int) Position, Grid - 1);
      return (Cumulative[i] + (Position - i)*(Cumulative[i+1] - Cumulative[i]))/Cumulative[Grid];
    };
    double D = 0.0;
    for (unsigned int i = 0; i < Samples.size(); ++i) {
      const double F = CDF(Samples[i]);
      D = max(D, max(fabs(F - double(i)/Samples.size()), fabs(F - double(i + 1)/Samples.size())));
    }
    return D;
  }

  //! Critical KS distance at about the 0.1% level
  static double Critical(unsigned int N) { return 1.95/sqrt(double(N)); }

  //! One analytic spectrum
  struct Spectrum {
    //! Name
    string m_Name;
    //! Spectral type
    int m_Type;
    //! Parameters in keV or unitless as in the source file
    vector<double> m_Parameters;
    //! Indicates which parameters are energies (scaled with keV)
    vector<bool> m_IsEnergy;
    //! The density in keV
    function<double(double)> m_Density;
    //! Lower end of the range
    double m_Min;
    //! Upper end of the range
    double m_Max;
  };
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("InitializeEnvironment()", "spectra", "The Geant4 environment can be created", InitializeEnvironment()) && Passed;

  Passed = TestSimpleSpectra() && Passed;
  Passed = TestAnalyticSpectra() && Passed;
  Passed = TestFileSpectrum() && Passed;
  Passed = TestFileInterpolation() && Passed;
  Passed = TestGaussian() && Passed;
  Passed = TestGaussianPositive() && Passed;
  Passed = TestLoopLimits() && Passed;
  Passed = TestReproducibility() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestSimpleSpectra()
{
  bool Passed = true;
  RandomGuard Guard(4711);

  // Monoenergetic:
  {
    MCSource S("Mono");
    Passed = EvaluateTrue("PrepareNearFieldPointSource()", "mono", "The source can be prepared", PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Monoenergetic, { 511*keV })) && Passed;
    bool OK = false;
    vector<double> E = SampleEnergies(S, 100, &OK);
    Passed = EvaluateTrue("GenerateEnergy()", "mono", "The generation succeeds", OK) && Passed;
    UTSourceStatistics T = Statistics(E);
    Passed = EvaluateNear("GenerateEnergy()", "mono, min", "All energies are the line energy (min)", T.m_Min, 511.0, 1e-9) && Passed;
    Passed = EvaluateNear("GenerateEnergy()", "mono, max", "All energies are the line energy (max)", T.m_Max, 511.0, 1e-9) && Passed;
  }

  // Activation:
  {
    MCSource S("Activation");
    SilenceOutput();
    S.SetParticleType(MCSource::c_Gamma);
    S.SetSpectralType(MCSource::c_Activation);
    S.SetEnergy();
    RestoreOutput();
    bool OK = false;
    vector<double> E = SampleEnergies(S, 10, &OK);
    Passed = EvaluateTrue("GenerateEnergy()", "activation", "The generation succeeds", OK) && Passed;
    Passed = EvaluateNear("GenerateEnergy()", "activation", "An activation source starts with energy zero", Statistics(E).m_Max, 0.0, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestAnalyticSpectra()
{
  bool Passed = true;

  const double Emin = 10.0;
  const double Emax = 1000.0;

  vector<Spectrum> Spectra;
  auto Add = [&](const string& Name, int Type, vector<double> Parameters, vector<bool> IsEnergy, function<double(double)> Density, double Min, double Max) {
    Spectra.push_back({ Name, Type, Parameters, IsEnergy, Density, Min, Max });
  };

  Add("linear", MCSource::c_Linear, { Emin, Emax }, { true, true }, [](double) { return 1.0; }, Emin, Emax);
  for (double Index: { 1.0, 2.0, 1.7, 0.0, -0.5 }) {
    Add(string("power law ") + to_string(Index), MCSource::c_PowerLaw, { Emin, Emax, Index }, { true, true, false }, [Index](double E) { return pow(E, -Index); }, Emin, Emax);
  }
  for (const pair<double, double>& I: vector<pair<double, double>>({ { 1.5, 2.5 }, { 0.5, 3.0 }, { 2.0, 1.0 } })) {
    const double Eb = 100.0;
    Add(string("broken power law ") + to_string(I.first) + "/" + to_string(I.second), MCSource::c_BrokenPowerLaw, { Emin, Emax, Eb, I.first, I.second }, { true, true, true, false, false },
        [=](double E) { return (E <= Eb) ? pow(E/Emin, -I.first) : pow(Eb/Emin, -I.first)*pow(E/Eb, -I.second); }, Emin, Emax);
  }
  for (const pair<double, double>& I: vector<pair<double, double>>({ { 1.0, 300.0 }, { 1.5, 300.0 }, { 2.0, 50.0 }, { 0.5, 100.0 } })) {
    Add(string("cut-off power law ") + to_string(I.first) + "/" + to_string(I.second), MCSource::c_CutOffPowerLaw, { Emin, Emax, I.first, I.second }, { true, true, false, true },
        [=](double E) { return pow(E, -I.first)*exp(-E/I.second); }, Emin, Emax);
  }
  for (const pair<double, double>& I: vector<pair<double, double>>({ { -1.0, 200.0 }, { 1.0, 300.0 }, { -1.5, 100.0 } })) {
    Add(string("comptonized ") + to_string(I.first) + "/" + to_string(I.second), MCSource::c_Comptonized, { Emin, Emax, I.first, I.second }, { true, true, false, true },
        [=](double E) { return pow(E, I.first)*exp(-(I.first + 2)*E/I.second); }, Emin, Emax);
  }
  for (double T: { 30.0, 300.0 }) {
    Add(string("thermal bremsstrahlung ") + to_string(T), MCSource::c_ThermalBremsstrahlung, { Emin, Emax, T }, { true, true, true },
        [T](double E) { return exp(-E/T)/E; }, Emin, Emax);
  }
  for (double T: { 30.0, 5.0, 2000.0 }) {
    Add(string("black body ") + to_string(T), MCSource::c_BlackBody, { Emin, Emax, T }, { true, true, true },
        [T](double E) { return E*E/(exp(E/T) - 1); }, Emin, Emax);
  }
  // Alpha, beta, E0: the cases with alpha > 0 rise up to alpha*E0 (inside the range for 0.5/200, 2/100 and 3/50, above the range for 1/2000, where the
  // maximum is at the upper end), the cases with E0 = 5 and 2000 have the break energy outside of the range; (-1, -2.3, 200) is the classical GRB case
  for (const vector<double>& I: vector<vector<double>>({ { -1.0, -2.3, 200.0 }, { -0.5, -1.5, 200.0 }, { -1.9, -3.0, 200.0 }, { 0.5, -2.5, 200.0 }, { 0.0, -2.0, 200.0 },
                                                          { 2.0, -3.0, 100.0 }, { 3.0, -1.0, 50.0 }, { 1.0, -2.5, 2000.0 }, { -1.0, -2.3, 5.0 } })) {
    const double Alpha = I[0];
    const double Beta = I[1];
    const double E0 = I[2];
    Add(string("band ") + to_string(Alpha) + "/" + to_string(Beta) + "/" + to_string(E0), MCSource::c_BandFunction, { Emin, Emax, Alpha, Beta, E0 }, { true, true, false, false, true },
        [=](double E) {
          const double Break = (Alpha - Beta)*E0;
          return (E <= Break) ? pow(E, Alpha)*exp(-E/E0) : pow(Break, Alpha - Beta)*exp(Beta - Alpha)*pow(E, Beta);
        }, Emin, Emax);
  }

  unsigned int Index = 0;
  for (const Spectrum& P: Spectra) {
    RandomGuard Guard(1000 + Index++);

    vector<double> Parameters;
    for (unsigned int i = 0; i < P.m_Parameters.size(); ++i) Parameters.push_back(P.m_IsEnergy[i] ? P.m_Parameters[i]*keV : P.m_Parameters[i]);

    MCSource S("Spectrum");
    bool Prepared = PrepareNearFieldPointSource(S, MCSource::c_Gamma, P.m_Type, Parameters);
    Passed = EvaluateTrue("PrepareNearFieldPointSource()", P.m_Name, "The source can be prepared", Prepared) && Passed;
    if (Prepared == false) continue;

    bool OK = false;
    vector<double> E = SampleEnergies(S, c_Samples, &OK);
    Passed = EvaluateTrue("GenerateEnergy()", P.m_Name, "The generation succeeds for all samples", OK) && Passed;
    if (E.size() == 0) continue;

    UTSourceStatistics T = Statistics(E);
    Passed = EvaluateNear("GenerateEnergy()", P.m_Name + ", range, min", "No energy is below the energy range (shown: the excess below the limit)", max(P.m_Min - T.m_Min, 0.0), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GenerateEnergy()", P.m_Name + ", range, max", "No energy is above the energy range (shown: the excess above the limit)", max(T.m_Max - P.m_Max, 0.0), 0.0, 1e-9) && Passed;

    const double D = KSDistance(E, P.m_Density, P.m_Min, P.m_Max);
    Passed = EvaluateNear("GenerateEnergy()", P.m_Name + ", KS " + to_string(D), "The energies follow the reference distribution (KS distance below the critical value)", D, 0.0, Critical(E.size())) && Passed;

    // The mean energy of the reference and the sample agree within five standard errors:
    const double ReferenceMean = MeanOfDensity(P.m_Density, P.m_Min, P.m_Max);
    Passed = EvaluateNear("GenerateEnergy()", P.m_Name + ", mean", "The sample mean agrees with the reference mean within five standard errors", T.m_Mean, ReferenceMean, 5*T.m_Sigma/sqrt(double(T.m_N))) && Passed;
  }

  // Spectra whose range is reduced because the spectrum is negligible: the samples stay inside the original range
  {
    RandomGuard Guard(77);
    MCSource S("Steep comptonized");
    Passed = EvaluateTrue("PrepareNearFieldPointSource()", "steep comptonized", "The source can be prepared", PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Comptonized, { 10*keV, 1000*keV, -1.0, 2*keV })) && Passed;
    bool OK = false;
    vector<double> E = SampleEnergies(S, c_Samples, &OK);
    Passed = EvaluateTrue("GenerateEnergy()", "steep comptonized", "The generation succeeds", OK) && Passed;
    const double D = KSDistance(E, [](double X) { return pow(X, -1.0)*exp(-X/2.0); }, 10.0, 60.0);
    Passed = EvaluateNear("GenerateEnergy()", "steep comptonized, KS " + to_string(D), "The energies follow the steep reference distribution", D, 0.0, Critical(E.size())) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestFileInterpolation()
{
  bool Passed = true;

  // The table points of the spectrum (keV, any flux unit) and the density of each interpolation mode between them, written down independently of MFunction:
  // LinLin: linear in E and y, LinLog: ln(y) linear in E, LogLin: y linear in ln(E), LogLog: power law, None: nearest table point (the value changes at the midpoints)
  const vector<double> X = { 100.0, 200.0, 400.0, 1000.0 };
  const vector<double> Y = { 1.0, 3.0, 0.5, 2.0 };
  struct Mode {
    const char* m_Keyword;
    function<double(double)> m_Density;
    //! The points at which the density has a kink or a jump: the reference integration is done piece by piece
    vector<double> m_Breaks;
    //! The known defect of the sampling in MFunction::GetRandom (outside of MCSource) for this mode, or nullptr
    const char* m_SamplingDefect;
  };
  auto Segment = [&X](double E) {
    unsigned int i = 0;
    while (i + 2 < X.size() && E > X[i+1]) ++i;
    return i;
  };
  const vector<Mode> Modes = {
    { "LinLin", [&](double E) { unsigned int i = Segment(E); return Y[i] + (Y[i+1] - Y[i])*(E - X[i])/(X[i+1] - X[i]); }, X, nullptr },
    { "LinLog", [&](double E) { unsigned int i = Segment(E); return exp(log(Y[i]) + (log(Y[i+1]) - log(Y[i]))*(E - X[i])/(X[i+1] - X[i])); }, X, nullptr },
    { "LogLin", [&](double E) { unsigned int i = Segment(E); return Y[i] + (Y[i+1] - Y[i])*(log(E) - log(X[i]))/(log(X[i+1]) - log(X[i])); }, X, "MFunction::GetRandom, LogLin: the inversion uses a LambertW formula which is 'just a guess' in the code, all energies are 0" },
    { "LogLog", [&](double E) { unsigned int i = Segment(E); return exp(log(Y[i]) + (log(Y[i+1]) - log(Y[i]))*(log(E) - log(X[i]))/(log(X[i+1]) - log(X[i]))); }, X, nullptr },
    { "None", [&](double E) {
        // The table point which is closest to E (the midpoint belongs to the upper point)
        unsigned int i = Segment(E);
        return (E >= 0.5*(X[i] + X[i+1])) ? Y[i+1] : Y[i];
      }, { 100.0, 150.0, 200.0, 300.0, 400.0, 700.0, 1000.0 }, "MFunction::GetRandom, None: the energies are drawn uniformly within each table interval instead of following the step function" }
  };

  unsigned int Index = 0;
  for (const Mode& M: Modes) {
    RandomGuard Guard(1400 + Index++);
    const MString Name = MString("file spectrum, ") + M.m_Keyword;
    const MString File = GetTemporaryFileName(MString("Interpolation") + M.m_Keyword + ".dat");
    MString Content = MString("IP ") + M.m_Keyword + "\n";
    for (unsigned int i = 0; i < X.size(); ++i) Content += MString("DP ") + X[i] + " " + Y[i] + "\n";
    Content += "EN\n";
    Passed = EvaluateTrue("WriteTextFile()", Name, "The spectrum fixture can be written", WriteTextFile(File, Content)) && Passed;

    MCSource S("Interpolation");
    SilenceOutput();
    S.SetParticleType(MCSource::c_Gamma);
    S.SetSpectralType(MCSource::c_FileDifferentialFlux);
    bool OK = S.SetEnergy(File);
    RestoreOutput();
    Passed = EvaluateTrue("SetEnergy()", Name, "The spectrum file is accepted", OK) && Passed;

    // The reference mean: Simpson integration piece by piece (every piece smooth, 2000 intervals)
    double Moment0 = 0.0, Moment1 = 0.0;
    for (unsigned int b = 0; b + 1 < M.m_Breaks.size(); ++b) {
      // Evaluate inside the piece: at the break the density of None is ambiguous
      const double Low = M.m_Breaks[b], High = M.m_Breaks[b+1];
      const unsigned int N = 2000;
      const double h = (High - Low)/N;
      for (unsigned int i = 0; i <= N; ++i) {
        const double W = (i == 0 || i == N) ? 1.0 : ((i % 2 == 1) ? 4.0 : 2.0);
        const double E = (i == 0) ? Low + 1e-9*h : ((i == N) ? High - 1e-9*h : Low + i*h);
        Moment0 += W*M.m_Density(E)*h/3.0;
        Moment1 += W*M.m_Density(E)*E*h/3.0;
      }
    }
    const double ExpectedMean = Moment1/Moment0;
    Passed = EvaluateNear("GetMeanEnergy()", Name, "The mean energy is the ratio of the integrals of E*f(E) and f(E) of the interpolated spectrum", S.GetMeanEnergy()/keV, ExpectedMean, 1e-6*ExpectedMean) && Passed;

    bool Generated = false;
    vector<double> E = SampleEnergies(S, c_Samples, &Generated);
    Passed = EvaluateTrue("GenerateEnergy()", Name, "The generation succeeds", Generated) && Passed;
    // The generation of a file spectrum is done by MFunction::GetRandom: where it has a known defect for a mode, the checks below fail until MFunction is fixed
    const MString Defect = (M.m_SamplingDefect != nullptr) ? MString(" (KNOWN DEFECT: ") + M.m_SamplingDefect + ")" : MString("");
    const double D = KSDistance(E, M.m_Density, X.front(), X.back());
    if (M.m_SamplingDefect != nullptr) {
      // Known failure: MFunction::GetRandom() samples LogLin with a guessed LambertW inversion (all energies 0) and None uniformly within each table interval instead of by the step function
      Passed = ExpectedToFail(EvaluateNear("GenerateEnergy()", Name + ", KS " + to_string(D), MString("The generated energies follow the interpolated spectrum") + Defect, D, 0.0, Critical(E.size()))) && Passed;
      // Known failure: MFunction::GetRandom() samples LogLin with a guessed LambertW inversion (all energies 0) and None uniformly within each table interval instead of by the step function
      Passed = ExpectedToFail(EvaluateNear("GenerateEnergy()", Name + ", mean", MString("The mean of the generated energies is the mean energy of the spectrum (5 standard errors)") + Defect, Statistics(E).m_Mean, ExpectedMean, 5*Statistics(E).m_Sigma/sqrt(double(E.size())))) && Passed;
    } else {
      Passed = EvaluateNear("GenerateEnergy()", Name + ", KS " + to_string(D), MString("The generated energies follow the interpolated spectrum") + Defect, D, 0.0, Critical(E.size())) && Passed;
      Passed = EvaluateNear("GenerateEnergy()", Name + ", mean", MString("The mean of the generated energies is the mean energy of the spectrum (5 standard errors)") + Defect, Statistics(E).m_Mean, ExpectedMean, 5*Statistics(E).m_Sigma/sqrt(double(E.size()))) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestFileSpectrum()
{
  bool Passed = true;
  RandomGuard Guard(2024);

  const MString File = GetTemporaryFileName("Differential.dat");
  Passed = EvaluateTrue("WriteTextFile()", "spectrum", "The spectrum fixture can be written", WriteTextFile(File, "DP 100 1.0\nDP 200 3.0\nDP 300 0.5\nDP 400 0.5\nEN\n")) && Passed;

  MCSource S("File spectrum");
  SilenceOutput();
  bool OK = S.SetParticleType(MCSource::c_Gamma);
  OK = S.SetSpectralType(MCSource::c_FileDifferentialFlux) && OK;
  OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint) && OK;
  OK = S.SetEnergy(File) && OK;
  OK = S.SetPosition(0.0, 0.0, 0.0) && OK;
  OK = S.SetFlux(1.0) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("SetEnergy()", "file spectrum", "The source can be prepared", OK) && Passed;

  bool Generated = false;
  vector<double> E = SampleEnergies(S, c_Samples, &Generated);
  Passed = EvaluateTrue("GenerateEnergy()", "file spectrum", "The generation succeeds", Generated) && Passed;

  auto Density = [](double X) {
    if (X <= 200) return 1.0 + 2.0*(X - 100)/100;
    if (X <= 300) return 3.0 - 2.5*(X - 200)/100;
    return 0.5;
  };
  UTSourceStatistics T = Statistics(E);
  Passed = EvaluateNear("GenerateEnergy()", "file spectrum, range, min", "No energy is below the range of the file (shown: the excess below the limit)", max(100.0 - T.m_Min, 0.0), 0.0, 1e-9) && Passed;
  Passed = EvaluateNear("GenerateEnergy()", "file spectrum, range, max", "No energy is above the range of the file (shown: the excess above the limit)", max(T.m_Max - 400.0, 0.0), 0.0, 1e-9) && Passed;
  const double D = KSDistance(E, Density, 100.0, 400.0);
  Passed = EvaluateNear("GenerateEnergy()", "file spectrum, KS " + to_string(D), "The energies follow the linearly interpolated spectrum of the file", D, 0.0, Critical(E.size())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestGaussianPositive()
{
  bool Passed = true;

  // Regression test (A9, MCSource::GenerateEnergy): energies of zero or below are thrown again.
  // Case 1: the cut-off reaches below zero (50 - 3*30 = -40 keV); case 2: a Gaussian centered at zero is a half Gaussian
  struct Case { const char* m_Name; double m_Center; double m_Sigma; double m_CutOff; };
  const vector<Case> Cases = { { "reaching below zero", 50.0, 30.0, 3.0 }, { "centered at zero", 0.0, 100.0, 5.0 } };
  unsigned int Index = 0;
  for (const Case& C: Cases) {
    RandomGuard Guard(2718 + Index++);
    MCSource S("Gaussian positive");
    const MString Name = MString("gaussian, ") + C.m_Name;
    Passed = EvaluateTrue("PrepareNearFieldPointSource()", Name, "The source can be prepared", PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Gaussian, { C.m_Center*keV, C.m_Sigma*keV, C.m_CutOff })) && Passed;
    bool OK = false;
    vector<double> E = SampleEnergies(S, c_Samples, &OK);
    Passed = EvaluateTrue("GenerateEnergy()", Name, "The generation succeeds", OK) && Passed;
    unsigned int NotPositive = 0;
    for (double V: E) {
      if (V <= 0.0) ++NotPositive;
    }
    Passed = Evaluate("GenerateEnergy()", Name + ", positive", "No energy is zero or below", NotPositive, 0U) && Passed;

    // The distribution is the Gaussian restricted to the positive energies and the cut-off window (not normalized: the KS test normalizes):
    const double Center = C.m_Center;
    const double Sigma = C.m_Sigma;
    const double Low = max(0.0, Center - C.m_CutOff*Sigma);
    const double High = Center + C.m_CutOff*Sigma;
    const double D = KSDistance(E, [Center, Sigma](double V) { return exp(-0.5*(V - Center)*(V - Center)/(Sigma*Sigma)); }, Low, High);
    Passed = EvaluateNear("GenerateEnergy()", Name + ", KS " + to_string(D), "The energies follow the Gaussian restricted to positive energies", D, 0.0, KSCritical(E.size())) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestGaussian()
{
  bool Passed = true;
  RandomGuard Guard(31415);

  const double Center = 50.0;
  const double Sigma = 10.0;
  const double CutOff = 2.0;

  MCSource S("Gaussian");
  Passed = EvaluateTrue("PrepareNearFieldPointSource()", "gaussian", "The source can be prepared", PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Gaussian, { Center*keV, Sigma*keV, CutOff })) && Passed;
  bool OK = false;
  vector<double> E = SampleEnergies(S, c_Samples, &OK);
  Passed = EvaluateTrue("GenerateEnergy()", "gaussian", "The generation succeeds", OK) && Passed;
  UTSourceStatistics T = Statistics(E);

  // Mean, in the absence of a cut-off, the sample mean is the center within five standard errors (the cut-off is symmetric):
  Passed = EvaluateNear("GenerateEnergy()", "gaussian, mean", "The mean is the center within five standard errors", T.m_Mean, Center, 5*T.m_Sigma/sqrt(double(T.m_N))) && Passed;

  // The third parameter is the cut-off in sigma (validated to be positive, printed in the description): no sample may be further away
  unsigned int Outside = 0;
  for (double V: E) {
    if (fabs(V - Center) > CutOff*Sigma) ++Outside;
  }
  Passed = Evaluate("GenerateEnergy()", "gaussian, cut-off", "No energy is further than the cut-off (in sigma) from the center", Outside, 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestLoopLimits()
{
  bool Passed = true;
  RandomGuard Guard(99);

  // A cut-off so small that no rejection step can ever succeed: the code has a limit of 10000 loops and claims to report an error:
  MCSource S("Impossible");
  Passed = EvaluateTrue("PrepareNearFieldPointSource()", "cut-off power law with tiny cut-off", "The source can be prepared", PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_CutOffPowerLaw, { 10*keV, 1000*keV, 1.5, 1e-7*keV })) && Passed;
  bool OK = true;
  SampleEnergies(S, 1, &OK);
  Passed = EvaluateFalse("GenerateEnergy()", "cut-off power law with tiny cut-off", "The generation reports a failure when the loop limit is exhausted", OK) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceSpectra::TestReproducibility()
{
  bool Passed = true;

  vector<double> First, Second;
  for (vector<double>* Result: { &First, &Second }) {
    RandomGuard Guard(123);
    MCSource S("Reproducible");
    PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_PowerLaw, { 10*keV, 1000*keV, 1.7 });
    *Result = SampleEnergies(S, 100);
  }
  Passed = EvaluateTrue("GenerateEnergy()", "same seed", "The same seed gives the same energies", First == Second && First.size() == 100) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceSpectra Test;
  return Test.Run() == true ? 0 : 1;
}
