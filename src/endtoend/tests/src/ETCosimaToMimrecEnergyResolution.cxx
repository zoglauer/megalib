/*
 * ETCosimaToMimrecEnergyResolution.cxx
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

#include "ETCosimaToMimrecEnergyResolution.h"

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
ETCosimaToMimrecEnergyResolution::ETCosimaToMimrecEnergyResolution() : MEndToEndTest("ETCosimaToMimrecEnergyResolution")
{
  LoadReferences();
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecEnergyResolution::RunTest()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // The energy resolution (sigma in keV) of the active detector of Max.geo.setup at the energies of its table
  // The simulated time is longer for 2000 keV: only 85 single hit full energy photo events per 300 s
  struct Line { double m_Energy; double m_Sigma; double m_Time; };
  const vector<Line> Lines = { { 100, 0.454, 300 }, { 200, 0.492, 300 }, { 500, 0.5875, 300 }, { 1000, 0.7217, 300 }, { 2000, 0.9317, 1500 } };
  const double Flux = 2.0;

  vector<ETSimScenario> Scenarios;
  for (const Line& L: Lines) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Line") + static_cast<int>(L.m_Energy);
    Scenario.m_Time = L.m_Time;
    Scenario.SetFarFieldPointSource(20, 0, MString("Mono ") + L.m_Energy, Flux);
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

  // The summary: the peaks, and the widths over the energy
  TPad* PeakPage = GetReport().AddPage("Energy peaks: all events (black) and single hit events with the full energy (red)", 3, 2);
  TGraphErrors* PeakWidths = new TGraphErrors();
  TGraphErrors* SingleWidths = new TGraphErrors();
  TGraphErrors* ReferenceWidths = new TGraphErrors();
  TGraph* TableWidths = new TGraph();

  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const MString Name = Scenarios[s].m_Name;
    const double Line = Lines[s].m_Energy;
    const double Sigma = Lines[s].m_Sigma;
    const double Time = Lines[s].m_Time;
    Passed = EvaluateTrue("Simulate()", Name, "Cosima and revan run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadTra()", Name, "The sim and tra files can be read", Sim.m_FileSuccessfullyRead == true && Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false || Tra.m_FileSuccessfullyRead == false) {
      continue;
    }

    // The number of hits and the energy deposit before the noising of every simulated event
    map<int, unsigned int> NumberOfHits;
    map<int, double> DepositBeforeNoising;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      NumberOfHits[Event->GetID()] = Event->GetNHTs();
      DepositBeforeNoising[Event->GetID()] = Event->GetTotalEnergyDepositBeforeNoising();
    }

    // The total energy of all reconstructed events (Compton and photo) and of the photo events with a single hit which absorbed the whole energy
    vector<double> All;
    vector<double> Peak;
    vector<double> PhotoPeak;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      const double Total = Event->Eg() + Event->Ee();
      if (Total > 0.1*Line) {
        All.push_back(Total);
      }
      if (fabs(Total - Line) < 0.03*Line) {
        Peak.push_back(Total);
      }
    }
    for (const shared_ptr<MPhotoEvent>& Event: Tra.Photo()) {
      const double Total = Event->GetEnergy();
      if (Total > 0.1*Line) {
        All.push_back(Total);
      }
      if (fabs(Total - Line) < 0.03*Line) {
        Peak.push_back(Total);
        auto Found = NumberOfHits.find(Event->GetId());
        if (Found != NumberOfHits.end() && Found->second == 1 && fabs(DepositBeforeNoising[Event->GetId()] - Line) < 0.01) {
          PhotoPeak.push_back(Total);
        }
      }
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers, "<<All.size()<<" reconstructed events, "<<Peak.size()<<" in the peak, "<<PhotoPeak.size()<<" photo events with a single hit and the full energy in the peak"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", peak events", "There are enough events in the peak (> 200)", Peak.size() > 200) && Passed;
    if (Peak.size() <= 200) {
      continue;
    }

    // Effective area in cm2 (Poisson uncertainty)
    ETMeasuredValue Area;
    Area.m_Value = Peak.size()/(Flux*Time);
    Area.m_Error = sqrt(static_cast<double>(Peak.size()))/(Flux*Time);
    Passed = CompareToReference(Name, "peak effective area / cm2", "The effective area of the peak (events in the peak per fluence)", Area) && Passed;

    // The fraction of the events in the peak
    const double Window = 0.03*Line;
    MStatisticFunction PeakFraction = [Line, Window](const vector<double>& Values) {
      double Inside = 0.0;
      for (double V: Values) {
        if (fabs(V - Line) < Window) {
          Inside += 1.0;
        }
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return Inside/Values.size();
    };
    Passed = CompareToReference(Name, "peak fraction", "The fraction of all reconstructed events (above 10% of the line energy) which is in the peak", Measure(All, PeakFraction, 50)) && Passed;

    // The width of the peak
    MStatisticFunction RobustWidth = [](const vector<double>& Values) { return MStatistics::GetRobustSigma(Values); };
    MStatisticFunction Fwhm = [Sigma](const vector<double>& Values) { return MStatistics::GetFullWidthAtHalfMaximum(Values, 0.4*Sigma); };
    MStatisticFunction Width90 = [](const vector<double>& Values) { return MStatistics::GetHalfWidthContaining(Values, 0.9); };
    const ETMeasuredValue PeakSigma = Measure(Peak, RobustWidth);
    mout<<Name<<": peak sigma "<<PeakSigma.m_Value<<" +- "<<PeakSigma.m_Error<<" keV (resolution table: "<<Sigma<<" keV)"<<endl;
    Passed = CompareToReference(Name, "peak sigma / keV", "The robust sigma of the peak", PeakSigma) && Passed;
    PeakPage->cd(s + 1);
    GetReport().DrawHistogram(Name + ";energy / keV;events", Peak, 60, 0.97*Line, 1.03*Line);
    TH1D* SingleHistogram = GetReport().DrawHistogram("", PhotoPeak, 60, 0.97*Line, 1.03*Line, "HIST SAME");
    SingleHistogram->SetLineColor(kRed);
    GetReport().DrawComparison(0, "sigma", PeakSigma, GetReference(Name, "peak sigma / keV"));
    ReferenceWidths->SetPoint(ReferenceWidths->GetN(), Line, GetReference(Name, "peak sigma / keV").m_Value);
    ReferenceWidths->SetPointError(ReferenceWidths->GetN() - 1, 0.0, GetReference(Name, "peak sigma / keV").m_Error);
    PeakWidths->SetPoint(PeakWidths->GetN(), Line, PeakSigma.m_Value);
    PeakWidths->SetPointError(PeakWidths->GetN() - 1, 0.0, PeakSigma.m_Error);
    TableWidths->SetPoint(TableWidths->GetN(), Line, Sigma);
    Passed = CompareToReference(Name, "peak FWHM / keV", "The full width at half maximum of the peak", Measure(Peak, Fwhm, 50)) && Passed;
    Passed = CompareToReference(Name, "peak 90% half width / keV", "The half width of the shortest interval which contains 90% of the events of the peak window", Measure(Peak, Width90, 50)) && Passed;

    // The single interaction events are measured in one hit: their width is the resolution of the table
    Passed = EvaluateTrue("ReadTra()", Name + ", single hit events", "There are enough photo events with a single hit and the full energy in the peak to test the resolution (> 200)", PhotoPeak.size() > 200) && Passed;
    if (PhotoPeak.size() > 200) {
      const ETMeasuredValue PhotoSigma = Measure(PhotoPeak, RobustWidth);
      mout<<Name<<": single hit full energy photo events: sigma "<<PhotoSigma.m_Value<<" +- "<<PhotoSigma.m_Error<<" keV (resolution table: "<<Sigma<<" keV)"<<endl;
      Passed = CompareToReference(Name, "single hit sigma / keV", "The robust sigma of the photo events with a single hit and the full energy", PhotoSigma) && Passed;
      // The table is exact: the tolerance is four standard errors of the measurement
      ETMeasuredValue TableValue;
      TableValue.m_Value = Sigma;
      const double TableTolerance = 4.0*PhotoSigma.m_Error;
      Passed = CompareValues("Energy", Name + ", resolution", "The robust sigma of the photo events with a single hit and the full energy is the energy resolution of the geometry (four standard errors)", Name + "/single hit sigma vs the table / keV", PhotoSigma, TableValue, TableTolerance, true) && Passed;
      PeakPage->cd(s + 1);
      GetReport().DrawComparison(1, "single hit sigma", PhotoSigma, TableValue, TableTolerance, true);
      SingleWidths->SetPoint(SingleWidths->GetN(), Line, PhotoSigma.m_Value);
      SingleWidths->SetPointError(SingleWidths->GetN() - 1, 0.0, PhotoSigma.m_Error);
    }
  }

  // The widths over the energy
  TPad* WidthPage = GetReport().AddPage("Energy resolution (robust sigma) over the energy: expected (table, reference) and measured", 1, 1);
  WidthPage->cd(1);
  if (TableWidths->GetN() == 0) {
    // No scenario could be evaluated, the report shows why the page is empty
    GetReport().DrawLabel(0.1, 0.5, "No scenario has enough events: there are no widths to show");
  } else {
    gPad->SetLogx();
    gPad->SetGrid();
    TableWidths->SetTitle(";line energy / keV;sigma / keV");
    TableWidths->SetLineColor(kRed);
    TableWidths->SetLineWidth(2);
    TableWidths->Draw("AL");
    TableWidths->SetMinimum(0.0);
    double MaximumWidth = GetMaximumY(PeakWidths);
    TableWidths->SetMaximum(1.2*max(MaximumWidth, GetMaximumY(TableWidths)));
    ReferenceWidths->SetMarkerStyle(24);
    ReferenceWidths->SetMarkerColor(kGray + 2);
    ReferenceWidths->SetLineColor(kGray + 2);
    ReferenceWidths->Draw("P SAME");
    PeakWidths->SetMarkerStyle(20);
    PeakWidths->Draw("P SAME");
    SingleWidths->SetMarkerStyle(21);
    SingleWidths->SetMarkerColor(kBlue);
    SingleWidths->SetLineColor(kBlue);
    SingleWidths->Draw("P SAME");
    TLegend* Legend = new TLegend(0.15, 0.66, 0.6, 0.88);
    Legend->AddEntry(TableWidths, "resolution table of the geometry", "l");
    Legend->AddEntry(ReferenceWidths, "reference of all events in the peak", "p");
    Legend->AddEntry(PeakWidths, "all events in the peak", "p");
    Legend->AddEntry(SingleWidths, "single hit events with the full energy", "p");
    Legend->Draw();
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("ETCosimaToMimrecEnergyResolution", "end-to-end test: energy resolution") == false) {
    return 1;
  }

  ETCosimaToMimrecEnergyResolution Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecEnergyResolution.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
