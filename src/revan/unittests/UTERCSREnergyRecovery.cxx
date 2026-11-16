/*
 * UTERCSREnergyRecovery.cxx
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
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSREnergyRecovery.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected functions and members of MERCSREnergyRecovery
class UTERCSREnergyRecoveryAccess : public MERCSREnergyRecovery
{
public:
  //! Default constructor
  UTERCSREnergyRecoveryAccess() : MERCSREnergyRecovery() {}
  //! Default destructor
  virtual ~UTERCSREnergyRecoveryAccess() {}

  using MERCSREnergyRecovery::ComputeQualityFactor;
  using MERCSREnergyRecovery::GetEscapedEnergy;
  using MERCSREnergyRecovery::m_MaxSearchSequence;
  using MERCSREnergyRecovery::m_TestStatisticsType;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the Compton sequence reconstruction with energy recovery MERCSREnergyRecovery
class UTERCSREnergyRecovery : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSREnergyRecovery() : UTERCSRBase("UTERCSREnergyRecovery") {}
  //! Default destructor
  virtual ~UTERCSREnergyRecovery() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the quality factor of a two-site sequence by hand: 1 - Klein-Nishina cross section normalized to its maximum
  double GetTwoSiteQualityFactor(double Ee, double Eg) const;
  //! Return a new three-site event with exact kinematics and position resolutions of 0.01 cm, and fill its energies
  MRERawEvent* CreatePreciseThreeSiteEvent(vector<double>& Energies);

  //! Test the constructor, the constants, and the text of the options
  bool TestConstructionAndToString();
  //! Test the quality factor of two-site sequences
  bool TestTwoSites();
  //! Test the quality factor of three-site sequences
  bool TestThreeSites();
  //! Test the quality factor of sequences with four sites
  bool TestFourSites();
  //! Test the escaped energy of sequences with four sites
  bool TestEscapedEnergy();
  //! Test the search for the sequence
  bool TestAnalyze();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSREnergyRecovery::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestConstructionAndToString() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestTwoSites() && Passed;
    Passed = TestThreeSites() && Passed;
    Passed = TestFourSites() && Passed;
    Passed = TestEscapedEnergy() && Passed;
    Passed = TestAnalyze() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the quality factor of a two-site sequence by hand: 1 - Klein-Nishina cross section normalized to its maximum
double UTERCSREnergyRecovery::GetTwoSiteQualityFactor(double Ee, double Eg) const
{
  // The normalized Klein-Nishina cross section is the one of MComptonEvent
  return 1.0 - MComptonEvent::GetKleinNishinaNormalized(Ee + Eg, acos(MComptonEvent::ComputeCosPhiViaEeEg(Ee, Eg)));
}


////////////////////////////////////////////////////////////////////////////////


//! Return a new three-site event with exact kinematics and position resolutions of 0.01 cm, and fill its energies
MRERawEvent* UTERCSREnergyRecovery::CreatePreciseThreeSiteEvent(vector<double>& Energies)
{
  vector<MVector> Positions = GetThreeSitePositions();
  Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  MRERawEvent* Event = CreateEvent(m_Geometry, Positions, Energies);
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
bool UTERCSREnergyRecovery::TestConstructionAndToString()
{
  bool Passed = true;

  Passed = Evaluate("c_TestStatisticsChiSquareWithoutErrors", "constant", "The value is 1", MERCSREnergyRecovery::c_TestStatisticsChiSquareWithoutErrors, 1) && Passed;
  Passed = Evaluate("c_TestStatisticsChiSquareWithErrors", "constant", "The value is 2", MERCSREnergyRecovery::c_TestStatisticsChiSquareWithErrors, 2) && Passed;
  Passed = Evaluate("c_TestStatisticsChiSquareProbability", "constant", "The value is 3", MERCSREnergyRecovery::c_TestStatisticsChiSquareProbability, 3) && Passed;
  Passed = Evaluate("c_TestStatisticsFirst", "constant", "The value is 1", MERCSREnergyRecovery::c_TestStatisticsFirst, 1) && Passed;
  Passed = Evaluate("c_TestStatisticsLast", "constant", "The value is 3", MERCSREnergyRecovery::c_TestStatisticsLast, 3) && Passed;

  UTERCSREnergyRecoveryAccess CSR;
  Passed = Evaluate("MERCSREnergyRecovery()", "default", "The test statistics is the chi-square with errors", CSR.m_TestStatisticsType, MERCSREnergyRecovery::c_TestStatisticsChiSquareWithErrors) && Passed;
  Passed = Evaluate("MERCSREnergyRecovery()", "default", "The first five hits are used for the estimate of the energy", CSR.m_MaxSearchSequence, 5) && Passed;

  CSR.SetParameters(&m_Geometry, 0.25, 7.5, 4, true);
  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "all parameters", "The title is listed", Text.Contains("# CSR - energy recovery options:\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The test statistics is listed", Text.Contains("# Test statistics:                 Energy recovery\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:               0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:               7.5\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:               4\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The start in the tracker is listed", Text.Contains("# GuaranteeStartD1:               1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of two-site sequences
bool UTERCSREnergyRecovery::TestTwoSites()
{
  bool Passed = true;

  UTERCSREnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  // 100 keV, then 400 keV: cos phi = 1 - 511*(1/400 - 1/500) = 0.74
  MRERawEvent* Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(1.0, 1.0, -8.0) }, { 100.0, 400.0 });
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1) };
  vector<MRESE*> Reversed = { Event->GetRESEAt(1), Event->GetRESEAt(0) };
  Passed = EvaluateNear("ComputeQualityFactor()", "100 keV, 400 keV", "The quality factor is 1 - normalized Klein-Nishina cross section", CSR.ComputeQualityFactor(Sequence), GetTwoSiteQualityFactor(100.0, 400.0), 1E-14) && Passed;
  Passed = EvaluateTrue("ComputeQualityFactor()", "100 keV, 400 keV", "The quality factor is between 0 and 1", CSR.ComputeQualityFactor(Sequence) > 0.0 && CSR.ComputeQualityFactor(Sequence) < 1.0) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "100 keV, 400 keV", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-12) && Passed;
  // 400 keV, then 100 keV: cos phi = 1 - 511*(1/100 - 1/500) = -3.1 is no Compton event
  Passed = EvaluateNear("ComputeQualityFactor()", "400 keV, 100 keV", "The sequence fails", CSR.ComputeQualityFactor(Reversed), MERCSR::c_CSRFailed, 0.0) && Passed;
  delete Event;

  // 300 keV and 400 keV are possible in both orders
  MRawEventIncarnations List;
  Event = CreateEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(0.0, 0.0, 3.0) }, { 400.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  double QualityFactor300First = GetTwoSiteQualityFactor(300.0, 400.0);
  double QualityFactor400First = GetTwoSiteQualityFactor(400.0, 300.0);
  Passed = EvaluateTrue("GetTwoSiteQualityFactor()", "300 keV and 400 keV", "The sequence with 300 keV first has the better quality factor", QualityFactor300First < QualityFactor400First) && Passed;
  Passed = EvaluateTrue("Analyze()", "300 keV and 400 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "300 keV and 400 keV", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = EvaluateTrue("Analyze()", "300 keV and 400 keV", "The start point is the hit with 300 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateNear("Analyze()", "300 keV and 400 keV", "The first quality factor is the one of the better sequence", Event->GetComptonQualityFactor1(), QualityFactor300First, 1E-14) && Passed;
  Passed = EvaluateNear("Analyze()", "300 keV and 400 keV", "The second quality factor is the one of the other sequence", Event->GetComptonQualityFactor2(), QualityFactor400First, 1E-14) && Passed;
  Passed = EvaluateNear("Analyze()", "300 keV and 400 keV", "No energy is added to the event", Event->GetEnergy(), 700.0, 1E-12) && Passed;

  // Only one possible order: there is no second quality factor
  Event = CreateEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(0.0, 0.0, 3.0) }, { 400.0, 100.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "100 keV and 400 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "100 keV and 400 keV", "The start point is the hit with 100 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateNear("Analyze()", "100 keV and 400 keV", "The second quality factor is the one of a failed sequence", Event->GetComptonQualityFactor2(), MERCSR::c_CSRFailed, 0.0) && Passed;

  // No possible order: 50 keV and 50 keV
  Event = CreateEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(0.0, 0.0, 3.0) }, { 50.0, 50.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "50 keV and 50 keV", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "50 keV and 50 keV", "The event is rejected since there is no good combination", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRNoGoodCombination) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of three-site sequences
bool UTERCSREnergyRecovery::TestThreeSites()
{
  bool Passed = true;

  UTERCSREnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  vector<double> Energies;
  MRERawEvent* Event = CreatePreciseThreeSiteEvent(Energies);
  Passed = EvaluateTrue("CreatePreciseThreeSiteEvent()", "three sites", "The event is created", Event != nullptr) && Passed;
  if (Event == nullptr) {
    return Passed;
  }
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };

  // The energy of the photon is estimated from the first two deposits and the scatter angle at the second hit
  vector<double> Estimates;
  vector<double> Errors;
  GetEnergyEstimates(Sequence, Estimates, Errors);
  double Total = Energies[0] + Energies[1] + Energies[2];
  Passed = EvaluateNear("GetEnergyEstimates()", "three sites, exact kinematics", "The estimated energy is the total energy", Estimates[0], Total, 1E-10) && Passed;

  // Complete absorption: 1 - product of the normalized Klein-Nishina cross sections of the two scatters at the estimated energy
  double First = MComptonEvent::GetKleinNishinaNormalized(Estimates[0], acos(MComptonEvent::ComputeCosPhiViaEeEg(Energies[0], Estimates[0] - Energies[0])));
  double Second = MComptonEvent::GetKleinNishinaNormalized(Estimates[0] - Energies[0], acos(MComptonEvent::ComputeCosPhiViaEeEg(Energies[1], Estimates[0] - Energies[0] - Energies[1])));
  double Expected = 1.0 - First*Second;
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, exact kinematics", "The quality factor is 1 - product of the normalized Klein-Nishina cross sections", CSR.ComputeQualityFactor(Sequence), Expected, 1E-12) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "three sites, exact kinematics", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;

  // The event is completely absorbed, if the total energy is within 2.5 sigma of the estimated energy
  // The estimate does not depend on the last deposit, so the last deposit is changed
  Event->GetRESEAt(2)->SetEnergy(Energies[2] + 2.4*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.4 sigma above the estimate", "The quality factor is the one of complete absorption", CSR.ComputeQualityFactor(Sequence), Expected, 1E-12) && Passed;
  Passed = EvaluateNear("GetEscapedEnergy()", "total energy 2.4 sigma above the estimate", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;
  Event->GetRESEAt(2)->SetEnergy(Energies[2] + 2.6*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.6 sigma above the estimate", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;
  Event->GetRESEAt(2)->SetEnergy(Energies[2] - 2.4*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.4 sigma below the estimate", "The quality factor is the one of complete absorption", CSR.ComputeQualityFactor(Sequence), Expected, 1E-12) && Passed;
  // Incomplete absorption of three-site events is not recovered
  Event->GetRESEAt(2)->SetEnergy(Energies[2] - 2.6*Errors[0]);
  Passed = EvaluateNear("ComputeQualityFactor()", "total energy 2.6 sigma below the estimate", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;
  Event->GetRESEAt(2)->SetEnergy(Energies[2]);

  // The reversed sequence: its estimated energy is far from the total energy
  delete Event;
  Event = CreatePreciseThreeSiteEvent(Energies);
  Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };
  vector<MRESE*> Reversed = { Event->GetRESEAt(2), Event->GetRESEAt(1), Event->GetRESEAt(0) };
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, reversed", "The reversed sequence fails", CSR.ComputeQualityFactor(Reversed), MERCSR::c_CSRFailed, 0.0) && Passed;

  // A hit without energy
  Event->GetRESEAt(1)->SetEnergy(0.0);
  DisableDefaultStreams();
  double NoEnergy = CSR.ComputeQualityFactor(Sequence);
  EnableDefaultStreams();
  Passed = EvaluateNear("ComputeQualityFactor()", "central hit without energy", "The sequence fails", NoEnergy, MERCSR::c_CSRFailed, 0.0) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of sequences with four sites
bool UTERCSREnergyRecovery::TestFourSites()
{
  bool Passed = true;

  UTERCSREnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  // Exact kinematics: both estimates of the energy are the total energy
  vector<MVector> Positions = GetFourSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  Passed = EvaluateSize("GetComptonEnergies()", "four sites", "The energies of the sequence exist", Energies.size(), 4) && Passed;
  if (Energies.size() != 4) {
    return Passed;
  }
  double Total = Energies[0] + Energies[1] + Energies[2] + Energies[3];
  MRERawEvent* Event = CreateEvent(m_Geometry, Positions, Energies);
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2), Event->GetRESEAt(3) };

  vector<double> Estimates;
  vector<double> Errors;
  GetEnergyEstimates(Sequence, Estimates, Errors);
  Passed = EvaluateNear("GetEnergyEstimates()", "four sites, exact kinematics", "The first estimated energy is the total energy", Estimates[0], Total, 1E-10) && Passed;
  Passed = EvaluateNear("GetEnergyEstimates()", "four sites, exact kinematics", "The second estimated energy is the total energy", Estimates[1], Total, 1E-10) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, exact kinematics", "The chi-square of the estimates is zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;

  // A central deposit which is 3 keV too large: the estimates differ
  Event->GetRESEAt(1)->SetEnergy(Energies[1] + 3.0);
  GetEnergyEstimates(Sequence, Estimates, Errors);
  double ChiSquare = GetEstimatesChiSquare(Estimates, Errors, true);
  Passed = EvaluateTrue("GetEstimatesChiSquare()", "four sites, 3 keV more in the second hit", "The chi-square is not zero", ChiSquare > 1E-6) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, 3 keV more in the second hit", "The quality factor is the chi-square of the estimates with their errors", CSR.ComputeQualityFactor(Sequence), ChiSquare, 1E-8*ChiSquare) && Passed;

  CSR.m_TestStatisticsType = MERCSREnergyRecovery::c_TestStatisticsChiSquareWithoutErrors;
  double SquaredDifferences = GetEstimatesChiSquare(Estimates, Errors, false);
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, without errors", "The quality factor is the sum of the squared differences to the average", CSR.ComputeQualityFactor(Sequence), SquaredDifferences, 1E-8*SquaredDifferences) && Passed;

  // The chi-square probability for two degrees of freedom is exp(-chi^2/2)
  CSR.m_TestStatisticsType = MERCSREnergyRecovery::c_TestStatisticsChiSquareProbability;
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, probability", "The quality factor is 1 - chi-square probability", CSR.ComputeQualityFactor(Sequence), 1.0 - exp(-0.5*ChiSquare), 1E-8) && Passed;
  CSR.m_TestStatisticsType = MERCSREnergyRecovery::c_TestStatisticsChiSquareWithErrors;
  Event->GetRESEAt(1)->SetEnergy(Energies[1]);

  // Much more energy than estimated
  Event->GetRESEAt(3)->SetEnergy(Energies[3] + 5000.0);
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, 5000 keV more in the last hit", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;
  Event->GetRESEAt(3)->SetEnergy(Energies[3]);

  // A sequence which is no Compton sequence: the reversed one
  vector<MRESE*> Reversed = { Event->GetRESEAt(3), Event->GetRESEAt(2), Event->GetRESEAt(1), Event->GetRESEAt(0) };
  Passed = EvaluateTrue("ComputeQualityFactor()", "four sites, reversed", "The reversed sequence has a worse quality factor", CSR.ComputeQualityFactor(Reversed) > CSR.ComputeQualityFactor(Sequence)) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the escaped energy of sequences with four sites
bool UTERCSREnergyRecovery::TestEscapedEnergy()
{
  bool Passed = true;

  UTERCSREnergyRecoveryAccess CSR;
  CSR.SetParameters(&m_Geometry);

  vector<MVector> Positions = GetFourSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  if (Energies.size() != 4) {
    return Passed;
  }
  MRERawEvent* Event = CreateEvent(m_Geometry, Positions, Energies);
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2), Event->GetRESEAt(3) };

  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, exact kinematics", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;

  // The last hit lost energy: the estimates do not depend on the last deposit, so the estimated energy stays the true one
  // The measured energy is used, if the missing energy is below 2.5 sigma of the energy resolution of the first three hits: 2.5*sqrt(3*2^2) keV = 8.66 keV
  double Limit = 2.5*sqrt(3*2.0*2.0);
  Event->GetRESEAt(3)->SetEnergy(Energies[3] - 0.99*Limit);
  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, just below the limit missing", "No energy escaped", CSR.GetEscapedEnergy(Sequence), 0.0, 1E-10) && Passed;
  Event->GetRESEAt(3)->SetEnergy(Energies[3] - 1.01*Limit);
  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, just above the limit missing", "The missing energy escaped", CSR.GetEscapedEnergy(Sequence), 1.01*Limit, 1E-9) && Passed;
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites, just above the limit missing", "The chi-square of the estimates is still zero", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12) && Passed;
  Event->GetRESEAt(3)->SetEnergy(Energies[3] - 100.0);
  Passed = EvaluateNear("GetEscapedEnergy()", "four sites, 100 keV missing", "The missing energy escaped", CSR.GetEscapedEnergy(Sequence), 100.0, 1E-9) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for the sequence
bool UTERCSREnergyRecovery::TestAnalyze()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSREnergyRecovery CSR;
  CSR.SetParameters(&m_Geometry);

  // Four sites with exact kinematics and 100 keV missing in the last hit, stored in the order 3, 1, 0, 2
  vector<MVector> Positions = GetFourSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  if (Energies.size() != 4) {
    return Passed;
  }
  double Total = Energies[0] + Energies[1] + Energies[2] + Energies[3];
  MRERawEvent* Event = CreateEvent(m_Geometry, { Positions[3], Positions[1], Positions[0], Positions[2] }, { Energies[3] - 100.0, Energies[1], Energies[0], Energies[2] });
  vector<int> Stored = GetStoredIDs(Event);
  List.SetInitialRawEvent(Event);
  Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  vector<int> Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "four sites, 100 keV missing", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = EvaluateSize("Analyze()", "four sites, 100 keV missing", "The sequence has four hits", Sequence.size(), 4) && Passed;
  if (Sequence.size() == 4) {
    Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The sequence is the true one", Sequence[0] == Stored[2] && Sequence[1] == Stored[1] && Sequence[2] == Stored[3] && Sequence[3] == Stored[0]) && Passed;
  }
  Passed = EvaluateNear("Analyze()", "four sites, 100 keV missing", "The first quality factor is zero", Event->GetComptonQualityFactor1(), 0.0, 1E-12) && Passed;
  Passed = EvaluateNear("Analyze()", "four sites, 100 keV missing", "The energy of the event is the true energy of the photon", Event->GetEnergy(), Total, 1E-9) && Passed;
  Passed = EvaluateTrue("Analyze()", "four sites, 100 keV missing", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;

  // The complete event has no additional energy
  Event = CreateEvent(m_Geometry, { Positions[3], Positions[1], Positions[0], Positions[2] }, { Energies[3], Energies[1], Energies[0], Energies[2] });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "four sites, complete", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateNear("Analyze()", "four sites, complete", "The energy of the event is the sum of the deposits", Event->GetEnergy(), Total, 1E-9) && Passed;

  // One hit more than allowed
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 3);
  Event = CreateEvent(m_Geometry, Positions, Energies);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one hit more than allowed", "The event is rejected since it has too many hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionTooManyHitsCSR) && Passed;

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

  UTERCSREnergyRecovery Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSREnergyRecovery.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
