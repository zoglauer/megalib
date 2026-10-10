/*
 * UTResponseMatrix.cxx
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
#include <sstream>
using namespace std;

// MEGAlib:
#include "MResponseMatrix.h"
#include "MFile.h"
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
  virtual bool ReadSpecific(MFileResponse&, const MString& Type, const int Version, const bool) override
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

  TestResponseMatrix Default;
  Passed = Evaluate("GetName()", "default constructor", "The default response matrix name is set correctly", Default.GetName(), MString("Unnamed response matrix")) && Passed;
  Passed = Evaluate("GetOrder()", "default constructor", "The default response matrix order starts at zero", Default.GetOrder(), 0U) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "default constructor", "The default response matrix starts with zero simulated events", Default.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetFarFieldStartArea()", "default constructor", "The default response matrix starts with zero far-field start area", Default.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetSpectralType()", "default constructor", "The default response matrix starts with an empty spectral type", Default.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("GetHash()", "default constructor", "The default response matrix starts with hash zero", Default.GetHash(), 0UL) && Passed;
  Passed = Evaluate("GetNBins()", "default constructor", "The representative derived response matrix reports its representative number of bins", Default.GetNBins(), 7UL) && Passed;
  Passed = EvaluateNear("GetMaximum()", "default constructor", "The representative derived response matrix reports its representative maximum", Default.GetMaximum(), 4.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimum()", "default constructor", "The representative derived response matrix reports its representative minimum", Default.GetMinimum(), -1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetSum()", "default constructor", "The representative derived response matrix reports its representative sum", Default.GetSum(), 10.5, 1e-12) && Passed;
  Passed = Evaluate("GetStatistics()", "default constructor", "The representative derived response matrix reports its representative statistics string", Default.GetStatistics(), MString("Representative statistics")) && Passed;

  TestResponseMatrix Named("Representative");
  Passed = Evaluate("GetName()", "named constructor", "The named response matrix constructor stores the representative name", Named.GetName(), MString("Representative")) && Passed;

  Named.SetName("Updated");
  Named.SetHash(12345UL);
  Named.SetSimulatedEvents(42);
  Named.SetFarFieldStartArea(17.5);
  Named.SetSpectralType("PowerLaw 1 2 3");
  Named.SetBeamType("FarFieldPointSource 0 0");
  Named.SetPolarizationMode("relativez");
  Passed = Evaluate("SetName()", "representative update", "SetName updates the representative matrix name", Named.GetName(), MString("Updated")) && Passed;
  Passed = Evaluate("SetHash()", "representative update", "SetHash updates the representative matrix hash", Named.GetHash(), 12345UL) && Passed;
  Passed = Evaluate("SetSimulatedEvents()", "representative update", "SetSimulatedEvents updates the representative simulated-event count", Named.GetSimulatedEvents(), 42L) && Passed;
  Passed = EvaluateNear("SetFarFieldStartArea()", "representative update", "SetFarFieldStartArea updates the representative far-field area", Named.GetFarFieldStartArea(), 17.5, 1e-12) && Passed;
  Passed = Evaluate("SetSpectralType()", "representative update", "SetSpectralType stores the representative spectral type including its parameters", Named.GetSpectralType(), MString("PowerLaw 1 2 3")) && Passed;
  Passed = Evaluate("SetBeamType()", "representative update", "SetBeamType stores the representative beam type", Named.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
  Passed = Evaluate("SetPolarizationMode()", "representative update", "SetPolarizationMode stores the representative polarization mode", Named.GetPolarizationMode(), MString("relativez")) && Passed;

  ostringstream Header;
  Named.ExposeWriteHeader(Header);
  Passed = Evaluate("WriteHeader()", "representative header", "WriteHeader serializes the representative base response header deterministically", MString(Header.str()), MString("# Response Matrix 0\nVersion 1\n\n# Name\nNM Updated\n\n# The order of the matrix\nOD 0\n\n# The number of simulated events\nTS 42\n\n# The far-field start area (if zero a non-far-field simulation, or non-spherical start area was used)\nSA 17.5\n\n# The spectral parameters (empty if not set)\nSP PowerLaw 1 2 3\n\n# The beam parameters (empty if not set)\nBE FarFieldPointSource 0 0\n\n# The polarization mode (empty if not set)\nPO relativez\n\n\n")) && Passed;

  Named.Clear();
  Passed = Evaluate("Clear()", "representative state", "Clear resets the representative matrix name", Named.GetName(), MString("Unnamed response matrix")) && Passed;
  Passed = Evaluate("Clear()", "representative state order", "Clear resets the representative matrix order", Named.GetOrder(), 0U) && Passed;
  Passed = Evaluate("Clear()", "representative state simulated events", "Clear resets the representative simulated-event count", Named.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("Clear()", "representative state area", "Clear resets the representative far-field area", Named.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("Clear()", "representative state spectrum", "Clear resets the representative spectral type", Named.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "representative state hash", "Clear resets the representative hash", Named.GetHash(), 0UL) && Passed;

  // Beam type and polarization mode are part of the copy, the assignment, and Clear()
  {
    TestResponseMatrix Source("Source");
    Source.SetHash(7UL);
    Source.SetSpectralType("Mono 511");
    Source.SetBeamType("FarFieldPointSource 0 0");
    Source.SetPolarizationMode("relativey");

    TestResponseMatrix Copied(Source);
    Passed = Evaluate("CopyConstructor()", "spectral type", "The copy constructor copies the spectral type", Copied.GetSpectralType(), MString("Mono 511")) && Passed;
    Passed = Evaluate("CopyConstructor()", "beam type", "The copy constructor copies the beam type", Copied.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
    Passed = Evaluate("CopyConstructor()", "polarization mode", "The copy constructor copies the polarization mode", Copied.GetPolarizationMode(), MString("relativey")) && Passed;

    TestResponseMatrix Assigned("Assigned");
    Assigned = Source;
    Passed = Evaluate("operator=", "spectral type", "The assignment operator copies the spectral type", Assigned.GetSpectralType(), MString("Mono 511")) && Passed;
    Passed = Evaluate("operator=", "beam type", "The assignment operator copies the beam type", Assigned.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
    Passed = Evaluate("operator=", "polarization mode", "The assignment operator copies the polarization mode", Assigned.GetPolarizationMode(), MString("relativey")) && Passed;

    Source.Clear();
    Passed = Evaluate("Clear()", "beam type", "Clear resets the beam type", Source.GetBeamType(), MString("")) && Passed;
    Passed = Evaluate("Clear()", "polarization mode", "Clear resets the polarization mode", Source.GetPolarizationMode(), MString("")) && Passed;
  }

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "temporary directory", "The temporary directory for MResponseMatrix fixtures can be created", PrepareTemporaryDirectory()) && Passed;
  MString TempFile = GetTemporaryFileName("UTResponseMatrix_base.rsp");
  Passed = EvaluateTrue("WriteTextFile()", "representative response fixture", "The representative response-matrix fixture can be written",
                        WriteTextFile(TempFile,
                                      "Version 1\n"
                                      "Type DummyResponse\n"
                                      "NM ReadBack\n"
                                      "TS 123\n"
                                      "SA 4.5\n"
                                      "SP Mono 511\n"
                                      "BE FarFieldPointSource 0 0\n"
                                      "PO relativex\n"
                                      "HA 999\n"
                                      "CE true\n")) && Passed;

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
  Passed = Evaluate("Read()", "representative spectral type", "Read stores the representative spectral type from the file header", ReadBack.GetSpectralType(), MString("Mono 511")) && Passed;
  Passed = Evaluate("Read()", "representative beam type", "Read stores the representative beam type from the file header", ReadBack.GetBeamType(), MString("FarFieldPointSource 0 0")) && Passed;
  Passed = Evaluate("Read()", "representative polarization mode", "Read stores the representative polarization mode from the file header", ReadBack.GetPolarizationMode(), MString("relativex")) && Passed;

  TestResponseMatrix ReadFail;
  ReadFail.SetReadSpecificResult(false);
  Passed = Evaluate("Read()", "representative failure", "Read returns failure when the representative derived reader fails", ReadFail.Read(TempFile), false) && Passed;
  {
    DisableDefaultStreams();
    Passed = Evaluate("Read()", "missing file", "Read returns false for a representative missing response file", ReadFail.Read(GetTemporaryFileName("does_not_exist.rsp")), false) && Passed;
    EnableDefaultStreams();
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
