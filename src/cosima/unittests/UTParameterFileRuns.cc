/*
 * UTParameterFileRuns.cc
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
#include "MCOrientation.hh"
#include "MCParameterFile.hh"
#include "UTParameterFileShared.h"

// Standard lib:
#include <limits>
using namespace std;


//! Unit test class for the run and activator keywords of MCParameterFile
class UTParameterFileRuns : public UTParameterFileTest
{
public:
  //! Default constructor
  UTParameterFileRuns() : UTParameterFileTest("UTParameterFileRuns") {}
  //! Default destructor
  virtual ~UTParameterFileRuns() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the defaults of a run without any keyword
  bool TestRunDefaults();
  //! Test the duration keyword
  bool TestDuration();
  //! Test the triggers and events keywords
  bool TestTriggersEvents();
  //! Test the file name, TCP/IP and reconstruction keywords
  bool TestOutputKeywords();
  //! Test the orientation keywords
  bool TestOrientations();
  //! Test keywords which belong to a source but are used on a run
  bool TestMisplacedKeywords();
  //! Test multiple sources and runs
  bool TestSourcesPerRun();
  //! Test the activator keywords
  bool TestActivators();
  //! Test the activation and isotope count sources
  bool TestIsotopeSources();
  //! Test the activation mode handed from the file to the runs
  bool TestRunActivationMode();
  //! Test restricting the parameter file to one run
  bool TestRestrictToRun();
};


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::Run()
{
  bool Passed = true;

  if (PrepareFiles() == false) {
    Passed = EvaluateTrue("PrepareFiles()", "private directory", "The private directory with the geometry fixture can be created", false) && Passed;
    Summarize();
    return Passed;
  }

  Passed = TestRunDefaults() && Passed;
  Passed = TestDuration() && Passed;
  Passed = TestTriggersEvents() && Passed;
  Passed = TestOutputKeywords() && Passed;
  Passed = TestOrientations() && Passed;
  Passed = TestMisplacedKeywords() && Passed;
  Passed = TestSourcesPerRun() && Passed;
  Passed = TestActivators() && Passed;
  Passed = TestIsotopeSources() && Passed;
  Passed = TestRunActivationMode() && Passed;
  Passed = TestRestrictToRun() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestRunDefaults()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "run defaults", "A run without further keywords is accepted", Open(P, Global(""))) && Passed;
  MCRun* R = GetR(P);
  if (R == nullptr) return EvaluateTrue("GetRun()", "R", "The run exists", false) && Passed;

  Passed = Evaluate("Run", "defaults", "Without keywords the run stops by triggers", R->GetStopCondition(), MCRun::c_StopByTriggers) && Passed;
  Passed = EvaluateTrue("Run", "defaults", "Without keywords the duration is unlimited", R->GetDuration() == numeric_limits<double>::max()) && Passed;
  Passed = EvaluateTrue("Run", "defaults", "Without keywords the number of triggers is unlimited", R->GetTriggers() == numeric_limits<long>::max()) && Passed;
  Passed = EvaluateTrue("Run", "defaults", "Without keywords the number of events is unlimited", R->GetEvents() == numeric_limits<long>::max()) && Passed;
  Passed = Evaluate("Run", "defaults", "Without keywords there is no file name", R->GetFileName(), MString("")) && Passed;
  Passed = Evaluate("Run", "defaults", "Without keywords there is no TCP/IP host", R->GetTcpIpHostName(), MString("")) && Passed;
  Passed = EvaluateFalse("Run", "defaults", "Without keywords the sky is not oriented", R->GetSkyOrientation().IsOriented()) && Passed;
  Passed = EvaluateFalse("Run", "defaults", "Without keywords the detector is not oriented", R->GetDetectorOrientation().IsOriented()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestDuration()
{
  bool Passed = true;

  for (const char* Keyword: { "Duration", "Time", "duration" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The duration keyword is accepted", Open(P, Global(MString("R.") + Keyword + " 12.5\n"))) && Passed;
    Passed = EvaluateNear("Run.Duration", Keyword, "The duration is given in seconds", GetR(P)->GetDuration()/s, 12.5, 1e-9) && Passed;
    Passed = Evaluate("Run.Duration", Keyword, "A duration makes the run stop by time", GetR(P)->GetStopCondition(), MCRun::c_StopByTime) && Passed;
  }

  Passed = ExpectRejected("Open()", "Duration 0", "A duration of zero is rejected", Global("R.Duration 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "Duration -5", "A negative duration is rejected", Global("R.Duration -5\n")) && Passed;
  Passed = ExpectRejected("Open()", "Duration without value", "The duration needs a value", Global("R.Duration\n")) && Passed;
  Passed = ExpectRejected("Open()", "Duration with two values", "The duration takes one value", Global("R.Duration 1 2\n")) && Passed;
  Passed = ExpectRejected("Open()", "Duration text", "A duration which is not a number is rejected", Global("R.Duration abc\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestTriggersEvents()
{
  bool Passed = true;

  for (const char* Keyword: { "Triggers", "Trigger", "NTrigger", "NTriggers" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The triggers keyword is accepted", Open(P, Global(MString("R.") + Keyword + " 1000\n"))) && Passed;
    Passed = Evaluate("Run.Triggers", Keyword, "The number of triggers is stored", GetR(P)->GetTriggers(), 1000L) && Passed;
    Passed = Evaluate("Run.Triggers", Keyword, "Triggers make the run stop by triggers", GetR(P)->GetStopCondition(), MCRun::c_StopByTriggers) && Passed;
  }
  for (const char* Keyword: { "Events", "Event", "NEvent", "NEvents" }) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", Keyword, "The events keyword is accepted", Open(P, Global(MString("R.") + Keyword + " 2000\n"))) && Passed;
    Passed = Evaluate("Run.Events", Keyword, "The number of events is stored", GetR(P)->GetEvents(), 2000L) && Passed;
    Passed = Evaluate("Run.Events", Keyword, "Events make the run stop by events", GetR(P)->GetStopCondition(), MCRun::c_StopByEvents) && Passed;
  }

  // Large numbers must not be truncated to int:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "large events", "A number of events above the int range is accepted", Open(P, Global("R.Events 5000000000\n"))) && Passed;
    Passed = Evaluate("Run.Events", "5000000000", "The number is not truncated", GetR(P)->GetEvents(), 5000000000L) && Passed;
  }

  // The last stop keyword defines the stop condition:
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "events and duration", "Several stop keywords are accepted", Open(P, Global("R.Events 10\nR.Duration 5\n"))) && Passed;
    Passed = Evaluate("Run.Duration", "after events", "The last stop keyword defines the stop condition", GetR(P)->GetStopCondition(), MCRun::c_StopByTime) && Passed;
    Passed = Evaluate("Run.Events", "after events", "The number of events is still stored", GetR(P)->GetEvents(), 10L) && Passed;
  }

  for (const char* Keyword: { "Triggers", "Events" }) {
    Passed = ExpectRejected("Open()", MString(Keyword) + " 0", "Zero is rejected", Global(MString("R.") + Keyword + " 0\n")) && Passed;
    Passed = ExpectRejected("Open()", MString(Keyword) + " -3", "A negative number is rejected", Global(MString("R.") + Keyword + " -3\n")) && Passed;
    Passed = ExpectRejected("Open()", MString(Keyword) + " 1.5", "A floating point number is rejected", Global(MString("R.") + Keyword + " 1.5\n")) && Passed;
    Passed = ExpectRejected("Open()", MString(Keyword) + " abc", "Text is rejected", Global(MString("R.") + Keyword + " abc\n")) && Passed;
    Passed = ExpectRejected("Open()", MString(Keyword) + " without value", "The keyword needs a value", Global(MString("R.") + Keyword + "\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestOutputKeywords()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "output keywords", "The output keywords are accepted", Open(P, Global(
    "R.FileName Output\nR.TcpIp localhost 9090\nR.Reconstruction Revan.cfg Result.tra\nR.IsotopeProductionFile Isotopes.dat\n"))) && Passed;
  MCRun* R = GetR(P);
  if (R == nullptr) return EvaluateTrue("GetRun()", "R", "The run exists", false) && Passed;
  Passed = Evaluate("Run.FileName", "Output", "The file name is stored", R->GetFileName(), MString("Output")) && Passed;
  Passed = Evaluate("Run.TcpIp", "localhost 9090", "The host is stored", R->GetTcpIpHostName(), MString("localhost")) && Passed;
  Passed = Evaluate("Run.TcpIp", "localhost 9090", "The port is stored", R->GetTcpIpPort(), 9090U) && Passed;
  Passed = Evaluate("Run.Reconstruction", "Revan.cfg Result.tra", "The configuration file name is stored", R->GetRevanConfigurationFileName(), MString("Revan.cfg")) && Passed;
  Passed = Evaluate("Run.Reconstruction", "Revan.cfg Result.tra", "The tra file name is stored", R->GetTraFileName(), MString("Result.tra")) && Passed;

  Passed = ExpectRejected("Open()", "FileName without value", "The file name needs a value", Global("R.FileName\n")) && Passed;
  Passed = ExpectRejected("Open()", "FileName with two values", "The file name takes one value", Global("R.FileName A B\n")) && Passed;
  Passed = ExpectRejected("Open()", "TcpIp without port", "TcpIp needs host and port", Global("R.TcpIp localhost\n")) && Passed;
  Passed = ExpectRejected("Open()", "TcpIp with three values", "TcpIp takes host and port", Global("R.TcpIp localhost 1 2\n")) && Passed;
  // Known failure: the parser reads the port with GetTokenAtAsUnsignedInt(), which turns a negative value into port 0 instead of an error
  Passed = ExpectedToFail(ExpectRejected("Open()", "TcpIp negative port", "A negative TCP/IP port is rejected", Global("R.TcpIp localhost -1\n"))) && Passed;
  // Known failure: the parser reads the port with GetTokenAtAsUnsignedInt(), which converts a non-numeric token with atof() to port 0 instead of an error
  Passed = ExpectedToFail(ExpectRejected("Open()", "TcpIp text port", "A text TCP/IP port is rejected", Global("R.TcpIp localhost abc\n"))) && Passed;
  Passed = ExpectRejected("Open()", "Reconstruction one value", "Reconstruction needs two file names", Global("R.Reconstruction Revan.cfg\n")) && Passed;
  Passed = ExpectRejected("Open()", "IsotopeProductionFile without value", "The isotope production file needs a value", Global("R.IsotopeProductionFile\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestOrientations()
{
  bool Passed = true;

  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "sky Galactic Fixed", "A fixed galactic sky orientation is accepted", Open(P, Global("R.OrientationSky Galactic Fixed\nR.OrientationDetector Local Fixed\n"))) && Passed;
    Passed = EvaluateTrue("Run.OrientationSky", "Galactic Fixed", "The sky is oriented", GetR(P)->GetSkyOrientation().IsOriented()) && Passed;
    Passed = EvaluateTrue("Run.OrientationSky", "Galactic Fixed", "The sky uses galactic coordinates", GetR(P)->GetSkyOrientation().GetCoordinateSystem() == MCOrientationCoordinateSystem::c_Galactic) && Passed;
    Passed = EvaluateFalse("Run.OrientationDetector", "Local Fixed", "The detector is not oriented", GetR(P)->GetDetectorOrientation().IsOriented()) && Passed;
    Passed = EvaluateTrue("Run.OrientationDetector", "Local Fixed", "The detector uses local coordinates", GetR(P)->GetDetectorOrientation().GetCoordinateSystem() == MCOrientationCoordinateSystem::c_Local) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "sky Galactic Fixed 6 tokens", "A fixed galactic sky orientation with Z axis is accepted", Open(P, Global("R.OrientationSky Galactic Fixed 20 30\n"))) && Passed;
    Passed = EvaluateTrue("Run.OrientationSky", "Galactic Fixed 20 30", "The sky is oriented", GetR(P)->GetSkyOrientation().IsOriented()) && Passed;
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "sky Local Fixed", "A local sky is accepted", Open(P, Global("R.OrientationSky Local Fixed\n"))) && Passed;
    Passed = EvaluateFalse("Run.OrientationSky", "Local Fixed", "A local sky is not oriented", GetR(P)->GetSkyOrientation().IsOriented()) && Passed;
  }

  Passed = ExpectRejected("Open()", "detector Galactic", "The detector cannot be oriented in galactic coordinates", Global("R.OrientationDetector Galactic Fixed\n")) && Passed;
  Passed = ExpectRejected("Open()", "unknown coordinate system", "An unknown coordinate system is rejected", Global("R.OrientationSky Equatorial Fixed\n")) && Passed;
  Passed = ExpectRejected("Open()", "sky too few tokens", "An orientation needs a coordinate system and a type", Global("R.OrientationSky Galactic\n")) && Passed;
  Passed = ExpectRejected("Open()", "detector too few tokens", "An orientation needs a coordinate system and a type", Global("R.OrientationDetector Local\n")) && Passed;
  Passed = ExpectRejected("Open()", "sky Galactic Fixed five values", "A galactic fixed orientation has 0, 2 or 4 values", Global("R.OrientationSky Galactic Fixed 1 2 3\n")) && Passed;

  // Latitudes are angles in degrees within [-90, 90]:
  Passed = ExpectRejected("Open()", "latitude 100", "A galactic latitude above 90 degrees is rejected", Global("R.OrientationSky Galactic Fixed 100 0\n")) && Passed;
  Passed = ExpectRejected("Open()", "latitude -100", "A galactic latitude below -90 degrees is rejected", Global("R.OrientationSky Galactic Fixed -100 0\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestMisplacedKeywords()
{
  bool Passed = true;

  // The parser says for each of these "A run doesn't have a ... only a source does": a message for a user error must end in a rejected file like all other errors of the parser
  // Known failure: the parser prints the Typo() message for a beam on a run but does not return false, the file is accepted
  Passed = ExpectedToFail(ExpectRejected("Open()", "Run.Beam", "A beam defined on a run is rejected", Global("R.Beam FarFieldPointSource 0 0\n"))) && Passed;
  // Known failure: the parser prints the Typo() message for a spectrum on a run but does not return false, the file is accepted
  Passed = ExpectedToFail(ExpectRejected("Open()", "Run.Spectrum", "A spectrum defined on a run is rejected", Global("R.Spectrum Mono 100\n"))) && Passed;
  // Known failure: the parser prints the Typo() message for a flux on a run but does not return false, the file is accepted
  Passed = ExpectedToFail(ExpectRejected("Open()", "Run.Flux", "A flux defined on a run is rejected", Global("R.Flux 1\n"))) && Passed;
  // Known failure: the parser prints the Typo() message for a particle on a run but does not return false, the file is accepted
  Passed = ExpectedToFail(ExpectRejected("Open()", "Run.Particle", "A particle defined on a run is rejected", Global("R.ParticleType 1\n"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestSourcesPerRun()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "three sources", "A run with three sources is accepted", Open(P, Header() + "Run R\nR.Source A\nR.Source B\nR.Source C\nRun Q\nQ.Source D\n")) && Passed;
  MCRun* R = GetR(P);
  if (R == nullptr) return EvaluateTrue("GetRun()", "R", "The run exists", false) && Passed;
  Passed = Evaluate("Run.Source", "three", "The run has three sources", R->GetNSources(), 3U) && Passed;
  if (R->GetNSources() == 3) {
    Passed = Evaluate("Run.Source", "order", "The sources are in the order of the file", R->GetSource(0)->GetName(), MString("A")) && Passed;
    Passed = Evaluate("Run.Source", "order", "The last source is last", R->GetSource(2)->GetName(), MString("C")) && Passed;
  }
  Passed = Evaluate("Run.Source", "second run", "The other run has one source", P.GetRun("Q")->GetNSources(), 1U) && Passed;

  // A source line before the run line must still find the run - the parser is order independent for the sources:
  Passed = ExpectAccepted("Open()", "source before run", "Run keywords before the run line are accepted", Header() + "R.Source S\nRun R\n") && Passed;

  // A run keyword for a run which does not exist is treated as unknown text and ignored or rejected, but must not crash:
  UTSDHitShared::ChildOutcome Outcome = OpenInChild(Header() + "Run R\nR.Source S\nUnknown.Duration 5\n");
  Passed = EvaluateTrue("Open()", "keyword of unknown run", "A keyword of an unknown run does not crash or hang", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedTrue || Outcome == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestActivators()
{
  bool Passed = true;

  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "activators", "Two activators are accepted", Open(P, Header() +
    "Activator A\nA.ActivationMode ConstantIrradiation 3600\nA.ActivationFile Activity.dat\n"
    "Activator B\nB.ActivationMode ConstantIrradiationWithCooldown 100 50\nB.IsotopeCountFile Counts.dat\n")) && Passed;
  Passed = Evaluate("GetNActivators()", "two", "Both activators are stored", P.GetNActivators(), 2U) && Passed;
  Passed = EvaluateTrue("GetActivator()", "A", "The first activator can be found by its name", P.GetActivator("A") != nullptr) && Passed;
  Passed = EvaluateTrue("GetActivator(unsigned)", "0", "The first activator can be found by index", P.GetActivator(0U).GetName() == MString("A")) && Passed;
  MCActivator* A = P.GetActivator("A");
  MCActivator* B = P.GetActivator("B");
  if (A != nullptr) {
    Passed = EvaluateNear("Activator.ActivationMode", "ConstantIrradiation 3600", "The activation time is given in seconds", A->GetActivationTime()/s, 3600.0, 1e-9) && Passed;
    Passed = Evaluate("Activator.ActivationFile", "Activity.dat", "The output file is stored", A->GetOutputFileName(), MString("Activity.dat")) && Passed;
    Passed = EvaluateTrue("Activator.ActivationFile", "Activity.dat", "ActivationFile sets the activity output mode", A->GetOutputModeActivities()) && Passed;
  }
  if (B != nullptr) {
    Passed = EvaluateNear("Activator.ActivationMode", "ConstantIrradiationWithCooldown 100 50", "The activation time is the first value in seconds", B->GetActivationTime()/s, 100.0, 1e-9) && Passed;
    Passed = Evaluate("Activator.IsotopeCountFile", "Counts.dat", "The output file is stored", B->GetOutputFileName(), MString("Counts.dat")) && Passed;
    Passed = EvaluateTrue("Activator.IsotopeCountFile", "Counts.dat", "IsotopeCountFile sets the particle output mode", B->GetOutputModeParticles()) && Passed;
  }

  // Errors:
  Passed = ExpectRejected("Open()", "Activator without name", "An activator needs a name", Header() + "Activator\n") && Passed;
  Passed = ExpectRejected("Open()", "Activator with two names", "An activator takes one name", Header() + "Activator A B\n") && Passed;
  Passed = ExpectRejected("Open()", "unknown activation mode", "An unknown activation mode is rejected", Header() + "Activator A\nA.ActivationMode Sometimes 100\n") && Passed;
  Passed = ExpectRejected("Open()", "activation mode without time", "The activation mode needs a time", Header() + "Activator A\nA.ActivationMode ConstantIrradiation\n") && Passed;
  Passed = ExpectRejected("Open()", "activation time zero", "An activation time of zero is rejected", Header() + "Activator A\nA.ActivationMode ConstantIrradiation 0\n") && Passed;
  Passed = ExpectRejected("Open()", "activation time negative", "A negative activation time is rejected", Header() + "Activator A\nA.ActivationMode ConstantIrradiation -5\n") && Passed;
  Passed = ExpectRejected("Open()", "constant irradiation with two values", "ConstantIrradiation takes one time", Header() + "Activator A\nA.ActivationMode ConstantIrradiation 5 6\n") && Passed;
  Passed = ExpectRejected("Open()", "cooldown with one value", "ConstantIrradiationWithCooldown needs two times", Header() + "Activator A\nA.ActivationMode ConstantIrradiationWithCooldown 5\n") && Passed;
  Passed = ExpectRejected("Open()", "cooldown zero", "A cooldown time of zero is rejected", Header() + "Activator A\nA.ActivationMode ConstantIrradiationWithCooldown 5 0\n") && Passed;
  Passed = ExpectRejected("Open()", "ActivationFile without value", "The activation file needs a value", Header() + "Activator A\nA.ActivationFile\n") && Passed;
  Passed = ExpectRejected("Open()", "IsotopeCountFile without value", "The isotope count file needs a value", Header() + "Activator A\nA.IsotopeCountFile\n") && Passed;
  Passed = ExpectRejected("Open()", "IsotopeProductionFile without value", "The isotope production file needs a value", Header() + "Activator A\nA.IsotopeProductionFile\n") && Passed;

  // The time profile is announced as not implemented: it must not be accepted silently
  // Known failure: the parser prints the "Not yet implemented" Typo() message for the TimeProfile mode but does not return false, the file is accepted
  Passed = ExpectedToFail(ExpectRejected("Open()", "TimeProfile", "The not implemented time profile mode is rejected", Header() + "Activator A\nA.ActivationMode TimeProfile Profile.dat 100\n")) && Passed;

  // A production file which does not exist is ignored on purpose (several can be given):
  Passed = ExpectAccepted("Open()", "missing production file", "A missing isotope production file is ignored", Header() + "Activator A\nA.ActivationMode ConstantIrradiation 100\nA.IsotopeProductionFile " + GetDirectory() + "/Missing.dat\n") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestIsotopeSources()
{
  bool Passed = true;

  // Al-26 in the volume Crystal:
  const MString Store = WriteSource("Store.dat", "TT 100\nVN Crystal\nRP 1000130260 0.0 5000\nRP 1000130260 1057.739 2000\n");
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "ActivationSources", "Activation sources from a file are accepted", Open(P, Header() + "Run R\nR.ActivationSources " + Store + "\n")) && Passed;
    MCRun* R = GetR(P);
    if (R != nullptr) {
      Passed = Evaluate("Run.ActivationSources", "two levels", "One source is created per isotope level", R->GetNSources(), 2U) && Passed;
      if (R->GetNSources() >= 1) {
        Passed = Evaluate("Run.ActivationSources", "first source", "The source is named by volume, ID and excitation", R->GetSource(0)->GetName(), MString("Crystal_1000130260[0]")) && Passed;
        Passed = EvaluateNear("Run.ActivationSources", "first source", "The flux is the activity per second", R->GetSource(0)->GetFlux()*s, 5000.0, 1e-6) && Passed;
      }
    }
  }
  {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "IsotopeCountSources", "Isotope count sources from a file are accepted", Open(P, Header() + "Run R\nR.IsotopeCountSources " + Store + "\n")) && Passed;
    MCRun* R = GetR(P);
    if (R != nullptr) {
      Passed = Evaluate("Run.IsotopeCountSources", "two levels", "One source is created per isotope level", R->GetNSources(), 2U) && Passed;
    }
  }
  {
    // Both sources of the activation file can be combined with an explicit source:
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "sources and activation", "Activation sources and a normal source in one run are accepted", Open(P, Header() + "Run R\nR.Source S\nR.ActivationSources " + Store + "\n")) && Passed;
    if (GetR(P) != nullptr) {
      Passed = Evaluate("Run.ActivationSources", "with source", "The normal source and the activation sources are all in the run", GetR(P)->GetNSources(), 3U) && Passed;
    }
  }

  Passed = ExpectRejected("Open()", "missing activation file", "A missing activation source file is rejected", Header() + "Run R\nR.ActivationSources " + GetDirectory() + "/Missing.dat\n") && Passed;
  Passed = ExpectRejected("Open()", "missing isotope count file", "A missing isotope count source file is rejected", Header() + "Run R\nR.IsotopeCountSources " + GetDirectory() + "/Missing.dat\n") && Passed;
  Passed = ExpectRejected("Open()", "ActivationSources without value", "The activation sources keyword needs a file", Header() + "Run R\nR.ActivationSources\n") && Passed;
  Passed = ExpectRejected("Open()", "IsotopeCountSources without value", "The isotope count sources keyword needs a file", Header() + "Run R\nR.IsotopeCountSources\n") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestRunActivationMode()
{
  bool Passed = true;

  // The decay mode of the file is handed to all runs - checked through the parameter file getter since the run has no public getter for the mode
  UTParameterFileAccess P;
  Passed = EvaluateTrue("Open()", "decay mode", "A file with a decay mode and two runs is accepted", Open(P, Header() + "DecayMode ActivationBuildUp\nRun A\nA.Source S1\nRun B\nB.Source S2\n")) && Passed;
  Passed = Evaluate("DecayMode", "ActivationBuildUp", "The decay mode of the file is kept", P.GetDecayMode(), MCParameterFile::c_DecayModeActivationBuildUp) && Passed;
  Passed = Evaluate("GetNRuns()", "decay mode", "Both runs exist", P.GetNRuns(), 2U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTParameterFileRuns::TestRestrictToRun()
{
  bool Passed = true;

  const MString Content = Header() + "Run First\nFirst.Source S1\nRun Second\nSecond.Source S2\nRun Third\nThird.Source S3\n";
  for (int Index = 0; Index < 3; ++Index) {
    UTParameterFileAccess P;
    Passed = EvaluateTrue("Open()", "three runs", "A file with three runs is accepted", Open(P, Content)) && Passed;
    P.RestrictToRun(Index);
    Passed = Evaluate("RestrictToRun()", Index, "Only one run is left", P.GetNRuns(), 1U) && Passed;
    const char* Names[] = { "First", "Second", "Third" };
    Passed = Evaluate("RestrictToRun()", Index, "The requested run is the one which is left", P.GetCurrentRun().GetName(), MString(Names[Index])) && Passed;
  }

  // An index outside of the list must not crash - it is checked in a child process:
  const MString Name = WriteSource("Restrict.source", Content);
  UTSDHitShared::ChildOutcome Outcome = UTSDHitShared::RunInChild([Name]() {
    UTParameterFileAccess P;
    if (P.Open(Name) == false) return false;
    P.RestrictToRun(7);
    return true;
  }, 5);
  // Known failure: RestrictToRun() erases the first run Index times without checking the size, erasing from the empty run list is undefined behavior
  Passed = ExpectedToFail(EvaluateTrue("RestrictToRun()", "7 of 3", "An index beyond the number of runs does not crash", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedTrue)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParameterFileRuns Test;
  return Test.Run() == true ? 0 : 1;
}
