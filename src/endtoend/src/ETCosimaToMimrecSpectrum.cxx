/*
 * ETCosimaToMimrecSpectrum.cxx
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

#include "ETCosimaToMimrecSpectrum.h"

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
bool ETCosimaToMimrecSpectrum::Run()
{
  bool Passed = true;

  if (VerifyEnvironment() == false) {
    Summarize();
    return false;
  }

  vector<ETSimScenario> Scenarios;
  const vector<double> Lines = { 150.0, 662.0, 1500.0 };
  for (double Energy: Lines) {
    ETSimScenario Scenario;
    Scenario.m_Name = MString("Line") + static_cast<int>(Energy);
    Scenario.m_Time = 150;
    Scenario.SetFarFieldPointSource(20, 0, MString("Mono ") + Energy, 2.0);
    Scenarios.push_back(Scenario);
  }
  {
    ETSimScenario Scenario;
    Scenario.m_Name = "PowerLaw";
    Scenario.m_Time = 150;
    Scenario.SetFarFieldPointSource(20, 0, "PowerLaw 100 2000 2", 2.0);
    Scenarios.push_back(Scenario);
  }
  if (PrepareSimulations(Scenarios) == false) {
    Summarize();
    return false;
  }
  const MString Common = Scenarios[0].m_Directory;
  Passed = EvaluateTrue("CreateRevanConfiguration()", "revan", "The default revan configuration can be created", CreateRevanConfiguration(Common)) && Passed;
  Passed = EvaluateTrue("CreateMimrecConfiguration()", "mimrec", "The default mimrec configuration can be created", CreateMimrecConfiguration(Common)) && Passed;

  vector<future<bool>> Runs;
  for (const ETSimScenario& Scenario: Scenarios) {
    // Select +-2% around a line, 100 to 2000 keV for the power law
    vector<MString> Window = { "EventSelections.FirstEnergyWindow.Min=100", "EventSelections.FirstEnergyWindow.Max=2000" };
    if (Runs.size() < Lines.size()) {
      Window = { MString("EventSelections.FirstEnergyWindow.Min=") + 0.98*Lines[Runs.size()], MString("EventSelections.FirstEnergyWindow.Max=") + 1.02*Lines[Runs.size()] };
    }
    Runs.push_back(async(launch::async, [this, Scenario, Common, Window]() {
      if (SimulateAndReconstruct(Scenario, Common + "/revan.cfg") == false) {
        return false;
      }
      return Mimrec(Scenario, Common + "/mimrec.cfg", "-s", "spectrum.root", Window);
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

    // The true energy of every event, by ID
    map<int, double> TrueEnergy;
    double MinTrue = 1e30, MaxTrue = 0.0;
    for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
      TrueEnergy[Event->GetID()] = Event->GetIAAt(0)->GetSecondaryEnergy();
      MinTrue = min(MinTrue, Event->GetIAAt(0)->GetSecondaryEnergy());
      MaxTrue = max(MaxTrue, Event->GetIAAt(0)->GetSecondaryEnergy());
    }

    // Pair the reconstructed total energy with the true energy
    struct Pair { double m_True; double m_Reconstructed; };
    vector<Pair> Pairs;
    for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
      auto Found = TrueEnergy.find(Event->GetId());
      if (Found != TrueEnergy.end()) {
        Pairs.push_back({ Found->second, Event->Eg() + Event->Ee() });
      }
    }
    for (const shared_ptr<MPhotoEvent>& Event: Tra.Photo()) {
      auto Found = TrueEnergy.find(Event->GetId());
      if (Found != TrueEnergy.end()) {
        Pairs.push_back({ Found->second, Event->GetEnergy() });
      }
    }
    mout<<Name<<": "<<Sim.m_Events.size()<<" triggers, "<<Pairs.size()<<" reconstructed events matched to the truth"<<endl;
    Passed = EvaluateTrue("ReadTra()", Name + ", matching", "Most of the reconstructed events can be matched to the true event by the ID (> 90%)", Pairs.size() > 0.9*(Tra.Count(MPhysicalEvent::c_Compton) + Tra.Count(MPhysicalEvent::c_Photo))) && Passed;

    // Check that no energy is created - noise of the energy resolution is allowed
    unsigned int TooMuch = 0;
    for (const Pair& Match: Pairs) {
      if (Match.m_Reconstructed > Match.m_True*1.05 + 10.0) {
        ++TooMuch;
      }
    }
    Passed = EvaluateNear("Energy", Name + ", no created energy", "Less than 1% of the events have more than 5% + 10 keV above the true energy (shown: the fraction in percent minus 1)", max(100.0*TooMuch/Pairs.size() - 1.0, 0.0), 0.0, 0.0) && Passed;

    if (s < Lines.size()) {
      const double Line = Lines[s];
      double MaxDeviation = 0.0;
      for (const shared_ptr<MSimEvent>& Event: Sim.m_Events) {
        MaxDeviation = max(MaxDeviation, fabs(Event->GetIAAt(0)->GetSecondaryEnergy() - Line));
      }
      Passed = EvaluateNear("ReadSim()", Name + ", true energy", "The true energy of every photon is the line energy", MaxDeviation, 0.0, 1e-3) && Passed;

      // The peak: events within +-4% of the line
      vector<double> Peak;
      for (const Pair& Match: Pairs) {
        if (fabs(Match.m_Reconstructed - Line) < 0.04*Line) {
          Peak.push_back(Match.m_Reconstructed);
        }
      }
      const double PeakMedian = Median(Peak);
      mout<<Name<<": "<<Peak.size()<<" events within 4% of the line, median "<<PeakMedian<<" keV"<<endl;
      Passed = EvaluateTrue("Energy", Name + ", peak events", "The line is visible: > 100 full energy events", Peak.size() > 100) && Passed;
      if (Peak.size() > 100) {
        Passed = EvaluateNear("Energy", Name + ", peak position", "The median reconstructed energy of the peak is the line energy (0.1%)", PeakMedian, Line, 0.001*Line) && Passed;
        // Check the width with the median absolute deviation (robust sigma = 1.4826*MAD)
        vector<double> Deviations;
        for (double Energy: Peak) {
          Deviations.push_back(fabs(Energy - PeakMedian));
        }
        const double Sigma = 1.4826*Median(Deviations);
        mout<<Name<<": peak sigma "<<Sigma<<" keV ("<<100.0*Sigma/Line<<"%)"<<endl;
        Passed = EvaluateNear("Energy", Name + ", peak width", "The width of the peak is the energy resolution of the instrument: below 0.6% (shown: the excess in percent)", max(100.0*Sigma/Line - 0.6, 0.0), 0.0, 0.0) && Passed;
      }

      // Check the mimrec spectrum of the window around the line
      unique_ptr<TH1D> Spectrum = FirstHistogram<TH1D>(Scenarios[s].m_Directory + "/spectrum.root");
      Passed = EvaluateTrue("Mimrec", Name + ", spectrum", "The spectrum of mimrec can be read", Spectrum != nullptr) && Passed;
      if (Spectrum != nullptr) {
        double OwnCount = 0.0;
        for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
          const double Total = Event->Eg() + Event->Ee();
          if (Total >= 0.98*Line && Total <= 1.02*Line) {
            OwnCount += 1.0;
          }
        }
        const double PeakPosition = Spectrum->GetXaxis()->GetBinCenter(Spectrum->GetMaximumBin());
        const double FWHM = FullWidthAtHalfMaximum(Spectrum.get());
        mout<<Name<<": mimrec spectrum with "<<Spectrum->Integral("width")<<" events (own analysis: "<<OwnCount<<"), peak at "<<PeakPosition<<" keV, FWHM "<<FWHM<<" keV ("<<100.0*FWHM/Line<<"%)"<<endl;
        // The spectrum is in counts per keV - use the integral over the bin widths
        // Mimrec applies quality cuts - expect more than 95% of the Compton events
        Passed = EvaluateNear("Mimrec", Name + ", events", "The spectrum of mimrec contains almost all Compton events of the window of the own analysis (95% to 100%)", Spectrum->Integral("width"), 0.9775*OwnCount, 0.0225*OwnCount + 0.001*OwnCount) && Passed;
        Passed = EvaluateNear("Mimrec", Name + ", peak", "The spectrum of mimrec peaks at the line energy (within 0.2%)", PeakPosition, Line, 0.002*Line) && Passed;
        // FWHM = 2.355 sigma - below 1.5% for a resolution below 0.6%
        Passed = EvaluateNear("Mimrec", Name + ", width", "The width of the peak in the spectrum of mimrec is the energy resolution: the FWHM is below 1.5% (shown: the excess in percent)", max(100.0*FWHM/Line - 1.5, 0.0), 0.0, 0.0) && Passed;
        Passed = EvaluateTrue("Mimrec", Name + ", width resolved", "The peak is resolved: the FWHM is larger than zero", FWHM > 0.0) && Passed;
      }
    } else {
      // Check the power law source: true energies in range, reconstructed energy unbiased
      Passed = EvaluateNear("ReadSim()", Name + ", range", "The true energies of the triggered photons are inside the energy range of the source (shown: the excess outside of 100 to 2000 keV)", max(max(100.0 - MinTrue, MaxTrue - 2000.0), 0.0), 0.0, 1e-3) && Passed;
      // Check the mimrec spectrum between 100 and 2000 keV
      unique_ptr<TH1D> Spectrum = FirstHistogram<TH1D>(Scenarios[s].m_Directory + "/spectrum.root");
      Passed = EvaluateTrue("Mimrec", Name + ", spectrum", "The spectrum of mimrec can be read", Spectrum != nullptr) && Passed;
      if (Spectrum != nullptr) {
        double OwnCount = 0.0;
        for (const shared_ptr<MComptonEvent>& Event: Tra.Compton()) {
          const double Total = Event->Eg() + Event->Ee();
          if (Total >= 100.0 && Total <= 2000.0) {
            OwnCount += 1.0;
          }
        }
        mout<<Name<<": mimrec spectrum with "<<Spectrum->Integral("width")<<" events (own analysis: "<<OwnCount<<")"<<endl;
        Passed = EvaluateNear("Mimrec", Name + ", events", "The spectrum of mimrec contains almost all Compton events between 100 and 2000 keV of the own analysis (95% to 100%)", Spectrum->Integral("width"), 0.9775*OwnCount, 0.0225*OwnCount + 0.001*OwnCount) && Passed;
      }
      const vector<pair<double, double>> Bands = { { 100, 200 }, { 200, 400 }, { 400, 800 }, { 800, 2000 } };
      for (const auto& Band: Bands) {
        vector<double> Ratios;
        for (const Pair& Match: Pairs) {
          if (Match.m_True >= Band.first && Match.m_True < Band.second && fabs(Match.m_Reconstructed/Match.m_True - 1.0) < 0.04) {
            Ratios.push_back(Match.m_Reconstructed/Match.m_True);
          }
        }
        const MString BandName = MString(" ") + static_cast<int>(Band.first) + "-" + static_cast<int>(Band.second) + " keV";
        Passed = EvaluateTrue("Energy", Name + BandName + ", events", "There are enough full energy events in the band (> 50)", Ratios.size() > 50) && Passed;
        if (Ratios.size() > 50) {
          mout<<Name<<BandName<<": "<<Ratios.size()<<" full energy events, median of reconstructed/true "<<Median(Ratios)<<endl;
          Passed = EvaluateNear("Energy", Name + BandName + ", calibration", "The reconstructed energy of the full energy events is the true energy (median ratio within 0.1%)", Median(Ratios), 1.0, 0.001) && Passed;
        }
      }
    }
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  ETCosimaToMimrecSpectrum Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// ETCosimaToMimrecSpectrum.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
