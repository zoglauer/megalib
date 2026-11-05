/*
 * UTParameterFileSources.cc
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
#include "UTParameterFileShared.h"

// Standard lib:
#include <cmath>
using namespace std;


//! Unit test class for the source keywords (spectra, flux, polarization, ...) of MCParameterFile
class UTParameterFileSources : public UTParameterFileTest
{
public:
  //! Default constructor
  UTParameterFileSources() : UTParameterFileTest("UTParameterFileSources") {}
  //! Default destructor
  virtual ~UTParameterFileSources() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One spectrum: its keyword line, the spectral type and the expected energy parameters (keV where applicable)
  struct SpectrumCase {
    MString Line;
    int Type;
    vector<double> Parameters;
  };

  //! Test all spectra with valid input
  bool TestSpectra();
  //! Test the rejection of invalid spectra
  bool TestSpectraErrors();
  //! Test the spectrum from a file
  bool TestSpectrumFile();
  //! Test the deprecated energy keywords
  bool TestDeprecatedEnergy();
  //! Test flux and intensity
  bool TestFlux();
  //! Test the particle type
  bool TestParticleType();
  //! Test the polarization keyword
  bool TestPolarization();
  //! Test successors
  bool TestSuccessors();
  //! Test the light curve
  bool TestLightCurve();
  //! Test the source orientation
  bool TestOrientation();
  //! Test the far field transmission probability and the Earth occultation
  bool TestFarFieldExtras();
  //! Test unknown keywords of a source
  bool TestUnknownKeywords();
};


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::Run()
{
  bool Passed = true;

  if (PrepareFiles() == false) {
    Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", false) && Passed;
    Summarize();
    return Passed;
  }

  Passed = TestSpectra() && Passed;
  Passed = TestSpectraErrors() && Passed;
  Passed = TestSpectrumFile() && Passed;
  Passed = TestDeprecatedEnergy() && Passed;
  Passed = TestFlux() && Passed;
  Passed = TestParticleType() && Passed;
  Passed = TestPolarization() && Passed;
  Passed = TestSuccessors() && Passed;
  Passed = TestLightCurve() && Passed;
  Passed = TestOrientation() && Passed;
  Passed = TestFarFieldExtras() && Passed;
  Passed = TestUnknownKeywords() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestSpectra()
{
  bool Passed = true;

  // Energies are given in keV and stored in the Geant4 unit (MeV), the indices are dimensionless
  vector<SpectrumCase> Cases = {
    { "Mono 662", MCSource::c_Monoenergetic, { 662*keV } },
    { "m 662", MCSource::c_Monoenergetic, { 662*keV } },
    { "Linear 100 500", MCSource::c_Linear, { 100*keV, 500*keV } },
    { "l 100 500", MCSource::c_Linear, { 100*keV, 500*keV } },
    { "PowerLaw 100 1000 2", MCSource::c_PowerLaw, { 100*keV, 1000*keV, 2 } },
    { "pl 100 1000 2", MCSource::c_PowerLaw, { 100*keV, 1000*keV, 2 } },
    { "BrokenPowerLaw 100 1000 300 1.5 2.5", MCSource::c_BrokenPowerLaw, { 100*keV, 1000*keV, 300*keV, 1.5, 2.5 } },
    { "bpl 100 1000 300 1.5 2.5", MCSource::c_BrokenPowerLaw, { 100*keV, 1000*keV, 300*keV, 1.5, 2.5 } },
    { "CutOffPowerLaw 100 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 100*keV, 1000*keV, 1.5, 500*keV } },
    { "cpl 100 1000 1.5 500", MCSource::c_CutOffPowerLaw, { 100*keV, 1000*keV, 1.5, 500*keV } },
    { "Comptonized 100 1000 -1 300", MCSource::c_Comptonized, { 100*keV, 1000*keV, -1, 300*keV } },
    { "Comptonization 100 1000 -1 300", MCSource::c_Comptonized, { 100*keV, 1000*keV, -1, 300*keV } },
    { "comp 100 1000 -1 300", MCSource::c_Comptonized, { 100*keV, 1000*keV, -1, 300*keV } },
    { "Gaussian 511 10 3", MCSource::c_Gaussian, { 511*keV, 10*keV, 3 } },
    { "g 511 10 3", MCSource::c_Gaussian, { 511*keV, 10*keV, 3 } },
    { "ThermalBremsstrahlung 50 500 100", MCSource::c_ThermalBremsstrahlung, { 50*keV, 500*keV, 100*keV } },
    { "tb 50 500 100", MCSource::c_ThermalBremsstrahlung, { 50*keV, 500*keV, 100*keV } },
    { "BlackBody 50 500 100", MCSource::c_BlackBody, { 50*keV, 500*keV, 100*keV } },
    { "bb 50 500 100", MCSource::c_BlackBody, { 50*keV, 500*keV, 100*keV } },
    { "BandFunction 50 500 -1 -2.5 200", MCSource::c_BandFunction, { 50*keV, 500*keV, -1, -2.5, 200*keV } },
    { "bf 50 500 -1 -2.5 200", MCSource::c_BandFunction, { 50*keV, 500*keV, -1, -2.5, 200*keV } },
    { "Band 50 500 -1 -2.5 200", MCSource::c_BandFunction, { 50*keV, 500*keV, -1, -2.5, 200*keV } },
    { "Band 50 500 1 -2.5 200", MCSource::c_BandFunction, { 50*keV, 500*keV, 1, -2.5, 200*keV } },
    { "Band 50 500 -1 -2.5 10", MCSource::c_BandFunction, { 50*keV, 500*keV, -1, -2.5, 10*keV } },
    { "Band 50 500 -1 -2.5 1000", MCSource::c_BandFunction, { 50*keV, 500*keV, -1, -2.5, 1000*keV } },
    { "MONO 100", MCSource::c_Monoenergetic, { 100*keV } }
  };

  for (const SpectrumCase& C: Cases) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", C.Line, "The spectrum is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum ") + C.Line + "\n"))) && Passed;
    MCSource* S = GetS(P);
    if (S == nullptr) continue;
    Passed = Evaluate("Spectrum", C.Line, "The spectral type is set", S->GetSpectralType(), C.Type) && Passed;
    for (unsigned int p = 0; p < C.Parameters.size(); ++p) {
      Passed = EvaluateNear("Spectrum", C.Line + " parameter " + (p+1), "The parameter is stored in its unit", S->GetEnergyParameter(p+1), C.Parameters[p], 1e-9*max(1.0, fabs(C.Parameters[p]))) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestSpectraErrors()
{
  bool Passed = true;

  vector<MString> Bad = {
    "Spectrum", "Spectrum Unknown 1",
    "Spectrum Mono", "Spectrum Mono 0", "Spectrum Mono -5", "Spectrum Mono 1 2", "Spectrum Mono abc",
    "Spectrum Linear 100", "Spectrum Linear 500 100", "Spectrum Linear 100 100", "Spectrum Linear 0 100", "Spectrum Linear 100 200 300",
    "Spectrum PowerLaw 100 1000", "Spectrum PowerLaw 1000 100 2", "Spectrum PowerLaw 0 100 2", "Spectrum PowerLaw 100 1000 2 3",
    "Spectrum BrokenPowerLaw 100 1000 300 1.5", "Spectrum BrokenPowerLaw 100 1000 50 1.5 2.5", "Spectrum BrokenPowerLaw 100 1000 2000 1.5 2.5",
    "Spectrum BrokenPowerLaw 100 1000 300 -1.5 2.5", "Spectrum BrokenPowerLaw 100 1000 300 1.5 -2.5",
    "Spectrum CutOffPowerLaw 100 1000 1.5", "Spectrum CutOffPowerLaw 100 1000 1.5 0", "Spectrum CutOffPowerLaw 1000 100 1.5 500",
    "Spectrum Comptonized 100 1000 -1", "Spectrum Comptonized 100 1000 -1 0", "Spectrum Comptonized 1000 100 -1 300",
    "Spectrum Gaussian 511 10", "Spectrum Gaussian -1 10 3", "Spectrum Gaussian 511 0 3", "Spectrum Gaussian 511 10 0",
    "Spectrum ThermalBremsstrahlung 50 500", "Spectrum ThermalBremsstrahlung 500 50 100", "Spectrum ThermalBremsstrahlung 50 500 0",
    "Spectrum BlackBody 50 500", "Spectrum BlackBody 500 50 100", "Spectrum BlackBody 50 500 -1",
    "Spectrum BandFunction 50 500 -1 -2.5", "Spectrum BandFunction 500 50 -1 -2.5 200", "Spectrum BandFunction 50 500 -2.5 -1 200",
    "Spectrum BandFunction 50 500 -1 2.5 200", "Spectrum BandFunction 50 500 -1 -2.5 0", "Spectrum BandFunction 50 500 -1 -2.5 -10",
     "Spectrum NormalizedEnergyBeamFluxFunction a b",
    "Spectrum File", "Spectrum File " + GetDirectory() + "/Missing.dat"
  };
  for (const MString& B: Bad) {
    Passed = ExpectRejected("Open()", B, "The invalid spectrum is rejected", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.") + B + "\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestSpectrumFile()
{
  bool Passed = true;

  const MString Good = WriteSource("Spectrum.dat", "DP 10 1.0\nDP 100 0.5\nDP 1000 0.1\n");
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "file spectrum", "A spectrum file is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum File ") + Good + "\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Spectrum File", "file", "The spectral type is file differential flux", GetS(P)->GetSpectralType(), MCSource::c_FileDifferentialFlux) && Passed;
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "filediffflux", "The long keyword is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum FileDiffFlux ") + Good + "\n"))) && Passed;
  }

  // Negative values and a single entry are invalid:
  const MString Negative = WriteSource("Negative.dat", "DP 10 1.0\nDP 100 -0.5\n");
  Passed = ExpectRejected("Open()", "negative flux file", "A spectrum file with a negative flux is rejected", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum File ") + Negative + "\n")) && Passed;
  const MString Single = WriteSource("Single.dat", "DP 10 1.0\n");
  Passed = ExpectRejected("Open()", "single entry file", "A spectrum file with one entry is rejected", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum File ") + Single + "\n")) && Passed;

  // The real example file:
  const MString Directory = GetExampleDirectory();
  if (Directory.IsEmpty() == false) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "example spectrum", "The example spectrum file is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum File ") + Directory + "/CosmicPhotonSpectrum.dat\n"))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestDeprecatedEnergy()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "SpectralType 1 Energy", "The deprecated spectral type and energy are accepted", Open(P, OneSource("S.Beam FarFieldPointSource 0 0\nS.SpectralType 1\nS.Energy 662\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("SpectralType", "1", "The spectral type is monoenergetic", GetS(P)->GetSpectralType(), MCSource::c_Monoenergetic) && Passed;
      Passed = EvaluateNear("Energy", "662", "The energy is given in keV", GetS(P)->GetEnergyParameter(1), 662*keV, 1e-9) && Passed;
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "SpectralType 3 Energy", "The deprecated power law is accepted", Open(P, OneSource("S.Beam FarFieldPointSource 0 0\nS.SpectralType 3\nS.Energy 100 1000 2\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("SpectralType", "3", "The spectral type is power law", GetS(P)->GetSpectralType(), MCSource::c_PowerLaw) && Passed;
      Passed = EvaluateNear("Energy", "100 1000 2", "The maximum energy is stored in keV", GetS(P)->GetEnergyParameter(2), 1000*keV, 1e-9) && Passed;
    }
  }
  Passed = ExpectRejected("Open()", "SpectralType 999", "An unknown spectral type is rejected", OneSource("S.SpectralType 999\n")) && Passed;
  Passed = ExpectRejected("Open()", "SpectralType without value", "The spectral type needs a value", OneSource("S.SpectralType\n")) && Passed;
  Passed = ExpectRejected("Open()", "Energy without spectral type", "An energy without a spectral type is rejected", OneSource("S.Energy 100\n")) && Passed;
  Passed = ExpectRejected("Open()", "Energy wrong token count", "An energy with the wrong number of values is rejected", OneSource("S.SpectralType 1\nS.Energy 100 200\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestFlux()
{
  bool Passed = true;

  // Near field: the flux is given per second:
  for (const char* Keyword: { "Flux", "Intensity" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The flux keyword is accepted", Open(P, OneSource(MString("S.Beam PointSource 0 0 0\nS.Spectrum Mono 100\nS.") + Keyword + " 250\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = EvaluateNear(Keyword, "near field 250", "A near field flux is given per second", GetS(P)->GetFlux()*s, 250.0, 1e-6) && Passed;
    }
  }

  // Far field: the flux is given per cm^2 and second and multiplied by the start area (pi r^2 for a sphere):
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "far field flux", "A far field flux with start area is accepted", Open(P, Header() + "StartArea Sphere 0 0 0 0 0 0 10\nRun R\nR.Source S\nS.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.Flux 2\n")) && Passed;
    if (GetS(P) != nullptr) {
      Passed = EvaluateNear("Flux", "far field 2", "The far field flux is per cm2 and multiplied with the start area", GetS(P)->GetFlux()*s, 2.0*M_PI*100.0, 1e-6) && Passed;
    }
  }

  // The flux line may come before the beam line in the file:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "flux before beam", "The flux before the beam is accepted", Open(P, OneSource("S.Flux 250\nS.Spectrum Mono 100\nS.Beam PointSource 0 0 0\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = EvaluateNear("Flux", "before beam", "The flux is the same independent of the line order", GetS(P)->GetFlux()*s, 250.0, 1e-6) && Passed;
    }
  }

  vector<MString> Bad = { "Flux", "Flux 0", "Flux -1", "Flux 1 2", "Flux abc", "Intensity 0", "TotalEnergyFlux 3" };
  for (const MString& B: Bad) {
    Passed = ExpectRejected("Open()", B, "The invalid value is rejected", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.") + B + "\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestParticleType()
{
  bool Passed = true;

  for (const char* Keyword: { "ParticleType", "Particle", "ParticleID" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The particle keyword is accepted", Open(P, OneSource(MString("S.") + Keyword + " 2\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate(Keyword, "2", "The particle type is stored", GetS(P)->GetParticleType(), MCSource::c_Positron) && Passed;
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "ParticleType 1", "Gamma is accepted", Open(P, OneSource("S.ParticleType 1\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = Evaluate("ParticleType", "1", "Gamma is stored", GetS(P)->GetParticleType(), MCSource::c_Gamma) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "ParticleType 1000020040", "An ion is accepted", Open(P, OneSource("S.ParticleType 1000020040\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = Evaluate("ParticleType", "ion", "The ion ID is stored", GetS(P)->GetParticleType(), 1000020040) && Passed;
  }

  Passed = ExpectRejected("Open()", "ParticleType 0", "Particle type zero is rejected", OneSource("S.ParticleType 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "ParticleType -3", "A negative particle type is rejected", OneSource("S.ParticleType -3\n")) && Passed;
  Passed = ExpectRejected("Open()", "ParticleType 999", "An unknown particle type is rejected", OneSource("S.ParticleType 999\n")) && Passed;
  Passed = ExpectRejected("Open()", "ParticleType without value", "The particle type needs a value", OneSource("S.ParticleType\n")) && Passed;
  Passed = ExpectRejected("Open()", "ParticleType with two values", "The particle type takes one value", OneSource("S.ParticleType 1 2\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestPolarization()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "none", "No polarization is accepted", Open(P, OneSource("S.Polarization none\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = Evaluate("Polarization", "none", "The type is none", GetS(P)->GetPolarizationType(), MCSource::c_PolarizationNone) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "random", "Random polarization is accepted", Open(P, OneSource("S.Polarization random\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = Evaluate("Polarization", "random", "The type is random", GetS(P)->GetPolarizationType(), MCSource::c_PolarizationRandom) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "absolute", "Absolute polarization is accepted", Open(P, OneSource("S.Polarization absolute 0.5 1 0 0\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Polarization", "absolute", "The type is absolute", GetS(P)->GetPolarizationType(), MCSource::c_PolarizationAbsolute) && Passed;
      Passed = EvaluateNear("Polarization", "absolute 0.5", "The degree is stored", GetS(P)->GetPolarizationDegree(), 0.5, 1e-12) && Passed;
    }
  }
  // An absolute vector must be usable: not zero, and not parallel to the fixed direction of the beam (here along z):
  for (const char* Vector: { "0 0 0", "0 0 1", "0 0 -2.5" }) {
    Passed = ExpectRejected("Open()", MString("absolute ") + Vector, "A zero vector or a vector parallel to the fixed beam direction is rejected",
                            OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.Polarization absolute 1 ") + Vector + "\n")) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "absolute skew", "A vector which is not orthogonal to the beam direction is accepted (its projection is used)",
                          Open(P, OneSource("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.Polarization absolute 1 1 0 1\n"))) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "degree above 1", "A degree above one is accepted", Open(P, OneSource("S.Polarization absolute 1.5 1 0 0\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = EvaluateNear("Polarization", "degree 1.5", "A degree above 1 is clipped to 1", GetS(P)->GetPolarizationDegree(), 1.0, 1e-12) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "negative degree", "A negative degree is accepted", Open(P, OneSource("S.Polarization absolute -0.5 1 0 0\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = EvaluateNear("Polarization", "degree -0.5", "A negative degree is clipped to 0", GetS(P)->GetPolarizationDegree(), 0.0, 1e-12) && Passed;
  }
  {
    struct Entry { const char* Keyword; int Type; };
    for (const Entry& E: { Entry{ "galactic", MCSource::c_PolarizationGalactic }, Entry{ "relativex", MCSource::c_PolarizationRelativeX },
                           Entry{ "RelativeY", MCSource::c_PolarizationRelativeY }, Entry{ "relativez", MCSource::c_PolarizationRelativeZ } }) {
      UTParameterFileAccess P;
      Passed = EvaluateTrue("Open()", E.Keyword, "The polarization is accepted", Open(P, OneSource(MString("S.Polarization ") + E.Keyword + " 0.7 30\n"))) && Passed;
      if (GetS(P) != nullptr) {
        Passed = Evaluate("Polarization", E.Keyword, "The type is set", GetS(P)->GetPolarizationType(), E.Type) && Passed;
        Passed = EvaluateNear("Polarization", E.Keyword, "The degree is stored", GetS(P)->GetPolarizationDegree(), 0.7, 1e-12) && Passed;
      }
    }
  }
  {
    // Deprecated format: true degree is the LAST value:
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "old format", "The old format is accepted", Open(P, OneSource("S.Polarization true 1 0 0 0.25\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("Polarization", "true", "The old format means absolute", GetS(P)->GetPolarizationType(), MCSource::c_PolarizationAbsolute) && Passed;
      Passed = EvaluateNear("Polarization", "true", "The degree is the last value", GetS(P)->GetPolarizationDegree(), 0.25, 1e-12) && Passed;
    }
  }

  vector<MString> Bad = { "Polarization", "Polarization unknown", "Polarization absolute 0.5 1 0", "Polarization absolute 0.5 1 0 0 0", "Polarization galactic 0.5", "Polarization relativex 0.5",
                          "Polarization relativey 0.5 10 20", "Polarization relativez", "Polarization true 1 0 0", "Polarization false 1 0 0 0.5" };
  for (const MString& B: Bad) {
    Passed = ExpectRejected("Open()", B, "The invalid polarization is rejected", OneSource(MString("S.") + B + "\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestSuccessors()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "chain", "A successor chain is accepted", Open(P, Header() + "Run R\nR.Source A\nR.Source B\nA.Successor B\nB.IsSuccessor true\n")) && Passed;
  if (P.GetSource("A") != nullptr && P.GetSource("B") != nullptr) {
    Passed = Evaluate("Successor", "A", "The successor name is stored", P.GetSource("A")->GetSuccessor(), MString("B")) && Passed;
    Passed = EvaluateFalse("IsSuccessor", "A", "A is no successor", P.GetSource("A")->IsSuccessor()) && Passed;
    Passed = EvaluateTrue("IsSuccessor", "B", "B is a successor", P.GetSource("B")->IsSuccessor()) && Passed;
  }

  Passed = ExpectRejected("Open()", "Successor without value", "The successor needs a value", OneSource("S.Successor\n")) && Passed;
  Passed = ExpectRejected("Open()", "Successor with two values", "The successor takes one value", OneSource("S.Successor A B\n")) && Passed;
  Passed = ExpectRejected("Open()", "IsSuccessor without value", "IsSuccessor needs a value", OneSource("S.IsSuccessor\n")) && Passed;


  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestLightCurve()
{
  bool Passed = true;

  // A flat light curve is the default, selecting it explicitly must be possible with the documented syntax:
  for (const char* Line: { "LightCurve Flat", "LightCurve None", "LightCurve flat" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Line, "A flat light curve is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.") + Line + "\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("LightCurve", Line, "The light curve type is flat", GetS(P)->GetLightCurveType(), MCSource::c_LightCurveFlat) && Passed;
    }
  }

  // A light curve file:
  const MString Curve = WriteSource("Curve.dat", "DP 0.0 1.0\nDP 1.0 2.0\nDP 2.0 1.0\n");
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "LightCurve File", "A light curve file is accepted", Open(P, OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.LightCurve File false ") + Curve + "\n"))) && Passed;
    if (GetS(P) != nullptr) {
      Passed = Evaluate("LightCurve", "File", "The light curve type is file", GetS(P)->GetLightCurveType(), MCSource::c_LightCurveFile) && Passed;
    }
  }

  Passed = ExpectRejected("Open()", "LightCurve without type", "The light curve needs a type", OneSource("S.LightCurve\n")) && Passed;
  Passed = ExpectRejected("Open()", "LightCurve unknown", "An unknown light curve type is rejected", OneSource("S.LightCurve Sine 1 2\n")) && Passed;
  Passed = ExpectRejected("Open()", "LightCurve File without name", "A light curve file needs a file name", OneSource("S.LightCurve File false\n")) && Passed;
  Passed = ExpectRejected("Open()", "LightCurve File missing", "A missing light curve file is rejected", OneSource(MString("S.LightCurve File false ") + GetDirectory() + "/Missing.dat\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestOrientation()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Local Fixed", "A local fixed orientation is accepted", Open(P, OneSource("S.Orientation Local Fixed\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = EvaluateFalse("Orientation", "Local Fixed", "The default local orientation is not oriented", GetS(P)->GetOrientation().IsOriented()) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Galactic Fixed", "A galactic fixed orientation is accepted", Open(P, OneSource("S.Orientation Galactic Fixed 10 20\n"))) && Passed;
    if (GetS(P) != nullptr) Passed = EvaluateTrue("Orientation", "Galactic Fixed", "The orientation is oriented", GetS(P)->GetOrientation().IsOriented()) && Passed;
  }
  Passed = ExpectRejected("Open()", "Orientation too short", "An orientation needs a coordinate system and a type", OneSource("S.Orientation Local\n")) && Passed;
  Passed = ExpectRejected("Open()", "Orientation unknown system", "An unknown coordinate system is rejected", OneSource("S.Orientation Equatorial Fixed\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestFarFieldExtras()
{
  bool Passed = true;

  const MString Transmission = WriteSource("Transmission.dat", "IP LIN\nXA 0.0 120.0 121.0 180.0\nYA 0.0 1.0E+18\nAP 0 0 1.0\nAP 1 0 1.0\nAP 2 0 0.0\nAP 3 0 0.0\nAP 0 1 1.0\nAP 1 1 1.0\nAP 2 1 0.0\nAP 3 1 0.0\n");
  Passed = ExpectAccepted("Open()", "transmission", "A far field transmission probability is accepted", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.FarFieldTransmissionProbability ") + Transmission + "\n")) && Passed;
  Passed = ExpectRejected("Open()", "transmission near field", "A transmission probability for a near field source is rejected", OneSource(MString("S.Beam PointSource 0 0 0\nS.Spectrum Mono 100\nS.FarFieldTransmissionProbability ") + Transmission + "\n")) && Passed;
  Passed = ExpectRejected("Open()", "transmission missing file", "A missing transmission file is rejected", OneSource(MString("S.Beam FarFieldPointSource 0 0\nS.FarFieldTransmissionProbability ") + GetDirectory() + "/Missing.dat\n")) && Passed;
  Passed = ExpectRejected("Open()", "transmission without value", "The transmission probability needs a file", OneSource("S.Beam FarFieldPointSource 0 0\nS.FarFieldTransmissionProbability\n")) && Passed;

  Passed = ExpectAccepted("Open()", "EarthOccultation", "The Earth occultation with theta is accepted", OneSource("S.Beam FarFieldPointSource 0 0\nS.EarthOccultation false 65\n")) && Passed;
  Passed = ExpectAccepted("Open()", "EarthOccultation", "The Earth occultation without theta is accepted", OneSource("S.Beam FarFieldPointSource 0 0\nS.EarthOccultation true\n")) && Passed;
  Passed = ExpectRejected("Open()", "EarthOccultation near field", "Earth occultation for a near field source is rejected", OneSource("S.Beam PointSource 0 0 0\nS.EarthOccultation true\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileSources::TestUnknownKeywords()
{
  bool Passed = true;

  Passed = ExpectRejected("Open()", "unknown source keyword", "An unknown keyword of a source is rejected", OneSource("S.Colour red\n")) && Passed;
  Passed = ExpectRejected("Open()", "misspelled keyword", "A misspelled keyword of a source is rejected", OneSource("S.Spectrun Mono 100\n")) && Passed;

  // Keywords are case independent:
  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "lower case", "Keywords in lower case are accepted", Open(P, OneSource("s.beam FarFieldPointSource 0 0\ns.spectrum mono 100\ns.flux 1\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParameterFileSources Test;
  return Test.Run() == true ? 0 : 1;
}
