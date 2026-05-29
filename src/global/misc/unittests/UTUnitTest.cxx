/*
 * UTUnitTest.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MFile.h"
#include "MUnitTest.h"

// Standard libs:
#include <filesystem>
#include <limits>
#include <stdexcept>
using namespace std;


//! Unit test class for MUnitTest
class UTUnitTest : public MUnitTest
{
public:
  //! Default constructor
  UTUnitTest() : MUnitTest("UTUnitTest") {}
  //! Default destructor
  virtual ~UTUnitTest() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test helper exposing protected MUnitTest functionality
  class UnitTestProbe : public MUnitTest
  {
  public:
    UnitTestProbe(const MString& Name) : MUnitTest(Name) {}
    virtual ~UnitTestProbe() {}

    virtual bool Run() { return true; }

    const MString& GetProbeName() const { return GetName(); }
    void Silence() { DisableDefaultStreams(); }
    void Unsilence() { EnableDefaultStreams(); }
  };

  //! Test exact, boolean, size, and floating-point evaluation helpers
  bool TestEvaluateHelpers();
  //! Test exception evaluation helper
  bool TestExceptionHelper();
  //! Test file-comparison helper
  bool TestFileComparison();
};


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::Run()
{
  bool Passed = true;

  Passed = TestEvaluateHelpers() && Passed;
  Passed = TestExceptionHelper() && Passed;
  Passed = TestFileComparison() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::TestEvaluateHelpers()
{
  bool Passed = true;

  UnitTestProbe Probe("Probe");
  Passed = Evaluate("MUnitTest()", "name", "The unit-test base stores the representative test name", Probe.GetProbeName(), MString("Probe")) && Passed;

  Passed = EvaluateTrue("Evaluate()", "matching integers", "Evaluate returns true when representative integer values match",
                        Probe.Evaluate("inner Evaluate()", "matching integers", "Matching values are accepted", 7, 7)) && Passed;

  Probe.Silence();
  const bool ExactFailure = Probe.Evaluate("inner Evaluate()", "different integers", "Different values are rejected", 7, 8);
  Probe.Unsilence();
  Passed = EvaluateFalse("Evaluate()", "different integers", "Evaluate returns false when representative integer values differ", ExactFailure) && Passed;

  Passed = EvaluateTrue("EvaluateTrue()", "true value", "EvaluateTrue returns true for a true representative boolean",
                        Probe.EvaluateTrue("inner EvaluateTrue()", "true value", "True values are accepted", true)) && Passed;
  Passed = EvaluateTrue("EvaluateFalse()", "false value", "EvaluateFalse returns true for a false representative boolean",
                        Probe.EvaluateFalse("inner EvaluateFalse()", "false value", "False values are accepted", false)) && Passed;

  Probe.Silence();
  const bool TrueFailure = Probe.EvaluateTrue("inner EvaluateTrue()", "false value", "False values are rejected", false);
  const bool FalseFailure = Probe.EvaluateFalse("inner EvaluateFalse()", "true value", "True values are rejected", true);
  Probe.Unsilence();
  Passed = EvaluateFalse("EvaluateTrue()", "false value", "EvaluateTrue returns false for a false representative boolean", TrueFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFalse()", "true value", "EvaluateFalse returns false for a true representative boolean", FalseFailure) && Passed;

  Passed = EvaluateTrue("EvaluateNear()", "inside tolerance", "EvaluateNear accepts representative values inside the tolerance",
                        Probe.EvaluateNear("inner EvaluateNear()", "inside tolerance", "Nearby values are accepted", 1.0005, 1.0, 0.001)) && Passed;

  Probe.Silence();
  const bool NearFailure = Probe.EvaluateNear("inner EvaluateNear()", "outside tolerance", "Distant values are rejected", 1.01, 1.0, 0.001);
  const bool NonFiniteFailure = Probe.EvaluateNear("inner EvaluateNear()", "non-finite", "Non-finite values are rejected", numeric_limits<double>::infinity(), 1.0, 0.001);
  Probe.Unsilence();
  Passed = EvaluateFalse("EvaluateNear()", "outside tolerance", "EvaluateNear returns false outside the representative tolerance", NearFailure) && Passed;
  Passed = EvaluateFalse("EvaluateNear()", "non-finite", "EvaluateNear returns false for a non-finite representative value", NonFiniteFailure) && Passed;

  vector<int> Values;
  Values.push_back(1);
  Values.push_back(2);
  Passed = EvaluateTrue("EvaluateSize()", "two values", "EvaluateSize accepts the representative container size",
                        Probe.EvaluateSize("inner EvaluateSize()", "two values", "The size matches", Values.size(), static_cast<size_t>(2))) && Passed;

  Probe.Silence();
  const bool SizeFailure = Probe.EvaluateSize("inner EvaluateSize()", "two values", "The size mismatch is rejected", Values.size(), static_cast<size_t>(3));
  Probe.Unsilence();
  Passed = EvaluateFalse("EvaluateSize()", "wrong size", "EvaluateSize returns false for a representative size mismatch", SizeFailure) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::TestExceptionHelper()
{
  bool Passed = true;

  UnitTestProbe Probe("Probe");
  Passed = EvaluateTrue("EvaluateException()", "matching exception", "EvaluateException accepts the representative expected exception type",
                        Probe.EvaluateException<runtime_error>("inner EvaluateException()", "matching exception", "The expected exception is thrown",
                                                               [](){ throw runtime_error("representative"); })) && Passed;

  Probe.Silence();
  const bool MissingException = Probe.EvaluateException<runtime_error>("inner EvaluateException()", "no exception", "Missing exceptions are rejected",
                                                                       [](){});
  const bool WrongException = Probe.EvaluateException<runtime_error>("inner EvaluateException()", "wrong exception", "Wrong exception types are rejected",
                                                                     [](){ throw logic_error("representative"); });
  Probe.Unsilence();

  Passed = EvaluateFalse("EvaluateException()", "no exception", "EvaluateException returns false when no representative exception is thrown", MissingException) && Passed;
  Passed = EvaluateFalse("EvaluateException()", "wrong exception", "EvaluateException returns false for a representative wrong exception type", WrongException) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::TestFileComparison()
{
  bool Passed = true;

  const MString ReferenceFile = GetTemporaryFileName("reference.txt");
  const MString MatchingFile = GetTemporaryFileName("matching.txt");
  const MString DifferentFile = GetTemporaryFileName("different.txt");
  const MString ShortFile = GetTemporaryFileName("short.txt");
  const MString MissingGeneratedFile = GetTemporaryFileName("missing_generated.txt");
  const MString MissingReferenceFile = GetTemporaryFileName("missing_reference.txt");
  const MString TemporaryDirectory = GetTemporaryDirectoryName("file_comparison");

  MFile::Remove(ReferenceFile);
  MFile::Remove(MatchingFile);
  MFile::Remove(DifferentFile);
  MFile::Remove(ShortFile);
  MFile::Remove(MissingGeneratedFile);
  MFile::Remove(MissingReferenceFile);

  Passed = EvaluateTrue("ofstream::is_open()", "reference fixture", "The representative reference file can be written",
                        WriteTextFile(ReferenceFile, "alpha\nbeta\n")) && Passed;
  Passed = EvaluateTrue("ofstream::is_open()", "matching fixture", "The representative matching file can be written",
                        WriteTextFile(MatchingFile, "alpha\nbeta\n")) && Passed;
  Passed = EvaluateTrue("ofstream::is_open()", "different fixture", "The representative different file can be written",
                        WriteTextFile(DifferentFile, "alpha\ngamma\n")) && Passed;
  Passed = EvaluateTrue("ofstream::is_open()", "short fixture", "The representative short file can be written",
                        WriteTextFile(ShortFile, "alpha\n")) && Passed;
  Passed = EvaluateTrue("ReadTextFile()", "reference fixture", "The unit-test helper can read back a representative fixture file",
                        ReadTextFile(ReferenceFile) == "alpha\nbeta\n") && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "scratch directory", "The unit-test helper can create a clean temporary directory",
                        PrepareTemporaryDirectory("file_comparison")) && Passed;
  Passed = EvaluateTrue("std::filesystem::is_directory()", "scratch directory", "The generated temporary directory exists after preparation",
                        std::filesystem::is_directory(TemporaryDirectory.Data())) && Passed;

  UnitTestProbe Probe("Probe");
  Passed = EvaluateTrue("EvaluateFilesIdentical()", "matching files", "EvaluateFilesIdentical accepts representative identical files",
                        Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "matching files", "The files match", MatchingFile, ReferenceFile)) && Passed;

  Probe.Silence();
  const bool DifferentFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "different files", "Different files are rejected", DifferentFile, ReferenceFile);
  const bool ShortFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "short file", "Short files are rejected", ShortFile, ReferenceFile);
  const bool MissingGeneratedFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "missing generated", "Missing generated files are rejected", MissingGeneratedFile, ReferenceFile);
  const bool MissingReferenceFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "missing reference", "Missing reference files are rejected", MatchingFile, MissingReferenceFile);
  Probe.Unsilence();

  Passed = EvaluateFalse("EvaluateFilesIdentical()", "different files", "EvaluateFilesIdentical returns false for representative different files", DifferentFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "short file", "EvaluateFilesIdentical returns false when the representative generated file is shorter", ShortFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "missing generated", "EvaluateFilesIdentical returns false when the representative generated file is missing", MissingGeneratedFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "missing reference", "EvaluateFilesIdentical returns false when the representative reference file is missing", MissingReferenceFailure) && Passed;

  Passed = EvaluateTrue("MFile::Remove()", "reference cleanup", "The representative reference file can be removed", MFile::Remove(ReferenceFile)) && Passed;
  Passed = EvaluateTrue("MFile::Remove()", "matching cleanup", "The representative matching file can be removed", MFile::Remove(MatchingFile)) && Passed;
  Passed = EvaluateTrue("MFile::Remove()", "different cleanup", "The representative different file can be removed", MFile::Remove(DifferentFile)) && Passed;
  Passed = EvaluateTrue("MFile::Remove()", "short cleanup", "The representative short file can be removed", MFile::Remove(ShortFile)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTUnitTest Test;
  return Test.Run() == true ? 0 : 1;
}
