/*
 * ETCosimaToMimrecAngularResolution.cxx
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

#include "ETCosimaToMimrecAngularResolution.h"

// Standard libs:
#include <cmath>
#include <future>
#include <iostream>
#include <map>

// ROOT libs:
#include "TGraphErrors.h"
#include "TLegend.h"


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecAngularResolution::ETCosimaToMimrecAngularResolution() : MEndToEndTest("ETCosimaToMimrecAngularResolution")
{
  LoadReferences();
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecAngularResolution::RunTest()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  struct Case { double m_Energy; double m_Theta; };
  const vector<Case> Cases = { { 200, 0 }, { 200, 40 }, { 500, 0 }, { 500, 40 }, { 1000, 0 }, { 1000, 40 } };
  const double Flux = 2.0;
  const double Time = 600.0;

  vector<ETSimScenario> Scenarios;
  for (const Case& C: Cases) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Line") + static_cast<int>(C.m_Energy) + "Theta" + static_cast<int>(C.m_Theta);
    Scenario.m_Time = Time;
    Scenario.SetFarFieldPointSource(C.m_Theta, 0, MString("Mono ") + C.m_Energy, Flux);
    Scenarios.push_back(Scenario);
  }
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;

  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    Runs.push_back(async(launch::async, [this, Scenario, Common]() {
      return SimulateAndReconstruct(Scenario, Common + "/revan.cfg");
    }));
  }

  // The summary: the ARM distributions, and the widths over the energy for the two directions
  TPad* ARMPage = GetReport().AddPage("ARM of the full energy Compton events: 200, 500, 1000 keV, 0 and 40 deg", 2, 3);
  TGraphErrors* FwhmGraph[2];
  TGraphErrors* ContainmentGraph[2];
  TGraphErrors* ReferenceFwhmGraph[2];
  TGraphErrors* ReferenceContainmentGraph[2];
  for (unsigned int d = 0; d < 2; ++d) {
    FwhmGraph[d] = new TGraphErrors();
    ContainmentGraph[d] = new TGraphErrors();
    ReferenceFwhmGraph[d] = new TGraphErrors();
    ReferenceContainmentGraph[d] = new TGraphErrors();
  }

  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const MString Name = Scenarios[s].m_Name;
    const double Line = Cases[s].m_Energy;
    Passed = EvaluateTrue("Simulate()", Name, "Cosima and revan run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadTra()", Name, "The sim and tra files can be read", Sim.m_FileSuccessfullyRead == true && Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false || Tra.m_FileSuccessfullyRead == false) {
      continue;
    }

    MVector Source;
    Source.SetMagThetaPhi(1.0, Cases[s].m_Theta*c_Rad, 0.0);

    // The ARM of the Compton events of the full energy peak with a physical scatter angle
    vector<double> ARMs;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      const double Total = Event->Eg() + Event->Ee();
      if (fabs(Total - Line) < 0.03*Line && Tra.HasPhysicalScatterAngle(*Event) == true) {
        ARMs.push_back(ARM(*Event, Source));
      }
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers, "<<ARMs.size()<<" Compton events in the peak"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", events", "There are enough Compton events in the peak (> 500)", ARMs.size() > 500) && Passed;
    if (ARMs.size() <= 500) {
      continue;
    }

    ETMeasuredValue Area;
    Area.m_Value = ARMs.size()/(Flux*Time);
    Area.m_Error = sqrt(static_cast<double>(ARMs.size()))/(Flux*Time);
    Passed = CompareToReference(Name, "Compton effective area / cm2", "The effective area of the Compton events in the peak", Area) && Passed;

    // The ARM distribution has asymmetric wings: the containments and the core are measured relative to its peak (the half-sample mode, found
    // on every resample of the bootstrap), not to zero, so that an offset of the peak does not widen them - the offset is an observable of its own
    MStatisticFunction Peak = [](const vector<double>& Values) { return MStatistics::GetPeakPosition(Values); };
    MStatisticFunction Fwhm = [](const vector<double>& Values) { return MStatistics::GetFullWidthAtHalfMaximum(Values, 0.5); };
    MStatisticFunction Containment68 = [](const vector<double>& Values) { return MStatistics::GetContainmentAroundPeak(Values, 0.68); };
    MStatisticFunction Containment90 = [](const vector<double>& Values) { return MStatistics::GetContainmentAroundPeak(Values, 0.90); };
    MStatisticFunction CoreFraction = [](const vector<double>& Values) { return MStatistics::GetFractionAroundPeak(Values, 5.0); };
    const ETMeasuredValue ARMPeak = Measure(ARMs, Peak, 50);
    const ETMeasuredValue ARMFwhm = Measure(ARMs, Fwhm, 50);
    mout<<Name<<": ARM peak at "<<ARMPeak.m_Value<<" +- "<<ARMPeak.m_Error<<" deg, FWHM "<<ARMFwhm.m_Value<<" +- "<<ARMFwhm.m_Error<<" deg"<<endl;
    Passed = CompareToReference(Name, "ARM peak / deg", "The position of the peak of the ARM distribution (the half-sample mode)", ARMPeak) && Passed;
    Passed = CompareToReference(Name, "ARM FWHM / deg", "The full width at half maximum of the ARM distribution (0.5 degree bins, three bin smoothing)", ARMFwhm) && Passed;
    const ETMeasuredValue ARM68 = Measure(ARMs, Containment68, 50);
    Passed = CompareToReference(Name, "ARM 68% containment / deg", "The 68% containment of |ARM - peak|", ARM68) && Passed;
    ARMPage->cd(s + 1);
    GetReport().DrawHistogram(Name + ";ARM / deg;events", ARMs, 80, -40.0, 40.0);
    GetReport().DrawComparison(0, "FWHM", ARMFwhm, GetReference(Name, "ARM FWHM / deg"));
    GetReport().DrawComparison(1, "68%", ARM68, GetReference(Name, "ARM 68% containment / deg"));
    const unsigned int Direction = s % 2;
    const ETMeasuredValue ReferenceFwhm = GetReference(Name, "ARM FWHM / deg");
    const ETMeasuredValue ReferenceContainment = GetReference(Name, "ARM 68% containment / deg");
    ReferenceFwhmGraph[Direction]->SetPoint(ReferenceFwhmGraph[Direction]->GetN(), Line, ReferenceFwhm.m_Value);
    ReferenceFwhmGraph[Direction]->SetPointError(ReferenceFwhmGraph[Direction]->GetN() - 1, 0.0, ReferenceFwhm.m_Error);
    ReferenceContainmentGraph[Direction]->SetPoint(ReferenceContainmentGraph[Direction]->GetN(), Line, ReferenceContainment.m_Value);
    ReferenceContainmentGraph[Direction]->SetPointError(ReferenceContainmentGraph[Direction]->GetN() - 1, 0.0, ReferenceContainment.m_Error);
    FwhmGraph[Direction]->SetPoint(FwhmGraph[Direction]->GetN(), Line, ARMFwhm.m_Value);
    FwhmGraph[Direction]->SetPointError(FwhmGraph[Direction]->GetN() - 1, 0.0, ARMFwhm.m_Error);
    ContainmentGraph[Direction]->SetPoint(ContainmentGraph[Direction]->GetN(), Line, ARM68.m_Value);
    ContainmentGraph[Direction]->SetPointError(ContainmentGraph[Direction]->GetN() - 1, 0.0, ARM68.m_Error);
    Passed = CompareToReference(Name, "ARM 90% containment / deg", "The 90% containment of |ARM - peak|", Measure(ARMs, Containment90, 50)) && Passed;
    Passed = CompareToReference(Name, "ARM core fraction (|ARM - peak| < 5 deg)", "The fraction of the events within 5 degrees of the peak", Measure(ARMs, CoreFraction, 50)) && Passed;
  }

  // The widths over the energy
  TPad* WidthPage = GetReport().AddPage("ARM FWHM and 68% containment: measured (filled) and reference (open), 0 deg (black), 40 deg (red)", 2, 1);
  for (unsigned int g = 0; g < 2; ++g) {
    WidthPage->cd(g + 1);
    gPad->SetGrid();
    gPad->SetLogx();
    for (unsigned int d = 0; d < 2; ++d) {
      TGraphErrors* Graph = FwhmGraph[d];
      if (g == 1) {
        Graph = ContainmentGraph[d];
      }
      Graph->SetMarkerStyle(20 + d);
      Graph->SetMarkerColor(1 + d);
      Graph->SetLineColor(1 + d);
      if (d == 0) {
        if (g == 0) {
          Graph->SetTitle(";line energy / keV;FWHM / deg");
        } else {
          Graph->SetTitle(";line energy / keV;68% containment / deg");
        }
        Graph->SetMinimum(0.0);
        Graph->Draw("AP");
      } else {
        Graph->Draw("P SAME");
      }
      TGraphErrors* Reference = ReferenceFwhmGraph[d];
      if (g == 1) {
        Reference = ReferenceContainmentGraph[d];
      }
      Reference->SetMarkerStyle(24 + d);
      Reference->SetMarkerSize(1.6);
      Reference->SetMarkerColor(1 + d);
      Reference->Draw("P SAME");
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("ETCosimaToMimrecAngularResolution", "end-to-end test: angular resolution") == false) {
    return 1;
  }

  ETCosimaToMimrecAngularResolution Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecAngularResolution.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
