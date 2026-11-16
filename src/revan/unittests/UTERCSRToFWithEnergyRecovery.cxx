/*
 * UTERCSRToFWithEnergyRecovery.cxx
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


// Standard libs:
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSRToFWithEnergyRecovery.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected functions and members of MERCSRToFWithEnergyRecovery
class UTERCSRToFWithEnergyRecoveryAccess : public MERCSRToFWithEnergyRecovery
{
public:
  //! Default constructor
  UTERCSRToFWithEnergyRecoveryAccess() : MERCSRToFWithEnergyRecovery() {}
  //! Default destructor
  virtual ~UTERCSRToFWithEnergyRecoveryAccess() {}

  using MERCSRToFWithEnergyRecovery::ComputeQualityFactor;
  using MERCSRToFWithEnergyRecovery::GetEscapedEnergy;
  using MERCSRToFWithEnergyRecovery::m_MaxSearchSequence;
  using MERCSRToFWithEnergyRecovery::m_TestStatisticsType;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the Compton sequence reconstruction with time of flight and energy recovery MERCSRToFWithEnergyRecovery
class UTERCSRToFWithEnergyRecovery : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSRToFWithEnergyRecovery() : UTERCSRBase("UTERCSRToFWithEnergyRecovery") {}
  //! Default destructor
  virtual ~UTERCSRToFWithEnergyRecovery() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the normalized Klein-Nishina cross section of MComptonEvent for a photon which deposits Ee (keV) and leaves with Eg (keV)
  double GetKleinNishinaNormalized(double Ee, double Eg) const;
  //! Return a new three-site event with exact kinematics and times, and position resolutions of 0.01 cm, and fill its energies
  MRERawEvent* CreatePreciseThreeSiteEvent(vector<double>& Energies);

  //! Test the constructor, the constants, and the text of the options
  bool TestConstructionAndToString();
  //! Test the quality factor of two-site sequences
  bool TestTwoSites();
  //! Test the quality factor and the escaped energy of three-site sequences
  bool TestThreeSites();
  //! Test the quality factor and the escaped energy of sequences with four sites
  bool TestFourSites();
  //! Test the search for the sequence
  bool TestAnalyze();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRToFWithEnergyRecovery::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestConstructionAndToString() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestTwoSites() && Passed;
    Passed = TestThreeSites() && Passed;
    Passed = TestFourSites() && Passed;
    Passed = TestAnalyze() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the normalized Klein-Nishina cross section of MComptonEvent for a photon which deposits Ee (keV) and leaves with Eg (keV)
double UTERCSRToFWithEnergyRecovery::GetKleinNishinaNormalized(double Ee, double Eg) const
{
  return MComptonEvent::GetKleinNishinaNormalized(Ee + Eg, acos(MComptonEvent::ComputeCosPhiViaEeEg(Ee, Eg)));
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new three-site event with exact kinematics and times, and position resolutions of 0.01 cm, and fill its energies
MRERawEvent* UTERCSRToFWithEnergyRecovery::CreatePreciseThreeSiteEvent(vector<double>& Energies)
{
  vector<MVector> Positions = GetThreeSitePositions();
  Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  MRERawEvent* Event = CreateTimedEvent(m_Geometry, Positions, Energies);
  if (Event == nullptr) {
    return nullptr;
  }
  for (int r = 0; r < Event->GetNRESEs(); ++r) {
    Event->GetRESEAt(r)->SetPositionResolution(MVector(0.01, 0.01, 0.01));
  }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor, the constants, and the text of the options
bool UTERCSRToFWithEnergyRecovery::TestConstructionAndToString()
{
  bool Passed = true;

  Passed = Evaluate("c_TestStatisticsChiSquareWithoutErrors", "constant", "The value is 1", MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareWithoutErrors, 1) && Passed;
  Passed = Evaluate("c_TestStatisticsChiSquareWithErrors", "constant", "The value is 2", MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareWithErrors, 2) && Passed;
  Passed = Evaluate("c_TestStatisticsChiSquareProbability", "constant", "The value is 3", MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareProbability, 3) && Passed;
  Passed = Evaluate("c_TestStatisticsFirst", "constant", "The value is 1", MERCSRToFWithEnergyRecovery::c_TestStatisticsFirst, 1) && Passed;
  Passed = Evaluate("c_TestStatisticsLast", "constant", "The value is 3", MERCSRToFWithEnergyRecovery::c_TestStatisticsLast, 3) && Passed;

  UTERCSRToFWithEnergyRecoveryAccess CSR;
  Passed = Evaluate("MERCSRToFWithEnergyRecovery()", "default", "The test statistics is the chi-square probability", CSR.m_TestStatisticsType, MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareProbability) && Passed;
  Passed = Evaluate("MERCSRToFWithEnergyRecovery()", "default", "All hits are used for the estimate of the energy", CSR.m_MaxSearchSequence, numeric_limits<int>::max()) && Passed;

  CSR.SetParameters(&m_Geometry, 0.25, 7.5, 4, true);
  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "all parameters", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:               0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:               7.5\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:               4\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The start in the tracker is listed", Text.Contains("# GuaranteeStartD1:               1\n")) && Passed;
  // Defect REV-G-7: The text is the one of MERCSREnergyRecovery, so the tra file does not tell that the time of flight was used
  Passed = ExpectedToFail(EvaluateFalse("ToString()", "all parameters", "The title is not the one of the energy recovery without time of flight", Text.Contains("# CSR - energy recovery options:\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of two-site sequences
bool UTERCSRToFWithEnergyRecovery::TestTwoSites()
{
  bool Passed = true;

  UTERCSRToFWithEnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  // 300 keV in the tracker, then 400 keV in the calorimeter, with the times of the speed of light:
  // Quality factor 1 - (1 - Q_energy)*(1 - Q_time) with Q_energy = 1 - normalized Klein-Nishina cross section and Q_time = 1 - chi-square probability of the time of flight
  MRERawEvent* Event = CreateTimedEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 300.0, 400.0 });
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  vector<MRESE*> Reversed = { Event->GetRESEAt(1), Event->GetRESEAt(0) };
  Passed = EvaluateNear("ComputeQualityFactor()", "two sites, true order", "The quality factor is the one of the energies alone", CSR.ComputeQualityFactor(Sequence), 1.0 - GetKleinNishinaNormalized(300.0, 400.0), 1E-12) && Passed;
  double ReversedTime = GetChiSquareCumulative(GetTimeOfFlightChiSquare(Reversed[0], Reversed[1]), 1);
  Passed = EvaluateNear("ComputeQualityFactor()", "two sites, reversed order", "The quality factor combines the energies and the time of flight", CSR.ComputeQualityFactor(Reversed), 1.0 - GetKleinNishinaNormalized(400.0, 300.0)*(1.0 - ReversedTime), 1E-12) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "two sites", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-12) && Passed;

  // The second hit is 150 ps late
  Event->GetRESEAt(1)->SetTime(Event->GetRESEAt(1)->GetTime() + 1.5E-10);
  double LateTime = GetChiSquareCumulative(GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]), 1);
  Passed = EvaluateNear("ComputeQualityFactor()", "second hit 150 ps late", "The quality factor combines the energies and the time of flight", CSR.ComputeQualityFactor(Sequence), 1.0 - GetKleinNishinaNormalized(300.0, 400.0)*(1.0 - LateTime), 1E-12) && Passed;

  // Energies which are no Compton event fail, whatever the times are: 400 keV, then 100 keV
  Event->GetRESEAt(0)->SetEnergy(400.0);
  Event->GetRESEAt(1)->SetEnergy(100.0);
  Passed = EvaluateNear("ComputeQualityFactor()", "400 keV, 100 keV", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;
  delete Event;

  // Two hits in the same detector volume: the time of flight is not used and counts as the worst one, so the quality factor is 1
  Event = CreateTimedEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(-2.0, 0.0, -12.0) }, { 300.0, 400.0 });
  Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  Passed = EvaluateNear("ComputeQualityFactor()", "two hits in the calorimeter", "The quality factor is 1", CSR.ComputeQualityFactor(Sequence), 1.0, 0.0) && Passed;
  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor and the escaped energy of three-site sequences
bool UTERCSRToFWithEnergyRecovery::TestThreeSites()
{
  bool Passed = true;

  UTERCSRToFWithEnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  vector<double> Energies;
  MRERawEvent* Event = CreatePreciseThreeSiteEvent(Energies);
  Passed = EvaluateTrue("CreatePreciseThreeSiteEvent()", "three sites", "The event is created", Event != nullptr) && Passed;
  if (Event == nullptr) {
    return Passed;
  }
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };

  vector<double> Estimates;
  vector<double> Errors;
  GetEnergyEstimates(Sequence, Estimates, Errors);

  // Complete absorption and exact times: 1 - product of the normalized Klein-Nishina cross sections of the two scatters at the estimated energy
  double KleinNishina = GetKleinNishinaNormalized(Energies[0], Estimates[0] - Energies[0])*GetKleinNishinaNormalized(Energies[1], Estimates[0] - Energies[0] - Energies[1]);
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, exact", "The quality factor is 1 - product of the normalized Klein-Nishina cross sections", CSR.ComputeQualityFactor(Sequence), 1.0 - KleinNishina, 1E-12) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "three sites, exact", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;

  // The total energy is within 2.5 sigma of the estimate: complete absorption
  Event->GetRESEAt(2)->SetEnergy(Energies[2] - 2.4*Errors[0]);
  Passed = EvaluateNear("GetEscapedEnergy()", "total energy 2.4 sigma below the estimate", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;
  // Below: the energy is recovered, and the quality factor is still the one of the first two scatters
  Event->GetRESEAt(2)->SetEnergy(Energies[2] - 2.6*Errors[0]);
  Passed = EvaluateNear("GetEscapedEnergy()", "total energy 2.6 sigma below the estimate", "The missing energy escaped", CSR.GetEscapedEnergy(Sequence), 2.6*Errors[0], 1E-9) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.6 sigma below the estimate", "The quality factor is 1 - product of the normalized Klein-Nishina cross sections", CSR.ComputeQualityFactor(Sequence), 1.0 - KleinNishina, 1E-12) && Passed;
  // Above: the sequence fails
  Event->GetRESEAt(2)->SetEnergy(Energies[2] + 2.4*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.4 sigma above the estimate", "The quality factor is the one of complete absorption", CSR.ComputeQualityFactor(Sequence), 1.0 - KleinNishina, 1E-12) && Passed;
  Event->GetRESEAt(2)->SetEnergy(Energies[2] + 2.6*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.6 sigma above the estimate", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;
  Event->GetRESEAt(2)->SetEnergy(Energies[2]);

  // Shifted times: two times of flight, i.e. two degrees of freedom
  Event->GetRESEAt(1)->SetTime(Event->GetRESEAt(1)->GetTime() + 1.0E-10);
  Event->GetRESEAt(2)->SetTime(Event->GetRESEAt(2)->GetTime() - 0.5E-10);
  double Time = GetChiSquareCumulative(GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]) + GetTimeOfFlightChiSquare(Sequence[1], Sequence[2]), 2);
  Passed = EvaluateTrue("GetChiSquareCumulative()", "three sites, shifted times", "The time of flight part is between 0.1 and 0.9", Time > 0.1 && Time < 0.9) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, shifted times", "The quality factor combines the energies and the times of flight", CSR.ComputeQualityFactor(Sequence), 1.0 - KleinNishina*(1.0 - Time), 1E-12) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor and the escaped energy of sequences with four sites
bool UTERCSRToFWithEnergyRecovery::TestFourSites()
{
  bool Passed = true;

  UTERCSRToFWithEnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  vector<MVector> Positions = GetFourSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  Passed = EvaluateSize("GetComptonEnergies()", "four sites", "The energies of the sequence exist", Energies.size(), 4) && Passed;
  if (Energies.size() != 4) {
    return Passed;
  }
  MRERawEvent* Event = CreateTimedEvent(m_Geometry, Positions, Energies);
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2), Event->GetRESEAt(3) };

  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, exact", "The quality factor is zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, exact", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;

  // 3 keV more in the second hit and shifted times: the chi-square probability of the two energy estimates (two degrees of freedom: exp(-chi^2/2)) and of the three times of flight
  Event->GetRESEAt(1)->SetEnergy(Energies[1] + 3.0);
  Event->GetRESEAt(2)->SetTime(Event->GetRESEAt(2)->GetTime() + 1.0E-10);
  vector<double> Estimates;
  vector<double> Errors;
  GetEnergyEstimates(Sequence, Estimates, Errors);
  double Energy = 1.0 - exp(-0.5*GetEstimatesChiSquare(Estimates, Errors, true));
  double Time = GetChiSquareCumulative(GetTimeOfFlightChiSquare(Sequence[0], Sequence[1]) + GetTimeOfFlightChiSquare(Sequence[1], Sequence[2]) + GetTimeOfFlightChiSquare(Sequence[2], Sequence[3]), 3);
  Passed = EvaluateTrue("GetEstimatesChiSquare()", "four sites, shifted", "The energy part is not zero", Energy > 1E-6) && Passed;
  Passed = EvaluateTrue("GetChiSquareCumulative()", "four sites, shifted", "The time of flight part is between 0.01 and 0.99", Time > 0.01 && Time < 0.99) && Passed;
  // The position errors of the expectation are numerical derivatives with a relative error of 1E-8
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, shifted", "The quality factor combines the energy estimates and the times of flight", CSR.ComputeQualityFactor(Sequence), 1.0 - (1.0 - Energy)*(1.0 - Time), 1E-8) && Passed;
  Event->GetRESEAt(1)->SetEnergy(Energies[1]);

  // 100 keV missing in the last hit
  Event->GetRESEAt(3)->SetEnergy(Energies[3] - 100.0);
  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, 100 keV missing", "The missing energy escaped", CSR.GetEscapedEnergy(Sequence), 100.0, 1E-9) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for the sequence
bool UTERCSRToFWithEnergyRecovery::TestAnalyze()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRToFWithEnergyRecovery CSR;
  CSR.SetParameters(&m_Geometry);

  // Four sites with exact kinematics and times and 100 keV missing in the last hit, stored in the order 3, 1, 0, 2
  vector<MVector> Positions = GetFourSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  if (Energies.size() != 4) {
    return Passed;
  }
  double Total = Energies[0] + Energies[1] + Energies[2] + Energies[3];
  MRERawEvent* Timed = CreateTimedEvent(m_Geometry, Positions, { Energies[0], Energies[1], Energies[2], Energies[3] - 100.0 });
  MRERawEvent* Event = new MRERawEvent(&m_Geometry);
  MRESE* Hit0 = Timed->RemoveRESEAt(0);
  MRESE* Hit1 = Timed->RemoveRESEAt(1);
  MRESE* Hit2 = Timed->RemoveRESEAt(2);
  MRESE* Hit3 = Timed->RemoveRESEAt(3);
  delete Timed;
  Event->AddRESE(Hit3);
  Event->AddRESE(Hit1);
  Event->AddRESE(Hit0);
  Event->AddRESE(Hit2);
  List.SetInitialRawEvent(Event);
  Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  vector<int> Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateSize("Analyze()", "four sites, 100 keV missing", "The sequence has four hits", Sequence.size(), 4) && Passed;
  if (Sequence.size() == 4) {
    Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The sequence is the true one", Sequence[0] == Hit0->GetID() && Sequence[1] == Hit1->GetID() && Sequence[2] == Hit2->GetID() && Sequence[3] == Hit3->GetID()) && Passed;
  }
  Passed = EvaluateNear("Analyze()", "four sites, 100 keV missing", "The first quality factor is zero", Event->GetComptonQualityFactor1(), 0.0, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "four sites, 100 keV missing", "The energy of the event is the true energy of the photon", Event->GetEnergy(), Total, 1E-9) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Batch mode first, otherwise ROOT connects to the display: a test must not depend on a display being available
  gROOT->SetBatch(true);
  TApplication ROOT("ROOT", 0, 0);
  // Hide the information messages of ROOT
  gErrorIgnoreLevel = kWarning;

  UTERCSRToFWithEnergyRecovery Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRToFWithEnergyRecovery.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
