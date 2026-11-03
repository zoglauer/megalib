/*
 * ETCosimaToMimrecOrientation.cxx
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

#include "ETCosimaToMimrecOrientation.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <sstream>


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool ETCosimaToMimrecOrientation::RunTest()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // The source in Galactic coordinates
  const double SourceLatitude = 0.0;
  const double SourceLongitude = 45.0;
  const MVector Source = Galactic(SourceLatitude, SourceLongitude);

  // Time between two entries of the slew in the orientation file (s)
  const double SlewStep = 0.5;

  auto SlewLines = [SlewStep]() {
    vector<MString> Lines;
    for (int i = 0; i <= 200; ++i) {
      Lines.push_back(MString("OG ") + SlewStep*i + " 90 0 0 " + 0.45*i);
    }
    Lines.push_back("OG 100000 90 0 0 90");
    return Lines;
  };

  struct Pointing {
    MString m_Name;
    //! The orientation file lines
    vector<MString> m_Lines;
    //! The z axis (latitude, longitude) as function of the time, for the expectation: constant or slewing along the equator
    function<MVector(double)> m_Z;
    bool m_XTowardsSource;
  };
  const vector<Pointing> Pointings = {
    // z at the source:
    { "OnAxis", { "OG 0 90 0 0 45", "OG 100000 90 0 0 45" }, [&](double) { return Galactic(0, 45); }, false },
    // z 90 degrees away, x toward the source:
    { "NinetyDegrees", { "OG 0 0 45 0 135", "OG 100000 0 45 0 135" }, [&](double) { return Galactic(0, 135); }, true },
    // z 60 degrees away:
    { "SixtyDegrees", { "OG 0 90 0 0 105", "OG 100000 90 0 0 105" }, [&](double) { return Galactic(0, 105); }, false },
    // z slews along the equator in 100 seconds - the time is rounded to the closest entry of the file:
    { "Slew", SlewLines(), [&](double Time) { return Galactic(0, 0.9*(SlewStep*round(Time/SlewStep))); }, false }
  };

  vector<ETSimScenario> Scenarios;
  for (const Pointing& Entry: Pointings) {
    ETSimScenario Scenario;
    Scenario.m_Name = Entry.m_Name;
    Scenario.m_Time = 100;
    MString Ori = "Type OrientationsGalactic\n";
    for (const MString& Line: Entry.m_Lines) {
      Ori += Line + "\n";
    }
    Scenario.m_Files = { { "Sky.ori", Ori } };
    Scenario.m_RunLines = { "OrientationSky Galactic File NoLoop Sky.ori" };
    Scenario.SetFarFieldPointSource(0, 0, "Mono 662", 2.0);
    Scenario.m_SourceLines.push_back(MString("Orientation Galactic Fixed ") + SourceLatitude + " " + SourceLongitude);
    Scenarios.push_back(Scenario);
  }
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;
  Passed = EvaluateTrue("CreateMimrecConfiguration()", "mimrec", "The default mimrec configuration can be created", CreateMimrecConfiguration(Common)) && Passed;
  // Select an energy window around the line
  const vector<MString> Window = { "EventSelections.FirstEnergyWindow.Min=637", "EventSelections.FirstEnergyWindow.Max=687" };

  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    Runs.push_back(async(launch::async, [this, Scenario, Common]() { return SimulateAndReconstruct(Scenario, Common + "/revan.cfg"); }));
  }

  // The summary: the ARM relative to the true direction of the events for every pointing
  TPad* ARMPage = GetReport().AddPage("ARM relative to the true direction of the full energy Compton events: on axis, 90 deg, 60 deg, slew", 2, 2);

  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const MString Name = Scenarios[s].m_Name;
    Passed = EvaluateTrue("Simulate()", Name, "Cosima and revan run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadSim()", Name, "The sim and tra files can be read", Sim.m_FileSuccessfullyRead == true && Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false || Tra.m_FileSuccessfullyRead == false) {
      continue;
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers, "<<Tra.Count(MPhysicalEvent::c_Compton)<<" Compton events"<<endl;
    Passed = EvaluateTrue("ReadSim()", Name + ", events", "There are enough triggered events (> 300)", Sim.m_Events.size() > 300) && Passed;
    if (Sim.m_Events.size() <= 300) {
      continue;
    }

    // Get the direction to the source in the detector frame from the sim file
    double MaxPolarDifference = 0.0, MaxAzimuthDifference = 0.0;
    double AzimuthBefore = -1.0, AzimuthAfter = -1.0;
    unsigned int NBefore = 0, NAfter = 0;
    map<int, MVector> ToSource;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      const MVector Direction = (Event->GetIAAt(0)->GetSecondaryDirection()*(-1.0)).Unit();
      ToSource[Event->GetID()] = Direction;
      const MVector ZAxis = Pointings[s].m_Z(Event->GetTime().GetAsSeconds());
      const double ExpectedPolar = acos(max(-1.0, min(1.0, ZAxis.Dot(Source))))*c_Deg;
      const double Polar = acos(max(-1.0, min(1.0, Direction.Z())))*c_Deg;
      MaxPolarDifference = GetMaximum(MaxPolarDifference, fabs(Polar - ExpectedPolar));
      double Azimuth = atan2(Direction.Y(), Direction.X())*c_Deg;
      if (Azimuth < 0) {
        Azimuth += 360.0;
      }
      if (Pointings[s].m_XTowardsSource == true) {
        MaxAzimuthDifference = GetMaximum(MaxAzimuthDifference, min(Azimuth, 360.0 - Azimuth));
      }
      // Check the slew away from the axis crossing - the azimuth is undefined there
      if (Pointings[s].m_Name == "Slew" && ExpectedPolar > 8.0) {
        if (Event->GetTime().GetAsSeconds() < 50.0) {
          if (AzimuthBefore < 0) {
            AzimuthBefore = Azimuth;
          }
          ++NBefore;
          MaxAzimuthDifference = GetMaximum(MaxAzimuthDifference, min(fabs(Azimuth - AzimuthBefore), 360.0 - fabs(Azimuth - AzimuthBefore)));
        } else {
          if (AzimuthAfter < 0) {
            AzimuthAfter = Azimuth;
          }
          ++NAfter;
          MaxAzimuthDifference = GetMaximum(MaxAzimuthDifference, min(fabs(Azimuth - AzimuthAfter), 360.0 - fabs(Azimuth - AzimuthAfter)));
        }
      }
    }
    // Tolerance 1e-3 deg - the precision of the angles in the sim file
    Passed = EvaluateNear("Orientation", Name + ", polar angle", "The polar angle of the direction to the source in the detector frame is the angle between the z axis and the source (degrees)", MaxPolarDifference, 0.0, 1e-3) && Passed;
    if (Pointings[s].m_XTowardsSource == true) {
      Passed = EvaluateNear("Orientation", Name + ", azimuth", "The azimuth is zero if the x axis points to the source (degrees)", MaxAzimuthDifference, 0.0, 1e-3) && Passed;
    }
    if (Pointings[s].m_Name == "Slew") {
      Passed = EvaluateTrue("Orientation", Name + ", azimuth sides", "The source is seen on both sides of the slew (events before and after the crossing)", NBefore > 50 && NAfter > 50) && Passed;
      // The azimuth is constant on each side ...
      Passed = EvaluateNear("Orientation", Name + ", azimuth constant", "The azimuth of the source is constant before and after the crossing of the z axis (degrees)", MaxAzimuthDifference, 0.0, 1e-3) && Passed;
      // ... and differs by 180 degrees between the two sides
      double Difference = fabs(AzimuthBefore - AzimuthAfter);
      if (Difference > 180.0) {
        Difference = 360.0 - Difference;
      }
      Passed = EvaluateNear("Orientation", Name + ", azimuth flip", "The azimuth of the source flips by 180 degrees when the source crosses the z axis (degrees)", Difference, 180.0, 1e-3) && Passed;
    }

    // Check the ARM relative to the true direction of each event:
    vector<double> ARMs;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      auto Found = ToSource.find(Event->GetId());
      if (Found == ToSource.end() || fabs(Event->Eg() + Event->Ee() - 662.0) > 25.0) {
        continue;
      }
      ARMs.push_back(ARM(*Event, Found->second));
    }
    ARMPage->cd(s + 1);
    GetReport().DrawHistogram(Name + ";ARM / deg;events", ARMs, 60, -30.0, 30.0);
    mout<<Name<<": "<<ARMs.size()<<" full energy Compton events"<<endl;
    Passed = EvaluateTrue("ARM", Name + ", events", "There are enough full energy Compton events (> 100)", ARMs.size() > 100) && Passed;
    if (ARMs.size() > 100) {
      // The ARM relative to its peak (the half-sample mode, found on every resample of the bootstrap): the peak is at the true direction,
      // and the peak, the width, the containment, and the core fraction are compared with the references
      MStatisticFunction Peak = [](const vector<double>& Values) { return MStatistics::GetPeakPosition(Values); };
      MStatisticFunction Fwhm = [](const vector<double>& Values) { return MStatistics::GetFullWidthAtHalfMaximum(Values, 0.5); };
      MStatisticFunction Containment68 = [](const vector<double>& Values) { return MStatistics::GetContainmentAroundPeak(Values, 0.68); };
      MStatisticFunction CoreFraction = [](const vector<double>& Values) { return MStatistics::GetFractionAroundPeak(Values, 5.0); };
      const ETMeasuredValue ARMPeak = Measure(ARMs, Peak, 50);
      const ETMeasuredValue ARMFwhm = Measure(ARMs, Fwhm, 50);
      const ETMeasuredValue ARM68 = Measure(ARMs, Containment68, 50);
      mout<<Name<<": ARM peak at "<<ARMPeak.m_Value<<" +- "<<ARMPeak.m_Error<<" deg, FWHM "<<ARMFwhm.m_Value<<" +- "<<ARMFwhm.m_Error<<" deg, 68% containment "<<ARM68.m_Value<<" +- "<<ARM68.m_Error<<" deg"<<endl;
      Passed = EvaluateNear("ARM", Name + ", peak at zero", "The peak of the ARM distribution is at the true direction of the source (5 sigma of the peak; degrees)", ARMPeak.m_Value, 0.0, MEndToEndResultList::c_PullLimit*ARMPeak.m_Error) && Passed;
      Passed = CompareToReference(Name, "ARM peak / deg", "The position of the peak of the ARM distribution (the half-sample mode)", ARMPeak) && Passed;
      Passed = CompareToReference(Name, "ARM FWHM / deg", "The full width at half maximum of the ARM distribution (0.5 degree bins, three bin smoothing)", ARMFwhm) && Passed;
      Passed = CompareToReference(Name, "ARM 68% containment / deg", "The 68% containment of |ARM - peak|", ARM68) && Passed;
      Passed = CompareToReference(Name, "ARM core fraction (|ARM - peak| < 5 deg)", "The fraction of the events within 5 degrees of the peak", Measure(ARMs, CoreFraction, 50)) && Passed;
      ARMPage->cd(s + 1);
      GetReport().DrawComparison(0, "FWHM", ARMFwhm, GetReference(Name, "ARM FWHM / deg"));
      GetReport().DrawComparison(1, "68%", ARM68, GetReference(Name, "ARM 68% containment / deg"));
    }

    // Check the mimrec ARM plot for the static pointings:
    if (Pointings[s].m_Name != "Slew") {
      const MVector True = ToSource.begin()->second;
      const double TrueTheta = acos(max(-1.0, min(1.0, True.Z())))*c_Deg;
      const double TruePhi = atan2(True.Y(), True.X())*c_Deg;
      vector<MString> Position = Window;
      Position.push_back("TestPositions.Use=true");
      Position.push_back(MString("TestPositions.CoordinateSystemSpherical.Theta=") + TrueTheta);
      Position.push_back(MString("TestPositions.CoordinateSystemSpherical.Phi=") + TruePhi);
      bool Done = true;
      if (Mimrec(Scenarios[s], Common + "/mimrec.cfg", "-a", "arm.root", Position) == false) {
        Done = false;
      }
      if (Done == true && Mimrec(Scenarios[s], Common + "/mimrec.cfg", "-x", "selected.tra", Window) == false) {
        Done = false;
      }
      Passed = EvaluateTrue("Mimrec", Name + ", run", "Mimrec runs through", Done) && Passed;
      unique_ptr<TH1D> ARMHistogram = FirstHistogram<TH1D>(Scenarios[s].m_Directory + "/arm.root");
      const ETTraFileCoreData Extracted = ReadTraFile(Scenarios[s].m_Directory + "/selected.tra");
      Passed = EvaluateTrue("Mimrec", Name + ", ARM", "The ARM histogram and the extracted events of mimrec can be read", ARMHistogram != nullptr && Extracted.m_FileSuccessfullyRead == true) && Passed;
      if (ARMHistogram != nullptr && Extracted.m_FileSuccessfullyRead == true) {
        const int Bins = ARMHistogram->GetNbinsX();
        vector<double> Own(Bins, 0.0);
        MVector Source;
        Source.SetMagThetaPhi(1.0, TrueTheta*c_Rad, TruePhi*c_Rad);
        for (const shared_ptr<MComptonEvent>& Event: Extracted.Compton()) {
          const double Arm = ARM(*Event, Source);
          const int Bin = static_cast<int>(floor((Arm - ARMHistogram->GetXaxis()->GetXmin())/(ARMHistogram->GetXaxis()->GetXmax() - ARMHistogram->GetXaxis()->GetXmin())*Bins));
          if (Bin >= 0 && Bin < Bins) {
            Own[Bin] += 1.0;
          }
        }
        double OwnInside = 0.0, MimrecInside = 0.0;
        for (int b = 0; b < Bins; ++b) {
          OwnInside += Own[b];
          MimrecInside += ARMHistogram->GetBinContent(b + 1);
        }
        mout<<Name<<": mimrec ARM plot for theta = "<<TrueTheta<<" deg, phi = "<<TruePhi<<" deg: "<<MimrecInside<<" events in the range (own analysis "<<OwnInside<<"), peak at "<<ARMHistogram->GetXaxis()->GetBinCenter(ARMHistogram->GetMaximumBin())<<" deg"<<endl;
        Passed = EvaluateNear("Mimrec", Name + ", ARM content", "The ARM histogram of mimrec for the true position has the same number of events in the range as the own analysis", MimrecInside, OwnInside, 0.0) && Passed;
        Passed = EvaluateNear("Mimrec", Name + ", ARM peak", "The ARM histogram of mimrec for the true position peaks at zero (within 2 degrees)", ARMHistogram->GetXaxis()->GetBinCenter(ARMHistogram->GetMaximumBin()), 0.0, 2.0) && Passed;
      }
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the Galactic unit vector of a latitude and longitude in degrees
MVector ETCosimaToMimrecOrientation::Galactic(double Latitude, double Longitude)
{
  MVector Direction;
  Direction.SetMagThetaPhi(1.0, (90.0 - Latitude)*c_Rad, Longitude*c_Rad);
  return Direction;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("ETCosimaToMimrecOrientation", "end-to-end test: orientation in the Galactic sky") == false) {
    return 1;
  }

  ETCosimaToMimrecOrientation Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecOrientation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
