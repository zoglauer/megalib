/*
 * UTEndToEndRecord.cxx
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


// MEGAlib libs:
#include "MEndToEndRecord.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndRecord
class UTEndToEndRecord : public MUnitTest
{
public:
  UTEndToEndRecord() : MUnitTest("UTEndToEndRecord") {}
  virtual ~UTEndToEndRecord() {}

  virtual bool Run();

private:
  bool TestFields();
  bool TestFromLine();
  bool TestFile();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndRecord::Run()
{
  bool Passed = true;
  Passed = TestFields() && Passed;
  Passed = TestFromLine() && Passed;
  Passed = TestFile() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndRecord::TestFields()
{
  bool Passed = true;

  MEndToEndRecord Record;
  Passed = Evaluate("MEndToEndRecord::Get()", "missing", "A field which was not set is empty", Record.Get("geant4"), MString("")) && Passed;
  Record.Set("geant4", "10.2.3");
  Record.Set("root", "6.40.04");
  Passed = Evaluate("MEndToEndRecord::Get()", "set", "A field which was set", Record.Get("geant4"), MString("10.2.3")) && Passed;
  Record.Set("geant4", "11.2.0");
  Passed = Evaluate("MEndToEndRecord::Get()", "replaced", "A field which was set again is replaced", Record.Get("geant4"), MString("11.2.0")) && Passed;
  Passed = EvaluateSize("MEndToEndRecord::GetFields()", "number", "A replaced field is not added twice", Record.GetFields().size(), 2) && Passed;
  Passed = Evaluate("MEndToEndRecord::GetFields()", "order", "The fields keep the order of the first setting", Record.GetFields()[0].first, MString("geant4")) && Passed;
  Passed = Evaluate("MEndToEndRecord::ToLines()", "lines", "The fields as lines", Record.ToLines(), MString("# geant4: 11.2.0\n# root: 6.40.04\n")) && Passed;
  Record.Clear();
  Passed = EvaluateSize("MEndToEndRecord::Clear()", "fields", "Clear removes the fields", Record.GetFields().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndRecord::TestFromLine()
{
  bool Passed = true;

  MEndToEndRecord Record;
  Passed = EvaluateTrue("MEndToEndRecord::FromLine()", "field", "A line with a key and a value is a field", Record.FromLine("# key: a value: with a colon")) && Passed;
  Passed = Evaluate("MEndToEndRecord::FromLine()", "colon", "The value is read completely", Record.Get("key"), MString("a value: with a colon")) && Passed;
  Passed = EvaluateFalse("MEndToEndRecord::FromLine()", "no field", "A comment without a key is no field", Record.FromLine("# a comment without a field")) && Passed;
  Passed = EvaluateFalse("MEndToEndRecord::FromLine()", "no key", "A comment without a name is no field", Record.FromLine("# : no key")) && Passed;
  Passed = EvaluateFalse("MEndToEndRecord::FromLine()", "columns", "The line of the columns is no field", Record.FromLine("# columns: Name, Mean")) && Passed;
  Passed = EvaluateFalse("MEndToEndRecord::FromLine()", "data", "A line which is not a comment is no field", Record.FromLine("A/b\t1.0: 2")) && Passed;
  Passed = EvaluateSize("MEndToEndRecord::FromLine()", "number", "Only the first line is a field", Record.GetFields().size(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndRecord::TestFile()
{
  bool Passed = true;

  const MString Directory = GetTemporaryDirectoryName("record");
  const MString FileName = Directory + "/sub/Test.record.tsv";

  MEndToEndRecord Written;
  Written.Set("test", "UTTest");
  Written.Set("reason", "A reason: with a colon");
  Passed = EvaluateTrue("MEndToEndRecord::Write()", "directory", "The file is written, the directory is created", Written.Write(FileName, "A title", "# columns: Name\nA/b\t1.0\n")) && Passed;

  MEndToEndRecord Read;
  Read.Set("old", "value");
  Passed = EvaluateTrue("MEndToEndRecord::Read()", "file", "The file can be read", Read.Read(FileName)) && Passed;
  Passed = Evaluate("MEndToEndRecord::Read()", "test", "A field", Read.Get("test"), MString("UTTest")) && Passed;
  Passed = Evaluate("MEndToEndRecord::Read()", "colon", "A value with a colon is read completely", Read.Get("reason"), MString("A reason: with a colon")) && Passed;
  Passed = Evaluate("MEndToEndRecord::Read()", "old", "The old fields are gone", Read.Get("old"), MString("")) && Passed;
  Passed = EvaluateSize("MEndToEndRecord::Read()", "fields", "The title, the line of the columns and the data are no fields", Read.GetFields().size(), 2) && Passed;

  DisableDefaultStreams();
  const bool Missing = Read.Read(Directory + "/Missing.tsv");
  EnableDefaultStreams();
  Passed = EvaluateFalse("MEndToEndRecord::Read()", "missing file", "A missing file cannot be read", Missing) && Passed;
  Passed = EvaluateSize("MEndToEndRecord::Read()", "after failure", "A failed read leaves no stale fields", Read.GetFields().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTEndToEndRecord Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndRecord.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
