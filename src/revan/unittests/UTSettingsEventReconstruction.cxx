/*
 * UTSettingsEventReconstruction.cxx
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
#include <new>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MERCSRTMVAMethods.h"
#include "MFile.h"
#include "MSettingsEventReconstruction.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"


////////////////////////////////////////////////////////////////////////////////


//! Give access to the protected XML functions of MSettingsEventReconstruction
class UTSettingsEventReconstructionAccess : public MSettingsEventReconstruction
{
public:
  using MSettingsEventReconstruction::ReadXml;
  using MSettingsEventReconstruction::WriteXml;
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the settings of the event reconstruction MSettingsEventReconstruction
class UTSettingsEventReconstruction : public MUnitTest
{
public:
  //! Default constructor
  UTSettingsEventReconstruction() : MUnitTest("UTSettingsEventReconstruction") {}
  //! Default destructor
  virtual ~UTSettingsEventReconstruction() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default values, also on memory with patterns
  bool TestDefaults();
  //! Test the setters and getters of the algorithms and of the general options
  bool TestGettersSettersGeneral();
  //! Test the setters and getters of the coincidence and clustering options
  bool TestGettersSettersClustering();
  //! Test the setters and getters of the tracking options
  bool TestGettersSettersTracking();
  //! Test the setters and getters of the Compton sequence and decay options
  bool TestGettersSettersCompton();
  //! Test a write and read round trip of all values
  bool TestRoundTrip();
  //! Test the round trip of the edge cases
  bool TestRoundTripEdgeCases();
  //! Test the values which are not part of the file
  bool TestNotStored();
  //! Test the paths in the XML tree
  bool TestPathCleaning();
  //! Test reading incomplete trees and the old keyword of the hit clustering algorithm
  bool TestReadIncomplete();
  //! Test repeated reads with one object
  bool TestReuse();

  //! Check that all values are the default values
  bool CheckDefaults(UTSettingsEventReconstructionAccess& Settings, const MString& Input);
  //! Set all values to values which are not the defaults
  void FillSettings(UTSettingsEventReconstructionAccess& Settings) const;
  //! Check that all stored values are the ones of FillSettings
  bool CheckFilled(UTSettingsEventReconstructionAccess& Settings, const MString& Input);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTSettingsEventReconstruction::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestGettersSettersGeneral() && Passed;
  Passed = TestGettersSettersClustering() && Passed;
  Passed = TestGettersSettersTracking() && Passed;
  Passed = TestGettersSettersCompton() && Passed;
  Passed = TestRoundTrip() && Passed;
  Passed = TestRoundTripEdgeCases() && Passed;
  Passed = TestNotStored() && Passed;
  Passed = TestPathCleaning() && Passed;
  Passed = TestReadIncomplete() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Check that all values are the default values
bool UTSettingsEventReconstruction::CheckDefaults(UTSettingsEventReconstructionAccess& Settings, const MString& Input)
{
  bool Passed = true;

  const MString Function = "MSettingsEventReconstruction()";

  // The algorithms
  Passed = Evaluate(Function, Input, "The default coincidence algorithm is 0", Settings.GetCoincidenceAlgorithm(), 0) && Passed;
  Passed = Evaluate(Function, Input, "The default event clustering algorithm is 0", Settings.GetEventClusteringAlgorithm(), 0) && Passed;
  Passed = Evaluate(Function, Input, "The default hit clustering algorithm is 1", Settings.GetHitClusteringAlgorithm(), 1) && Passed;
  Passed = Evaluate(Function, Input, "The default tracking algorithm is 0", Settings.GetTrackingAlgorithm(), 0) && Passed;
  Passed = Evaluate(Function, Input, "The default Compton sequence algorithm is 1", Settings.GetCSRAlgorithm(), 1) && Passed;
  Passed = Evaluate(Function, Input, "The default decay algorithm is 0", Settings.GetDecayAlgorithm(), 0) && Passed;

  // Coincidence and clustering
  Passed = EvaluateNear(Function, Input, "The default coincidence window is 1 microsecond", Settings.GetCoincidenceWindow(), 1E-6, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default event clustering distance cut-off is 1000", Settings.GetEventClusteringDistanceCutOff(), 1000.0, 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default event clustering TMVA file name is empty", Settings.GetEventClusteringTMVAFileName(), MString("")) && Passed;
  Passed = Evaluate(Function, Input, "The default event clustering TMVA method is BDTD", Settings.GetEventClusteringTMVAMethods().GetUsedMethodsString(), MString("BDTD")) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D1 is 0.2", Settings.GetStandardClusterizerMinDistanceD1(), 0.2, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D2 is 1.1", Settings.GetStandardClusterizerMinDistanceD2(), 1.1, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D3 is 0.3", Settings.GetStandardClusterizerMinDistanceD3(), 0.3, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D4 is 0", Settings.GetStandardClusterizerMinDistanceD4(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D5 is 0.2", Settings.GetStandardClusterizerMinDistanceD5(), 0.2, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D6 is 0.3", Settings.GetStandardClusterizerMinDistanceD6(), 0.3, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D7 is 0.3", Settings.GetStandardClusterizerMinDistanceD7(), 0.3, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum distance of the standard clusterizer for D8 is 0.3", Settings.GetStandardClusterizerMinDistanceD8(), 0.3, 0.0) && Passed;
  Passed = EvaluateFalse(Function, Input, "The center is not the reference of the standard clusterizer by default", Settings.GetStandardClusterizerCenterIsReference()) && Passed;
  Passed = Evaluate(Function, Input, "The default adjacent level is 2", Settings.GetAdjacentLevel(), 2) && Passed;
  Passed = EvaluateNear(Function, Input, "The default adjacent sigma is -1", Settings.GetAdjacentSigma(), -1.0, 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default base file name of the PDF clusterizer is empty", Settings.GetPDFClusterizerBaseFileName(), MString("")) && Passed;

  // Tracking
  Passed = EvaluateTrue(Function, Input, "The tracking is done by default", Settings.GetDoTracking()) && Passed;
  Passed = EvaluateTrue(Function, Input, "Pairs are searched by default", Settings.GetSearchPairs()) && Passed;
  Passed = EvaluateTrue(Function, Input, "MIPs are searched by default", Settings.GetSearchMIPs()) && Passed;
  Passed = EvaluateTrue(Function, Input, "Comptons are searched by default", Settings.GetSearchComptons()) && Passed;
  Passed = Evaluate(Function, Input, "The default maximum Compton jump is 2", Settings.GetMaxComptonJump(), 2) && Passed;
  Passed = Evaluate(Function, Input, "The default number of track sequences to keep is 1", Settings.GetNTrackSequencesToKeep(), 1) && Passed;
  Passed = EvaluateFalse(Function, Input, "Purely ambiguous track sequences are not rejected by default", Settings.GetRejectPurelyAmbiguousTrackSequences()) && Passed;
  Passed = Evaluate(Function, Input, "The default number of layers for the vertex search is 6", Settings.GetNLayersForVertexSearch(), 6) && Passed;
  Passed = Evaluate(Function, Input, "There are no electron tracking detectors by default", Settings.GetNElectronTrackingDetectors(), 0U) && Passed;
  Passed = Evaluate(Function, Input, "The default Bayesian electron file name is empty", Settings.GetBayesianElectronFileName(), MString("")) && Passed;

  // Compton sequence reconstruction
  Passed = EvaluateFalse(Function, Input, "D1 is not assumed first by default", Settings.GetAssumeD1First()) && Passed;
  Passed = Evaluate(Function, Input, "The default classic undecided handling is 0", Settings.GetClassicUndecidedHandling(), 0) && Passed;
  Passed = EvaluateFalse(Function, Input, "Tracks are not assumed to go from top to bottom by default", Settings.GetAssumeTrackTopBottom()) && Passed;
  Passed = EvaluateTrue(Function, Input, "COMPTEL type events are used by default", Settings.GetUseComptelTypeEvents()) && Passed;
  Passed = EvaluateFalse(Function, Input, "A start in D1 is not guaranteed by default", Settings.GetGuaranteeStartD1()) && Passed;
  Passed = EvaluateFalse(Function, Input, "Events in one detector type only are not rejected by default", Settings.GetRejectOneDetectorTypeOnlyEvents()) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum Compton sequence threshold is 0", Settings.GetCSRThresholdMin(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default maximum Compton sequence threshold is 1000", Settings.GetCSRThresholdMax(), 1000.0, 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default maximum number of hits for the Compton sequence is 5", Settings.GetCSRMaxNHits(), 5) && Passed;
  Passed = Evaluate(Function, Input, "The default Bayesian Compton file name is empty", Settings.GetBayesianComptonFileName(), MString("")) && Passed;
  Passed = Evaluate(Function, Input, "The default Compton sequence TMVA file name is empty", Settings.GetCSRTMVAFileName(), MString("")) && Passed;
  Passed = Evaluate(Function, Input, "The default Compton sequence TMVA method is BDTD", Settings.GetCSRTMVAMethods().GetUsedMethodsString(), MString("BDTD")) && Passed;
  Passed = Evaluate(Function, Input, "The default origin objects file name is empty", Settings.GetOriginObjectsFileName(), MString("")) && Passed;
  Passed = EvaluateVectorNear(Function, Input, "The default lens center is 10000 cm above the origin", Settings.GetLensCenter(), MVector(0.0, 0.0, 10000.0), 0.0) && Passed;
  Passed = EvaluateVectorNear(Function, Input, "The default focal spot center is the origin", Settings.GetFocalSpotCenter(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default decay file name is empty", Settings.GetDecayFileName(), MString("")) && Passed;

  // General options
  Passed = EvaluateNear(Function, Input, "The default minimum total energy is 0", Settings.GetTotalEnergyMin(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default maximum total energy is 1000000", Settings.GetTotalEnergyMax(), 1000000.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum lever arm is 0", Settings.GetLeverArmMin(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default maximum lever arm is 1000", Settings.GetLeverArmMax(), 1000.0, 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default minimum event ID is -1", Settings.GetEventIdMin(), -1L) && Passed;
  Passed = Evaluate(Function, Input, "The default maximum event ID is -1", Settings.GetEventIdMax(), -1L) && Passed;
  Passed = EvaluateTrue(Function, Input, "All bad events are rejected by default", Settings.GetRejectAllBadEvents()) && Passed;
  Passed = Evaluate(Function, Input, "The default number of jobs is 1", Settings.GetNJobs(), 1U) && Passed;
  Passed = EvaluateFalse(Function, Input, "The origin information is not saved by default", Settings.GetSaveOI()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The special mode is off by default", Settings.GetSpecialMode()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all values to values which are not the defaults
void UTSettingsEventReconstruction::FillSettings(UTSettingsEventReconstructionAccess& Settings) const
{
  Settings.SetNJobs(7);
  Settings.SetCoincidenceAlgorithm(1);
  Settings.SetEventClusteringAlgorithm(2);
  Settings.SetHitClusteringAlgorithm(3);
  Settings.SetTrackingAlgorithm(4);
  Settings.SetCSRAlgorithm(5);
  Settings.SetDecayAlgorithm(6);

  Settings.SetCoincidenceWindow(2.5E-5);
  Settings.SetEventClusteringDistanceCutOff(12.25);
  Settings.SetEventClusteringTMVAFileName("/some/where/EventClustering.tmva");
  MERCSRTMVAMethods EventClusteringMethods;
  EventClusteringMethods.SetUsedMethods("MLP");
  Settings.SetEventClusteringTMVAMethods(EventClusteringMethods);

  Settings.SetStandardClusterizerMinDistanceD1(1.125);
  Settings.SetStandardClusterizerMinDistanceD2(2.125);
  Settings.SetStandardClusterizerMinDistanceD3(3.125);
  Settings.SetStandardClusterizerMinDistanceD4(4.125);
  Settings.SetStandardClusterizerMinDistanceD5(5.125);
  Settings.SetStandardClusterizerMinDistanceD6(6.125);
  Settings.SetStandardClusterizerMinDistanceD7(7.125);
  Settings.SetStandardClusterizerMinDistanceD8(8.125);
  Settings.SetStandardClusterizerCenterIsReference(true);
  Settings.SetAdjacentSigma(3.5);
  Settings.SetAdjacentLevel(4);
  Settings.SetPDFClusterizerBaseFileName("/some/where/PDFClusterizer");

  Settings.SetDoTracking(false);
  Settings.SetSearchPairs(false);
  Settings.SetSearchMIPs(false);
  Settings.SetSearchComptons(false);
  Settings.SetMaxComptonJump(9);
  Settings.SetNTrackSequencesToKeep(11);
  Settings.SetRejectPurelyAmbiguousTrackSequences(true);
  Settings.SetNLayersForVertexSearch(13);
  Settings.AddElectronTrackingDetector("Tracker");
  Settings.AddElectronTrackingDetector("Second Tracker");
  Settings.SetBayesianElectronFileName("/some/where/Electron.rsp");

  Settings.SetAssumeD1First(true);
  Settings.SetClassicUndecidedHandling(3);
  Settings.SetAssumeTrackTopBottom(true);
  Settings.SetUseComptelTypeEvents(false);
  Settings.SetGuaranteeStartD1(true);
  Settings.SetRejectOneDetectorTypeOnlyEvents(true);
  Settings.SetCSRThresholdMin(0.125);
  Settings.SetCSRThresholdMax(0.875);
  Settings.SetCSRMaxNHits(8);
  Settings.SetBayesianComptonFileName("/some/where/Compton.rsp");
  Settings.SetCSRTMVAFileName("/some/where/CSR.tmva");
  MERCSRTMVAMethods CSRMethods;
  CSRMethods.SetUsedMethods("MLP,DNN_CPU");
  Settings.SetCSRTMVAMethods(CSRMethods);
  Settings.SetOriginObjectsFileName("/some/where/Origins.dat");
  Settings.SetLensCenter(MVector(1.25, -2.5, 3.0625));
  Settings.SetFocalSpotCenter(MVector(-10.5, 20.25, -0.125));
  Settings.SetDecayFileName("/some/where/Decays.decay");

  Settings.SetTotalEnergyMin(100.5);
  Settings.SetTotalEnergyMax(2000.25);
  Settings.SetLeverArmMin(0.5);
  Settings.SetLeverArmMax(55.5);
  Settings.SetEventIdMin(1000);
  Settings.SetEventIdMax(123456789012L);
  Settings.SetRejectAllBadEvents(false);
}


////////////////////////////////////////////////////////////////////////////////


//! Check that all stored values are the ones of FillSettings
bool UTSettingsEventReconstruction::CheckFilled(UTSettingsEventReconstructionAccess& Settings, const MString& Input)
{
  bool Passed = true;

  const MString Function = "ReadXml()";
  const MString Description = "The value of the XML tree is used";

  Passed = Evaluate(Function, Input + ": number of jobs", Description, Settings.GetNJobs(), 7U) && Passed;
  Passed = Evaluate(Function, Input + ": coincidence algorithm", Description, Settings.GetCoincidenceAlgorithm(), 1) && Passed;
  Passed = Evaluate(Function, Input + ": event clustering algorithm", Description, Settings.GetEventClusteringAlgorithm(), 2) && Passed;
  Passed = Evaluate(Function, Input + ": hit clustering algorithm", Description, Settings.GetHitClusteringAlgorithm(), 3) && Passed;
  Passed = Evaluate(Function, Input + ": tracking algorithm", Description, Settings.GetTrackingAlgorithm(), 4) && Passed;
  Passed = Evaluate(Function, Input + ": Compton sequence algorithm", Description, Settings.GetCSRAlgorithm(), 5) && Passed;
  Passed = Evaluate(Function, Input + ": decay algorithm", Description, Settings.GetDecayAlgorithm(), 6) && Passed;

  // The numbers are written with 15 or more significant digits, and all values but the window are exact binary fractions
  Passed = EvaluateNear(Function, Input + ": coincidence window", Description, Settings.GetCoincidenceWindow(), 2.5E-5, 1E-19) && Passed;
  Passed = EvaluateNear(Function, Input + ": event clustering distance cut-off", Description, Settings.GetEventClusteringDistanceCutOff(), 12.25, 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": event clustering TMVA file", Description, Settings.GetEventClusteringTMVAFileName(), MString("/some/where/EventClustering.tmva")) && Passed;
  Passed = Evaluate(Function, Input + ": event clustering TMVA methods", Description, Settings.GetEventClusteringTMVAMethods().GetUsedMethodsString(), MString("MLP")) && Passed;

  Passed = EvaluateNear(Function, Input + ": minimum distance D1", Description, Settings.GetStandardClusterizerMinDistanceD1(), 1.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D2", Description, Settings.GetStandardClusterizerMinDistanceD2(), 2.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D3", Description, Settings.GetStandardClusterizerMinDistanceD3(), 3.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D4", Description, Settings.GetStandardClusterizerMinDistanceD4(), 4.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D5", Description, Settings.GetStandardClusterizerMinDistanceD5(), 5.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D6", Description, Settings.GetStandardClusterizerMinDistanceD6(), 6.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D7", Description, Settings.GetStandardClusterizerMinDistanceD7(), 7.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum distance D8", Description, Settings.GetStandardClusterizerMinDistanceD8(), 8.125, 0.0) && Passed;
  Passed = EvaluateTrue(Function, Input + ": center is reference", Description, Settings.GetStandardClusterizerCenterIsReference()) && Passed;
  Passed = EvaluateNear(Function, Input + ": adjacent sigma", Description, Settings.GetAdjacentSigma(), 3.5, 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": adjacent level", Description, Settings.GetAdjacentLevel(), 4) && Passed;
  Passed = Evaluate(Function, Input + ": PDF clusterizer base file name", Description, Settings.GetPDFClusterizerBaseFileName(), MString("/some/where/PDFClusterizer")) && Passed;

  Passed = EvaluateFalse(Function, Input + ": do tracking", Description, Settings.GetDoTracking()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": search pairs", Description, Settings.GetSearchPairs()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": search MIPs", Description, Settings.GetSearchMIPs()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": search Comptons", Description, Settings.GetSearchComptons()) && Passed;
  Passed = Evaluate(Function, Input + ": maximum Compton jump", Description, Settings.GetMaxComptonJump(), 9) && Passed;
  Passed = Evaluate(Function, Input + ": track sequences to keep", Description, Settings.GetNTrackSequencesToKeep(), 11) && Passed;
  Passed = EvaluateTrue(Function, Input + ": reject purely ambiguous track sequences", Description, Settings.GetRejectPurelyAmbiguousTrackSequences()) && Passed;
  Passed = Evaluate(Function, Input + ": layers for vertex search", Description, Settings.GetNLayersForVertexSearch(), 13) && Passed;
  Passed = Evaluate(Function, Input + ": electron tracking detectors", "Both detectors are read", Settings.GetNElectronTrackingDetectors(), 2U) && Passed;
  Passed = Evaluate(Function, Input + ": first electron tracking detector", Description, Settings.GetElectronTrackingDetector(0), MString("Tracker")) && Passed;
  Passed = Evaluate(Function, Input + ": second electron tracking detector", "A name with a space is kept", Settings.GetElectronTrackingDetector(1), MString("Second Tracker")) && Passed;
  Passed = Evaluate(Function, Input + ": Bayesian electron file", Description, Settings.GetBayesianElectronFileName(), MString("/some/where/Electron.rsp")) && Passed;

  Passed = EvaluateTrue(Function, Input + ": assume D1 first", Description, Settings.GetAssumeD1First()) && Passed;
  Passed = Evaluate(Function, Input + ": classic undecided handling", Description, Settings.GetClassicUndecidedHandling(), 3) && Passed;
  Passed = EvaluateTrue(Function, Input + ": assume track top to bottom", Description, Settings.GetAssumeTrackTopBottom()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": use COMPTEL type events", Description, Settings.GetUseComptelTypeEvents()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": guarantee start in D1", Description, Settings.GetGuaranteeStartD1()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": reject one detector type only events", Description, Settings.GetRejectOneDetectorTypeOnlyEvents()) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum Compton sequence threshold", Description, Settings.GetCSRThresholdMin(), 0.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": maximum Compton sequence threshold", Description, Settings.GetCSRThresholdMax(), 0.875, 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": maximum number of hits", Description, Settings.GetCSRMaxNHits(), 8) && Passed;
  Passed = Evaluate(Function, Input + ": Bayesian Compton file", Description, Settings.GetBayesianComptonFileName(), MString("/some/where/Compton.rsp")) && Passed;
  Passed = Evaluate(Function, Input + ": Compton sequence TMVA file", Description, Settings.GetCSRTMVAFileName(), MString("/some/where/CSR.tmva")) && Passed;
  MERCSRTMVAMethods Methods = Settings.GetCSRTMVAMethods();
  Passed = Evaluate(Function, Input + ": Compton sequence TMVA methods", "Two methods are used", Methods.GetNumberOfUsedMethods(), 2U) && Passed;
  Passed = EvaluateTrue(Function, Input + ": Compton sequence TMVA methods", "MLP is used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_MLP)) && Passed;
  Passed = EvaluateTrue(Function, Input + ": Compton sequence TMVA methods", "DNN_CPU is used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_DNN_CPU)) && Passed;
  Passed = EvaluateFalse(Function, Input + ": Compton sequence TMVA methods", "The default BDTD is not used", Methods.IsUsedMethod(MERCSRTMVAMethod::c_BDTD)) && Passed;
  Passed = Evaluate(Function, Input + ": origin objects file", Description, Settings.GetOriginObjectsFileName(), MString("/some/where/Origins.dat")) && Passed;
  Passed = EvaluateVectorNear(Function, Input + ": lens center", Description, Settings.GetLensCenter(), MVector(1.25, -2.5, 3.0625), 0.0) && Passed;
  Passed = EvaluateVectorNear(Function, Input + ": focal spot center", Description, Settings.GetFocalSpotCenter(), MVector(-10.5, 20.25, -0.125), 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": decay file", Description, Settings.GetDecayFileName(), MString("/some/where/Decays.decay")) && Passed;

  Passed = EvaluateNear(Function, Input + ": minimum total energy", Description, Settings.GetTotalEnergyMin(), 100.5, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": maximum total energy", Description, Settings.GetTotalEnergyMax(), 2000.25, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": minimum lever arm", Description, Settings.GetLeverArmMin(), 0.5, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": maximum lever arm", Description, Settings.GetLeverArmMax(), 55.5, 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": minimum event ID", Description, Settings.GetEventIdMin(), 1000L) && Passed;
  Passed = Evaluate(Function, Input + ": maximum event ID", "An ID beyond 32 bits is kept", Settings.GetEventIdMax(), 123456789012L) && Passed;
  Passed = EvaluateFalse(Function, Input + ": reject all bad events", Description, Settings.GetRejectAllBadEvents()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default values, also on memory with patterns
bool UTSettingsEventReconstruction::TestDefaults()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Settings;
  Passed = CheckDefaults(Settings, "default") && Passed;

  // On memory with patterns: 0xA5 shows numbers and false flags which are not set, 0x00 shows true flags which are not set
  {
    alignas(UTSettingsEventReconstructionAccess) unsigned char Block[sizeof(UTSettingsEventReconstructionAccess)];
    FillMemory(Block, sizeof(Block), 0xA5);
    UTSettingsEventReconstructionAccess* Pattern = new (Block) UTSettingsEventReconstructionAccess();
    Passed = CheckDefaults(*Pattern, "memory with 0xA5") && Passed;
    Pattern->~UTSettingsEventReconstructionAccess();
  }
  {
    alignas(UTSettingsEventReconstructionAccess) unsigned char Block[sizeof(UTSettingsEventReconstructionAccess)];
    FillMemory(Block, sizeof(Block), 0x00);
    UTSettingsEventReconstructionAccess* Pattern = new (Block) UTSettingsEventReconstructionAccess();
    Passed = CheckDefaults(*Pattern, "memory with 0x00") && Passed;
    Pattern->~UTSettingsEventReconstructionAccess();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the algorithms and of the general options
bool UTSettingsEventReconstruction::TestGettersSettersGeneral()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Settings;

  for (int ID : { 3, 0, -1, 2147483647 }) {
    Settings.SetCoincidenceAlgorithm(ID);
    Passed = Evaluate("SetCoincidenceAlgorithm()", ID, "The algorithm is stored", Settings.GetCoincidenceAlgorithm(), ID) && Passed;
    Settings.SetEventClusteringAlgorithm(ID);
    Passed = Evaluate("SetEventClusteringAlgorithm()", ID, "The algorithm is stored", Settings.GetEventClusteringAlgorithm(), ID) && Passed;
    Settings.SetHitClusteringAlgorithm(ID);
    Passed = Evaluate("SetHitClusteringAlgorithm()", ID, "The algorithm is stored", Settings.GetHitClusteringAlgorithm(), ID) && Passed;
    Settings.SetTrackingAlgorithm(ID);
    Passed = Evaluate("SetTrackingAlgorithm()", ID, "The algorithm is stored", Settings.GetTrackingAlgorithm(), ID) && Passed;
    Settings.SetCSRAlgorithm(ID);
    Passed = Evaluate("SetCSRAlgorithm()", ID, "The algorithm is stored", Settings.GetCSRAlgorithm(), ID) && Passed;
    Settings.SetDecayAlgorithm(ID);
    Passed = Evaluate("SetDecayAlgorithm()", ID, "The algorithm is stored", Settings.GetDecayAlgorithm(), ID) && Passed;
  }

  // The six algorithms are independent
  Settings.SetCoincidenceAlgorithm(1);
  Settings.SetEventClusteringAlgorithm(2);
  Settings.SetHitClusteringAlgorithm(3);
  Settings.SetTrackingAlgorithm(4);
  Settings.SetCSRAlgorithm(5);
  Settings.SetDecayAlgorithm(6);
  Passed = Evaluate("SetCoincidenceAlgorithm()", "all six set", "The coincidence algorithm is its own value", Settings.GetCoincidenceAlgorithm(), 1) && Passed;
  Passed = Evaluate("SetEventClusteringAlgorithm()", "all six set", "The event clustering algorithm is its own value", Settings.GetEventClusteringAlgorithm(), 2) && Passed;
  Passed = Evaluate("SetHitClusteringAlgorithm()", "all six set", "The hit clustering algorithm is its own value", Settings.GetHitClusteringAlgorithm(), 3) && Passed;
  Passed = Evaluate("SetTrackingAlgorithm()", "all six set", "The tracking algorithm is its own value", Settings.GetTrackingAlgorithm(), 4) && Passed;
  Passed = Evaluate("SetCSRAlgorithm()", "all six set", "The Compton sequence algorithm is its own value", Settings.GetCSRAlgorithm(), 5) && Passed;
  Passed = Evaluate("SetDecayAlgorithm()", "all six set", "The decay algorithm is its own value", Settings.GetDecayAlgorithm(), 6) && Passed;

  for (unsigned int Jobs : { 4U, 0U, 4294967295U }) {
    Settings.SetNJobs(Jobs);
    Passed = Evaluate("SetNJobs()", Jobs, "The number of jobs is stored", Settings.GetNJobs(), Jobs) && Passed;
  }

  for (bool Flag : { true, false, true }) {
    Settings.SetSaveOI(Flag);
    Passed = Evaluate("SetSaveOI()", Flag, "The flag is stored", Settings.GetSaveOI(), Flag) && Passed;
    Settings.SetSpecialMode(!Flag);
    Passed = Evaluate("SetSpecialMode()", !Flag, "The flag is stored", Settings.GetSpecialMode(), !Flag) && Passed;
    Settings.SetRejectAllBadEvents(Flag);
    Passed = Evaluate("SetRejectAllBadEvents()", Flag, "The flag is stored", Settings.GetRejectAllBadEvents(), Flag) && Passed;
    Passed = Evaluate("SetRejectAllBadEvents()", Flag, "The save origin information flag is not changed", Settings.GetSaveOI(), Flag) && Passed;
  }

  for (double Value : { 123.456, 0.0, -1.5, 1E30 }) {
    Settings.SetTotalEnergyMin(Value);
    Passed = EvaluateNear("SetTotalEnergyMin()", Value, "The value is stored", Settings.GetTotalEnergyMin(), Value, 0.0) && Passed;
    Settings.SetTotalEnergyMax(2*Value);
    Passed = EvaluateNear("SetTotalEnergyMax()", 2*Value, "The value is stored", Settings.GetTotalEnergyMax(), 2*Value, 0.0) && Passed;
    Settings.SetLeverArmMin(3*Value);
    Passed = EvaluateNear("SetLeverArmMin()", 3*Value, "The value is stored", Settings.GetLeverArmMin(), 3*Value, 0.0) && Passed;
    Settings.SetLeverArmMax(4*Value);
    Passed = EvaluateNear("SetLeverArmMax()", 4*Value, "The value is stored", Settings.GetLeverArmMax(), 4*Value, 0.0) && Passed;
    Passed = EvaluateNear("SetLeverArmMax()", 4*Value, "The minimum total energy is not changed", Settings.GetTotalEnergyMin(), Value, 0.0) && Passed;
  }

  for (long ID : { 5L, 0L, -1L, 9223372036854775807L }) {
    Settings.SetEventIdMin(ID);
    Passed = Evaluate("SetEventIdMin()", ID, "The ID is stored", Settings.GetEventIdMin(), ID) && Passed;
    Settings.SetEventIdMax(ID - 1);
    Passed = Evaluate("SetEventIdMax()", ID - 1, "The ID is stored", Settings.GetEventIdMax(), ID - 1) && Passed;
    Passed = Evaluate("SetEventIdMax()", ID - 1, "The minimum ID is not changed", Settings.GetEventIdMin(), ID) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the coincidence and clustering options
bool UTSettingsEventReconstruction::TestGettersSettersClustering()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Settings;

  for (double Value : { 2.5E-6, 0.0, -1.0, 1E20 }) {
    Settings.SetCoincidenceWindow(Value);
    Passed = EvaluateNear("SetCoincidenceWindow()", Value, "The value is stored", Settings.GetCoincidenceWindow(), Value, 0.0) && Passed;
    Settings.SetEventClusteringDistanceCutOff(Value);
    Passed = EvaluateNear("SetEventClusteringDistanceCutOff()", Value, "The value is stored", Settings.GetEventClusteringDistanceCutOff(), Value, 0.0) && Passed;
    Settings.SetAdjacentSigma(Value);
    Passed = EvaluateNear("SetAdjacentSigma()", Value, "The value is stored", Settings.GetAdjacentSigma(), Value, 0.0) && Passed;
  }

  // The eight minimum distances are independent
  Settings.SetStandardClusterizerMinDistanceD1(1.5);
  Settings.SetStandardClusterizerMinDistanceD2(2.5);
  Settings.SetStandardClusterizerMinDistanceD3(3.5);
  Settings.SetStandardClusterizerMinDistanceD4(4.5);
  Settings.SetStandardClusterizerMinDistanceD5(5.5);
  Settings.SetStandardClusterizerMinDistanceD6(6.5);
  Settings.SetStandardClusterizerMinDistanceD7(7.5);
  Settings.SetStandardClusterizerMinDistanceD8(8.5);
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD1()", 1.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD1(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD2()", 2.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD2(), 2.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD3()", 3.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD3(), 3.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD4()", 4.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD4(), 4.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD5()", 5.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD5(), 5.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD6()", 6.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD6(), 6.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD7()", 7.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD7(), 7.5, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD8()", 8.5, "The value is stored", Settings.GetStandardClusterizerMinDistanceD8(), 8.5, 0.0) && Passed;
  Settings.SetStandardClusterizerMinDistanceD1(0.0);
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD1()", 0.0, "Zero is stored", Settings.GetStandardClusterizerMinDistanceD1(), 0.0, 0.0) && Passed;
  Settings.SetStandardClusterizerMinDistanceD8(-2.0);
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD8()", -2.0, "A negative value is stored", Settings.GetStandardClusterizerMinDistanceD8(), -2.0, 0.0) && Passed;
  Passed = EvaluateNear("SetStandardClusterizerMinDistanceD8()", -2.0, "The value for D7 is not changed", Settings.GetStandardClusterizerMinDistanceD7(), 7.5, 0.0) && Passed;

  for (bool Flag : { true, false, true }) {
    Settings.SetStandardClusterizerCenterIsReference(Flag);
    Passed = Evaluate("SetStandardClusterizerCenterIsReference()", Flag, "The flag is stored", Settings.GetStandardClusterizerCenterIsReference(), Flag) && Passed;
  }

  for (int Level : { 1, 0, -3, 2147483647 }) {
    Settings.SetAdjacentLevel(Level);
    Passed = Evaluate("SetAdjacentLevel()", Level, "The level is stored", Settings.GetAdjacentLevel(), Level) && Passed;
  }

  for (const MString& Name : { MString("Other"), MString(""), MString("/with/path and space.tmva") }) {
    Settings.SetEventClusteringTMVAFileName(Name);
    Passed = Evaluate("SetEventClusteringTMVAFileName()", Name, "The name is stored", Settings.GetEventClusteringTMVAFileName(), Name) && Passed;
    Settings.SetPDFClusterizerBaseFileName(Name + "x");
    Passed = Evaluate("SetPDFClusterizerBaseFileName()", Name + "x", "The name is stored", Settings.GetPDFClusterizerBaseFileName(), Name + "x") && Passed;
    Passed = Evaluate("SetPDFClusterizerBaseFileName()", Name + "x", "The event clustering TMVA file name is not changed", Settings.GetEventClusteringTMVAFileName(), Name) && Passed;
  }

  // The methods are replaced as a whole
  MERCSRTMVAMethods Methods;
  Methods.SetUsedMethods("MLP,DNN_GPU");
  Settings.SetEventClusteringTMVAMethods(Methods);
  Passed = Evaluate("SetEventClusteringTMVAMethods()", "MLP,DNN_GPU", "Two methods are used", Settings.GetEventClusteringTMVAMethods().GetNumberOfUsedMethods(), 2U) && Passed;
  Passed = EvaluateTrue("SetEventClusteringTMVAMethods()", "MLP,DNN_GPU", "MLP is used", Settings.GetEventClusteringTMVAMethods().IsUsedMethod(MERCSRTMVAMethod::c_MLP)) && Passed;
  Passed = EvaluateFalse("SetEventClusteringTMVAMethods()", "MLP,DNN_GPU", "The default BDTD is not used any more", Settings.GetEventClusteringTMVAMethods().IsUsedMethod(MERCSRTMVAMethod::c_BDTD)) && Passed;
  Passed = Evaluate("SetEventClusteringTMVAMethods()", "MLP,DNN_GPU", "The methods of the Compton sequence reconstruction are not changed", Settings.GetCSRTMVAMethods().GetUsedMethodsString(), MString("BDTD")) && Passed;
  MERCSRTMVAMethods None;
  Settings.SetEventClusteringTMVAMethods(None);
  Passed = Evaluate("SetEventClusteringTMVAMethods()", "no methods", "No method is used", Settings.GetEventClusteringTMVAMethods().GetNumberOfUsedMethods(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the tracking options
bool UTSettingsEventReconstruction::TestGettersSettersTracking()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Settings;

  for (bool Flag : { false, true, false }) {
    Settings.SetDoTracking(Flag);
    Passed = Evaluate("SetDoTracking()", Flag, "The flag is stored", Settings.GetDoTracking(), Flag) && Passed;
    Settings.SetSearchPairs(!Flag);
    Passed = Evaluate("SetSearchPairs()", !Flag, "The flag is stored", Settings.GetSearchPairs(), !Flag) && Passed;
    Settings.SetSearchMIPs(Flag);
    Passed = Evaluate("SetSearchMIPs()", Flag, "The flag is stored", Settings.GetSearchMIPs(), Flag) && Passed;
    Settings.SetSearchComptons(!Flag);
    Passed = Evaluate("SetSearchComptons()", !Flag, "The flag is stored", Settings.GetSearchComptons(), !Flag) && Passed;
    Settings.SetRejectPurelyAmbiguousTrackSequences(Flag);
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", Flag, "The flag is stored", Settings.GetRejectPurelyAmbiguousTrackSequences(), Flag) && Passed;
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", Flag, "The do tracking flag is not changed", Settings.GetDoTracking(), Flag) && Passed;
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", Flag, "The search pairs flag is not changed", Settings.GetSearchPairs(), !Flag) && Passed;
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", Flag, "The search MIPs flag is not changed", Settings.GetSearchMIPs(), Flag) && Passed;
    Passed = Evaluate("SetRejectPurelyAmbiguousTrackSequences()", Flag, "The search Comptons flag is not changed", Settings.GetSearchComptons(), !Flag) && Passed;
  }

  for (int Value : { 7, 0, -1, 2147483647 }) {
    Settings.SetMaxComptonJump(Value);
    Passed = Evaluate("SetMaxComptonJump()", Value, "The value is stored", Settings.GetMaxComptonJump(), Value) && Passed;
    Settings.SetNTrackSequencesToKeep(Value - 1);
    Passed = Evaluate("SetNTrackSequencesToKeep()", Value - 1, "The value is stored", Settings.GetNTrackSequencesToKeep(), Value - 1) && Passed;
    Settings.SetNLayersForVertexSearch(Value - 2);
    Passed = Evaluate("SetNLayersForVertexSearch()", Value - 2, "The value is stored", Settings.GetNLayersForVertexSearch(), Value - 2) && Passed;
    Passed = Evaluate("SetNLayersForVertexSearch()", Value - 2, "The maximum Compton jump is not changed", Settings.GetMaxComptonJump(), Value) && Passed;
  }

  for (const MString& Name : { MString("Electron.rsp"), MString(""), MString("/with/path and space.rsp") }) {
    Settings.SetBayesianElectronFileName(Name);
    Passed = Evaluate("SetBayesianElectronFileName()", Name, "The name is stored", Settings.GetBayesianElectronFileName(), Name) && Passed;
  }

  // The list of the electron tracking detectors
  Passed = Evaluate("GetElectronTrackingDetector()", "empty list, index 0", "An index beyond the list gives an empty name", Settings.GetElectronTrackingDetector(0), MString("")) && Passed;
  Passed = EvaluateSize("GetElectronTrackingDetectors()", "empty list", "The list is empty", Settings.GetElectronTrackingDetectors().size(), 0) && Passed;
  Settings.AddElectronTrackingDetector("First");
  Passed = Evaluate("AddElectronTrackingDetector()", "First", "There is one detector", Settings.GetNElectronTrackingDetectors(), 1U) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "one detector, index 0", "The detector is returned", Settings.GetElectronTrackingDetector(0), MString("First")) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "one detector, index 1", "An index beyond the list gives an empty name", Settings.GetElectronTrackingDetector(1), MString("")) && Passed;
  Settings.AddElectronTrackingDetector("Second");
  Settings.AddElectronTrackingDetector("First");
  Passed = Evaluate("AddElectronTrackingDetector()", "First, Second, First", "A name can be added twice: there are three detectors", Settings.GetNElectronTrackingDetectors(), 3U) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "three detectors, index 1", "The detectors keep their order", Settings.GetElectronTrackingDetector(1), MString("Second")) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "three detectors, index 2", "The last detector is returned", Settings.GetElectronTrackingDetector(2), MString("First")) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "three detectors, index 3", "An index beyond the list gives an empty name", Settings.GetElectronTrackingDetector(3), MString("")) && Passed;
  Passed = Evaluate("GetElectronTrackingDetector()", "three detectors, largest index", "The largest index gives an empty name", Settings.GetElectronTrackingDetector(4294967295U), MString("")) && Passed;
  vector<MString> Detectors = Settings.GetElectronTrackingDetectors();
  Passed = EvaluateSize("GetElectronTrackingDetectors()", "three detectors", "The list has three entries", Detectors.size(), 3) && Passed;
  if (Detectors.size() == 3) {
    Passed = Evaluate("GetElectronTrackingDetectors()", "three detectors", "The first entry is the first detector", Detectors[0], MString("First")) && Passed;
    Passed = Evaluate("GetElectronTrackingDetectors()", "three detectors", "The second entry is the second detector", Detectors[1], MString("Second")) && Passed;
  }
  Settings.RemoveAllElectronTrackingDetectors();
  Passed = Evaluate("RemoveAllElectronTrackingDetectors()", "three detectors", "The list is empty", Settings.GetNElectronTrackingDetectors(), 0U) && Passed;
  Settings.RemoveAllElectronTrackingDetectors();
  Passed = Evaluate("RemoveAllElectronTrackingDetectors()", "empty list", "The list is still empty", Settings.GetNElectronTrackingDetectors(), 0U) && Passed;
  Settings.AddElectronTrackingDetector("Again");
  Passed = Evaluate("AddElectronTrackingDetector()", "after the removal", "The list can be filled again", Settings.GetElectronTrackingDetector(0), MString("Again")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters of the Compton sequence and decay options
bool UTSettingsEventReconstruction::TestGettersSettersCompton()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Settings;

  for (bool Flag : { true, false, true }) {
    Settings.SetAssumeD1First(Flag);
    Passed = Evaluate("SetAssumeD1First()", Flag, "The flag is stored", Settings.GetAssumeD1First(), Flag) && Passed;
    Settings.SetAssumeTrackTopBottom(!Flag);
    Passed = Evaluate("SetAssumeTrackTopBottom()", !Flag, "The flag is stored", Settings.GetAssumeTrackTopBottom(), !Flag) && Passed;
    Settings.SetUseComptelTypeEvents(Flag);
    Passed = Evaluate("SetUseComptelTypeEvents()", Flag, "The flag is stored", Settings.GetUseComptelTypeEvents(), Flag) && Passed;
    Settings.SetGuaranteeStartD1(!Flag);
    Passed = Evaluate("SetGuaranteeStartD1()", !Flag, "The flag is stored", Settings.GetGuaranteeStartD1(), !Flag) && Passed;
    Settings.SetRejectOneDetectorTypeOnlyEvents(Flag);
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", Flag, "The flag is stored", Settings.GetRejectOneDetectorTypeOnlyEvents(), Flag) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", Flag, "The assume D1 first flag is not changed", Settings.GetAssumeD1First(), Flag) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", Flag, "The track top to bottom flag is not changed", Settings.GetAssumeTrackTopBottom(), !Flag) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", Flag, "The COMPTEL type events flag is not changed", Settings.GetUseComptelTypeEvents(), Flag) && Passed;
    Passed = Evaluate("SetRejectOneDetectorTypeOnlyEvents()", Flag, "The start in D1 flag is not changed", Settings.GetGuaranteeStartD1(), !Flag) && Passed;
  }

  for (int Value : { 2, 0, -1, 2147483647 }) {
    Settings.SetClassicUndecidedHandling(Value);
    Passed = Evaluate("SetClassicUndecidedHandling()", Value, "The value is stored", Settings.GetClassicUndecidedHandling(), Value) && Passed;
    Settings.SetCSRMaxNHits(Value - 1);
    Passed = Evaluate("SetCSRMaxNHits()", Value - 1, "The value is stored", Settings.GetCSRMaxNHits(), Value - 1) && Passed;
    Passed = Evaluate("SetCSRMaxNHits()", Value - 1, "The classic undecided handling is not changed", Settings.GetClassicUndecidedHandling(), Value) && Passed;
  }

  for (double Value : { 0.25, 0.0, -1.0, 1E30 }) {
    Settings.SetCSRThresholdMin(Value);
    Passed = EvaluateNear("SetCSRThresholdMin()", Value, "The value is stored", Settings.GetCSRThresholdMin(), Value, 0.0) && Passed;
    Settings.SetCSRThresholdMax(2*Value);
    Passed = EvaluateNear("SetCSRThresholdMax()", 2*Value, "The value is stored", Settings.GetCSRThresholdMax(), 2*Value, 0.0) && Passed;
    Passed = EvaluateNear("SetCSRThresholdMax()", 2*Value, "The minimum is not changed", Settings.GetCSRThresholdMin(), Value, 0.0) && Passed;
  }

  // The five file names are independent
  Settings.SetBayesianComptonFileName("Compton.rsp");
  Settings.SetCSRTMVAFileName("CSR.tmva");
  Settings.SetOriginObjectsFileName("Origins.dat");
  Settings.SetDecayFileName("Decays.decay");
  Settings.SetBayesianElectronFileName("Electron.rsp");
  Passed = Evaluate("SetBayesianComptonFileName()", "Compton.rsp", "The name is stored", Settings.GetBayesianComptonFileName(), MString("Compton.rsp")) && Passed;
  Passed = Evaluate("SetCSRTMVAFileName()", "CSR.tmva", "The name is stored", Settings.GetCSRTMVAFileName(), MString("CSR.tmva")) && Passed;
  Passed = Evaluate("SetOriginObjectsFileName()", "Origins.dat", "The name is stored", Settings.GetOriginObjectsFileName(), MString("Origins.dat")) && Passed;
  Passed = Evaluate("SetDecayFileName()", "Decays.decay", "The name is stored", Settings.GetDecayFileName(), MString("Decays.decay")) && Passed;
  Passed = Evaluate("SetBayesianElectronFileName()", "Electron.rsp", "The name is stored", Settings.GetBayesianElectronFileName(), MString("Electron.rsp")) && Passed;
  Settings.SetBayesianComptonFileName("");
  Settings.SetCSRTMVAFileName("/with/path and space.tmva");
  Passed = Evaluate("SetBayesianComptonFileName()", "empty", "An empty name is stored", Settings.GetBayesianComptonFileName(), MString("")) && Passed;
  Passed = Evaluate("SetCSRTMVAFileName()", "/with/path and space.tmva", "A name with path and space is stored", Settings.GetCSRTMVAFileName(), MString("/with/path and space.tmva")) && Passed;
  Passed = Evaluate("SetCSRTMVAFileName()", "/with/path and space.tmva", "The decay file name is not changed", Settings.GetDecayFileName(), MString("Decays.decay")) && Passed;

  MERCSRTMVAMethods Methods;
  Methods.SetUsedMethods("DNN_CPU");
  Settings.SetCSRTMVAMethods(Methods);
  Passed = Evaluate("SetCSRTMVAMethods()", "DNN_CPU", "The method is stored", Settings.GetCSRTMVAMethods().GetUsedMethodsString(), MString("DNN_CPU")) && Passed;
  Passed = Evaluate("SetCSRTMVAMethods()", "DNN_CPU", "The methods of the event clustering are not changed", Settings.GetEventClusteringTMVAMethods().GetUsedMethodsString(), MString("BDTD")) && Passed;

  Settings.SetLensCenter(MVector(1.5, -2.5, 3.5));
  Passed = EvaluateVectorNear("SetLensCenter()", "(1.5, -2.5, 3.5)", "The lens center is stored", Settings.GetLensCenter(), MVector(1.5, -2.5, 3.5), 0.0) && Passed;
  Settings.SetFocalSpotCenter(MVector(-4.5, 5.5, -6.5));
  Passed = EvaluateVectorNear("SetFocalSpotCenter()", "(-4.5, 5.5, -6.5)", "The focal spot center is stored", Settings.GetFocalSpotCenter(), MVector(-4.5, 5.5, -6.5), 0.0) && Passed;
  Passed = EvaluateVectorNear("SetFocalSpotCenter()", "(-4.5, 5.5, -6.5)", "The lens center is not changed", Settings.GetLensCenter(), MVector(1.5, -2.5, 3.5), 0.0) && Passed;
  Settings.SetLensCenter(MVector(0.0, 0.0, 0.0));
  Passed = EvaluateVectorNear("SetLensCenter()", "(0, 0, 0)", "The zero vector is stored", Settings.GetLensCenter(), MVector(0.0, 0.0, 0.0), 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a write and read round trip of all values
bool UTSettingsEventReconstruction::TestRoundTrip()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Source;
  FillSettings(Source);
  MXmlDocument Tree("Test");
  Passed = EvaluateTrue("WriteXml()", "all values", "The values are written into the XML tree", Source.WriteXml(&Tree)) && Passed;

  // Directly from the tree
  UTSettingsEventReconstructionAccess Target;
  Passed = EvaluateTrue("ReadXml()", "tree of WriteXml()", "The XML tree is read", Target.ReadXml(&Tree)) && Passed;
  Passed = CheckFilled(Target, "tree") && Passed;

  // Through a file
  const MString FileName = GetTemporaryFileName("RoundTrip.xml");
  Passed = EvaluateTrue("Save()", FileName, "The XML tree is saved", Tree.Save(FileName)) && Passed;
  MXmlDocument Loaded;
  Passed = EvaluateTrue("Load()", FileName, "The XML tree is loaded", Loaded.Load(FileName)) && Passed;
  UTSettingsEventReconstructionAccess FromFile;
  Passed = EvaluateTrue("ReadXml()", "tree of the file", "The XML tree is read", FromFile.ReadXml(&Loaded)) && Passed;
  Passed = CheckFilled(FromFile, "file") && Passed;

  // The default values survive the round trip too
  UTSettingsEventReconstructionAccess DefaultSource;
  MXmlDocument DefaultTree("Test");
  DefaultSource.WriteXml(&DefaultTree);
  UTSettingsEventReconstructionAccess DefaultTarget;
  FillSettings(DefaultTarget);
  DefaultTarget.ReadXml(&DefaultTree);
  Passed = CheckDefaults(DefaultTarget, "defaults read over other values") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the round trip of the edge cases
bool UTSettingsEventReconstruction::TestRoundTripEdgeCases()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Source;
  Source.SetNJobs(4294967295U);
  Source.SetCoincidenceAlgorithm(-2147483647);
  Source.SetCoincidenceWindow(1.23456789012345E-20);
  Source.SetEventClusteringDistanceCutOff(-9876.54321098765);
  Source.SetAdjacentSigma(1E30);
  Source.SetEventIdMin(-9223372036854775807L);
  Source.SetEventIdMax(9223372036854775807L);
  Source.SetCSRThresholdMin(-0.5);
  Source.SetCSRThresholdMax(-0.25);
  Source.SetDecayFileName("Name with spaces & <special> characters.decay");
  MERCSRTMVAMethods None;
  Source.SetCSRTMVAMethods(None);
  MERCSRTMVAMethods All;
  All.SetUsedMethods("MLP,BDTD,PDEFOAM,PDEFOAMBOOST,DNN_CPU,DNN_GPU");
  Source.SetEventClusteringTMVAMethods(All);

  MXmlDocument Tree("Test");
  Source.WriteXml(&Tree);
  const MString FileName = GetTemporaryFileName("Edge.xml");
  Passed = EvaluateTrue("Save()", FileName, "The XML tree is saved", Tree.Save(FileName)) && Passed;
  MXmlDocument Loaded;
  Passed = EvaluateTrue("Load()", FileName, "The XML tree is loaded", Loaded.Load(FileName)) && Passed;

  UTSettingsEventReconstructionAccess Target;
  Target.AddElectronTrackingDetector("To be removed");
  DisableDefaultStreams();
  Target.ReadXml(&Loaded);
  EnableDefaultStreams();
  Passed = Evaluate("ReadXml()", "largest number of jobs", "The value is kept", Target.GetNJobs(), 4294967295U) && Passed;
  Passed = Evaluate("ReadXml()", "negative algorithm", "The value is kept", Target.GetCoincidenceAlgorithm(), -2147483647) && Passed;
  Passed = EvaluateNear("ReadXml()", "tiny coincidence window", "The value has 15 digits", Target.GetCoincidenceWindow(), 1.23456789012345E-20, 1E-34) && Passed;
  Passed = EvaluateNear("ReadXml()", "negative distance cut-off", "The value has 15 digits", Target.GetEventClusteringDistanceCutOff(), -9876.54321098765, 1E-10) && Passed;
  Passed = EvaluateNear("ReadXml()", "huge adjacent sigma", "The value is kept", Target.GetAdjacentSigma(), 1E30, 1E16) && Passed;
  Passed = Evaluate("ReadXml()", "smallest event ID", "The value is kept", Target.GetEventIdMin(), -9223372036854775807L) && Passed;
  Passed = Evaluate("ReadXml()", "largest event ID", "The value is kept", Target.GetEventIdMax(), 9223372036854775807L) && Passed;
  Passed = EvaluateNear("ReadXml()", "negative minimum threshold", "The value is kept", Target.GetCSRThresholdMin(), -0.5, 0.0) && Passed;
  Passed = EvaluateNear("ReadXml()", "negative maximum threshold", "The value is kept", Target.GetCSRThresholdMax(), -0.25, 0.0) && Passed;
  Passed = Evaluate("ReadXml()", "name with special characters", "The name is identical", Target.GetDecayFileName(), MString("Name with spaces & <special> characters.decay")) && Passed;
  Passed = Evaluate("ReadXml()", "empty file name", "An empty name stays empty", Target.GetBayesianComptonFileName(), MString("")) && Passed;
  Passed = Evaluate("ReadXml()", "empty list of electron tracking detectors", "The empty list replaces the detectors", Target.GetNElectronTrackingDetectors(), 0U) && Passed;
  Passed = Evaluate("ReadXml()", "no TMVA methods", "No method is used, also not the default one", Target.GetCSRTMVAMethods().GetNumberOfUsedMethods(), 0U) && Passed;
  // The methods PDEFoam and PDEFoamBoost are written in mixed case, and the reading does not depend on the case
  Passed = Evaluate("ReadXml()", "all TMVA methods", "All six methods are used", Target.GetEventClusteringTMVAMethods().GetNumberOfUsedMethods(), 6U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the values which are not part of the file
bool UTSettingsEventReconstruction::TestNotStored()
{
  bool Passed = true;

  // Saving the origin information and the special mode come from the command line only: they are not written and not read
  UTSettingsEventReconstructionAccess Source;
  Source.SetSaveOI(true);
  Source.SetSpecialMode(true);
  MXmlDocument Tree("Test");
  Source.WriteXml(&Tree);
  Passed = EvaluateTrue("WriteXml()", "save origin information", "The tree has no node SaveOI", Tree.GetNode("SaveOI") == nullptr) && Passed;
  Passed = EvaluateTrue("WriteXml()", "special mode", "The tree has no node SpecialMode", Tree.GetNode("SpecialMode") == nullptr) && Passed;

  UTSettingsEventReconstructionAccess Target;
  Target.ReadXml(&Tree);
  Passed = EvaluateFalse("ReadXml()", "save origin information", "The flag keeps its value", Target.GetSaveOI()) && Passed;
  Passed = EvaluateFalse("ReadXml()", "special mode", "The flag keeps its value", Target.GetSpecialMode()) && Passed;

  // Also a node in the tree is not read
  MXmlDocument WithNodes("Test");
  new MXmlNode(&WithNodes, "SaveOI", true);
  new MXmlNode(&WithNodes, "SpecialMode", true);
  Target.ReadXml(&WithNodes);
  Passed = EvaluateFalse("ReadXml()", "node SaveOI", "The node is not read", Target.GetSaveOI()) && Passed;
  Passed = EvaluateFalse("ReadXml()", "node SpecialMode", "The node is not read", Target.GetSpecialMode()) && Passed;

  // The names of all nodes which are written: 47 values, with the minimum and maximum of a range in one node
  vector<MString> Names = { "NJobs", "CoincidenceAlgorithm", "EventClusteringAlgorithm", "HitClusteringAlgorithm", "TrackingAlgorithm", "CSRAlgorithm", "DecayAlgorithm",
                            "CoincidenceWindow", "EventClusteringDistanceCutOff", "EventClusteringTMVAMethods", "EventClusteringTMVAFile",
                            "StandardClusterizerMinDistanceD1", "StandardClusterizerMinDistanceD2", "StandardClusterizerMinDistanceD3", "StandardClusterizerMinDistanceD4",
                            "StandardClusterizerMinDistanceD5", "StandardClusterizerMinDistanceD6", "StandardClusterizerMinDistanceD7", "StandardClusterizerMinDistanceD8",
                            "StandardClusterizerCenterIsReference", "AdjacentSigma", "AdjacentLevel", "PDFClusterizerBaseFileName",
                            "DoTracking", "SearchPairs", "SearchMIPs", "SearchComptons", "MaxComptonJump", "NTrackSequencesToKeep", "RejectPurelyAmbiguousTrackSequences",
                            "NLayersForVertexSearch", "ElectronTrackingDetectors", "AssumeD1First", "ClassicUndecidedHandling", "AssumeTrackTopBottom", "UseComptelTypeEvents",
                            "GuaranteeStartD1", "RejectOneDetectorTypeOnlyEvents", "CSRThreshold", "CSRMaxNHits", "LensCenter", "FocalSpotCenter", "OriginObjectsFile", "DecayFile",
                            "BayesianComptonFile", "BayesianElectronFile", "CSRTMVAFile", "CSRTMVAMethods", "TotalEnergy", "LeverArm", "EventId", "RejectAllBadEvents" };
  for (unsigned int n = 0; n < Names.size(); ++n) {
    Passed = EvaluateTrue("WriteXml()", Names[n], "The node is written", Tree.GetNode(Names[n]) != nullptr) && Passed;
  }
  Passed = Evaluate("WriteXml()", "all values", "No other nodes are written", Tree.GetNNodes(), static_cast<unsigned int>(Names.size())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the paths in the XML tree
bool UTSettingsEventReconstruction::TestPathCleaning()
{
  bool Passed = true;

  // The MEGAlib path goes as variable into the tree
  MString Expanded = "$(MEGALIB)/some/file";
  MFile::ExpandFileName(Expanded);

  UTSettingsEventReconstructionAccess Source;
  Source.SetEventClusteringTMVAFileName(Expanded + "1");
  Source.SetOriginObjectsFileName(Expanded + "2");
  Source.SetDecayFileName(Expanded + "3");
  Source.SetBayesianComptonFileName(Expanded + "4");
  Source.SetBayesianElectronFileName(Expanded + "5");
  Source.SetCSRTMVAFileName(Expanded + "6");
  MXmlDocument Tree("Test");
  Source.WriteXml(&Tree);

  vector<MString> Nodes = { "EventClusteringTMVAFile", "OriginObjectsFile", "DecayFile", "BayesianComptonFile", "BayesianElectronFile", "CSRTMVAFile" };
  for (unsigned int n = 0; n < Nodes.size(); ++n) {
    MXmlNode* Node = Tree.GetNode(Nodes[n]);
    Passed = EvaluateTrue("WriteXml()", Nodes[n], "The node is written", Node != nullptr) && Passed;
    if (Node != nullptr) {
      Passed = Evaluate("WriteXml()", Nodes[n], "The path has the variable", Node->GetValueAsString(), MString("$(MEGALIB)/some/file") + (n+1)) && Passed;
    }
  }

  // Other paths are not touched
  UTSettingsEventReconstructionAccess Other;
  Other.SetDecayFileName("/not/megalib/file");
  MXmlDocument OtherTree("Test");
  Other.WriteXml(&OtherTree);
  MXmlNode* Node = OtherTree.GetNode("DecayFile");
  Passed = EvaluateTrue("WriteXml()", "/not/megalib/file", "The node is written", Node != nullptr) && Passed;
  if (Node != nullptr) {
    Passed = Evaluate("WriteXml()", "/not/megalib/file", "The path is not changed", Node->GetValueAsString(), MString("/not/megalib/file")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading incomplete trees and the old keyword of the hit clustering algorithm
bool UTSettingsEventReconstruction::TestReadIncomplete()
{
  bool Passed = true;

  // An empty tree changes nothing
  {
    MXmlDocument Tree("Test");
    UTSettingsEventReconstructionAccess Settings;
    FillSettings(Settings);
    Passed = EvaluateTrue("ReadXml()", "empty tree", "The empty tree is read", Settings.ReadXml(&Tree)) && Passed;
    Passed = CheckFilled(Settings, "empty tree") && Passed;
  }

  // An incomplete tree only changes what it contains
  {
    MXmlDocument Tree("Test");
    new MXmlNode(&Tree, "CSRAlgorithm", 3);
    new MXmlNode(&Tree, "TotalEnergy", 5.5, 6.5);
    new MXmlNode(&Tree, "UnknownNode", 1);
    UTSettingsEventReconstructionAccess Settings;
    Passed = EvaluateTrue("ReadXml()", "incomplete tree", "The tree is read", Settings.ReadXml(&Tree)) && Passed;
    Passed = Evaluate("ReadXml()", "incomplete tree", "The value in the tree is used", Settings.GetCSRAlgorithm(), 3) && Passed;
    Passed = EvaluateNear("ReadXml()", "incomplete tree", "The minimum in the tree is used", Settings.GetTotalEnergyMin(), 5.5, 0.0) && Passed;
    Passed = EvaluateNear("ReadXml()", "incomplete tree", "The maximum in the tree is used", Settings.GetTotalEnergyMax(), 6.5, 0.0) && Passed;
    Passed = Evaluate("ReadXml()", "incomplete tree", "The value not in the tree stays", Settings.GetTrackingAlgorithm(), 0) && Passed;
    Passed = EvaluateNear("ReadXml()", "incomplete tree", "The value not in the tree stays", Settings.GetLeverArmMax(), 1000.0, 0.0) && Passed;
    Passed = EvaluateTrue("ReadXml()", "incomplete tree", "The value not in the tree stays", Settings.GetDoTracking()) && Passed;
  }

  // The hit clustering algorithm had the keyword ClusteringAlgorithm in old files
  {
    MXmlDocument Tree("Test");
    new MXmlNode(&Tree, "ClusteringAlgorithm", 4);
    UTSettingsEventReconstructionAccess Settings;
    Settings.ReadXml(&Tree);
    Passed = Evaluate("ReadXml()", "old keyword ClusteringAlgorithm", "The hit clustering algorithm is set", Settings.GetHitClusteringAlgorithm(), 4) && Passed;
    Passed = Evaluate("ReadXml()", "old keyword ClusteringAlgorithm", "The event clustering algorithm is not changed", Settings.GetEventClusteringAlgorithm(), 0) && Passed;
  }

  // The electron tracking detectors without the list node are not changed
  {
    MXmlDocument Tree("Test");
    new MXmlNode(&Tree, "CSRAlgorithm", 3);
    UTSettingsEventReconstructionAccess Settings;
    Settings.AddElectronTrackingDetector("Stays");
    Settings.ReadXml(&Tree);
    Passed = Evaluate("ReadXml()", "no node ElectronTrackingDetectors", "The detectors stay", Settings.GetNElectronTrackingDetectors(), 1U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test repeated reads with one object
bool UTSettingsEventReconstruction::TestReuse()
{
  bool Passed = true;

  UTSettingsEventReconstructionAccess Filled;
  FillSettings(Filled);
  MXmlDocument FilledTree("Test");
  Filled.WriteXml(&FilledTree);

  UTSettingsEventReconstructionAccess Default;
  MXmlDocument DefaultTree("Test");
  Default.WriteXml(&DefaultTree);

  // One object reads the two trees several times: nothing accumulates
  UTSettingsEventReconstructionAccess Settings;
  Settings.ReadXml(&FilledTree);
  Settings.ReadXml(&FilledTree);
  Passed = CheckFilled(Settings, "filled tree twice") && Passed;
  Settings.ReadXml(&DefaultTree);
  Passed = CheckDefaults(Settings, "default tree after the filled tree") && Passed;
  Settings.ReadXml(&FilledTree);
  Passed = CheckFilled(Settings, "filled tree after the default tree") && Passed;

  // Writing twice into one tree
  MXmlDocument Twice("Test");
  Filled.WriteXml(&Twice);
  unsigned int NNodes = Twice.GetNNodes();
  Passed = Evaluate("WriteXml()", "one write", "The tree has as many nodes as the reference tree", NNodes, FilledTree.GetNNodes()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsEventReconstruction Test;
  return Test.Run() == true ? 0 : 1;
}


// UTSettingsEventReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
