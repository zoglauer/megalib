/*
 * ETCosimaToMimrecThresholds.cxx
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

#include "ETCosimaToMimrecThresholds.h"

// Standard libs:
#include <cmath>
#include <future>
#include <iostream>
#include <map>

// ROOT libs:
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLine.h"


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecThresholds::ETCosimaToMimrecThresholds() : MEndToEndTest("ETCosimaToMimrecThresholds")
{
  LoadReferences();
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecThresholds::RunTest()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // The trigger threshold of the active detector of Max.geo.setup is 30 keV, the energy resolution at 20 to 50 keV is 0.42 to 0.44 keV (sigma):
  // the lines 27 to 33 keV cover the turn-on
  const double TriggerThreshold = 30.0;
  const vector<double> Lines = { 20, 25, 27, 29, 31, 33, 35, 45, 60, 80, 120 };
  const double Flux = 2.0;
  const double Time = 300.0;

  vector<ETSimScenario> Scenarios;
  for (double Line: Lines) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Line") + static_cast<int>(Line);
    Scenario.m_Time = Time;
    Scenario.SetFarFieldPointSource(0, 0, MString("Mono ") + Line, Flux);
    Scenarios.push_back(Scenario);
  }
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;

  vector<future<bool>> Runs;
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const ETSimScenario Scenario = Scenarios[s];
    Runs.push_back(async(launch::async, [this, Scenario, Common]() {
      return SimulateAndReconstruct(Scenario, Common + "/revan.cfg");
    }));
  }

  // The summary: the areas, and the fractions over the line energy
  TGraphErrors* SimulatedAreas = new TGraphErrors();
  TGraphErrors* ReconstructedAreas = new TGraphErrors();
  TGraphErrors* PeakFractions = new TGraphErrors();
  TGraphErrors* LostFractions = new TGraphErrors();
  TGraphErrors* ReferenceSimulatedAreas = new TGraphErrors();
  TGraphErrors* ReferenceReconstructedAreas = new TGraphErrors();
  TGraphErrors* ReferencePeakFractions = new TGraphErrors();
  TGraphErrors* ReferenceLostFractions = new TGraphErrors();
  auto AddReference = [this](TGraphErrors* Graph, double X, const MString& Name, const MString& Observable) {
    const ETMeasuredValue Reference = GetReference(Name, Observable);
    Graph->SetPoint(Graph->GetN(), X, Reference.m_Value);
    Graph->SetPointError(Graph->GetN() - 1, 0.0, Reference.m_Error);
  };

  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const MString Name = Scenarios[s].m_Name;
    const double Line = Lines[s];
    Passed = EvaluateTrue("Simulate()", Name, "Cosima (and revan) run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    Passed = EvaluateTrue("ReadSim()", Name, "The sim file can be read", Sim.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false) {
      continue;
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers"<<endl;

    ETMeasuredValue TriggerArea;
    TriggerArea.m_Value = Sim.m_Events.size()/(Flux*Time);
    TriggerArea.m_Error = sqrt(static_cast<double>(Sim.m_Events.size()))/(Flux*Time);
    Passed = CompareToReference(Name, "simulated events area / cm2", "The events in the sim file per fluence", TriggerArea) && Passed;

    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadTra()", Name, "The tra file can be read", Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Tra.m_FileSuccessfullyRead == false) {
      continue;
    }

    ReconstructedAreas->SetPoint(ReconstructedAreas->GetN(), Line, Tra.m_Events.size()/(Flux*Time));
    ReconstructedAreas->SetPointError(ReconstructedAreas->GetN() - 1, 0.0, sqrt(static_cast<double>(Tra.m_Events.size()))/(Flux*Time));
    AddReference(ReferenceSimulatedAreas, Line, Name, "simulated events area / cm2");
    AddReference(ReferenceReconstructedAreas, Line, Name, "reconstructed area / cm2");
    SimulatedAreas->SetPoint(SimulatedAreas->GetN(), Line, TriggerArea.m_Value);
    SimulatedAreas->SetPointError(SimulatedAreas->GetN() - 1, 0.0, TriggerArea.m_Error);

    if (Line + 2.5 < TriggerThreshold) {
      // The line plus 2.5 keV (six sigma of the energy resolution) is below the trigger threshold: nothing can trigger
      Passed = EvaluateNear("Trigger", Name + ", no trigger", "A line which is more than 2.5 keV below the trigger threshold of the detector never triggers: no event is reconstructed", Tra.m_Events.size(), 0.0, 0.0) && Passed;
      continue;
    }

    // The total energies of all reconstructed Compton and photo events
    vector<double> All;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      All.push_back(Event->Eg() + Event->Ee());
    }
    for (const shared_ptr<MPhotoEvent>& Event: Tra.Photo()) {
      All.push_back(Event->GetEnergy());
    }
    mout<<Name<<": "<<All.size()<<" reconstructed events"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", events", "There are enough reconstructed events (> 100)", All.size() > 100) && Passed;
    if (All.size() <= 100) {
      continue;
    }

    ETMeasuredValue Reconstructed;
    Reconstructed.m_Value = All.size()/(Flux*Time);
    Reconstructed.m_Error = sqrt(static_cast<double>(All.size()))/(Flux*Time);
    Passed = CompareToReference(Name, "reconstructed area / cm2", "The reconstructed events per fluence", Reconstructed) && Passed;

    MStatisticFunction InPeak = [Line](const vector<double>& Values) {
      double Inside = 0.0;
      for (double V: Values) {
        if (fabs(V - Line) < 0.03*Line) {
          Inside += 1.0;
        }
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return Inside/Values.size();
    };
    // A hit of the K fluorescence of germanium (about 10 keV) which is below the noise threshold of 15 keV is lost
    MStatisticFunction LostFluorescence = [Line](const vector<double>& Values) {
      double Inside = 0.0;
      for (double V: Values) {
        if (fabs(V - (Line - 10.0)) < 2.0) {
          Inside += 1.0;
        }
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return Inside/Values.size();
    };
    MStatisticFunction Mean = [](const vector<double>& Values) {
      double Sum = 0.0;
      for (double V: Values) {
        Sum += V;
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return Sum/Values.size();
    };
    const ETMeasuredValue PeakFraction = Measure(All, InPeak, 50);
    const ETMeasuredValue LostFraction = Measure(All, LostFluorescence, 50);
    Passed = CompareToReference(Name, "peak fraction", "The fraction of the reconstructed events in the peak (3% around the line)", PeakFraction) && Passed;
    Passed = CompareToReference(Name, "lost fluorescence fraction", "The fraction of the reconstructed events 10 keV (+-2 keV) below the line: a germanium fluorescence photon was lost", LostFraction) && Passed;
    AddReference(ReferencePeakFractions, Line, Name, "peak fraction");
    AddReference(ReferenceLostFractions, Line, Name, "lost fluorescence fraction");
    PeakFractions->SetPoint(PeakFractions->GetN(), Line, PeakFraction.m_Value);
    PeakFractions->SetPointError(PeakFractions->GetN() - 1, 0.0, PeakFraction.m_Error);
    LostFractions->SetPoint(LostFractions->GetN(), Line, LostFraction.m_Value);
    LostFractions->SetPointError(LostFractions->GetN() - 1, 0.0, LostFraction.m_Error);
    Passed = CompareToReference(Name, "mean energy / keV", "The mean reconstructed energy", Measure(All, Mean, 50)) && Passed;
  }

  // The turn-on of the trigger at 30 keV
  TPad* TurnOnPage = GetReport().AddPage("Trigger turn-on: events per fluence, measured (filled) and reference (open), trigger threshold 30 keV (dashed)", 1, 1);
  TurnOnPage->cd(1);
  gPad->SetGrid();
  if (SimulatedAreas->GetN() == 0) {
    // No scenario could be evaluated, the report shows why the page is empty
    GetReport().DrawLabel(0.1, 0.5, "No scenario was evaluated: there are no events to show");
  }
  SimulatedAreas->SetTitle(";line energy / keV;events per fluence / cm^{2}");
  SimulatedAreas->SetMarkerStyle(20);
  SimulatedAreas->Draw("AP");
  SimulatedAreas->SetMinimum(0.0);
  ReferenceSimulatedAreas->SetMarkerStyle(24);
  ReferenceSimulatedAreas->SetMarkerSize(1.5);
  ReferenceSimulatedAreas->SetMarkerColor(kGray + 2);
  ReferenceSimulatedAreas->Draw("P SAME");
  ReferenceReconstructedAreas->SetMarkerStyle(25);
  ReferenceReconstructedAreas->SetMarkerSize(1.5);
  ReferenceReconstructedAreas->SetMarkerColor(kRed - 7);
  ReferenceReconstructedAreas->Draw("P SAME");
  ReconstructedAreas->SetMarkerStyle(21);
  ReconstructedAreas->SetMarkerColor(kRed);
  ReconstructedAreas->SetLineColor(kRed);
  ReconstructedAreas->Draw("P SAME");
  TLine* Threshold = new TLine(TriggerThreshold, 0.0, TriggerThreshold, GetMaximumY(SimulatedAreas));
  Threshold->SetLineStyle(2);
  Threshold->Draw();
  TLegend* TurnOnLegend = new TLegend(0.55, 0.2, 0.88, 0.32);
  TurnOnLegend->AddEntry(SimulatedAreas, "events in the sim file", "p");
  TurnOnLegend->AddEntry(ReferenceSimulatedAreas, "reference: sim file", "p");
  TurnOnLegend->AddEntry(ReferenceReconstructedAreas, "reference: reconstructed", "p");
  TurnOnLegend->AddEntry(ReconstructedAreas, "reconstructed events", "p");
  TurnOnLegend->Draw();

  // The fractions of the reconstructed events
  TPad* FractionPage = GetReport().AddPage("Fraction in the peak (left) and with a lost fluorescence photon (right): measured (filled), reference (open)", 2, 1);
  FractionPage->cd(1);
  gPad->SetGrid();
  if (PeakFractions->GetN() == 0) {
    GetReport().DrawLabel(0.1, 0.5, "No scenario has reconstructed events: there are no fractions to show");
  }
  PeakFractions->SetTitle(";line energy / keV;fraction in the peak");
  PeakFractions->SetMarkerStyle(20);
  PeakFractions->Draw("AP");
  ReferencePeakFractions->SetMarkerStyle(24);
  ReferencePeakFractions->SetMarkerSize(1.5);
  ReferencePeakFractions->SetMarkerColor(kGray + 2);
  ReferencePeakFractions->Draw("P SAME");
  FractionPage->cd(2);
  gPad->SetGrid();
  LostFractions->SetTitle(";line energy / keV;fraction 10 keV below the line");
  LostFractions->SetMarkerStyle(20);
  LostFractions->Draw("AP");
  ReferenceLostFractions->SetMarkerStyle(24);
  ReferenceLostFractions->SetMarkerSize(1.5);
  ReferenceLostFractions->SetMarkerColor(kGray + 2);
  ReferenceLostFractions->Draw("P SAME");

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("ETCosimaToMimrecThresholds", "end-to-end test: noise and trigger thresholds") == false) {
    return 1;
  }

  ETCosimaToMimrecThresholds Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecThresholds.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
