/*
 * ETCosimaToMimrecImaging.cxx
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

#include "ETCosimaToMimrecImaging.h"

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
bool ETCosimaToMimrecImaging::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  // Source directions (polar angle, azimuth in degree) in the detector coordinate system
  const vector<pair<double, double>> Directions = { { 0, 0 }, { 20, 0 }, { 40, 90 }, { 60, 225 }, { 75, 315 } };
  vector<ETSimScenario> Scenarios;
  for (const auto& Direction: Directions) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Theta") + Direction.first + "Phi" + Direction.second;
    Scenario.m_Time = 300;
    Scenario.SetFarFieldPointSource(Direction.first, Direction.second, "Mono 662", 2.0);
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
  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const ETSimScenario Scenario = Scenarios[s];
    // Set the ARM plot for the source direction, and the image with one iteration
    vector<MString> Position = Window;
    Position.push_back("TestPositions.Use=true");
    Position.push_back(MString("TestPositions.CoordinateSystemSpherical.Theta=") + Directions[s].first);
    Position.push_back(MString("TestPositions.CoordinateSystemSpherical.Phi=") + Directions[s].second);
    vector<MString> Iterations = Window;
    Iterations.push_back("ImageReconstructionAlgorithms.ListMode.NIterations=1");
    Runs.push_back(async(launch::async, [this, Scenario, Common, Window, Position, Iterations]() {
      if (SimulateAndReconstruct(Scenario, Common + "/revan.cfg") == false) {
        return false;
      }
      if (Mimrec(Scenario, Common + "/mimrec.cfg", "-i", "image.root", Iterations) == false) {
        return false;
      }
      if (Mimrec(Scenario, Common + "/mimrec.cfg", "-a", "arm.root", Position) == false) {
        return false;
      }
      return Mimrec(Scenario, Common + "/mimrec.cfg", "-x", "selected.tra", Window);
    }));
  }

  for (unsigned int s = 0; s < Scenarios.size(); ++s) {
    const MString Name = Scenarios[s].m_Name;
    Passed = EvaluateTrue("Simulate()", Name, "Cosima, revan, and mimrec run through", Runs[s].get()) && Passed;
    const ETSimFileCoreData Sim = ReadSim(Scenarios[s]);
    const ETTraFileCoreData Tra = ReadTra(Scenarios[s]);
    Passed = EvaluateTrue("ReadTra()", Name, "The sim and tra files can be read", Sim.m_FileSuccessfullyRead == true && Tra.m_FileSuccessfullyRead == true) && Passed;
    if (Sim.m_FileSuccessfullyRead == false || Tra.m_FileSuccessfullyRead == false) {
      continue;
    }

    const double Theta = Directions[s].first*c_Rad;
    const double Phi = Directions[s].second*c_Rad;
    MVector Source;
    Source.SetMagThetaPhi(1.0, Theta, Phi);

    // Get the true direction from the sim file - it is the flight direction, opposite to the source
    double MaxDirectionDifference = 0.0;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      MaxDirectionDifference = GetMaximum(MaxDirectionDifference, (Event->GetIAAt(0)->GetSecondaryDirection() + Source).Mag());
    }
    Passed = EvaluateNear("ReadSim()", Name + ", true direction", "The flight direction of the initial particle in the sim file is opposite to the direction to the source", MaxDirectionDifference, 0.0, 1e-4) && Passed;

    // Complete absorption events: the total energy is close to the line energy
    vector<shared_ptr<MComptonEvent>> Full;
    vector<double> Energies;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      const double Total = Event->Eg() + Event->Ee();
      Energies.push_back(Total);
      if (fabs(Total - 662.0) < 25.0) {
        Full.push_back(Event);
      }
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers, "<<Tra.Count(MPhysicalEvent::c_Compton)<<" Compton events, "<<Full.size()<<" with the full energy"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", events", "There are enough Compton events with the full energy (> 150)", Full.size() > 150) && Passed;
    if (Full.size() <= 150) {
      continue;
    }

    // Use the peak of the ARM histogram - its median is shifted by asymmetric tails:
    auto Fraction = [&](const MVector& Direction, double Limit) {
      unsigned int Inside = 0;
      for (const shared_ptr<MComptonEvent>& Event: Full) {
        if (fabs(ARM(*Event, Direction)) < Limit) {
          ++Inside;
        }
      }
      return static_cast<double>(Inside)/Full.size();
    };
    vector<double> ARMs;
    vector<double> Histogram(61, 0.0);
    for (const shared_ptr<MComptonEvent>& Event: Full) {
      const double Arm = ARM(*Event, Source);
      ARMs.push_back(Arm);
      const int Bin = static_cast<int>(floor(Arm + 30.5));
      if (Bin >= 0 && Bin < 61) {
        Histogram[Bin] += 1.0;
      }
    }
    double BestSmoothed = -1.0, PeakARM = 0.0;
    for (int b = 1; b < 60; ++b) {
      const double Smoothed = Histogram[b-1] + Histogram[b] + Histogram[b+1];
      if (Smoothed > BestSmoothed) {
        BestSmoothed = Smoothed;
        PeakARM = b - 30.0;
      }
    }
    const double FractionTrue = Fraction(Source, 5.0);
    // Create a ring of directions 20 degrees around the source:
    MVector Helper(1.0, 0.0, 0.0);
    if (fabs(Source.Z()) < 0.9) {
      Helper = MVector(0.0, 0.0, 1.0);
    }
    const MVector FirstAxis = Helper.Cross(Source).Unit();
    const MVector Direction = Source.Cross(FirstAxis);
    double FractionRing = 0.0;
    for (unsigned int r = 0; r < 8; ++r) {
      const double Angle = r*c_Pi/4.0;
      const MVector Ring = (Source*cos(20.0*c_Rad) + (FirstAxis*cos(Angle) + Direction*sin(Angle))*sin(20.0*c_Rad)).Unit();
      FractionRing = max(FractionRing, Fraction(Ring, 5.0));
    }
    mout<<Name<<": ARM peak at "<<PeakARM<<" deg, fraction with |ARM| < 5 deg: "<<FractionTrue<<" (the best of eight directions displaced by 20 deg: "<<FractionRing<<")"<<endl;
    Passed = EvaluateNear("ARM", Name + ", peak", "The peak of the ARM distribution relative to the true source direction is at zero", PeakARM, 0.0, 2.0) && Passed;
    // Expected: mean zero within the statistical uncertainty, RMS 4.1 deg for |ARM| < 10 deg (baseline, measured 4.0 to 4.3 deg)
    unsigned int NumberCore = 0;
    double MeanCore = 0.0, RMSCore = 0.0;
    GetTruncatedMoments(ARMs, 5.0, NumberCore, MeanCore, RMSCore);
    unsigned int NumberWidth = 0;
    double MeanWidth = 0.0, RMSWidth = 0.0;
    GetTruncatedMoments(ARMs, 10.0, NumberWidth, MeanWidth, RMSWidth);
    mout<<Name<<": ARM mean "<<MeanCore<<" deg (|ARM| < 5 deg), RMS "<<RMSWidth<<" deg (|ARM| < 10 deg)"<<endl;
    Passed = EvaluateNear("ARM", Name + ", mean", "The mean ARM of the events with |ARM| < 5 degrees is zero (5 sigma of the mean; degrees)", MeanCore, 0.0, 5.0*RMSCore/sqrt(static_cast<double>(NumberCore))) && Passed;
    Passed = EvaluateNear("ARM", Name + ", width", "The RMS of the ARM of the events with |ARM| < 10 degrees is 4.1 degrees (+-0.5; degrees)", RMSWidth, 4.1, 0.5) && Passed;
    vector<double> AbsoluteARMs;
    for (double Arm: ARMs) {
      AbsoluteARMs.push_back(fabs(Arm));
    }
    const double MedianARM = GetMedian(AbsoluteARMs);
    const double MedianError = GetMedianError(AbsoluteARMs);
    mout<<Name<<": median |ARM| "<<MedianARM<<" +- "<<MedianError<<" deg"<<endl;
    // Expected median of |ARM| per direction: mean of sixteen runs (deg)
    static const map<string, double> BaselineMedians = {{"Theta0Phi0", 7.0}, {"Theta20Phi0", 7.3}, {"Theta40Phi90", 7.8}, {"Theta60Phi225", 8.1}, {"Theta75Phi315", 8.2}};
    // Tolerance: 4 sigma of the median including the baseline uncertainty (sigma/sqrt(16))
    const double MedianTolerance = 4.0*sqrt(1.0 + 1.0/16.0)*MedianError;
    Passed = EvaluateNear("ARM", Name + ", median", "The median of |ARM| is the baseline of this direction (4 sigma of the median; degrees)", MedianARM, BaselineMedians.at(Name.Data()), MedianTolerance) && Passed;
    Passed = EvaluateNear("ARM", Name + ", contrast", "The true direction is clearly better than every direction displaced by 20 degrees (shown: the shortfall below 1.3 times the best displaced direction)", max(1.3*FractionRing - FractionTrue, 0.0), 0.0, 0.0) && Passed;

    // Check the total energy: a line at 662 keV
    vector<double> NearLine;
    for (double Energy: Energies) {
      if (fabs(Energy - 662.0) < 25.0) {
        NearLine.push_back(Energy);
      }
    }
    Passed = EvaluateNear("Energy", Name + ", line", "The median total energy of the full energy events is the line energy", GetMedian(NearLine), 662.0, 3.0) && Passed;

    // Back project on a grid of directions - the maximum is near the true direction:
    double BestCount = -1.0, BestTheta = 0.0, BestPhi = 0.0;
    for (double Theta = 0.0; Theta <= 90.0; Theta += 5.0) {
      // At the pole the azimuth is degenerate
      double AzimuthStep = 10.0;
      if (Theta == 0.0) {
        AzimuthStep = 360.0;
      }
      for (double Phi = 0.0; Phi < 360.0; Phi += AzimuthStep) {
        MVector Direction;
        Direction.SetMagThetaPhi(1.0, Theta*c_Rad, Phi*c_Rad);
        double Count = 0.0;
        for (const shared_ptr<MComptonEvent>& Event: Full) {
          if (fabs(ARM(*Event, Direction)) < 5.0) {
            Count += 1.0;
          }
        }
        if (Count > BestCount) {
          BestCount = Count;
          BestTheta = Theta;
          BestPhi = Phi;
        }
      }
    }
    MVector Best;
    Best.SetMagThetaPhi(1.0, BestTheta*c_Rad, BestPhi*c_Rad);
    const double Separation = acos(max(-1.0, min(1.0, Best.Dot(Source))))*c_Deg;
    mout<<Name<<": the back projection peaks at theta = "<<BestTheta<<" deg, phi = "<<BestPhi<<" deg, "<<Separation<<" deg from the source"<<endl;
    Passed = EvaluateNear("Imaging", Name + ", peak", "The back projection of the events peaks at the true source direction (within 8 deg; shown: the excess)", max(Separation - 8.0, 0.0), 0.0, 0.0) && Passed;

    // Check the mimrec image (spherical, y = 180 degrees - theta):
    unique_ptr<TH2D> Image = FirstHistogram<TH2D>(Scenarios[s].m_Directory + "/image.root");
    Passed = EvaluateTrue("Mimrec", Name + ", image", "The image of mimrec can be read", Image != nullptr) && Passed;
    if (Image != nullptr) {
      double PeakPhi = 0.0, PeakY = 0.0;
      if (ImagePeak(Image.get(), 8.0, PeakPhi, PeakY) == true) {
        MVector Peak;
        Peak.SetMagThetaPhi(1.0, (180.0 - PeakY)*c_Rad, PeakPhi*c_Rad);
        const double MimrecSeparation = acos(max(-1.0, min(1.0, Peak.Dot(Source))))*c_Deg;
        mout<<Name<<": the mimrec image peaks at phi = "<<PeakPhi<<" deg, theta = "<<180.0 - PeakY<<" deg, "<<MimrecSeparation<<" deg from the source"<<endl;
        // Allow a larger limit at the pole - phi is degenerate there
        double Limit = 2.0;
        if (Directions[s].first < 10.0) {
          Limit = 4.0;
        }
        Passed = EvaluateNear("Mimrec", Name + ", image peak", MString("The peak of the mimrec image is at the true source direction (within ") + Limit + " deg; shown: the excess)", max(MimrecSeparation - Limit, 0.0), 0.0, 0.0) && Passed;
      } else {
        Passed = EvaluateTrue("Mimrec", Name + ", image peak", "The mimrec image has content", false) && Passed;
      }
    }

    // Check the mimrec ARM plot against the own ARM of the extracted events
    unique_ptr<TH1D> ARMHistogram = FirstHistogram<TH1D>(Scenarios[s].m_Directory + "/arm.root");
    const ETTraFileCoreData Extracted = ReadTraFile(Scenarios[s].m_Directory + "/selected.tra");
    Passed = EvaluateTrue("Mimrec", Name + ", ARM", "The ARM histogram and the extracted events of mimrec can be read", ARMHistogram != nullptr && Extracted.m_FileSuccessfullyRead == true) && Passed;
    if (ARMHistogram != nullptr && Extracted.m_FileSuccessfullyRead == true) {
      const int Bins = ARMHistogram->GetNbinsX();
      vector<double> Own(Bins, 0.0);
      for (const shared_ptr<MComptonEvent>& Event: Extracted.Compton()) {
        const double Arm = ARM(*Event, Source);
        const int Bin = static_cast<int>(floor((Arm - ARMHistogram->GetXaxis()->GetXmin())/(ARMHistogram->GetXaxis()->GetXmax() - ARMHistogram->GetXaxis()->GetXmin())*Bins));
        if (Bin >= 0 && Bin < Bins) {
          Own[Bin] += 1.0;
        }
      }
      double OwnInside = 0.0, MimrecInside = 0.0, Difference = 0.0;
      for (int b = 0; b < Bins; ++b) {
        OwnInside += Own[b];
        MimrecInside += ARMHistogram->GetBinContent(b + 1);
        Difference += fabs(Own[b] - ARMHistogram->GetBinContent(b + 1));
      }
      Passed = EvaluateNear("Mimrec", Name + ", ARM content", "The ARM histogram of mimrec has exactly the same number of events in the range as the own ARM analysis", MimrecInside, OwnInside, 0.0) && Passed;
      Passed = EvaluateNear("Mimrec", Name + ", ARM shape", "The ARM histogram of mimrec has the same shape as the own analysis (sum of absolute bin differences is zero)", Difference, 0.0, 0.0) && Passed;
      Passed = EvaluateNear("Mimrec", Name + ", ARM peak", "The ARM histogram of mimrec peaks at zero (within 2 degrees)", ARMHistogram->GetXaxis()->GetBinCenter(ARMHistogram->GetMaximumBin()), 0.0, 2.0) && Passed;
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecImaging Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecImaging.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
