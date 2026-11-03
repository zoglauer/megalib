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
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
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
#include "MEndToEndReferenceList.h"
#include "MEndToEndReport.h"
#include "MStatistics.h"
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
  //! Return true if |cos(phi)| of the Compton event is not larger than one
  bool HasPhysicalScatterAngle(const MComptonEvent& Event) const
  {
    const double CosPhi = 1.0 - c_E0*(1.0/Event.Eg() - 1.0/(Event.Eg() + Event.Ee()));
    if (fabs(CosPhi) > 1.0) {
      return false;
    }
    return true;
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
  virtual ~MEndToEndTest() = default;

  //! Run the test and write the summary (title page, key results, pages) into the log directory: tests implement RunTest()
  virtual bool Run() final
  {
    const bool Passed = RunTest();
    ReportReferenceEnvironment();
    m_Report.Write(GetGeometry(), m_Results);
    WriteResults();
    WriteFingerprint();
    return Passed;
  }

  //! Set the geometry file name, the default is the Max.geo.setup of the examples
  void SetGeometry(const MString& FileName) { m_Geometry = FileName; m_GeometryHash = ""; }
  //! Return the geometry file: the one which was set, otherwise the default Max.geo.setup of the examples, empty only if MEGALIB is not defined
  MString GetGeometry() const;


  // protected methods:
 protected:
  //! The test itself: run it and evaluate the results, the summary is written afterwards
  virtual bool RunTest() = 0;


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
  unsigned int RandomSeed() const;
  //! Prepare the directory of a scenario (no random numbers are involved: e.g. a scenario which starts from stored data)
  bool PrepareScenario(ETSimScenario& Scenario);
  //! Prepare the directory of a scenario which is simulated: as PrepareScenario, and give it a random seed unless it has one already
  bool PrepareSimulation(ETSimScenario& Scenario);
  //! Prepare the directories of the scenarios of a simulation and give them their seeds (as a test for each scenario), the seeds are printed
  bool PrepareSimulations(vector<ETSimScenario>& Scenarios);
  //! Return the hash of the definition of a scenario: the name, the source and run lines, the time or the triggers, and the files (not the seed and the directory)
  MString GetScenarioHash(const ETSimScenario& Scenario) const;


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
  vector<MString> ReadLines(const MString& FileName) const;
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
        // Delete the histogram once - the canvas of mimrec holds it twice
        while (Canvas->GetListOfPrimitives()->Remove(Histogram) != nullptr) {
          // Intentionally left blank
        }
        delete Histogram;
        return unique_ptr<T>(Copy);
      }
    }
    return nullptr;
  }


  // Analyzing histograms:

  //! Get the centroid of an image around its maximum (all bins within the range in both axes), return false if the image is empty
  bool ImagePeak(const TH2* Image, double Range, double& CentroidX, double& CentroidY) const;


  // Statistics and physics:

  //! Return the angular resolution measure (ARM) in degree of a Compton event relative to the unit vector to the source
  double ARM(const MComptonEvent& Event, const MVector& ToSource) const;
  //! Measure a statistic of the values together with its bootstrap uncertainty
  static ETMeasuredValue Measure(const vector<double>& Values, const MStatisticFunction& Statistic, unsigned int NResamples = 100);
  //! Return the largest y value of the graph, zero if it has no points
  static double GetMaximumY(const TGraph* Graph);


  // Comparison with reference values and expectations:

  //! Check that the measured value is the expected value within the tolerance, add the comparison with the outcome of this check to the key results under the name, return the outcome
  bool CompareValues(const MString& Function, const MString& Input, const MString& Description, const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Load the references of this test from the default file: $(MEGALIB)/resource/endtoend/<name of the test>.references.tsv, return false if it cannot be read
  bool LoadReferences();
  //! Load the references from the file, return false if it cannot be read
  bool LoadReferences(const MString& FileName);
  //! Set the references
  void SetReferences(const MEndToEndReferenceList& References) { m_References = References; }
  //! Return the environment of this run which the references depend on: Geant4, ROOT, the geometry, and the scenarios (keys of MEndToEndFingerprint::GetKeys())
  //! The geometry is read once per geometry file: it does not change during a test
  map<MString, MString> GetFingerprint() const;
  //! Return the differences between the record of the references and the environment of this run, empty if they match (a difference is a warning, not a failure)
  vector<MString> CheckReferences() const;
  //! Tell if the references were made in another environment: at the end of the test, when every scenario is prepared and the fingerprint is the one which is written
  void ReportReferenceEnvironment();
  //! Write the key results as tab separated text into the directory of the summary, if the test has key results
  void WriteResults() const;
  //! Write the environment of this run (the fingerprint) as file with a header only into the directory of the summary, if the test has key results
  void WriteFingerprint() const;
  //! Compare a measured value with its reference: the bootstrap error of the run is calibrated with the factor of the reference, the tolerance is five combined standard errors but at least the floors of the reference, prints a REFERENCE line
  bool CompareToReference(const MString& Scenario, const MString& Observable, const MString& Description, const ETMeasuredValue& Measured);

  // Summary (PDF pages and key results): fill the report and the results directly
  //! Return the report of the test
  MEndToEndReport& GetReport() { return m_Report; }
  //! Return the report of the test
  const MEndToEndReport& GetReport() const { return m_Report; }
  //! Return the key results of the test
  MEndToEndResultList& GetResults() { return m_Results; }
  //! Return the key results of the test
  const MEndToEndResultList& GetResults() const { return m_Results; }
  //! Return the reference value of "scenario/observable" (zero if there is none)
  ETMeasuredValue GetReference(const MString& Scenario, const MString& Observable) const;


  // private members:
 private:
  //! The generator of the random seeds: seeded by the base seed in the environment (a campaign makes its runs reproducible), else random
  mutable mt19937 m_SeedGenerator;
  //! The geometry file
  MString m_Geometry;
  //! The time out of one test in seconds, from the testing settings (~/.testdrive.cfg), adapted to the speed of this machine
  double m_Timeout;
  //! The time when this test was created: the time out counts from here
  chrono::steady_clock::time_point m_StartTime;
  //! The reference values for CompareToReference
  MEndToEndReferenceList m_References;
  //! The name of the file of the references, empty if there is none
  MString m_ReferenceFile;
  //! True after the first comparison with a reference: the references are named on the title page and their environment is checked at the end
  bool m_ReferencesChecked = false;
  //! The hashes of the scenarios which were prepared: name and hash
  map<MString, MString> m_ScenarioHashes;
  //! The hash of the geometry for the fingerprint, read once per geometry (empty: not read yet)
  mutable MString m_GeometryHash;
  //! The key results
  MEndToEndResultList m_Results;
  //! The summary: the pages and the presentation of the key results
  MEndToEndReport m_Report;
  //! The geometry which the sim file reader needs, loaded when it is needed for the first time
  shared_ptr<MDGeometryQuest> m_ReaderGeometry;

};

#endif


////////////////////////////////////////////////////////////////////////////////
