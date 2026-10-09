/*
 * MEndToEndTest.h
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

#ifndef __MEndToEndTest__
#define __MEndToEndTest__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TKey.h"
#include "TPad.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MFileEventsTra.h"
#include "MSimEvent.h"
#include "MSimIA.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MPhotoEvent.h"
#include "MUnitTest.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing the core content of a sim file
class ETSimFileCoreData
{
public:
  //! Flag indicating if the file was successfully read
  bool m_FileSuccessfullyRead = false;
  //! The simulated time in seconds
  double m_SimulationTime = 0.0;
  //! The total number of simulated particles
  unsigned long m_SimulatedParticles = 0;
  //! The area from which the particles were started
  double m_StartArea = 0.0;
  //! The triggered events
  vector<shared_ptr<MSimEvent>> m_Events;
  //! The geometry
  shared_ptr<MDGeometryQuest> m_Geometry;
};


//! Class representing the core content of a tra file
class ETTraFileCoreData
{
public:
  //! Flag indicating if the file was successfully read
  bool m_FileSuccessfullyRead = false;
  //! List of the events of the file, of any type
  vector<shared_ptr<MPhysicalEvent>> m_Events;

  //! Return the Compton events
  vector<shared_ptr<MComptonEvent>> Compton() const
  {
    vector<shared_ptr<MComptonEvent>> Events;
    for (const shared_ptr<MPhysicalEvent>& Event: m_Events) {
      if (Event->GetType() == MPhysicalEvent::c_Compton) {
        Events.push_back(dynamic_pointer_cast<MComptonEvent>(Event));
      }
    }
    return Events;
  }
  //! Return the photo events (single interaction events)
  vector<shared_ptr<MPhotoEvent>> Photo() const
  {
    vector<shared_ptr<MPhotoEvent>> Events;
    for (const shared_ptr<MPhysicalEvent>& Event: m_Events) {
      if (Event->GetType() == MPhysicalEvent::c_Photo) {
        Events.push_back(dynamic_pointer_cast<MPhotoEvent>(Event));
      }
    }
    return Events;
  }
  //! Return the number of events of a type (MPhysicalEvent::c_Compton, c_Photo, c_Unidentifiable, ...)
  unsigned long Count(int Type) const
  {
    unsigned long Number = 0;
    for (const shared_ptr<MPhysicalEvent>& Event: m_Events) {
      if (Event->GetType() == Type) {
        ++Number;
      }
    }
    return Number;
  }
};


//! Class representing one simulation scenario
class ETSimScenario
{
public:
  //! Name: also the file name prefix
  MString m_Name;
  //! The source lines (without the source name prefix): ParticleType, Beam, Spectrum, Flux, ...
  vector<MString> m_SourceLines;
  //! Additional lines for the run (without the run name prefix), e.g. Orientation
  vector<MString> m_RunLines;
  //! The simulated time in seconds
  double m_Time = 100.0;
  //! If larger than zero: the number of triggers at which the run stops instead of the time
  unsigned int m_Triggers = 0;
  //! Files which are written into the scenario directory before the simulation: (name, content)
  vector<pair<MString, MString>> m_Files;
  //! The random seed: 0 (default) chooses a random seed when the scenario is prepared, any other value is kept
  unsigned int m_Seed = 0;
  //! The directory of the scenario
  MString m_Directory;

  //! Set the source to a far field point source of photons (polar angle and azimuth in degree, text of the spectrum, flux in ph/cm2/s)
  void SetFarFieldPointSource(double Theta, double Phi, const MString& Spectrum, double Flux)
  {
    m_SourceLines = { "ParticleType 1", MString("Beam FarFieldPointSource ") + Theta + " " + Phi, MString("Spectrum ") + Spectrum, MString("Flux ") + Flux };
  }
};


////////////////////////////////////////////////////////////////////////////////


//! Class representing an end-to-end test (base class): runs MEGAlib programs in private directories and compares the results with independent expectations
//! * The sim and tra files are read with the MEGAlib readers
//! * The functions which run external programs can be called from std::async threads, the Evaluate* functions and everything which uses ROOT only from the main thread
class MEndToEndTest : public MUnitTest
{
  // public interface:
 public:
  //! Standard constructor giving the name of the test
  MEndToEndTest(const MString& Name);
  //! Default destructor
  virtual ~MEndToEndTest();

  //! Set the geometry file name, the default is the Max.geo.setup of the examples
  void SetGeometry(const MString& FileName) { m_Geometry = FileName; }
  //! Return the geometry file: the one which was set, otherwise the default Max.geo.setup of the examples, empty only if MEGALIB is not defined
  MString GetGeometry() const;


  // protected methods:
 protected:
  // Environment and programs:
  //! Check (as a test) that MEGAlib is set up ($(MEGALIB), the geometry file, and the programs cosima, revan, and mimrec in the path)
  bool VerifyEnvironment();
  //! Return the time in seconds which is left of the time out of this test, at least one second, 0 without time out
  unsigned int GetRemainingTimeBudget() const;

  // Running external programs:
  //! Execute a program with its arguments (a shell fragment, e.g. with a redirect) in a directory with a private home directory and a timeout in seconds (0: none), return true if the exit status is zero
  bool Execute(const MString& Directory, const MString& Executable, const MString& Arguments, unsigned int Timeout) const;

  // Scenarios:
  //! Return a random seed (1 .. 2e9) which is printed by the tests: any seed has to pass
  static unsigned int RandomSeed();
  //! Prepare the directory of a scenario (no random numbers are involved: e.g. a scenario which starts from stored data)
  bool PrepareScenario(ETSimScenario& Scenario);
  //! Prepare the directory of a scenario which is simulated: as PrepareScenario, and give it a random seed unless it has one already
  bool PrepareSimulation(ETSimScenario& Scenario);
  //! Prepare the directories of the scenarios of a simulation and give them their seeds (as a test for each scenario), the seeds are printed
  bool PrepareSimulations(vector<ETSimScenario>& Scenarios);

  // The chain of programs:
  //! Write the source file (and the files of the scenario) of a scenario
  bool WriteSource(const ETSimScenario& Scenario) const;
  //! Simulate with cosima
  bool Simulate(const ETSimScenario& Scenario) const;
  //! Create the default revan configuration (revan.cfg) in the given directory
  bool CreateRevanConfiguration(const MString& Directory) const;
  //! Reconstruct with revan using the given configuration file
  bool Reconstruct(const ETSimScenario& Scenario, const MString& Configuration) const;
  //! Simulate with cosima and reconstruct the result with revan using the given configuration file
  bool SimulateAndReconstruct(const ETSimScenario& Scenario, const MString& Configuration) const;
  //! Create the default mimrec configuration (mimrec.cfg) in the given directory
  bool CreateMimrecConfiguration(const MString& Directory) const;
  //! Run mimrec on the tra file of a scenario with the analysis option, the output file, and the configuration changes ("Key=Value")
  bool Mimrec(const ETSimScenario& Scenario, const MString& Configuration, const MString& Option, const MString& Output, const vector<MString>& Changes) const;

  // Reading the results:
  //! Read the sim file of a scenario with MFileEventsSim
  ETSimFileCoreData ReadSim(const ETSimScenario& Scenario);
  //! Read the tra file of a scenario with MFileEventsTra
  ETTraFileCoreData ReadTra(const ETSimScenario& Scenario) const;
  //! Read any tra file (gzipped or not) with MFileEventsTra
  ETTraFileCoreData ReadTraFile(const MString& FileName) const;
  //! Read a plain text file (e.g. the log of a program) line by line
  static vector<MString> ReadLines(const MString& FileName);
  //! Return a copy of the first histogram of the class T in the first canvas of a ROOT file, nullptr if there is none
  template<class T>
  unique_ptr<T> FirstHistogram(const MString& FileName)
  {
    TFile File(FileName.Data());
    if (File.IsZombie() == true) {
      return nullptr;
    }
    TIter NextKey(File.GetListOfKeys());
    TKey* Key = static_cast<TKey*>(NextKey());
    if (Key == nullptr) {
      return nullptr;
    }
    unique_ptr<TObject> CanvasObject(Key->ReadObj());
    TCanvas* Canvas = dynamic_cast<TCanvas*>(CanvasObject.get());
    if (Canvas == nullptr) {
      return nullptr;
    }
    TIter NextObject(Canvas->GetListOfPrimitives());
    TObject* Object = nullptr;
    while ((Object = NextObject()) != nullptr) {
      T* Histogram = dynamic_cast<T*>(Object);
      if (Histogram != nullptr) {
        T* Copy = static_cast<T*>(Histogram->Clone());
        Copy->SetDirectory(nullptr);
        return unique_ptr<T>(Copy);
      }
    }
    return nullptr;
  }

  // Analyzing histograms:
  //! Get the centroid of an image around its maximum (all bins within the range in both axes), return false if the image is empty
  bool ImagePeak(const TH2* Image, double Range, double& CentroidX, double& CentroidY) const;
  //! Return the full width at half maximum of the peak of a histogram, 0 if it cannot be determined
  double FullWidthAtHalfMaximum(const TH1* Histogram) const;

  // Statistics and physics:
  //! Return the angular resolution measure (ARM) in degree of a Compton event relative to the unit vector to the source
  static double ARM(const MComptonEvent& Event, const MVector& ToSource);
  //! Return the median of a vector
  static double Median(vector<double> Values);
  //! Return the significance in sigma of the ratio of two Poisson counts CountA/CountB relative to the expected ratio
  static double RatioSigma(double CountA, double CountB, double ExpectedRatio);
  //! Return the two-proportion z value of the efficiencies K1/N1 and K2/N2
  static double ProportionSigma(double K1, double N1, double K2, double N2);

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! The geometry file
  MString m_Geometry;
  //! The time out of one test in seconds, from the testing settings (~/.testdrive.cfg), adapted to the speed of this machine
  double m_Timeout;
  //! The time when this test was created: the time out counts from here
  chrono::steady_clock::time_point m_StartTime;
  //! The geometry which the sim file reader needs, loaded when it is needed for the first time
  shared_ptr<MDGeometryQuest> m_ReaderGeometry;

};

#endif


////////////////////////////////////////////////////////////////////////////////
