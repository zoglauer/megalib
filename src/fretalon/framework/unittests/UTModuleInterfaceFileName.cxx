/*
 * UTModuleInterfaceFileName.cxx
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


// MEGAlib:
#include "MModuleInterfaceFileName.h"
#include "MUnitTest.h"

// Standard libs:
using namespace std;


//! Unit test class for MModuleInterfaceFileName
class UTModuleInterfaceFileName : public MUnitTest
{
public:
  //! Default constructor
  UTModuleInterfaceFileName() : MUnitTest("UTModuleInterfaceFileName") {}
  //! Default destructor
  virtual ~UTModuleInterfaceFileName() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Concrete probe for the abstract file-name interface
  class FileNameInterfaceProbe : public MModuleInterfaceFileName
  {
  public:
    FileNameInterfaceProbe() : MModuleInterfaceFileName() {}
    virtual ~FileNameInterfaceProbe() {}

    virtual MModuleInterfaceFileName* Clone()
    {
      FileNameInterfaceProbe* Probe = new FileNameInterfaceProbe();
      Probe->SetFileName(GetFileName());
      return Probe;
    }
  };

  //! Test default state and direct setter/getter behavior
  bool TestFileNameState();
  //! Test clone and base-interface dispatch
  bool TestCloneAndBasePointer();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleInterfaceFileName::Run()
{
  bool Passed = true;

  Passed = TestFileNameState() && Passed;
  Passed = TestCloneAndBasePointer() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleInterfaceFileName::TestFileNameState()
{
  bool Passed = true;

  FileNameInterfaceProbe Probe;
  Passed = Evaluate("GetFileName()", "default", "The file-name interface starts with an empty file name", Probe.GetFileName(), MString("")) && Passed;

  Probe.SetFileName("representative.roa");
  Passed = Evaluate("SetFileName()/GetFileName()", "representative file", "The file-name interface stores a representative file name", Probe.GetFileName(), MString("representative.roa")) && Passed;

  Probe.SetFileName("/tmp/representative/path/output.evta");
  Passed = Evaluate("SetFileName()/GetFileName()", "absolute path", "The file-name interface stores path-like names without normalization", Probe.GetFileName(), MString("/tmp/representative/path/output.evta")) && Passed;

  Probe.SetFileName("relative/path/input.roa");
  Passed = Evaluate("SetFileName()/GetFileName()", "relative path", "The file-name interface stores relative path-like names unchanged", Probe.GetFileName(), MString("relative/path/input.roa")) && Passed;

  Probe.SetFileName("");
  Passed = Evaluate("SetFileName()/GetFileName()", "empty reset", "Setting an empty file name clears the stored value", Probe.GetFileName(), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleInterfaceFileName::TestCloneAndBasePointer()
{
  bool Passed = true;

  FileNameInterfaceProbe Probe;
  Probe.SetFileName("clone-source.roa");

  MModuleInterfaceFileName* Base = &Probe;
  Passed = Evaluate("GetFileName()", "base pointer", "The file-name interface can be used through a base pointer", Base->GetFileName(), MString("clone-source.roa")) && Passed;

  Base->SetFileName("base-updated.roa");
  Passed = Evaluate("SetFileName()", "base pointer", "Setting the file name through a base pointer updates the concrete object", Probe.GetFileName(), MString("base-updated.roa")) && Passed;

  MModuleInterfaceFileName* Clone = Base->Clone();
  Passed = EvaluateTrue("Clone()", "interface clone", "The concrete probe clone returns a distinct file-name interface object", Clone != nullptr && Clone != Base) && Passed;
  Passed = Evaluate("Clone()", "copied file name", "The representative probe clone preserves the configured file name", Clone->GetFileName(), MString("base-updated.roa")) && Passed;

  Clone->SetFileName("clone-updated.roa");
  Passed = Evaluate("SetFileName()", "clone independence", "Changing the clone file name does not change the original interface object", Probe.GetFileName(), MString("base-updated.roa")) && Passed;
  Passed = Evaluate("SetFileName()", "clone value", "The clone stores its own updated file name", Clone->GetFileName(), MString("clone-updated.roa")) && Passed;

  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleInterfaceFileName Test;
  return Test.Run() == true ? 0 : 1;
}


// UTModuleInterfaceFileName.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
