/*
 * UTEndToEndCampaign.cxx
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
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MEndToEndCampaign.h"
#include "MEndToEndFingerprint.h"
#include "MEndToEndReferenceList.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndCampaign: the tests of the campaign are shell scripts
class UTEndToEndCampaign : public MUnitTest
{
public:
  UTEndToEndCampaign() : MUnitTest("UTEndToEndCampaign") {}
  virtual ~UTEndToEndCampaign() {}

  virtual bool Run();

private:
  //! Write an executable shell script into the directory
  bool WriteScript(const MString& Directory, const MString& Name, const MString& Body);
  //! Write the test programs into the directory: ETFake (writes its key results, its environment, its home, and its seed), ETFailing, ETKnown, ETSlow, and ETSilent
  bool CreateTests(const MString& Directory, const MString& CounterFile);
  //! Return the number of lines of a text file, zero if it does not exist
  unsigned int CountLines(const MString& FileName);

  bool TestNames();
  bool TestRuns();
  bool TestFailures();
  bool TestContainment();
  bool TestAnalysis();
  bool TestRecordFailure();
  bool TestLostRuns();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::WriteScript(const MString& Directory, const MString& Name, const MString& Body)
{
  const MString FileName = Directory + "/" + Name;
  {
    ofstream Out(FileName.Data());
    if (Out.is_open() == false) {
      return false;
    }
    Out<<"#!/bin/sh"<<endl<<Body<<endl;
  }
  error_code Error;
  filesystem::permissions(FileName.Data(), filesystem::perms::owner_all, filesystem::perm_options::add, Error);
  if (Error.value() != 0) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::CreateTests(const MString& Directory, const MString& CounterFile)
{
  bool OK = MFile::CreateDirectory(Directory);

  // The value of the first observable depends on the seed, the second is constant, the files are in the working directory of the run
  const MString Fake =
    "V=$((MEGALIB_END_TO_END_SEED % 97))\n"
    "printf '# header\\n' > ETFake.keyresults.tsv\n"
    "printf 'A/x\\t%s.5\\t0.5\\t0\\t0\\t0\\t0\\t0\\t1\\t1\\t1\\t1\\n' \"$V\" >> ETFake.keyresults.tsv\n"
    "printf 'A/y\\t2.0\\t0.1\\t0\\t0\\t0\\t0\\t0\\t1\\t1\\t1\\t1\\n' >> ETFake.keyresults.tsv\n"
    "printf '# test: ETFake\\n# geant4: 10.2.3\\n# root: 6.40.04\\n# geometry: 00000000000000aa\\n# scenarios: 00000000000000bb\\n' > ETFake.fingerprint.tsv\n"
    "echo \"$HOME\" > home.txt\n"
    "echo \"$MEGALIB_END_TO_END_SEED\" > seed.txt\n"
    "echo \"Passed tests: 3\"; echo \"Failed tests: 0\"";
  OK = WriteScript(Directory, "ETFake", Fake) && OK;
  OK = WriteScript(Directory, "ETFailing", "echo x >> \"" + CounterFile + "\"; echo \"Passed tests: 1\"; echo \"Failed tests: 2\"; exit 1") && OK;
  // The same test under another name whose only failed check is a known failure: it exits with 0, as the test driver lets it pass
  MString Known = Fake;
  Known.ReplaceAllInPlace("ETFake", "ETKnown");
  OK = WriteScript(Directory, "ETKnown", Known + "; echo \"Known failures: 1\"") && OK;
  OK = WriteScript(Directory, "ETSlow", "sleep 20; echo \"Passed tests: 1\"; echo \"Failed tests: 0\"") && OK;
  OK = WriteScript(Directory, "ETSilent", "echo \"no summary\"") && OK;
  // The same test under another name, which makes the directory of its second run read-only at the end
  MString ReadOnly = Fake;
  ReadOnly.ReplaceAllInPlace("ETFake", "ETReadOnly");
  OK = WriteScript(Directory, "ETReadOnly", ReadOnly + "\nif [ \"${HOME##*_}\" = 001 ]; then chmod a-w \"$HOME\"; fi") && OK;
  return OK;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTEndToEndCampaign::CountLines(const MString& FileName)
{
  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    return 0;
  }
  return Content.Tokenize("\n").size();
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::Run()
{
  bool Passed = true;
  Passed = TestNames() && Passed;
  Passed = TestRuns() && Passed;
  Passed = TestFailures() && Passed;
  Passed = TestContainment() && Passed;
  Passed = TestAnalysis() && Passed;
  Passed = TestRecordFailure() && Passed;
  Passed = TestLostRuns() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestNames()
{
  bool Passed = true;

  MEndToEndCampaign Campaign("/bin", "/tmp/campaign");
  Passed = Evaluate("MEndToEndCampaign::GetRunDirectory()", "number", "The runs are numbered with three digits", Campaign.GetRunDirectory("ETFoo", 7), MString("/tmp/campaign/ETFoo/run_007")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::GetRunDirectory()", "large", "A number with three digits is kept", Campaign.GetRunDirectory("ETFoo", 123), MString("/tmp/campaign/ETFoo/run_123")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::GetCandidateFileName()", "name", "The candidate references are in the campaign directory", Campaign.GetCandidateFileName("ETFoo"), MString("/tmp/campaign/ETFoo.references.tsv")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::GetReportFileName()", "name", "The report is in the campaign directory", Campaign.GetReportFileName("ETFoo"), MString("/tmp/campaign/ETFoo.campaign.pdf")) && Passed;

  // The seeds: reproducible, different for runs, tests, and base seeds, in the range of the seeds of the tests
  Passed = Evaluate("MEndToEndCampaign::GetSeed()", "reproducible", "The same run has the same seed", Campaign.GetSeed("ETFoo", 3), Campaign.GetSeed("ETFoo", 3)) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::GetSeed()", "runs", "Two runs have different seeds", Campaign.GetSeed("ETFoo", 3) != Campaign.GetSeed("ETFoo", 4)) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::GetSeed()", "tests", "Two tests have different seeds", Campaign.GetSeed("ETFoo", 3) != Campaign.GetSeed("ETBar", 3)) && Passed;
  const unsigned int First = Campaign.GetSeed("ETFoo", 3);
  Campaign.SetBaseSeed(2);
  Passed = EvaluateTrue("MEndToEndCampaign::GetSeed()", "base seed", "Another base seed changes the seed", Campaign.GetSeed("ETFoo", 3) != First) && Passed;
  bool InRange = true;
  for (unsigned int r = 0; r < 200; ++r) {
    const unsigned int Seed = Campaign.GetSeed("ETFoo", r);
    if (Seed < 1 || Seed > 2000000000U) {
      InRange = false;
    }
  }
  Passed = EvaluateTrue("MEndToEndCampaign::GetSeed()", "range", "All seeds are between 1 and 2000000000", InRange) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestRuns()
{
  bool Passed = true;

  const MString Bin = GetTemporaryDirectoryName("bin");
  const MString Analysis = GetTemporaryDirectoryName("campaign");
  const MString Counter = GetTemporaryDirectoryName("counter") + ".txt";
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, Counter)) && Passed;

  MEndToEndCampaign Campaign(Bin, Analysis);
  Campaign.SetNumberOfRuns(4);
  Campaign.SetMaximumParallel(2);
  const vector<MString> Tests = { "ETFake" };
  vector<ETCampaignJob> Jobs = Campaign.GetJobs(Tests);
  Passed = EvaluateSize("MEndToEndCampaign::GetJobs()", "all", "Nothing is done: all four runs are jobs", Jobs.size(), 4) && Passed;
  Passed = Evaluate("MEndToEndCampaign::GetJobs()", "directory", "The directory of the second job", Jobs[1].m_Directory, Campaign.GetRunDirectory("ETFake", 1)) && Passed;
  Passed = Evaluate("MEndToEndCampaign::GetJobs()", "seed", "The seed of the second job", Jobs[1].m_Seed, Campaign.GetSeed("ETFake", 1)) && Passed;

  DisableDefaultStreams();
  const bool Done = Campaign.RunJobs(Jobs);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "success", "The runs are made", Done) && Passed;
  for (unsigned int r = 0; r < 4; ++r) {
    ETCampaignRecord Finished;
    Passed = EvaluateTrue("ETCampaignRecord::Read()", MString("run ") + r, "A run which passed has a record", Finished.Read(Campaign.GetRunDirectory("ETFake", r))) && Passed;
    Passed = EvaluateTrue("ETCampaignRecord::Succeeded()", MString("run ") + r, "A run which passed has no failed check", Finished.Succeeded()) && Passed;
  }

  // The run has its directory as home and as log directory, and its seed in the environment
  const MString Directory = Campaign.GetRunDirectory("ETFake", 2);
  MString Home, Seed, Settings;
  MFile::ReadTextFile(Directory + "/home.txt", Home);
  MFile::ReadTextFile(Directory + "/seed.txt", Seed);
  MFile::ReadTextFile(Directory + "/.testdrive.cfg", Settings);
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "home", "The home of the run is its directory", Home.Tokenize("\n")[0], Directory) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "seed", "The base seed of the run is in its environment", Seed.Tokenize("\n")[0], MString(Campaign.GetSeed("ETFake", 2))) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "settings", "The settings of the run name its directory as the log directory", Settings.Contains(Directory) == true) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "log", "The output of the run is in its directory", MFile::Exists(Directory + "/ETFake.log")) && Passed;
  MEndToEndRecord Marker;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "marker", "The file which says how the run ended can be read", Marker.Read(Directory + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "marker test", "The marker names the test", Marker.Get("test"), MString("ETFake")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "marker run", "The marker names the run", Marker.Get("run"), MString("2")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "marker failed", "The marker has no failed check", Marker.Get("failed_checks"), MString("0")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "marker exit", "The marker has the exit code", Marker.Get("exit_code"), MString("0")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "marker timeout", "The run did not time out", Marker.Get("timed_out"), MString("0")) && Passed;

  // A campaign which is started again starts over: all runs are jobs, and the old results are removed
  Passed = EvaluateSize("MEndToEndCampaign::GetJobs()", "again", "After the runs all four runs are jobs again", Campaign.GetJobs(Tests).size(), 4) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "old result", "A file of the old results can be written", WriteTextFile(Directory + "/old.txt", "old")) && Passed;
  DisableDefaultStreams();
  const bool Again = Campaign.RunJobs(Campaign.GetJobs(Tests));
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "again", "The runs are made again", Again) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "old result", "The old results are removed", MFile::Exists(Directory + "/old.txt")) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "new marker", "The run has a new marker", MFile::Exists(Directory + "/run.done")) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::GetJobs()", "no jobs", "No tests are no jobs", Campaign.GetJobs({ }).size(), 0) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "empty", "Without jobs the run is a success", Campaign.RunJobs({ })) && Passed;

  // A relative directory of the test programs: the runs change their directory, the tests are still found
  const MString RelativeCampaign = GetTemporaryDirectoryName("campaign_relative");
  error_code RelativeError;
  const MString RelativeBin = filesystem::relative(Bin.Data(), filesystem::current_path(RelativeError), RelativeError).string().c_str();
  MEndToEndCampaign Relative(RelativeBin, RelativeCampaign);
  Relative.SetNumberOfRuns(1);
  DisableDefaultStreams();
  Relative.RunJobs(Relative.GetJobs(Tests));
  EnableDefaultStreams();
  ETCampaignRecord RelativeRecord;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "relative bin", "A relative directory of the test programs works", RelativeRecord.Read(Relative.GetRunDirectory("ETFake", 0)) == true && RelativeRecord.Succeeded() == true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestFailures()
{
  bool Passed = true;

  const MString Bin = GetTemporaryDirectoryName("failbin");
  const MString Analysis = GetTemporaryDirectoryName("failcampaign");
  const MString Counter = GetTemporaryDirectoryName("failcounter") + ".txt";
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, Counter)) && Passed;

  MEndToEndCampaign Campaign(Bin, Analysis);
  Campaign.SetNumberOfRuns(2);
  Campaign.SetMaximumParallel(2);
  Campaign.SetTimeout(1.0);

  // A test which fails: two failed checks
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETFailing" }));
  EnableDefaultStreams();
  MEndToEndRecord Marker;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "failing marker", "The marker of a failed run can be read", Marker.Read(Campaign.GetRunDirectory("ETFailing", 0) + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "failed checks", "The marker has the two failed checks", Marker.Get("failed_checks"), MString("2")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "failed exit", "The marker has the exit code", Marker.Get("exit_code"), MString("1")) && Passed;
  ETCampaignRecord Failing;
  Failing.Read(Campaign.GetRunDirectory("ETFailing", 0));
  Passed = EvaluateFalse("ETCampaignRecord::Succeeded()", "failed", "A run with failed checks does not succeed", Failing.Succeeded()) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::GetJobs()", "again", "The failed runs are jobs again", Campaign.GetJobs({ "ETFailing" }).size(), 2) && Passed;
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETFailing" }));
  EnableDefaultStreams();
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "started again", "The failed runs started again", CountLines(Counter), 4U) && Passed;

  // A test which does not report its checks has failed
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETSilent" }));
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "silent marker", "The marker of a test without summary can be read", Marker.Read(Campaign.GetRunDirectory("ETSilent", 0) + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "silent", "A test without summary has a failed check", Marker.Get("failed_checks"), MString("1")) && Passed;

  // A test with a known failure has failed in a campaign, although it exits with 0: no references are made from a broken check
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETKnown" }));
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "known marker", "The marker of a test with a known failure can be read", Marker.Read(Campaign.GetRunDirectory("ETKnown", 0) + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "known failure", "The known failure is a failed check", Marker.Get("failed_checks"), MString("1")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "known exit", "The marker has the exit code 0 of the test", Marker.Get("exit_code"), MString("0")) && Passed;
  ETCampaignRecord Known;
  Known.Read(Campaign.GetRunDirectory("ETKnown", 0));
  Passed = EvaluateFalse("ETCampaignRecord::Succeeded()", "known", "A run with a known failure does not succeed", Known.Succeeded()) && Passed;

  // A test which takes longer than the time out is stopped
  const chrono::steady_clock::time_point Start = chrono::steady_clock::now();
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETSlow" }));
  EnableDefaultStreams();
  const double Seconds = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - Start).count();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "stopped", "The runs of 20 s are stopped after the time out of 1 s (both in parallel)", Seconds < 10.0) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "slow marker", "The marker of a run which was stopped can be read", Marker.Read(Campaign.GetRunDirectory("ETSlow", 1) + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::RunJobs()", "timed out", "The marker says that the run timed out", Marker.Get("timed_out"), MString("1")) && Passed;
  ETCampaignRecord Slow;
  Slow.Read(Campaign.GetRunDirectory("ETSlow", 1));
  Passed = EvaluateFalse("ETCampaignRecord::Succeeded()", "timed out", "A run which timed out does not succeed", Slow.Succeeded()) && Passed;

  // A test which does not exist cannot be started: the run has a marker with the shell exit code
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETMissing" }));
  EnableDefaultStreams();
  ETCampaignRecord Missing;
  Missing.Read(Campaign.GetRunDirectory("ETMissing", 0));
  Passed = EvaluateFalse("ETCampaignRecord::Succeeded()", "missing", "A run of a test which does not exist does not succeed", Missing.Succeeded()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestContainment()
{
  bool Passed = true;

  const MString Bin = GetTemporaryDirectoryName("containbin");
  const MString Directory = GetTemporaryDirectoryName("containcampaign");
  const MString Outside = GetTemporaryDirectoryName("containoutside");
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, GetTemporaryDirectoryName("containcounter") + ".txt")) && Passed;
  Passed = EvaluateTrue("MFile::CreateDirectory()", "campaign", "The campaign directory can be created", MFile::CreateDirectory(Directory)) && Passed;

  // A file outside of the campaign which must survive: <outside>/<name>/run_000/sentinel.txt
  auto MakeSentinel = [](const MString& Base, const MString& Name) {
    MFile::CreateDirectory(Base + "/" + Name + "/run_000");
    ofstream Out((Base + "/" + Name + "/run_000/sentinel.txt").Data());
    Out<<"keep"<<endl;
    return MFile::Exists(Base + "/" + Name + "/run_000/sentinel.txt");
  };

  MEndToEndCampaign Campaign(Bin, Directory);
  Campaign.SetNumberOfRuns(1);
  Campaign.SetMaximumParallel(1);

  // The directory of the test is a link to a directory outside: nothing there is touched, the run does not start
  Passed = EvaluateTrue("MakeSentinel()", "linked test", "The file outside can be written", MakeSentinel(Outside, "linked")) && Passed;
  error_code Error;
  filesystem::create_directory_symlink((Outside + "/linked").Data(), (Directory + "/ETFake").Data(), Error);
  Passed = EvaluateTrue("create_directory_symlink()", "linked test", "The link can be made", Error.value() == 0) && Passed;
  DisableDefaultStreams();
  bool Started = Campaign.RunJobs(Campaign.GetJobs({ "ETFake" }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "linked test", "A test directory which is a link is not used", Started) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "linked test file", "The file outside the campaign was not deleted", MFile::Exists(Outside + "/linked/run_000/sentinel.txt")) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "linked test run", "Nothing was written outside the campaign", MFile::Exists(Outside + "/linked/run_000/ETFake.log")) && Passed;

  // The directory of the run is a link: the link stays, the directory behind it is not touched
  Passed = EvaluateTrue("MakeSentinel()", "linked run", "The file outside can be written", MakeSentinel(Outside, "linkedrun")) && Passed;
  Passed = EvaluateTrue("MFile::CreateDirectory()", "test directory", "The directory of the test can be created", MFile::CreateDirectory(Directory + "/ETSilent")) && Passed;
  filesystem::create_directory_symlink((Outside + "/linkedrun/run_000").Data(), (Directory + "/ETSilent/run_000").Data(), Error);
  DisableDefaultStreams();
  Started = Campaign.RunJobs(Campaign.GetJobs({ "ETSilent" }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "linked run", "A run directory which is a link is not used", Started) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "linked run file", "The file behind the link was not deleted", MFile::Exists(Outside + "/linkedrun/run_000/sentinel.txt")) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "linked run link", "The link is still there", filesystem::is_symlink((Directory + "/ETSilent/run_000").Data())) && Passed;

  // A test name which leaves the campaign directory: ../<outside>/escape is <outside>/escape, next to the campaign directory
  Passed = Evaluate("GetTemporaryDirectoryName()", "same parent", "The directories of the test are next to each other", MFile::GetDirectoryName(Outside), MFile::GetDirectoryName(Directory)) && Passed;
  Passed = EvaluateTrue("MakeSentinel()", "escape", "The file outside can be written", MakeSentinel(Outside, "escape")) && Passed;
  DisableDefaultStreams();
  Started = Campaign.RunJobs(Campaign.GetJobs({ MString("../") + MFile::GetBaseName(Outside) + "/escape" }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "escape", "A test name with .. is not used", Started) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "escape file", "The file outside the campaign was not deleted", MFile::Exists(Outside + "/escape/run_000/sentinel.txt")) && Passed;

  // A job which names another directory than the one of its run
  Passed = EvaluateTrue("MakeSentinel()", "foreign job", "The file outside can be written", MakeSentinel(Outside, "foreign")) && Passed;
  ETCampaignJob Foreign;
  Foreign.m_Test = "ETFake";
  Foreign.m_Run = 0;
  Foreign.m_Directory = Outside + "/foreign/run_000";
  Foreign.m_Seed = 1;
  DisableDefaultStreams();
  Started = Campaign.RunJobs({ Foreign });
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "foreign job", "A job with a directory which is not the directory of its run is not started", Started) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "foreign file", "The file in the foreign directory was not deleted", MFile::Exists(Outside + "/foreign/run_000/sentinel.txt")) && Passed;

  // The campaign directory itself may be a link
  const MString Linked = GetTemporaryDirectoryName("containlinked");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "real campaign", "The directory behind the link can be created", MFile::CreateDirectory(Outside + "/realcampaign")) && Passed;
  filesystem::create_directory_symlink((Outside + "/realcampaign").Data(), Linked.Data(), Error);
  MEndToEndCampaign ThroughLink(Bin, Linked);
  ThroughLink.SetNumberOfRuns(1);
  DisableDefaultStreams();
  Started = ThroughLink.RunJobs(ThroughLink.GetJobs({ "ETSilent" }));
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "campaign link", "A campaign directory which is a link is used", Started) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::RunJobs()", "campaign link run", "The run is made behind the link", MFile::Exists(Outside + "/realcampaign/ETSilent/run_000/run.done")) && Passed;

  // No campaign directory: nothing runs, nothing is created at the root of the file system
  MEndToEndCampaign Nowhere(Bin, "");
  Nowhere.SetNumberOfRuns(1);
  DisableDefaultStreams();
  Started = Nowhere.RunJobs(Nowhere.GetJobs({ "ETSilent" }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "no campaign directory", "A campaign without a directory is not run", Started) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "root", "Nothing is made at the root of the file system", MFile::Exists("/ETSilent")) && Passed;

  // A stale run directory which can not be emptied: the job fails instead of running on old files
  MFile::CreateDirectory(Directory + "/ETStale/run_000/sub");
  { ofstream Out((Directory + "/ETStale/run_000/sub/old.txt").Data()); Out<<"old"<<endl; }
  filesystem::permissions((Directory + "/ETStale/run_000/sub").Data(), filesystem::perms::owner_read | filesystem::perms::owner_exec, Error);
  DisableDefaultStreams();
  Started = Campaign.RunJobs(Campaign.GetJobs({ "ETStale" }));
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "stale", "A run directory which can not be removed fails the job", Started) && Passed;
  filesystem::permissions((Directory + "/ETStale/run_000/sub").Data(), filesystem::perms::owner_all, Error);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestAnalysis()
{
  bool Passed = true;

  const MString Bin = GetTemporaryDirectoryName("anabin");
  const MString Analysis = GetTemporaryDirectoryName("anacampaign");
  const MString References = GetTemporaryDirectoryName("references");
  const MString Counter = GetTemporaryDirectoryName("anacounter") + ".txt";
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, Counter)) && Passed;

  MEndToEndCampaign Campaign(Bin, Analysis);
  Campaign.SetNumberOfRuns(6);
  Campaign.SetMinimumRuns(5);
  Campaign.SetMaximumParallel(3);
  Campaign.SetReferencesDirectory(References);
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETFake" }));
  EnableDefaultStreams();

  MEndToEndCampaignAnalysis Result;
  DisableDefaultStreams();
  const bool Written = Campaign.Analyze("ETFake", "A reason", Result);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "written", "The candidate references and the report are written", Written) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "runs", "All six runs are analyzed", Result.GetNumberOfRuns(), 6U) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::Analyze()", "observables", "Both observables of the test", Result.GetObservables().size(), 2) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::Analyze()", "errors", "A good campaign has no error", Result.HasErrors()) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "candidate", "The candidate references exist", MFile::Exists(Campaign.GetCandidateFileName("ETFake"))) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "report", "The report exists", MFile::Exists(Campaign.GetReportFileName("ETFake"))) && Passed;
  MEndToEndReferenceList Candidate;
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "candidate read", "The candidate references can be read", Candidate.Read(Campaign.GetCandidateFileName("ETFake"))) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "reason", "The record has the reason", Candidate.GetHeader().Get("reason"), MString("A reason")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "base seed", "The record has the base seed of the campaign", Candidate.GetHeader().Get("base_seed"), MString("1")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "geant4", "The record has the environment of the runs", Candidate.GetHeader().Get("geant4"), MString("10.2.3")) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::Analyze()", "references", "The candidate has a reference for both observables", Candidate.GetNumberOfReferences(), 2) && Passed;

  // Install: the references are in the references directory and match the environment of the runs
  Passed = EvaluateTrue("MEndToEndCampaign::Install()", "install", "The references of a good campaign are installed", Campaign.Install("ETFake", "A reason", Result)) && Passed;
  MEndToEndReferenceList Installed;
  Passed = EvaluateTrue("MEndToEndCampaign::Install()", "installed read", "The installed references can be read", Installed.Read(References + "/ETFake.references.tsv")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Install()", "installed reason", "The installed record has the reason and the base seed", Installed.GetHeader().Get("reason") + "/" + Installed.GetHeader().Get("base_seed"), MString("A reason/1")) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::Install()", "installed number", "Both references are installed", Installed.GetNumberOfReferences(), 2) && Passed;
  Passed = EvaluateSize("MEndToEndCampaign::Install()", "installed fingerprint", "The installed references match the environment of the runs", MEndToEndFingerprint::Compare(Installed.GetHeader(), Result.GetFingerprint()).size(), 0) && Passed;

  // A campaign with too few runs has an error: nothing is installed
  MEndToEndCampaign Strict(Bin, Analysis);
  Strict.SetNumberOfRuns(6);
  Strict.SetMinimumRuns(10);
  Strict.SetReferencesDirectory(References + "/strict");
  MEndToEndCampaignAnalysis Few;
  DisableDefaultStreams();
  Strict.Analyze("ETFake", "Too few", Few);
  const bool Blocked = Strict.Install("ETFake", "Too few", Few);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "too few", "Fewer runs than the minimum are an error", Few.HasErrors()) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::Install()", "blocked", "The references of a campaign with an error are not installed", Blocked) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::Install()", "no file", "No file was written", MFile::Exists(References + "/strict/ETFake.references.tsv")) && Passed;

  // A test whose runs failed: the failed checks are errors
  Campaign.SetNumberOfRuns(2);
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETFailing" }));
  MEndToEndCampaignAnalysis Failing;
  Campaign.Analyze("ETFailing", "Failing", Failing);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "failing", "Runs with failed checks are an error", Failing.HasErrors()) && Passed;

  // A test which is the fake test with a known failure: enough good runs, but the known failures are errors too, and its references are not installed
  Campaign.SetNumberOfRuns(6);
  DisableDefaultStreams();
  Campaign.RunJobs(Campaign.GetJobs({ "ETKnown" }));
  MEndToEndCampaignAnalysis Known;
  Campaign.Analyze("ETKnown", "Known", Known);
  const bool KnownInstalled = Campaign.Install("ETKnown", "Known", Known);
  EnableDefaultStreams();
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "known runs", "All six runs of the test with a known failure are analyzed", Known.GetNumberOfRuns(), 6U) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "known", "Runs with known failures are an error", Known.HasErrors()) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::Install()", "known", "The references of a test with known failures are not installed", KnownInstalled) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::Install()", "known no file", "No references of the test with a known failure are written", MFile::Exists(References + "/ETKnown.references.tsv")) && Passed;

  // A key result which is malformed in every run of a test is an error, not an observable which is silently missing
  Campaign.SetNumberOfRuns(6);
  MEndToEndCampaignAnalysis Intact;
  DisableDefaultStreams();
  Campaign.Analyze("ETFake", "Intact", Intact);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::Analyze()", "intact", "The six runs of the fake test have no error before one key result is malformed", Intact.HasErrors()) && Passed;
  for (unsigned int r = 0; r < 6; ++r) {
    MString Run = "00";
    Run += r;
    ofstream Out((Analysis + "/ETFake/run_" + Run + "/ETFake.keyresults.tsv").Data(), ios::app);
    Out<<"Fake/nan\tnan\t0.1\t1\t0\t0\t0\t0\t1\t0\t1\t1"<<endl;
  }
  MEndToEndCampaignAnalysis Corrupted;
  DisableDefaultStreams();
  Campaign.Analyze("ETFake", "Corrupted", Corrupted);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "malformed", "A malformed key result in every run is an error", Corrupted.HasErrors()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestRecordFailure()
{
  bool Passed = true;

  // The owner of a directory without write permission can still write to it if the user is root
  if (geteuid() == 0) {
    mout<<"UTEndToEndCampaign: the record failure is not tested as root"<<endl;
    return true;
  }

  const MString Bin = GetTemporaryDirectoryName("recordbin");
  const MString Analysis = GetTemporaryDirectoryName("recordcampaign");
  const MString Counter = GetTemporaryDirectoryName("recordcounter") + ".txt";
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, Counter)) && Passed;

  // The second of three runs ends in a directory in which its record cannot be written: the other runs are fine, the campaign is not
  MEndToEndCampaign Campaign(Bin, Analysis);
  Campaign.SetNumberOfRuns(3);
  Campaign.SetMinimumRuns(1);
  Campaign.SetMaximumParallel(3);
  Campaign.SetTimeout(10.0);
  DisableDefaultStreams();
  const bool Ran = Campaign.RunJobs(Campaign.GetJobs({ "ETReadOnly" }));
  MEndToEndCampaignAnalysis Result;
  Campaign.Analyze("ETReadOnly", "Read only", Result);
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "record failed", "A record which cannot be written is a failure of the campaign", Ran) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "no record", "The run has no record", MFile::Exists(Campaign.GetRunDirectory("ETReadOnly", 1) + "/run.done")) && Passed;
  Passed = Evaluate("MEndToEndCampaign::Analyze()", "runs", "A run without a record is a run of the analysis", Result.GetNumberOfRuns(), 3U) && Passed;
  Passed = EvaluateTrue("MEndToEndCampaign::Analyze()", "errors", "A run without a record is an error", Result.HasErrors()) && Passed;

  // Make the directories writable again, otherwise they cannot be removed
  error_code Error;
  filesystem::permissions(Campaign.GetRunDirectory("ETReadOnly", 1).Data(), filesystem::perms::owner_write, filesystem::perm_options::add, Error);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndCampaign::TestLostRuns()
{
  bool Passed = true;

  const MString Bin = GetTemporaryDirectoryName("lostbin");
  const MString Analysis = GetTemporaryDirectoryName("lostcampaign");
  const MString Counter = GetTemporaryDirectoryName("lostcounter") + ".txt";
  Passed = EvaluateTrue("CreateTests()", "scripts", "The test programs can be written", CreateTests(Bin, Counter)) && Passed;

  // With SIGCHLD ignored the system reaps the runs itself: waiting for them fails although they are recorded as running, they are lost
  MEndToEndCampaign Campaign(Bin, Analysis);
  Campaign.SetNumberOfRuns(2);
  Campaign.SetMaximumParallel(2);
  Campaign.SetTimeout(10.0);
  void (*OldChild)(int) = signal(SIGCHLD, SIG_IGN);
  DisableDefaultStreams();
  const bool Ran = Campaign.RunJobs(Campaign.GetJobs({ "ETFake" }));
  EnableDefaultStreams();
  signal(SIGCHLD, OldChild);
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "lost runs", "Runs which cannot be waited for are a failure of the campaign", Ran) && Passed;
  Passed = EvaluateFalse("MEndToEndCampaign::RunJobs()", "no record", "A lost run has no record", MFile::Exists(Campaign.GetRunDirectory("ETFake", 0) + "/run.done")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("UTEndToEndCampaign", "unit test of the end-to-end campaign") == false) {
    return 1;
  }

  // ROOT runs without graphics - the report of the analysis is written without a display
  TApplication ROOT("ROOT", 0, 0);
  gROOT->SetBatch(true);

  UTEndToEndCampaign Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndCampaign.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
