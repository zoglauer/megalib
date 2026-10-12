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

////////////////////////////////////////////////////////////////////////////////


//! The reference values of the detector effects engine MEGAlib 1-4: mean of independent runs and its standard error
static const ETReferences c_References = {
  { "Line100/peak effective area / cm2", { 48.3422, 0.101 } },  // n=12, scatter of runs 0.349, mean bootstrap error 0.284, ratio 1.23
  { "Line100/peak fraction", { 0.872851, 0.000611 } },  // n=12, scatter of runs 0.00211, mean bootstrap error 0.00174, ratio 1.22
  { "Line100/peak sigma / keV", { 0.472844, 0.000887 } },  // n=12, scatter of runs 0.00307, mean bootstrap error 0.00345, ratio 0.89
  { "Line100/peak FWHM / keV", { 1.14851, 0.00249 } },  // n=12, scatter of runs 0.00864, mean bootstrap error 0.0101, ratio 0.85
  { "Line100/peak 90% half width / keV", { 0.803504, 0.000905 } },  // n=12, scatter of runs 0.00313, mean bootstrap error 0.00502, ratio 0.62
  { "Line100/single hit sigma / keV", { 0.453299, 0.001 } },  // n=12, scatter of runs 0.00347, mean bootstrap error 0.00339, ratio 1.02
  { "Line200/peak effective area / cm2", { 38.0422, 0.0486 } },  // n=12, scatter of runs 0.169, mean bootstrap error 0.252, ratio 0.67
  { "Line200/peak fraction", { 0.708611, 0.000549 } },  // n=12, scatter of runs 0.0019, mean bootstrap error 0.00259, ratio 0.73
  { "Line200/peak sigma / keV", { 0.605469, 0.0021 } },  // n=12, scatter of runs 0.00728, mean bootstrap error 0.00487, ratio 1.50
  { "Line200/peak FWHM / keV", { 1.35719, 0.0054 } },  // n=12, scatter of runs 0.0187, mean bootstrap error 0.0155, ratio 1.20
  { "Line200/peak 90% half width / keV", { 1.16865, 0.00461 } },  // n=12, scatter of runs 0.016, mean bootstrap error 0.0125, ratio 1.28
  { "Line200/single hit sigma / keV", { 0.492965, 0.00211 } },  // n=12, scatter of runs 0.0073, mean bootstrap error 0.00529, ratio 1.38
  { "Line500/peak effective area / cm2", { 15.0568, 0.0401 } },  // n=12, scatter of runs 0.139, mean bootstrap error 0.158, ratio 0.88
  { "Line500/peak fraction", { 0.359892, 0.000871 } },  // n=12, scatter of runs 0.00302, mean bootstrap error 0.00308, ratio 0.98
  { "Line500/peak sigma / keV", { 0.95687, 0.00476 } },  // n=12, scatter of runs 0.0165, mean bootstrap error 0.014, ratio 1.18
  { "Line500/peak FWHM / keV", { 1.82719, 0.01 } },  // n=12, scatter of runs 0.0348, mean bootstrap error 0.0394, ratio 0.88
  { "Line500/peak 90% half width / keV", { 4.37644, 0.043 } },  // n=12, scatter of runs 0.149, mean bootstrap error 0.161, ratio 0.93
  { "Line500/single hit sigma / keV", { 0.587851, 0.00436 } },  // n=12, scatter of runs 0.0151, mean bootstrap error 0.017, ratio 0.89
  { "Line1000/peak effective area / cm2", { 8.66222, 0.0313 } },  // n=12, scatter of runs 0.108, mean bootstrap error 0.12, ratio 0.90
  { "Line1000/peak fraction", { 0.232554, 0.000799 } },  // n=12, scatter of runs 0.00277, mean bootstrap error 0.00267, ratio 1.04
  { "Line1000/peak sigma / keV", { 1.24485, 0.00987 } },  // n=12, scatter of runs 0.0342, mean bootstrap error 0.0243, ratio 1.41
  { "Line1000/peak FWHM / keV", { 2.28552, 0.0251 } },  // n=12, scatter of runs 0.0871, mean bootstrap error 0.0657, ratio 1.33
  { "Line1000/peak 90% half width / keV", { 5.9343, 0.0532 } },  // n=12, scatter of runs 0.184, mean bootstrap error 0.146, ratio 1.26
  { "Line1000/single hit sigma / keV", { 0.729625, 0.00863 } },  // n=12, scatter of runs 0.0299, mean bootstrap error 0.0401, ratio 0.75
  { "Line2000/peak effective area / cm2", { 5.08219, 0.0143 } },  // n=12, scatter of runs 0.0495, mean bootstrap error 0.0412, ratio 1.20
  { "Line2000/peak fraction", { 0.161584, 0.000426 } },  // n=12, scatter of runs 0.00148, mean bootstrap error 0.00124, ratio 1.19
  { "Line2000/peak sigma / keV", { 1.58176, 0.00403 } },  // n=12, scatter of runs 0.014, mean bootstrap error 0.0185, ratio 0.75
  { "Line2000/peak FWHM / keV", { 2.86591, 0.0122 } },  // n=12, scatter of runs 0.0423, mean bootstrap error 0.0453, ratio 0.93
  { "Line2000/peak 90% half width / keV", { 6.79418, 0.0216 } },  // n=12, scatter of runs 0.0747, mean bootstrap error 0.0781, ratio 0.96
  { "Line2000/single hit sigma / keV", { 0.940215, 0.0146 } },  // n=12, scatter of runs 0.0505, mean bootstrap error 0.0529, ratio 0.95
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecEnergyResolution::ETCosimaToMimrecEnergyResolution() : MEndToEndTest("ETCosimaToMimrecEnergyResolution")
{
  SetReferences(c_References);
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecEnergyResolution::Run()
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
    ETStatistic PeakFraction = [Line, Window](const vector<double>& Values) {
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
    ETStatistic RobustWidth = [](const vector<double>& Values) { return GetRobustSigma(Values); };
    ETStatistic Fwhm = [Sigma](const vector<double>& Values) { return FullWidthAtHalfMaximum(Values, 0.4*Sigma); };
    ETStatistic Width90 = [](const vector<double>& Values) { return GetHalfWidthContaining(Values, 0.9); };
    const ETMeasuredValue PeakSigma = Measure(Peak, RobustWidth);
    mout<<Name<<": peak sigma "<<PeakSigma.m_Value<<" +- "<<PeakSigma.m_Error<<" keV (resolution table: "<<Sigma<<" keV)"<<endl;
    Passed = CompareToReference(Name, "peak sigma / keV", "The robust sigma of the peak", PeakSigma) && Passed;
    Passed = CompareToReference(Name, "peak FWHM / keV", "The full width at half maximum of the peak", Measure(Peak, Fwhm, 50)) && Passed;
    Passed = CompareToReference(Name, "peak 90% half width / keV", "The half width of the shortest interval which contains 90% of the events of the peak window", Measure(Peak, Width90, 50)) && Passed;

    // The single interaction events are measured in one hit: their width is the resolution of the table
    Passed = EvaluateTrue("ReadTra()", Name + ", single hit events", "There are enough photo events with a single hit and the full energy in the peak to test the resolution (> 200)", PhotoPeak.size() > 200) && Passed;
    if (PhotoPeak.size() > 200) {
      const ETMeasuredValue PhotoSigma = Measure(PhotoPeak, RobustWidth);
      mout<<Name<<": single hit full energy photo events: sigma "<<PhotoSigma.m_Value<<" +- "<<PhotoSigma.m_Error<<" keV (resolution table: "<<Sigma<<" keV)"<<endl;
      Passed = EvaluateNear("Energy", Name + ", resolution", "The robust sigma of the photo events with a single hit and the full energy is the energy resolution of the geometry (four standard errors)", PhotoSigma.m_Value, Sigma, 4.0*PhotoSigma.m_Error) && Passed;
      Passed = CompareToReference(Name, "single hit sigma / keV", "The robust sigma of the photo events with a single hit and the full energy", PhotoSigma) && Passed;
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecEnergyResolution Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecEnergyResolution.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
