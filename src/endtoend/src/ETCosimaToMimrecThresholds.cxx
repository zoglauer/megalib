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

////////////////////////////////////////////////////////////////////////////////


//! The reference values of the detector effects engine MEGAlib 1-4: mean of independent runs and its standard error
static const ETReferences c_References = {
  { "Line20/simulated events area / cm2", { 22.3546, 0.0533 } },  // n=12, scatter of runs 0.185, mean bootstrap error 0.193, ratio 0.96
  { "Line25/simulated events area / cm2", { 34.4224, 0.0833 } },  // n=12, scatter of runs 0.289, mean bootstrap error 0.24, ratio 1.20
  { "Line27/simulated events area / cm2", { 37.7725, 0.066 } },  // n=12, scatter of runs 0.228, mean bootstrap error 0.251, ratio 0.91
  { "Line29/simulated events area / cm2", { 40.3853, 0.0633 } },  // n=12, scatter of runs 0.219, mean bootstrap error 0.259, ratio 0.85
  { "Line29/reconstructed area / cm2", { 0.359444, 0.00463 } },  // n=12, scatter of runs 0.016, mean bootstrap error 0.0245, ratio 0.65
  { "Line29/peak fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line29/lost fluorescence fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line29/mean energy / keV", { 30.1447, 0.00144 } },  // n=12, scatter of runs 0.00498, mean bootstrap error 0.00839, ratio 0.59
  { "Line31/simulated events area / cm2", { 42.5086, 0.0673 } },  // n=12, scatter of runs 0.233, mean bootstrap error 0.266, ratio 0.88
  { "Line31/reconstructed area / cm2", { 39.7368, 0.0666 } },  // n=12, scatter of runs 0.231, mean bootstrap error 0.257, ratio 0.90
  { "Line31/peak fraction", { 0.979385, 0.000216 } },  // n=12, scatter of runs 0.000749, mean bootstrap error 0.000931, ratio 0.80
  { "Line31/lost fluorescence fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line31/mean energy / keV", { 31.0061, 0.000518 } },  // n=12, scatter of runs 0.00179, mean bootstrap error 0.00259, ratio 0.69
  { "Line33/simulated events area / cm2", { 44.3745, 0.0772 } },  // n=12, scatter of runs 0.268, mean bootstrap error 0.272, ratio 0.98
  { "Line33/reconstructed area / cm2", { 42.3058, 0.0762 } },  // n=12, scatter of runs 0.264, mean bootstrap error 0.266, ratio 0.99
  { "Line33/peak fraction", { 0.970704, 0.000233 } },  // n=12, scatter of runs 0.000807, mean bootstrap error 0.00103, ratio 0.79
  { "Line33/lost fluorescence fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line33/mean energy / keV", { 32.9801, 0.000528 } },  // n=12, scatter of runs 0.00183, mean bootstrap error 0.00278, ratio 0.66
  { "Line35/simulated events area / cm2", { 45.6571, 0.0688 } },  // n=12, scatter of runs 0.238, mean bootstrap error 0.276, ratio 0.86
  { "Line35/reconstructed area / cm2", { 43.7333, 0.0613 } },  // n=12, scatter of runs 0.212, mean bootstrap error 0.27, ratio 0.79
  { "Line35/peak fraction", { 0.97389, 0.000279 } },  // n=12, scatter of runs 0.000967, mean bootstrap error 0.000999, ratio 0.97
  { "Line35/lost fluorescence fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line35/mean energy / keV", { 34.9706, 0.000771 } },  // n=12, scatter of runs 0.00267, mean bootstrap error 0.00298, ratio 0.90
  { "Line45/simulated events area / cm2", { 49.6854, 0.107 } },  // n=12, scatter of runs 0.37, mean bootstrap error 0.288, ratio 1.29
  { "Line45/reconstructed area / cm2", { 49.3567, 0.105 } },  // n=12, scatter of runs 0.364, mean bootstrap error 0.287, ratio 1.27
  { "Line45/peak fraction", { 0.953205, 0.000411 } },  // n=12, scatter of runs 0.00142, mean bootstrap error 0.00122, ratio 1.16
  { "Line45/lost fluorescence fraction", { 0.0253622, 0.000229 } },  // n=12, scatter of runs 0.000793, mean bootstrap error 0.000924, ratio 0.86
  { "Line45/mean energy / keV", { 44.6672, 0.0023 } },  // n=12, scatter of runs 0.00798, mean bootstrap error 0.0102, ratio 0.79
  { "Line60/simulated events area / cm2", { 52.0235, 0.098 } },  // n=12, scatter of runs 0.34, mean bootstrap error 0.294, ratio 1.15
  { "Line60/reconstructed area / cm2", { 51.5099, 0.0933 } },  // n=12, scatter of runs 0.323, mean bootstrap error 0.293, ratio 1.10
  { "Line60/peak fraction", { 0.944256, 0.00038 } },  // n=12, scatter of runs 0.00132, mean bootstrap error 0.00134, ratio 0.98
  { "Line60/lost fluorescence fraction", { 0.0245935, 0.000295 } },  // n=12, scatter of runs 0.00102, mean bootstrap error 0.000871, ratio 1.17
  { "Line60/mean energy / keV", { 59.5713, 0.00391 } },  // n=12, scatter of runs 0.0135, mean bootstrap error 0.0114, ratio 1.19
  { "Line80/simulated events area / cm2", { 53.2496, 0.0871 } },  // n=12, scatter of runs 0.302, mean bootstrap error 0.298, ratio 1.01
  { "Line80/reconstructed area / cm2", { 52.2421, 0.0868 } },  // n=12, scatter of runs 0.301, mean bootstrap error 0.295, ratio 1.02
  { "Line80/peak fraction", { 0.91116, 0.000517 } },  // n=12, scatter of runs 0.00179, mean bootstrap error 0.00162, ratio 1.10
  { "Line80/lost fluorescence fraction", { 0.0330238, 0.000329 } },  // n=12, scatter of runs 0.00114, mean bootstrap error 0.00103, ratio 1.10
  { "Line80/mean energy / keV", { 79.1529, 0.00589 } },  // n=12, scatter of runs 0.0204, mean bootstrap error 0.0183, ratio 1.12
  { "Line120/simulated events area / cm2", { 54.3101, 0.119 } },  // n=12, scatter of runs 0.412, mean bootstrap error 0.301, ratio 1.37
  { "Line120/reconstructed area / cm2", { 52.2554, 0.115 } },  // n=12, scatter of runs 0.397, mean bootstrap error 0.295, ratio 1.35
  { "Line120/peak fraction", { 0.837431, 0.00059 } },  // n=12, scatter of runs 0.00204, mean bootstrap error 0.00207, ratio 0.99
  { "Line120/lost fluorescence fraction", { 0.0448388, 0.00042 } },  // n=12, scatter of runs 0.00146, mean bootstrap error 0.00115, ratio 1.26
  { "Line120/mean energy / keV", { 116.155, 0.0187 } },  // n=12, scatter of runs 0.0648, mean bootstrap error 0.0784, ratio 0.83
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecThresholds::ETCosimaToMimrecThresholds() : MEndToEndTest("ETCosimaToMimrecThresholds")
{
  SetReferences(c_References);
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecThresholds::Run()
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

    ETStatistic InPeak = [Line](const vector<double>& Values) {
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
    ETStatistic LostFluorescence = [Line](const vector<double>& Values) {
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
    ETStatistic Mean = [](const vector<double>& Values) {
      double Sum = 0.0;
      for (double V: Values) {
        Sum += V;
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return Sum/Values.size();
    };
    Passed = CompareToReference(Name, "peak fraction", "The fraction of the reconstructed events in the peak (3% around the line)", Measure(All, InPeak, 50)) && Passed;
    Passed = CompareToReference(Name, "lost fluorescence fraction", "The fraction of the reconstructed events 10 keV (+-2 keV) below the line: a germanium fluorescence photon was lost", Measure(All, LostFluorescence, 50)) && Passed;
    Passed = CompareToReference(Name, "mean energy / keV", "The mean reconstructed energy", Measure(All, Mean, 50)) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecThresholds Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecThresholds.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
