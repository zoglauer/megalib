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

////////////////////////////////////////////////////////////////////////////////


//! The reference values of the detector effects engine MEGAlib 1-4: mean of independent runs and its standard error
static const ETReferences c_References = {
  { "Line200Theta0/Compton effective area / cm2", { 11.7283, 0.0324 } },  // n=12, scatter of runs 0.112, mean bootstrap error 0.0989, ratio 1.14
  { "Line200Theta0/ARM FWHM / deg", { 11.8556, 0.171 } },  // n=12, scatter of runs 0.592, mean bootstrap error 0.622, ratio 0.95
  { "Line200Theta0/ARM 68% containment / deg", { 11.4172, 0.0333 } },  // n=12, scatter of runs 0.115, mean bootstrap error 0.154, ratio 0.75
  { "Line200Theta0/ARM 90% containment / deg", { 28.8836, 0.141 } },  // n=12, scatter of runs 0.49, mean bootstrap error 0.461, ratio 1.06
  { "Line200Theta0/ARM core fraction (|ARM| < 5 deg)", { 0.420044, 0.00117 } },  // n=12, scatter of runs 0.00407, mean bootstrap error 0.00407, ratio 1.00
  { "Line200Theta0/ARM mean (|ARM| < 10 deg) / deg", { 0.00236789, 0.00978 } },  // n=12, scatter of runs 0.0339, mean bootstrap error 0.0493, ratio 0.69
  { "Line200Theta40/Compton effective area / cm2", { 13.928, 0.0277 } },  // n=12, scatter of runs 0.0959, mean bootstrap error 0.108, ratio 0.89
  { "Line200Theta40/ARM FWHM / deg", { 13.5592, 0.202 } },  // n=12, scatter of runs 0.699, mean bootstrap error 0.563, ratio 1.24
  { "Line200Theta40/ARM 68% containment / deg", { 12.061, 0.0318 } },  // n=12, scatter of runs 0.11, mean bootstrap error 0.136, ratio 0.81
  { "Line200Theta40/ARM 90% containment / deg", { 29.6298, 0.0985 } },  // n=12, scatter of runs 0.341, mean bootstrap error 0.412, ratio 0.83
  { "Line200Theta40/ARM core fraction (|ARM| < 5 deg)", { 0.396183, 0.00108 } },  // n=12, scatter of runs 0.00375, mean bootstrap error 0.00372, ratio 1.01
  { "Line200Theta40/ARM mean (|ARM| < 10 deg) / deg", { 0.00519126, 0.0134 } },  // n=12, scatter of runs 0.0465, mean bootstrap error 0.0496, ratio 0.94
  { "Line500Theta0/Compton effective area / cm2", { 9.9993, 0.0283 } },  // n=12, scatter of runs 0.098, mean bootstrap error 0.0913, ratio 1.07
  { "Line500Theta0/ARM FWHM / deg", { 5.91609, 0.0772 } },  // n=12, scatter of runs 0.267, mean bootstrap error 0.281, ratio 0.95
  { "Line500Theta0/ARM 68% containment / deg", { 17.6262, 0.123 } },  // n=12, scatter of runs 0.426, mean bootstrap error 0.494, ratio 0.86
  { "Line500Theta0/ARM 90% containment / deg", { 58.5881, 0.221 } },  // n=12, scatter of runs 0.767, mean bootstrap error 0.784, ratio 0.98
  { "Line500Theta0/ARM core fraction (|ARM| < 5 deg)", { 0.445378, 0.00122 } },  // n=12, scatter of runs 0.00422, mean bootstrap error 0.00462, ratio 0.91
  { "Line500Theta0/ARM mean (|ARM| < 10 deg) / deg", { 0.120404, 0.0126 } },  // n=12, scatter of runs 0.0435, mean bootstrap error 0.0491, ratio 0.89
  { "Line500Theta40/Compton effective area / cm2", { 11.1676, 0.0212 } },  // n=12, scatter of runs 0.0736, mean bootstrap error 0.0965, ratio 0.76
  { "Line500Theta40/ARM FWHM / deg", { 6.36792, 0.0636 } },  // n=12, scatter of runs 0.22, mean bootstrap error 0.29, ratio 0.76
  { "Line500Theta40/ARM 68% containment / deg", { 18.6038, 0.216 } },  // n=12, scatter of runs 0.748, mean bootstrap error 0.48, ratio 1.56
  { "Line500Theta40/ARM 90% containment / deg", { 58.8081, 0.243 } },  // n=12, scatter of runs 0.841, mean bootstrap error 0.694, ratio 1.21
  { "Line500Theta40/ARM core fraction (|ARM| < 5 deg)", { 0.429395, 0.0014 } },  // n=12, scatter of runs 0.00487, mean bootstrap error 0.00431, ratio 1.13
  { "Line500Theta40/ARM mean (|ARM| < 10 deg) / deg", { 0.109465, 0.0138 } },  // n=12, scatter of runs 0.0479, mean bootstrap error 0.0457, ratio 1.05
  { "Line1000Theta0/Compton effective area / cm2", { 6.88806, 0.0269 } },  // n=12, scatter of runs 0.0932, mean bootstrap error 0.0758, ratio 1.23
  { "Line1000Theta0/ARM FWHM / deg", { 4.57497, 0.0596 } },  // n=12, scatter of runs 0.207, mean bootstrap error 0.241, ratio 0.86
  { "Line1000Theta0/ARM 68% containment / deg", { 25.4944, 0.293 } },  // n=12, scatter of runs 1.02, mean bootstrap error 1.28, ratio 0.79
  { "Line1000Theta0/ARM 90% containment / deg", { 81.4486, 0.245 } },  // n=12, scatter of runs 0.849, mean bootstrap error 0.749, ratio 1.13
  { "Line1000Theta0/ARM core fraction (|ARM| < 5 deg)", { 0.459042, 0.00138 } },  // n=12, scatter of runs 0.0048, mean bootstrap error 0.00589, ratio 0.81
  { "Line1000Theta0/ARM mean (|ARM| < 10 deg) / deg", { 0.291674, 0.0161 } },  // n=12, scatter of runs 0.0557, mean bootstrap error 0.0561, ratio 0.99
  { "Line1000Theta40/Compton effective area / cm2", { 7.42299, 0.0261 } },  // n=12, scatter of runs 0.0906, mean bootstrap error 0.0786, ratio 1.15
  { "Line1000Theta40/ARM FWHM / deg", { 5.1543, 0.0456 } },  // n=12, scatter of runs 0.158, mean bootstrap error 0.264, ratio 0.60
  { "Line1000Theta40/ARM 68% containment / deg", { 27.0714, 0.383 } },  // n=12, scatter of runs 1.33, mean bootstrap error 1.15, ratio 1.15
  { "Line1000Theta40/ARM 90% containment / deg", { 82.5622, 0.319 } },  // n=12, scatter of runs 1.11, mean bootstrap error 0.757, ratio 1.46
  { "Line1000Theta40/ARM core fraction (|ARM| < 5 deg)", { 0.434644, 0.00147 } },  // n=12, scatter of runs 0.00508, mean bootstrap error 0.00523, ratio 0.97
  { "Line1000Theta40/ARM mean (|ARM| < 10 deg) / deg", { 0.221962, 0.0151 } },  // n=12, scatter of runs 0.0523, mean bootstrap error 0.0576, ratio 0.91
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecAngularResolution::ETCosimaToMimrecAngularResolution() : MEndToEndTest("ETCosimaToMimrecAngularResolution")
{
  SetReferences(c_References);
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecAngularResolution::Run()
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

    ETStatistic Fwhm = [](const vector<double>& Values) { return FullWidthAtHalfMaximum(Values, 0.5); };
    ETStatistic Containment68 = [](const vector<double>& Values) {
      vector<double> Absolute;
      for (double V: Values) {
        Absolute.push_back(fabs(V));
      }
      return GetQuantile(Absolute, 0.68);
    };
    ETStatistic Containment90 = [](const vector<double>& Values) {
      vector<double> Absolute;
      for (double V: Values) {
        Absolute.push_back(fabs(V));
      }
      return GetQuantile(Absolute, 0.90);
    };
    ETStatistic Core = [](const vector<double>& Values) {
      double Sum = 0.0, N = 0.0;
      for (double V: Values) {
        if (fabs(V) < 10.0) {
          Sum += V;
          N += 1.0;
        }
      }
      if (N == 0.0) {
        return 0.0;
      }
      return Sum/N;
    };
    ETStatistic CoreFraction = [](const vector<double>& Values) {
      double N = 0.0;
      for (double V: Values) {
        if (fabs(V) < 5.0) {
          N += 1.0;
        }
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return N/Values.size();
    };
    const ETMeasuredValue ARMFwhm = Measure(ARMs, Fwhm, 50);
    mout<<Name<<": ARM FWHM "<<ARMFwhm.m_Value<<" +- "<<ARMFwhm.m_Error<<" deg"<<endl;
    Passed = CompareToReference(Name, "ARM FWHM / deg", "The full width at half maximum of the ARM distribution (0.5 degree bins, three bin smoothing)", ARMFwhm) && Passed;
    Passed = CompareToReference(Name, "ARM 68% containment / deg", "The 68% containment of |ARM|", Measure(ARMs, Containment68, 50)) && Passed;
    Passed = CompareToReference(Name, "ARM 90% containment / deg", "The 90% containment of |ARM|", Measure(ARMs, Containment90, 50)) && Passed;
    Passed = CompareToReference(Name, "ARM core fraction (|ARM| < 5 deg)", "The fraction of the events with |ARM| < 5 degrees", Measure(ARMs, CoreFraction, 50)) && Passed;
    Passed = CompareToReference(Name, "ARM mean (|ARM| < 10 deg) / deg", "The mean ARM of the events with |ARM| < 10 degrees", Measure(ARMs, Core, 50)) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecAngularResolution Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecAngularResolution.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
