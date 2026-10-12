/*
 * ETCosimaToMimrecPositionResolution.cxx
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

#include "ETCosimaToMimrecPositionResolution.h"

// Standard libs:
#include <cmath>
#include <future>
#include <iostream>
#include <map>

////////////////////////////////////////////////////////////////////////////////


//! The reference values of the detector effects engine MEGAlib 1-4: mean of independent runs and its standard error
static const ETReferences c_References = {
  { "Line100/x RMS / cm", { 0.0527852, 3.23e-05 } },  // n=12, scatter of runs 0.000112, mean bootstrap error 0.000139, ratio 0.81
  { "Line100/y RMS / cm", { 0.0527769, 3.66e-05 } },  // n=12, scatter of runs 0.000127, mean bootstrap error 0.000143, ratio 0.89
  { "Line100/z robust sigma / cm", { 0.0195901, 3.47e-05 } },  // n=12, scatter of runs 0.00012, mean bootstrap error 0.000142, ratio 0.85
  { "Line100/x mean / cm", { -0.000774414, 8.64e-05 } },  // n=12, scatter of runs 0.000299, mean bootstrap error 0.000326, ratio 0.92
  { "Line100/y mean / cm", { 0.00012526, 0.000116 } },  // n=12, scatter of runs 0.000403, mean bootstrap error 0.00033, ratio 1.22
  { "Line100/z mean / cm", { -0.000695666, 2.31e-05 } },  // n=12, scatter of runs 8.01e-05, mean bootstrap error 0.00012, ratio 0.67
  { "Line100/x outside strip fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line100/y outside strip fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line200/x RMS / cm", { 0.0524079, 5.04e-05 } },  // n=12, scatter of runs 0.000175, mean bootstrap error 0.000209, ratio 0.84
  { "Line200/y RMS / cm", { 0.05248, 5.86e-05 } },  // n=12, scatter of runs 0.000203, mean bootstrap error 0.000223, ratio 0.91
  { "Line200/z robust sigma / cm", { 0.0199329, 7.28e-05 } },  // n=12, scatter of runs 0.000252, mean bootstrap error 0.000217, ratio 1.16
  { "Line200/x mean / cm", { -0.000647914, 9.68e-05 } },  // n=12, scatter of runs 0.000335, mean bootstrap error 0.000466, ratio 0.72
  { "Line200/y mean / cm", { -9.69388e-05, 0.000163 } },  // n=12, scatter of runs 0.000566, mean bootstrap error 0.000462, ratio 1.22
  { "Line200/z mean / cm", { -0.000163743, 3.9e-05 } },  // n=12, scatter of runs 0.000135, mean bootstrap error 0.000177, ratio 0.76
  { "Line200/x outside strip fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
  { "Line200/y outside strip fraction", { 0, 0 } },  // n=12, scatter of runs 0, mean bootstrap error 0, ratio 0.00
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ETCosimaToMimrecPositionResolution::ETCosimaToMimrecPositionResolution() : MEndToEndTest("ETCosimaToMimrecPositionResolution")
{
  SetReferences(c_References);
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecPositionResolution::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // The active wafer of Max.geo.setup is 7.4 cm wide (8 cm minus two guard rings of 0.3 cm) with 40 strips, the depth resolution is 0.02 cm (sigma) at all energies
  const double Pitch = 7.4/40.0;
  const double DepthSigma = 0.02;
  const vector<double> Lines = { 100, 200 };
  const double Flux = 2.0;
  const double Time = 300.0;

  vector<ETSimScenario> Scenarios;
  for (double Line: Lines) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Line") + static_cast<int>(Line);
    Scenario.m_Time = Time;
    Scenario.SetFarFieldPointSource(20, 0, MString("Mono ") + Line, Flux);
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
    const double Line = Lines[s];
    Passed = EvaluateTrue("Simulate()", Name, "Cosima and revan run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadTra()", Name, "The sim and tra files can be read", Sim.m_FileSuccessfullyRead == true && Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false || Tra.m_FileSuccessfullyRead == false) {
      continue;
    }

    // The position of the photo absorption of the events with a single hit which absorbed the whole energy
    map<int, MVector> TruePosition;
    map<int, MVector> HitPosition;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      if (Event->GetNHTs() == 1 && fabs(Event->GetTotalEnergyDepositBeforeNoising() - Line) < 0.01 && Event->GetHTAt(0)->GetNOrigins() > 0) {
        // The origin of the hit: the interaction which deposited the energy (IDs start at 1, the INIT interaction)
        HitPosition[Event->GetID()] = Event->GetHTAt(0)->GetOriginalPosition();
        const unsigned int Origin = Event->GetHTAt(0)->GetOriginAt(0);
        if (Origin >= 1 && Origin <= Event->GetNIAs()) {
          TruePosition[Event->GetID()] = Event->GetIAAt(Origin - 1)->GetPosition();
        }
      }
    }

    vector<double> ResidualX, ResidualY, ResidualZ, HitResidualX, HitResidualY;
    for (const shared_ptr<MPhotoEvent>& Event: Tra.Photo()) {
      auto Found = TruePosition.find(Event->GetId());
      if (Found != TruePosition.end() && fabs(Event->GetEnergy() - Line) < 0.03*Line) {
        ResidualX.push_back(Event->GetPosition().X() - Found->second.X());
        ResidualY.push_back(Event->GetPosition().Y() - Found->second.Y());
        // The depth is compared with the hit before the noising: the energy deposit extends over some 100 micrometers
        const MVector Hit = HitPosition[Event->GetId()];
        ResidualZ.push_back(Event->GetPosition().Z() - Hit.Z());
        HitResidualX.push_back(Event->GetPosition().X() - Hit.X());
        HitResidualY.push_back(Event->GetPosition().Y() - Hit.Y());
      }
    }
    mout<<Name<<": "<<ResidualX.size()<<" single hit full energy photo events"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", events", "There are enough single hit full energy photo events (> 1000)", ResidualX.size() > 1000) && Passed;
    if (ResidualX.size() <= 1000) {
      continue;
    }

    ETStatistic RMS = [](const vector<double>& Values) {
      double Sum = 0.0;
      for (double V: Values) {
        Sum += V*V;
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return sqrt(Sum/Values.size());
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
    ETStatistic OutsideStrip = [Pitch](const vector<double>& Values) {
      double N = 0.0;
      for (double V: Values) {
        if (fabs(V) > 0.5*Pitch + 0.02) {
          N += 1.0;
        }
      }
      if (Values.empty() == true) {
        return 0.0;
      }
      return N/Values.size();
    };
    ETStatistic RobustWidth = [](const vector<double>& Values) { return GetRobustSigma(Values); };

    const ETMeasuredValue RMSX = Measure(ResidualX, RMS, 50);
    const ETMeasuredValue RMSY = Measure(ResidualY, RMS, 50);
    mout<<Name<<": x RMS "<<RMSX.m_Value<<" +- "<<RMSX.m_Error<<" cm, y RMS "<<RMSY.m_Value<<" +- "<<RMSY.m_Error<<" cm (uniform over the strip: "<<Pitch/sqrt(12.0)<<" cm)"<<endl;
    // The positions are uniformly distributed over the strip: RMS = pitch/sqrt(12), the interaction point is not the mean position of the energy deposit: allow 2%
    Passed = EvaluateNear("Position", Name + ", x resolution", "The RMS of the x residual is the strip pitch divided by the square root of twelve (five standard errors and 2%)", RMSX.m_Value, Pitch/sqrt(12.0), 5.0*RMSX.m_Error + 0.02*Pitch/sqrt(12.0)) && Passed;
    Passed = EvaluateNear("Position", Name + ", y resolution", "The RMS of the y residual is the strip pitch divided by the square root of twelve (five standard errors and 2%)", RMSY.m_Value, Pitch/sqrt(12.0), 5.0*RMSY.m_Error + 0.02*Pitch/sqrt(12.0)) && Passed;
    // The sim file stores the center of the strip, x and y of the reconstructed event are this position
    double MaxHitX = 0.0, MaxHitY = 0.0;
    for (unsigned int i = 0; i < HitResidualX.size(); ++i) {
      MaxHitX = max(MaxHitX, fabs(HitResidualX[i]));
      MaxHitY = max(MaxHitY, fabs(HitResidualY[i]));
    }
    Passed = EvaluateNear("Position", Name + ", x strip", "The x position of the reconstructed event is the x position of the hit in the sim file (strip center)", MaxHitX, 0.0, 1e-3) && Passed;
    Passed = EvaluateNear("Position", Name + ", y strip", "The y position of the reconstructed event is the y position of the hit in the sim file (strip center)", MaxHitY, 0.0, 1e-3) && Passed;
    const ETMeasuredValue SigmaZ = Measure(ResidualZ, RobustWidth, 50);
    mout<<Name<<": z robust sigma "<<SigmaZ.m_Value<<" +- "<<SigmaZ.m_Error<<" cm (depth resolution "<<DepthSigma<<" cm)"<<endl;
    Passed = EvaluateNear("Position", Name + ", z resolution", "The robust sigma of the z residual relative to the hit of the sim file is the depth resolution of the geometry (five standard errors)", SigmaZ.m_Value, DepthSigma, 5.0*SigmaZ.m_Error) && Passed;

    Passed = CompareToReference(Name, "x RMS / cm", "The RMS of the x residual", RMSX) && Passed;
    Passed = CompareToReference(Name, "y RMS / cm", "The RMS of the y residual", RMSY) && Passed;
    Passed = CompareToReference(Name, "z robust sigma / cm", "The robust sigma of the z residual", SigmaZ) && Passed;
    Passed = CompareToReference(Name, "x mean / cm", "The mean x residual", Measure(ResidualX, Mean, 50)) && Passed;
    Passed = CompareToReference(Name, "y mean / cm", "The mean y residual", Measure(ResidualY, Mean, 50)) && Passed;
    Passed = CompareToReference(Name, "z mean / cm", "The mean z residual", Measure(ResidualZ, Mean, 50)) && Passed;
    Passed = CompareToReference(Name, "x outside strip fraction", "The fraction of events with a x residual larger than half a pitch plus 0.2 mm", Measure(ResidualX, OutsideStrip, 50)) && Passed;
    Passed = CompareToReference(Name, "y outside strip fraction", "The fraction of events with a y residual larger than half a pitch plus 0.2 mm", Measure(ResidualY, OutsideStrip, 50)) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecPositionResolution Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecPositionResolution.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
