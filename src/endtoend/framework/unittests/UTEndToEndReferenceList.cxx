/*
 * UTEndToEndReferenceList.cxx
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
#include <fstream>
using namespace std;

// MEGAlib libs:
#include "MEndToEndReferenceList.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndReferenceList
class UTEndToEndReferenceList : public MUnitTest
{
public:
  UTEndToEndReferenceList() : MUnitTest("UTEndToEndReferenceList") {}
  virtual ~UTEndToEndReferenceList() {}

  virtual bool Run();

private:
  bool TestReferences();
  bool TestFile();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReferenceList::Run()
{
  bool Passed = true;
  Passed = TestReferences() && Passed;
  Passed = TestFile() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReferenceList::TestReferences()
{
  bool Passed = true;

  MEndToEndReferenceList References;
  Passed = EvaluateFalse("MEndToEndReferenceList::HasReference()", "empty", "There is no reference in an empty set", References.HasReference("A/b")) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::GetReference()", "missing", "A missing reference has the mean zero", References.GetReference("A/b").GetMean(), 0.0, 0.0) && Passed;

  MEndToEndReference Reference("A/b");
  Reference.SetMean(1.5);
  Reference.SetError(0.25);
  Reference.SetScatter(0.875);
  Reference.SetRuns(12);
  References.SetReference(Reference);
  Passed = EvaluateTrue("MEndToEndReferenceList::HasReference()", "set", "The reference exists", References.HasReference("A/b")) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::GetReference()", "mean", "The mean", References.GetReference("A/b").GetMean(), 1.5, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::GetReference()", "error", "The error", References.GetReference("A/b").GetError(), 0.25, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::GetReference()", "scatter", "The scatter", References.GetReference("A/b").GetScatter(), 0.875, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::GetReference()", "runs", "The number of runs", References.GetReference("A/b").GetRuns(), 12U) && Passed;
  Reference.SetMean(2.5);
  References.SetReference(Reference);
  Passed = EvaluateNear("MEndToEndReferenceList::SetReference()", "replace", "A reference which was set again is replaced", References.GetReference("A/b").GetMean(), 2.5, 0.0) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::GetNumberOfReferences()", "number", "A replaced reference is not added twice", References.GetNumberOfReferences(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReferenceList::TestFile()
{
  bool Passed = true;

  const MString Directory = GetTemporaryDirectoryName("references");
  const MString FileName = Directory + "/sub/Test.references.tsv";

  // Values which need all 17 digits survive the round trip, a tab in a name is replaced by a blank
  MEndToEndReferenceList Written;
  Written.GetHeader().Set("test", "UTTest");
  Written.GetHeader().Set("reason", "A reason: with a colon");
  MEndToEndReference Third("Line100/third");
  Third.SetMean(1.0/3.0);
  Third.SetError(1.0e-9/3.0);
  Third.SetScatter(12345.678901234567);
  Third.SetRuns(50);
  Third.SetCalibration(1.37);
  Third.SetRelativeFloor(0.005);
  Third.SetAbsoluteFloor(2.5e-4);
  Written.SetReference(Third);
  Third.SetName("Line100/with\ttab");
  Written.SetReference(Third);
  Third.SetName("Line100/third");
  MEndToEndReference Zero("Line100/zero");
  Zero.SetRuns(50);
  Written.SetReference(Zero);
  Passed = EvaluateTrue("MEndToEndReferenceList::Write()", "directory", "The file is written, the directory is created", Written.Write(FileName)) && Passed;

  MEndToEndReferenceList Read;
  Passed = EvaluateTrue("MEndToEndReferenceList::Read()", "file", "The file can be read", Read.Read(FileName)) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::Read()", "header", "A field of the header", Read.GetHeader().Get("test"), MString("UTTest")) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::Read()", "colon", "A value with a colon is read completely", Read.GetHeader().Get("reason"), MString("A reason: with a colon")) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::Read()", "fields", "The line of the columns is not a field of the header", Read.GetHeader().GetFields().size(), 2) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::Read()", "number", "All references are read", Read.GetNumberOfReferences(), 3) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "mean", "The mean is exact", Read.GetReference("Line100/third").GetMean(), 1.0/3.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "error", "The error is exact", Read.GetReference("Line100/third").GetError(), 1.0e-9/3.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "scatter", "The scatter is exact", Read.GetReference("Line100/third").GetScatter(), 12345.678901234567, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::Read()", "runs", "The number of runs", Read.GetReference("Line100/third").GetRuns(), 50U) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "calibration", "The calibration of the bootstrap error", Read.GetReference("Line100/third").GetCalibration(), 1.37, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "relative floor", "The relative floor", Read.GetReference("Line100/third").GetRelativeFloor(), 0.005, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "absolute floor", "The absolute floor", Read.GetReference("Line100/third").GetAbsoluteFloor(), 2.5e-4, 0.0) && Passed;
  Passed = EvaluateTrue("MEndToEndReferenceList::Read()", "tab", "The name with the tab is read with a blank", Read.HasReference("Line100/with tab")) && Passed;
  Passed = EvaluateTrue("MEndToEndReferenceList::Read()", "zero", "A reference of zero is read", Read.HasReference("Line100/zero")) && Passed;

  // Reading replaces the content
  MEndToEndReferenceList Reused;
  Reused.GetHeader().Set("old", "value");
  Zero.SetName("Old/reference");
  Reused.SetReference(Zero);
  Passed = EvaluateTrue("MEndToEndReferenceList::Read()", "reused", "A used object can read a file", Reused.Read(FileName)) && Passed;
  Passed = EvaluateFalse("MEndToEndReferenceList::Read()", "old reference", "The old references are gone", Reused.HasReference("Old/reference")) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::Read()", "old header", "The old header is gone", Reused.GetHeader().Get("old"), MString("")) && Passed;

  // Missing file and malformed lines
  DisableDefaultStreams();
  const bool Missing = Reused.Read(Directory + "/Missing.tsv");
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndReferenceList::Read()", "missing file", "A missing file cannot be read", Missing) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::Read()", "after failure", "A failed read leaves no stale references", Reused.GetNumberOfReferences(), 0) && Passed;
  const MString Malformed = Directory + "/Malformed.tsv";
  {
    ofstream Out(Malformed.Data());
    Passed = EvaluateTrue("ofstream", "open", "The file with malformed lines can be written", Out.is_open()) && Passed;
    Out<<"# a comment without a field"<<endl;
    Out<<"# : no key"<<endl;
    Out<<"# key: value"<<endl;
    Out<<"only a name"<<endl;
    Out<<"A/b\t1.0\t0.5"<<endl;
    Out<<"A/c\t2.0\t0.5\t1.0\t12"<<endl;
    Out<<"A/d\t3.0\t0.5\t1.0\t12\t99"<<endl;
    Out<<"A/e\t3.0\t0.5\t1.0\t12\t1.5\t0.01"<<endl;
  }
  MEndToEndReferenceList Partly;
  DisableDefaultStreams();
  const bool ReadMalformed = Partly.Read(Malformed);
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndReferenceList::Read()", "malformed", "A file with malformed lines can be read", ReadMalformed) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::Read()", "malformed references", "Only the line with five columns is a reference", Partly.GetNumberOfReferences(), 1) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "no calibration", "A reference with five columns has the calibration one", Partly.GetReference("A/c").GetCalibration(), 1.0, 0.0) && Passed;
  Passed = EvaluateNear("MEndToEndReferenceList::Read()", "no floors", "A reference with five columns has no floors", Partly.GetReference("A/c").GetRelativeFloor() + Partly.GetReference("A/c").GetAbsoluteFloor(), 0.0, 0.0) && Passed;
  Passed = Evaluate("MEndToEndReferenceList::Read()", "malformed field", "The field of a comment with a key is read", Partly.GetHeader().Get("key"), MString("value")) && Passed;
  Passed = EvaluateSize("MEndToEndReferenceList::Read()", "malformed fields", "A comment without a key is ignored", Partly.GetHeader().GetFields().size(), 1) && Passed;

  // The default file name
  Passed = Evaluate("MEndToEndReferenceList::GetFileName()", "default", "The file of a test is in the resources of the end-to-end tests", MEndToEndReferenceList::GetFileName("ETFoo"), MString("$(MEGALIB)/resource/endtoend/ETFoo.references.tsv")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEndToEndReferenceList Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndReferenceList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
