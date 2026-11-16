/*
 * UTERCSRChiSquare.cxx
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
#include <cfloat>
#include <cmath>
#include <iostream>
#include <new>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MERCSRChiSquare.h"
#include "MRETrack.h"
#include "UTERCSRShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Access to the protected functions and members of MERCSRChiSquare
class UTERCSRChiSquareAccess : public MERCSRChiSquare
{
public:
  //! Default constructor
  UTERCSRChiSquareAccess() : MERCSRChiSquare() {}
  //! Default destructor
  virtual ~UTERCSRChiSquareAccess() {}

  using MERCSRChiSquare::ComputeQualityFactor;
  using MERCSRChiSquare::FindComptonSequenceDualHitEvent;
  using MERCSRChiSquare::m_AssumeD1First;
  using MERCSRChiSquare::m_OriginObjects;
  using MERCSRChiSquare::m_RejectOneDetectorTypeOnlyEvents;
  using MERCSRChiSquare::m_TypeTestStatistics;
  using MERCSRChiSquare::m_UndecidedHandling;
  using MERCSRChiSquare::m_UseComptelTypeEvents;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the classic Compton sequence reconstruction MERCSRChiSquare
class UTERCSRChiSquare : public UTERCSRBase
{
public:
  //! Default constructor
  UTERCSRChiSquare() : UTERCSRBase("UTERCSRChiSquare") {}
  //! Default destructor
  virtual ~UTERCSRChiSquare() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return the position of the hit in the tracker (layer 3) of the two-site events
  MVector GetTrackerPosition() const { return MVector(0.0, 0.0, 3.0); }
  //! Return the position of the hit in the calorimeter of the two-site events
  MVector GetCalorimeterPosition() const { return MVector(1.0, 1.0, -8.0); }
  //! Return the position of the hit in the depth detector of the two-site events
  MVector GetDepthPosition() const { return MVector(30.2, 0.3, 0.5); }

  //! Analyze a new event with two hits and return it - the list owns it
  MRERawEvent* AnalyzeTwoSiteEvent(MERCSRChiSquare& CSR, MRawEventIncarnations& List, const MVector& Position0, double Energy0, const MVector& Position1, double Energy1);
  //! Return the Klein-Nishina cross section per scatter angle, up to a constant factor, of a photon of Ei (keV) which leaves with Eg (keV)
  double GetKleinNishina(double Ei, double Eg) const;
  //! Return the test statistics of the sequence by hand: the average of (cos phi by energies - cos phi by geometry)^2 of all central hits, each divided by the squared errors if requested
  double GetTestStatistics(vector<MRESE*>& Interactions, bool WithErrors) const;

  //! Test the constructor and the constants
  bool TestConstruction();
  //! Test setting the parameters and the text of the options
  bool TestSetParametersAndToString();
  //! Test the rejection of two-site events in one detector type
  bool TestDualHitOneDetectorType();
  //! Test two-site events which have to start in the tracker
  bool TestDualHitStartD1();
  //! Test two-site events with a unique sequence, or none
  bool TestDualHitKinematics();
  //! Test two-site events for which both sequences are possible
  bool TestDualHitUndecided();
  //! Test two-site events with tracks, and the call with a wrong number of hits
  bool TestDualHitSpecialCases();
  //! Test the quality factor of a sequence
  bool TestComputeQualityFactor();
  //! Test the limit of the cosine of the Compton scatter angle
  bool TestCosineLimit();
  //! Test the search for the sequence of events with three and more hits
  bool TestSequences();
  //! Test the events which are no sequences and the limits of the quality factor
  bool TestSpecialEventsAndLimits();

  //! The test telescope
  MGeometryRevan m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTERCSRChiSquare::Run()
{
  bool Passed = true;

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The test telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  Passed = TestConstruction() && Passed;
  if (m_Geometry.IsScanned() == true) {
    Passed = TestSetParametersAndToString() && Passed;
    Passed = TestDualHitOneDetectorType() && Passed;
    Passed = TestDualHitStartD1() && Passed;
    Passed = TestDualHitKinematics() && Passed;
    Passed = TestDualHitUndecided() && Passed;
    Passed = TestDualHitSpecialCases() && Passed;
    Passed = TestComputeQualityFactor() && Passed;
    Passed = TestCosineLimit() && Passed;
    Passed = TestSequences() && Passed;
    Passed = TestSpecialEventsAndLimits() && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze a new event with two hits and return it - the list owns it
MRERawEvent* UTERCSRChiSquare::AnalyzeTwoSiteEvent(MERCSRChiSquare& CSR, MRawEventIncarnations& List, const MVector& Position0, double Energy0, const MVector& Position1, double Energy1)
{
  vector<MVector> Positions = { Position0, Position1 };
  vector<double> Energies = { Energy0, Energy1 };
  MRERawEvent* Event = CreateEvent(m_Geometry, Positions, Energies);
  if (Event == nullptr) {
    return nullptr;
  }

  List.SetInitialRawEvent(Event);
  DisableDefaultStreams();
  CSR.Analyze(&List);
  EnableDefaultStreams();

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the Klein-Nishina cross section per scatter angle, up to a constant factor, of a photon of Ei (keV) which leaves with Eg (keV)
double UTERCSRChiSquare::GetKleinNishina(double Ei, double Eg) const
{
  // d sigma / d phi ~ (Eg/Ei)^2 * (Eg/Ei + Ei/Eg - sin^2 phi) * sin phi
  double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(Ei - Eg, Eg);
  double SinPhi = sqrt(1.0 - CosPhi*CosPhi);

  return Eg*Eg/(Ei*Ei)*(Eg/Ei + Ei/Eg - SinPhi*SinPhi)*SinPhi;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the test statistics of the sequence by hand: the average of (cos phi by energies - cos phi by geometry)^2 of all central hits, each divided by the squared errors if requested
double UTERCSRChiSquare::GetTestStatistics(vector<MRESE*>& Interactions, bool WithErrors) const
{
  double Sum = 0.0;
  for (unsigned int i = 1; i + 1 < Interactions.size(); ++i) {
    // The energy of the recoil electron and of the scattered photon, which is deposited in all later hits
    double Ee = Interactions[i]->GetEnergy();
    double dEe = Interactions[i]->GetEnergyResolution();
    double Eg = 0.0;
    double dEg2 = 0.0;
    for (unsigned int j = i + 1; j < Interactions.size(); ++j) {
      Eg += Interactions[j]->GetEnergy();
      dEg2 += Interactions[j]->GetEnergyResolution()*Interactions[j]->GetEnergyResolution();
    }

    double CosPhiE = MComptonEvent::ComputeCosPhiViaEeEg(Ee, Eg);
    double CosPhiA = GetCosScatterAngle(Interactions[i - 1]->GetPosition(), Interactions[i]->GetPosition(), Interactions[i + 1]->GetPosition());

    if (WithErrors == true) {
      // Gaussian error propagation of cos phi = 1 - E0/Eg + E0/(Ee + Eg):
      // d/dEe = -E0/(Ee + Eg)^2, d/dEg = E0/Eg^2 - E0/(Ee + Eg)^2
      double dCosPhiE2 = pow(c_E0/((Ee + Eg)*(Ee + Eg)), 2)*dEe*dEe + pow(c_E0/(Eg*Eg) - c_E0/((Ee + Eg)*(Ee + Eg)), 2)*dEg2;
      double dCosPhiA = GetNumericalPositionError(Interactions[i - 1], Interactions[i], Interactions[i + 1]);
      Sum += (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA)/(dCosPhiE2 + dCosPhiA*dCosPhiA);
    } else {
      Sum += (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA);
    }
  }

  return Sum/(Interactions.size() - 2);
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructor and the constants
bool UTERCSRChiSquare::TestConstruction()
{
  bool Passed = true;

  // Construct the object in a block filled with a pattern, which is no valid value of any member
  alignas(UTERCSRChiSquareAccess) unsigned char Block[sizeof(UTERCSRChiSquareAccess)];
  FillMemory(Block, sizeof(Block), 0xA5);
  UTERCSRChiSquareAccess* CSR = new (Block) UTERCSRChiSquareAccess();

  Passed = EvaluateFalse("MERCSRChiSquare()", "default", "Events without track are not used", CSR->m_UseComptelTypeEvents) && Passed;
  Passed = EvaluateFalse("MERCSRChiSquare()", "default", "Events in one detector type are not rejected", CSR->m_RejectOneDetectorTypeOnlyEvents) && Passed;
  Passed = Evaluate("MERCSRChiSquare()", "default", "Undecided events are ignored", CSR->m_UndecidedHandling, MERCSRChiSquare::c_UndecidedIgnore) && Passed;
  Passed = EvaluateFalse("MERCSRChiSquare()", "default", "The first hit is not assumed to be in the tracker", CSR->m_AssumeD1First) && Passed;
  Passed = Evaluate("MERCSRChiSquare()", "default", "The test statistics is the simple one", CSR->m_TypeTestStatistics, MERCSRChiSquare::c_TSSimple) && Passed;
  // Defect REV-G-1: The constructor does not initialize the geometry with the origin objects
  Passed = ExpectedToFail(EvaluateFalse("MERCSRChiSquare()", "default", "The geometry with the origin objects is initialized", IsFillPattern(&CSR->m_OriginObjects, sizeof(MGeometryRevan*), 0xA5))) && Passed;

  CSR->~UTERCSRChiSquareAccess();

  Passed = Evaluate("c_TSUnknown", "constant", "The value is -1", MERCSRChiSquare::c_TSUnknown, -1) && Passed;
  Passed = Evaluate("c_TSSimple", "constant", "The value is 0", MERCSRChiSquare::c_TSSimple, 0) && Passed;
  Passed = Evaluate("c_TSSimpleWithErrors", "constant", "The value is 1", MERCSRChiSquare::c_TSSimpleWithErrors, 1) && Passed;
  Passed = Evaluate("c_TSChiSquare", "constant", "The value is 2", MERCSRChiSquare::c_TSChiSquare, 2) && Passed;
  Passed = Evaluate("c_TSFirst", "constant", "The first test statistics is the simple one", MERCSRChiSquare::c_TSFirst, MERCSRChiSquare::c_TSSimple) && Passed;
  Passed = Evaluate("c_TSLast", "constant", "The last test statistics is the chi-square", MERCSRChiSquare::c_TSLast, MERCSRChiSquare::c_TSChiSquare) && Passed;
  Passed = Evaluate("c_UndecidedIgnore", "constant", "The value is 0", MERCSRChiSquare::c_UndecidedIgnore, 0) && Passed;
  Passed = Evaluate("c_UndecidedAssumestartD1", "constant", "The value is 1", MERCSRChiSquare::c_UndecidedAssumestartD1, 1) && Passed;
  Passed = Evaluate("c_UndecidedLargerKleinNishina", "constant", "The value is 2", MERCSRChiSquare::c_UndecidedLargerKleinNishina, 2) && Passed;
  Passed = Evaluate("c_UndecidedLargerKleinNishinaTimesPhoto", "constant", "The value is 3", MERCSRChiSquare::c_UndecidedLargerKleinNishinaTimesPhoto, 3) && Passed;
  Passed = Evaluate("c_UndecidedLargerEnergyDeposit", "constant", "The value is 4", MERCSRChiSquare::c_UndecidedLargerEnergyDeposit, 4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test setting the parameters and the text of the options
bool UTERCSRChiSquare::TestSetParametersAndToString()
{
  bool Passed = true;

  UTERCSRChiSquareAccess CSR;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The parameters are accepted", CSR.SetParameters(&m_Geometry, 0.25, 7.5, 4, false, false, false, MERCSRChiSquare::c_UndecidedLargerEnergyDeposit, false, &m_Geometry)) && Passed;
  Passed = EvaluateFalse("SetParameters()", "all parameters", "Events without track are not used", CSR.m_UseComptelTypeEvents) && Passed;
  Passed = Evaluate("SetParameters()", "all parameters", "The handling of undecided events is stored", CSR.m_UndecidedHandling, MERCSRChiSquare::c_UndecidedLargerEnergyDeposit) && Passed;
  Passed = EvaluateFalse("SetParameters()", "all parameters", "Events in one detector type are not rejected", CSR.m_RejectOneDetectorTypeOnlyEvents) && Passed;
  Passed = EvaluateTrue("SetParameters()", "all parameters", "The geometry with the origin objects is stored", CSR.m_OriginObjects == &m_Geometry) && Passed;

  MString Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "all parameters", "The title is listed", Text.Contains("# CSR Chi-Square options:\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The test statistics is listed", Text.Contains("# Test statistics:                 0\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The minimum quality factor is listed", Text.Contains("# QualityFactorMin:                0.25\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum quality factor is listed", Text.Contains("# QualityFactorMax:                7.5\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The maximum number of interactions is listed", Text.Contains("# MaxNInteractions:                4\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The use of events without track is listed", Text.Contains("# UseComptelTypeEvents:            0\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The handling of undecided events is listed", Text.Contains("# UndecidedHandling:               4\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The start in the tracker is listed", Text.Contains("# GuaranteeStartD1:                0\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "all parameters", "The rejection of events in one detector type is listed", Text.Contains("# RejectOneDetectorTypeOnlyEvents: 0\n")) && Passed;

  // The defaults of the parameters
  Passed = EvaluateTrue("SetParameters()", "defaults", "The defaults are accepted", CSR.SetParameters(&m_Geometry)) && Passed;
  Passed = EvaluateTrue("SetParameters()", "defaults", "Events without track are used", CSR.m_UseComptelTypeEvents) && Passed;
  Passed = Evaluate("SetParameters()", "defaults", "Undecided events are ignored", CSR.m_UndecidedHandling, MERCSRChiSquare::c_UndecidedIgnore) && Passed;
  Passed = EvaluateTrue("SetParameters()", "defaults", "Events in one detector type are rejected", CSR.m_RejectOneDetectorTypeOnlyEvents) && Passed;
  Passed = EvaluateTrue("SetParameters()", "defaults", "There is no geometry with origin objects", CSR.m_OriginObjects == nullptr) && Passed;
  Text = CSR.ToString();
  Passed = EvaluateTrue("ToString()", "defaults", "The start in the tracker is required", Text.Contains("# GuaranteeStartD1:                1\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "defaults", "The maximum quality factor is 1000", Text.Contains("# QualityFactorMax:                1000\n")) && Passed;
  Passed = EvaluateTrue("ToString()", "defaults", "The maximum number of interactions is 5", Text.Contains("# MaxNInteractions:                5\n")) && Passed;

  // Use photo absorption to order undecided events with stored cross sections:
  Passed = EvaluateTrue("AreCrossSectionsPresent()", "telescope", "The test telescope has cross sections", m_Geometry.AreCrossSectionsPresent()) && Passed;
  Passed = EvaluateTrue("SetParameters()", "Klein-Nishina times photo absorption with cross sections", "The parameters are accepted", CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedLargerKleinNishinaTimesPhoto)) && Passed;

  // Reject photo absorption if any material lacks cross sections:
  MGeometryRevan NoCrossSections;
  Passed = EvaluateTrue("ScanTelescopeGeometry()", "material without cross sections", "The telescope is scanned", ScanTelescopeGeometry(NoCrossSections, GetMaterialWithoutCrossSectionsLines())) && Passed;
  Passed = EvaluateFalse("AreCrossSectionsPresent()", "material without cross sections", "The geometry has no cross sections", NoCrossSections.AreCrossSectionsPresent()) && Passed;
  DisableDefaultStreams();
  bool Accepted = CSR.SetParameters(&NoCrossSections, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedLargerKleinNishinaTimesPhoto);
  EnableDefaultStreams();
  Passed = EvaluateFalse("SetParameters()", "Klein-Nishina times photo absorption without cross sections", "The parameters are rejected", Accepted) && Passed;
  Passed = EvaluateTrue("SetParameters()", "Klein-Nishina without cross sections", "The parameters are accepted", CSR.SetParameters(&NoCrossSections, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedLargerKleinNishina)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejection of two-site events in one detector type
bool UTERCSRChiSquare::TestDualHitOneDetectorType()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedIgnore, true);

  // Two hits in the tracker, the calorimeter, the depth detector, and the shield
  MRERawEvent* Event = AnalyzeTwoSiteEvent(CSR, List, MVector(0.0, 0.0, 3.0), 100.0, MVector(2.0, 0.0, 1.0), 400.0);
  Passed = Evaluate("Analyze()", "two hits in the tracker", "The event is rejected as D1 only", Event->GetRejectionReason(), MRERawEvent::c_RejectionD1Only) && Passed;
  Passed = EvaluateFalse("Analyze()", "two hits in the tracker", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in the tracker", "The event has no start point", Event->GetStartPoint() == nullptr) && Passed;

  Event = AnalyzeTwoSiteEvent(CSR, List, MVector(1.0, 1.0, -8.0), 100.0, MVector(-2.0, 0.0, -12.0), 400.0);
  Passed = Evaluate("Analyze()", "two hits in the calorimeter", "The event is rejected as D2 only", Event->GetRejectionReason(), MRERawEvent::c_RejectionD2Only) && Passed;

  Event = AnalyzeTwoSiteEvent(CSR, List, MVector(30.2, 0.3, 0.5), 100.0, MVector(32.2, -2.3, -0.5), 400.0);
  Passed = Evaluate("Analyze()", "two hits in the depth detector", "The event is rejected as D3 only", Event->GetRejectionReason(), MRERawEvent::c_RejectionD3Only) && Passed;

  Event = AnalyzeTwoSiteEvent(CSR, List, MVector(0.0, 0.0, 10.0), 100.0, MVector(5.0, 5.0, 10.0), 400.0);
  Passed = Evaluate("Analyze()", "two hits in the shield", "The event is rejected as D4 only", Event->GetRejectionReason(), MRERawEvent::c_RejectionD4Only) && Passed;

  // The detector types 5 to 8 are not in the telescope: the type is set by hand
  for (int Type = 5; Type <= 8; ++Type) {
    Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, 3.0), MVector(2.0, 0.0, 1.0) }, { 100.0, 400.0 });
    Event->GetRESEAt(0)->SetDetector(Type);
    Event->GetRESEAt(1)->SetDetector(Type);
    List.SetInitialRawEvent(Event);
    CSR.Analyze(&List);
    Passed = Evaluate("Analyze()", MString("two hits in detector type ") + Type, "The event is rejected as being in this detector type only", Event->GetRejectionReason(), MRERawEvent::c_RejectionD1Only + Type - 1) && Passed;
  }

  // Two different detector types are not rejected: 100 keV in the tracker, then 400 keV in the calorimeter
  Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 100.0, GetCalorimeterPosition(), 400.0);
  Passed = EvaluateTrue("Analyze()", "tracker and calorimeter", "The event is good", Event->IsGoodEvent()) && Passed;

  // Without the rejection two hits in the tracker are analyzed: only 100 keV first is a possible Compton event (see TestDualHitKinematics)
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedIgnore, false);
  Event = AnalyzeTwoSiteEvent(CSR, List, MVector(0.0, 0.0, 3.0), 400.0, MVector(2.0, 0.0, 1.0), 100.0);
  Passed = EvaluateTrue("Analyze()", "two hits in the tracker, no rejection", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "two hits in the tracker, no rejection", "The start point is the hit with 100 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two-site events which have to start in the tracker
bool UTERCSRChiSquare::TestDualHitStartD1()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedIgnore, true);

  // 100 keV in the tracker, 400 keV in the calorimeter: cos phi = 1 - 511*(1/400 - 1/500) = 0.74
  MRERawEvent* Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 100.0, GetCalorimeterPosition(), 400.0);
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = Evaluate("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The event is not rejected", Event->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The start point is the hit in the tracker", Event->GetStartPoint() == Event->GetRESEAt(0)) && Passed;
  Passed = Evaluate("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The first hit is linked to the second", Event->GetRESEAt(0)->GetNLinks(), 1) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The second hit is linked to the first", Event->GetRESEAt(1)->GetNLinks() == 1 && Event->GetRESEAt(1)->GetLinkAt(0) == Event->GetRESEAt(0)) && Passed;
  // The simple test statistics has the quality factors 0 and 1 for a two-site event without a choice
  Passed = EvaluateNear("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The first quality factor is 0", Event->GetComptonQualityFactor1(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The second quality factor is 1", Event->GetComptonQualityFactor2(), 1.0, 0.0) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;

  // The same event with the calorimeter hit stored first
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 100.0);
  Passed = EvaluateTrue("Analyze()", "calorimeter 400 keV, tracker 100 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "calorimeter 400 keV, tracker 100 keV", "The start point is the hit in the tracker", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;

  // 400 keV in the tracker, 100 keV in the calorimeter: cos phi = 1 - 511*(1/100 - 1/500) = -3.1 is no Compton event
  Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 400.0, GetCalorimeterPosition(), 100.0);
  Passed = EvaluateFalse("Analyze()", "tracker 400 keV, calorimeter 100 keV", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "tracker 400 keV, calorimeter 100 keV", "The event is rejected since it does not start in the tracker", Event->GetRejectionReason(), MRERawEvent::c_RejectionEventStartNotD1) && Passed;
  Passed = EvaluateFalse("Analyze()", "tracker 400 keV, calorimeter 100 keV", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  // No hit in the tracker
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 100.0, GetDepthPosition(), 400.0);
  Passed = EvaluateFalse("Analyze()", "calorimeter and depth detector", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "calorimeter and depth detector", "The event is rejected since there is no hit in the tracker", Event->GetRejectionReason(), MRERawEvent::c_RejectionNoHitsInTracker) && Passed;

  // Events without tracks are not used
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true, false, false, MERCSRChiSquare::c_UndecidedIgnore, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 100.0, GetCalorimeterPosition(), 400.0);
  Passed = EvaluateFalse("Analyze()", "no events without track", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "no events without track", "The event is rejected as an event without track", Event->GetRejectionReason(), MRERawEvent::c_RejectionComptelTypeEvent) && Passed;
  Passed = Evaluate("Analyze()", "no events without track", "The hits are not linked", Event->GetRESEAt(0)->GetNLinks(), 0) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two-site events with a unique sequence, or none
bool UTERCSRChiSquare::TestDualHitKinematics()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedIgnore, true);

  // 100 keV, then 400 keV: cos phi = 1 - 511*(1/400 - 1/500) = 0.74 is possible, the opposite order (-3.1) not
  MRERawEvent* Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 100.0, GetCalorimeterPosition(), 400.0);
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The start point is the hit with 100 keV in the tracker", Event->GetStartPoint() == Event->GetRESEAt(0)) && Passed;
  Passed = EvaluateNear("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The first quality factor is 0", Event->GetComptonQualityFactor1(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "tracker 100 keV, calorimeter 400 keV", "The second quality factor is 1", Event->GetComptonQualityFactor2(), 1.0, 0.0) && Passed;

  // The only possible sequence starts in the calorimeter
  Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 400.0, GetCalorimeterPosition(), 100.0);
  Passed = EvaluateTrue("Analyze()", "tracker 400 keV, calorimeter 100 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "tracker 400 keV, calorimeter 100 keV", "The start point is the hit with 100 keV in the calorimeter", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = Evaluate("Analyze()", "tracker 400 keV, calorimeter 100 keV", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;

  // No detector type 1: the first stored hit is the first candidate
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetDepthPosition(), 100.0);
  Passed = EvaluateTrue("Analyze()", "calorimeter 400 keV, depth detector 100 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "calorimeter 400 keV, depth detector 100 keV", "The start point is the hit with 100 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;

  // 50 keV and 50 keV: cos phi = 1 - 511*(1/50 - 1/100) = -4.1 is no Compton event in both orders
  Event = AnalyzeTwoSiteEvent(CSR, List, GetTrackerPosition(), 50.0, GetCalorimeterPosition(), 50.0);
  Passed = EvaluateFalse("Analyze()", "50 keV and 50 keV", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "50 keV and 50 keV", "The event is rejected since no order is a Compton event", Event->GetRejectionReason(), MRERawEvent::c_RejectionComptelTypeKinematicsBad) && Passed;
  Passed = EvaluateTrue("Analyze()", "50 keV and 50 keV", "The event has no start point", Event->GetStartPoint() == nullptr) && Passed;
  Passed = EvaluateFalse("Analyze()", "50 keV and 50 keV", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two-site events for which both sequences are possible
bool UTERCSRChiSquare::TestDualHitUndecided()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;

  // 300 keV and 400 keV: cos phi = 1 - 511*(1/400 - 1/700) = 0.45 and 1 - 511*(1/300 - 1/700) = 0.03 are both possible
  // The calorimeter hit is stored first, so the candidate for the first hit is the second stored hit, in the tracker

  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedIgnore, true);
  MRERawEvent* Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateFalse("Analyze()", "undecided, ignore", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "undecided, ignore", "The event is rejected as undecided", Event->GetRejectionReason(), MRERawEvent::c_RejectionEventStartUndecided) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedAssumestartD1, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateTrue("Analyze()", "undecided, assume start in the tracker", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "undecided, assume start in the tracker", "The start point is the hit in the tracker", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, assume start in the tracker", "The first quality factor is 0", Event->GetComptonQualityFactor1(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, assume start in the tracker", "The second quality factor is 1", Event->GetComptonQualityFactor2(), 1.0, 0.0) && Passed;

  // The larger deposit is first: 400 keV in the calorimeter
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedLargerEnergyDeposit, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateTrue("Analyze()", "undecided, larger deposit", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "undecided, larger deposit", "The start point is the hit with 400 keV", Event->GetStartPoint() == Event->GetRESEAt(0)) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, larger deposit", "The first quality factor is the fraction of the larger deposit", Event->GetComptonQualityFactor1(), 400.0/700.0, 1E-15) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, larger deposit", "The second quality factor is the fraction of the smaller deposit", Event->GetComptonQualityFactor2(), 300.0/700.0, 1E-15) && Passed;

  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 300.0, GetTrackerPosition(), 400.0);
  Passed = EvaluateTrue("Analyze()", "undecided, larger deposit in the tracker", "The start point is the hit with 400 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;

  // The sequence with the larger Klein-Nishina cross section is first
  double KleinNishina300First = GetKleinNishina(700.0, 400.0);
  double KleinNishina400First = GetKleinNishina(700.0, 300.0);
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedLargerKleinNishina, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateTrue("GetKleinNishina()", "700 keV", "The cross section for a first deposit of 300 keV is larger than for 400 keV", KleinNishina300First > KleinNishina400First) && Passed;
  Passed = EvaluateTrue("Analyze()", "undecided, larger Klein-Nishina", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "undecided, larger Klein-Nishina", "The start point is the hit with 300 keV", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, larger Klein-Nishina", "The first quality factor is the fraction of the larger cross section", Event->GetComptonQualityFactor1(), KleinNishina300First/(KleinNishina300First + KleinNishina400First), 1E-13) && Passed;
  Passed = EvaluateNear("Analyze()", "undecided, larger Klein-Nishina", "The second quality factor is the fraction of the smaller cross section", Event->GetComptonQualityFactor2(), KleinNishina400First/(KleinNishina300First + KleinNishina400First), 1E-13) && Passed;

  // An unknown handling rejects the event
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, 99, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateFalse("Analyze()", "undecided, unknown handling", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "undecided, unknown handling", "The event is rejected as undecided", Event->GetRejectionReason(), MRERawEvent::c_RejectionEventStartUndecided) && Passed;

  // With a required start in the tracker the event is not undecided
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true, false, true, MERCSRChiSquare::c_UndecidedIgnore, true);
  Event = AnalyzeTwoSiteEvent(CSR, List, GetCalorimeterPosition(), 400.0, GetTrackerPosition(), 300.0);
  Passed = EvaluateTrue("Analyze()", "both orders possible, start in tracker required", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "both orders possible, start in tracker required", "The start point is the hit in the tracker", Event->GetStartPoint() == Event->GetRESEAt(1)) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test two-site events with tracks, and the call with a wrong number of hits
bool UTERCSRChiSquare::TestDualHitSpecialCases()
{
  bool Passed = true;

  MRawEventIncarnations List;
  UTERCSRChiSquareAccess CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false, false, true, MERCSRChiSquare::c_UndecidedIgnore, false);

  // Two tracks
  MRETrack* Track0 = new MRETrack();
  Track0->AddRESE(CreateHit(MVector(0.0, 0.0, 3.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Track0->AddRESE(CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  MRETrack* Track1 = new MRETrack();
  Track1->AddRESE(CreateHit(MVector(3.0, 3.0, 1.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Track1->AddRESE(CreateHit(MVector(3.0, 3.0, 0.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  MRERawEvent* Event = new MRERawEvent(&m_Geometry);
  Event->AddRESE(Track0);
  Event->AddRESE(Track1);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "two tracks", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "two tracks", "The event is rejected since it has only two tracks", Event->GetRejectionReason(), MRERawEvent::c_RejectionTwoTracksOnly) && Passed;

  // The two-site analysis with three hits does nothing
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  DisableDefaultStreams();
  CSR.FindComptonSequenceDualHitEvent(Event);
  EnableDefaultStreams();
  Passed = EvaluateFalse("FindComptonSequenceDualHitEvent()", "three hits", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("FindComptonSequenceDualHitEvent()", "three hits", "The event is not rejected", Event->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
  Passed = EvaluateTrue("FindComptonSequenceDualHitEvent()", "three hits", "The event has no start point", Event->GetStartPoint() == nullptr) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the quality factor of a sequence
bool UTERCSRChiSquare::TestComputeQualityFactor()
{
  bool Passed = true;

  UTERCSRChiSquareAccess CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false);

  // Three sites with exact Compton kinematics: 150 keV, the deposit which fits to the scatter angle, 300 keV
  vector<MVector> Positions = GetThreeSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  Passed = EvaluateSize("GetComptonEnergies()", "three sites", "The energies of the sequence exist", Energies.size(), 3) && Passed;
  if (Energies.size() != 3) {
    return Passed;
  }
  MRERawEvent* Exact = CreateEvent(m_Geometry, Positions, Energies);
  vector<MRESE*> Sequence = { Exact->GetRESEAt(0), Exact->GetRESEAt(1), Exact->GetRESEAt(2) };
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, exact kinematics", "The scatter angles by energy and by geometry agree", CSR.ComputeQualityFactor(Sequence), 0.0, 1E-12)) && Passed;

  // The reversed sequence does not fit
  vector<MRESE*> Reversed = { Exact->GetRESEAt(2), Exact->GetRESEAt(1), Exact->GetRESEAt(0) };
  Passed = EvaluateTrue("ComputeQualityFactor()", "three sites, reversed", "The reversed sequence has a worse quality factor", CSR.ComputeQualityFactor(Reversed) > CSR.ComputeQualityFactor(Sequence)) && Passed;
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, reversed", "The quality factor is the squared difference of the cosines of the scatter angle", CSR.ComputeQualityFactor(Reversed), GetTestStatistics(Reversed, false), 1E-12)) && Passed;

  // Three sites, which are no exact Compton sequence: 100, 200, 300 keV
  MRERawEvent* Inexact = CreateEvent(m_Geometry, Positions, { 100.0, 200.0, 300.0 });
  vector<MRESE*> InexactSequence = { Inexact->GetRESEAt(0), Inexact->GetRESEAt(1), Inexact->GetRESEAt(2) };
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, 100, 200, 300 keV", "The quality factor is the squared difference of the cosines of the scatter angle", CSR.ComputeQualityFactor(InexactSequence), GetTestStatistics(InexactSequence, false), 1E-12)) && Passed;
  // The energy of the first hit has no influence
  double Before = CSR.ComputeQualityFactor(InexactSequence);
  Inexact->GetRESEAt(0)->SetEnergy(555.0);
  Passed = EvaluateNear("ComputeQualityFactor()", "three sites, other first energy", "The energy of the first hit has no influence", CSR.ComputeQualityFactor(InexactSequence), Before, 0.0) && Passed;

  // Four sites: the average over the two central hits
  vector<MVector> FourPositions = GetFourSitePositions();
  MRERawEvent* Four = CreateEvent(m_Geometry, FourPositions, { 100.0, 60.0, 80.0, 300.0 });
  vector<MRESE*> FourSequence = { Four->GetRESEAt(0), Four->GetRESEAt(1), Four->GetRESEAt(2), Four->GetRESEAt(3) };
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "four sites", "The quality factor is the average squared difference of the cosines of the scatter angle", CSR.ComputeQualityFactor(FourSequence), GetTestStatistics(FourSequence, false), 1E-12)) && Passed;
  // The same average from the two scatters alone: the first one sees a photon of 80 + 300 keV leaving the second hit
  MRERawEvent* FirstScatter = CreateEvent(m_Geometry, { FourPositions[0], FourPositions[1], FourPositions[2] }, { 100.0, 60.0, 380.0 });
  MRERawEvent* SecondScatter = CreateEvent(m_Geometry, { FourPositions[1], FourPositions[2], FourPositions[3] }, { 60.0, 80.0, 300.0 });
  vector<MRESE*> FirstSequence = { FirstScatter->GetRESEAt(0), FirstScatter->GetRESEAt(1), FirstScatter->GetRESEAt(2) };
  vector<MRESE*> SecondSequence = { SecondScatter->GetRESEAt(0), SecondScatter->GetRESEAt(1), SecondScatter->GetRESEAt(2) };
  Passed = EvaluateNear("ComputeQualityFactor()", "four sites", "The quality factor is the average of the quality factors of its two scatters", CSR.ComputeQualityFactor(FourSequence), 0.5*(CSR.ComputeQualityFactor(FirstSequence) + CSR.ComputeQualityFactor(SecondSequence)), 1E-15) && Passed;

  // The test statistics with errors
  CSR.m_TypeTestStatistics = MERCSRChiSquare::c_TSChiSquare;
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, chi-square", "The quality factor is the squared difference of the cosines divided by the squared errors", CSR.ComputeQualityFactor(InexactSequence), GetTestStatistics(InexactSequence, true), 1E-9)) && Passed;
  CSR.m_TypeTestStatistics = MERCSRChiSquare::c_TSSimpleWithErrors;
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("ComputeQualityFactor()", "three sites, simple with errors", "The quality factor is the squared difference of the cosines divided by the squared errors", CSR.ComputeQualityFactor(InexactSequence), GetTestStatistics(InexactSequence, true), 1E-9)) && Passed;
  CSR.m_TypeTestStatistics = MERCSRChiSquare::c_TSSimple;

  // Two hits and one hit have no central hit
  vector<MRESE*> Two = { Exact->GetRESEAt(0), Exact->GetRESEAt(1) };
  Passed = EvaluateNear("ComputeQualityFactor()", "two hits", "The sequence fails", CSR.ComputeQualityFactor(Two), MERCSR::c_CSRFailed, 0.0) && Passed;
  vector<MRESE*> One = { Exact->GetRESEAt(0) };
  Passed = EvaluateNear("ComputeQualityFactor()", "one hit", "The sequence fails", CSR.ComputeQualityFactor(One), MERCSR::c_CSRFailed, 0.0) && Passed;

  // A last hit without energy
  Exact->GetRESEAt(2)->SetEnergy(0.0);
  DisableDefaultStreams();
  double NoEnergy = CSR.ComputeQualityFactor(Sequence);
  EnableDefaultStreams();
  Passed = EvaluateNear("ComputeQualityFactor()", "last hit without energy", "The sequence fails", NoEnergy, MERCSR::c_CSRFailed, 0.0) && Passed;
  Exact->GetRESEAt(2)->SetEnergy(Energies[2]);

  // The required start in the tracker: the third hit is in the calorimeter
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true);
  Passed = EvaluateNear("ComputeQualityFactor()", "start in the calorimeter, tracker required", "The sequence fails", CSR.ComputeQualityFactor(Reversed), MERCSR::c_CSRFailed, 0.0) && Passed;
  Passed = EvaluateTrue("ComputeQualityFactor()", "start in the tracker, tracker required", "The sequence does not fail", CSR.ComputeQualityFactor(Sequence) < 1.0) && Passed;
  // Detector type 5 is a tracker, too
  Exact->GetRESEAt(2)->SetDetector(5);
  Passed = EvaluateTrue("ComputeQualityFactor()", "start in detector type 5, tracker required", "The sequence does not fail", CSR.ComputeQualityFactor(Reversed) < 100.0) && Passed;

  delete Exact;
  delete Inexact;
  delete Four;
  delete FirstScatter;
  delete SecondScatter;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the limit of the cosine of the Compton scatter angle
bool UTERCSRChiSquare::TestCosineLimit()
{
  bool Passed = true;

  UTERCSRChiSquareAccess CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false);

  // A central deposit of 1000 keV and a last deposit of 128 keV: cos phi = 1 - 511/128 + 511/1128 = -2.54 is not possible
  // The sequence only fails, if cos phi is more than 100.5 sigma below -1, i.e. for sigma < 1.54/100.5
  // sigma = (E0/Eg^2 - E0/(Ee + Eg)^2) * dEg for an ideal central hit
  MRERawEvent* Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 1000.0, 128.0 });
  vector<MRESE*> Sequence = { Event->GetRESEAt(0), Event->GetRESEAt(1), Event->GetRESEAt(2) };
  double CosPhi = MComptonEvent::ComputeCosPhiViaEeEg(1000.0, 128.0);
  double Derivative = c_E0/(128.0*128.0) - c_E0/(1128.0*1128.0);
  double LimitResolution = (-1.0 - CosPhi)/100.5/Derivative;

  Event->GetRESEAt(1)->SetEnergyResolution(0.0);
  Event->GetRESEAt(2)->SetEnergyResolution(1.01*LimitResolution);
  Passed = EvaluateTrue("ComputeQualityFactor()", "cos phi 99.5 sigma below -1", "The sequence does not fail", CSR.ComputeQualityFactor(Sequence) < MERCSR::c_CSRFailed) && Passed;

  Event->GetRESEAt(2)->SetEnergyResolution(0.99*LimitResolution);
  Passed = EvaluateNear("ComputeQualityFactor()", "cos phi 101.5 sigma below -1", "The sequence fails", CSR.ComputeQualityFactor(Sequence), MERCSR::c_CSRFailed, 0.0) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search for the sequence of events with three and more hits
bool UTERCSRChiSquare::TestSequences()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, false);

  // Three sites with exact kinematics, stored in the order 2, 0, 1
  vector<MVector> Positions = GetThreeSitePositions();
  vector<double> Energies = GetComptonEnergies(Positions, 150.0, 300.0);
  MRERawEvent* Event = CreateEvent(m_Geometry, { Positions[2], Positions[0], Positions[1] }, { Energies[2], Energies[0], Energies[1] });
  vector<int> Stored = GetStoredIDs(Event);
  List.SetInitialRawEvent(Event);
  Passed = EvaluateTrue("Analyze()", "three sites, exact kinematics", "The analysis returns true", CSR.Analyze(&List)) && Passed;
  vector<int> Sequence = GetSequenceIDs(Event);
  Passed = EvaluateTrue("Analyze()", "three sites, exact kinematics", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "three sites, exact kinematics", "The event is a Compton event", Event->GetEventType(), MRERawEvent::c_ComptonEvent) && Passed;
  Passed = EvaluateSize("Analyze()", "three sites, exact kinematics", "The sequence has three hits", Sequence.size(), 3) && Passed;
  if (Sequence.size() == 3) {
    Passed = EvaluateTrue("Analyze()", "three sites, exact kinematics", "The sequence is the true one", Sequence[0] == Stored[1] && Sequence[1] == Stored[2] && Sequence[2] == Stored[0]) && Passed;
  }
  // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
  Passed = ExpectedToFail(EvaluateNear("Analyze()", "three sites, exact kinematics", "The first quality factor is zero", Event->GetComptonQualityFactor1(), 0.0, 1E-12)) && Passed;
  Passed = EvaluateTrue("Analyze()", "three sites, exact kinematics", "The second quality factor is worse than the first", Event->GetComptonQualityFactor2() > Event->GetComptonQualityFactor1()) && Passed;
  Passed = EvaluateNear("Analyze()", "three sites, exact kinematics", "No energy is added to the event", Event->GetEnergy(), Energies[0] + Energies[1] + Energies[2], 1E-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "three sites, exact kinematics", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;

  // Four sites with exact kinematics, stored in the order 3, 1, 0, 2
  Positions = GetFourSitePositions();
  Energies = GetComptonEnergies(Positions, 100.0, 200.0);
  Passed = EvaluateSize("GetComptonEnergies()", "four sites", "The energies of the sequence exist", Energies.size(), 4) && Passed;
  if (Energies.size() == 4) {
    Event = CreateEvent(m_Geometry, { Positions[3], Positions[1], Positions[0], Positions[2] }, { Energies[3], Energies[1], Energies[0], Energies[2] });
    Stored = GetStoredIDs(Event);
    List.SetInitialRawEvent(Event);
    CSR.Analyze(&List);
    Sequence = GetSequenceIDs(Event);
    Passed = EvaluateTrue("Analyze()", "four sites, exact kinematics", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateSize("Analyze()", "four sites, exact kinematics", "The sequence has four hits", Sequence.size(), 4) && Passed;
    if (Sequence.size() == 4) {
      Passed = EvaluateTrue("Analyze()", "four sites, exact kinematics", "The sequence is the true one", Sequence[0] == Stored[2] && Sequence[1] == Stored[1] && Sequence[2] == Stored[3] && Sequence[3] == Stored[0]) && Passed;
    }
    // Defect REV-G-4: The quality factor uses an electron rest mass of 511.044 keV, and not the 510.999 keV of MEGAlib
    Passed = ExpectedToFail(EvaluateNear("Analyze()", "four sites, exact kinematics", "The first quality factor is zero", Event->GetComptonQualityFactor1(), 0.0, 1E-12)) && Passed;

    // The start in the tracker is required: the true sequence starts there
    CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true);
    Event = CreateEvent(m_Geometry, { Positions[3], Positions[1], Positions[0], Positions[2] }, { Energies[3], Energies[1], Energies[0], Energies[2] });
    Stored = GetStoredIDs(Event);
    List.SetInitialRawEvent(Event);
    CSR.Analyze(&List);
    Passed = EvaluateTrue("Analyze()", "four sites, start in tracker required", "The event is good", Event->IsGoodEvent()) && Passed;
    Passed = EvaluateTrue("Analyze()", "four sites, start in tracker required", "The start point is the true first hit", Event->GetStartPoint() == Event->GetRESEAt(2)) && Passed;
  }

  // The start in the tracker is required, but there is no hit in the tracker
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 5, true);
  Event = CreateEvent(m_Geometry, { MVector(1.0, 1.0, -8.0), MVector(30.2, 0.3, 0.5), MVector(-2.0, 0.0, -12.0) }, { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "three sites without tracker hit, start in tracker required", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "three sites without tracker hit, start in tracker required", "The event is rejected since there is no good combination", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRNoGoodCombination) && Passed;
  Passed = EvaluateFalse("Analyze()", "three sites without tracker hit, start in tracker required", "There is no optimum event", List.HasOptimumEvent()) && Passed;

  List.DeleteAll();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the events which are no sequences and the limits of the quality factor
bool UTERCSRChiSquare::TestSpecialEventsAndLimits()
{
  bool Passed = true;

  MRawEventIncarnations List;
  MERCSRChiSquare CSR;
  CSR.SetParameters(&m_Geometry, 0.0, 1000.0, 3, false);

  // No hits
  MRERawEvent* Event = new MRERawEvent(&m_Geometry);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "no hits", "The event is rejected since it has no hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionNoHits) && Passed;

  // One hit is a photo event
  Event = CreateEvent(m_Geometry, { MVector(0.0, 0.0, -10.0) }, { 662.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one hit", "The event is a photo event", Event->GetEventType(), MRERawEvent::c_PhotoEvent) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "one hit", "The event is the optimum event", List.GetOptimumEvent() == Event) && Passed;

  // One track is rejected
  MRETrack* Track = new MRETrack();
  Track->AddRESE(CreateHit(MVector(0.0, 0.0, 3.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Track->AddRESE(CreateHit(MVector(0.0, 0.0, 2.0), 100.0, MDDetector::c_Strip2D, &m_Geometry));
  Event = new MRERawEvent(&m_Geometry);
  Event->AddRESE(Track);
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one track", "The event is rejected since it is only one track", Event->GetRejectionReason(), MRERawEvent::c_RejectionOneTrackOnly) && Passed;

  // Four hits are too many
  Event = CreateEvent(m_Geometry, GetFourSitePositions(), { 100.0, 60.0, 80.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = Evaluate("Analyze()", "one hit more than allowed", "The event is rejected since it has too many hits", Event->GetRejectionReason(), MRERawEvent::c_RejectionTooManyHitsCSR) && Passed;
  Passed = EvaluateNear("Analyze()", "one hit more than allowed", "The first quality factor is the largest double", Event->GetComptonQualityFactor1(), DBL_MAX, 0.0) && Passed;

  // The limits of the quality factor: the event is analyzed without limits first, then with limits just around its quality factor
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  double QualityFactor = Event->GetComptonQualityFactor1();
  Passed = EvaluateTrue("Analyze()", "three sites, 100, 200, 300 keV", "The event is good", Event->IsGoodEvent()) && Passed;
  Passed = EvaluateTrue("Analyze()", "three sites, 100, 200, 300 keV", "The quality factor is not zero", QualityFactor > 1E-6) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, QualityFactor*(1.0 + 1E-9), 3, false);
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "just below the maximum", "The event is good", Event->IsGoodEvent()) && Passed;

  CSR.SetParameters(&m_Geometry, 0.0, QualityFactor*(1.0 - 1E-9), 3, false);
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "just above the maximum", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "just above the maximum", "The event is rejected by the limits of the quality factor", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRThreshold) && Passed;

  CSR.SetParameters(&m_Geometry, QualityFactor*(1.0 - 1E-9), 1000.0, 3, false);
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateTrue("Analyze()", "just above the minimum", "The event is good", Event->IsGoodEvent()) && Passed;

  CSR.SetParameters(&m_Geometry, QualityFactor*(1.0 + 1E-9), 1000.0, 3, false);
  Event = CreateEvent(m_Geometry, GetThreeSitePositions(), { 100.0, 200.0, 300.0 });
  List.SetInitialRawEvent(Event);
  CSR.Analyze(&List);
  Passed = EvaluateFalse("Analyze()", "just below the minimum", "The event is not good", Event->IsGoodEvent()) && Passed;
  Passed = Evaluate("Analyze()", "just below the minimum", "The event is rejected by the limits of the quality factor", Event->GetRejectionReason(), MRERawEvent::c_RejectionCSRThreshold) && Passed;

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

  UTERCSRChiSquare Test;
  return Test.Run() == true ? 0 : 1;
}


// UTERCSRChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
