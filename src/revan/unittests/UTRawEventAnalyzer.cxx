/*
 * UTRawEventAnalyzer.cxx
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


// Standard libs:
#include <cmath>
#include <memory>
#include <new>
#include <string>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MComptonEvent.h"
#include "MFileEventsEvta.h"
#include "MFileEventsTra.h"
#include "MGlobal.h"
#include "MPhotoEvent.h"
#include "MPhysicalEvent.h"
#include "MRawEventAnalyzer.h"
#include "MRawEventIncarnationList.h"
#include "MRERawEvent.h"
#include "MSettingsEventReconstruction.h"
#include "UTRevanShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MRawEventAnalyzer: the chain of the reconstruction stages and the handling of the files
//! The stages themselves have their own tests; here the hits are chosen such that the result of each stage is obvious
class UTRawEventAnalyzer : public UTRevanBase
{
public:
  //! Default constructor
  UTRawEventAnalyzer() : UTRevanBase("UTRawEventAnalyzer") {}
  //! Default destructor
  virtual ~UTRawEventAnalyzer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the values of the algorithm IDs and return codes, which are stored in configuration files
  bool TestConstants();
  //! Test that the constructor initializes all members which the analysis uses
  bool TestConstructor();
  //! Test opening input and output files, and reading the initial events
  bool TestInputAndOutputFiles();
  //! Test the rejections of PreAnalysis() which do not depend on an algorithm
  bool TestPreAnalysisGuards();
  //! Test which algorithm IDs PreAnalysis() accepts
  bool TestPreAnalysisAlgorithms();
  //! Test that the options arrive in the reconstruction stages
  bool TestOptions();
  //! Test SetSettings()
  bool TestSettings();
  //! Test a single-site event
  bool TestPhotoEvent();
  //! Test a two-site Compton event
  bool TestComptonEvent();
  //! Test writing the tra file and reading it back
  bool TestTraFile();
  //! Test a simulation file: thresholds, veto, and the information of the footer
  bool TestSimulationFile();
  //! Test events with hits which cannot be used
  bool TestInvalidHits();
  //! Test the event selections: total energy, event ID, and lever arm
  bool TestSelections();
  //! Test events with a bad flag
  bool TestBadEvents();
  //! Test the coincidence search
  bool TestCoincidence();
  //! Test switching the hit clustering and the Compton sequence reconstruction on and off
  bool TestStageSwitches();
  //! Test the event clustering
  bool TestEventClustering();
  //! Test adding events to the store instead of reading them from a file
  bool TestStoreMode();
  //! Test saving the origin information
  bool TestSaveOI();
  //! Test TransferFileInformation()
  bool TestTransferFileInformation();
  //! Test PostAnalysis() and its statistics
  bool TestPostAnalysis();
  //! Test using one analyzer for two files
  bool TestReuse();
  //! Test JoinStatistics()
  bool TestJoinStatistics();
  //! Test calls on an analyzer which is not completely set up
  bool TestIncompleteSetup();

  //! Return the header of an evta file for the telescope
  MString GetEvtaHeader();
  //! Return the header of a sim file for the given geometry
  MString GetSimHeader(MGeometryRevan& Geometry);
  //! Return the start of an event: the SE, ID, and TI lines
  MString GetEventStart(unsigned int ID, const MString& Time) const;
  //! Return an evta hit of 662 keV in the center of the calorimeter block, with the resolutions of the telescope
  MString GetCalorimeterHit() const;
  //! Return two evta hits: 200 keV in the center of tracker layer 0, and 500 keV in the calorimeter at (1, 2, -10) cm
  MString GetComptonHits() const;
  //! Return the Compton scatter angle in rad for the energies of the recoil electron and the scattered gamma ray in keV
  double GetComptonAngle(double ElectronEnergy, double GammaEnergy) const;

  //! Set the geometry and the batch mode
  void Prepare(MRawEventAnalyzer& Analyzer, MGeometryRevan& Geometry);
  //! Call SetInputModeFile() with all output silenced
  bool OpenInput(MRawEventAnalyzer& Analyzer, const MString& FileName);
  //! Call SetOutputModeFile() with all output silenced
  bool OpenOutput(MRawEventAnalyzer& Analyzer, const MString& FileName);
  //! Call PreAnalysis() with all output silenced
  bool RunPreAnalysis(MRawEventAnalyzer& Analyzer);
  //! Call AnalyzeEvent() with all output silenced
  unsigned int AnalyzeNext(MRawEventAnalyzer& Analyzer);
  //! Call AnalyzeEvent() until no event is left (at most 100 times), and return the codes
  vector<unsigned int> AnalyzeAll(MRawEventAnalyzer& Analyzer);
  //! Call PostAnalysis() and return its output
  MString RunPostAnalysis(MRawEventAnalyzer& Analyzer);
  //! Add an event string to the store with all output silenced
  bool AddEventString(MRawEventAnalyzer& Analyzer, const MString& Event, bool NeedsNoising, int Version);
  //! Open the file, analyze all events, and return the output of PostAnalysis()
  MString AnalyzeFile(MRawEventAnalyzer& Analyzer, const MString& FileName, const MString& OutputFileName = "");

  //! Return the count of the line of the statistics which starts with the label, -1 if there is none
  int GetStatistic(const MString& Output, const MString& Label) const;
  //! Return the value of the option "# <Key>: <Value>" of the options block, "<missing>" if there is none
  MString GetOption(const MString& Output, const MString& Key) const;
  //! Read all events of a tra file, its number of simulated events, and its observation time in s
  bool ReadTraFile(const MString& FileName, vector<unique_ptr<MPhysicalEvent>>& Events, long& SimulatedEvents, double& ObservationTime);

  //! Return true if PreAnalysis() succeeds after the algorithm of the stage (0: coincidence, 1: event clustering, 2: hit clustering, 3: tracking, 4: Compton sequence, 5: decay) is set
  bool IsAlgorithmAccepted(unsigned int Stage, int Algorithm);
  //! Return true if PreAnalysis() accepts the event clustering by distance without a cut off, on an analyzer built in memory filled with the pattern
  bool IsDistanceClusteringAcceptedOnPattern(unsigned char Pattern);
  //! Return the number of events found by the coincidence search without a coincidence window, on an analyzer built in memory filled with the pattern
  unsigned int GetNCoincidentEventsOnPattern(unsigned char Pattern, const MString& FileName);
  //! Join the statistics of an analyzer without rejections into one with a rejection reason of 30 or above, return true if the rejection is still counted
  bool JoinIntoLargerRejectionList();
  //! Add an event string to an analyzer without a geometry, return true if it is refused
  bool AddEventWithoutGeometry();
  //! Call PostAnalysis() on an analyzer without a geometry, return true if it returns
  bool PostAnalysisWithoutGeometry();

private:
  //! The telescope of the shared header
  MGeometryRevan m_Geometry;
  //! The telescope with ideal energy resolutions in tracker and calorimeter, for noising without random numbers
  MGeometryRevan m_IdealGeometry;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTRawEventAnalyzer::Run()
{
  bool Passed = true;

  // The sim events are noised with ideal energy resolutions, so no test depends on random numbers; the seed is fixed anyway
  gRandom->SetSeed(1);

  Passed = EvaluateTrue("ScanTelescopeGeometry()", "telescope", "The telescope can be scanned", ScanTelescopeGeometry(m_Geometry)) && Passed;

  MString Ideal = GetTelescopeGeometry("", true);
  Passed = EvaluateTrue("ScanGeometry()", "ideal telescope", "The telescope with ideal energy resolutions can be scanned", ScanGeometry(m_IdealGeometry, Ideal, "Ideal.geo.setup")) && Passed;

  if (Passed == false) {
    // Without the geometries nothing else can be tested
    Summarize();
    return false;
  }

  Passed = TestConstants() && Passed;
  Passed = TestConstructor() && Passed;
  Passed = TestInputAndOutputFiles() && Passed;
  Passed = TestPreAnalysisGuards() && Passed;
  Passed = TestPreAnalysisAlgorithms() && Passed;
  Passed = TestOptions() && Passed;
  Passed = TestSettings() && Passed;
  Passed = TestPhotoEvent() && Passed;
  Passed = TestComptonEvent() && Passed;
  Passed = TestTraFile() && Passed;
  Passed = TestSimulationFile() && Passed;
  Passed = TestInvalidHits() && Passed;
  Passed = TestSelections() && Passed;
  Passed = TestBadEvents() && Passed;
  Passed = TestCoincidence() && Passed;
  Passed = TestStageSwitches() && Passed;
  Passed = TestEventClustering() && Passed;
  Passed = TestStoreMode() && Passed;
  Passed = TestSaveOI() && Passed;
  Passed = TestTransferFileInformation() && Passed;
  Passed = TestPostAnalysis() && Passed;
  Passed = TestReuse() && Passed;
  Passed = TestJoinStatistics() && Passed;
  Passed = TestIncompleteSetup() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the values of the algorithm IDs and return codes, which are stored in configuration files
bool UTRawEventAnalyzer::TestConstants()
{
  bool Passed = true;

  // The IDs are written into the revan configuration files, so they must never change ("DONT'T CHANGE THE NUMBERS" in the source)
  Passed = Evaluate("c_CoincidenceAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_CoincidenceAlgoNone, 0) && Passed;
  Passed = Evaluate("c_CoincidenceAlgoWindow", "constant", "The ID is 1", MRawEventAnalyzer::c_CoincidenceAlgoWindow, 1) && Passed;

  Passed = Evaluate("c_EventClusteringAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_EventClusteringAlgoNone, 0) && Passed;
  Passed = Evaluate("c_EventClusteringAlgoTMVA", "constant", "The ID is 1", MRawEventAnalyzer::c_EventClusteringAlgoTMVA, 1) && Passed;
  Passed = Evaluate("c_EventClusteringAlgoDistance", "constant", "The ID is 2", MRawEventAnalyzer::c_EventClusteringAlgoDistance, 2) && Passed;

  Passed = Evaluate("c_HitClusteringAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_HitClusteringAlgoNone, 0) && Passed;
  Passed = Evaluate("c_HitClusteringAlgoDistance", "constant", "The ID is 1", MRawEventAnalyzer::c_HitClusteringAlgoDistance, 1) && Passed;
  Passed = Evaluate("c_HitClusteringAlgoAdjacent", "constant", "The ID is 2", MRawEventAnalyzer::c_HitClusteringAlgoAdjacent, 2) && Passed;
  Passed = Evaluate("c_HitClusteringAlgoPDF", "constant", "The ID is 3", MRawEventAnalyzer::c_HitClusteringAlgoPDF, 3) && Passed;

  Passed = Evaluate("c_TrackingAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_TrackingAlgoNone, 0) && Passed;
  Passed = Evaluate("c_TrackingAlgoModifiedPearson", "constant", "The ID is 1", MRawEventAnalyzer::c_TrackingAlgoModifiedPearson, 1) && Passed;
  Passed = Evaluate("c_TrackingAlgoChiSquare", "constant", "The ID is 2", MRawEventAnalyzer::c_TrackingAlgoChiSquare, 2) && Passed;
  Passed = Evaluate("c_TrackingAlgoGas", "constant", "The ID is 3", MRawEventAnalyzer::c_TrackingAlgoGas, 3) && Passed;
  Passed = Evaluate("c_TrackingAlgoDirectional", "constant", "The ID is 4", MRawEventAnalyzer::c_TrackingAlgoDirectional, 4) && Passed;
  Passed = Evaluate("c_TrackingAlgoBayesian", "constant", "The ID is 5", MRawEventAnalyzer::c_TrackingAlgoBayesian, 5) && Passed;
  Passed = Evaluate("c_TrackingAlgoRank", "constant", "The ID is 6", MRawEventAnalyzer::c_TrackingAlgoRank, 6) && Passed;
  Passed = Evaluate("c_TrackingAlgoPearson", "constant", "The ID is 7", MRawEventAnalyzer::c_TrackingAlgoPearson, 7) && Passed;

  Passed = Evaluate("c_CSRAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_CSRAlgoNone, 0) && Passed;
  Passed = Evaluate("c_CSRAlgoFoM", "constant", "The ID is 1", MRawEventAnalyzer::c_CSRAlgoFoM, 1) && Passed;
  Passed = Evaluate("c_CSRAlgoFoME", "constant", "The ID is 2", MRawEventAnalyzer::c_CSRAlgoFoME, 2) && Passed;
  Passed = Evaluate("c_CSRAlgoFoMToF", "constant", "The ID is 3", MRawEventAnalyzer::c_CSRAlgoFoMToF, 3) && Passed;
  Passed = Evaluate("c_CSRAlgoBayesian", "constant", "The ID is 4", MRawEventAnalyzer::c_CSRAlgoBayesian, 4) && Passed;
  Passed = Evaluate("c_CSRAlgoTMVA", "constant", "The ID is 5", MRawEventAnalyzer::c_CSRAlgoTMVA, 5) && Passed;
  Passed = Evaluate("c_CSRAlgoFoMToFAndE", "constant", "The ID is 6", MRawEventAnalyzer::c_CSRAlgoFoMToFAndE, 6) && Passed;

  Passed = Evaluate("c_DecayAlgoNone", "constant", "The ID is 0", MRawEventAnalyzer::c_DecayAlgoNone, 0) && Passed;
  Passed = Evaluate("c_DecayAlgoStandard", "constant", "The ID is 1", MRawEventAnalyzer::c_DecayAlgoStandard, 1) && Passed;

  Passed = Evaluate("c_AnalysisSucess", "constant", "The code is 0", MRawEventAnalyzer::c_AnalysisSucess, 0U) && Passed;
  Passed = Evaluate("c_AnalysisCoincidenceWindowWait", "constant", "The code is 1", MRawEventAnalyzer::c_AnalysisCoincidenceWindowWait, 1U) && Passed;
  Passed = Evaluate("c_AnalysisEventClusteringFailed", "constant", "The code is 2", MRawEventAnalyzer::c_AnalysisEventClusteringFailed, 2U) && Passed;
  Passed = Evaluate("c_AnalysisNoEventsInStore", "constant", "The code is 3", MRawEventAnalyzer::c_AnalysisNoEventsInStore, 3U) && Passed;
  Passed = Evaluate("c_AnalysisNoEventsLeftInFile", "constant", "The code is 4", MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile, 4U) && Passed;
  Passed = Evaluate("c_AnalysisSavingEventFailed", "constant", "The code is 5", MRawEventAnalyzer::c_AnalysisSavingEventFailed, 5U) && Passed;
  Passed = Evaluate("c_AnalysisUndefinedError", "constant", "The code is 6", MRawEventAnalyzer::c_AnalysisUndefinedError, 6U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructor initializes all members which the analysis uses
bool UTRawEventAnalyzer::TestConstructor()
{
  bool Passed = true;

  // A new analyzer has no events and no results
  MRawEventAnalyzer Analyzer;
  Passed = EvaluateTrue("GetInitialRawEvent()", "new analyzer", "There is no initial raw event", Analyzer.GetInitialRawEvent() == nullptr) && Passed;
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "new analyzer", "There is no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
  Passed = EvaluateTrue("GetSingleBestTryEvent()", "new analyzer", "There is no best try event", Analyzer.GetSingleBestTryEvent() == nullptr) && Passed;
  Passed = EvaluateSize("GetOptimumEvents()", "new analyzer", "The list of optimum events is empty", Analyzer.GetOptimumEvents().size(), 0) && Passed;
  Passed = EvaluateSize("GetBestTryEvents()", "new analyzer", "The list of best try events is empty", Analyzer.GetBestTryEvents().size(), 0) && Passed;
  // Intended: the list of initial raw events always has one entry, which is a nullptr before the first event
  Passed = EvaluateSize("GetInitialRawEvents()", "new analyzer", "The list of initial events has one entry", Analyzer.GetInitialRawEvents().size(), 1) && Passed;
  Passed = EvaluateTrue("GetRawEventList()", "new analyzer", "The list of incarnations exists", Analyzer.GetRawEventList() != nullptr) && Passed;
  Passed = Evaluate("GetRawEventList()", "new analyzer", "The list of incarnations is empty", Analyzer.GetRawEventList()->Size(), 0U) && Passed;
  Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "new analyzer", "There is no event without an input file", Analyzer.GetNextInitialRawEventFromFile() == nullptr) && Passed;

  // The behavior of a new analyzer must not depend on what was in its memory before: build it in memory filled with two patterns
  // As a double, eight bytes 0x41 are +2261634.5 and eight bytes 0xC1 are -2261634.5

  // Defect REV-H-1: The constructor does not initialize m_EventClusteringDistanceCutOff, so PreAnalysis() accepts or refuses the event clustering by distance depending on the memory content
  bool AcceptedOnPositive = IsDistanceClusteringAcceptedOnPattern(0x41);
  bool AcceptedOnNegative = IsDistanceClusteringAcceptedOnPattern(0xC1);
  Passed = ExpectedToFail(Evaluate("MRawEventAnalyzer()", "event clustering by distance without a cut off, two memory patterns", "PreAnalysis() gives the same result for both patterns", AcceptedOnPositive, AcceptedOnNegative)) && Passed;

  // Two single-site events which are one second apart
  const MString FileName = GetTemporaryFileName("Constructor.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + "HT 1;0;0;0;200;0.29;0.29;0.014;2\n";
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "two events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Defect REV-H-1: The constructor does not initialize m_CoincidenceWindow, so the coincidence search merges the two events or not depending on the memory content
  unsigned int EventsOnPositive = GetNCoincidentEventsOnPattern(0x41, FileName);
  unsigned int EventsOnNegative = GetNCoincidentEventsOnPattern(0xC1, FileName);
  Passed = ExpectedToFail(Evaluate("MRawEventAnalyzer()", "coincidence search without a window, two memory patterns", "The number of events after the coincidence search is the same for both patterns", EventsOnPositive, EventsOnNegative)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test opening input and output files, and reading the initial events
bool UTRawEventAnalyzer::TestInputAndOutputFiles()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Input.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "two events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Input failures
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateFalse("SetInputModeFile()", "missing file", "A file which does not exist is refused", OpenInput(Analyzer, GetTemporaryFileName("DoesNotExist.evta"))) && Passed;
    Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "after a missing file", "There is no event", Analyzer.GetNextInitialRawEventFromFile() == nullptr) && Passed;
    Passed = EvaluateFalse("SetInputModeFile()", "geometry file", "A file which is neither a sim nor an evta file is refused", OpenInput(Analyzer, m_Geometry.GetFileName())) && Passed;
    Passed = EvaluateFalse("SetInputModeFile()", "empty name", "An empty file name is refused", OpenInput(Analyzer, "")) && Passed;
  }
  {
    MGeometryRevan Unscanned;
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, Unscanned);
    Passed = EvaluateFalse("SetInputModeFile()", "geometry not scanned", "The file is refused if the geometry is not scanned", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateFalse("PreAnalysis()", "geometry not scanned, input refused", "The pre-analysis fails", RunPreAnalysis(Analyzer)) && Passed;
  }

  // Reading the initial events
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "evta file", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "evta file, no output", "The pre-analysis succeeds without an output file", RunPreAnalysis(Analyzer)) && Passed;

    unique_ptr<MRERawEvent> First(Analyzer.GetNextInitialRawEventFromFile());
    Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "first event", "The first event is returned", First != nullptr) && Passed;
    if (First != nullptr) {
      Passed = Evaluate("GetNextInitialRawEventFromFile()", "first event", "The event ID is 1", First->GetEventID(), 1UL) && Passed;
      Passed = Evaluate("GetNextInitialRawEventFromFile()", "first event", "The event has one hit", First->GetNRESEs(), 1) && Passed;
      Passed = EvaluateNear("GetNextInitialRawEventFromFile()", "first event", "The energy is that of the hit, 662 keV", First->GetEnergy(), 662.0, 1e-12) && Passed;
    }
    unique_ptr<MRERawEvent> Second(Analyzer.GetNextInitialRawEventFromFile());
    Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "second event", "The second event is returned", Second != nullptr) && Passed;
    if (Second != nullptr) {
      Passed = Evaluate("GetNextInitialRawEventFromFile()", "second event", "The event ID is 2", Second->GetEventID(), 2UL) && Passed;
      Passed = Evaluate("GetNextInitialRawEventFromFile()", "second event", "The event has two hits", Second->GetNRESEs(), 2) && Passed;
    }
    unique_ptr<MRERawEvent> Third(Analyzer.GetNextInitialRawEventFromFile());
    Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "after the last event", "A nullptr is returned", Third == nullptr) && Passed;

    // Opening the file again starts over
    Passed = EvaluateTrue("SetInputModeFile()", "same file again", "The file is opened again", OpenInput(Analyzer, FileName)) && Passed;
    unique_ptr<MRERawEvent> Again(Analyzer.GetNextInitialRawEventFromFile());
    Passed = EvaluateTrue("GetNextInitialRawEventFromFile()", "file opened again", "The first event is returned again", Again != nullptr) && Passed;
    if (Again != nullptr) {
      Passed = Evaluate("GetNextInitialRawEventFromFile()", "file opened again", "The event ID is 1", Again->GetEventID(), 1UL) && Passed;
    }
  }

  // Output failures
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateFalse("SetOutputModeFile()", "empty name", "An empty file name is refused", OpenOutput(Analyzer, "")) && Passed;
    Passed = EvaluateFalse("SetOutputModeFile()", "missing directory", "A file in a directory which does not exist is refused", OpenOutput(Analyzer, GetTemporaryDirectoryName("NoSuchDirectory") + "/Output.tra")) && Passed;
    Passed = EvaluateTrue("SetOutputModeFile()", "writable file", "The file is opened", OpenOutput(Analyzer, GetTemporaryFileName("Output.tra"))) && Passed;
    Passed = EvaluateTrue("SetOutputModeFile()", "second file", "A second output file replaces the first one", OpenOutput(Analyzer, GetTemporaryFileName("Output2.tra"))) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "output file only", "The pre-analysis succeeds without an input file", RunPreAnalysis(Analyzer)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the rejections of PreAnalysis() which do not depend on an algorithm
bool UTRawEventAnalyzer::TestPreAnalysisGuards()
{
  bool Passed = true;

  {
    MRawEventAnalyzer Analyzer;
    Analyzer.SetBatch(true);
    Passed = EvaluateFalse("PreAnalysis()", "no geometry", "The pre-analysis fails without a geometry", RunPreAnalysis(Analyzer)) && Passed;
  }
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("PreAnalysis()", "geometry only", "The pre-analysis succeeds with a geometry and the default options", RunPreAnalysis(Analyzer)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "called twice", "The pre-analysis succeeds again", RunPreAnalysis(Analyzer)) && Passed;
  }
  {
    // Defect REV-H-2: A failure to load the geometry of the origin objects sets the return value to false, but the following check of the input file overwrites it
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetOriginObjectsFileName(GetTemporaryFileName("NoOriginObjects.geo.setup"));
    Passed = ExpectedToFail(EvaluateFalse("PreAnalysis()", "origin objects file does not exist", "The pre-analysis fails", RunPreAnalysis(Analyzer))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test which algorithm IDs PreAnalysis() accepts
bool UTRawEventAnalyzer::TestPreAnalysisAlgorithms()
{
  bool Passed = true;

  // Coincidence: none and window are known
  Passed = EvaluateTrue("PreAnalysis()", "coincidence: none", "The algorithm is accepted", IsAlgorithmAccepted(0, MRawEventAnalyzer::c_CoincidenceAlgoNone)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "coincidence: window", "The algorithm is accepted", IsAlgorithmAccepted(0, MRawEventAnalyzer::c_CoincidenceAlgoWindow)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "coincidence: 2", "An unknown algorithm is refused", IsAlgorithmAccepted(0, 2)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "coincidence: -1", "A negative algorithm is refused", IsAlgorithmAccepted(0, -1)) && Passed;

  // Event clustering: the distance needs a cut off (set by IsAlgorithmAccepted), TMVA needs a file
  Passed = EvaluateTrue("PreAnalysis()", "event clustering: none", "The algorithm is accepted", IsAlgorithmAccepted(1, MRawEventAnalyzer::c_EventClusteringAlgoNone)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "event clustering: distance with a cut off of 1 cm", "The algorithm is accepted", IsAlgorithmAccepted(1, MRawEventAnalyzer::c_EventClusteringAlgoDistance)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "event clustering: TMVA without a file", "The algorithm is refused", IsAlgorithmAccepted(1, MRawEventAnalyzer::c_EventClusteringAlgoTMVA)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "event clustering: 3", "An unknown algorithm is refused", IsAlgorithmAccepted(1, 3)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "event clustering: -1", "A negative algorithm is refused", IsAlgorithmAccepted(1, -1)) && Passed;

  // Hit clustering: PDF needs files
  Passed = EvaluateTrue("PreAnalysis()", "hit clustering: none", "The algorithm is accepted", IsAlgorithmAccepted(2, MRawEventAnalyzer::c_HitClusteringAlgoNone)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "hit clustering: distance", "The algorithm is accepted", IsAlgorithmAccepted(2, MRawEventAnalyzer::c_HitClusteringAlgoDistance)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "hit clustering: adjacent", "The algorithm is accepted", IsAlgorithmAccepted(2, MRawEventAnalyzer::c_HitClusteringAlgoAdjacent)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "hit clustering: PDF without files", "The algorithm is refused", IsAlgorithmAccepted(2, MRawEventAnalyzer::c_HitClusteringAlgoPDF)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "hit clustering: 4", "An unknown algorithm is refused", IsAlgorithmAccepted(2, 4)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "hit clustering: -1", "A negative algorithm is refused", IsAlgorithmAccepted(2, -1)) && Passed;

  // Tracking: Bayesian needs a file
  Passed = EvaluateTrue("PreAnalysis()", "tracking: none", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoNone)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: modified Pearson", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoModifiedPearson)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: chi square", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoChiSquare)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: gas", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoGas)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: directional", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoDirectional)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "tracking: Bayesian without a file", "The algorithm is refused", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoBayesian)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: rank", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoRank)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "tracking: Pearson", "The algorithm is accepted", IsAlgorithmAccepted(3, MRawEventAnalyzer::c_TrackingAlgoPearson)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "tracking: 8", "An unknown algorithm is refused", IsAlgorithmAccepted(3, 8)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "tracking: -1", "A negative algorithm is refused", IsAlgorithmAccepted(3, -1)) && Passed;

  // Compton sequence reconstruction: Bayesian and TMVA need files
  Passed = EvaluateTrue("PreAnalysis()", "CSR: none", "The algorithm is accepted", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoNone)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "CSR: figure of merit", "The algorithm is accepted", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoFoM)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "CSR: figure of merit with energy recovery", "The algorithm is accepted", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoFoME)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "CSR: figure of merit with time of flight", "The algorithm is accepted", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoFoMToF)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "CSR: Bayesian without a file", "The algorithm is refused", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoBayesian)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "CSR: TMVA without a file", "The algorithm is refused", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoTMVA)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "CSR: figure of merit with time of flight and energy recovery", "The algorithm is accepted", IsAlgorithmAccepted(4, MRawEventAnalyzer::c_CSRAlgoFoMToFAndE)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "CSR: 7", "An unknown algorithm is refused", IsAlgorithmAccepted(4, 7)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "CSR: -1", "A negative algorithm is refused", IsAlgorithmAccepted(4, -1)) && Passed;

  // Decay: the standard algorithm needs a file
  Passed = EvaluateTrue("PreAnalysis()", "decay: none", "The algorithm is accepted", IsAlgorithmAccepted(5, MRawEventAnalyzer::c_DecayAlgoNone)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "decay: standard without a file", "The algorithm is refused", IsAlgorithmAccepted(5, MRawEventAnalyzer::c_DecayAlgoStandard)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "decay: 2", "An unknown algorithm is refused", IsAlgorithmAccepted(5, 2)) && Passed;
  Passed = EvaluateFalse("PreAnalysis()", "decay: -1", "A negative algorithm is refused", IsAlgorithmAccepted(5, -1)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the options arrive in the reconstruction stages
bool UTRawEventAnalyzer::TestOptions()
{
  bool Passed = true;

  // The analyzer has no getters: the options are visible in the options block which PostAnalysis() prints and stores in the tra file

  // Hit clustering by distance
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoDistance);
    Analyzer.SetStandardClusterizerMinDistanceD1(0.11);
    Analyzer.SetStandardClusterizerMinDistanceD2(0.12);
    Analyzer.SetStandardClusterizerMinDistanceD3(0.13);
    Analyzer.SetStandardClusterizerMinDistanceD4(0.14);
    Analyzer.SetStandardClusterizerMinDistanceD5(0.15);
    Analyzer.SetStandardClusterizerMinDistanceD6(0.16);
    Analyzer.SetStandardClusterizerMinDistanceD7(0.17);
    Analyzer.SetStandardClusterizerMinDistanceD8(0.18);
    Passed = EvaluateTrue("PreAnalysis()", "hit clustering by distance", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateTrue("SetHitClusteringAlgorithm()", "distance", "The clustering by distance is used", Output.Contains("Clusterizing by distance")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD1()", "0.11", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D1"), MString("0.11")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD2()", "0.12", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D2"), MString("0.12")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD3()", "0.13", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D3"), MString("0.13")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD4()", "0.14", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D4"), MString("0.14")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD5()", "0.15", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D5"), MString("0.15")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD6()", "0.16", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D6"), MString("0.16")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD7()", "0.17", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D7"), MString("0.17")) && Passed;
    Passed = Evaluate("SetStandardClusterizerMinDistanceD8()", "0.18", "The distance arrives in the hit clusterizer", GetOption(Output, "Min distance D8"), MString("0.18")) && Passed;
  }

  // Hit clustering by adjacent voxels, and the Compton sequence reconstruction
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoAdjacent);
    Analyzer.SetAdjacentLevel(3);
    Analyzer.SetAdjacentSigma(1.5);
    Analyzer.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
    Analyzer.SetCSRThresholdMin(0.25);
    Analyzer.SetCSRThresholdMax(0.75);
    Analyzer.SetCSRMaxNHits(6);
    Analyzer.SetUseComptelTypeEvents(false);
    Analyzer.SetClassicUndecidedHandling(2);
    Analyzer.SetGuaranteeStartD1(false);
    Analyzer.SetRejectOneDetectorTypeOnlyEvents(false);
    Passed = EvaluateTrue("PreAnalysis()", "adjacent hit clustering and CSR options", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateTrue("SetHitClusteringAlgorithm()", "adjacent", "The clustering by adjacent voxels is used", Output.Contains("Clusterizing by adjacent voxels")) && Passed;
    Passed = Evaluate("SetAdjacentLevel()", "3", "The level arrives in the hit clusterizer", GetOption(Output, "Level"), MString("3")) && Passed;
    Passed = Evaluate("SetAdjacentSigma()", "1.5", "The sigma arrives in the hit clusterizer", GetOption(Output, "Sigma"), MString("1.5")) && Passed;
    Passed = Evaluate("SetCSRThresholdMin()", "0.25", "The threshold arrives in the Compton sequence reconstruction", GetOption(Output, "QualityFactorMin"), MString("0.25")) && Passed;
    Passed = Evaluate("SetCSRThresholdMax()", "0.75", "The threshold arrives in the Compton sequence reconstruction", GetOption(Output, "QualityFactorMax"), MString("0.75")) && Passed;
    Passed = Evaluate("SetCSRMaxNHits()", "6", "The maximum number of hits arrives in the Compton sequence reconstruction", GetOption(Output, "MaxNInteractions"), MString("6")) && Passed;
    Passed = Evaluate("SetUseComptelTypeEvents()", "false", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "UseComptelTypeEvents"), MString("0")) && Passed;
    Passed = Evaluate("SetClassicUndecidedHandling()", "2", "The handling arrives in the Compton sequence reconstruction", GetOption(Output, "UndecidedHandling"), MString("2")) && Passed;
    Passed = Evaluate("SetGuaranteeStartD1()", "false", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "GuaranteeStartD1"), MString("0")) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", "false", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "RejectOneDetectorTypeOnlyEvents"), MString("0")) && Passed;
    Passed = EvaluateFalse("SetTrackingAlgorithm()", "default", "No tracking is done by default", Output.Contains("Tracking -")) && Passed;
  }

  // The opposite values of the flags
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetUseComptelTypeEvents(true);
    Analyzer.SetGuaranteeStartD1(true);
    Analyzer.SetRejectOneDetectorTypeOnlyEvents(true);
    Passed = EvaluateTrue("PreAnalysis()", "CSR flags true", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = Evaluate("SetUseComptelTypeEvents()", "true", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "UseComptelTypeEvents"), MString("1")) && Passed;
    Passed = Evaluate("SetGuaranteeStartD1()", "true", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "GuaranteeStartD1"), MString("1")) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", "true", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "RejectOneDetectorTypeOnlyEvents"), MString("1")) && Passed;
  }

  // Electron tracking
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoModifiedPearson);
    Analyzer.SetSearchMIPTracks(true);
    Analyzer.SetSearchPairTracks(false);
    Analyzer.SetSearchComptonTracks(false);
    Analyzer.SetMaxComptonJump(3);
    Analyzer.SetNTrackSequencesToKeep(4);
    Analyzer.SetRejectPurelyAmbiguousTrackSequences(true);
    Analyzer.SetNLayersForVertexSearch(5);
    vector<MString> Detectors;
    Detectors.push_back("Tracker");
    Analyzer.SetElectronTrackingDetectorList(Detectors);
    Passed = EvaluateTrue("PreAnalysis()", "tracking options", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateTrue("SetTrackingAlgorithm()", "modified Pearson", "The tracking is listed in the options", Output.Contains("Tracking -")) && Passed;
    Passed = Evaluate("SetSearchMIPTracks()", "true", "The flag arrives in the tracker", GetOption(Output, "Search MIPs"), MString("1")) && Passed;
    Passed = Evaluate("SetSearchPairTracks()", "false", "The flag arrives in the tracker", GetOption(Output, "Search Pairs"), MString("0")) && Passed;
    Passed = Evaluate("SetSearchComptonTracks()", "false", "The flag arrives in the tracker", GetOption(Output, "Search Comptons"), MString("0")) && Passed;
    Passed = Evaluate("SetMaxComptonJump()", "3", "The jump arrives in the tracker", GetOption(Output, "Max. layer jump Compton"), MString("3")) && Passed;
    Passed = Evaluate("SetNTrackSequencesToKeep()", "4", "The number arrives in the tracker", GetOption(Output, "Number of sequences to keep"), MString("4")) && Passed;
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", "true", "The flag arrives in the tracker", GetOption(Output, "Reject pure ambiguities"), MString("1")) && Passed;

    // SetDoTracking(false) switches the tracking off (see the header)
    Analyzer.SetDoTracking(false);
    Passed = EvaluateTrue("PreAnalysis()", "after SetDoTracking(false)", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateFalse("SetDoTracking()", "false", "No tracking is listed in the options", Output.Contains("Tracking -")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test SetSettings()
bool UTRawEventAnalyzer::TestSettings()
{
  bool Passed = true;

  MSettingsEventReconstruction Settings;
  Settings.SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoNone);
  Settings.SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoNone);
  Settings.SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoAdjacent);
  Settings.SetAdjacentLevel(4);
  Settings.SetAdjacentSigma(2.5);
  Settings.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoRank);
  Settings.SetSearchPairs(false);
  Settings.SetSearchMIPs(false);
  Settings.SetSearchComptons(true);
  Settings.SetMaxComptonJump(5);
  Settings.SetNTrackSequencesToKeep(7);
  Settings.SetRejectPurelyAmbiguousTrackSequences(true);
  Settings.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoFoM);
  Settings.SetCSRThresholdMin(0.125);
  Settings.SetCSRThresholdMax(0.875);
  Settings.SetCSRMaxNHits(3);
  Settings.SetUseComptelTypeEvents(true);
  Settings.SetClassicUndecidedHandling(1);
  Settings.SetGuaranteeStartD1(true);
  Settings.SetRejectOneDetectorTypeOnlyEvents(true);
  Settings.SetDecayAlgorithm(MRawEventAnalyzer::c_DecayAlgoNone);
  Settings.SetTotalEnergyMin(680);
  Settings.SetTotalEnergyMax(720);
  Settings.SetEventIdMin(-1);
  Settings.SetEventIdMax(-1);
  Settings.SetRejectAllBadEvents(true);
  Settings.SetSaveOI(false);

  const MString FileName = GetTemporaryFileName("Settings.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "two events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  Analyzer.SetSettings(&Settings);
  MString Output = AnalyzeFile(Analyzer, FileName);

  Passed = EvaluateTrue("SetSettings()", "hit clustering: adjacent", "The clustering by adjacent voxels is used", Output.Contains("Clusterizing by adjacent voxels")) && Passed;
  Passed = Evaluate("SetSettings()", "adjacent level 4", "The level arrives in the hit clusterizer", GetOption(Output, "Level"), MString("4")) && Passed;
  Passed = Evaluate("SetSettings()", "adjacent sigma 2.5", "The sigma arrives in the hit clusterizer", GetOption(Output, "Sigma"), MString("2.5")) && Passed;
  Passed = EvaluateTrue("SetSettings()", "tracking: rank", "The tracking is listed in the options", Output.Contains("Tracking -")) && Passed;
  Passed = Evaluate("SetSettings()", "search MIPs false", "The flag arrives in the tracker", GetOption(Output, "Search MIPs"), MString("0")) && Passed;
  Passed = Evaluate("SetSettings()", "search pairs false", "The flag arrives in the tracker", GetOption(Output, "Search Pairs"), MString("0")) && Passed;
  Passed = Evaluate("SetSettings()", "search Comptons true", "The flag arrives in the tracker", GetOption(Output, "Search Comptons"), MString("1")) && Passed;
  Passed = Evaluate("SetSettings()", "maximum Compton jump 5", "The jump arrives in the tracker", GetOption(Output, "Max. layer jump Compton"), MString("5")) && Passed;
  Passed = Evaluate("SetSettings()", "7 track sequences to keep", "The number arrives in the tracker", GetOption(Output, "Number of sequences to keep"), MString("7")) && Passed;
  Passed = Evaluate("SetSettings()", "reject purely ambiguous sequences", "The flag arrives in the tracker", GetOption(Output, "Reject pure ambiguities"), MString("1")) && Passed;
  Passed = Evaluate("SetSettings()", "CSR threshold minimum 0.125", "The threshold arrives in the Compton sequence reconstruction", GetOption(Output, "QualityFactorMin"), MString("0.125")) && Passed;
  Passed = Evaluate("SetSettings()", "CSR threshold maximum 0.875", "The threshold arrives in the Compton sequence reconstruction", GetOption(Output, "QualityFactorMax"), MString("0.875")) && Passed;
  Passed = Evaluate("SetSettings()", "CSR maximum hits 3", "The number arrives in the Compton sequence reconstruction", GetOption(Output, "MaxNInteractions"), MString("3")) && Passed;
  Passed = Evaluate("SetSettings()", "classic undecided handling 1", "The handling arrives in the Compton sequence reconstruction", GetOption(Output, "UndecidedHandling"), MString("1")) && Passed;
  Passed = Evaluate("SetSettings()", "guarantee start D1", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "GuaranteeStartD1"), MString("1")) && Passed;
  Passed = Evaluate("SetSettings()", "reject one detector type only", "The flag arrives in the Compton sequence reconstruction", GetOption(Output, "RejectOneDetectorTypeOnlyEvents"), MString("1")) && Passed;

  // The energy window 680 to 720 keV keeps the Compton event of 700 keV and rejects the single-site event of 662 keV
  Passed = Evaluate("SetSettings()", "energy window 680 to 720 keV", "Both events are analyzed", GetStatistic(Output, "Number of events ."), 2) && Passed;
  Passed = Evaluate("SetSettings()", "energy window 680 to 720 keV", "Only the event of 700 keV passes the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
  Passed = Evaluate("SetSettings()", "energy window 680 to 720 keV", "The event of 662 keV is rejected for its energy", GetStatistic(Output, MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionTotalEnergyOutOfLimits) + " ."), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a single-site event
bool UTRawEventAnalyzer::TestPhotoEvent()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Photo.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.5") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  Passed = EvaluateTrue("SetInputModeFile()", "one event", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "one event", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "single-site event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;

  MRERawEvent* Initial = Analyzer.GetInitialRawEvent();
  Passed = EvaluateTrue("GetInitialRawEvent()", "single-site event", "The initial event exists", Initial != nullptr) && Passed;
  if (Initial != nullptr) {
    Passed = Evaluate("GetInitialRawEvent()", "single-site event", "The event ID is 1", Initial->GetEventID(), 1UL) && Passed;
    Passed = Evaluate("GetInitialRawEvent()", "single-site event", "The event has one hit", Initial->GetNRESEs(), 1) && Passed;
    Passed = EvaluateNear("GetInitialRawEvent()", "single-site event", "The energy is 662 keV", Initial->GetEnergy(), 662.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetInitialRawEvent()", "single-site event", "The time is 1.5 s", Initial->GetEventTime().GetAsDouble(), 1.5, 1e-12) && Passed;
  }
  Passed = EvaluateSize("GetInitialRawEvents()", "single-site event", "The list has one entry", Analyzer.GetInitialRawEvents().size(), 1) && Passed;
  Passed = EvaluateTrue("GetInitialRawEvents()", "single-site event", "The entry is the initial event", Analyzer.GetInitialRawEvents()[0] == Initial) && Passed;

  MRERawEvent* Optimum = Analyzer.GetSingleOptimumEvent();
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "single-site event", "There is an optimum event", Optimum != nullptr) && Passed;
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "single-site event", "The optimum event is not the copy which is kept as initial event", Optimum != Initial) && Passed;
  Passed = EvaluateTrue("GetSingleBestTryEvent()", "single-site event", "The best try is the optimum event", Analyzer.GetSingleBestTryEvent() == Optimum) && Passed;
  Passed = EvaluateSize("GetOptimumEvents()", "single-site event", "The list has one entry", Analyzer.GetOptimumEvents().size(), 1) && Passed;
  Passed = EvaluateTrue("GetOptimumEvents()", "single-site event", "The entry is the optimum event", Analyzer.GetOptimumEvents()[0] == Optimum) && Passed;
  Passed = EvaluateSize("GetBestTryEvents()", "single-site event", "The list has one entry", Analyzer.GetBestTryEvents().size(), 1) && Passed;
  Passed = Evaluate("GetRawEventList()", "single-site event", "There is one list of incarnations", Analyzer.GetRawEventList()->Size(), 1U) && Passed;

  if (Optimum != nullptr) {
    Passed = Evaluate("GetSingleOptimumEvent()", "single-site event", "The event is not rejected", Optimum->GetRejectionReason(), MRERawEvent::c_RejectionNone) && Passed;
    MPhysicalEvent* Physical = Optimum->GetPhysicalEvent();
    Passed = EvaluateTrue("GetPhysicalEvent()", "single-site event", "There is a physical event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetPhysicalEvent()", "single-site event", "It is a photo event", Physical->GetType(), MPhysicalEvent::c_Photo) && Passed;
      Passed = Evaluate("GetPhysicalEvent()", "single-site event", "The ID is 1", Physical->GetId(), 1L) && Passed;
      Passed = EvaluateNear("GetPhysicalEvent()", "single-site event", "The time is 1.5 s", Physical->GetTime().GetAsDouble(), 1.5, 1e-12) && Passed;
      MPhotoEvent* Photo = dynamic_cast<MPhotoEvent*>(Physical);
      Passed = EvaluateTrue("GetPhysicalEvent()", "single-site event", "The class is MPhotoEvent", Photo != nullptr) && Passed;
      if (Photo != nullptr) {
        Passed = EvaluateNear("GetPhysicalEvent()", "single-site event", "The energy is 662 keV", Photo->GetEnergy(), 662.0, 1e-12) && Passed;
        Passed = EvaluateVectorNear("GetPhysicalEvent()", "single-site event", "The position is that of the hit", Photo->GetPosition(), MVector(0.0, 0.0, -10.0), 1e-12) && Passed;
      }
    }
  }

  // The end of the file is reported at every further call
  Passed = Evaluate("AnalyzeEvent()", "after the last event", "No events are left in the file", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "after the last event, again", "No events are left in the file", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) && Passed;
  // The results of the last event stay
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "after the last event", "The last optimum event is still available", Analyzer.GetSingleOptimumEvent() == Optimum) && Passed;

  MString Output = RunPostAnalysis(Analyzer);
  Passed = Evaluate("PostAnalysis()", "single-site event", "One event was analyzed", GetStatistic(Output, "Number of events ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "single-site event", "One event passed the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "single-site event", "One single-site event was found", GetStatistic(Output, "Single-site  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "single-site event", "No Compton event was found", GetStatistic(Output, "Compton  ."), 0) && Passed;
  Passed = Evaluate("PostAnalysis()", "single-site event", "No event was rejected", GetStatistic(Output, "Total ."), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a two-site Compton event
bool UTRawEventAnalyzer::TestComptonEvent()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Compton.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(7, "2.5") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  Passed = EvaluateTrue("SetInputModeFile()", "one event", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "one event", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "two-site event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;

  MRERawEvent* Initial = Analyzer.GetInitialRawEvent();
  Passed = EvaluateTrue("GetInitialRawEvent()", "two-site event", "The initial event exists", Initial != nullptr) && Passed;
  if (Initial != nullptr) {
    Passed = Evaluate("GetInitialRawEvent()", "two-site event", "The event has two hits", Initial->GetNRESEs(), 2) && Passed;
    Passed = EvaluateNear("GetInitialRawEvent()", "two-site event", "The energy is 200 keV + 500 keV", Initial->GetEnergy(), 700.0, 1e-12) && Passed;
  }

  MRERawEvent* Optimum = Analyzer.GetSingleOptimumEvent();
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "two-site event", "There is an optimum event", Optimum != nullptr) && Passed;
  if (Optimum != nullptr) {
    MPhysicalEvent* Physical = Optimum->GetPhysicalEvent();
    Passed = EvaluateTrue("GetPhysicalEvent()", "two-site event", "There is a physical event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetPhysicalEvent()", "two-site event", "It is a Compton event", Physical->GetType(), MPhysicalEvent::c_Compton) && Passed;
      Passed = Evaluate("GetPhysicalEvent()", "two-site event", "The ID is 7", Physical->GetId(), 7L) && Passed;
      Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The time is 2.5 s", Physical->GetTime().GetAsDouble(), 2.5, 1e-12) && Passed;
      MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Physical);
      Passed = EvaluateTrue("GetPhysicalEvent()", "two-site event", "The class is MComptonEvent", Compton != nullptr) && Passed;
      if (Compton != nullptr) {
        // The default options guarantee the start in D1, so the tracker hit (200 keV) is the first interaction: its energy is that of the recoil electron
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The electron energy is that of the tracker hit, 200 keV", Compton->Ee(), 200.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The gamma energy is that of the calorimeter hit, 500 keV", Compton->Eg(), 500.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The electron energy resolution is that of the tracker hit, 2 keV", Compton->dEe(), 2.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The gamma energy resolution is that of the calorimeter hit, 10 keV", Compton->dEg(), 10.0, 1e-12) && Passed;
        Passed = EvaluateVectorNear("GetPhysicalEvent()", "two-site event", "The first interaction is the tracker hit", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
        Passed = EvaluateVectorNear("GetPhysicalEvent()", "two-site event", "The second interaction is the calorimeter hit", Compton->C2(), MVector(1.0, 2.0, -10.0), 1e-12) && Passed;
        // cos(phi) = 1 - E0/Eg + E0/(Ee + Eg) = 1 - 510.999/500 + 510.999/700, phi = 44.93 degree
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The Compton scatter angle follows from the two energies", Compton->Phi(), GetComptonAngle(200.0, 500.0), 1e-12) && Passed;
        Passed = Evaluate("GetPhysicalEvent()", "two-site event", "The sequence has two interactions", Compton->SequenceLength(), 2U) && Passed;
        // The distance of (0, 0, 0) and (1, 2, -10): sqrt(1 + 4 + 100) cm
        Passed = EvaluateNear("GetPhysicalEvent()", "two-site event", "The lever arm is the distance of the two hits", Compton->FirstLeverArm(), sqrt(105.0), 1e-12) && Passed;
      }
    }
  }

  MString Output = RunPostAnalysis(Analyzer);
  Passed = Evaluate("PostAnalysis()", "two-site event", "One event was analyzed", GetStatistic(Output, "Number of events ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "two-site event", "One Compton event was found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "two-site event", "No single-site event was found", GetStatistic(Output, "Single-site  ."), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test writing the tra file and reading it back
bool UTRawEventAnalyzer::TestTraFile()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Tra.evta");
  const MString TraFileName = GetTemporaryFileName("Tra.tra");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += GetEventStart(3, "3.0") + "BD Bad\n" + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "three events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  MString Output = AnalyzeFile(Analyzer, FileName, TraFileName);
  Passed = Evaluate("PostAnalysis()", "three events", "Three events were analyzed", GetStatistic(Output, "Number of events ."), 3) && Passed;

  vector<unique_ptr<MPhysicalEvent>> Read;
  long SimulatedEvents = 0;
  double ObservationTime = 0.0;
  Passed = EvaluateTrue("SetOutputModeFile()", "three events", "The tra file can be read", ReadTraFile(TraFileName, Read, SimulatedEvents, ObservationTime)) && Passed;
  Passed = EvaluateSize("SetOutputModeFile()", "three events", "The tra file has three events", Read.size(), 3) && Passed;

  if (Read.size() == 3) {
    Passed = Evaluate("SetOutputModeFile()", "first event", "It is a photo event", Read[0]->GetType(), MPhysicalEvent::c_Photo) && Passed;
    Passed = Evaluate("SetOutputModeFile()", "first event", "The ID is 1", Read[0]->GetId(), 1L) && Passed;
    Passed = EvaluateNear("SetOutputModeFile()", "first event", "The time is 1 s", Read[0]->GetTime().GetAsDouble(), 1.0, 1e-9) && Passed;
    MPhotoEvent* Photo = dynamic_cast<MPhotoEvent*>(Read[0].get());
    if (Photo != nullptr) {
      Passed = EvaluateNear("SetOutputModeFile()", "first event", "The energy is 662 keV", Photo->GetEnergy(), 662.0, 1e-9) && Passed;
      Passed = EvaluateVectorNear("SetOutputModeFile()", "first event", "The position is that of the hit", Photo->GetPosition(), MVector(0.0, 0.0, -10.0), 1e-9) && Passed;
    }

    Passed = Evaluate("SetOutputModeFile()", "second event", "It is a Compton event", Read[1]->GetType(), MPhysicalEvent::c_Compton) && Passed;
    Passed = Evaluate("SetOutputModeFile()", "second event", "The ID is 2", Read[1]->GetId(), 2L) && Passed;
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Read[1].get());
    if (Compton != nullptr) {
      // All values of this event are short decimals, so the text of the tra file holds them exactly
      Passed = EvaluateNear("SetOutputModeFile()", "second event", "The electron energy is 200 keV", Compton->Ee(), 200.0, 1e-9) && Passed;
      Passed = EvaluateNear("SetOutputModeFile()", "second event", "The gamma energy is 500 keV", Compton->Eg(), 500.0, 1e-9) && Passed;
      Passed = EvaluateVectorNear("SetOutputModeFile()", "second event", "The first interaction is the tracker hit", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-9) && Passed;
      Passed = EvaluateVectorNear("SetOutputModeFile()", "second event", "The second interaction is the calorimeter hit", Compton->C2(), MVector(1.0, 2.0, -10.0), 1e-9) && Passed;
      Passed = EvaluateNear("SetOutputModeFile()", "second event", "The Compton scatter angle follows from the two energies", Compton->Phi(), GetComptonAngle(200.0, 500.0), 1e-9) && Passed;
    }

    // An event which is not reconstructed is stored as unidentifiable event with its bad flag
    Passed = Evaluate("SetOutputModeFile()", "third event", "It is an unidentifiable event", Read[2]->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    Passed = Evaluate("SetOutputModeFile()", "third event", "The ID is 3", Read[2]->GetId(), 3L) && Passed;
    Passed = EvaluateTrue("SetOutputModeFile()", "third event", "The event is flagged as bad", Read[2]->IsBad()) && Passed;
  }

  // The header names the geometry, and the footer holds the options and the statistics
  MString Tra = ReadTextFile(TraFileName);
  Passed = EvaluateTrue("PreAnalysis()", "header of the tra file", "The header names the geometry file", Tra.Contains(m_Geometry.GetFileName())) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "footer of the tra file", "The footer names the original file", Tra.Contains(MString("# OriginalFile ") + FileName)) && Passed;
  Passed = EvaluateTrue("PostAnalysis()", "footer of the tra file", "The footer names the geometry file", Tra.Contains(MString("# GeometryFile ") + m_Geometry.GetFileName())) && Passed;
  Passed = Evaluate("PostAnalysis()", "footer of the tra file", "The footer holds the number of events", GetStatistic(Tra, "Number of events ."), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "footer of the tra file", "The footer holds the number of single-site events", GetStatistic(Tra, "Single-site  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "footer of the tra file", "The footer holds the number of Compton events", GetStatistic(Tra, "Compton  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "footer of the tra file", "The footer holds the rejection", GetStatistic(Tra, MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionExternalBadEventFlag) + " ."), 1) && Passed;

  // A measured evta file has neither TB nor TE, and revan does not add them (Compton events have their own TE line, thus only header and footer are checked)
  MString TraHeader = Tra.GetSubString(0, Tra.FindFirst("\nSE"));
  MString TraFooter = Tra.GetSubString(Tra.FindLast("\nEN\n"));
  Passed = EvaluateFalse("PreAnalysis()", "tra file of an evta file without TB", "The tra file has no TB, since the evta file has none", TraHeader.Contains("\nTB ")) && Passed;
  Passed = EvaluateFalse("PostAnalysis()", "tra file of an evta file without TE", "The tra file has no TE, since the evta file has none", TraFooter.Contains("\nTE ")) && Passed;
  Passed = EvaluateNear("SetOutputModeFile()", "tra file of an evta file without TB and TE", "The observation time of the tra file is the last minus the first event time", ObservationTime, 2.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a simulation file: thresholds, veto, and the information of the footer
bool UTRawEventAnalyzer::TestSimulationFile()
{
  bool Passed = true;

  // The hits of a sim file are noised and the trigger is applied. The geometry has ideal energy resolutions and all hits
  // are in the center of a strip or a block, so the noising changes nothing, and only thresholds and veto act:
  // 1: 662 keV in the calorimeter (trigger threshold 50 keV) -> single-site event
  // 2: 662 keV in the calorimeter and 51 keV in the shield (veto threshold 50 keV) -> vetoed
  // 3: 5 keV in the tracker (noise threshold 10 keV) -> no hit left, no trigger
  // 4: 200 keV in the tracker and 500 keV in the calorimeter -> Compton event
  // 5: 500 keV in the calorimeter and 49 keV in the shield, just below its threshold -> single-site event of 500 keV
  // 6: no hits
  const MString FileName = GetTemporaryFileName("Simulation.sim");
  const MString TraFileName = GetTemporaryFileName("Simulation.tra");
  MString Events = GetSimHeader(m_IdealGeometry);
  Events += "TB 0.5\n\n";
  Events += GetEventStart(1, "1.0") + "HTsim 2;0;0;-10;662;0\n";
  Events += GetEventStart(2, "2.0") + "HTsim 2;0;0;-10;662;0\nHTsim 4;0;0;10;51;0\n";
  Events += GetEventStart(3, "3.0") + "HTsim 1;0;0;0;5;0\n";
  Events += GetEventStart(4, "4.0") + "HTsim 1;0;0;0;200;0\nHTsim 2;0;0;-10;500;0\n";
  Events += GetEventStart(5, "5.0") + "HTsim 2;0;0;-10;500;0\nHTsim 4;0;0;10;49;0\n";
  Events += GetEventStart(6, "6.0");
  Events += "EN\n\nTE 7.0\nTS 6\n";
  Passed = EvaluateTrue("WriteTextFile()", "six events", "The sim file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_IdealGeometry);
  Passed = EvaluateTrue("SetInputModeFile()", "sim file", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
  Passed = EvaluateTrue("SetOutputModeFile()", "sim file", "The output file is opened", OpenOutput(Analyzer, TraFileName)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "sim file", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

  // Event 1
  Passed = Evaluate("AnalyzeEvent()", "sim file, first call", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "sim file, first call", "The event is event 1", Analyzer.GetInitialRawEvent()->GetEventID(), 1UL) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "sim file, first call", "The energy is 662 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 662.0, 1e-12) && Passed;
  }
  // Events 2 and 3 never arrive: event 4
  Passed = Evaluate("AnalyzeEvent()", "sim file, second call", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "sim file, second call", "The vetoed and the not triggered event are skipped: the event is event 4", Analyzer.GetInitialRawEvent()->GetEventID(), 4UL) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "sim file, second call", "The event has two hits", Analyzer.GetInitialRawEvent()->GetNRESEs(), 2) && Passed;
  }
  if (Analyzer.GetSingleOptimumEvent() != nullptr && Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent() != nullptr) {
    MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent());
    Passed = EvaluateTrue("AnalyzeEvent()", "sim file, second call", "The event is a Compton event", Compton != nullptr) && Passed;
    if (Compton != nullptr) {
      Passed = EvaluateNear("AnalyzeEvent()", "sim file, second call", "The electron energy is 200 keV", Compton->Ee(), 200.0, 1e-12) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "sim file, second call", "The gamma energy is 500 keV", Compton->Eg(), 500.0, 1e-12) && Passed;
      Passed = EvaluateVectorNear("AnalyzeEvent()", "sim file, second call", "The first interaction is in the tracker", Compton->C1(), MVector(0.0, 0.0, 0.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("AnalyzeEvent()", "sim file, second call", "The second interaction is in the calorimeter", Compton->C2(), MVector(0.0, 0.0, -10.0), 1e-12) && Passed;
    }
  } else {
    Passed = EvaluateTrue("AnalyzeEvent()", "sim file, second call", "There is an optimum event with a physical event", false) && Passed;
  }
  // Event 5
  Passed = Evaluate("AnalyzeEvent()", "sim file, third call", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "sim file, third call", "The event is event 5", Analyzer.GetInitialRawEvent()->GetEventID(), 5UL) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "sim file, third call", "The shield hit below its threshold is removed: one hit", Analyzer.GetInitialRawEvent()->GetNRESEs(), 1) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "sim file, third call", "The energy is 500 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 500.0, 1e-12) && Passed;
  }
  // Event 6 is empty
  Passed = Evaluate("AnalyzeEvent()", "sim file, fourth call", "The empty event is skipped: no events are left", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) && Passed;

  MString Output = RunPostAnalysis(Analyzer);
  Passed = Evaluate("PostAnalysis()", "sim file", "Three events were analyzed", GetStatistic(Output, "Number of events ."), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "sim file", "Three events passed the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "sim file", "Two single-site events were found", GetStatistic(Output, "Single-site  ."), 2) && Passed;
  Passed = Evaluate("PostAnalysis()", "sim file", "One Compton event was found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  // The statistics of the noising of the reader are part of the output
  Passed = Evaluate("PostAnalysis()", "sim file", "One event was vetoed", GetStatistic(Output, "Number of vetoed events: ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "sim file", "Three events triggered", GetStatistic(Output, "Number of triggered events: ."), 3) && Passed;

  // The start and end of the observation and the number of simulated events are handed over to the tra file
  vector<unique_ptr<MPhysicalEvent>> Read;
  long SimulatedEvents = 0;
  double ObservationTime = 0.0;
  Passed = EvaluateTrue("SetOutputModeFile()", "sim file", "The tra file can be read", ReadTraFile(TraFileName, Read, SimulatedEvents, ObservationTime)) && Passed;
  Passed = EvaluateSize("SetOutputModeFile()", "sim file", "The tra file has three events", Read.size(), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "sim file with TS 6", "The tra file has the number of simulated events", SimulatedEvents, 6L) && Passed;
  // TB 0.5 and TE 7.0: 6.5 s
  Passed = EvaluateNear("PostAnalysis()", "sim file with TB 0.5 and TE 7.0", "The tra file has the observation time", ObservationTime, 6.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events with hits which cannot be used
bool UTRawEventAnalyzer::TestInvalidHits()
{
  bool Passed = true;

  // 1: the only hit is outside of all detectors (z = 50 cm) -> the hit is removed, no event
  // 2: the only hit has no energy -> no event
  // 3: no hits -> no event
  // 4: a good single-site event
  // 5: a good hit and a hit outside of all detectors -> the event arrives with one hit, but it is not valid
  const MString FileName = GetTemporaryFileName("Invalid.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + "HT 1;0;0;50;200;0.29;0.29;0.014;2\n";
  Events += GetEventStart(2, "2.0") + "HT 2;0;0;-10;0;2.88;2.88;2.3;10\n";
  Events += GetEventStart(3, "3.0");
  Events += GetEventStart(4, "4.0") + GetCalorimeterHit();
  Events += GetEventStart(5, "5.0") + GetCalorimeterHit() + "HT 1;0;0;50;200;0.29;0.29;0.014;2\n";
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "five events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  Passed = EvaluateTrue("SetInputModeFile()", "five events", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "five events", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "events without usable hits, first call", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "events without usable hits, first call", "The events without usable hits are skipped: the event is event 4", Analyzer.GetInitialRawEvent()->GetEventID(), 4UL) && Passed;
  }
  Passed = EvaluateTrue("AnalyzeEvent()", "events without usable hits, first call", "Event 4 has an optimum event", Analyzer.GetSingleOptimumEvent() != nullptr) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event with a hit outside of the detectors", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "event with a hit outside of the detectors", "The event is event 5", Analyzer.GetInitialRawEvent()->GetEventID(), 5UL) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event with a hit outside of the detectors", "Only the hit in the calorimeter is left", Analyzer.GetInitialRawEvent()->GetNRESEs(), 1) && Passed;
  }
  Passed = EvaluateTrue("GetSingleOptimumEvent()", "event with a hit outside of the detectors", "The event which lost a hit has no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
  // Intended: the list has an entry per incarnation, which is a nullptr if there is no optimum event; the callers check for it
  Passed = EvaluateSize("GetOptimumEvents()", "event with a hit outside of the detectors", "The list has one entry", Analyzer.GetOptimumEvents().size(), 1) && Passed;
  if (Analyzer.GetOptimumEvents().size() == 1) {
    Passed = EvaluateTrue("GetOptimumEvents()", "event with a hit outside of the detectors", "The entry is a nullptr", Analyzer.GetOptimumEvents()[0] == nullptr) && Passed;
  }
  MRERawEvent* BestTry = Analyzer.GetSingleBestTryEvent();
  Passed = EvaluateTrue("GetSingleBestTryEvent()", "event with a hit outside of the detectors", "There is a best try event", BestTry != nullptr) && Passed;
  if (BestTry != nullptr) {
    MPhysicalEvent* Physical = BestTry->GetPhysicalEvent();
    Passed = EvaluateTrue("GetSingleBestTryEvent()", "event with a hit outside of the detectors", "The best try has a physical event", Physical != nullptr) && Passed;
    if (Physical != nullptr) {
      Passed = Evaluate("GetSingleBestTryEvent()", "event with a hit outside of the detectors", "The physical event is unidentifiable", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
    }
    // Defect REV-H-6: An event which is not valid (here: it lost a hit while reading) is rejected without a rejection reason, so the statistics and the bad flag in the tra file name the reason "None"
    Passed = ExpectedToFail(EvaluateTrue("GetSingleBestTryEvent()", "event with a hit outside of the detectors", "The rejected event has a rejection reason", BestTry->GetRejectionReason() != MRERawEvent::c_RejectionNone)) && Passed;
  }

  Passed = Evaluate("AnalyzeEvent()", "after the last event", "No events are left", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) && Passed;

  MString Output = RunPostAnalysis(Analyzer);
  Passed = Evaluate("PostAnalysis()", "events without usable hits", "Two events were analyzed", GetStatistic(Output, "Number of events ."), 2) && Passed;
  Passed = Evaluate("PostAnalysis()", "events without usable hits", "One event passed the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "events without usable hits", "One event was rejected", GetStatistic(Output, "Total ."), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the event selections: total energy, event ID, and lever arm
bool UTRawEventAnalyzer::TestSelections()
{
  bool Passed = true;

  // Five events: single-site events of 662 keV with the IDs 1, 3, 4, 5, and a Compton event of 700 keV with the ID 2
  const MString FileName = GetTemporaryFileName("Selections.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += GetEventStart(3, "3.0") + GetCalorimeterHit();
  Events += GetEventStart(4, "4.0") + GetCalorimeterHit();
  Events += GetEventStart(5, "5.0") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "five events", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  const MString EnergyReason = MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionTotalEnergyOutOfLimits) + " .";
  const MString IDReason = MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionEventIdOutOfLimits) + " .";

  // No selection
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    MString Output = AnalyzeFile(Analyzer, FileName);
    Passed = Evaluate("AnalyzeEvent()", "no selection", "Five events are analyzed", GetStatistic(Output, "Number of events ."), 5) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no selection", "All events pass the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 5) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no selection", "Four single-site events are found", GetStatistic(Output, "Single-site  ."), 4) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no selection", "One Compton event is found", GetStatistic(Output, "Compton  ."), 1) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no selection", "No event is rejected", GetStatistic(Output, "Total ."), 0) && Passed;
  }

  // Energy window around the Compton event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetTotalEnergyMin(680.0);
    Analyzer.SetTotalEnergyMax(720.0);
    Passed = EvaluateTrue("SetInputModeFile()", "energy window", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "energy window", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

    Passed = Evaluate("AnalyzeEvent()", "662 keV, window 680 to 720 keV", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetTotalEnergyMin()", "662 keV, window 680 to 720 keV", "There is no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
    if (Analyzer.GetSingleBestTryEvent() != nullptr) {
      Passed = Evaluate("SetTotalEnergyMin()", "662 keV, window 680 to 720 keV", "The event is rejected for its energy", Analyzer.GetSingleBestTryEvent()->GetRejectionReason(), MRERawEvent::c_RejectionTotalEnergyOutOfLimits) && Passed;
    } else {
      Passed = EvaluateTrue("SetTotalEnergyMin()", "662 keV, window 680 to 720 keV", "There is a best try event", false) && Passed;
    }
    Passed = Evaluate("AnalyzeEvent()", "700 keV, window 680 to 720 keV", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetTotalEnergyMin()", "700 keV, window 680 to 720 keV", "There is an optimum event", Analyzer.GetSingleOptimumEvent() != nullptr) && Passed;

    vector<unsigned int> Codes = AnalyzeAll(Analyzer);
    Passed = EvaluateSize("AnalyzeEvent()", "window 680 to 720 keV, remaining events", "Three events and the end of the file follow", Codes.size(), 4) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = Evaluate("SetTotalEnergyMin()", "window 680 to 720 keV", "Five events are analyzed", GetStatistic(Output, "Number of events ."), 5) && Passed;
    Passed = Evaluate("SetTotalEnergyMin()", "window 680 to 720 keV", "One event passes the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
    Passed = Evaluate("SetTotalEnergyMin()", "window 680 to 720 keV", "Four events are rejected for their energy", GetStatistic(Output, EnergyReason), 4) && Passed;
    Passed = Evaluate("SetTotalEnergyMin()", "window 680 to 720 keV", "No single-site event is found", GetStatistic(Output, "Single-site  ."), 0) && Passed;
    Passed = Evaluate("SetTotalEnergyMin()", "window 680 to 720 keV", "One Compton event is found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  }

  // The limits belong to the window
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetTotalEnergyMin(662.0);
    Analyzer.SetTotalEnergyMax(700.0);
    MString Output = AnalyzeFile(Analyzer, FileName);
    Passed = Evaluate("SetTotalEnergyMin()", "662 and 700 keV, window 662 to 700 keV", "Events with an energy at the limits pass", GetStatistic(Output, "Number of events, which passed event selections ."), 5) && Passed;
  }
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetTotalEnergyMin(662.5);
    Analyzer.SetTotalEnergyMax(699.5);
    MString Output = AnalyzeFile(Analyzer, FileName);
    Passed = Evaluate("SetTotalEnergyMin()", "662 and 700 keV, window 662.5 to 699.5 keV", "Events just outside of the limits are rejected", GetStatistic(Output, EnergyReason), 5) && Passed;
    Passed = Evaluate("SetTotalEnergyMin()", "662 and 700 keV, window 662.5 to 699.5 keV", "No event passes the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 0) && Passed;
  }

  // Minimum event ID: -1 for the maximum means no upper limit
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetEventIdMin(4);
    MString Output = AnalyzeFile(Analyzer, FileName);
    Passed = Evaluate("SetEventIdMin()", "IDs 1 to 5, minimum 4", "Five events are analyzed", GetStatistic(Output, "Number of events ."), 5) && Passed;
    Passed = Evaluate("SetEventIdMin()", "IDs 1 to 5, minimum 4", "The events 4 and 5 pass", GetStatistic(Output, "Number of events, which passed event selections ."), 2) && Passed;
    Passed = Evaluate("SetEventIdMin()", "IDs 1 to 5, minimum 4", "The events 1 to 3 are rejected for their ID", GetStatistic(Output, IDReason), 3) && Passed;
  }

  // Event ID window: only the events up to the maximum are looked at here, see the questions in the report for the events above the maximum
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetEventIdMin(2);
    Analyzer.SetEventIdMax(3);
    Passed = EvaluateTrue("SetInputModeFile()", "ID window", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "ID window", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

    Passed = Evaluate("AnalyzeEvent()", "ID 1, window 2 to 3", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetEventIdMin()", "ID 1, window 2 to 3", "There is no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
    if (Analyzer.GetSingleBestTryEvent() != nullptr) {
      Passed = Evaluate("SetEventIdMin()", "ID 1, window 2 to 3", "The event is rejected for its ID", Analyzer.GetSingleBestTryEvent()->GetRejectionReason(), MRERawEvent::c_RejectionEventIdOutOfLimits) && Passed;
    } else {
      Passed = EvaluateTrue("SetEventIdMin()", "ID 1, window 2 to 3", "There is a best try event", false) && Passed;
    }
    Passed = Evaluate("AnalyzeEvent()", "ID 2, window 2 to 3", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetEventIdMin()", "ID 2, window 2 to 3", "The event at the lower limit has an optimum event", Analyzer.GetSingleOptimumEvent() != nullptr) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "ID 3, window 2 to 3", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetEventIdMax()", "ID 3, window 2 to 3", "The event at the upper limit has an optimum event", Analyzer.GetSingleOptimumEvent() != nullptr) && Passed;
  }

  // Lever arm: the two hits of the Compton event are sqrt(105) cm = 10.247 cm apart
  const MString ComptonFileName = GetTemporaryFileName("LeverArm.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "Compton event", "The evta file can be written", WriteTextFile(ComptonFileName, Events)) && Passed;
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetLeverArmMin(10.2);
    Analyzer.SetLeverArmMax(10.3);
    MString Output = AnalyzeFile(Analyzer, ComptonFileName);
    Passed = Evaluate("SetLeverArmMin()", "lever arm 10.247 cm, window 10.2 to 10.3 cm", "The event passes", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
  }
  {
    // Defect REV-H-3: The lever arm limits are stored but never applied, although the settings, the GUI, and a rejection reason exist for them
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetLeverArmMin(10.3);
    MString Output = AnalyzeFile(Analyzer, ComptonFileName);
    Passed = ExpectedToFail(Evaluate("SetLeverArmMin()", "lever arm 10.247 cm, minimum 10.3 cm", "The event is rejected", GetStatistic(Output, "Number of events, which passed event selections ."), 0)) && Passed;
  }
  {
    // Defect REV-H-3: see above
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetLeverArmMax(10.2);
    MString Output = AnalyzeFile(Analyzer, ComptonFileName);
    Passed = ExpectedToFail(Evaluate("SetLeverArmMax()", "lever arm 10.247 cm, maximum 10.2 cm", "The event is rejected", GetStatistic(Output, "Number of events, which passed event selections ."), 0)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test events with a bad flag
bool UTRawEventAnalyzer::TestBadEvents()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Bad.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + "BD Bad\n" + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one bad event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Bad events are rejected by default
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "bad event", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "bad event", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "bad event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetRejectAllBadEvents()", "default, bad event", "There is no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
    MRERawEvent* BestTry = Analyzer.GetSingleBestTryEvent();
    Passed = EvaluateTrue("SetRejectAllBadEvents()", "default, bad event", "There is a best try event", BestTry != nullptr) && Passed;
    if (BestTry != nullptr) {
      Passed = Evaluate("SetRejectAllBadEvents()", "default, bad event", "The event is rejected for its bad flag", BestTry->GetRejectionReason(), MRERawEvent::c_RejectionExternalBadEventFlag) && Passed;
      MPhysicalEvent* Physical = BestTry->GetPhysicalEvent();
      if (Physical != nullptr) {
        Passed = Evaluate("SetRejectAllBadEvents()", "default, bad event", "The physical event is unidentifiable", Physical->GetType(), MPhysicalEvent::c_Unidentifiable) && Passed;
        Passed = EvaluateTrue("SetRejectAllBadEvents()", "default, bad event", "The physical event is flagged as bad", Physical->IsBad()) && Passed;
      } else {
        Passed = EvaluateTrue("SetRejectAllBadEvents()", "default, bad event", "The best try has a physical event", false) && Passed;
      }
    }
    MString Output = RunPostAnalysis(Analyzer);
    Passed = Evaluate("SetRejectAllBadEvents()", "default, bad event", "No event passes the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 0) && Passed;
    Passed = Evaluate("SetRejectAllBadEvents()", "default, bad event", "The rejection is counted", GetStatistic(Output, MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionExternalBadEventFlag) + " ."), 1) && Passed;
  }

  // Without the rejection, the event is reconstructed and keeps its flag
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetRejectAllBadEvents(false);
    Passed = EvaluateTrue("SetInputModeFile()", "bad event, not rejected", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "bad event, not rejected", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "bad event, not rejected", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    MRERawEvent* Optimum = Analyzer.GetSingleOptimumEvent();
    Passed = EvaluateTrue("SetRejectAllBadEvents()", "false, bad event", "There is an optimum event", Optimum != nullptr) && Passed;
    if (Optimum != nullptr && Optimum->GetPhysicalEvent() != nullptr) {
      Passed = Evaluate("SetRejectAllBadEvents()", "false, bad event", "The physical event is a photo event", Optimum->GetPhysicalEvent()->GetType(), MPhysicalEvent::c_Photo) && Passed;
      Passed = EvaluateTrue("SetRejectAllBadEvents()", "false, bad event", "The physical event is still flagged as bad", Optimum->GetPhysicalEvent()->IsBad()) && Passed;
    }
    MString Output = RunPostAnalysis(Analyzer);
    Passed = Evaluate("SetRejectAllBadEvents()", "false, bad event", "The event passes the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the coincidence search
bool UTRawEventAnalyzer::TestCoincidence()
{
  bool Passed = true;

  // Four triggers: a tracker hit at 1 s, a calorimeter hit 0.5 us later (inside the window of 1 us),
  // a calorimeter hit at 2 s, and one 1.1 us later (outside of the window)
  const MString FileName = GetTemporaryFileName("Coincidence.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + "HT 1;0;0;0;200;0.29;0.29;0.014;2\n";
  Events += GetEventStart(2, "1.0000005") + "HT 2;1;2;-10;500;2.88;2.88;2.3;10\n";
  Events += GetEventStart(3, "2.0") + GetCalorimeterHit();
  Events += GetEventStart(4, "2.0000011") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "four triggers", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Without coincidence search every trigger is an event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoNone);
    MString Output = AnalyzeFile(Analyzer, FileName);
    Passed = Evaluate("SetCoincidenceAlgorithm()", "none", "Four events are analyzed", GetStatistic(Output, "Number of events ."), 4) && Passed;
    Passed = Evaluate("SetCoincidenceAlgorithm()", "none", "All are single-site events", GetStatistic(Output, "Single-site  ."), 4) && Passed;
  }

  // With a window of 1 us the first two triggers are one event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoWindow);
    Analyzer.SetCoincidenceWindow(1E-6);
    Passed = EvaluateTrue("SetInputModeFile()", "coincidence window", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "coincidence window", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;

    // The search needs a trigger outside of the window (or the end of the file) to close an event
    Passed = Evaluate("AnalyzeEvent()", "window, first trigger read", "The analysis waits for the window to close", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisCoincidenceWindowWait) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "window, second trigger read", "The analysis waits for the window to close", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisCoincidenceWindowWait) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "window, third trigger read", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "The event has the hits of both triggers", Analyzer.GetInitialRawEvent()->GetNRESEs(), 2) && Passed;
      Passed = EvaluateNear("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "The energy is 200 keV + 500 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 700.0, 1e-12) && Passed;
    } else {
      Passed = EvaluateTrue("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "There is an initial event", false) && Passed;
    }
    if (Analyzer.GetSingleOptimumEvent() != nullptr && Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent() != nullptr) {
      MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent());
      Passed = EvaluateTrue("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "The merged event is reconstructed as Compton event", Compton != nullptr) && Passed;
      if (Compton != nullptr) {
        Passed = EvaluateNear("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "The Compton scatter angle follows from the two energies", Compton->Phi(), GetComptonAngle(200.0, 500.0), 1e-12) && Passed;
      }
    } else {
      Passed = EvaluateTrue("SetCoincidenceWindow()", "triggers 0.5 us apart, window 1 us", "There is an optimum event with a physical event", false) && Passed;
    }

    Passed = Evaluate("AnalyzeEvent()", "window, fourth trigger read", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("SetCoincidenceWindow()", "triggers 1.1 us apart, window 1 us", "The event has one hit", Analyzer.GetInitialRawEvent()->GetNRESEs(), 1) && Passed;
      Passed = Evaluate("SetCoincidenceWindow()", "triggers 1.1 us apart, window 1 us", "The event is the third trigger", Analyzer.GetInitialRawEvent()->GetEventID(), 3UL) && Passed;
    }
    Passed = Evaluate("AnalyzeEvent()", "window, end of file", "The last trigger is analyzed when the file ends", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("SetCoincidenceWindow()", "last trigger", "The event is the fourth trigger", Analyzer.GetInitialRawEvent()->GetEventID(), 4UL) && Passed;
    }
    Passed = Evaluate("AnalyzeEvent()", "window, after the last event", "No events are left", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile) && Passed;

    MString Output = RunPostAnalysis(Analyzer);
    Passed = Evaluate("SetCoincidenceWindow()", "four triggers, window 1 us", "Three events are analyzed", GetStatistic(Output, "Number of events ."), 3) && Passed;
    Passed = Evaluate("SetCoincidenceWindow()", "four triggers, window 1 us", "One Compton event is found", GetStatistic(Output, "Compton  ."), 1) && Passed;
    Passed = Evaluate("SetCoincidenceWindow()", "four triggers, window 1 us", "Two single-site events are found", GetStatistic(Output, "Single-site  ."), 2) && Passed;
  }

  // Two coincident triggers in the same voxel are merged into one hit
  const MString SameVoxelFileName = GetTemporaryFileName("SameVoxel.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "1.0000005") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "two triggers in one voxel", "The evta file can be written", WriteTextFile(SameVoxelFileName, Events)) && Passed;
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoWindow);
    Analyzer.SetCoincidenceWindow(1E-6);
    Passed = EvaluateTrue("SetInputModeFile()", "two triggers in one voxel", "The file is opened", OpenInput(Analyzer, SameVoxelFileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "two triggers in one voxel", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    vector<unsigned int> Codes = AnalyzeAll(Analyzer);
    Passed = EvaluateSize("AnalyzeEvent()", "two triggers in one voxel", "Two waits, one event, and the end of the file", Codes.size(), 4) && Passed;
    if (Codes.size() == 4) {
      Passed = Evaluate("AnalyzeEvent()", "two triggers in one voxel", "The third call gives the event", Codes[2], MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    }
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("AnalyzeEvent()", "two triggers in one voxel", "The two hits are merged into one", Analyzer.GetInitialRawEvent()->GetNRESEs(), 1) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "two triggers in one voxel", "The energy is the sum, 2 x 662 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 1324.0, 1e-12) && Passed;
    } else {
      Passed = EvaluateTrue("AnalyzeEvent()", "two triggers in one voxel", "There is an initial event", false) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test switching the hit clustering and the Compton sequence reconstruction on and off
bool UTRawEventAnalyzer::TestStageSwitches()
{
  bool Passed = true;

  // One event: 200 keV in the tracker, and 300 keV and 200 keV in the calorimeter, 0.5 cm apart
  const MString FileName = GetTemporaryFileName("Stages.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + "HT 1;0;0;0;200;0.29;0.29;0.014;2\nHT 2;0;0;-10;300;2.88;2.88;2.3;10\nHT 2;0;0;-10.5;200;2.88;2.88;2.3;10\n";
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "three hits", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Without hit clustering the three hits stay
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoNone);
    Passed = EvaluateTrue("SetInputModeFile()", "no hit clustering", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "no hit clustering", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no hit clustering", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetSingleBestTryEvent() != nullptr) {
      Passed = Evaluate("SetHitClusteringAlgorithm()", "none, two calorimeter hits 0.5 cm apart", "The event keeps its three hits", Analyzer.GetSingleBestTryEvent()->GetNRESEs(), 3) && Passed;
    } else {
      Passed = EvaluateTrue("SetHitClusteringAlgorithm()", "none", "There is a best try event", false) && Passed;
    }
    MString Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateFalse("SetHitClusteringAlgorithm()", "none", "No clusterizer is listed in the options", Output.Contains("Clusterizer options")) && Passed;
  }

  // With clustering by distance (1.1 cm in a calorimeter) the two calorimeter hits become one cluster of 500 keV, and the event a Compton event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetHitClusteringAlgorithm(MRawEventAnalyzer::c_HitClusteringAlgoDistance);
    Analyzer.SetStandardClusterizerMinDistanceD2(1.1);
    Passed = EvaluateTrue("SetInputModeFile()", "hit clustering by distance", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "hit clustering by distance", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "hit clustering by distance", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("GetInitialRawEvent()", "hit clustering by distance", "The initial event keeps its three hits", Analyzer.GetInitialRawEvent()->GetNRESEs(), 3) && Passed;
    }
    MRERawEvent* Optimum = Analyzer.GetSingleOptimumEvent();
    Passed = EvaluateTrue("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "There is an optimum event", Optimum != nullptr) && Passed;
    if (Optimum != nullptr) {
      Passed = Evaluate("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "The event has a hit and a cluster", Optimum->GetNRESEs(), 2) && Passed;
      Passed = EvaluateNear("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "The energy is unchanged, 700 keV", Optimum->GetEnergy(), 700.0, 1e-12) && Passed;
      MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(Optimum->GetPhysicalEvent());
      Passed = EvaluateTrue("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "The event is a Compton event", Compton != nullptr) && Passed;
      if (Compton != nullptr) {
        Passed = EvaluateNear("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "The electron energy is that of the tracker hit, 200 keV", Compton->Ee(), 200.0, 1e-12) && Passed;
        Passed = EvaluateNear("SetHitClusteringAlgorithm()", "distance, two calorimeter hits 0.5 cm apart", "The gamma energy is that of the cluster, 300 keV + 200 keV", Compton->Eg(), 500.0, 1e-12) && Passed;
      }
    }
  }

  // Without Compton sequence reconstruction a two-site event has no optimum event
  const MString ComptonFileName = GetTemporaryFileName("StagesCompton.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "Compton event", "The evta file can be written", WriteTextFile(ComptonFileName, Events)) && Passed;
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetCSRAlgorithm(MRawEventAnalyzer::c_CSRAlgoNone);
    Passed = EvaluateTrue("SetInputModeFile()", "no CSR", "The file is opened", OpenInput(Analyzer, ComptonFileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "no CSR", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no CSR", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = EvaluateTrue("SetCSRAlgorithm()", "none, two-site event", "There is no optimum event", Analyzer.GetSingleOptimumEvent() == nullptr) && Passed;
    Passed = EvaluateTrue("SetCSRAlgorithm()", "none, two-site event", "There is a best try event", Analyzer.GetSingleBestTryEvent() != nullptr) && Passed;
    MString Output = RunPostAnalysis(Analyzer);
    Passed = EvaluateFalse("SetCSRAlgorithm()", "none", "No Compton sequence reconstruction is listed in the options", Output.Contains("CSR")) && Passed;
    Passed = Evaluate("SetCSRAlgorithm()", "none, two-site event", "No Compton event is found", GetStatistic(Output, "Compton  ."), 0) && Passed;
  }

  // With electron tracking in the tracker, the event without a track is still a Compton event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoModifiedPearson);
    vector<MString> Detectors;
    Detectors.push_back("Tracker");
    Analyzer.SetElectronTrackingDetectorList(Detectors);
    MString Output = AnalyzeFile(Analyzer, ComptonFileName);
    Passed = Evaluate("SetTrackingAlgorithm()", "modified Pearson, two-site event without a track", "One Compton event is found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the event clustering
bool UTRawEventAnalyzer::TestEventClustering()
{
  bool Passed = true;

  // The two hits of the Compton event are sqrt(105) cm = 10.247 cm apart
  const MString FileName = GetTemporaryFileName("EventClustering.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetComptonHits();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "Compton event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

  // A cut off above the distance keeps the hits in one event
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoDistance);
    Analyzer.SetEventClusteringDistanceCutOff(10.3);
    Passed = EvaluateTrue("SetInputModeFile()", "event clustering, cut off 10.3 cm", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "event clustering, cut off 10.3 cm", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event clustering, cut off 10.3 cm", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = Evaluate("SetEventClusteringDistanceCutOff()", "hits 10.247 cm apart, cut off 10.3 cm", "The hits stay in one event", Analyzer.GetRawEventList()->Size(), 1U) && Passed;
    Passed = EvaluateSize("GetOptimumEvents()", "hits 10.247 cm apart, cut off 10.3 cm", "There is one optimum event", Analyzer.GetOptimumEvents().size(), 1) && Passed;
  }

  // A cut off below the distance splits the hits into two events
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Analyzer.SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoDistance);
    Analyzer.SetEventClusteringDistanceCutOff(10.2);
    Passed = EvaluateTrue("SetInputModeFile()", "event clustering, cut off 10.2 cm", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "event clustering, cut off 10.2 cm", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event clustering, cut off 10.2 cm", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    Passed = Evaluate("SetEventClusteringDistanceCutOff()", "hits 10.247 cm apart, cut off 10.2 cm", "The hits are split into two events", Analyzer.GetRawEventList()->Size(), 2U) && Passed;
    vector<MRERawEvent*> Optimum = Analyzer.GetOptimumEvents();
    Passed = EvaluateSize("GetOptimumEvents()", "hits 10.247 cm apart, cut off 10.2 cm", "There are two optimum events", Optimum.size(), 2) && Passed;
    if (Optimum.size() == 2 && Optimum[0] != nullptr && Optimum[1] != nullptr) {
      Passed = EvaluateNear("GetOptimumEvents()", "hits 10.247 cm apart, cut off 10.2 cm", "The two events have the 200 keV and the 500 keV of the hits", Optimum[0]->GetEnergy() + Optimum[1]->GetEnergy(), 700.0, 1e-12) && Passed;
      Passed = EvaluateNear("GetOptimumEvents()", "hits 10.247 cm apart, cut off 10.2 cm", "The energies of the two events differ by 500 keV - 200 keV", fabs(Optimum[0]->GetEnergy() - Optimum[1]->GetEnergy()), 300.0, 1e-12) && Passed;
    } else {
      Passed = EvaluateTrue("GetOptimumEvents()", "hits 10.247 cm apart, cut off 10.2 cm", "Both events have an optimum event", false) && Passed;
    }
    // The initial event is still the complete event
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("GetInitialRawEvent()", "hits 10.247 cm apart, cut off 10.2 cm", "The initial event has both hits", Analyzer.GetInitialRawEvent()->GetNRESEs(), 2) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test adding events to the store instead of reading them from a file
bool UTRawEventAnalyzer::TestStoreMode()
{
  bool Passed = true;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_IdealGeometry);

  Passed = Evaluate("AnalyzeEvent()", "empty store, before the pre-analysis", "There are no events in the store", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsInStore) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "no files", "The pre-analysis succeeds without files", RunPreAnalysis(Analyzer)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "empty store", "There are no events in the store", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsInStore) && Passed;

  // Events as text in the evta format, without noising
  Passed = EvaluateTrue("AddRawEvent(MString)", "evta single-site event", "The event is added", AddEventString(Analyzer, GetEventStart(7, "3.5") + GetCalorimeterHit(), false, 21)) && Passed;
  Passed = EvaluateTrue("AddRawEvent(MString)", "evta Compton event", "The event is added", AddEventString(Analyzer, GetEventStart(8, "4.5") + GetComptonHits(), false, 21)) && Passed;
  Passed = EvaluateFalse("AddRawEvent(MString)", "event without hits", "An event without hits is refused", AddEventString(Analyzer, GetEventStart(9, "5.5"), false, 21)) && Passed;
  Passed = EvaluateFalse("AddRawEvent(MString)", "event with a line which cannot be parsed", "The event is refused", AddEventString(Analyzer, "SE\nID 10\nTW abc\n" + GetCalorimeterHit(), false, 21)) && Passed;
  Passed = EvaluateFalse("AddRawEvent(MString)", "empty text", "An empty text is refused", AddEventString(Analyzer, "", false, 21)) && Passed;

  // Events as text in the sim format, with noising (ideal resolutions: nothing changes) and trigger
  Passed = EvaluateTrue("AddRawEvent(MString)", "sim single-site event", "The event is added", AddEventString(Analyzer, GetEventStart(11, "6.5") + "HTsim 2;0;0;-10;300;0\n", true, 25)) && Passed;
  Passed = EvaluateFalse("AddRawEvent(MString)", "sim event with 51 keV in the shield", "The vetoed event is refused", AddEventString(Analyzer, GetEventStart(12, "7.5") + "HTsim 2;0;0;-10;300;0\nHTsim 4;0;0;10;51;0\n", true, 25)) && Passed;
  Passed = EvaluateFalse("AddRawEvent(MString)", "sim event with 5 keV in the tracker", "The event below the threshold is refused", AddEventString(Analyzer, GetEventStart(13, "8.5") + "HTsim 1;0;0;0;5;0\n", true, 25)) && Passed;

  // A raw event object: the analyzer takes it over
  MRERawEvent* Event = new MRERawEvent(&m_IdealGeometry);
  Event->SetEventID(14);
  Event->AddRESE(CreateHit(MVector(0.0, 0.0, -10.0), 400.0, MDDetector::c_Calorimeter, &m_IdealGeometry));
  Passed = EvaluateTrue("AddRawEvent(MRERawEvent*)", "event with one hit", "The event is added", Analyzer.AddRawEvent(Event)) && Passed;

  // The events come back in the order in which they were added
  Passed = Evaluate("AnalyzeEvent()", "store, first event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "store, first event", "The event ID is 7", Analyzer.GetInitialRawEvent()->GetEventID(), 7UL) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "store, first event", "The time is 3.5 s", Analyzer.GetInitialRawEvent()->GetEventTime().GetAsDouble(), 3.5, 1e-12) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "store, first event", "The energy is 662 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 662.0, 1e-12) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "store, second event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "store, second event", "The event ID is 8", Analyzer.GetInitialRawEvent()->GetEventID(), 8UL) && Passed;
  }
  if (Analyzer.GetSingleOptimumEvent() != nullptr && Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "store, second event", "The event is a Compton event", Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent()->GetType(), MPhysicalEvent::c_Compton) && Passed;
  } else {
    Passed = EvaluateTrue("AnalyzeEvent()", "store, second event", "There is an optimum event with a physical event", false) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "store, third event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "store, third event", "The event ID is 11", Analyzer.GetInitialRawEvent()->GetEventID(), 11UL) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "store, third event", "The energy is 300 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 300.0, 1e-12) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "store, fourth event", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  if (Analyzer.GetInitialRawEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "store, fourth event", "The event ID is 14", Analyzer.GetInitialRawEvent()->GetEventID(), 14UL) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "store, fourth event", "The energy is 400 keV", Analyzer.GetInitialRawEvent()->GetEnergy(), 400.0, 1e-12) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "store, no event left", "There are no events in the store", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisNoEventsInStore) && Passed;

  MString Output = RunPostAnalysis(Analyzer);
  Passed = Evaluate("PostAnalysis()", "store mode", "Four events were analyzed", GetStatistic(Output, "Number of events ."), 4) && Passed;
  Passed = Evaluate("PostAnalysis()", "store mode", "Three single-site events were found", GetStatistic(Output, "Single-site  ."), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "store mode", "One Compton event was found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  // The statistics of the noising of the analyzer are part of the output: three sim events were added
  Passed = Evaluate("PostAnalysis()", "store mode", "One event was vetoed", GetStatistic(Output, "Number of vetoed events: ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "store mode", "One event triggered", GetStatistic(Output, "Number of triggered events: ."), 1) && Passed;

  // With an input file, events cannot be added
  {
    const MString FileName = GetTemporaryFileName("Store.evta");
    MString Events = GetEvtaHeader();
    Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
    Events += "EN\n";
    Passed = EvaluateTrue("WriteTextFile()", "one event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

    MRawEventAnalyzer WithFile;
    Prepare(WithFile, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "one event", "The file is opened", OpenInput(WithFile, FileName)) && Passed;
    Passed = EvaluateFalse("AddRawEvent(MString)", "analyzer with an input file", "The event is refused", AddEventString(WithFile, GetEventStart(7, "3.5") + GetCalorimeterHit(), false, 21)) && Passed;
    // The analyzer does not take over a refused event
    unique_ptr<MRERawEvent> Refused(new MRERawEvent(&m_Geometry));
    DisableDefaultStreams();
    bool Added = WithFile.AddRawEvent(Refused.get());
    EnableDefaultStreams();
    Passed = EvaluateFalse("AddRawEvent(MRERawEvent*)", "analyzer with an input file", "The event is refused", Added) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test saving the origin information
bool UTRawEventAnalyzer::TestSaveOI()
{
  bool Passed = true;

  // The initial interaction: a gamma ray (particle ID 1) of 662 keV starting at (1.5, 2.5, 30) cm in direction (0, 0, -1) with polarization (1, 0, 0)
  const MString IA = "IA INIT 1;0;0;0;1.5;2.5;30;0;0;0;0;0;0;0;0;1;0;0;-1;1;0;0;662\n";
  const MString FileName = GetTemporaryFileName("OI.sim");
  MString Events = GetSimHeader(m_IdealGeometry);
  Events += GetEventStart(1, "1.0") + IA + "HTsim 2;0;0;-10;662;0\n";
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The sim file can be written", WriteTextFile(FileName, Events)) && Passed;

  // Default: the origin information is not saved
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_IdealGeometry);
    Passed = EvaluateTrue("SetInputModeFile()", "sim file with IA", "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "sim file with IA", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "sim file with IA", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetInitialRawEvent() != nullptr) {
      Passed = Evaluate("SetSaveOI()", "default", "The event has no additional measurement", Analyzer.GetInitialRawEvent()->GetNREAMs(), 0U) && Passed;
    } else {
      Passed = EvaluateTrue("SetSaveOI()", "default", "There is an initial event", false) && Passed;
    }
  }

  // Set before or after the file is opened
  for (unsigned int Order = 0; Order < 2; ++Order) {
    MString Case = "set before the file is opened";
    if (Order == 1) {
      Case = "set after the file is opened";
    }
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_IdealGeometry);
    if (Order == 0) {
      Analyzer.SetSaveOI(true);
    }
    Passed = EvaluateTrue("SetInputModeFile()", Case, "The file is opened", OpenInput(Analyzer, FileName)) && Passed;
    if (Order == 1) {
      Analyzer.SetSaveOI(true);
    }
    Passed = EvaluateTrue("PreAnalysis()", Case, "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", Case, "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetSingleOptimumEvent() != nullptr && Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent() != nullptr) {
      MPhysicalEvent* Physical = Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent();
      Passed = EvaluateVectorNear("SetSaveOI()", Case, "The physical event has the start position", Physical->GetOIPosition(), MVector(1.5, 2.5, 30.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("SetSaveOI()", Case, "The physical event has the start direction", Physical->GetOIDirection(), MVector(0.0, 0.0, -1.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("SetSaveOI()", Case, "The physical event has the polarization", Physical->GetOIPolarization(), MVector(1.0, 0.0, 0.0), 1e-12) && Passed;
      Passed = EvaluateNear("SetSaveOI()", Case, "The physical event has the start energy", Physical->GetOIEnergy(), 662.0, 1e-12) && Passed;
      Passed = Evaluate("SetSaveOI()", Case, "The physical event has the particle ID", Physical->GetOIParticleID(), 1) && Passed;
    } else {
      Passed = EvaluateTrue("SetSaveOI()", Case, "There is an optimum event with a physical event", false) && Passed;
    }
  }

  // Store mode: the same event as text
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_IdealGeometry);
    Analyzer.SetSaveOI(true);
    Passed = EvaluateTrue("PreAnalysis()", "store mode", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Passed = EvaluateTrue("AddRawEvent(MString)", "sim event with IA", "The event is added", AddEventString(Analyzer, GetEventStart(1, "1.0") + IA + "HTsim 2;0;0;-10;662;0\n", true, 25)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "store mode, sim event with IA", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
    if (Analyzer.GetSingleOptimumEvent() != nullptr && Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent() != nullptr) {
      MPhysicalEvent* Physical = Analyzer.GetSingleOptimumEvent()->GetPhysicalEvent();
      Passed = EvaluateVectorNear("SetSaveOI()", "store mode", "The physical event has the start position", Physical->GetOIPosition(), MVector(1.5, 2.5, 30.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("SetSaveOI()", "store mode", "The physical event has the start direction", Physical->GetOIDirection(), MVector(0.0, 0.0, -1.0), 1e-12) && Passed;
      Passed = EvaluateVectorNear("SetSaveOI()", "store mode", "The physical event has the polarization", Physical->GetOIPolarization(), MVector(1.0, 0.0, 0.0), 1e-12) && Passed;
      Passed = EvaluateNear("SetSaveOI()", "store mode", "The physical event has the start energy", Physical->GetOIEnergy(), 662.0, 1e-12) && Passed;
      // Defect REV-H-7: AddRawEvent() has its own copy of the conversion of the IA line, which skips the particle ID that the file reader hands over
      Passed = ExpectedToFail(Evaluate("SetSaveOI()", "store mode", "The physical event has the particle ID", Physical->GetOIParticleID(), 1)) && Passed;
    } else {
      Passed = EvaluateTrue("SetSaveOI()", "store mode", "There is an optimum event with a physical event", false) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test TransferFileInformation()
bool UTRawEventAnalyzer::TestTransferFileInformation()
{
  bool Passed = true;

  // The pattern of cosima: the events are added to the store, and the information of the header and the footer comes from another file
  const MString FileName = GetTemporaryFileName("Transfer.sim");
  const MString TraFileName = GetTemporaryFileName("Transfer.tra");
  MString Events = GetSimHeader(m_IdealGeometry);
  Events += "TB 0.5\n\n";
  Events += GetEventStart(1, "1.0") + "HTsim 2;0;0;-10;662;0\n";
  Events += "EN\n\nTE 7.0\nTS 6\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The sim file can be written", WriteTextFile(FileName, Events)) && Passed;

  MFileEventsEvta External(&m_IdealGeometry);
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Opened = External.Open(FileName);
  if (Opened == true) {
    // Read to the end, so that the footer is read
    MRERawEvent* Event = nullptr;
    while ((Event = External.GetNextEvent()) != nullptr) {
      delete Event;
    }
  }
  EnableDefaultStreams();
  EndCaptureOutput();
  Passed = EvaluateTrue("MFileEventsEvta::Open()", "sim file", "The external file is opened", Opened) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_IdealGeometry);
  Passed = EvaluateTrue("TransferFileInformation()", "no output file", "The call returns true and does nothing", Analyzer.TransferFileInformation(&External)) && Passed;
  Passed = EvaluateTrue("TransferFileInformation()", "no output file, no external file", "The call returns true and does nothing", Analyzer.TransferFileInformation(nullptr)) && Passed;
  Passed = EvaluateTrue("SetOutputModeFile()", "store mode", "The output file is opened", OpenOutput(Analyzer, TraFileName)) && Passed;
  Passed = EvaluateTrue("TransferFileInformation()", "no external file", "The call returns true and does nothing", Analyzer.TransferFileInformation(nullptr)) && Passed;
  Passed = EvaluateTrue("TransferFileInformation()", "external sim file", "The information is transferred", Analyzer.TransferFileInformation(&External)) && Passed;
  Passed = EvaluateTrue("PreAnalysis()", "store mode with output file", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
  Passed = EvaluateTrue("AddRawEvent(MString)", "sim single-site event", "The event is added", AddEventString(Analyzer, GetEventStart(1, "1.0") + "HTsim 2;0;0;-10;662;0\n", true, 25)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "store mode with output file", "The analysis succeeds", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisSucess) && Passed;
  RunPostAnalysis(Analyzer);

  vector<unique_ptr<MPhysicalEvent>> Read;
  long SimulatedEvents = 0;
  double ObservationTime = 0.0;
  Passed = EvaluateTrue("TransferFileInformation()", "external sim file", "The tra file can be read as tra file", ReadTraFile(TraFileName, Read, SimulatedEvents, ObservationTime)) && Passed;
  Passed = EvaluateSize("TransferFileInformation()", "external sim file", "The tra file has the event", Read.size(), 1) && Passed;
  Passed = Evaluate("TransferFileInformation()", "external sim file with TS 6", "The tra file has the number of simulated events", SimulatedEvents, 6L) && Passed;
  // TB 0.5 and TE 7.0: 6.5 s
  Passed = EvaluateNear("TransferFileInformation()", "external sim file with TB 0.5 and TE 7.0", "The tra file has the observation time", ObservationTime, 6.5, 1e-9) && Passed;

  External.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test PostAnalysis() and its statistics
bool UTRawEventAnalyzer::TestPostAnalysis()
{
  bool Passed = true;

  // Without events
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("PreAnalysis()", "no events", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    BeginCaptureOutput();
    bool Result = Analyzer.PostAnalysis();
    MString Output = EndCaptureOutput();
    Passed = EvaluateTrue("PostAnalysis()", "no events", "The post-analysis succeeds", Result) && Passed;
    Passed = EvaluateTrue("PostAnalysis()", "no events", "The output says that there are no events", Output.Contains("No events available")) && Passed;
    Passed = Evaluate("PostAnalysis()", "no events", "There is no line with the number of events", GetStatistic(Output, "Number of events ."), -1) && Passed;
    Passed = EvaluateTrue("PostAnalysis()", "no events", "The output names the geometry file", Output.Contains(MString("# GeometryFile ") + m_Geometry.GetFileName())) && Passed;
  }

  // Without a pre-analysis: only the header of the options is printed
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    BeginCaptureOutput();
    bool Result = Analyzer.PostAnalysis();
    MString Output = EndCaptureOutput();
    Passed = EvaluateTrue("PostAnalysis()", "no pre-analysis", "The post-analysis succeeds", Result) && Passed;
    Passed = EvaluateFalse("PostAnalysis()", "no pre-analysis", "No stage is listed in the options", Output.Contains("options:")) && Passed;
  }

  // Twice: the second call must not fail on the closed files
  {
    const MString FileName = GetTemporaryFileName("Post.evta");
    MString Events = GetEvtaHeader();
    Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
    Events += "EN\n";
    Passed = EvaluateTrue("WriteTextFile()", "one event", "The evta file can be written", WriteTextFile(FileName, Events)) && Passed;

    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    MString Output = AnalyzeFile(Analyzer, FileName, GetTemporaryFileName("Post.tra"));
    Passed = Evaluate("PostAnalysis()", "one event", "One event was analyzed", GetStatistic(Output, "Number of events ."), 1) && Passed;
    // The second call complains with merr about the closed input file, so its output cannot be read here
    BeginCaptureOutput();
    DisableDefaultStreams();
    bool Result = Analyzer.PostAnalysis();
    EnableDefaultStreams();
    EndCaptureOutput();
    Passed = EvaluateTrue("PostAnalysis()", "one event, second call", "The post-analysis succeeds again", Result) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test using one analyzer for two files
bool UTRawEventAnalyzer::TestReuse()
{
  bool Passed = true;

  // File 1: a single-site event, a Compton event, and a bad event. File 2: one single-site event
  const MString FileName1 = GetTemporaryFileName("Reuse1.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += GetEventStart(3, "3.0") + "BD Bad\n" + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "three events", "The first evta file can be written", WriteTextFile(FileName1, Events)) && Passed;
  const MString FileName2 = GetTemporaryFileName("Reuse2.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The second evta file can be written", WriteTextFile(FileName2, Events)) && Passed;

  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  MString Output = AnalyzeFile(Analyzer, FileName1, GetTemporaryFileName("Reuse1.tra"));
  Passed = Evaluate("PostAnalysis()", "first file", "Three events were analyzed", GetStatistic(Output, "Number of events ."), 3) && Passed;
  Passed = Evaluate("PostAnalysis()", "first file", "Two events passed the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 2) && Passed;
  Passed = Evaluate("PostAnalysis()", "first file", "One single-site event was found", GetStatistic(Output, "Single-site  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "first file", "One Compton event was found", GetStatistic(Output, "Compton  ."), 1) && Passed;
  Passed = Evaluate("PostAnalysis()", "first file", "One event was rejected", GetStatistic(Output, "Total ."), 1) && Passed;

  // PreAnalysis() starts a new analysis: the statistics are those of the second file
  Output = AnalyzeFile(Analyzer, FileName2, GetTemporaryFileName("Reuse2.tra"));
  Passed = Evaluate("PreAnalysis()", "second file with the same analyzer", "One event was analyzed", GetStatistic(Output, "Number of events ."), 1) && Passed;
  // Defect REV-H-4: PreAnalysis() resets only the number of events and of reconstructable events, all other counters go on from the first file
  Passed = ExpectedToFail(Evaluate("PreAnalysis()", "second file with the same analyzer", "One event passed the selections", GetStatistic(Output, "Number of events, which passed event selections ."), 1)) && Passed;
  Passed = ExpectedToFail(Evaluate("PreAnalysis()", "second file with the same analyzer", "One single-site event was found", GetStatistic(Output, "Single-site  ."), 1)) && Passed;
  Passed = ExpectedToFail(Evaluate("PreAnalysis()", "second file with the same analyzer", "No Compton event was found", GetStatistic(Output, "Compton  ."), 0)) && Passed;
  Passed = ExpectedToFail(Evaluate("PreAnalysis()", "second file with the same analyzer", "No event was rejected", GetStatistic(Output, "Total ."), 0)) && Passed;

  // The events of the second file are in the second tra file
  vector<unique_ptr<MPhysicalEvent>> Read;
  long SimulatedEvents = 0;
  double ObservationTime = 0.0;
  Passed = EvaluateTrue("SetOutputModeFile()", "second file with the same analyzer", "The second tra file can be read", ReadTraFile(GetTemporaryFileName("Reuse2.tra"), Read, SimulatedEvents, ObservationTime)) && Passed;
  Passed = EvaluateSize("SetOutputModeFile()", "second file with the same analyzer", "The second tra file has one event", Read.size(), 1) && Passed;
  Read.clear();
  Passed = EvaluateTrue("SetOutputModeFile()", "first file", "The first tra file can still be read", ReadTraFile(GetTemporaryFileName("Reuse1.tra"), Read, SimulatedEvents, ObservationTime)) && Passed;
  Passed = EvaluateSize("SetOutputModeFile()", "first file", "The first tra file has three events", Read.size(), 3) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test JoinStatistics()
bool UTRawEventAnalyzer::TestJoinStatistics()
{
  bool Passed = true;

  // File 1: a single-site event, a Compton event, and an event outside of the energy window. File 2: one single-site event
  const MString FileName1 = GetTemporaryFileName("Join1.evta");
  MString Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += GetEventStart(2, "2.0") + GetComptonHits();
  Events += GetEventStart(3, "3.0") + "HT 2;0;0;-10;900;2.88;2.88;2.3;10\n";
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "three events", "The first evta file can be written", WriteTextFile(FileName1, Events)) && Passed;
  const MString FileName2 = GetTemporaryFileName("Join2.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one event", "The second evta file can be written", WriteTextFile(FileName2, Events)) && Passed;

  {
    MRawEventAnalyzer First;
    Prepare(First, m_Geometry);
    First.SetTotalEnergyMax(800.0);
    Passed = EvaluateTrue("SetInputModeFile()", "first file", "The file is opened", OpenInput(First, FileName1)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "first file", "The pre-analysis succeeds", RunPreAnalysis(First)) && Passed;
    AnalyzeAll(First);

    MRawEventAnalyzer Second;
    Prepare(Second, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "second file", "The file is opened", OpenInput(Second, FileName2)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "second file", "The pre-analysis succeeds", RunPreAnalysis(Second)) && Passed;
    AnalyzeAll(Second);

    Second.JoinStatistics(First);
    MString Output = RunPostAnalysis(Second);
    Passed = Evaluate("JoinStatistics()", "1 + 3 events", "The number of events is the sum", GetStatistic(Output, "Number of events ."), 4) && Passed;
    Passed = Evaluate("JoinStatistics()", "1 + 2 events which passed", "The number of events which passed the selections is the sum", GetStatistic(Output, "Number of events, which passed event selections ."), 3) && Passed;
    Passed = Evaluate("JoinStatistics()", "1 + 1 single-site events", "The number of single-site events is the sum", GetStatistic(Output, "Single-site  ."), 2) && Passed;
    Passed = Evaluate("JoinStatistics()", "0 + 1 Compton events", "The number of Compton events is the sum", GetStatistic(Output, "Compton  ."), 1) && Passed;
    Passed = Evaluate("JoinStatistics()", "0 + 1 rejections for the energy", "The rejection is taken over", GetStatistic(Output, MRERawEvent::GetRejectionReasonAsString(MRERawEvent::c_RejectionTotalEnergyOutOfLimits) + " ."), 1) && Passed;
    Passed = Evaluate("JoinStatistics()", "0 + 1 rejections", "The number of rejections is the sum", GetStatistic(Output, "Total ."), 1) && Passed;

    // The analyzer which was joined is unchanged
    Output = RunPostAnalysis(First);
    Passed = Evaluate("JoinStatistics()", "joined analyzer", "The joined analyzer keeps its number of events", GetStatistic(Output, "Number of events ."), 3) && Passed;
  }

  // A rejection reason of 30 or above (here: the external bad flag, 30) is behind the initial size of the list of rejections
  const MString FileName3 = GetTemporaryFileName("Join3.evta");
  Events = GetEvtaHeader();
  Events += GetEventStart(1, "1.0") + "BD Bad\n" + GetCalorimeterHit();
  Events += "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "one bad event", "The third evta file can be written", WriteTextFile(FileName3, Events)) && Passed;
  {
    MRawEventAnalyzer Bad;
    Prepare(Bad, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "third file", "The file is opened", OpenInput(Bad, FileName3)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "third file", "The pre-analysis succeeds", RunPreAnalysis(Bad)) && Passed;
    AnalyzeAll(Bad);

    MRawEventAnalyzer Second;
    Prepare(Second, m_Geometry);
    Passed = EvaluateTrue("SetInputModeFile()", "second file", "The file is opened", OpenInput(Second, FileName2)) && Passed;
    Passed = EvaluateTrue("PreAnalysis()", "second file", "The pre-analysis succeeds", RunPreAnalysis(Second)) && Passed;
    AnalyzeAll(Second);

    // Defect REV-H-5: JoinStatistics() only adds the rejection reasons which its own list already has, so the reasons 30 and above of the other analyzer are lost
    Second.JoinStatistics(Bad);
    MString Output = RunPostAnalysis(Second);
    Passed = Evaluate("JoinStatistics()", "1 + 1 events", "The number of events is the sum", GetStatistic(Output, "Number of events ."), 2) && Passed;
    Passed = ExpectedToFail(Evaluate("JoinStatistics()", "0 + 1 rejections for a bad flag", "The rejection is taken over", GetStatistic(Output, "Total ."), 1)) && Passed;
  }

  // Defect REV-H-5: In the other direction JoinStatistics() reads behind the end of the shorter list of the other analyzer
  Passed = ExpectedToCrash(EvaluateTrue("JoinStatistics()", "1 + 0 rejections for a bad flag", "The rejection is kept", JoinIntoLargerRejectionList())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test calls on an analyzer which is not completely set up
bool UTRawEventAnalyzer::TestIncompleteSetup()
{
  bool Passed = true;

  // An event in the store, but no pre-analysis: the stages do not exist
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("AddRawEvent(MString)", "no pre-analysis", "The event is added", AddEventString(Analyzer, GetEventStart(7, "3.5") + GetCalorimeterHit(), false, 21)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "no pre-analysis", "The analysis reports an error", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisUndefinedError) && Passed;
  }

  // The tracking is switched on after the pre-analysis
  {
    MRawEventAnalyzer Analyzer;
    Prepare(Analyzer, m_Geometry);
    Passed = EvaluateTrue("PreAnalysis()", "no tracking", "The pre-analysis succeeds", RunPreAnalysis(Analyzer)) && Passed;
    Analyzer.SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoModifiedPearson);
    Passed = EvaluateTrue("AddRawEvent(MString)", "tracking set after the pre-analysis", "The event is added", AddEventString(Analyzer, GetEventStart(7, "3.5") + GetCalorimeterHit(), false, 21)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "tracking set after the pre-analysis", "The analysis reports an error", AnalyzeNext(Analyzer), MRawEventAnalyzer::c_AnalysisUndefinedError) && Passed;
  }

  // Defect REV-H-8: AddRawEvent() with noising uses the noising object, which only exists after SetGeometry()
  Passed = ExpectedToCrash(EvaluateTrue("AddRawEvent(MString)", "no geometry", "The event is refused", AddEventWithoutGeometry())) && Passed;
  // Defect REV-H-8: PostAnalysis() asks the geometry for its file name without checking that there is a geometry
  Passed = ExpectedToCrash(EvaluateTrue("PostAnalysis()", "no geometry", "The call returns", PostAnalysisWithoutGeometry())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the header of an evta file for the telescope
MString UTRawEventAnalyzer::GetEvtaHeader()
{
  // Version 21: the hits are "HT <detector type>;<x>;<y>;<z>;<energy>;<dx>;<dy>;<dz>;<energy resolution>", and are not noised
  return MString("Type EVTA\nVersion 21\nGeometry ") + m_Geometry.GetFileName() + "\n\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the header of a sim file for the given geometry
MString UTRawEventAnalyzer::GetSimHeader(MGeometryRevan& Geometry)
{
  // Version 25: the hits are "HTsim <detector type>;<x>;<y>;<z>;<energy>;<time>", and are noised
  return MString("Type SIM\nVersion 25\nGeometry ") + Geometry.GetFileName() + "\n\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the start of an event: the SE, ID, and TI lines
MString UTRawEventAnalyzer::GetEventStart(unsigned int ID, const MString& Time) const
{
  MString Start = "SE\nID ";
  Start += ID;
  Start += "\nTI ";
  Start += Time;
  Start += "\n";
  return Start;
}


////////////////////////////////////////////////////////////////////////////////


//! Return an evta hit of 662 keV in the center of the calorimeter block, with the resolutions of the telescope
MString UTRawEventAnalyzer::GetCalorimeterHit() const
{
  return "HT 2;0;0;-10;662;2.88;2.88;2.3;10\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return two evta hits: 200 keV in the center of tracker layer 0, and 500 keV in the calorimeter at (1, 2, -10) cm
MString UTRawEventAnalyzer::GetComptonHits() const
{
  return "HT 1;0;0;0;200;0.29;0.29;0.014;2\nHT 2;1;2;-10;500;2.88;2.88;2.3;10\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the Compton scatter angle in rad for the energies of the recoil electron and the scattered gamma ray in keV
double UTRawEventAnalyzer::GetComptonAngle(double ElectronEnergy, double GammaEnergy) const
{
  // The Compton equation: cos(phi) = 1 - E0/Eg + E0/(Ee + Eg)
  return acos(1.0 - c_E0/GammaEnergy + c_E0/(ElectronEnergy + GammaEnergy));
}


////////////////////////////////////////////////////////////////////////////////


//! Set the geometry and the batch mode
void UTRawEventAnalyzer::Prepare(MRawEventAnalyzer& Analyzer, MGeometryRevan& Geometry)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  Analyzer.SetGeometry(&Geometry);
  Analyzer.SetBatch(true);
  EnableDefaultStreams();
  EndCaptureOutput();
}


////////////////////////////////////////////////////////////////////////////////


//! Call SetInputModeFile() with all output silenced
bool UTRawEventAnalyzer::OpenInput(MRawEventAnalyzer& Analyzer, const MString& FileName)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Result = Analyzer.SetInputModeFile(FileName);
  EnableDefaultStreams();
  EndCaptureOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Call SetOutputModeFile() with all output silenced
bool UTRawEventAnalyzer::OpenOutput(MRawEventAnalyzer& Analyzer, const MString& FileName)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Result = Analyzer.SetOutputModeFile(FileName);
  EnableDefaultStreams();
  EndCaptureOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Call PreAnalysis() with all output silenced
bool UTRawEventAnalyzer::RunPreAnalysis(MRawEventAnalyzer& Analyzer)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Result = Analyzer.PreAnalysis();
  EnableDefaultStreams();
  EndCaptureOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Call AnalyzeEvent() with all output silenced
unsigned int UTRawEventAnalyzer::AnalyzeNext(MRawEventAnalyzer& Analyzer)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  unsigned int Code = Analyzer.AnalyzeEvent();
  EnableDefaultStreams();
  EndCaptureOutput();
  return Code;
}


////////////////////////////////////////////////////////////////////////////////


//! Call AnalyzeEvent() until no event is left (at most 100 times), and return the codes
vector<unsigned int> UTRawEventAnalyzer::AnalyzeAll(MRawEventAnalyzer& Analyzer)
{
  vector<unsigned int> Codes;
  for (unsigned int i = 0; i < 100; ++i) {
    unsigned int Code = AnalyzeNext(Analyzer);
    Codes.push_back(Code);
    if (Code == MRawEventAnalyzer::c_AnalysisNoEventsLeftInFile || Code == MRawEventAnalyzer::c_AnalysisNoEventsInStore) {
      break;
    }
  }
  return Codes;
}


////////////////////////////////////////////////////////////////////////////////


//! Call PostAnalysis() and return its output
MString UTRawEventAnalyzer::RunPostAnalysis(MRawEventAnalyzer& Analyzer)
{
  // The statistics are printed with mout, which writes to cout
  BeginCaptureOutput();
  Analyzer.PostAnalysis();
  return EndCaptureOutput();
}


////////////////////////////////////////////////////////////////////////////////


//! Add an event string to the store with all output silenced
bool UTRawEventAnalyzer::AddEventString(MRawEventAnalyzer& Analyzer, const MString& Event, bool NeedsNoising, int Version)
{
  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Result = Analyzer.AddRawEvent(Event, NeedsNoising, Version);
  EnableDefaultStreams();
  EndCaptureOutput();
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Open the file, analyze all events, and return the output of PostAnalysis()
MString UTRawEventAnalyzer::AnalyzeFile(MRawEventAnalyzer& Analyzer, const MString& FileName, const MString& OutputFileName)
{
  if (OpenInput(Analyzer, FileName) == false) {
    return "";
  }
  if (OutputFileName != "") {
    if (OpenOutput(Analyzer, OutputFileName) == false) {
      return "";
    }
  }
  if (RunPreAnalysis(Analyzer) == false) {
    return "";
  }
  AnalyzeAll(Analyzer);
  return RunPostAnalysis(Analyzer);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the count of the line of the statistics which starts with the label, -1 if there is none
int UTRawEventAnalyzer::GetStatistic(const MString& Output, const MString& Label) const
{
  // The lines look like "  Number of events ...........      3 (100.000%)" or "  The total energy is out of limits ......      4"
  vector<MString> Lines = Output.Tokenize("\n");
  for (unsigned int l = 0; l < Lines.size(); ++l) {
    MString Line = Lines[l];
    Line.StripFrontInPlace(' ');
    if (Line.BeginsWith(Label) == true) {
      // The count follows the last group of dots; the percentage behind it has only single dots
      size_t Dots = Line.FindLast(".. ");
      if (Dots == string::npos) {
        return -1;
      }
      return Line.GetSubString(Dots + 3).ToInt();
    }
  }

  return -1;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the value of the option "# <Key>: <Value>" of the options block, "<missing>" if there is none
MString UTRawEventAnalyzer::GetOption(const MString& Output, const MString& Key) const
{
  const MString Start = MString("# ") + Key + ":";
  vector<MString> Lines = Output.Tokenize("\n");
  for (unsigned int l = 0; l < Lines.size(); ++l) {
    if (Lines[l].BeginsWith(Start) == true) {
      MString Value = Lines[l].GetSubString(Start.Length());
      Value.TrimInPlace();
      return Value;
    }
  }

  return "<missing>";
}


////////////////////////////////////////////////////////////////////////////////


//! Read all events of a tra file, its number of simulated events, and its observation time in s
bool UTRawEventAnalyzer::ReadTraFile(const MString& FileName, vector<unique_ptr<MPhysicalEvent>>& Events, long& SimulatedEvents, double& ObservationTime)
{
  MFileEventsTra File;

  BeginCaptureOutput();
  DisableDefaultStreams();
  bool Opened = File.Open(FileName);
  if (Opened == true) {
    MPhysicalEvent* Event = nullptr;
    while ((Event = File.GetNextEvent()) != nullptr) {
      Events.push_back(unique_ptr<MPhysicalEvent>(Event));
    }
    SimulatedEvents = File.GetSimulatedEvents();
    ObservationTime = File.GetObservationTime().GetAsDouble();
    File.Close();
  }
  EnableDefaultStreams();
  EndCaptureOutput();

  return Opened;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if PreAnalysis() succeeds after the algorithm of the stage (0: coincidence, 1: event clustering, 2: hit clustering, 3: tracking, 4: Compton sequence, 5: decay) is set
bool UTRawEventAnalyzer::IsAlgorithmAccepted(unsigned int Stage, int Algorithm)
{
  MRawEventAnalyzer Analyzer;
  Prepare(Analyzer, m_Geometry);
  // Parameters which the constructor does not set (see REV-H-1)
  Analyzer.SetCoincidenceWindow(1E-6);
  Analyzer.SetEventClusteringDistanceCutOff(1.0);

  if (Stage == 0) {
    Analyzer.SetCoincidenceAlgorithm(Algorithm);
  } else if (Stage == 1) {
    Analyzer.SetEventClusteringAlgorithm(Algorithm);
  } else if (Stage == 2) {
    Analyzer.SetHitClusteringAlgorithm(Algorithm);
  } else if (Stage == 3) {
    Analyzer.SetTrackingAlgorithm(Algorithm);
  } else if (Stage == 4) {
    Analyzer.SetCSRAlgorithm(Algorithm);
  } else {
    Analyzer.SetDecayAlgorithm(Algorithm);
  }

  return RunPreAnalysis(Analyzer);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if PreAnalysis() accepts the event clustering by distance without a cut off, on an analyzer built in memory filled with the pattern
bool UTRawEventAnalyzer::IsDistanceClusteringAcceptedOnPattern(unsigned char Pattern)
{
  alignas(MRawEventAnalyzer) unsigned char Block[sizeof(MRawEventAnalyzer)];
  FillMemory(Block, sizeof(Block), Pattern);
  MRawEventAnalyzer* Analyzer = new (Block) MRawEventAnalyzer();

  Prepare(*Analyzer, m_Geometry);
  Analyzer->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoDistance);
  bool Accepted = RunPreAnalysis(*Analyzer);

  Analyzer->~MRawEventAnalyzer();

  return Accepted;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of events found by the coincidence search without a coincidence window, on an analyzer built in memory filled with the pattern
unsigned int UTRawEventAnalyzer::GetNCoincidentEventsOnPattern(unsigned char Pattern, const MString& FileName)
{
  alignas(MRawEventAnalyzer) unsigned char Block[sizeof(MRawEventAnalyzer)];
  FillMemory(Block, sizeof(Block), Pattern);
  MRawEventAnalyzer* Analyzer = new (Block) MRawEventAnalyzer();

  Prepare(*Analyzer, m_Geometry);
  Analyzer->SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoWindow);
  unsigned int NEvents = 0;
  if (OpenInput(*Analyzer, FileName) == true && RunPreAnalysis(*Analyzer) == true) {
    vector<unsigned int> Codes = AnalyzeAll(*Analyzer);
    for (unsigned int c = 0; c < Codes.size(); ++c) {
      if (Codes[c] == MRawEventAnalyzer::c_AnalysisSucess) {
        ++NEvents;
      }
    }
  }

  Analyzer->~MRawEventAnalyzer();

  return NEvents;
}


////////////////////////////////////////////////////////////////////////////////


//! Join the statistics of an analyzer without rejections into one with a rejection reason of 30 or above, return true if the rejection is still counted
bool UTRawEventAnalyzer::JoinIntoLargerRejectionList()
{
  MRawEventAnalyzer Bad;
  Prepare(Bad, m_Geometry);
  if (OpenInput(Bad, GetTemporaryFileName("Join3.evta")) == false || RunPreAnalysis(Bad) == false) {
    return false;
  }
  AnalyzeAll(Bad);

  MRawEventAnalyzer Empty;
  Prepare(Empty, m_Geometry);

  Bad.JoinStatistics(Empty);
  MString Output = RunPostAnalysis(Bad);
  if (GetStatistic(Output, "Total .") != 1) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Add an event string to an analyzer without a geometry, return true if it is refused
bool UTRawEventAnalyzer::AddEventWithoutGeometry()
{
  MRawEventAnalyzer Analyzer;
  if (AddEventString(Analyzer, GetEventStart(1, "1.0") + "HTsim 2;0;0;-10;662;0\n", true, 25) == true) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Call PostAnalysis() on an analyzer without a geometry, return true if it returns
bool UTRawEventAnalyzer::PostAnalysisWithoutGeometry()
{
  MRawEventAnalyzer Analyzer;
  RunPostAnalysis(Analyzer);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  MGlobal::Initialize("UTRawEventAnalyzer", "Unit tests for MRawEventAnalyzer");

  UTRawEventAnalyzer Test;
  if (Test.Run() == false) {
    return 1;
  }

  return 0;
}


// UTRawEventAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
