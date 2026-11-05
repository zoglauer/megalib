/*
 * UTParameterFileShared.h
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


#ifndef ___UTParameterFileShared___
#define ___UTParameterFileShared___


// Cosima:
#include "MCActivator.hh"
#include "MCParameterFile.hh"
#include "MCRegion.hh"
#include "MCRun.hh"
#include "MCSource.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4SystemOfUnits.hh"

// Standard lib:
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// Shared helper to run code in a forked child:
#include "UTSDHitShared.h"


//! Parameter file which makes the protected lookup functions public
class UTParameterFileAccess : public MCParameterFile
{
public:
  using MCParameterFile::GetActivator;
  using MCParameterFile::GetRegion;
  using MCParameterFile::GetRun;
  using MCParameterFile::GetSource;
};


//! Base class of the unit tests of MCParameterFile, providing the fixture files and helper functions
class UTParameterFileTest : public MUnitTest
{
public:
  //! Standard constructor
  explicit UTParameterFileTest(const MString& Name) : MUnitTest(Name), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_FilesReady(false) {}
  //! Default destructor
  virtual ~UTParameterFileTest() {}

protected:
  //! Redirect cout, cerr and the MEGAlib streams - the parser prints a lot
  void SilenceOutput()
  {
    DisableDefaultStreams();
    m_Sink.str("");
    m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
    m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
  }

  //! Restore cout, cerr and the MEGAlib streams
  void RestoreOutput()
  {
    if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
    if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
    m_CoutBuffer = nullptr;
    m_CerrBuffer = nullptr;
    EnableDefaultStreams();
  }

  //! Create the private directory with the dummy geometry file
  bool PrepareFiles()
  {
    if (m_FilesReady == true) return true;
    if (PrepareTemporaryDirectory("Files") == false) return false;
    if (WriteTextFile(GetGeometryFileName(), "Name Dummy\nVersion 1.0\n") == false) return false;
    m_FilesReady = true;
    return true;
  }

  //! Return the directory of the fixture files
  MString GetDirectory() const { return GetTemporaryDirectoryName("Files"); }

  //! Return the directory of the fixture files relative to the working directory of the test (the private temporary root)
  //! A parameter file names its files relative: the format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  MString GetRelativeDirectory() const { return "Files"; }

  //! Return the name of the dummy geometry file - the parser only checks that it exists
  MString GetGeometryFileName() const { return GetDirectory() + "/Dummy.geo.setup"; }

  //! Return the directory with the example data files, or an empty string if $MEGALIB is not set
  MString GetExampleDirectory() const
  {
    const char* Root = getenv("MEGALIB");
    if (Root == nullptr) return "";
    return MString(Root) + "/resource/examples/cosima/source";
  }

  //! Return the header of every source file: version and geometry
  MString Header() const { return MString("Version 1\nGeometry ") + GetGeometryFileName() + "\n"; }

  //! Return a source file with the given lines in front of a run "R" with the source "S"
  MString Global(const MString& Lines) const { return Header() + Lines + "Run R\nR.Source S\n"; }

  //! Return a source file with the run "R", the source "S" and the given source lines
  MString OneSource(const MString& SourceLines) const { return Header() + "Run R\nR.Source S\n" + SourceLines; }

  //! Write a source file into the private directory and return its name relative to the working directory of the test (for parameter files)
  MString WriteSource(const MString& FileName, const MString& Content) const
  {
    WriteTextFile(GetDirectory() + "/" + FileName, Content);
    return GetRelativeDirectory() + "/" + FileName;
  }

  //! Write the content into a file and open it with the given parameter file, silently
  bool Open(UTParameterFileAccess& Parameters, const MString& Content, const MString& FileName = "Test.source")
  {
    const MString Name = WriteSource(FileName, Content);
    SilenceOutput();
    bool Result = Parameters.Open(Name);
    RestoreOutput();
    return Result;
  }

  //! Open the content in a forked child with a time limit and return how it ended - for inputs which might crash or hang
  UTSDHitShared::ChildOutcome OpenInChild(const MString& Content, unsigned int TimeLimit = 5, const MString& FileName = "Child.source")
  {
    const MString Name = WriteSource(FileName, Content);
    return UTSDHitShared::RunInChild([Name]() {
      UTParameterFileAccess Parameters;
      return Parameters.Open(Name);
    }, TimeLimit);
  }

  //! Return the source "S" of the parameter file or nullptr
  MCSource* GetS(UTParameterFileAccess& Parameters) { return Parameters.GetSource("S"); }

  //! Return the run "R" of the parameter file or nullptr
  MCRun* GetR(UTParameterFileAccess& Parameters) { return Parameters.GetRun("R"); }

  //! Evaluate that the content is rejected by the parser
  bool ExpectRejected(const MString& Function, const MString& Input, const MString& Description, const MString& Content)
  {
    UTParameterFileAccess Parameters;
    return EvaluateFalse(Function, Input, Description, Open(Parameters, Content));
  }

  //! Evaluate that the content is accepted by the parser
  bool ExpectAccepted(const MString& Function, const MString& Input, const MString& Description, const MString& Content)
  {
    UTParameterFileAccess Parameters;
    return EvaluateTrue(Function, Input, Description, Open(Parameters, Content));
  }

private:
  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The sink for the silenced output
  ostringstream m_Sink;
  //! True if the fixture files exist
  bool m_FilesReady;
};


#endif


/*
 * UTParameterFileShared.h: the end...
 ******************************************************************************/
