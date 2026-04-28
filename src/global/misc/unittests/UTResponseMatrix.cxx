/*
 * UTResponseMatrix.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MResponseMatrix.h"
#include "MUnitTest.h"


class TestResponseMatrix : public MResponseMatrix
{
public:
  TestResponseMatrix() : MResponseMatrix(), ReadSpecificCalled(false), ReadSpecificResult(true), RecordedVersion(-1), RecordedType("") {}
  TestResponseMatrix(MString Name) : MResponseMatrix(Name), ReadSpecificCalled(false), ReadSpecificResult(true), RecordedVersion(-1), RecordedType("") {}
  virtual ~TestResponseMatrix() {}

  virtual bool Write(MString, bool = false) { return true; }
  virtual unsigned long GetNBins() const { return 7; }
  virtual float GetMaximum() const { return 4.5f; }
  virtual float GetMinimum() const { return -1.5f; }
  virtual double GetSum() const { return 10.5; }
  virtual MString GetStatistics() const { return "Representative statistics"; }

  void ExposeWriteHeader(ostringstream& Out) { WriteHeader(Out); }
  void SetReadSpecificResult(bool Result) { ReadSpecificResult = Result; }
  bool WasReadSpecificCalled() const { return ReadSpecificCalled; }
  int GetRecordedVersion() const { return RecordedVersion; }
  MString GetRecordedType() const { return RecordedType; }

protected:
  virtual bool ReadSpecific(MFileResponse&, const MString& Type, const int Version)
  {
    ReadSpecificCalled = true;
    RecordedType = Type;
    RecordedVersion = Version;
    return ReadSpecificResult;
  }

private:
  bool ReadSpecificCalled;
  bool ReadSpecificResult;
  int RecordedVersion;
  MString RecordedType;
};


//! Unit test class for MResponseMatrix
class UTResponseMatrix : public MUnitTest
{
public:
  UTResponseMatrix() : MUnitTest("UTResponseMatrix") {}
  virtual ~UTResponseMatrix() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrix::Run()
{
  bool Passed = true;

  auto SilenceOutput = []() {
    int SavedStdout = dup(STDOUT_FILENO);
    int SavedStderr = dup(STDERR_FILENO);
    int DevNull = open("/dev/null", O_WRONLY);
    if (DevNull >= 0) {
      dup2(DevNull, STDOUT_FILENO);
      dup2(DevNull, STDERR_FILENO);
      close(DevNull);
    }
    return pair<int, int>(SavedStdout, SavedStderr);
  };
  auto RestoreOutput = [](pair<int, int> Saved) {
    if (Saved.first >= 0) {
      dup2(Saved.first, STDOUT_FILENO);
      close(Saved.first);
    }
    if (Saved.second >= 0) {
      dup2(Saved.second, STDERR_FILENO);
      close(Saved.second);
    }
  };

  TestResponseMatrix Default;
  Passed = Evaluate("GetName()", "default constructor", "The default response matrix name is set correctly", Default.GetName(), MString("Unnamed response matrix")) && Passed;
  Passed = Evaluate("GetOrder()", "default constructor", "The default response matrix order starts at zero", Default.GetOrder(), 0U) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "default constructor", "The default response matrix starts with zero simulated events", Default.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetFarFieldStartArea()", "default constructor", "The default response matrix starts with zero far-field start area", Default.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetSpectralType()", "default constructor", "The default response matrix starts with an empty spectral type", Default.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("GetSpectralParameters()", "default constructor", "The default response matrix starts with no spectral parameters", Default.GetSpectralParameters().size(), 0UL) && Passed;
  Passed = Evaluate("GetHash()", "default constructor", "The default response matrix starts with hash zero", Default.GetHash(), 0UL) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The representative derived response matrix reports its representative number of bins", Default.GetNBins(), 7UL) && Passed;
  Passed = EvaluateNear("GetMaximum()", "default constructor", "The representative derived response matrix reports its representative maximum", Default.GetMaximum(), 4.5, 1e-6) && Passed;
  Passed = EvaluateNear("GetMinimum()", "default constructor", "The representative derived response matrix reports its representative minimum", Default.GetMinimum(), -1.5, 1e-6) && Passed;
  Passed = EvaluateNear("GetSum()", "default constructor", "The representative derived response matrix reports its representative sum", Default.GetSum(), 10.5, 1e-12) && Passed;
  Passed = Evaluate("GetStatistics()", "default constructor", "The representative derived response matrix reports its representative statistics string", Default.GetStatistics(), MString("Representative statistics")) && Passed;

  TestResponseMatrix Named("Representative");
  Passed = Evaluate("GetName()", "named constructor", "The named response matrix constructor stores the representative name", Named.GetName(), MString("Representative")) && Passed;

  Named.SetName("Updated");
  Named.SetHash(12345UL);
  Named.SetSimulatedEvents(42);
  Named.SetFarFieldStartArea(17.5);
  Named.SetSpectrum("PowerLaw", vector<double>{1.0, 2.0, 3.0});
  Passed = Evaluate("SetName()", "representative update", "SetName updates the representative matrix name", Named.GetName(), MString("Updated")) && Passed;
  Passed = Evaluate("SetHash()", "representative update", "SetHash updates the representative matrix hash", Named.GetHash(), 12345UL) && Passed;
  Passed = Evaluate("SetSimulatedEvents()", "representative update", "SetSimulatedEvents updates the representative simulated-event count", Named.GetSimulatedEvents(), 42L) && Passed;
  Passed = EvaluateNear("SetFarFieldStartArea()", "representative update", "SetFarFieldStartArea updates the representative far-field area", Named.GetFarFieldStartArea(), 17.5, 1e-12) && Passed;
  Passed = Evaluate("SetSpectrum()", "representative update", "SetSpectrum stores the representative spectral type", Named.GetSpectralType(), MString("PowerLaw")) && Passed;
  Passed = Evaluate("SetSpectrum()", "representative update parameter count", "SetSpectrum stores the representative number of spectral parameters", Named.GetSpectralParameters().size(), 3UL) && Passed;
  Passed = EvaluateNear("SetSpectrum()", "representative update last parameter", "SetSpectrum stores the representative last spectral parameter", Named.GetSpectralParameters()[2], 3.0, 1e-12) && Passed;

  ostringstream Header;
  Named.ExposeWriteHeader(Header);
  Passed = Evaluate("WriteHeader()", "representative header", "WriteHeader serializes the representative base response header deterministically", MString(Header.str()), MString("# Response Matrix 0\nVersion 1\n\n# Name\nNM Updated\n\n# The order of the matrix\nOD 0\n\n# The number of simulated events\nTS 42\n\n# The far-field start area (if zero a non-far-field simulation, or non-spherical start area was used)\nSA 17.5\n\n# The spectral parameters (empty if not set)\nSM PowerLaw 1 2 3\n\n")) && Passed;

  Named.Clear();
  Passed = Evaluate("Clear()", "representative state", "Clear resets the representative matrix name", Named.GetName(), MString("Unnamed response matrix")) && Passed;
  Passed = Evaluate("Clear()", "representative state order", "Clear resets the representative matrix order", Named.GetOrder(), 0U) && Passed;
  Passed = Evaluate("Clear()", "representative state simulated events", "Clear resets the representative simulated-event count", Named.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("Clear()", "representative state area", "Clear resets the representative far-field area", Named.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("Clear()", "representative state spectrum", "Clear resets the representative spectral type", Named.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "representative state spectral parameters", "Clear removes representative spectral parameters", Named.GetSpectralParameters().size(), 0UL) && Passed;
  Passed = Evaluate("Clear()", "representative state hash", "Clear resets the representative hash", Named.GetHash(), 0UL) && Passed;

  MString TempFile = "/tmp/UTResponseMatrix/UTResponseMatrix_base.rsp";
  system("mkdir -p /tmp/UTResponseMatrix");
  {
    ofstream Out(TempFile.Data());
    Out<<"Version 1\n";
    Out<<"Type DummyResponse\n";
    Out<<"NM ReadBack\n";
    Out<<"TS 123\n";
    Out<<"SA 4.5\n";
    Out<<"SM Mono 511\n";
    Out<<"HA 999\n";
    Out<<"CE true\n";
  }

  TestResponseMatrix ReadBack;
  ReadBack.SetReadSpecificResult(true);
  Passed = Evaluate("Read()", "representative success", "Read delegates to ReadSpecific and returns success when the representative derived reader succeeds", ReadBack.Read(TempFile), true) && Passed;
  Passed = Evaluate("ReadSpecific()", "representative success", "Read invokes the representative derived ReadSpecific implementation", ReadBack.WasReadSpecificCalled(), true) && Passed;
  Passed = Evaluate("ReadSpecific()", "representative type", "Read forwards the representative file type to ReadSpecific", ReadBack.GetRecordedType(), MString("DummyResponse")) && Passed;
  Passed = Evaluate("ReadSpecific()", "representative version", "Read forwards the representative file version to ReadSpecific", ReadBack.GetRecordedVersion(), 1) && Passed;
  Passed = Evaluate("Read()", "representative name", "Read stores the representative matrix name from the file header", ReadBack.GetName(), MString("ReadBack")) && Passed;
  Passed = Evaluate("Read()", "representative hash", "Read stores the representative matrix hash from the file header", ReadBack.GetHash(), 999UL) && Passed;
  Passed = Evaluate("Read()", "representative simulated events", "Read stores the representative simulated-event count from the file header", ReadBack.GetSimulatedEvents(), 123L) && Passed;
  Passed = EvaluateNear("Read()", "representative area", "Read stores the representative far-field area from the file header", ReadBack.GetFarFieldStartArea(), 4.5, 1e-12) && Passed;
  Passed = Evaluate("Read()", "representative spectral type", "Read stores the representative spectral type from the file header", ReadBack.GetSpectralType(), MString("Mono")) && Passed;
  Passed = Evaluate("Read()", "representative spectral parameter count", "Read stores the representative number of spectral parameters from the file header", ReadBack.GetSpectralParameters().size(), 1UL) && Passed;
  Passed = EvaluateNear("Read()", "representative spectral parameter", "Read stores the representative spectral parameter from the file header", ReadBack.GetSpectralParameters()[0], 511.0, 1e-12) && Passed;

  TestResponseMatrix ReadFail;
  ReadFail.SetReadSpecificResult(false);
  Passed = Evaluate("Read()", "representative failure", "Read returns failure when the representative derived reader fails", ReadFail.Read(TempFile), false) && Passed;
  {
    pair<int, int> Saved = SilenceOutput();
    Passed = Evaluate("Read()", "missing file", "Read returns false for a representative missing response file", ReadFail.Read("/tmp/UTResponseMatrix/does_not_exist.rsp"), false) && Passed;
    RestoreOutput(Saved);
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTResponseMatrix Test;
  return Test.Run() == true ? 0 : 1;
}
