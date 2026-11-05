/*
 * UTSourceGeneration.cc
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
#include "MCOrientation.hh"
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MTokenizer.h"
#include "MUnitTest.h"

// Geant4:
#include "G4Ions.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <utility>
#include <vector>
using namespace std;


//! Unit test class for the time handling, the particle generation, the polarization, the event lists,
//! and the isotope sources of MCSource
class UTSourceGeneration : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceGeneration() : UTSourceTestBase("UTSourceGeneration") {}
  //! Default destructor
  virtual ~UTSourceGeneration() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Number of samples
  static const unsigned int c_Samples = 20000;
  //! Configure a near field point source at the origin emitting 511 keV gammas with the given flux
  bool ConfigurePoint(MCSource& S, double Flux)
  {
    return PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Monoenergetic, { 511*keV }) && SetFluxSilent(S, Flux);
  }
  //! Set the flux silently
  bool SetFluxSilent(MCSource& S, double Flux)
  {
    SilenceOutput();
    bool OK = S.SetFlux(Flux);
    RestoreOutput();
    return OK;
  }

  bool TestEmissionTimes();
  bool TestLightCurve();
  bool TestLightCurveOrder();
  bool TestLightCurveInvalid();
  bool TestIsotopeCount();
  bool TestEventList();
  bool TestEventListFile();
  bool TestEventListOrder();
  bool TestEventListCap();
  bool TestEventListParticles();
  bool TestSkippedEvents();
  bool TestGenerateParticles();
  bool TestPolarization();
  bool TestAbsolutePolarization();
  bool TestTransmission();
  bool TestTransmissionVarying();
  bool TestOrientationCombination();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("InitializeEnvironment()", "generation", "The Geant4 environment can be created", InitializeEnvironment()) && Passed;

  Passed = TestEmissionTimes() && Passed;
  Passed = TestLightCurve() && Passed;
  Passed = TestLightCurveOrder() && Passed;
  Passed = TestLightCurveInvalid() && Passed;
  Passed = TestIsotopeCount() && Passed;
  Passed = TestEventList() && Passed;
  Passed = TestEventListFile() && Passed;
  Passed = TestEventListOrder() && Passed;
  Passed = TestEventListCap() && Passed;
  Passed = TestEventListParticles() && Passed;
  Passed = TestSkippedEvents() && Passed;
  Passed = TestGenerateParticles() && Passed;
  Passed = TestPolarization() && Passed;
  Passed = TestAbsolutePolarization() && Passed;
  Passed = TestTransmission() && Passed;
  Passed = TestTransmissionVarying() && Passed;
  Passed = TestOrientationCombination() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEmissionTimes()
{
  bool Passed = true;
  RandomGuard Guard(5);

  const double Flux = 3.0/second;
  MCSource S("Times");
  Passed = EvaluateTrue("ConfigurePoint()", "flat", "The source can be configured", ConfigurePoint(S, Flux)) && Passed;

  vector<double> Intervals;
  double Time = 0.0;
  bool Monotonic = true;
  bool OK = true;
  SilenceOutput();
  for (unsigned int i = 0; i < 20000; ++i) {
    OK = S.CalculateNextEmission(Time, 1.0) && OK;
    if (S.GetNextEmission() < Time) Monotonic = false;
    Intervals.push_back((S.GetNextEmission() - Time)/second);
    Time = S.GetNextEmission();
  }
  RestoreOutput();
  Passed = EvaluateTrue("CalculateNextEmission()", "flat", "The calculation succeeds", OK) && Passed;
  Passed = EvaluateTrue("CalculateNextEmission()", "flat, order", "The emission times never go backwards", Monotonic) && Passed;
  Passed = EvaluateTrue("IsActive()", "flat", "The source stays active", S.IsActive()) && Passed;
  const double Rate = Flux*second;
  const double D = KSDistance(Intervals, [Rate](double T) { return Rate*exp(-Rate*T); }, 0.0, 15.0/Rate);
  Passed = EvaluateNear("CalculateNextEmission()", MString("flat, KS ") + D, "The intervals are exponentially distributed with the mean 1/flux", D, 0.0, KSCritical(Intervals.size())) && Passed;
  Passed = EvaluateNear("CalculateNextEmission()", "flat, rate", "The mean rate is the flux", 20000.0/(Time/second), Rate, 0.03*Rate) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestLightCurveOrder()
{
  bool Passed = true;
  RandomGuard Guard(8);

  // Regression test (A1, MCSource::SetFlux/SetLightCurve): the normalization of the light curve used to be done with the flux known at that time,
  // so setting the flux before the light curve doubled/quadrupled the rate
  // The emission rate must not depend on the order of the keywords: the light curve is normalized so that its mean is the flux,
  // no matter if the flux is set before or after the light curve (a parameter file may list them in any order)
  const MString File = GetTemporaryFileName("LightCurveOrder.dat");
  Passed = EvaluateTrue("WriteTextFile()", "light curve", "The light curve fixture can be written", WriteTextFile(File, "DP 0 1\nDP 10 4\nDP 20 1\nDP 30 1\nEN\n")) && Passed;

  const double Flux = 2.0/second;
  for (int FluxFirst = 0; FluxFirst < 2; ++FluxFirst) {
    MCSource S("Light curve order");
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint) && OK;
    OK = S.SetPosition(0.0, 0.0, 0.0) && OK;
    if (FluxFirst == 1) OK = S.SetFlux(Flux) && OK;
    OK = S.SetLightCurveType(MCSource::c_LightCurveFile) && OK;
    OK = S.SetLightCurve(File, true) && OK;
    if (FluxFirst == 0) OK = S.SetFlux(Flux) && OK;
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    MString Input = (FluxFirst == 1) ? "flux before the light curve" : "flux after the light curve";
    Passed = EvaluateTrue("SetLightCurve()", Input, "The source can be configured", OK) && Passed;

    double Time = 0.0;
    SilenceOutput();
    for (unsigned int i = 0; i < 20000; ++i) {
      S.CalculateNextEmission(Time, 1.0);
      Time = S.GetNextEmission();
    }
    RestoreOutput();
    // 20000 events: the statistical uncertainty of the mean rate is 0.7%, 3% is more than four standard deviations
    Passed = EvaluateNear("CalculateNextEmission()", Input, "The mean rate over the light curve is the flux", 20000.0/(Time/second), Flux*second, 0.03*Flux*second) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestLightCurve()
{
  bool Passed = true;
  RandomGuard Guard(6);

  const MString File = GetTemporaryFileName("LightCurve.dat");
  Passed = EvaluateTrue("WriteTextFile()", "light curve", "The light curve fixture can be written", WriteTextFile(File, "DP 0 1\nDP 10 4\nDP 20 1\nDP 30 1\nEN\n")) && Passed;

  const double Flux = 2.0/second;
  MCSource S("Light curve");
  SilenceOutput();
  bool OK = S.SetParticleType(MCSource::c_Gamma);
  OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
  OK = S.SetEnergy(511*keV) && OK;
  OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint) && OK;
  OK = S.SetPosition(0.0, 0.0, 0.0) && OK;
  OK = S.SetLightCurveType(MCSource::c_LightCurveFile) && OK;
  OK = S.SetLightCurve(File, true) && OK;
  OK = S.SetFlux(Flux) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("SetLightCurve()", "repeating", "The source can be configured (light curve before the flux like in the parameter file)", OK) && Passed;
  Passed = EvaluateTrue("IsFluxVariable()", "repeating", "The flux is variable", S.IsFluxVariable()) && Passed;

  vector<double> Phases;
  double Time = 0.0;
  bool Monotonic = true;
  SilenceOutput();
  for (unsigned int i = 0; i < 20000; ++i) {
    S.CalculateNextEmission(Time, 1.0);
    if (S.GetNextEmission() < Time) Monotonic = false;
    Time = S.GetNextEmission();
    Phases.push_back(fmod(Time/second, 30.0));
  }
  RestoreOutput();
  Passed = EvaluateTrue("CalculateNextEmission()", "light curve, order", "The emission times never go backwards", Monotonic) && Passed;
  Passed = EvaluateTrue("IsActive()", "repeating", "A repeating light curve never ends", S.IsActive()) && Passed;
  // The mean flux is the flux of the source, the shape follows the light curve:
  Passed = EvaluateNear("CalculateNextEmission()", "light curve, mean rate", "The mean rate over the light curve is the flux", 20000.0/(Time/second), Flux*second, 0.03*Flux*second) && Passed;
  auto Curve = [](double T) { return (T <= 10) ? 1.0 + 0.3*T : (T <= 20) ? 4.0 - 0.3*(T - 10) : 1.0; };
  const double D = KSDistance(Phases, Curve, 0.0, 30.0);
  Passed = EvaluateNear("CalculateNextEmission()", MString("light curve, KS ") + D, "The emission times follow the light curve", D, 0.0, KSCritical(Phases.size())) && Passed;

  // A light curve which does not repeat ends:
  MCSource E("Light curve once");
  SilenceOutput();
  E.SetParticleType(MCSource::c_Gamma);
  E.SetSpectralType(MCSource::c_Monoenergetic);
  E.SetEnergy(511*keV);
  E.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint);
  E.SetPosition(0.0, 0.0, 0.0);
  E.SetLightCurveType(MCSource::c_LightCurveFile);
  E.SetLightCurve(File, false);
  E.SetFlux(Flux);
  E.GenerateParticleDefinition();
  unsigned int Emissions = 0;
  Time = 0.0;
  while (E.IsActive() == true && Emissions < 100000) {
    E.CalculateNextEmission(Time, 1.0);
    Time = E.GetNextEmission();
    ++Emissions;
  }
  RestoreOutput();
  Passed = EvaluateFalse("IsActive()", "non-repeating", "A light curve which does not repeat ends the source", E.IsActive()) && Passed;
  // Flux 2/s over 30 s give 60 emissions on average:
  Passed = EvaluateNear("CalculateNextEmission()", "non-repeating, count", "The number of emissions over the light curve is flux times duration (60 +- 5 sigma)", double(Emissions), 60.0, 5*sqrt(60.0) + 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestLightCurveInvalid()
{
  bool Passed = true;

  // Regression test (A3, MCSource::UpgradeFlux): the result of UpgradeLightCurve() used to be ignored, thus SetFlux() reported success
  // although the light curve cannot be normalized (its integral is infinite)
  const MString File = GetTemporaryFileName("LightCurveInfinite.dat");
  Passed = EvaluateTrue("WriteTextFile()", "infinite light curve", "The light curve fixture can be written", WriteTextFile(File, "DP 0 1e400\nDP 10 1e400\nEN\n")) && Passed;

  MCSource S("Infinite light curve");
  SilenceOutput();
  S.SetParticleType(MCSource::c_Gamma);
  S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint);
  S.SetPosition(0.0, 0.0, 0.0);
  S.SetLightCurveType(MCSource::c_LightCurveFile);
  S.SetLightCurve(File, true);
  bool FluxResult = S.SetFlux(2.0/second);
  RestoreOutput();
  Passed = EvaluateFalse("SetFlux()", "infinite light curve", "A flux which cannot be applied to the light curve is rejected", FluxResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEventListOrder()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  if (Gamma == nullptr) return false;

  // Regression test (A8, MCSource::AddToEventList): events with identical times used to be inserted before the existing ones (lower_bound),
  // i.e. they were generated in reverse order of adding. Events at the same time are separate entries and must keep their order
  MCSource S("Event list order");
  S.SetEventList();
  SilenceOutput();
  bool Added = true;
  for (unsigned int i = 0; i < 3; ++i) {
    Added = S.AddToEventList(100.0*(i + 1)*keV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 1.0*second, Gamma, "Cube") && Added;
  }
  RestoreOutput();
  Passed = EvaluateTrue("AddToEventList()", "same time", "Events with the same time can be added", Added) && Passed;
  for (unsigned int i = 0; i < 3; ++i) {
    SilenceOutput();
    S.CalculateNextEmission(0.0, 1.0);
    bool Generated = S.GenerateParticles(m_Gun);
    RestoreOutput();
    Passed = EvaluateTrue("GenerateParticles()", MString("same time ") + i, "The event is generated", Generated) && Passed;
    Passed = EvaluateNear("GenerateParticles()", MString("same time ") + i, "Events with the same time are generated in the order of adding", GetGunEnergy()/keV, 100.0*(i + 1), 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEventListCap()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  if (Gamma == nullptr) return false;

  // Regression test (A2, MCSource::AddToEventList): at the cap the last event is removed - it used to delete the first one (dangling pointer, leak)
  MCSource S("Event list cap");
  S.SetEventList();
  Passed = Evaluate("GetMaxEventListSize()", "default", "The default maximum is ten million events", S.GetMaxEventListSize(), 10000000U) && Passed;

  SilenceOutput();
  bool Zero = S.SetMaxEventListSize(0);
  bool TooSmall = S.SetMaxEventListSize(99);
  RestoreOutput();
  Passed = EvaluateFalse("SetMaxEventListSize()", "0", "A maximum of 0 is rejected", Zero) && Passed;
  Passed = EvaluateFalse("SetMaxEventListSize()", "99", "A maximum below 100 is rejected", TooSmall) && Passed;
  Passed = Evaluate("GetMaxEventListSize()", "rejected", "A rejected maximum does not change the stored one", S.GetMaxEventListSize(), 10000000U) && Passed;
  Passed = EvaluateTrue("SetMaxEventListSize()", "100", "A maximum of 100 is accepted", S.SetMaxEventListSize(100)) && Passed;
  Passed = Evaluate("GetMaxEventListSize()", "100", "The maximum is stored", S.GetMaxEventListSize(), 100U) && Passed;

  SilenceOutput();
  bool Added = true;
  for (unsigned int i = 1; i <= 101; ++i) {
    Added = S.AddToEventList(100.0*keV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), double(i)*second, Gamma, "Cube") && Added;
  }
  RestoreOutput();
  Passed = EvaluateTrue("AddToEventList()", "cap", "All events can be added", Added) && Passed;
  Passed = Evaluate("SizeEventList()", "cap", "The list is limited to the maximum", S.SizeEventList(), 100U) && Passed;

  // The previously last event (t = 100 s) is eliminated to make room for the new one (t = 101 s): the emission times are 1, ..., 99, 101 s
  double LastTime = 0.0;
  bool Valid = true;
  for (unsigned int i = 1; i <= 100; ++i) {
    SilenceOutput();
    S.CalculateNextEmission(0.0, 1.0);
    const double Time = S.GetNextEmission()/second;
    const double Expected = (i < 100) ? double(i) : 101.0;
    Valid = S.GenerateParticles(m_Gun) && Valid;
    RestoreOutput();
    Passed = EvaluateNear("GetNextEmission()", MString("cap, event ") + i, "The events are kept in order, only the previously last one is eliminated", Time, Expected, 1e-9) && Passed;
    LastTime = Time;
  }
  Passed = EvaluateTrue("GenerateParticles()", "cap", "All kept events are generated", Valid) && Passed;
  Passed = EvaluateNear("GetNextEmission()", "cap, last", "The last kept event is the newly added one at 101 s", LastTime, 101.0, 1e-9) && Passed;

  // Lowering the maximum below the number of events in a populated list is rejected: the previous maximum and all events stay (the list is never larger than its maximum)
  {
    MCSource L("Event list lowered cap");
    L.SetEventList();
    SilenceOutput();
    for (unsigned int i = 1; i <= 150; ++i) {
      L.AddToEventList(100.0*keV*i, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), double(i)*second, Gamma, "Cube");
    }
    const bool Lowered = L.SetMaxEventListSize(100);
    const bool Equal = L.SetMaxEventListSize(150);
    const bool Added = L.AddToEventList(100.0*keV*151, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 151.0*second, Gamma, "Cube");
    RestoreOutput();
    Passed = EvaluateFalse("SetMaxEventListSize()", "lowered maximum", "A maximum below the number of events in the list is rejected", Lowered) && Passed;
    Passed = EvaluateTrue("SetMaxEventListSize()", "equal maximum", "A maximum equal to the number of events in the list is accepted", Equal) && Passed;
    Passed = Evaluate("GetMaxEventListSize()", "equal maximum", "The accepted maximum is stored", L.GetMaxEventListSize(), 150U) && Passed;
    Passed = EvaluateTrue("AddToEventList()", "at the maximum", "An event can be added at the maximum", Added) && Passed;
    Passed = Evaluate("SizeEventList()", "at the maximum", "The list stays at its maximum", L.SizeEventList(), 150U) && Passed;
    // The events at the start of the list are untouched (the last one was replaced): the energies 100 keV * i belong to the times i s
    SilenceOutput();
    L.CalculateNextEmission(0.0, 1.0);
    const double FirstTime = L.GetNextEmission()/second;
    const bool Generated = L.GenerateParticles(m_Gun);
    RestoreOutput();
    Passed = EvaluateNear("GetNextEmission()", "after the rejection", "The first event of the list is untouched", FirstTime, 1.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GenerateParticles()", "after the rejection", "The first event can be generated", Generated) && Passed;
    Passed = EvaluateNear("GenerateParticles()", "after the rejection", "The first event keeps its energy", GetGunEnergy()/keV, 100.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEventListParticles()
{
  bool Passed = true;
  RandomGuard Guard(57);

  // Regression test (A5, MCSource::GenerateParticle): a particle type from a file which Geant4 cannot resolve gives a nullptr definition
  // which used to be handed to the gun: the event must be rejected instead
  const MString Unknown = GetTemporaryFileName("EventsUnknownParticle.dat");
  Passed = EvaluateTrue("WriteTextFile()", "unknown particle", "The fixture can be written", WriteTextFile(Unknown, "1 false 99 0 0.5 1 2 3 0 0 1 1 0 0 511\n")) && Passed;
  {
    MCSource S("Unknown particle");
    SilenceOutput();
    bool Opened = S.SetEventListFromFile(Unknown);
    RestoreOutput();
    Passed = EvaluateTrue("SetEventListFromFile()", "unknown particle", "The file can be read", Opened) && Passed;
    UTSDHitShared::ChildOutcome Outcome = UTSDHitShared::RunInChild([&S, this]() { SilenceOutput(); S.CalculateNextEmission(0.0, 1.0); return S.GenerateParticles(m_Gun); });
    Passed = EvaluateTrue("GenerateParticles()", "unknown particle", "An unknown particle type does not crash, but is rejected", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;
    // The event which cannot be generated is discarded, so that it does not block the list (checked in the child, which owns the changed copy):
    UTSDHitShared::ChildOutcome Discarded = UTSDHitShared::RunInChild([&S, this]() {
      SilenceOutput();
      S.CalculateNextEmission(0.0, 1.0);
      return S.GenerateParticles(m_Gun) == false && S.SizeEventList() == 0;
    });
    Passed = EvaluateTrue("GenerateParticles()", "unknown particle, discarded", "The event with the unknown particle is removed from the list", Discarded == UTSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
  }

  // Regression test (A6, MCSource::ContinueReadingEventList): the excitation energy of the event is in keV (like the energy), it used to be read as an unscaled integer,
  // i.e. 142 MeV instead of 142.68 keV, and no isomer could be created
  const MString Isomer = GetTemporaryFileName("EventsIsomer.dat");
  Passed = EvaluateTrue("WriteTextFile()", "isomer", "The fixture can be written", WriteTextFile(Isomer, "1 false 43099 142.68 0.5 1 2 3 0 0 1 1 0 0 511\n")) && Passed;
  {
    MCSource S("Isomer");
    SilenceOutput();
    bool Opened = S.SetEventListFromFile(Isomer);
    RestoreOutput();
    Passed = EvaluateTrue("SetEventListFromFile()", "isomer", "The file can be read", Opened) && Passed;
    UTSDHitShared::ChildOutcome Outcome = UTSDHitShared::RunInChild([&S, this]() {
      SilenceOutput();
      S.CalculateNextEmission(0.0, 1.0);
      if (S.GenerateParticles(m_Gun) == false) return false;
      const G4Ions* Ion = dynamic_cast<const G4Ions*>(m_Gun->GetParticleDefinition());
      return Ion != nullptr && fabs(Ion->GetExcitationEnergy()/keV - 142.68) < 1.0;
    });
    Passed = EvaluateTrue("GenerateParticles()", "isomer", "The excitation energy of the event is used (in keV)", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestIsotopeCount()
{
  bool Passed = true;
  RandomGuard Guard(7);

  MCSource S("Isotopes");
  SilenceOutput();
  bool OK = S.SetParticleType(27060);
  OK = S.SetSpectralType(MCSource::c_Activation) && OK;
  OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint) && OK;
  OK = S.SetEnergy() && OK;
  OK = S.SetPosition(0.0, 0.0, 0.0) && OK;
  OK = S.SetIsotopeCount(3000.0) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("SetIsotopeCount()", "Co60", "The isotope source can be configured", OK) && Passed;
  if (S.GetParticleDefinition() == nullptr) return false;

  // The independent half-life: the lifetime of the Geant4 particle definition
  const double Lifetime = S.GetParticleDefinition()->GetPDGLifeTime();
  Passed = EvaluateNear("GetPDGLifeTime()", "Co60", "Geant4 knows the lifetime of Co60 (shown: the shortfall below a positive value)", max(1e-30 - Lifetime, 0.0), 0.0, 0.0) && Passed;
  if (Lifetime <= 0) return false;
  const double Lambda = 1.0/Lifetime;

  vector<double> Times;
  double Time = 0.0;
  SilenceOutput();
  for (unsigned int i = 0; i < 3000; ++i) {
    S.CalculateNextEmission(Time, 1.0);
    Time = S.GetNextEmission();
    Times.push_back(Time);
  }
  RestoreOutput();
  // The decay times of all isotopes are exponentially distributed (measured in the lifetime):
  vector<double> Scaled;
  for (double T: Times) Scaled.push_back(T*Lambda);
  const double D = KSDistance(Scaled, [](double X) { return exp(-X); }, 0.0, 15.0);
  Passed = EvaluateNear("CalculateNextEmission()", MString("isotopes, KS ") + D, "The decay times of the isotopes follow the exponential decay law with the half-life of the isotope", D, 0.0, KSCritical(Scaled.size())) && Passed;

  // All isotopes are used up: the next emission must not be a finite time:
  SilenceOutput();
  S.CalculateNextEmission(Time, 1.0);
  RestoreOutput();
  Passed = EvaluateTrue("CalculateNextEmission()", "isotopes used up", "After the last isotope the next emission is at infinity (or the source is inactive)", S.GetNextEmission() > 1e300 || S.IsActive() == false) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEventList()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  Passed = EvaluateTrue("FindParticle()", "gamma", "Geant4 knows the gamma", Gamma != nullptr) && Passed;
  if (Gamma == nullptr) return false;

  MCSource S("Event list");
  S.SetEventList();
  Passed = EvaluateTrue("IsEventList()", "set", "The source is an event list", S.IsEventList()) && Passed;

  // Empty:
  SilenceOutput();
  bool EmptyGenerate = S.GenerateParticles(m_Gun);
  bool EmptyTime = S.CalculateNextEmission(0.0, 1.0);
  RestoreOutput();
  Passed = EvaluateFalse("GenerateParticles()", "empty event list", "An empty event list does not generate a particle", EmptyGenerate) && Passed;
  Passed = EvaluateTrue("CalculateNextEmission()", "empty event list", "An empty event list can calculate its next emission", EmptyTime) && Passed;
  Passed = EvaluateNear("GetNextEmission()", "empty event list", "An empty event list never emits (shown: the shortfall below 1e300)", max(1e300 - S.GetNextEmission(), 0.0), 0.0, 0.0) && Passed;

  // Add out of order - they are sorted by time:
  SilenceOutput();
  bool Added = S.AddToEventList(300*keV, G4ThreeVector(3*cm, 0, 0), G4ThreeVector(0, 0, 1), G4ThreeVector(1, 0, 0), 3.0*second, Gamma, "Cube");
  Added = S.AddToEventList(100*keV, G4ThreeVector(1*cm, 0, 0), G4ThreeVector(0, 0, 1), G4ThreeVector(1, 0, 0), 1.0*second, Gamma, "Cube") && Added;
  Added = S.AddToEventList(200*keV, G4ThreeVector(2*cm, 0, 0), G4ThreeVector(0, 0, 1), G4ThreeVector(1, 0, 0), 2.0*second, Gamma, "Cube") && Added;
  RestoreOutput();
  Passed = EvaluateTrue("AddToEventList()", "three events", "Events can be added", Added) && Passed;
  Passed = Evaluate("SizeEventList()", "three events", "The list has three events", S.SizeEventList(), 3U) && Passed;
  Passed = EvaluateTrue("GetEventListNextParticle()", "three events", "The next particle is the gamma", S.GetEventListNextParticle() == Gamma) && Passed;
  Passed = Evaluate("GetEventListNextVolume()", "three events", "The next volume is the given one", S.GetEventListNextVolume(), MString("Cube")) && Passed;

  // Emission times are the event times:
  SilenceOutput();
  S.CalculateNextEmission(0.0, 1.0);
  RestoreOutput();
  Passed = EvaluateNear("GetNextEmission()", "first", "The first emission is the earliest event", S.GetNextEmission()/second, 1.0, 1e-9) && Passed;

  // Generate in time order:
  for (unsigned int i = 0; i < 3; ++i) {
    SilenceOutput();
    bool Generated = S.GenerateParticles(m_Gun);
    RestoreOutput();
    const MString Name = MString("event ") + (i + 1);
    Passed = EvaluateTrue("GenerateParticles()", Name, "The event is generated", Generated) && Passed;
    Passed = EvaluateNear("GenerateParticles()", Name + ", energy", "The energy is the one of the event", GetGunEnergy()/keV, 100.0*(i + 1), 1e-9) && Passed;
    Passed = EvaluateNear("GenerateParticles()", Name + ", position", "The position is the one of the event", GetGunPosition().x()/cm, double(i + 1), 1e-9) && Passed;
    Passed = EvaluateNear("GenerateParticles()", Name + ", direction", "The direction is the one of the event", (GetGunDirection() - G4ThreeVector(0, 0, 1)).mag(), 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GenerateParticles()", Name + ", polarization", "The polarization is the one of the event", (GetGunPolarization() - G4ThreeVector(1, 0, 0)).mag(), 0.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GenerateParticles()", Name + ", particle", "The particle is the gamma", m_Gun->GetParticleDefinition() == Gamma) && Passed;
    Passed = Evaluate("SizeEventList()", Name, "The used event is removed from the list", S.SizeEventList(), 2U - i) && Passed;
    Passed = Evaluate("GetNGeneratedParticles()", Name, "The generated particles are counted", S.GetNGeneratedParticles(), long(i + 1)) && Passed;
  }

  // Now it is empty again:
  SilenceOutput();
  bool After = S.GenerateParticles(m_Gun);
  RestoreOutput();
  Passed = EvaluateFalse("GenerateParticles()", "after last event", "No particle is generated after the list is used up", After) && Passed;

  // Events of a build-up list which the next source would regenerate are skipped:
  MCSource List("Build-up list");
  List.SetBuildUpEventList();
  Passed = EvaluateTrue("IsBuildUpEventList()", "set", "The source is a build-up event list", List.IsBuildUpEventList() && List.IsEventList()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestEventListFile()
{
  bool Passed = true;

  const MString File = GetTemporaryFileName("Events.dat");
  const MString Wrong = GetTemporaryFileName("EventsWrong.dat");
  Passed = EvaluateTrue("WriteTextFile()", "events", "The event fixture can be written",
                        WriteTextFile(File, "1 false 1 0 0.5 1 2 3 0 0 1 1 0 0 511\n2 true 1 0 1.5 -1 -2 -3 0 0 1 0 1 0 662\n\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "wrong events", "The wrong event fixture can be written", WriteTextFile(Wrong, "1 false 1 0 0.5 1 2 3 0 0 1 1 0 0\n")) && Passed;

  MCSource Bad("Wrong event list");
  SilenceOutput();
  bool BadResult = Bad.SetEventListFromFile(Wrong);
  bool Missing = Bad.SetEventListFromFile(GetTemporaryFileName("Missing.dat"));
  RestoreOutput();
  Passed = EvaluateFalse("SetEventListFromFile()", "14 tokens", "A line with less than 15 tokens is rejected", BadResult) && Passed;
  Passed = EvaluateFalse("SetEventListFromFile()", "missing", "A missing file is rejected", Missing) && Passed;

  // The list holds at most 500000 events at once, the rest of the file is read later by ContinueReadingEventList():
  // a file with a few events more than the limit must in the end deliver all of them
  {
    const unsigned int Limit = 500000;
    const unsigned int Extra = 5;
    const MString Large = GetTemporaryFileName("EventsLarge.dat");
    {
      ofstream Out(Large.Data());
      for (unsigned int i = 0; i < Limit + Extra; ++i) {
        Out<<i + 1<<" false 1 0 "<<0.001*(i + 1)<<" 0 0 0 0 0 1 0 0 0 511\n";
      }
    }
    MCSource L("Large event list");
    SilenceOutput();
    bool Opened = L.SetEventListFromFile(Large);
    RestoreOutput();
    Passed = EvaluateTrue("SetEventListFromFile()", "large file", "A file with more events than the limit is accepted", Opened) && Passed;
    Passed = Evaluate("SizeEventList()", "large file", "The list is limited to the maximum number of events", L.SizeEventList(), Limit) && Passed;

    bool Continued = true;
    for (unsigned int i = 0; i < 2*Extra && L.SizeEventList() < Limit + Extra; ++i) {
      SilenceOutput();
      Continued = L.ContinueReadingEventList() && Continued;
      RestoreOutput();
    }
    Passed = EvaluateTrue("ContinueReadingEventList()", "large file", "Continuing to read succeeds", Continued) && Passed;
    Passed = Evaluate("SizeEventList()", "large file, continued", "All events of the file are in the list in the end", L.SizeEventList(), Limit + Extra) && Passed;
    remove(Large.Data());
  }

  MCSource S("File events");
  SilenceOutput();
  bool OK = S.SetEventListFromFile(File);
  RestoreOutput();
  Passed = EvaluateTrue("SetEventListFromFile()", "valid", "A valid event file is accepted", OK) && Passed;
  Passed = Evaluate("SizeEventList()", "valid", "Both events are read", S.SizeEventList(), 2U) && Passed;
  Passed = EvaluateTrue("IsEventList()", "valid", "The source is an event list", S.IsEventList()) && Passed;

  SilenceOutput();
  bool Adding = S.AddToEventList(1*keV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 1*second, nullptr, "");
  S.CalculateNextEmission(0.0, 1.0);
  RestoreOutput();
  Passed = EvaluateFalse("AddToEventList()", "file list", "Events cannot be added to an event list from a file", Adding) && Passed;
  Passed = EvaluateNear("GetNextEmission()", "file list", "The first emission is the time of the first event", S.GetNextEmission()/second, 0.5, 1e-9) && Passed;

  SilenceOutput();
  bool First = S.GenerateParticles(m_Gun);
  RestoreOutput();
  Passed = EvaluateTrue("GenerateParticles()", "first file event", "The first event is generated", First) && Passed;
  Passed = EvaluateNear("GenerateParticles()", "first file event, energy", "The energy is in keV", GetGunEnergy()/keV, 511.0, 1e-9) && Passed;
  Passed = EvaluateNear("GenerateParticles()", "first file event, position", "The position is in cm", (GetGunPosition() - G4ThreeVector(1*cm, 2*cm, 3*cm)).mag()/cm, 0.0, 1e-9) && Passed;
  Passed = EvaluateTrue("GenerateParticles()", "first file event, particle", "The particle is the gamma (type 1)", m_Gun->GetParticleDefinition() != nullptr && m_Gun->GetParticleDefinition()->GetParticleName() == "gamma") && Passed;
  // The second event is a successor, which has to be reflected in the source:
  Passed = EvaluateTrue("IsSuccessor()", "after first file event", "The source is a successor when the next event is a successor", S.IsSuccessor()) && Passed;
  Passed = Evaluate("GetSuccessor()", "after first file event", "The successor is the source itself", S.GetSuccessor(), MString("File events")) && Passed;

  SilenceOutput();
  bool Second = S.GenerateParticles(m_Gun);
  RestoreOutput();
  Passed = EvaluateTrue("GenerateParticles()", "second file event", "The second event is generated", Second) && Passed;
  Passed = EvaluateNear("GenerateParticles()", "second file event, energy", "The energy is in keV", GetGunEnergy()/keV, 662.0, 1e-9) && Passed;
  Passed = EvaluateFalse("IsSuccessor()", "after last file event", "The source is no successor after the last event", S.IsSuccessor()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestSkippedEvents()
{
  bool Passed = true;

  G4ParticleDefinition* Gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  G4ParticleDefinition* Electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");
  if (Gamma == nullptr || Electron == nullptr) return EvaluateTrue("FindParticle()", "gamma and e-", "Geant4 knows the gamma and the electron", false) && Passed;

  MCSource Target("Target");
  Passed = EvaluateTrue("ConfigurePoint()", "target", "The target source can be configured", ConfigurePoint(Target, 1.0/second)) && Passed;
  Target.SetVolume("Cube");

  MCSource List("Build-up");
  List.SetBuildUpEventList();
  SilenceOutput();
  List.AddToEventList(100*keV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 1*second, Gamma, "Cube");
  RestoreOutput();

  Passed = EvaluateTrue("GenerateSkippedEvents()", "matching", "A matching particle and volume results in a skipped event", List.GenerateSkippedEvents(&Target)) && Passed;
  Passed = Evaluate("GetNEventsToSkip()", "matching", "One event is skipped", Target.GetNEventsToSkip(), 1U) && Passed;

  MCSource OtherVolume("Other volume");
  ConfigurePoint(OtherVolume, 1.0/second);
  OtherVolume.SetVolume("Elsewhere");
  Passed = EvaluateFalse("GenerateSkippedEvents()", "other volume", "A different volume does not skip", List.GenerateSkippedEvents(&OtherVolume)) && Passed;

  MCSource Plain("Plain list");
  Plain.SetEventList();
  SilenceOutput();
  Plain.AddToEventList(100*keV, G4ThreeVector(), G4ThreeVector(0, 0, 1), G4ThreeVector(), 1*second, Gamma, "Cube");
  RestoreOutput();
  MCSource Target2("Target 2");
  ConfigurePoint(Target2, 1.0/second);
  Target2.SetVolume("Cube");
  Passed = EvaluateFalse("GenerateSkippedEvents()", "no build-up list", "A list which is not a build-up list does not skip events", Plain.GenerateSkippedEvents(&Target2)) && Passed;

  MCSource Empty("Empty build-up");
  Empty.SetBuildUpEventList();
  Passed = EvaluateFalse("GenerateSkippedEvents()", "empty list", "An empty build-up list does not skip events", Empty.GenerateSkippedEvents(&Target2)) && Passed;

  // A target which is an event list itself is never skipped:
  MCSource EventTarget("Event target");
  EventTarget.SetEventList();
  Passed = EvaluateFalse("GenerateSkippedEvents()", "event list target", "An event list as target is not skipped", List.GenerateSkippedEvents(&EventTarget)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestGenerateParticles()
{
  bool Passed = true;
  RandomGuard Guard(8);

  // Skipping:
  {
    MCSource S("Skipping");
    Passed = EvaluateTrue("ConfigurePoint()", "skipping", "The source can be configured", ConfigurePoint(S, 1.0/second)) && Passed;
    S.NEventsToSkip(2);
    Passed = Evaluate("GetNEventsToSkip()", "two", "Two events are to be skipped", S.GetNEventsToSkip(), 2U) && Passed;
    SilenceOutput();
    bool First = S.GenerateParticles(m_Gun);
    bool Second = S.GenerateParticles(m_Gun);
    bool Third = S.GenerateParticles(m_Gun);
    RestoreOutput();
    Passed = EvaluateFalse("GenerateParticles()", "first skipped", "The first event is skipped", First) && Passed;
    Passed = EvaluateFalse("GenerateParticles()", "second skipped", "The second event is skipped", Second) && Passed;
    Passed = EvaluateTrue("GenerateParticles()", "third", "The third event is generated", Third) && Passed;
    Passed = Evaluate("GetNEventsSkipped()", "after", "Two events have been skipped", S.GetNEventsSkipped(), 2U) && Passed;
    Passed = Evaluate("GetNEventsToSkip()", "after", "No events are left to skip", S.GetNEventsToSkip(), 0U) && Passed;
    Passed = Evaluate("GetNGeneratedParticles()", "after", "Only the generated particle is counted", S.GetNGeneratedParticles(), 1L) && Passed;
  }

  // A complete source generates all parts:
  {
    MCSource S("Complete");
    Passed = EvaluateTrue("ConfigurePoint()", "complete", "The source can be configured", ConfigurePoint(S, 1.0/second)) && Passed;
    SilenceOutput();
    bool Generated = S.GenerateParticles(m_Gun);
    RestoreOutput();
    Passed = EvaluateTrue("GenerateParticles()", "complete", "A complete source generates a particle", Generated) && Passed;
    Passed = EvaluateNear("GenerateParticles()", "complete, energy", "The energy is the one of the source", GetGunEnergy()/keV, 511.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GenerateParticles()", "complete, particle", "The particle is the gamma", m_Gun->GetParticleDefinition() != nullptr && m_Gun->GetParticleDefinition()->GetParticleName() == "gamma") && Passed;
  }

  // Without a particle type the generation fails (and does not crash):
  {
    MCSource S("No particle");
    SilenceOutput();
    S.SetSpectralType(MCSource::c_Monoenergetic);
    S.SetEnergy(511*keV);
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint);
    S.SetPosition(0.0, 0.0, 0.0);
    S.SetFlux(1.0);
    bool Generated = S.GenerateParticles(m_Gun);
    RestoreOutput();
    Passed = EvaluateFalse("GenerateParticles()", "no particle", "A source without particle does not generate", Generated) && Passed;
  }

  // The start position must be inside the world volume:
  {
    MCSource S("Outside");
    SilenceOutput();
    S.SetParticleType(MCSource::c_Gamma);
    S.SetSpectralType(MCSource::c_Monoenergetic);
    S.SetEnergy(511*keV);
    S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint);
    S.SetPosition(0.0, 0.0, 500*cm);
    S.SetFlux(1.0);
    S.GenerateParticleDefinition();
    bool Generated = S.GeneratePosition(m_Gun);
    RestoreOutput();
    Passed = EvaluateFalse("GeneratePosition()", "outside the world", "A start position outside the world volume is rejected", Generated) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestAbsolutePolarization()
{
  bool Passed = true;

  // Regression test (A10, MCSource::GeneratePolarization): an absolute vector is projected perpendicular to the photon direction
  // (it was set to zero when it was not orthogonal to ~1e-14), and it is checked before the run (CheckPolarization)
  auto ConfigureBeam = [&](MCSource& S, const G4ThreeVector& Direction, const G4ThreeVector& Vector, int Type) {
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam) && OK;
    OK = S.SetPosition(0.0, 0.0, 0.0, Direction.x(), Direction.y(), Direction.z(), 1*cm) && OK;
    OK = S.SetFlux(1.0) && OK;
    OK = S.SetPolarizationType(Type) && OK;
    S.SetPolarizationDegree(1.0);
    S.SetPolarization(Vector.x(), Vector.y(), Vector.z());
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  };
  auto Projection = [](const G4ThreeVector& Vector, const G4ThreeVector& Direction) {
    const G4ThreeVector D = Direction.unit();
    return (Vector - Vector.dot(D)*D).unit();
  };

  // A fixed beam and a skew vector: the projection is the expected polarization (independent formula, computed here by hand for the numbers)
  {
    const G4ThreeVector D(0.6, 0.0, 0.8);
    const G4ThreeVector V(0.6, 0.8, 0.3);
    // V.D = 0.6, V - 0.6*D = (0.24, 0.8, -0.18)
    const G4ThreeVector Expected = G4ThreeVector(0.24, 0.8, -0.18).unit();
    MCSource S("Absolute skew");
    Passed = EvaluateTrue("Configure()", "skew", "The source can be configured", ConfigureBeam(S, D, V, MCSource::c_PolarizationAbsolute)) && Passed;
    Passed = EvaluateTrue("CheckPolarization()", "skew", "A skew vector passes the check", S.CheckPolarization(false)) && Passed;
    SilenceOutput();
    bool OK = S.GeneratePosition(m_Gun) && S.GeneratePolarization(m_Gun);
    RestoreOutput();
    Passed = EvaluateTrue("GeneratePolarization()", "skew", "The polarization is generated", OK) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", "skew", "The polarization is the part of the vector perpendicular to the direction", (GetGunPolarization() - Expected).mag(), 0.0, 1e-9) && Passed;
  }

  // An isotropic far field source: the direction changes, every polarization is the projection of the same vector
  {
    RandomGuard Guard(77);
    const G4ThreeVector V(1.0, 2.0, 3.0);
    MCSource S("Absolute isotropic");
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldIsotropic) && OK;
    OK = S.SetPosition() && OK;
    OK = S.SetFlux(1.0/(cm*cm*second)) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm) && OK;
    OK = S.SetPolarizationType(MCSource::c_PolarizationAbsolute) && OK;
    S.SetPolarizationDegree(1.0);
    S.SetPolarization(V.x(), V.y(), V.z());
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    Passed = EvaluateTrue("Configure()", "isotropic", "The source can be configured", OK) && Passed;
    Passed = EvaluateTrue("CheckPolarization()", "isotropic", "The direction is not fixed: the check passes", S.CheckPolarization(false)) && Passed;
    double MaxDifference = 0.0, MaxDot = 0.0;
    unsigned int Generated = 0;
    for (unsigned int i = 0; i < 1000; ++i) {
      SilenceOutput();
      bool Result = S.GeneratePosition(m_Gun) && S.GeneratePolarization(m_Gun);
      RestoreOutput();
      if (Result == false) break;
      ++Generated;
      const G4ThreeVector Direction = GetGunDirection();
      MaxDifference = max(MaxDifference, (GetGunPolarization() - Projection(V, Direction)).mag());
      MaxDot = max(MaxDot, fabs(GetGunPolarization().dot(Direction.unit())));
    }
    Passed = Evaluate("GeneratePolarization()", "isotropic", "All polarizations are generated", Generated, 1000U) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", "isotropic, projection", "Every polarization is the projection of the vector", MaxDifference, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", "isotropic, orthogonal", "Every polarization is orthogonal to the direction", MaxDot, 0.0, 1e-9) && Passed;
  }

  // Vectors which are only slightly tilted from a direction which is not along an axis: the projection is a small difference of large numbers,
  // the polarization must still be a unit vector which is perpendicular to the direction (it used to be set to zero by the orthogonality check)
  for (double Tilt: { 1e-2, 1e-3, 1e-4, 1e-5 }) {
    const G4ThreeVector D = G4ThreeVector(0.3, -0.5, 0.8).unit();
    const G4ThreeVector T = D.orthogonal().unit();
    const G4ThreeVector V = cos(Tilt)*D + sin(Tilt)*T;
    MCSource S("Absolute tilted");
    const MString Name = MString("tilted by ") + Tilt + " rad";
    Passed = EvaluateTrue("Configure()", Name, "The source can be configured", ConfigureBeam(S, D, V, MCSource::c_PolarizationAbsolute)) && Passed;
    Passed = EvaluateTrue("CheckPolarization()", Name, "The vector passes the check", S.CheckPolarization(false)) && Passed;
    SilenceOutput();
    bool OK = S.GeneratePosition(m_Gun) && S.GeneratePolarization(m_Gun);
    RestoreOutput();
    Passed = EvaluateTrue("GeneratePolarization()", Name, "The polarization is generated", OK) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", Name + ", length", "The polarization is a unit vector", GetGunPolarization().mag(), 1.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", Name + ", orthogonal", "The polarization is perpendicular to the direction", GetGunPolarization().dot(D), 0.0, 1e-12) && Passed;
    // The expected direction is the tilt direction T (the part of V perpendicular to D)
    Passed = EvaluateNear("GeneratePolarization()", Name + ", direction", "The polarization is the direction in which the vector is tilted", (GetGunPolarization() - T).mag(), 0.0, 1e-6) && Passed;
  }

  // A vector parallel to the photon direction: no random fallback, the event fails
  {
    const G4ThreeVector D(0.6, 0.0, 0.8);
    MCSource S("Absolute parallel");
    Passed = EvaluateTrue("Configure()", "parallel", "The source can be configured", ConfigureBeam(S, D, 5.0*D, MCSource::c_PolarizationAbsolute)) && Passed;
    SilenceOutput();
    bool Position = S.GeneratePosition(m_Gun);
    bool Polarization = S.GeneratePolarization(m_Gun);
    RestoreOutput();
    Passed = EvaluateTrue("GeneratePosition()", "parallel", "The position is generated", Position) && Passed;
    Passed = EvaluateFalse("GeneratePolarization()", "parallel", "A vector parallel to the photon direction does not give a random polarization", Polarization) && Passed;
  }

  // The check before the run:
  {
    const G4ThreeVector D(0.6, 0.0, 0.8);
    struct Case { const char* m_Name; G4ThreeVector m_Vector; bool m_Oriented; int m_Type; bool m_Expected; };
    const vector<Case> Cases = {
      { "zero vector", G4ThreeVector(0, 0, 0), false, MCSource::c_PolarizationAbsolute, false },
      { "parallel", 2.0*D, false, MCSource::c_PolarizationAbsolute, false },
      { "antiparallel", -3.0*D, false, MCSource::c_PolarizationAbsolute, false },
      { "almost parallel (1e-8 rad)", D + 1e-8*G4ThreeVector(0, 1, 0), false, MCSource::c_PolarizationAbsolute, false },
      { "tilted by 1e-3 rad", D + 1e-3*G4ThreeVector(0, 1, 0), false, MCSource::c_PolarizationAbsolute, true },
      { "perpendicular", G4ThreeVector(0, 1, 0), false, MCSource::c_PolarizationAbsolute, true },
      { "parallel, but the run is oriented", D, true, MCSource::c_PolarizationAbsolute, true },
      { "zero vector, but the run is oriented", G4ThreeVector(0, 0, 0), true, MCSource::c_PolarizationAbsolute, false },
      { "parallel with a relative polarization", D, false, MCSource::c_PolarizationRelativeX, true },
      { "zero vector without polarization", G4ThreeVector(0, 0, 0), false, MCSource::c_PolarizationNone, true }
    };
    for (const Case& C: Cases) {
      MCSource S("Check");
      ConfigureBeam(S, D, C.m_Vector, C.m_Type);
      SilenceOutput();
      const bool Result = S.CheckPolarization(C.m_Oriented);
      RestoreOutput();
      Passed = Evaluate("CheckPolarization()", C.m_Name, "The check accepts or rejects the polarization as expected", Result, C.m_Expected) && Passed;
    }

    // The far field point source has its direction from theta and phi (60 deg, 40 deg):
    const G4ThreeVector Source(sin(60*deg)*cos(40*deg), sin(60*deg)*sin(40*deg), cos(60*deg));
    for (int Parallel = 1; Parallel >= 0; --Parallel) {
      MCSource S("Check far field point");
      SilenceOutput();
      S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
      S.SetStartAreaType(MCSource::c_StartAreaSphere);
      S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm);
      S.SetPosition(60*deg, 40*deg);
      S.SetPolarizationType(MCSource::c_PolarizationAbsolute);
      const G4ThreeVector V = (Parallel == 1) ? -2.0*Source : G4ThreeVector(0, 0, 1);
      S.SetPolarization(V.x(), V.y(), V.z());
      const bool Result = S.CheckPolarization(false);
      RestoreOutput();
      Passed = Evaluate("CheckPolarization()", Parallel == 1 ? "far field point, parallel" : "far field point, skew", "The direction from theta and phi is checked",
                        Result, Parallel == 0) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestPolarization()
{
  bool Passed = true;

  // A beam along (0.6, 0, 0.8):
  const G4ThreeVector D = G4ThreeVector(0.6, 0.0, 0.8);

  auto Configure = [&](MCSource& S, int Type, double Degree, double P1, double P2, double P3) {
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldBeam) && OK;
    OK = S.SetPosition(0.0, 0.0, 0.0, D.x(), D.y(), D.z(), 1*cm) && OK;
    OK = S.SetFlux(1.0) && OK;
    OK = S.SetPolarizationType(Type) && OK;
    S.SetPolarizationDegree(Degree);
    S.SetPolarization(P1, P2, P3);
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  };
  auto Generate = [&](MCSource& S, unsigned int N) {
    vector<G4ThreeVector> P;
    SilenceOutput();
    for (unsigned int i = 0; i < N; ++i) {
      if (S.GeneratePosition(m_Gun) == false || S.GeneratePolarization(m_Gun) == false) break;
      P.push_back(GetGunPolarization());
    }
    RestoreOutput();
    return P;
  };

  // None:
  {
    MCSource S("None");
    Passed = EvaluateTrue("Configure()", "none", "The source can be configured", Configure(S, MCSource::c_PolarizationNone, 1.0, 0, 0, 0)) && Passed;
    vector<G4ThreeVector> P = Generate(S, 100);
    Passed = Evaluate("GeneratePolarization()", "none", "All polarizations are generated", P.size(), size_t(100)) && Passed;
    double Max = 0.0;
    for (const G4ThreeVector& V: P) Max = max(Max, V.mag());
    Passed = EvaluateNear("GeneratePolarization()", "none", "No polarization is set", Max, 0.0, 1e-12) && Passed;
  }

  // Random:
  {
    RandomGuard Guard(9);
    MCSource S("Random");
    Passed = EvaluateTrue("Configure()", "random", "The source can be configured", Configure(S, MCSource::c_PolarizationRandom, 1.0, 0, 0, 0)) && Passed;
    vector<G4ThreeVector> P = Generate(S, c_Samples);
    Passed = Evaluate("GeneratePolarization()", "random", "All polarizations are generated", P.size(), size_t(c_Samples)) && Passed;
    double MaxNorm = 0.0, MaxDot = 0.0;
    vector<double> Angles;
    const G4ThreeVector U = D.orthogonal().unit();
    const G4ThreeVector V = D.cross(U);
    for (const G4ThreeVector& Q: P) {
      MaxNorm = max(MaxNorm, fabs(Q.mag() - 1.0));
      MaxDot = max(MaxDot, fabs(Q.dot(D)));
      double Angle = atan2(Q.dot(V), Q.dot(U));
      Angles.push_back(Angle < 0 ? Angle + 2*c_Pi : Angle);
    }
    Passed = EvaluateNear("GeneratePolarization()", "random, norm", "The polarization vectors are unit vectors", MaxNorm, 0.0, 1e-9) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", "random, orthogonal", "The polarization vectors are orthogonal to the direction", MaxDot, 0.0, 1e-9) && Passed;
    Passed = EvaluateTrue("GeneratePolarization()", "random, angle", "The polarization angle is uniform", KSDistance(Angles, [](double) { return 1.0; }, 0.0, 2*c_Pi) < KSCritical(Angles.size())) && Passed;
  }

  // Absolute with degree:
  for (double Degree: { 1.0, 0.3 }) {
    RandomGuard Guard(10);
    MCSource S("Absolute");
    Passed = EvaluateTrue("Configure()", MString("absolute ") + Degree, "The source can be configured", Configure(S, MCSource::c_PolarizationAbsolute, Degree, 0.0, 1.0, 0.0)) && Passed;
    vector<G4ThreeVector> P = Generate(S, c_Samples);
    Passed = Evaluate("GeneratePolarization()", MString("absolute ") + Degree, "All polarizations are generated", P.size(), size_t(c_Samples)) && Passed;
    unsigned int Polarized = 0;
    double MaxDot = 0.0;
    for (const G4ThreeVector& Q: P) {
      if ((Q - G4ThreeVector(0, 1, 0)).mag() < 1e-9) ++Polarized;
      MaxDot = max(MaxDot, fabs(Q.dot(D)));
    }
    const double Expected = Degree*c_Samples;
    const double Sigma = sqrt(c_Samples*Degree*(1.0 - Degree)) + 1.0;
    Passed = EvaluateNear("GeneratePolarization()", MString("absolute ") + Degree + ", degree", "The fraction of fully polarized photons is the polarization degree", double(Polarized), Expected, 5*Sigma) && Passed;
    Passed = EvaluateNear("GeneratePolarization()", MString("absolute ") + Degree + ", orthogonal", "All polarization vectors are orthogonal to the direction", MaxDot, 0.0, 1e-9) && Passed;
  }

  // Relative:
  {
    const vector<tuple<int, G4ThreeVector, MString>> Cases = {
      { MCSource::c_PolarizationRelativeX, G4ThreeVector(0.0, 1.0, 0.0), "relative x" },
      { MCSource::c_PolarizationRelativeY, G4ThreeVector(-0.8, 0.0, 0.6), "relative y" },
      { MCSource::c_PolarizationRelativeZ, G4ThreeVector(0.0, -1.0, 0.0), "relative z" }
    };
    for (const auto& C: Cases) {
      for (double Angle: { 0.0, 60*deg }) {
        MCSource S("Relative");
        Passed = EvaluateTrue("Configure()", get<2>(C), "The source can be configured", Configure(S, get<0>(C), 1.0, Angle, 0, 0)) && Passed;
        vector<G4ThreeVector> P = Generate(S, 20);
        Passed = Evaluate("GeneratePolarization()", get<2>(C), "All polarizations are generated", P.size(), size_t(20)) && Passed;
        // The reference vector is perpendicular to the direction and the axis, rotated around the direction by the angle (right-handed):
        const G4ThreeVector E = get<1>(C);
        const G4ThreeVector Expected = E*cos(Angle) + D.cross(E)*sin(Angle);
        double Max = 0.0;
        for (const G4ThreeVector& Q: P) Max = max(Max, (Q - Expected).mag());
        Passed = EvaluateNear("GeneratePolarization()", get<2>(C) + ", angle " + Angle/deg, "The polarization is the direction-axis normal rotated by the angle", Max, 0.0, 1e-9) && Passed;
      }
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestTransmission()
{
  bool Passed = true;
  RandomGuard Guard(12);

  // Constant transmission of 30%: absorbed particles get energy zero
  const MString File = GetTemporaryFileName("Transmission.dat");
  Passed = EvaluateTrue("WriteTextFile()", "transmission", "The transmission fixture can be written", WriteTextFile(File, "XA 0 90\nYA 100 1000\nAP 0 0 0.3\nAP 0 1 0.3\nAP 1 0 0.3\nAP 1 1 0.3\nEN\n")) && Passed;

  MCSource S("Transmission");
  SilenceOutput();
  bool OK = S.SetParticleType(MCSource::c_Gamma);
  OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
  OK = S.SetEnergy(511*keV) && OK;
  OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint) && OK;
  OK = S.SetPosition(30*deg, 45*deg) && OK;
  OK = S.SetFlux(1.0) && OK;
  OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
  OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm) && OK;
  OK = S.SetFarFieldTransmissionProbability(File) && OK;
  OK = S.GenerateParticleDefinition() && OK;
  RestoreOutput();
  Passed = EvaluateTrue("SetFarFieldTransmissionProbability()", "configure", "The source can be configured", OK) && Passed;

  unsigned int Absorbed = 0;
  const unsigned int N = 20000;
  bool AllGenerated = true;
  for (unsigned int i = 0; i < N; ++i) {
    SilenceOutput();
    AllGenerated = S.GenerateParticles(m_Gun) && AllGenerated;
    RestoreOutput();
    if (GetGunEnergy() == 0.0) ++Absorbed;
  }
  Passed = EvaluateTrue("GenerateParticles()", "transmission", "All particles are generated", AllGenerated) && Passed;
  const double Expected = 0.7*N;
  Passed = EvaluateNear("GenerateParticles()", "transmission", "The absorbed fraction is one minus the transmission probability", double(Absorbed), Expected, 5*sqrt(N*0.7*0.3)) && Passed;
  Passed = Evaluate("GetNGeneratedParticles()", "transmission", "Absorbed particles are still counted as generated", S.GetNGeneratedParticles(), long(N)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestTransmissionVarying()
{
  bool Passed = true;

  // A transmission probability which depends on the zenith angle (x axis, degree) and on the energy (y axis, keV), bilinearly interpolated between the corners:
  // P(0 deg, 100 keV) = 1.0, P(0 deg, 1000 keV) = 0.6, P(90 deg, 100 keV) = 0.2, P(90 deg, 1000 keV) = 0.0
  // A constant probability cannot show swapped axes or wrong units, this one does: the fixture is asymmetric in both variables
  const MString File = GetTemporaryFileName("TransmissionVarying.dat");
  Passed = EvaluateTrue("WriteTextFile()", "varying transmission", "The transmission fixture can be written", WriteTextFile(File, "XA 0 90\nYA 100 1000\nAP 0 0 1.0\nAP 0 1 0.6\nAP 1 0 0.2\nAP 1 1 0.0\nEN\n")) && Passed;

  auto Probability = [](double ThetaDeg, double EnergyKeV) {
    const double x = ThetaDeg/90.0;
    const double y = (EnergyKeV - 100.0)/900.0;
    return (1 - x)*(1 - y)*1.0 + (1 - x)*y*0.6 + x*(1 - y)*0.2 + x*y*0.0;
  };

  unsigned int Index = 0;
  for (const pair<double, double>& C: vector<pair<double, double>>({ { 30.0, 200.0 }, { 30.0, 800.0 }, { 60.0, 200.0 }, { 60.0, 800.0 }, { 0.0, 550.0 } })) {
    RandomGuard Guard(120 + Index++);
    const double Theta = C.first;
    const double Energy = C.second;
    const MString Name = MString("theta ") + Theta + " deg, " + Energy + " keV";
    MCSource S("Varying transmission");
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(Energy*keV) && OK;
    OK = S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint) && OK;
    OK = S.SetPosition(Theta*deg, 45*deg) && OK;
    OK = S.SetFlux(1.0) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm) && OK;
    OK = S.SetFarFieldTransmissionProbability(File) && OK;
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    Passed = EvaluateTrue("SetFarFieldTransmissionProbability()", Name, "The source can be configured", OK) && Passed;

    const unsigned int N = 20000;
    unsigned int Absorbed = 0;
    bool AllGenerated = true;
    for (unsigned int i = 0; i < N; ++i) {
      SilenceOutput();
      AllGenerated = S.GenerateParticles(m_Gun) && AllGenerated;
      RestoreOutput();
      if (GetGunEnergy() == 0.0) ++Absorbed;
    }
    const double P = Probability(Theta, Energy);
    Passed = EvaluateTrue("GenerateParticles()", Name, "All particles are generated", AllGenerated) && Passed;
    Passed = EvaluateNear("GenerateParticles()", Name, "The absorbed fraction is one minus the interpolated transmission probability of the zenith angle and the energy",
                          double(Absorbed), N*(1.0 - P), 5*sqrt(N*P*(1.0 - P)) + 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSourceGeneration::TestOrientationCombination()
{
  bool Passed = true;

  // The default sky is local: a source with an orientation in Galactic coordinates is not allowed:
  MCOrientation Galactic;
  MTokenizer Tokens;
  Tokens.Analyze("Src.Orientation Galactic Fixed 0 0 0 90", false);
  SilenceOutput();
  bool Parsed = Galactic.Parse(Tokens);
  RestoreOutput();
  Passed = EvaluateTrue("MCOrientation::Parse()", "galactic fixed", "The orientation can be parsed", Parsed) && Passed;

  for (int Coordinates: { MCSource::c_FarField, MCSource::c_NearField }) {
    MCSource S("Orientation");
    if (Coordinates == MCSource::c_FarField) {
      SilenceOutput();
      S.SetParticleType(MCSource::c_Gamma);
      S.SetBeamType(MCSource::c_FarField, MCSource::c_FarFieldPoint);
      S.SetPosition(30*deg, 45*deg);
      S.SetFlux(1.0);
      S.SetStartAreaType(MCSource::c_StartAreaSphere);
      S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm);
      S.GenerateParticleDefinition();
      RestoreOutput();
    } else {
      PrepareNearFieldPointSource(S, MCSource::c_Gamma, MCSource::c_Monoenergetic, { 511*keV });
    }
    S.SetOrientation(Galactic);
    SilenceOutput();
    bool Generated = S.GeneratePosition(m_Gun);
    RestoreOutput();
    Passed = EvaluateFalse("GeneratePosition()", Coordinates == MCSource::c_FarField ? "galactic source, local sky, far field" : "galactic source, local sky, near field", "A Galactic source orientation in a local sky is not allowed", Generated) && Passed;
  }

  // A local orientation which is not oriented does not change anything:
  MCSource P("Local");
  PrepareNearFieldPointSource(P, MCSource::c_Gamma, MCSource::c_Monoenergetic, { 511*keV }, 1*cm, 2*cm, 3*cm);
  MCOrientation Local;
  Tokens.Analyze("Src.Orientation Local Fixed", false);
  SilenceOutput();
  bool LocalParsed = Local.Parse(Tokens);
  P.SetOrientation(Local);
  bool LocalGenerated = P.GeneratePosition(m_Gun);
  RestoreOutput();
  Passed = EvaluateTrue("MCOrientation::Parse()", "local fixed", "The local orientation can be parsed", LocalParsed) && Passed;
  Passed = EvaluateTrue("GeneratePosition()", "local source, local sky", "A local source in a local sky is generated", LocalGenerated) && Passed;
  Passed = EvaluateNear("GeneratePosition()", "local source, local sky", "A source without orientation is not moved", (GetGunPosition() - G4ThreeVector(1*cm, 2*cm, 3*cm)).mag()/cm, 0.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceGeneration Test;
  return Test.Run() == true ? 0 : 1;
}
