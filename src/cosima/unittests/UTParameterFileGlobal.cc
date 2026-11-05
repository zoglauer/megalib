/*
 * UTParameterFileGlobal.cc
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


// Cosima:
#include "MCParameterFile.hh"
#include "MCPhysicsList.hh"
#include "UTParameterFileShared.h"

// MEGAlib:
#include "MSimEvent.h"

// Standard lib:
#include <cmath>
#include <limits>
using namespace std;


//! Unit test class for the global keywords of MCParameterFile
class UTParameterFileGlobal : public UTParameterFileTest
{
public:
  //! Default constructor
  UTParameterFileGlobal() : UTParameterFileTest("UTParameterFileGlobal") {}
  //! Default destructor
  virtual ~UTParameterFileGlobal() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the values of a default constructed parameter file
  bool TestDefaults();
  //! Test the setters and getters
  bool TestSettersGetters();
  //! Test the smallest valid file and the run access
  bool TestMinimalFile();
  //! Test the geometry keyword
  bool TestGeometry();
  //! Test the detector time constant
  bool TestDetectorTimeConstant();
  //! Test the overlap check keyword
  bool TestCheckForOverlaps();
  //! Test the cross section file keyword
  bool TestCreateCrossSectionFiles();
  //! Test the default range cut
  bool TestDefaultRangeCut();
  //! Test the regions
  bool TestRegions();
  //! Test the electromagnetic physics lists
  bool TestPhysicsListEM();
  //! Test the hadronic physics lists
  bool TestPhysicsListHD();
  //! Test the decay modes
  bool TestDecayMode();
  //! Test the keywords defining what is stored
  bool TestStoreKeywords();
  //! Test the keywords defining the storage format
  bool TestStoreFormat();
  //! Test the pre-trigger keywords
  bool TestPreTrigger();
  //! Test the lists of watched volumes and black absorbers
  bool TestVolumeLists();
  //! Test the start area
  bool TestStartArea();
  //! Test the include mechanism
  bool TestIncludes();
  //! Test the consistency checks after parsing
  bool TestConsistency();
};


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::Run()
{
  bool Passed = true;

  if (PrepareFiles() == false) {
    Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", false) && Passed;
    Summarize();
    return Passed;
  }

  Passed = TestDefaults() && Passed;
  Passed = TestSettersGetters() && Passed;
  Passed = TestMinimalFile() && Passed;
  Passed = TestGeometry() && Passed;
  Passed = TestDetectorTimeConstant() && Passed;
  Passed = TestCheckForOverlaps() && Passed;
  Passed = TestCreateCrossSectionFiles() && Passed;
  Passed = TestDefaultRangeCut() && Passed;
  Passed = TestRegions() && Passed;
  Passed = TestPhysicsListEM() && Passed;
  Passed = TestPhysicsListHD() && Passed;
  Passed = TestDecayMode() && Passed;
  Passed = TestStoreKeywords() && Passed;
  Passed = TestStoreFormat() && Passed;
  Passed = TestPreTrigger() && Passed;
  Passed = TestVolumeLists() && Passed;
  Passed = TestStartArea() && Passed;
  Passed = TestIncludes() && Passed;
  Passed = TestConsistency() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestDefaults()
{
  bool Passed = true;

  MCParameterFile P;
  Passed = Evaluate("MCParameterFile()", "default", "A new parameter file has no runs", P.GetNRuns(), 0U) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "A new parameter file has no activators", P.GetNActivators(), 0U) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The geometry file name is empty", P.GetGeometryFileName(), string("")) && Passed;
  Passed = EvaluateNear("MCParameterFile()", "default", "The default range cut is 0.005 mm", P.GetDefaultRangeCut()/mm, 0.005, 1e-12) && Passed;
  Passed = EvaluateNear("MCParameterFile()", "default", "The default detector time constant is 1 ns", P.GetDetectorTimeConstant()/ns, 1.0, 1e-12) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default EM physics list is Livermore", P.GetPhysicsListEM(), MCPhysicsList::c_EMLivermore) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default hadron physics list is none", P.GetPhysicsListHD(), MCPhysicsList::c_HDNone) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default decay mode is ignore", P.GetDecayMode(), MCParameterFile::c_DecayModeIgnore) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default pre-trigger mode is full", P.GetPreTriggerMode(), MCParameterFile::c_PreTriggerFull) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default the data is not stored in binary format", P.StoreBinary()) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "By default all simulation info is stored", P.StoreSimulationInfo(), MSimEvent::c_StoreSimulationInfoAll) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default simulation info version is the current output version", P.StoreSimulationInfoVersion(), MSimEvent::GetOutputVersion()) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default the ionization info is not stored", P.StoreSimulationInfoIonization()) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default the text output is not scientific", P.StoreScientific()) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default precision is 5", P.StoreScientificPrecision(), 5) && Passed;
  Passed = EvaluateTrue("MCParameterFile()", "default", "By default calibrated hits are stored", P.StoreCalibrated()) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default the hits are not stored one per event", P.StoreOneHitPerEvent()) && Passed;
  Passed = EvaluateTrue("MCParameterFile()", "default", "The default minimum energy is -1e40 keV", P.StoreMinimumEnergy() == -1E+40*keV) && Passed;
  Passed = EvaluateTrue("MCParameterFile()", "default", "The default maximum energy loss is 1e40 keV", P.StoreMaximumEnergyLoss() == 1E+40*keV) && Passed;
  Passed = EvaluateTrue("MCParameterFile()", "default", "By default the hits are discretized", P.DiscretizeHits()) && Passed;
  Passed = EvaluateSize("MCParameterFile()", "default", "There are no watched volumes", P.GetStoreSimulationInfoWatchedVolumes().size(), 0) && Passed;
  Passed = EvaluateSize("MCParameterFile()", "default", "There are no black absorbers", P.GetBlackAbsorbers().size(), 0) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default there is no overlap check", P.PerformOverlapCheck()) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default overlap check resolution is 1000", P.ResolutionOverlapCheck(), 1000) && Passed;
  Passed = EvaluateNear("MCParameterFile()", "default", "The default overlap check tolerance is 1 um", P.ToleranceOverlapCheck()/um, 1.0, 1e-12) && Passed;
  Passed = EvaluateFalse("MCParameterFile()", "default", "By default no cross section files are created", P.CreateCrossSectionFiles()) && Passed;
  Passed = Evaluate("MCParameterFile()", "default", "The default cross section directory is empty", P.GetCrossSectionFileDirectory(), MString("")) && Passed;
  Passed = EvaluateSize("MCParameterFile()", "default", "The region list is empty", P.GetRegionList().size(), 0) && Passed;
  Passed = EvaluateSize("MCParameterFile()", "default", "The run list is empty", P.GetRunList().size(), 0) && Passed;

  // The mode constants are all different:
  vector<int> Modes = { MCParameterFile::c_DecayModeNormal, MCParameterFile::c_DecayModeIgnore, MCParameterFile::c_DecayModeBuildUp,
                        MCParameterFile::c_DecayModeActivationBuildUp, MCParameterFile::c_DecayModeActivationDelayedDecay,
                        MCParameterFile::c_DecayModeBuildUpAndSaveActivation };
  bool Different = true;
  for (unsigned int a = 0; a < Modes.size(); ++a) {
    for (unsigned int b = a+1; b < Modes.size(); ++b) {
      if (Modes[a] == Modes[b]) Different = false;
    }
  }
  Passed = EvaluateTrue("c_DecayMode", "all", "All decay mode constants are different", Different) && Passed;
  Passed = EvaluateTrue("c_PreTrigger", "all", "All pre-trigger constants are different", MCParameterFile::c_PreTriggerEverything != MCParameterFile::c_PreTriggerEveryEventWithHits &&
                        MCParameterFile::c_PreTriggerEveryEventWithHits != MCParameterFile::c_PreTriggerFull && MCParameterFile::c_PreTriggerEverything != MCParameterFile::c_PreTriggerFull) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestSettersGetters()
{
  bool Passed = true;

  MCParameterFile P;
  P.SetDefaultRangeCut(0.25*mm);
  Passed = EvaluateNear("SetDefaultRangeCut/GetDefaultRangeCut", "0.25 mm", "The range cut is stored", P.GetDefaultRangeCut()/mm, 0.25, 1e-12) && Passed;
  P.SetDetectorTimeConstant(3.5*us);
  Passed = EvaluateNear("SetDetectorTimeConstant/GetDetectorTimeConstant", "3.5 us", "The time constant is stored", P.GetDetectorTimeConstant()/us, 3.5, 1e-12) && Passed;
  P.SetPhysicsListEM(MCPhysicsList::c_EMPenelope);
  Passed = Evaluate("SetPhysicsListEM/GetPhysicsListEM", "Penelope", "The EM physics list is stored", P.GetPhysicsListEM(), MCPhysicsList::c_EMPenelope) && Passed;
  P.SetPhysicsListHD(MCPhysicsList::c_HDFTFP_BERT_HP);
  Passed = Evaluate("SetPhysicsListHD/GetPhysicsListHD", "FTFP_BERT_HP", "The hadron physics list is stored", P.GetPhysicsListHD(), MCPhysicsList::c_HDFTFP_BERT_HP) && Passed;
  P.SetDecayMode(MCParameterFile::c_DecayModeBuildUp);
  Passed = Evaluate("SetDecayMode/GetDecayMode", "BuildUp", "The decay mode is stored", P.GetDecayMode(), MCParameterFile::c_DecayModeBuildUp) && Passed;
  P.SetPreTriggerMode(MCParameterFile::c_PreTriggerEverything);
  Passed = Evaluate("SetPreTriggerMode/GetPreTriggerMode", "everything", "The pre-trigger mode is stored", P.GetPreTriggerMode(), MCParameterFile::c_PreTriggerEverything) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestMinimalFile()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "minimal", "The smallest valid file is accepted", Open(P, Global(""))) && Passed;
  Passed = Evaluate("GetNRuns()", "minimal", "The file defines one run", P.GetNRuns(), 1U) && Passed;
  Passed = EvaluateSize("GetRunList()", "minimal", "The run list contains the run", P.GetRunList().size(), 1) && Passed;
  Passed = Evaluate("GetRunList()", "minimal", "The run has the name given in the file", P.GetRunList()[0].GetName(), MString("R")) && Passed;
  Passed = EvaluateTrue("GetRun()", "R", "The run can be found by its name", P.GetRun("R") != nullptr) && Passed;
  Passed = EvaluateTrue("GetRun()", "unknown", "An unknown run is not found", P.GetRun("Unknown") == nullptr) && Passed;
  Passed = EvaluateTrue("GetSource()", "S", "The source can be found by its name", P.GetSource("S") != nullptr) && Passed;
  Passed = EvaluateTrue("GetSource()", "unknown", "An unknown source is not found", P.GetSource("Unknown") == nullptr) && Passed;
  Passed = EvaluateTrue("GetRegion()", "unknown", "There is no region", P.GetRegion("Unknown") == nullptr) && Passed;
  Passed = EvaluateTrue("GetActivator()", "unknown", "There is no activator", P.GetActivator("Unknown") == nullptr) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "minimal", "The geometry file name is the one in the file", P.GetGeometryFileName(), string(GetGeometryFileName().Data())) && Passed;
  Passed = Evaluate("GetCurrentRun()", "minimal", "The current run is the first run", P.GetCurrentRun().GetName(), MString("R")) && Passed;
  Passed = EvaluateNear("Open()", "minimal", "A file without settings keeps the default range cut", P.GetDefaultRangeCut()/mm, 0.005, 1e-12) && Passed;
  Passed = Evaluate("GetNSources()", "minimal", "The run contains the source", P.GetRun("R")->GetNSources(), 1U) && Passed;

  // Two runs and the run iteration:
  UTParameterFileAccess Two;
  Passed = EvaluateTrue("Open()", "two runs", "A file with two runs is accepted", Open(Two, Header() + "Run First\nFirst.Source S1\nRun Second\nSecond.Source S2\n")) && Passed;
  Passed = Evaluate("GetNRuns()", "two runs", "The file defines two runs", Two.GetNRuns(), 2U) && Passed;
  Passed = Evaluate("GetCurrentRun()", "two runs", "The current run is the first one", Two.GetCurrentRun().GetName(), MString("First")) && Passed;
  Two.NextRun();
  Passed = Evaluate("NextRun()", "two runs", "After NextRun the current run is the second one", Two.GetCurrentRun().GetName(), MString("Second")) && Passed;
  Passed = Evaluate("GetRun()", "two runs", "The sources belong to their runs", Two.GetRun("Second")->GetSource(0)->GetName(), MString("S2")) && Passed;

  // Errors of the run and source definitions:
  Passed = ExpectRejected("Open()", "Run without name", "A run without a name is rejected", Header() + "Run\n") && Passed;
  Passed = ExpectRejected("Open()", "Run with two names", "A run with two names is rejected", Header() + "Run A B\n") && Passed;
  Passed = ExpectRejected("Open()", "Source without name", "A source without a name is rejected", Header() + "Run R\nR.Source\n") && Passed;
  Passed = ExpectRejected("Open()", "same source in two runs", "A source name can only be used once", Header() + "Run A\nA.Source S\nRun B\nB.Source S\n") && Passed;

  // Missing file:
  MCParameterFile Missing;
  SilenceOutput();
  bool Opened = Missing.Open(GetDirectory() + "/DoesNotExist.source");
  RestoreOutput();
  Passed = EvaluateFalse("Open()", "missing file", "A missing parameter file is rejected", Opened) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestGeometry()
{
  bool Passed = true;

  Passed = ExpectRejected("Open()", "missing geometry", "A geometry file which does not exist is rejected", MString("Version 1\nGeometry ") + GetDirectory() + "/Missing.geo.setup\nRun R\nR.Source S\n") && Passed;
  Passed = ExpectRejected("Open()", "no geometry keyword", "A file without geometry is rejected", "Version 1\nRun R\nR.Source S\n") && Passed;
  Passed = ExpectRejected("Open()", "Geometry without file", "The geometry keyword needs a file name", "Version 1\nGeometry\nRun R\nR.Source S\n") && Passed;

  // Environment variables in the file name:
  const MString Directory = GetExampleDirectory();
  if (Directory.IsEmpty() == false) {
    UTParameterFileAccess P;
    const MString Geometry = "$(MEGALIB)/resource/examples/geomega/special/Spheric.geo.setup";
    Passed = EvaluateTrue("Open()", "MEGALIB variable", "A geometry file name with an environment variable is accepted", Open(P, MString("Version 1\nGeometry ") + Geometry + "\nRun R\nR.Source S\n")) && Passed;
    Passed = EvaluateTrue("GetGeometryFileName()", "MEGALIB variable", "The environment variable is expanded", MString(P.GetGeometryFileName().c_str()).Contains("$(MEGALIB)") == false) && Passed;
    Passed = EvaluateTrue("GetGeometryFileName()", "MEGALIB variable", "The expanded name ends with the geometry file", MString(P.GetGeometryFileName().c_str()).EndsWith("resource/examples/geomega/special/Spheric.geo.setup")) && Passed;
  }

  // The last geometry line wins:
  UTParameterFileAccess Last;
  const MString Second = WriteSource("Second.geo.setup", "Name Second\nVersion 1.0\n");
  Passed = EvaluateTrue("Open()", "two geometry lines", "A file with two geometry lines is accepted", Open(Last, MString("Version 1\nGeometry ") + GetGeometryFileName() + "\nGeometry " + Second + "\nRun R\nR.Source S\n")) && Passed;
  // The parser expands the relative name to the absolute one
  Passed = Evaluate("GetGeometryFileName()", "two geometry lines", "The last geometry line is used", Last.GetGeometryFileName(), string((GetDirectory() + "/Second.geo.setup").Data())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestDetectorTimeConstant()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "DetectorTimeConstant", "A detector time constant is accepted", Open(P, Global("DetectorTimeConstant 0.000005\n"))) && Passed;
  Passed = EvaluateNear("DetectorTimeConstant", "0.000005", "The value is given in seconds", P.GetDetectorTimeConstant()/us, 5.0, 1e-9) && Passed;

  UTParameterFileAccess Large;
  Passed = EvaluateTrue("Open()", "DetectorTimeConstant 2", "A large detector time constant is accepted", Open(Large, Global("DetectorTimeConstant 2\n"))) && Passed;
  Passed = EvaluateNear("DetectorTimeConstant", "2", "Two seconds are stored as two seconds", Large.GetDetectorTimeConstant()/s, 2.0, 1e-9) && Passed;

  Passed = ExpectRejected("Open()", "DetectorTimeConstant without value", "The keyword needs a value", Global("DetectorTimeConstant\n")) && Passed;
  Passed = ExpectRejected("Open()", "DetectorTimeConstant with two values", "The keyword takes exactly one value", Global("DetectorTimeConstant 1 2\n")) && Passed;

  // The detector time constant is given to the activators as half life cut off:
  UTParameterFileAccess A;
  Passed = EvaluateTrue("Open()", "activator", "A file with an activator and a time constant is accepted", Open(A, Header() + "DetectorTimeConstant 0.001\nActivator Act\nAct.ActivationMode ConstantIrradiation 100\n")) && Passed;
  Passed = EvaluateTrue("GetActivator()", "Act", "The activator exists", A.GetActivator("Act") != nullptr) && Passed;
  if (A.GetActivator("Act") != nullptr) {
    // The cut off is not readable, but it defines when a level is an immediate decay: Al26[1057.739] has 2.5e-14 s
    Passed = EvaluateNear("GetActivator()", "Act", "The activator has the given activation time", A.GetActivator("Act")->GetActivationTime()/s, 100.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestCheckForOverlaps()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "CheckForOverlaps", "The overlap check keyword is accepted", Open(P, Global("CheckForOverlaps 500 0.001\n"))) && Passed;
  Passed = EvaluateTrue("CheckForOverlaps", "500 0.001", "The overlap check is switched on", P.PerformOverlapCheck()) && Passed;
  Passed = Evaluate("CheckForOverlaps", "500 0.001", "The resolution is the first value", P.ResolutionOverlapCheck(), 500) && Passed;
  Passed = EvaluateNear("CheckForOverlaps", "500 0.001", "The tolerance is given in cm", P.ToleranceOverlapCheck()/um, 10.0, 1e-9) && Passed;

  UTParameterFileAccess None;
  Passed = EvaluateTrue("Open()", "without", "A file without the keyword is accepted", Open(None, Global(""))) && Passed;
  Passed = EvaluateFalse("CheckForOverlaps", "not given", "Without the keyword there is no overlap check", None.PerformOverlapCheck()) && Passed;

  Passed = ExpectRejected("Open()", "one value", "The keyword needs two values", Global("CheckForOverlaps 500\n")) && Passed;
  Passed = ExpectRejected("Open()", "three values", "The keyword takes two values", Global("CheckForOverlaps 500 0.001 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "no value", "The keyword needs values", Global("CheckForOverlaps\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestCreateCrossSectionFiles()
{
  bool Passed = true;

  // Cross section files do not need a run (the directory is relative to the working directory of the test: the format has no quoting)
  UTParameterFileAccess P;
  const MString Directory = GetRelativeDirectory() + "/CrossSections";
  Passed = EvaluateTrue("Open()", "CreateCrossSectionFiles", "A file only creating cross sections needs no run", Open(P, Header() + "CreateCrossSectionFiles " + Directory + "\n")) && Passed;
  Passed = EvaluateTrue("CreateCrossSectionFiles", "directory", "The cross section creation is switched on", P.CreateCrossSectionFiles()) && Passed;
  Passed = Evaluate("CreateCrossSectionFiles", "directory", "The directory is stored, expanded to the absolute path", P.GetCrossSectionFileDirectory(), GetDirectory() + "/CrossSections") && Passed;
  Passed = Evaluate("GetNRuns()", "CreateCrossSectionFiles", "The file has no runs", P.GetNRuns(), 0U) && Passed;

  // A run without sources is also fine in that mode:
  Passed = ExpectAccepted("Open()", "with run without source", "In cross section mode a run needs no source", Header() + "CreateCrossSectionFiles " + Directory + "\nRun R\n") && Passed;

  Passed = ExpectRejected("Open()", "without directory", "The keyword needs a directory", Header() + "CreateCrossSectionFiles\n") && Passed;
  Passed = ExpectRejected("Open()", "two directories", "The keyword takes one directory", Header() + "CreateCrossSectionFiles A B\n") && Passed;

  // Without the keyword a run is required:
  Passed = ExpectRejected("Open()", "no run", "Without cross section creation a file needs a run", Header()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestDefaultRangeCut()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "DefaultRangeCut", "A default range cut is accepted", Open(P, Global("DefaultRangeCut 0.1\n"))) && Passed;
  Passed = EvaluateNear("DefaultRangeCut", "0.1", "The value is given in cm", P.GetDefaultRangeCut()/mm, 1.0, 1e-9) && Passed;

  UTParameterFileAccess Small;
  Passed = EvaluateTrue("Open()", "DefaultRangeCut 0.0005", "A small default range cut is accepted", Open(Small, Global("DefaultRangeCut 0.0005\n"))) && Passed;
  Passed = EvaluateNear("DefaultRangeCut", "0.0005", "The small value is stored", Small.GetDefaultRangeCut()/um, 5.0, 1e-9) && Passed;

  Passed = ExpectRejected("Open()", "no value", "The keyword needs a value", Global("DefaultRangeCut\n")) && Passed;
  Passed = ExpectRejected("Open()", "two values", "The keyword takes one value", Global("DefaultRangeCut 0.1 0.2\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestRegions()
{
  bool Passed = true;

  UTParameterFileAccess P;
  const MString Content = Global("Region Shield\nRegion Crystal\nShield.Volume ShieldVolume\nShield.RangeCut 0.05\nShield.CutAllSecondaries true\n"
                                 "Crystal.Volume CrystalVolume\nCrystal.RangeCut 0.001\n");
  Passed = EvaluateTrue("Open()", "two regions", "Two complete regions are accepted", Open(P, Content)) && Passed;
  Passed = EvaluateSize("GetRegionList()", "two regions", "Both regions are in the list", P.GetRegionList().size(), 2) && Passed;
  const MCRegion* Shield = P.GetRegion("Shield");
  const MCRegion* Crystal = P.GetRegion("Crystal");
  Passed = EvaluateTrue("GetRegion()", "Shield", "The first region can be found", Shield != nullptr) && Passed;
  Passed = EvaluateTrue("GetRegion()", "Crystal", "The second region can be found", Crystal != nullptr) && Passed;
  if (Shield != nullptr) {
    Passed = Evaluate("Region.Volume", "Shield", "The volume of the first region is stored", Shield->GetVolumeName(), MString("ShieldVolume")) && Passed;
    Passed = EvaluateNear("Region.RangeCut", "0.05", "The range cut is given in cm", Shield->GetRangeCut()/mm, 0.5, 1e-9) && Passed;
    Passed = EvaluateTrue("Region.CutAllSecondaries", "true", "The flag to cut all secondaries is set", Shield->GetCutAllSecondaries()) && Passed;
  }
  if (Crystal != nullptr) {
    Passed = Evaluate("Region.Volume", "Crystal", "The volume of the second region is stored", Crystal->GetVolumeName(), MString("CrystalVolume")) && Passed;
    Passed = EvaluateNear("Region.RangeCut", "0.001", "The range cut of the second region is stored", Crystal->GetRangeCut()/um, 10.0, 1e-9) && Passed;
    Passed = EvaluateFalse("Region.CutAllSecondaries", "not given", "Cutting all secondaries is off by default", Crystal->GetCutAllSecondaries()) && Passed;
  }

  // Flag values:
  UTParameterFileAccess Off;
  Passed = EvaluateTrue("Open()", "CutAllSecondaries false", "A region with cut all secondaries false is accepted", Open(Off, Global("Region A\nA.Volume V\nA.RangeCut 1\nA.CutAllSecondaries false\n"))) && Passed;
  Passed = EvaluateFalse("Region.CutAllSecondaries", "false", "The flag is false", Off.GetRegion("A")->GetCutAllSecondaries()) && Passed;

  // Incomplete regions:
  Passed = ExpectRejected("Open()", "no volume", "A region without volume is rejected", Global("Region A\nA.RangeCut 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "no range cut", "A region without range cut is rejected", Global("Region A\nA.Volume V\n")) && Passed;
  Passed = ExpectRejected("Open()", "empty region", "A region without any property is rejected", Global("Region A\n")) && Passed;

  // Malformed lines:
  Passed = ExpectRejected("Open()", "Region without name", "A region needs a name", Global("Region\n")) && Passed;
  Passed = ExpectRejected("Open()", "Region with two names", "A region takes one name", Global("Region A B\n")) && Passed;
  Passed = ExpectRejected("Open()", "Volume without value", "The region volume needs a value", Global("Region A\nA.Volume\nA.RangeCut 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "Volume with two values", "The region volume takes one value", Global("Region A\nA.Volume V W\nA.RangeCut 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "RangeCut without value", "The region range cut needs a value", Global("Region A\nA.Volume V\nA.RangeCut\n")) && Passed;
  Passed = ExpectRejected("Open()", "RangeCut with two values", "The region range cut takes one value", Global("Region A\nA.Volume V\nA.RangeCut 1 2\n")) && Passed;
  Passed = ExpectRejected("Open()", "CutAllSecondaries without value", "The region flag needs a value", Global("Region A\nA.Volume V\nA.RangeCut 1\nA.CutAllSecondaries\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestPhysicsListEM()
{
  bool Passed = true;

  struct Entry { MString Keyword; int Expected; };
  vector<Entry> Entries = {
    { "None", MCPhysicsList::c_EMNone },
    { "Standard", MCPhysicsList::c_EMStandard },
    { "Livermore", MCPhysicsList::c_EMLivermore },
    { "livermore-pol", MCPhysicsList::c_EMLivermorePolarized },
    { "LivermorePol", MCPhysicsList::c_EMLivermorePolarized },
    { "LIVERMORE-POL", MCPhysicsList::c_EMLivermorePolarized },
    { "livermore-g4lecs", MCPhysicsList::c_EMLivermore },
    { "Penelope", MCPhysicsList::c_EMPenelope }
  };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, Global(MString("PhysicsListEM ") + E.Keyword + "\n"));
    Passed = EvaluateTrue("Open()", MString("PhysicsListEM ") + E.Keyword, "The physics list is accepted", Opened) && Passed;
    Passed = Evaluate("PhysicsListEM", E.Keyword, "The EM physics list is set", P.GetPhysicsListEM(), E.Expected) && Passed;
  }

  Passed = ExpectRejected("Open()", "unknown list", "An unknown EM physics list is rejected", Global("PhysicsListEM Unknown\n")) && Passed;
  Passed = ExpectRejected("Open()", "no list", "The EM physics list keyword needs a value", Global("PhysicsListEM\n")) && Passed;
  Passed = ExpectRejected("Open()", "two lists", "The EM physics list keyword takes one value", Global("PhysicsListEM Livermore Standard\n")) && Passed;

  // Deprecated numeric package:
  for (int Package = MCPhysicsList::c_EMMin; Package <= MCPhysicsList::c_EMMax; ++Package) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", MString("Package ") + Package, "A valid package ID is accepted", Open(P, Global(MString("Package ") + Package + "\n"))) && Passed;
    Passed = Evaluate("Package", Package, "The package ID is the EM physics list", P.GetPhysicsListEM(), Package) && Passed;
  }
  Passed = ExpectRejected("Open()", "Package 6", "A package ID above the maximum is rejected", Global(MString("Package ") + (MCPhysicsList::c_EMMax + 1) + "\n")) && Passed;
  Passed = ExpectRejected("Open()", "Package -1", "A negative package ID is rejected", Global("Package -1\n")) && Passed;
  Passed = ExpectRejected("Open()", "Package without value", "The package keyword needs a value", Global("Package\n")) && Passed;

  // Deprecated fluorescence keywords are ignored, but accepted:
  Passed = ExpectAccepted("Open()", "ActivateFluorescence", "The deprecated fluorescence keyword is accepted", Global("ActivateFluorescence true\n")) && Passed;
  Passed = ExpectAccepted("Open()", "PhysicsListEMActivateFluorescence", "The deprecated fluorescence keyword with the list name is accepted", Global("PhysicsListEMActivateFluorescence true\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestPhysicsListHD()
{
  bool Passed = true;

  struct Entry { MString Keyword; int Expected; };
  vector<Entry> Entries = {
    { "None", MCPhysicsList::c_HDNone },
    { "qgsp-bic-hp", MCPhysicsList::c_HDQGSP_BIC_HP },
    { "QGSP-BERT-HP", MCPhysicsList::c_HDQGSP_BERT_HP },
    { "ftfp-bert-hp", MCPhysicsList::c_HDFTFP_BERT_HP },
    { "ftfp-inclxx-hp", MCPhysicsList::c_HDFTFP_INCLXX_HP },
    { "qgsp-inclxx-hp", MCPhysicsList::c_HDQGSP_INCLXX_HP }
  };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, Global(MString("PhysicsListHD ") + E.Keyword + "\n"));
    Passed = EvaluateTrue("Open()", MString("PhysicsListHD ") + E.Keyword, "The physics list is accepted", Opened) && Passed;
    Passed = Evaluate("PhysicsListHD", E.Keyword, "The hadron physics list is set", P.GetPhysicsListHD(), E.Expected) && Passed;
  }

  Passed = ExpectRejected("Open()", "unknown list", "An unknown hadron physics list is rejected", Global("PhysicsListHD Unknown\n")) && Passed;
  Passed = ExpectRejected("Open()", "no list", "The hadron physics list keyword needs a value", Global("PhysicsListHD\n")) && Passed;
  Passed = ExpectRejected("Open()", "two lists", "The hadron physics list keyword takes one value", Global("PhysicsListHD none none\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestDecayMode()
{
  bool Passed = true;

  struct Entry { MString Keyword; int Expected; };
  vector<Entry> Entries = {
    { "Normal", MCParameterFile::c_DecayModeNormal },
    { "BuildUp", MCParameterFile::c_DecayModeBuildUp },
    { "Build-Up", MCParameterFile::c_DecayModeBuildUp },
    { "BuildUpAndSave", MCParameterFile::c_DecayModeBuildUpAndSaveActivation },
    { "Ignore", MCParameterFile::c_DecayModeIgnore },
    { "ActivationBuildUp", MCParameterFile::c_DecayModeActivationBuildUp },
    { "ActivationBuildup", MCParameterFile::c_DecayModeActivationBuildUp },
    { "ActivationDelayedDecay", MCParameterFile::c_DecayModeActivationDelayedDecay }
  };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, Global(MString("DecayMode ") + E.Keyword + "\n"));
    Passed = EvaluateTrue("Open()", MString("DecayMode ") + E.Keyword, "The decay mode is accepted", Opened) && Passed;
    Passed = Evaluate("DecayMode", E.Keyword, "The decay mode is set", P.GetDecayMode(), E.Expected) && Passed;
  }

  Passed = ExpectRejected("Open()", "unknown mode", "An unknown decay mode is rejected", Global("DecayMode Unknown\n")) && Passed;
  Passed = ExpectRejected("Open()", "no mode", "The decay mode keyword needs a value", Global("DecayMode\n")) && Passed;
  Passed = ExpectRejected("Open()", "two modes", "The decay mode keyword takes one value", Global("DecayMode Normal Ignore\n")) && Passed;

  // The decay mode is given to the runs, but the parameter file keeps it:
  UTParameterFileAccess Two;
  Passed = EvaluateTrue("Open()", "two runs", "Two runs with a decay mode are accepted", Open(Two, Header() + "DecayMode BuildUp\nRun A\nA.Source S1\nRun B\nB.Source S2\n")) && Passed;
  Passed = Evaluate("DecayMode", "two runs", "The decay mode of the file is kept", Two.GetDecayMode(), MCParameterFile::c_DecayModeBuildUp) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestStoreKeywords()
{
  bool Passed = true;

  // StoreCalibrated:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "StoreCalibrated false", "StoreCalibrated is accepted", Open(P, Global("StoreCalibrated false\n"))) && Passed;
    Passed = EvaluateFalse("StoreCalibrated", "false", "Calibrated hits are not stored", P.StoreCalibrated()) && Passed;
    UTParameterFileAccess Q;
    Passed = EvaluateTrue("Open()", "StoreCalibrated true", "StoreCalibrated true is accepted", Open(Q, Global("StoreCalibrated false\nStoreCalibrated true\n"))) && Passed;
    Passed = EvaluateTrue("StoreCalibrated", "false, true", "The last line wins", Q.StoreCalibrated()) && Passed;
    Passed = ExpectRejected("Open()", "StoreCalibrated without value", "StoreCalibrated needs a value", Global("StoreCalibrated\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreCalibrated with two values", "StoreCalibrated takes one value", Global("StoreCalibrated true false\n")) && Passed;
  }

  // StoreSimulationInfoVersion:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "version 1", "The simulation info version is accepted", Open(P, Global("StoreSimulationInfoVersion 1\n"))) && Passed;
    Passed = Evaluate("StoreSimulationInfoVersion", "1", "The version is stored", P.StoreSimulationInfoVersion(), 1) && Passed;
    Passed = ExpectRejected("Open()", "version without value", "The version needs a value", Global("StoreSimulationInfoVersion\n")) && Passed;
  }

  // StoreSimulationInfo:
  {
    struct Entry { MString Keyword; int Expected; };
    vector<Entry> Entries = {
      { "yes", MSimEvent::c_StoreSimulationInfoAll }, { "all", MSimEvent::c_StoreSimulationInfoAll }, { "True", MSimEvent::c_StoreSimulationInfoAll },
      { "init", MSimEvent::c_StoreSimulationInfoInitOnly }, { "init-only", MSimEvent::c_StoreSimulationInfoInitOnly },
      { "no", MSimEvent::c_StoreSimulationInfoNone }, { "none", MSimEvent::c_StoreSimulationInfoNone }, { "false", MSimEvent::c_StoreSimulationInfoNone },
      { "ia", MSimEvent::c_StoreSimulationInfoIAOnly }, { "IA-Only", MSimEvent::c_StoreSimulationInfoIAOnly }
    };
    for (const Entry& E: Entries) {
      UTParameterFileAccess P;
      bool Opened = Open(P, Global(MString("StoreSimulationInfo ") + E.Keyword + "\n"));
      Passed = EvaluateTrue("Open()", MString("StoreSimulationInfo ") + E.Keyword, "The value is accepted", Opened) && Passed;
      Passed = Evaluate("StoreSimulationInfo", E.Keyword, "The storage level is set", P.StoreSimulationInfo(), E.Expected) && Passed;
    }
    Passed = ExpectRejected("Open()", "StoreSimulationInfo unknown", "An unknown storage level is rejected", Global("StoreSimulationInfo Unknown\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreSimulationInfo without value", "The storage level needs a value", Global("StoreSimulationInfo\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreSimulationInfo with two values", "The storage level takes one value", Global("StoreSimulationInfo all none\n")) && Passed;
  }

  // StoreSimulationInfoIonization:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "ionization true", "The ionization keyword is accepted", Open(P, Global("StoreSimulationInfo none\nStoreSimulationInfoIonization true\n"))) && Passed;
    Passed = EvaluateTrue("StoreSimulationInfoIonization", "true", "The ionization info is stored", P.StoreSimulationInfoIonization()) && Passed;
    Passed = Evaluate("StoreSimulationInfoIonization", "true", "Storing the ionization info needs the full simulation info", P.StoreSimulationInfo(), MSimEvent::c_StoreSimulationInfoAll) && Passed;
    UTParameterFileAccess Q;
    Passed = EvaluateTrue("Open()", "ionization false", "The ionization keyword with false is accepted", Open(Q, Global("StoreSimulationInfo none\nStoreSimulationInfoIonization false\n"))) && Passed;
    Passed = EvaluateFalse("StoreSimulationInfoIonization", "false", "The ionization info is not stored", Q.StoreSimulationInfoIonization()) && Passed;
    Passed = Evaluate("StoreSimulationInfoIonization", "false", "Switching it off does not change the storage level", Q.StoreSimulationInfo(), MSimEvent::c_StoreSimulationInfoNone) && Passed;
    Passed = ExpectRejected("Open()", "ionization without value", "The ionization keyword needs a value", Global("StoreSimulationInfoIonization\n")) && Passed;
  }

  // Event selection:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "one hit per event", "The keywords for the event selection are accepted", Open(P, Global("StoreOneHitPerEvent true\nStoreMinimumEnergy 30\nStoreMaximumEnergyLoss 12.5\nDiscretizeHits false\n"))) && Passed;
    Passed = EvaluateTrue("StoreOneHitPerEvent", "true", "Each hit is stored in its own event", P.StoreOneHitPerEvent()) && Passed;
    Passed = EvaluateNear("StoreMinimumEnergy", "30", "The minimum energy is given in keV", P.StoreMinimumEnergy()/keV, 30.0, 1e-9) && Passed;
    Passed = EvaluateNear("StoreMaximumEnergyLoss", "12.5", "The maximum energy loss is given in keV", P.StoreMaximumEnergyLoss()/keV, 12.5, 1e-9) && Passed;
    Passed = EvaluateFalse("DiscretizeHits", "false", "The hits are not discretized", P.DiscretizeHits()) && Passed;

    Passed = ExpectRejected("Open()", "StoreOneHitPerEvent without value", "StoreOneHitPerEvent needs a value", Global("StoreOneHitPerEvent\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreMinimumEnergy without value", "StoreMinimumEnergy needs a value", Global("StoreMinimumEnergy\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreMinimumEnergy with two values", "StoreMinimumEnergy takes one value", Global("StoreMinimumEnergy 1 2\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreMaximumEnergyLoss without value", "StoreMaximumEnergyLoss needs a value", Global("StoreMaximumEnergyLoss\n")) && Passed;
    Passed = ExpectRejected("Open()", "StoreMaximumEnergyLoss with two values", "StoreMaximumEnergyLoss takes one value", Global("StoreMaximumEnergyLoss 1 2\n")) && Passed;
    Passed = ExpectRejected("Open()", "DiscretizeHits without value", "DiscretizeHits needs a value", Global("DiscretizeHits\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestStoreFormat()
{
  bool Passed = true;

  // File format:
  struct Entry { MString Keyword; bool Binary; };
  vector<Entry> Entries = { { "binary", true }, { "b", true }, { "BIN", true }, { "text", false }, { "t", false }, { "plaintext", false }, { "ASCII", false } };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, Global(MString("FileFormat ") + E.Keyword + "\n"));
    Passed = EvaluateTrue("Open()", MString("FileFormat ") + E.Keyword, "The file format is accepted", Opened) && Passed;
    Passed = Evaluate("FileFormat", E.Keyword, "The binary flag is set as requested", P.StoreBinary(), E.Binary) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "binary then text", "Two file format lines are accepted", Open(P, Global("FileFormat binary\nFileFormat text\n"))) && Passed;
    Passed = EvaluateFalse("FileFormat", "binary, text", "The last line wins", P.StoreBinary()) && Passed;
  }
  Passed = ExpectRejected("Open()", "FileFormat unknown", "An unknown file format is rejected", Global("FileFormat xml\n")) && Passed;
  Passed = ExpectRejected("Open()", "FileFormat without value", "The file format needs a value", Global("FileFormat\n")) && Passed;
  Passed = ExpectRejected("Open()", "FileFormat with two values", "The file format takes one value", Global("FileFormat text binary\n")) && Passed;

  // Scientific output:
  vector<MString> Keywords = { "StoreTextScientific", "StoreScientific", "StoreSimulationInfoScientific" };
  for (const MString& Keyword: Keywords) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The scientific keyword is accepted", Open(P, Global(Keyword + " true 8\n"))) && Passed;
    Passed = EvaluateTrue(Keyword, "true 8", "The scientific format is switched on", P.StoreScientific()) && Passed;
    Passed = Evaluate(Keyword, "true 8", "The precision is stored", P.StoreScientificPrecision(), 8) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "StoreScientific false 6", "The scientific keyword with false is accepted", Open(P, Global("StoreScientific false 6\n"))) && Passed;
    Passed = EvaluateFalse("StoreScientific", "false 6", "The scientific format is off", P.StoreScientific()) && Passed;
    Passed = Evaluate("StoreScientific", "false 6", "The precision is stored", P.StoreScientificPrecision(), 6) && Passed;
    UTParameterFileAccess Minimum;
    Passed = EvaluateTrue("Open()", "StoreScientific true 3", "The minimum precision is accepted", Open(Minimum, Global("StoreScientific true 3\n"))) && Passed;
    Passed = Evaluate("StoreScientific", "true 3", "A precision of 3 is kept", Minimum.StoreScientificPrecision(), 3) && Passed;
    UTParameterFileAccess Low;
    Passed = EvaluateTrue("Open()", "StoreScientific true 1", "A too low precision is accepted", Open(Low, Global("StoreScientific true 1\n"))) && Passed;
    Passed = Evaluate("StoreScientific", "true 1", "A precision below 3 is raised to 3", Low.StoreScientificPrecision(), 3) && Passed;
    UTParameterFileAccess Negative;
    Passed = EvaluateTrue("Open()", "StoreScientific true -5", "A negative precision is accepted", Open(Negative, Global("StoreScientific true -5\n"))) && Passed;
    Passed = Evaluate("StoreScientific", "true -5", "A negative precision is raised to 3", Negative.StoreScientificPrecision(), 3) && Passed;
  }
  Passed = ExpectRejected("Open()", "StoreScientific one value", "The scientific keyword needs two values", Global("StoreScientific true\n")) && Passed;
  Passed = ExpectRejected("Open()", "StoreScientific three values", "The scientific keyword takes two values", Global("StoreScientific true 5 1\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestPreTrigger()
{
  bool Passed = true;

  struct Entry { MString Keyword; int Expected; };
  vector<Entry> Entries = {
    { "Everything", MCParameterFile::c_PreTriggerEverything },
    { "Full", MCParameterFile::c_PreTriggerFull },
    { "EveryEventWithHits", MCParameterFile::c_PreTriggerEveryEventWithHits }
  };
  for (const Entry& E: Entries) {
    UTParameterFileAccess P;
    bool Opened = Open(P, Global(MString("PreTriggerMode ") + E.Keyword + "\n"));
    Passed = EvaluateTrue("Open()", MString("PreTriggerMode ") + E.Keyword, "The pre-trigger mode is accepted", Opened) && Passed;
    Passed = Evaluate("PreTriggerMode", E.Keyword, "The pre-trigger mode is set", P.GetPreTriggerMode(), E.Expected) && Passed;
  }
  Passed = ExpectRejected("Open()", "unknown mode", "An unknown pre-trigger mode is rejected", Global("PreTriggerMode Unknown\n")) && Passed;
  Passed = ExpectRejected("Open()", "no mode", "The pre-trigger mode keyword needs a value", Global("PreTriggerMode\n")) && Passed;
  Passed = ExpectRejected("Open()", "two modes", "The pre-trigger mode keyword takes one value", Global("PreTriggerMode Full Everything\n")) && Passed;

  // The deprecated keywords:
  vector<MString> Deprecated = { "StoreOnlyEventsWithEnergyLoss", "StoreOnlyTriggeredEvents" };
  for (const MString& Keyword: Deprecated) {
    UTParameterFileAccess On;
    Passed = EvaluateTrue("Open()", Keyword + " true", "The deprecated keyword is accepted", Open(On, Global(Keyword + " true\n"))) && Passed;
    Passed = Evaluate(Keyword, "true", "Storing only triggered events means the full pre-trigger", On.GetPreTriggerMode(), MCParameterFile::c_PreTriggerFull) && Passed;
    UTParameterFileAccess Off;
    Passed = EvaluateTrue("Open()", Keyword + " false", "The deprecated keyword with false is accepted", Open(Off, Global(Keyword + " false\n"))) && Passed;
    Passed = Evaluate(Keyword, "false", "Storing all events means no pre-trigger", Off.GetPreTriggerMode(), MCParameterFile::c_PreTriggerEverything) && Passed;
    Passed = ExpectRejected("Open()", Keyword + " without value", "The deprecated keyword needs a value", Global(Keyword + "\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestVolumeLists()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "watched volumes", "Watched volumes and black absorbers are accepted", Open(P, Global(
    "StoreSimulationInfoWatchedVolumes Crystal Shield\nStoreSimulationInfoWatchedVolume Tracker\n"
    "BlackAbsorber Sphere\nBlackAbsorber Wall Floor\n"))) && Passed;
  vector<MString> Watched = P.GetStoreSimulationInfoWatchedVolumes();
  Passed = EvaluateSize("StoreSimulationInfoWatchedVolumes", "three volumes", "All watched volumes of both keywords are stored", Watched.size(), 3) && Passed;
  if (Watched.size() == 3) {
    Passed = Evaluate("StoreSimulationInfoWatchedVolumes", "first", "The first watched volume is stored", Watched[0], MString("Crystal")) && Passed;
    Passed = Evaluate("StoreSimulationInfoWatchedVolumes", "second", "The second watched volume is stored", Watched[1], MString("Shield")) && Passed;
    Passed = Evaluate("StoreSimulationInfoWatchedVolume", "third", "The volume of the singular keyword is stored", Watched[2], MString("Tracker")) && Passed;
  }
  vector<MString> Absorbers = P.GetBlackAbsorbers();
  Passed = EvaluateSize("BlackAbsorber", "three volumes", "All black absorbers are stored", Absorbers.size(), 3) && Passed;
  if (Absorbers.size() == 3) {
    Passed = Evaluate("BlackAbsorber", "first", "The first black absorber is stored", Absorbers[0], MString("Sphere")) && Passed;
    Passed = Evaluate("BlackAbsorber", "third", "The last black absorber is stored", Absorbers[2], MString("Floor")) && Passed;
  }

  Passed = ExpectRejected("Open()", "watched volumes without value", "The watched volumes keyword needs a volume", Global("StoreSimulationInfoWatchedVolumes\n")) && Passed;
  Passed = ExpectRejected("Open()", "black absorber without value", "The black absorber keyword needs a volume", Global("BlackAbsorber\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestStartArea()
{
  bool Passed = true;

  // Sphere for all sources of all runs:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "StartArea Sphere", "A start area sphere is accepted", Open(P, Header() + "StartArea Sphere 0 0 0 0 0 0 10\nRun A\nA.Source S1\nS1.Beam FarFieldPointSource 0 0\nS1.Spectrum Mono 100\nS1.Flux 1\nRun B\nB.Source S2\nS2.Beam FarFieldPointSource 0 0\nS2.Spectrum Mono 100\nS2.Flux 1\n")) && Passed;
    for (const char* Name: { "S1", "S2" }) {
      MCSource* S = P.GetSource(Name);
      Passed = EvaluateTrue("GetSource()", Name, "The source exists", S != nullptr) && Passed;
      if (S != nullptr) {
        Passed = Evaluate("StartArea Sphere", Name, "The start area type is a sphere", S->GetStartAreaType(), MCSource::c_StartAreaSphere) && Passed;
        Passed = EvaluateNear("StartArea Sphere", Name, "The average area is pi r^2 with r given in cm", S->GetStartAreaAverageArea()/(cm*cm), M_PI*100.0, 1e-6) && Passed;
      }
    }
  }

  // Tube for a far field point source: A = 4 R (H sin(theta) + R |cos(theta)|)
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "StartArea Tube", "A start area tube is accepted", Open(P, OneSource("S.Beam FarFieldPointSource 90 0\nS.Spectrum Mono 100\nS.Flux 1\nStartArea Tube 0 0 0 0 0 0 10 20\n"))) && Passed;
    MCSource* S = GetS(P);
    if (S != nullptr) {
      Passed = Evaluate("StartArea Tube", "S", "The start area type is a tube", S->GetStartAreaType(), MCSource::c_StartAreaTube) && Passed;
      Passed = EvaluateNear("StartArea Tube", "S", "The average area of a tube seen at 90 degrees is 4 R H", S->GetStartAreaAverageArea()/(cm*cm), 4*10.0*20.0, 1e-6) && Passed;
    }
  }

  // The start area is applied even if the keyword comes last:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "StartArea last", "A start area after the sources is accepted", Open(P, OneSource("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\nS.Flux 1\nStartArea Sphere 0 0 0 0 0 0 5\n"))) && Passed;
    Passed = Evaluate("StartArea Sphere", "after source", "The start area applies to all sources independent of the position of the keyword", GetS(P)->GetStartAreaType(), MCSource::c_StartAreaSphere) && Passed;
  }

  // A source without flux must be rejected, not abort the program:
  Passed = EvaluateTrue("Open()", "StartArea without flux", "A start area for a source without flux does not abort the program",
                        OpenInChild(OneSource("S.Beam FarFieldPointSource 0 0\nS.Spectrum Mono 100\n") + "StartArea Sphere 0 0 0 0 0 0 5\n") != UTSDHitShared::ChildOutcome::c_Crashed) && Passed;

  // No start area keyword:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "no StartArea", "A file without start area is accepted", Open(P, Global(""))) && Passed;
    Passed = Evaluate("StartArea", "not given", "Without the keyword there is no start area", GetS(P)->GetStartAreaType(), MCSource::c_StartAreaUnknown) && Passed;
  }

  // Errors:
  Passed = ExpectRejected("Open()", "StartArea without shape", "The start area needs a shape", Global("StartArea\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea unknown shape", "An unknown start area shape is rejected", Global("StartArea Cube 0 0 0 0 0 0 10\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Sphere too few", "A start area sphere needs 7 values", Global("StartArea Sphere 0 0 0 0 0 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Sphere too many", "A start area sphere takes 7 values", Global("StartArea Sphere 0 0 0 0 0 0 10 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Tube too few", "A start area tube needs 8 values", Global("StartArea Tube 0 0 0 0 0 0 10\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Tube too many", "A start area tube takes 8 values", Global("StartArea Tube 0 0 0 0 0 0 10 20 1\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Sphere zero radius", "A start area sphere needs a positive radius", Global("StartArea Sphere 0 0 0 0 0 0 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Sphere negative radius", "A start area sphere needs a positive radius", Global("StartArea Sphere 0 0 0 0 0 0 -3\n")) && Passed;
  Passed = ExpectRejected("Open()", "StartArea Sphere rotated", "A rotated start area is rejected since rotations are not yet implemented", Global("StartArea Sphere 0 0 0 1 0 0 10\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestIncludes()
{
  bool Passed = true;

  // A simple include:
  const MString Included = WriteSource("Included.source", "DefaultRangeCut 0.5\nStoreCalibrated false\n");
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "Include", "A file with an include is accepted", Open(P, Header() + "Include " + Included + "\nRun R\nR.Source S\n")) && Passed;
    Passed = EvaluateNear("Include", "range cut", "The lines of the included file are parsed", P.GetDefaultRangeCut()/mm, 5.0, 1e-9) && Passed;
    Passed = EvaluateFalse("Include", "calibrated", "All lines of the included file are parsed", P.StoreCalibrated()) && Passed;
  }

  // An included file can define runs and can include other files:
  const MString Innermost = WriteSource("Innermost.source", "Run Inner\nInner.Source SI\nInner.Time 12\n");
  const MString Middle = WriteSource("Middle.source", MString("DetectorTimeConstant 0.002\nInclude ") + Innermost + "\n");
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "nested include", "A nested include is accepted", Open(P, Header() + "Include " + Middle + "\n")) && Passed;
    Passed = Evaluate("Include", "nested", "The run of the nested include exists", P.GetNRuns(), 1U) && Passed;
    Passed = EvaluateNear("Include", "nested", "The keyword of the middle file is parsed", P.GetDetectorTimeConstant()/s, 0.002, 1e-9) && Passed;
    if (P.GetRun("Inner") != nullptr) {
      Passed = EvaluateNear("Include", "nested", "The run keyword of the innermost file is parsed", P.GetRun("Inner")->GetDuration()/s, 12.0, 1e-9) && Passed;
    }
  }

  // Two includes in one file:
  {
    const MString Other = WriteSource("Other.source", "DiscretizeHits false\n");
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "two includes", "Two includes are accepted", Open(P, Global(MString("Include ") + Included + "\nInclude " + Other + "\n"))) && Passed;
    Passed = EvaluateFalse("Include", "second", "The second include is parsed too", P.DiscretizeHits()) && Passed;
    Passed = EvaluateNear("Include", "first", "The first include is parsed too", P.GetDefaultRangeCut()/mm, 5.0, 1e-9) && Passed;
  }

  // Errors:
  Passed = ExpectRejected("Open()", "missing include", "An include of a missing file is rejected", Global(MString("Include ") + GetDirectory() + "/Missing.source\n")) && Passed;

  // A file including itself or two files including each other must not hang:
  const MString SelfName = GetDirectory() + "/SelfInclude.source";
  WriteTextFile(SelfName, Header() + "Include " + SelfName + "\nRun R\nR.Source S\n");
  const MString FirstName = GetDirectory() + "/CycleA.source";
  const MString SecondName = GetDirectory() + "/CycleB.source";
  WriteTextFile(FirstName, Header() + "Include " + SecondName + "\n");
  WriteTextFile(SecondName, MString("Include ") + FirstName + "\nRun R\nR.Source S\n");
  UTSDHitShared::ChildOutcome Self = UTSDHitShared::RunInChild([SelfName]() { UTParameterFileAccess P; return P.Open(SelfName); }, 1);
  // Known failure: the include expansion of Open() has no cycle detection, the included file re-inserts its own Include line and the loop never ends
  Passed = ExpectedToFail(EvaluateTrue("Open()", "self include", "A file including itself terminates", Self != UTSDHitShared::ChildOutcome::c_TimedOut)) && Passed;
  Passed = EvaluateFalse("Open()", "self include", "A file including itself is rejected", Self == UTSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;
  UTSDHitShared::ChildOutcome Cycle = UTSDHitShared::RunInChild([FirstName]() { UTParameterFileAccess P; return P.Open(FirstName); }, 1);
  // Known failure: the include expansion of Open() has no cycle detection, the two files keep inserting each other's Include line and the loop never ends
  Passed = ExpectedToFail(EvaluateTrue("Open()", "include cycle", "Two files including each other terminate", Cycle != UTSDHitShared::ChildOutcome::c_TimedOut)) && Passed;
  Passed = EvaluateFalse("Open()", "include cycle", "Two files including each other are rejected", Cycle == UTSDHitShared::ChildOutcome::c_ReturnedTrue) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileGlobal::TestConsistency()
{
  bool Passed = true;

  Passed = ExpectRejected("Open()", "empty file", "An empty file is rejected", "") && Passed;
  Passed = ExpectRejected("Open()", "only comments", "A file with only comments is rejected", "# Just a comment\n\n") && Passed;
  Passed = ExpectRejected("Open()", "geometry only", "A file without runs and activators is rejected", Header()) && Passed;
  Passed = ExpectRejected("Open()", "run without source", "A run without sources is rejected", Header() + "Run R\n") && Passed;
  Passed = ExpectRejected("Open()", "second run without source", "Each run needs a source", Header() + "Run A\nA.Source S\nRun B\n") && Passed;

  // An activator alone is fine:
  Passed = ExpectAccepted("Open()", "activator only", "A file with an activator and no run is accepted", Header() + "Activator Act\nAct.ActivationMode ConstantIrradiation 100\n") && Passed;

  // Comments and empty lines:
  Passed = ExpectAccepted("Open()", "comments", "Comments and empty lines are ignored", Header() + "# A comment\n\n   \nRun R\nR.Source S\n") && Passed;

  // Successors:
  Passed = ExpectAccepted("Open()", "successor", "A valid successor chain is accepted", OneSource("") + "R.Source T\nS.Successor T\nT.IsSuccessor true\n") && Passed;
  Passed = ExpectRejected("Open()", "unknown successor", "A successor which does not exist is rejected", OneSource("S.Successor Missing\n")) && Passed;
  Passed = ExpectRejected("Open()", "is successor without predecessor", "A source flagged as successor needs a predecessor", OneSource("S.IsSuccessor true\n")) && Passed;
  Passed = ExpectRejected("Open()", "two predecessors", "A successor with two predecessors is rejected", OneSource("") + "R.Source T\nR.Source U\nS.Successor U\nT.Successor U\nU.IsSuccessor true\n") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParameterFileGlobal Test;
  return Test.Run() == true ? 0 : 1;
}
