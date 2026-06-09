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
#include <atomic>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <thread>
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
    MString TemporaryFile(const MString& Name) const { return GetTemporaryFileName(Name); }
    MString TemporaryDirectory(const MString& Name = "") const { return GetTemporaryDirectoryName(Name); }
    bool PrepareDirectory(const MString& Name = "") const { return PrepareTemporaryDirectory(Name); }
    bool RemoveFile(const MString& FileName) const { return RemoveTemporaryFile(FileName); }
    bool RemoveDirectory(const MString& DirectoryName = "") const { return RemoveTemporaryDirectory(DirectoryName); }
    bool WriteFile(const MString& FileName, const MString& Content) const { return WriteTextFile(FileName, Content); }
    MString ReadFile(const MString& FileName) const { return ReadTextFile(FileName); }
  };

  //! Test exact, boolean, size, and floating-point evaluation helpers
  bool TestEvaluateHelpers();
  //! Test exception evaluation helper
  bool TestExceptionHelper();
  //! Test file-comparison helper
  bool TestFileComparison();
  //! Test numerical line and file-comparison helpers
  bool TestNumericalFileComparison();
  //! Test randomized temporary roots and guarded cleanup helpers
  bool TestTemporaryPaths();
};


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::Run()
{
  bool Passed = true;

  Passed = TestEvaluateHelpers() && Passed;
  Passed = TestExceptionHelper() && Passed;
  Passed = TestFileComparison() && Passed;
  Passed = TestNumericalFileComparison() && Passed;
  Passed = TestTemporaryPaths() && Passed;

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
  const MString WhitespaceFile = GetTemporaryFileName("whitespace.txt");
  const MString CrlfFile = GetTemporaryFileName("crlf.txt");
  const MString NoFinalNewlineFile = GetTemporaryFileName("no_final_newline.txt");
  const MString ShortFile = GetTemporaryFileName("short.txt");
  const MString LongFile = GetTemporaryFileName("long.txt");
  const MString EmptyReferenceFile = GetTemporaryFileName("empty_reference.txt");
  const MString EmptyTestFile = GetTemporaryFileName("empty_test.txt");
  const MString MissingGeneratedFile = GetTemporaryFileName("missing_generated.txt");
  const MString MissingReferenceFile = GetTemporaryFileName("missing_reference.txt");
  const MString TemporaryDirectory = GetTemporaryDirectoryName("file_comparison");

  RemoveTemporaryFile(ReferenceFile);
  RemoveTemporaryFile(MatchingFile);
  RemoveTemporaryFile(DifferentFile);
  RemoveTemporaryFile(WhitespaceFile);
  RemoveTemporaryFile(CrlfFile);
  RemoveTemporaryFile(NoFinalNewlineFile);
  RemoveTemporaryFile(ShortFile);
  RemoveTemporaryFile(LongFile);
  RemoveTemporaryFile(EmptyReferenceFile);
  RemoveTemporaryFile(EmptyTestFile);
  RemoveTemporaryFile(MissingGeneratedFile);
  RemoveTemporaryFile(MissingReferenceFile);

  Passed = EvaluateTrue("WriteTextFile()", "reference fixture", "The representative reference file can be written",
                        WriteTextFile(ReferenceFile, "alpha\nbeta\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "matching fixture", "The representative matching file can be written",
                        WriteTextFile(MatchingFile, "alpha\nbeta\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "different fixture", "The representative different file can be written",
                        WriteTextFile(DifferentFile, "alpha\ngamma\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "whitespace fixture", "The representative whitespace-different file can be written",
                        WriteTextFile(WhitespaceFile, "alpha \nbeta\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "CRLF fixture", "The representative CRLF file can be written",
                        WriteTextFile(CrlfFile, "alpha\r\nbeta\r\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "no-final-newline fixture", "The representative file without a final newline can be written",
                        WriteTextFile(NoFinalNewlineFile, "alpha\nbeta")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "short fixture", "The representative short file can be written",
                        WriteTextFile(ShortFile, "alpha\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "long fixture", "The representative long file can be written",
                        WriteTextFile(LongFile, "alpha\nbeta\ngamma\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "empty reference fixture", "The empty reference file can be written",
                        WriteTextFile(EmptyReferenceFile, "")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "empty test fixture", "The empty test file can be written",
                        WriteTextFile(EmptyTestFile, "")) && Passed;
  Passed = EvaluateTrue("ReadTextFile()", "reference fixture", "The unit-test helper can read back a representative fixture file",
                        ReadTextFile(ReferenceFile) == "alpha\nbeta\n") && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "scratch directory", "The unit-test helper can create a clean temporary directory",
                        PrepareTemporaryDirectory("file_comparison")) && Passed;
  Passed = EvaluateTrue("std::filesystem::is_directory()", "scratch directory", "The generated temporary directory exists after preparation",
                        std::filesystem::is_directory(TemporaryDirectory.Data())) && Passed;
  const MString NestedFixture = TemporaryDirectory + "/nested.txt";
  Passed = EvaluateTrue("WriteTextFile()", "nested scratch fixture", "The unit-test helper can write a representative fixture into a prepared nested directory",
                        WriteTextFile(NestedFixture, "nested\n")) && Passed;
  Passed = EvaluateTrue("ReadTextFile()", "nested scratch fixture", "The unit-test helper can read back a representative fixture from a prepared nested directory",
                        ReadTextFile(NestedFixture) == "nested\n") && Passed;
  UnitTestProbe Probe("Probe");
  Passed = EvaluateTrue("EvaluateFilesIdentical()", "matching files", "EvaluateFilesIdentical accepts representative identical files",
                        Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "matching files", "The files match", MatchingFile, ReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesIdentical()", "empty files", "EvaluateFilesIdentical accepts two empty files",
                        Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "empty files", "The empty files match", EmptyTestFile, EmptyReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesIdentical()", "LF and CRLF files", "EvaluateFilesIdentical consumes LF and CRLF line-ending characters",
                        Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "LF and CRLF files", "The files contain the same lines", CrlfFile, ReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesIdentical()", "optional final newline", "EvaluateFilesIdentical accepts files differing only by a final line-ending character",
                        Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "optional final newline", "The files contain the same lines", NoFinalNewlineFile, ReferenceFile)) && Passed;

  Probe.Silence();
  const bool DifferentFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "different files", "Different files are rejected", DifferentFile, ReferenceFile);
  const bool WhitespaceFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "whitespace difference", "Whitespace differences are rejected", WhitespaceFile, ReferenceFile);
  const bool ShortFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "short file", "Short files are rejected", ShortFile, ReferenceFile);
  const bool LongFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "long file", "Long files are rejected", LongFile, ReferenceFile);
  const bool MissingGeneratedFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "missing generated", "Missing generated files are rejected", MissingGeneratedFile, ReferenceFile);
  const bool MissingReferenceFailure = Probe.EvaluateFilesIdentical("inner EvaluateFilesIdentical()", "missing reference", "Missing reference files are rejected", MatchingFile, MissingReferenceFile);
  Probe.Unsilence();

  Passed = EvaluateFalse("EvaluateFilesIdentical()", "different files", "EvaluateFilesIdentical returns false for representative different files", DifferentFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "whitespace difference", "EvaluateFilesIdentical returns false for a difference in trailing whitespace", WhitespaceFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "short file", "EvaluateFilesIdentical returns false when the representative generated file is shorter", ShortFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "long file", "EvaluateFilesIdentical returns false when the representative test file is longer", LongFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "missing generated", "EvaluateFilesIdentical returns false when the representative generated file is missing", MissingGeneratedFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesIdentical()", "missing reference", "EvaluateFilesIdentical returns false when the representative reference file is missing", MissingReferenceFailure) && Passed;

  Passed = EvaluateTrue("RemoveTemporaryFile()", "reference cleanup", "The representative reference file can be removed", RemoveTemporaryFile(ReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "matching cleanup", "The representative matching file can be removed", RemoveTemporaryFile(MatchingFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "different cleanup", "The representative different file can be removed", RemoveTemporaryFile(DifferentFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "whitespace cleanup", "The representative whitespace-different file can be removed", RemoveTemporaryFile(WhitespaceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "CRLF cleanup", "The representative CRLF file can be removed", RemoveTemporaryFile(CrlfFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "no-final-newline cleanup", "The representative file without a final newline can be removed", RemoveTemporaryFile(NoFinalNewlineFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "short cleanup", "The representative short file can be removed", RemoveTemporaryFile(ShortFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "long cleanup", "The representative long file can be removed", RemoveTemporaryFile(LongFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "empty reference cleanup", "The empty reference file can be removed", RemoveTemporaryFile(EmptyReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "empty test cleanup", "The empty test file can be removed", RemoveTemporaryFile(EmptyTestFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "scratch cleanup", "The prepared nested directory can be removed recursively", RemoveTemporaryDirectory(TemporaryDirectory)) && Passed;
  Passed = EvaluateFalse("std::filesystem::exists()", "scratch cleanup", "The prepared nested directory no longer exists after removal",
                         std::filesystem::exists(TemporaryDirectory.Data())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::TestNumericalFileComparison()
{
  bool Passed = true;

  UnitTestProbe Probe("NumericalProbe");
  const MString EmptyReferenceFile = GetTemporaryFileName("numerical_empty_reference.txt");
  const MString EmptyTestFile = GetTemporaryFileName("numerical_empty_test.txt");
  const MString ReferenceFile = GetTemporaryFileName("numerical_reference.txt");
  const MString MatchingFile = GetTemporaryFileName("numerical_matching.txt");
  const MString CrlfFile = GetTemporaryFileName("numerical_crlf.txt");
  const MString ExtremeExponentReferenceFile = GetTemporaryFileName("numerical_extreme_exponent_reference.txt");
  const MString ExtremeExponentFile = GetTemporaryFileName("numerical_extreme_exponent.txt");
  const MString LargeReferenceFile = GetTemporaryFileName("numerical_large_reference.txt");
  const MString LargeOppositeFile = GetTemporaryFileName("numerical_large_opposite.txt");
  const MString ZeroToleranceFile = GetTemporaryFileName("numerical_zero_tolerance.txt");
  const MString ZeroToleranceDifferentFile = GetTemporaryFileName("numerical_zero_tolerance_different.txt");
  const MString EmptyLineReferenceFile = GetTemporaryFileName("numerical_empty_line_reference.txt");
  const MString WhitespaceLineFile = GetTemporaryFileName("numerical_whitespace_line.txt");
  const MString CustomToleranceFile = GetTemporaryFileName("numerical_custom_tolerance.txt");
  const MString DifferentTokenFile = GetTemporaryFileName("numerical_different_token.txt");
  const MString DifferentIntegerFile = GetTemporaryFileName("numerical_different_integer.txt");
  const MString ExtraTokenFile = GetTemporaryFileName("numerical_extra_token.txt");
  const MString MissingTokenFile = GetTemporaryFileName("numerical_missing_token.txt");
  const MString OutsideToleranceFile = GetTemporaryFileName("numerical_outside_tolerance.txt");
  const MString ShortFile = GetTemporaryFileName("numerical_short.txt");
  const MString LongFile = GetTemporaryFileName("numerical_long.txt");
  const MString MissingTestFile = GetTemporaryFileName("numerical_missing_test.txt");
  const MString MissingReferenceFile = GetTemporaryFileName("numerical_missing_reference.txt");

  RemoveTemporaryFile(EmptyReferenceFile);
  RemoveTemporaryFile(EmptyTestFile);
  RemoveTemporaryFile(ReferenceFile);
  RemoveTemporaryFile(MatchingFile);
  RemoveTemporaryFile(CrlfFile);
  RemoveTemporaryFile(ExtremeExponentReferenceFile);
  RemoveTemporaryFile(ExtremeExponentFile);
  RemoveTemporaryFile(LargeReferenceFile);
  RemoveTemporaryFile(LargeOppositeFile);
  RemoveTemporaryFile(ZeroToleranceFile);
  RemoveTemporaryFile(ZeroToleranceDifferentFile);
  RemoveTemporaryFile(EmptyLineReferenceFile);
  RemoveTemporaryFile(WhitespaceLineFile);
  RemoveTemporaryFile(CustomToleranceFile);
  RemoveTemporaryFile(DifferentTokenFile);
  RemoveTemporaryFile(DifferentIntegerFile);
  RemoveTemporaryFile(ExtraTokenFile);
  RemoveTemporaryFile(MissingTokenFile);
  RemoveTemporaryFile(OutsideToleranceFile);
  RemoveTemporaryFile(ShortFile);
  RemoveTemporaryFile(LongFile);
  RemoveTemporaryFile(MissingTestFile);
  RemoveTemporaryFile(MissingReferenceFile);

  Passed = EvaluateTrue("WriteTextFile()", "empty numerical reference fixture", "The empty numerical reference fixture can be written",
                        WriteTextFile(EmptyReferenceFile, "")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "empty numerical test fixture", "The empty numerical test fixture can be written",
                        WriteTextFile(EmptyTestFile, "")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "numerical reference fixture", "The numerical reference fixture can be written",
                        WriteTextFile(ReferenceFile, "CT 0.518236 0.481764\nscale 1.234e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "numerically matching fixture", "The numerically matching fixture can be written",
                        WriteTextFile(MatchingFile, "CT\t0.518237  0.481763\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "numerical CRLF fixture", "The numerically matching CRLF fixture can be written",
                        WriteTextFile(CrlfFile, "CT\t0.518237  0.481763\r\nscale 1.235e-4\r\nbins 16\r\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "extreme-exponent reference fixture", "The extreme-exponent reference fixture can be written",
                        WriteTextFile(ExtremeExponentReferenceFile, "value 0.0\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "extreme-exponent fixture", "The extreme-exponent numerical fixture can be written",
                        WriteTextFile(ExtremeExponentFile, "value 0.0e-2147483648\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "large-value reference fixture", "The large-value reference fixture can be written",
                        WriteTextFile(LargeReferenceFile, "value 1e308\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "opposite-sign large-value fixture", "The opposite-sign large-value fixture can be written",
                        WriteTextFile(LargeOppositeFile, "value -1e308\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "zero-tolerance fixture", "The equal-value zero-tolerance fixture can be written",
                        WriteTextFile(ZeroToleranceFile, "CT 0.5182360 0.4817640\nscale 0.0001234\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "zero-tolerance difference fixture", "The unequal-value zero-tolerance fixture can be written",
                        WriteTextFile(ZeroToleranceDifferentFile, "CT 0.518237 0.481764\nscale 1.234e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "empty-line reference fixture", "The empty-line reference fixture can be written",
                        WriteTextFile(EmptyLineReferenceFile, "CT 0.518236 0.481764\n\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "whitespace-line fixture", "The whitespace-line fixture can be written",
                        WriteTextFile(WhitespaceLineFile, "CT 0.518237 0.481763\n\t   \nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "custom-tolerance fixture", "The custom-tolerance fixture can be written",
                        WriteTextFile(CustomToleranceFile, "CT 0.518239 0.481763\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "different-token fixture", "The changed-token fixture can be written",
                        WriteTextFile(DifferentTokenFile, "CH 0.518237 0.481763\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "different-integer fixture", "The changed-integer fixture can be written",
                        WriteTextFile(DifferentIntegerFile, "CT 0.518237 0.481763\nscale 1.235e-4\nbins 17\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "extra-token fixture", "The extra-token fixture can be written",
                        WriteTextFile(ExtraTokenFile, "CT 0.518237 0.481763 extra\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "missing-token fixture", "The missing-token fixture can be written",
                        WriteTextFile(MissingTokenFile, "CT 0.518237\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "outside-tolerance fixture", "The outside-tolerance fixture can be written",
                        WriteTextFile(OutsideToleranceFile, "CT 0.518240 0.481763\nscale 1.235e-4\nbins 16\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "short numerical fixture", "The short numerical fixture can be written",
                        WriteTextFile(ShortFile, "CT 0.518237 0.481763\nscale 1.235e-4\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "long numerical fixture", "The long numerical fixture can be written",
                        WriteTextFile(LongFile, "CT 0.518237 0.481763\nscale 1.235e-4\nbins 16\nextra\n")) && Passed;

  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "empty files", "EvaluateFilesNumericallyEquivalent accepts two empty files",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "empty files", "The empty files match", EmptyTestFile, EmptyReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "numerically matching files", "EvaluateFilesNumericallyEquivalent accepts representative files with matching numeric tokens",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "numerically matching files", "The files match numerically", MatchingFile, ReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "LF and CRLF files", "EvaluateFilesNumericallyEquivalent accepts matching files with LF and CRLF line endings",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "LF and CRLF files", "The files match numerically", CrlfFile, ReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "extreme exponent", "EvaluateFilesNumericallyEquivalent handles fractional zero at the minimum integer exponent",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "extreme exponent", "The zero values match numerically", ExtremeExponentFile, ExtremeExponentReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "equal values at zero tolerance", "EvaluateFilesNumericallyEquivalent accepts equal numeric values with different representations at zero tolerance",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "equal values at zero tolerance", "The files have equal numeric values", ZeroToleranceFile, ReferenceFile, 0)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "whitespace-only line", "EvaluateFilesNumericallyEquivalent treats whitespace-only lines as equivalent",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "whitespace-only line", "The whitespace-only line matches an empty line", WhitespaceLineFile, EmptyLineReferenceFile)) && Passed;
  Passed = EvaluateTrue("EvaluateFilesNumericallyEquivalent()", "custom tolerance of three units", "EvaluateFilesNumericallyEquivalent honors a custom last-printed-digit tolerance",
                        Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "custom tolerance of three units", "The files match at the custom tolerance", CustomToleranceFile, ReferenceFile, 3)) && Passed;

  Probe.Silence();
  const bool DefaultToleranceFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "default tolerance", "The default tolerance rejects a three-unit difference", CustomToleranceFile, ReferenceFile);
  const bool ZeroToleranceFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "unequal values at zero tolerance", "Zero tolerance rejects unequal numeric values", ZeroToleranceDifferentFile, ReferenceFile, 0);
  const bool LargeOppositeFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "large opposite-sign values", "Large opposite-sign values are rejected", LargeOppositeFile, LargeReferenceFile);
  const bool DifferentTokenFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "different text token", "Changed text tokens are rejected", DifferentTokenFile, ReferenceFile);
  const bool DifferentIntegerFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "different integer token", "Changed integer tokens are rejected", DifferentIntegerFile, ReferenceFile);
  const bool ExtraTokenFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "extra token", "Different token counts are rejected", ExtraTokenFile, ReferenceFile);
  const bool MissingTokenFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "missing token", "Different token counts are rejected in the opposite direction", MissingTokenFile, ReferenceFile);
  const bool OutsideToleranceFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "outside tolerance", "Numeric differences outside the tolerance are rejected", OutsideToleranceFile, ReferenceFile);
  const bool ShortFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "short file", "Short files are rejected", ShortFile, ReferenceFile);
  const bool LongFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "long file", "Long files are rejected", LongFile, ReferenceFile);
  const bool MissingTestFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "missing test file", "Missing test files are rejected", MissingTestFile, ReferenceFile);
  const bool MissingReferenceFailure = Probe.EvaluateFilesNumericallyEquivalent("inner EvaluateFilesNumericallyEquivalent()", "missing reference file", "Missing reference files are rejected", MatchingFile, MissingReferenceFile);
  Probe.Unsilence();

  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "default tolerance", "EvaluateFilesNumericallyEquivalent rejects a three-unit difference at the default tolerance", DefaultToleranceFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "unequal values at zero tolerance", "EvaluateFilesNumericallyEquivalent rejects unequal numeric values at zero tolerance", ZeroToleranceFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "large opposite-sign values", "EvaluateFilesNumericallyEquivalent rejects values whose difference overflows double", LargeOppositeFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "different text token", "EvaluateFilesNumericallyEquivalent returns false for a changed non-numeric token", DifferentTokenFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "different integer token", "EvaluateFilesNumericallyEquivalent returns false for a changed integer-only token", DifferentIntegerFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "extra token", "EvaluateFilesNumericallyEquivalent returns false for different token counts", ExtraTokenFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "missing token", "EvaluateFilesNumericallyEquivalent returns false when the test line has fewer tokens", MissingTokenFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "outside tolerance", "EvaluateFilesNumericallyEquivalent returns false for a numeric difference outside the tolerance", OutsideToleranceFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "short file", "EvaluateFilesNumericallyEquivalent returns false when the test file is shorter", ShortFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "long file", "EvaluateFilesNumericallyEquivalent returns false when the test file is longer", LongFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "missing test file", "EvaluateFilesNumericallyEquivalent returns false when the test file is missing", MissingTestFailure) && Passed;
  Passed = EvaluateFalse("EvaluateFilesNumericallyEquivalent()", "missing reference file", "EvaluateFilesNumericallyEquivalent returns false when the reference file is missing", MissingReferenceFailure) && Passed;

  Passed = EvaluateTrue("RemoveTemporaryFile()", "empty numerical reference cleanup", "The empty numerical reference fixture can be removed", RemoveTemporaryFile(EmptyReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "empty numerical test cleanup", "The empty numerical test fixture can be removed", RemoveTemporaryFile(EmptyTestFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "numerical reference cleanup", "The numerical reference fixture can be removed", RemoveTemporaryFile(ReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "numerically matching cleanup", "The numerically matching fixture can be removed", RemoveTemporaryFile(MatchingFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "numerical CRLF cleanup", "The numerically matching CRLF fixture can be removed", RemoveTemporaryFile(CrlfFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "extreme-exponent reference cleanup", "The extreme-exponent reference fixture can be removed", RemoveTemporaryFile(ExtremeExponentReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "extreme-exponent cleanup", "The extreme-exponent numerical fixture can be removed", RemoveTemporaryFile(ExtremeExponentFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "large-value reference cleanup", "The large-value reference fixture can be removed", RemoveTemporaryFile(LargeReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "opposite-sign large-value cleanup", "The opposite-sign large-value fixture can be removed", RemoveTemporaryFile(LargeOppositeFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "zero-tolerance cleanup", "The equal-value zero-tolerance fixture can be removed", RemoveTemporaryFile(ZeroToleranceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "zero-tolerance difference cleanup", "The unequal-value zero-tolerance fixture can be removed", RemoveTemporaryFile(ZeroToleranceDifferentFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "empty-line reference cleanup", "The empty-line reference fixture can be removed", RemoveTemporaryFile(EmptyLineReferenceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "whitespace-line cleanup", "The whitespace-line fixture can be removed", RemoveTemporaryFile(WhitespaceLineFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "custom-tolerance cleanup", "The custom-tolerance fixture can be removed", RemoveTemporaryFile(CustomToleranceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "different-token cleanup", "The changed-token fixture can be removed", RemoveTemporaryFile(DifferentTokenFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "different-integer cleanup", "The changed-integer fixture can be removed", RemoveTemporaryFile(DifferentIntegerFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "extra-token cleanup", "The extra-token fixture can be removed", RemoveTemporaryFile(ExtraTokenFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "missing-token cleanup", "The missing-token fixture can be removed", RemoveTemporaryFile(MissingTokenFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "outside-tolerance cleanup", "The outside-tolerance fixture can be removed", RemoveTemporaryFile(OutsideToleranceFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "short numerical cleanup", "The short numerical fixture can be removed", RemoveTemporaryFile(ShortFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "long numerical cleanup", "The long numerical fixture can be removed", RemoveTemporaryFile(LongFile)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTUnitTest::TestTemporaryPaths()
{
  bool Passed = true;

  DisableDefaultStreams();
  UnitTestProbe EmptyNameProbe("");
  UnitTestProbe TraversalNameProbe("../TraversalProbe");
  UnitTestProbe WhitespaceNameProbe("Whitespace Probe");
  UnitTestProbe SpecialCharacterNameProbe("Special@Probe");
  EnableDefaultStreams();
  const MString EmptyNameRoot = EmptyNameProbe.TemporaryDirectory();
  const MString TraversalNameRoot = TraversalNameProbe.TemporaryDirectory();
  const MString WhitespaceNameRoot = WhitespaceNameProbe.TemporaryDirectory();
  const MString SpecialCharacterNameRoot = SpecialCharacterNameProbe.TemporaryDirectory();
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "empty test name", "An empty unit-test name uses the safe fallback temporary basename", EmptyNameRoot.Contains("_" + c_FallbackTemporaryBaseName)) && Passed;
  Passed = Evaluate("MUnitTest()", "traversal test name", "An unsafe traversal-style unit-test name remains unchanged for reporting", TraversalNameProbe.GetProbeName(), MString("../TraversalProbe")) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "traversal test name", "An unsafe traversal-style unit-test name uses the safe fallback temporary basename", TraversalNameRoot.Contains("_" + c_FallbackTemporaryBaseName)) && Passed;
  Passed = Evaluate("MUnitTest()", "whitespace test name", "An unsafe whitespace-containing unit-test name remains unchanged for reporting", WhitespaceNameProbe.GetProbeName(), MString("Whitespace Probe")) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "whitespace test name", "An unsafe whitespace-containing unit-test name uses the safe fallback temporary basename", WhitespaceNameRoot.Contains("_" + c_FallbackTemporaryBaseName)) && Passed;
  Passed = Evaluate("MUnitTest()", "special-character test name", "An unsafe special-character unit-test name remains unchanged for reporting", SpecialCharacterNameProbe.GetProbeName(), MString("Special@Probe")) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "special-character test name", "An unsafe special-character unit-test name uses the safe fallback temporary basename", SpecialCharacterNameRoot.Contains("_" + c_FallbackTemporaryBaseName)) && Passed;
  Passed = EvaluateFalse("GetTemporaryDirectoryName()", "fallback distinctness", "Probes with different unsafe names but the same fallback basename still receive distinct randomized roots", TraversalNameRoot == WhitespaceNameRoot) && Passed;
  Passed = EvaluateFalse("GetTemporaryDirectoryName()", "fallback distinctness", "Three probes with different unsafe names but the same fallback basename receive three distinct randomized roots", WhitespaceNameRoot == SpecialCharacterNameRoot) && Passed;

  UnitTestProbe First("FirstProbe");
  UnitTestProbe Second("SecondProbe");
  const MString FirstRoot = First.TemporaryDirectory();
  const MString SecondRoot = Second.TemporaryDirectory();

  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "first randomized root", "The first randomized temporary root can be created", FirstRoot.IsEmpty() == false) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "valid test name", "A valid unit-test name is used directly as the temporary basename", FirstRoot.Contains("_FirstProbe")) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "second randomized root", "The second randomized temporary root can be created", SecondRoot.IsEmpty() == false) && Passed;
  Passed = EvaluateFalse("GetTemporaryDirectoryName()", "distinct randomized roots", "Separate unit-test instances use distinct randomized temporary roots", FirstRoot == SecondRoot) && Passed;

  const MString FileName = First.TemporaryFile("representative.txt");
  Passed = EvaluateTrue("GetTemporaryFileName()", "representative file", "Generated file paths remain below the randomized private root", FileName.BeginsWith(FirstRoot + "/")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "representative file", "A representative temporary file can be written", First.WriteFile(FileName, "temporary\n")) && Passed;
  Passed = EvaluateTrue("ReadTextFile()", "representative file", "A representative temporary file can be read", First.ReadFile(FileName) == "temporary\n") && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "representative file", "A representative temporary file can be removed through the guarded helper", First.RemoveFile(FileName)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryFile()", "missing representative file", "Removing an already absent temporary file succeeds", First.RemoveFile(FileName)) && Passed;

  First.Silence();
  const MString InvalidTraversal = First.TemporaryFile("../outside");
  const MString InvalidDot = First.TemporaryDirectory(".");
  const MString InvalidDotDot = First.TemporaryDirectory("..");
  const MString InvalidSeparator = First.TemporaryDirectory("nested/directory");
  const MString InvalidNull = First.TemporaryFile(MString(std::string("nul\0suffix", 10)));
  First.Unsilence();
  Passed = EvaluateTrue("GetTemporaryFileName()", "traversal name", "Temporary file names containing parent traversal are rejected", InvalidTraversal.IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "dot name", "The current-directory path component is rejected", InvalidDot.IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "dot-dot name", "The parent-directory path component is rejected", InvalidDotDot.IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "separator name", "Temporary directory names containing path separators are rejected", InvalidSeparator.IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetTemporaryFileName()", "embedded NUL name", "Temporary file names containing an embedded NUL byte are rejected", InvalidNull.IsEmpty()) && Passed;

  const MString NestedDirectory = First.TemporaryDirectory("nested");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "nested directory", "A nested temporary directory can be prepared", First.PrepareDirectory("nested")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "nested file", "A nested representative fixture can be written", First.WriteFile(NestedDirectory + "/fixture.txt", "temporary\n")) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "existing nested directory", "Preparing an existing nested directory recreates it", First.PrepareDirectory("nested")) && Passed;
  Passed = EvaluateFalse("std::filesystem::exists()", "removed nested fixture", "Recreating a nested temporary directory removes its previous contents", std::filesystem::exists((NestedDirectory + "/fixture.txt").Data())) && Passed;
  Passed = EvaluateTrue("std::filesystem::is_directory()", "recreated nested directory", "Recreating a nested temporary directory leaves the directory available", std::filesystem::is_directory(NestedDirectory.Data())) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "nested directory", "A nested temporary directory can be removed recursively", First.RemoveDirectory(NestedDirectory)) && Passed;
  Passed = EvaluateFalse("std::filesystem::exists()", "nested directory", "The nested temporary directory no longer exists after removal", std::filesystem::exists(NestedDirectory.Data())) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "missing nested directory", "Removing an already absent temporary directory succeeds", First.RemoveDirectory(NestedDirectory)) && Passed;

  const MString SiblingDirectory = MFile::CreateTemporaryDirectory("UTUnitTestSibling");
  Passed = EvaluateTrue("MFile::CreateTemporaryDirectory()", "sibling directory", "A representative sibling directory can be created below the system temporary directory", SiblingDirectory.IsEmpty() == false) && Passed;
  const MString SiblingFile = SiblingDirectory + "/outside.txt";
  First.Silence();
  const bool WroteSibling = First.WriteFile(SiblingFile, "unsafe\n");
  const MString ReadSibling = First.ReadFile(SiblingFile);
  const bool RemovedSibling = First.RemoveDirectory(SiblingDirectory);
  const bool RemovedTraversal = First.RemoveDirectory(FirstRoot + "/../../home/andreas");
  First.Unsilence();
  Passed = EvaluateFalse("WriteTextFile()", "sibling file", "Writing outside the randomized private root is rejected", WroteSibling) && Passed;
  Passed = EvaluateTrue("ReadTextFile()", "sibling file", "Reading outside the randomized private root is rejected", ReadSibling.IsEmpty()) && Passed;
  Passed = EvaluateFalse("std::filesystem::exists()", "sibling file", "A rejected write does not create a sibling file", std::filesystem::exists(SiblingFile.Data())) && Passed;
  Passed = EvaluateFalse("RemoveTemporaryDirectory()", "sibling directory", "A sibling directory below the system temporary directory is rejected", RemovedSibling) && Passed;
  Passed = EvaluateFalse("RemoveTemporaryDirectory()", "traversal path", "A path escaping the randomized private root is rejected", RemovedTraversal) && Passed;

  const MString Symlink = FirstRoot + "/outside_link";
  std::error_code Error;
  std::filesystem::create_directory_symlink(SiblingDirectory.Data(), Symlink.Data(), Error);
  Passed = EvaluateTrue("std::filesystem::create_directory_symlink()", "outside symlink", "A representative symlink to a sibling temporary directory can be created", Error.value() == 0) && Passed;
  First.Silence();
  const bool WroteThroughSymlink = First.WriteFile(Symlink + "/outside.txt", "unsafe\n");
  const bool RemovedSymlinkTarget = First.RemoveDirectory(Symlink);
  First.Unsilence();
  Passed = EvaluateFalse("WriteTextFile()", "outside symlink", "Writing through a symlink that resolves outside the randomized private root is rejected", WroteThroughSymlink) && Passed;
  Passed = EvaluateFalse("RemoveTemporaryDirectory()", "outside symlink", "A symlink resolving outside the randomized private root is rejected", RemovedSymlinkTarget) && Passed;

  std::filesystem::remove(Symlink.Data(), Error);
  Passed = EvaluateTrue("std::filesystem::remove()", "sibling directory cleanup", "The representative sibling directory can be removed explicitly", std::filesystem::remove(SiblingDirectory.Data())) && Passed;
  First.Silence();
  const bool RemovedRootAsFile = First.RemoveFile(FirstRoot);
  First.Unsilence();
  Passed = EvaluateFalse("RemoveTemporaryFile()", "first randomized root", "The randomized private root cannot be removed through the file-removal helper", RemovedRootAsFile) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "first randomized root", "The first randomized private root can be removed during teardown without repeating its path", First.RemoveDirectory()) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "second randomized root", "The second randomized private root can be removed during teardown", Second.RemoveDirectory(SecondRoot)) && Passed;

  const MString RecreatedRoot = First.TemporaryDirectory();
  Passed = EvaluateFalse("GetTemporaryDirectoryName()", "recreated randomized root", "Requesting another path after teardown creates a fresh randomized private root", RecreatedRoot == FirstRoot) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "recreated randomized root", "The recreated randomized private root can be removed during teardown", First.RemoveDirectory(RecreatedRoot)) && Passed;

  MString DestructorRoot;
  {
    UnitTestProbe DestructorProbe("DestructorProbe");
    DestructorRoot = DestructorProbe.TemporaryDirectory();
  }
  Passed = EvaluateFalse("~MUnitTest()", "automatic root cleanup", "Destroying a unit-test instance removes its randomized private root",
                         std::filesystem::exists(DestructorRoot.Data())) && Passed;

  UnitTestProbe RootResetProbe("RootResetProbe");
  const MString RootResetDirectory = RootResetProbe.TemporaryDirectory();
  const MString RootResetFile = RootResetProbe.TemporaryFile("before_reset.txt");
  Passed = EvaluateTrue("WriteTextFile()", "root reset fixture", "A representative fixture can be written before resetting the randomized root", RootResetProbe.WriteFile(RootResetFile, "temporary\n")) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "randomized root", "Preparing the randomized root without a child name recreates the entire root", RootResetProbe.PrepareDirectory()) && Passed;
  Passed = EvaluateFalse("std::filesystem::exists()", "root reset fixture", "Resetting the randomized root removes its previous contents", std::filesystem::exists(RootResetFile.Data())) && Passed;
  Passed = EvaluateTrue("std::filesystem::is_directory()", "randomized root", "Resetting the randomized root leaves the root directory available", std::filesystem::is_directory(RootResetDirectory.Data())) && Passed;

  UnitTestProbe ValidationProbe("ValidationProbe");
  ValidationProbe.Silence();
  const MString EmptyFileName = ValidationProbe.TemporaryFile("");
  const MString InvalidBackslash = ValidationProbe.TemporaryDirectory("nested\\directory");
  // This is rejected by the empty-root guard before path resolution. The
  // representative literal path is intentionally irrelevant.
  const bool RemovedBeforeRootCreation = ValidationProbe.RemoveFile("/tmp/UTUnitTest_pre_root.txt");
  ValidationProbe.Unsilence();
  Passed = EvaluateTrue("GetTemporaryFileName()", "empty file name", "An empty temporary file name is rejected", EmptyFileName.IsEmpty()) && Passed;
  Passed = EvaluateTrue("GetTemporaryDirectoryName()", "backslash name", "Temporary directory names containing a backslash are rejected", InvalidBackslash.IsEmpty()) && Passed;
  Passed = EvaluateFalse("RemoveTemporaryFile()", "pre-root path", "Removing a temporary file before creating a randomized root is rejected", RemovedBeforeRootCreation) && Passed;

  UnitTestProbe ThreadProbe("ThreadProbe");
  std::vector<MString> ThreadPaths[2];
  std::atomic<unsigned int> ReadyThreads{0};
  std::atomic<bool> StartThreads{false};
  std::thread Threads[2];
  for (unsigned int t = 0; t < 2; ++t) {
    Threads[t] = std::thread([&ThreadProbe, &ThreadPaths, &ReadyThreads, &StartThreads, t]() {
      ++ReadyThreads;
      while (StartThreads.load() == false) {
        std::this_thread::yield();
      }
      for (unsigned int i = 0; i < 100; ++i) {
        ThreadPaths[t].push_back(ThreadProbe.TemporaryFile(MString("thread_") + t + "_" + i + ".txt"));
      }
    });
  }
  while (ReadyThreads.load() < 2) {
    std::this_thread::yield();
  }
  StartThreads.store(true);
  for (unsigned int t = 0; t < 2; ++t) {
    Threads[t].join();
  }
  const MString ThreadRoot = ThreadProbe.TemporaryDirectory();
  for (unsigned int t = 0; t < 2; ++t) {
    bool PathsUseSharedRoot = ThreadPaths[t].size() == 100;
    for (const MString& Path: ThreadPaths[t]) {
      PathsUseSharedRoot = Path.BeginsWith(ThreadRoot + "/") && PathsUseSharedRoot;
    }
    Passed = EvaluateTrue("GetTemporaryFileName()", MString("concurrent thread ") + t, "Every concurrently generated temporary path remains below the shared randomized root", PathsUseSharedRoot) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTUnitTest Test;
  return Test.Run() == true ? 0 : 1;
}
