/*
 * UTSettingsGeomega.cxx
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
#include <new>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MSettingsGeomega.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the geomega settings MSettingsGeomega
//! Attention: The destructor writes the settings file in HOME, MUnitTest redirects HOME to its private directory
class UTSettingsGeomega : public MUnitTest
{
public:
  //! Default constructor
  UTSettingsGeomega() : MUnitTest("UTSettingsGeomega") {}
  //! Default destructor
  virtual ~UTSettingsGeomega() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default values
  bool TestDefaults();
  //! Test that the constructor sets all flags on memory with a pattern
  bool TestInitialization();
  //! Test the setters and getters
  bool TestGettersSetters();
  //! Test a write and read round trip with typical values
  bool TestRoundTrip();
  //! Test a write and read round trip with edge cases
  bool TestRoundTripEdgeCases();
  //! Test the paths in the settings file
  bool TestPathCleaning();
  //! Test the reading of incomplete and wrong files
  bool TestReadFailures();
  //! Test the automatic loading and the writing by the destructor
  bool TestAutoLoadAndDestructor();
  //! Test the suffix of the file and the name of the settings file
  bool TestFileNames();
  //! Test repeated reads with one object
  bool TestReuse();

  //! Fill the settings with the values of the round trip test
  void FillSettings(MSettingsGeomega& Settings) const;
  //! Return the file content for the given nodes
  MString GetFileContent(const MString& Nodes) const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTSettingsGeomega::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestInitialization() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestRoundTrip() && Passed;
  Passed = TestRoundTripEdgeCases() && Passed;
  Passed = TestPathCleaning() && Passed;
  Passed = TestReadFailures() && Passed;
  Passed = TestAutoLoadAndDestructor() && Passed;
  Passed = TestFileNames() && Passed;
  Passed = TestReuse() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Fill the settings with the values of the round trip test
void UTSettingsGeomega::FillSettings(MSettingsGeomega& Settings) const
{
  Settings.SetMGeantOutputMode(3);
  Settings.SetMGeantFileName("/some/where/MyGeant");
  Settings.SetPosition(MVector(1.25, -2.5, 3.0625));
  Settings.SetPathLengthStart(MVector(-10.5, 20.25, 1000.125));
  Settings.SetPathLengthStop(MVector(0.125, -0.25, -500.5));
  Settings.SetStoreIAs(true);
  Settings.SetStoreVetoes(false);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the file content for the given nodes
MString UTSettingsGeomega::GetFileContent(const MString& Nodes) const
{
  return MString("<GeomegaConfigurationFile>\n") + Nodes + "</GeomegaConfigurationFile>\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default values
bool UTSettingsGeomega::TestDefaults()
{
  bool Passed = true;

  MSettingsGeomega Settings(false);
  Passed = Evaluate("MSettingsGeomega()", "no auto load", "The default output mode is zero", Settings.GetMGeantOutputMode(), 0) && Passed;
  Passed = EvaluateTrue("MSettingsGeomega()", "no auto load", "The default file name is MGeantGeo", Settings.GetMGeantFileName() == "MGeantGeo") && Passed;
  Passed = EvaluateFalse("MSettingsGeomega()", "no auto load", "Interactions are not stored by default", Settings.GetStoreIAs()) && Passed;
  Passed = EvaluateTrue("MSettingsGeomega()", "no auto load", "Vetoes are stored by default", Settings.GetStoreVetoes()) && Passed;
  Passed = EvaluateVectorNear("MSettingsGeomega()", "no auto load", "The default position is the origin", Settings.GetPosition(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateVectorNear("MSettingsGeomega()", "no auto load", "The default path starts 1000 cm above the origin", Settings.GetPathLengthStart(), MVector(0, 0, 1000), 0.0) && Passed;
  Passed = EvaluateVectorNear("MSettingsGeomega()", "no auto load", "The default path stops at the origin", Settings.GetPathLengthStop(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateTrue("MSettingsGeomega()", "no auto load", "The settings file is the one in HOME", Settings.GetSettingsFileName() == "~/.geomega.cfg") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test that the constructor sets all flags on memory with a pattern
bool UTSettingsGeomega::TestInitialization()
{
  bool Passed = true;

  // Two valid bool patterns: an initialized member has the same value on both, and all flags have their defaults
  bool Results[2][3];
  for (unsigned int p = 0; p < 2; ++p) {
    alignas(MSettingsGeomega) unsigned char Block[sizeof(MSettingsGeomega)];
    FillMemory(Block, sizeof(Block), static_cast<unsigned char>(p));
    MSettingsGeomega* Settings = new (Block) MSettingsGeomega(false);
    Results[p][0] = Settings->GetStoreIAs();
    Results[p][1] = Settings->GetStoreVetoes();
    Results[p][2] = Settings->GetSpecialMode();
    Settings->~MSettingsGeomega();
  }

  Passed = EvaluateFalse("MSettingsGeomega()", "memory with 0x00", "The store interactions flag is false", Results[0][0]) && Passed;
  Passed = EvaluateFalse("MSettingsGeomega()", "memory with 0x01", "The store interactions flag is false", Results[1][0]) && Passed;
  Passed = EvaluateTrue("MSettingsGeomega()", "memory with 0x00", "The store vetoes flag is true", Results[0][1]) && Passed;
  Passed = EvaluateTrue("MSettingsGeomega()", "memory with 0x01", "The store vetoes flag is true", Results[1][1]) && Passed;
  Passed = EvaluateFalse("MSettingsGeomega()", "memory with 0x00", "The special mode is false", Results[0][2]) && Passed;

  // Defect GEO-F-13: The special mode flag is not initialized by the constructor
  Passed = ExpectedToFail(EvaluateFalse("MSettingsGeomega()", "memory with 0x01", "The special mode is false independent of the memory", Results[1][2])) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters
bool UTSettingsGeomega::TestGettersSetters()
{
  bool Passed = true;

  MSettingsGeomega Settings(false);

  for (int Mode : {1, 0, -1, 2147483647}) {
    Settings.SetMGeantOutputMode(Mode);
    Passed = Evaluate("SetMGeantOutputMode()", Mode, "The mode is stored", Settings.GetMGeantOutputMode(), Mode) && Passed;
  }

  for (const MString& Name : {MString("Other"), MString(""), MString("/with/path and space.geo")}) {
    Settings.SetMGeantFileName(Name);
    Passed = EvaluateTrue("SetMGeantFileName()", Name, "The name is stored", Settings.GetMGeantFileName() == Name) && Passed;
  }

  Settings.SetPosition(MVector(1.5, -2.5, 3.5));
  Passed = EvaluateVectorNear("SetPosition()", "(1.5, -2.5, 3.5)", "The position is stored", Settings.GetPosition(), MVector(1.5, -2.5, 3.5), 0.0) && Passed;
  Settings.SetPathLengthStart(MVector(-4.5, 5.5, -6.5));
  Passed = EvaluateVectorNear("SetPathLengthStart()", "(-4.5, 5.5, -6.5)", "The start is stored", Settings.GetPathLengthStart(), MVector(-4.5, 5.5, -6.5), 0.0) && Passed;
  Settings.SetPathLengthStop(MVector(7.5, -8.5, 9.5));
  Passed = EvaluateVectorNear("SetPathLengthStop()", "(7.5, -8.5, 9.5)", "The stop is stored", Settings.GetPathLengthStop(), MVector(7.5, -8.5, 9.5), 0.0) && Passed;

  // The three vectors are independent
  Passed = EvaluateVectorNear("SetPathLengthStop()", "(7.5, -8.5, 9.5)", "The position does not change", Settings.GetPosition(), MVector(1.5, -2.5, 3.5), 0.0) && Passed;
  Passed = EvaluateVectorNear("SetPathLengthStop()", "(7.5, -8.5, 9.5)", "The start does not change", Settings.GetPathLengthStart(), MVector(-4.5, 5.5, -6.5), 0.0) && Passed;

  for (bool Flag : {true, false, true}) {
    Settings.SetStoreIAs(Flag);
    Passed = Evaluate("SetStoreIAs()", Flag, "The flag is stored", Settings.GetStoreIAs(), Flag) && Passed;
    Settings.SetStoreVetoes(!Flag);
    Passed = Evaluate("SetStoreVetoes()", !Flag, "The flag is stored", Settings.GetStoreVetoes(), !Flag) && Passed;
    Passed = Evaluate("SetStoreVetoes()", !Flag, "The other flag is not changed", Settings.GetStoreIAs(), Flag) && Passed;
    Settings.SetSpecialMode(Flag);
    Passed = Evaluate("SetSpecialMode()", Flag, "The flag is stored", Settings.GetSpecialMode(), Flag) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a write and read round trip with typical values
bool UTSettingsGeomega::TestRoundTrip()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("RoundTrip.cfg");

  MSettingsGeomega Source(false);
  FillSettings(Source);
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;
  Passed = EvaluateTrue("Write()", FileName, "The file exists", MFile::Exists(FileName)) && Passed;

  MSettingsGeomega Target(false);
  DisableDefaultStreams();
  bool Result = Target.Read(FileName);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Read()", FileName, "The file can be read", Result) && Passed;

  Passed = Evaluate("Read()", "output mode", "The value of the file is used", Target.GetMGeantOutputMode(), 3) && Passed;
  Passed = EvaluateTrue("Read()", "file name", "The value of the file is used", Target.GetMGeantFileName() == "/some/where/MyGeant") && Passed;
  Passed = EvaluateVectorNear("Read()", "position", "The value of the file is used", Target.GetPosition(), MVector(1.25, -2.5, 3.0625), 1E-9) && Passed;
  Passed = EvaluateVectorNear("Read()", "path start", "The value of the file is used", Target.GetPathLengthStart(), MVector(-10.5, 20.25, 1000.125), 1E-9) && Passed;
  Passed = EvaluateVectorNear("Read()", "path stop", "The value of the file is used", Target.GetPathLengthStop(), MVector(0.125, -0.25, -500.5), 1E-9) && Passed;
  Passed = EvaluateTrue("Read()", "store interactions", "The value of the file is used", Target.GetStoreIAs()) && Passed;
  Passed = EvaluateFalse("Read()", "store vetoes", "The value of the file is used", Target.GetStoreVetoes()) && Passed;

  // The other way around for the flags
  Target.SetStoreIAs(false);
  Target.SetStoreVetoes(true);
  const MString Flags = GetTemporaryFileName("Flags.cfg");
  Passed = EvaluateTrue("Write()", Flags, "The file can be written", Target.Write(Flags)) && Passed;
  MSettingsGeomega FlagsTarget(false);
  FlagsTarget.SetStoreIAs(true);
  FlagsTarget.SetStoreVetoes(false);
  DisableDefaultStreams();
  FlagsTarget.Read(Flags);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Read()", "store interactions false", "The value of the file overwrites the old one", FlagsTarget.GetStoreIAs()) && Passed;
  Passed = EvaluateTrue("Read()", "store vetoes true", "The value of the file overwrites the old one", FlagsTarget.GetStoreVetoes()) && Passed;

  // The special mode comes only from the command line and is not part of the file
  MSettingsGeomega Special(false);
  Special.SetSpecialMode(true);
  const MString SpecialFile = GetTemporaryFileName("Special.cfg");
  Passed = EvaluateTrue("Write()", SpecialFile, "The file can be written", Special.Write(SpecialFile)) && Passed;
  Passed = EvaluateFalse("Write()", SpecialFile, "The file has no special mode", ReadTextFile(SpecialFile).Contains("SpecialMode")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a write and read round trip with edge cases
bool UTSettingsGeomega::TestRoundTripEdgeCases()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Edge.cfg");

  // The numbers are written with 15 significant digits
  const MVector Precise(0.123456789012345, -9876.54321098765, 1.23456789012345E-20);

  MSettingsGeomega Source(false);
  Source.SetMGeantOutputMode(-2147483647);
  Source.SetMGeantFileName("Name with spaces & <special> characters.geo");
  Source.SetPosition(Precise);
  Source.SetPathLengthStart(MVector(0, 0, 0));
  Source.SetPathLengthStop(MVector(-1E30, 1E30, 0));
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;

  MSettingsGeomega Target(false);
  DisableDefaultStreams();
  bool Result = Target.Read(FileName);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Read()", FileName, "The file can be read", Result) && Passed;
  Passed = Evaluate("Read()", "negative output mode", "The value of the file is used", Target.GetMGeantOutputMode(), -2147483647) && Passed;
  Passed = EvaluateTrue("Read()", "name with special characters", "The name is identical", Target.GetMGeantFileName() == "Name with spaces & <special> characters.geo") && Passed;
  Passed = EvaluateNear("Read()", "precise x", "The value has 15 digits", Target.GetPosition().X(), Precise.X(), 1E-15) && Passed;
  Passed = EvaluateNear("Read()", "precise y", "The value has 15 digits", Target.GetPosition().Y(), Precise.Y(), 1E-10) && Passed;
  Passed = EvaluateNear("Read()", "tiny z", "The value has 15 digits", Target.GetPosition().Z(), Precise.Z(), 1E-34) && Passed;
  Passed = EvaluateVectorNear("Read()", "zero vector", "The zero vector is not lost", Target.GetPathLengthStart(), MVector(0, 0, 0), 0.0) && Passed;
  Passed = EvaluateNear("Read()", "huge value", "Large values are not lost", Target.GetPathLengthStop().X(), -1E30, 1E16) && Passed;

  // An empty name
  const MString Empty = GetTemporaryFileName("Empty.cfg");
  MSettingsGeomega EmptySource(false);
  EmptySource.SetMGeantFileName("");
  Passed = EvaluateTrue("Write()", Empty, "The file can be written", EmptySource.Write(Empty)) && Passed;
  MSettingsGeomega EmptyTarget(false);
  DisableDefaultStreams();
  EmptyTarget.Read(Empty);
  EnableDefaultStreams();
  Passed = EvaluateTrue("Read()", "empty name", "An empty name stays empty", EmptyTarget.GetMGeantFileName() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the paths in the settings file
bool UTSettingsGeomega::TestPathCleaning()
{
  bool Passed = true;

  // The MEGAlib path goes as variable into the file
  MString Expanded = "$(MEGALIB)/some/file";
  MFile::ExpandFileName(Expanded);
  const MString FileName = GetTemporaryFileName("Path.cfg");
  MSettingsGeomega Source(false);
  Source.SetMGeantFileName(Expanded);
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;
  Passed = EvaluateTrue("Write()", FileName, "The path has the variable", ReadTextFile(FileName).Contains("$(MEGALIB)/some/file")) && Passed;

  // Other paths are not touched
  const MString Other = GetTemporaryFileName("OtherPath.cfg");
  MSettingsGeomega OtherSource(false);
  OtherSource.SetMGeantFileName("/not/megalib/file");
  Passed = EvaluateTrue("Write()", Other, "The file can be written", OtherSource.Write(Other)) && Passed;
  Passed = EvaluateTrue("Write()", Other, "The path is not changed", ReadTextFile(Other).Contains("/not/megalib/file")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reading of incomplete and wrong files
bool UTSettingsGeomega::TestReadFailures()
{
  bool Passed = true;

  // Fill and read a settings object, the values of a failed read have to stay
  auto ReadInto = [this](MSettingsGeomega& Settings, const MString& File) {
    FillSettings(Settings);
    DisableDefaultStreams();
    bool Result = Settings.Read(File);
    EnableDefaultStreams();
    return Result;
  };

  // No file
  {
    MSettingsGeomega Settings(false);
    Passed = EvaluateFalse("Read()", "missing file", "A missing file is a failure", ReadInto(Settings, GetTemporaryFileName("DoesNotExist.cfg"))) && Passed;
    Passed = Evaluate("Read()", "missing file", "The values stay", Settings.GetMGeantOutputMode(), 3) && Passed;
    Passed = EvaluateTrue("Read()", "missing file", "The values stay", Settings.GetMGeantFileName() == "/some/where/MyGeant") && Passed;
  }

  // Empty file
  {
    const MString File = GetTemporaryFileName("EmptyFile.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, "")) && Passed;
    MSettingsGeomega Settings(false);
    Passed = EvaluateFalse("Read()", "empty file", "An empty file is a failure", ReadInto(Settings, File)) && Passed;
    Passed = Evaluate("Read()", "empty file", "The values stay", Settings.GetMGeantOutputMode(), 3) && Passed;
  }

  // Settings of another program
  {
    const MString File = GetTemporaryFileName("WrongProgram.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, "<RevanConfigurationFile>\n<MGeantOutputMode>5</MGeantOutputMode>\n</RevanConfigurationFile>\n")) && Passed;
    MSettingsGeomega Settings(false);
    Passed = EvaluateFalse("Read()", "settings of another program", "The master node is checked", ReadInto(Settings, File)) && Passed;
    Passed = Evaluate("Read()", "settings of another program", "The values stay", Settings.GetMGeantOutputMode(), 3) && Passed;
  }

  // Attention: A file without XML is accepted by the base class and only the message tells that the default configuration is used
  {
    const MString File = GetTemporaryFileName("NoXml.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, "MGeantOutputMode 5\n")) && Passed;
    MSettingsGeomega Settings(false);
    ReadInto(Settings, File);
    Passed = Evaluate("Read()", "no XML", "The values stay", Settings.GetMGeantOutputMode(), 3) && Passed;
  }

  // An incomplete file only changes what it contains
  {
    const MString File = GetTemporaryFileName("Incomplete.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, GetFileContent("<MGeantOutputMode>5</MGeantOutputMode>\n<PathLengthStop><X>1.5</X><Y>2.5</Y><Z>3.5</Z></PathLengthStop>\n"))) && Passed;
    MSettingsGeomega Settings(false);
    Passed = EvaluateTrue("Read()", "incomplete file", "The file can be read", ReadInto(Settings, File)) && Passed;
    Passed = Evaluate("Read()", "incomplete file", "The value in the file is used", Settings.GetMGeantOutputMode(), 5) && Passed;
    Passed = EvaluateVectorNear("Read()", "incomplete file", "The value in the file is used", Settings.GetPathLengthStop(), MVector(1.5, 2.5, 3.5), 1E-12) && Passed;
    Passed = EvaluateTrue("Read()", "incomplete file", "The value not in the file stays", Settings.GetMGeantFileName() == "/some/where/MyGeant") && Passed;
    Passed = EvaluateVectorNear("Read()", "incomplete file", "The value not in the file stays", Settings.GetPosition(), MVector(1.25, -2.5, 3.0625), 1E-12) && Passed;
    Passed = EvaluateTrue("Read()", "incomplete file", "The value not in the file stays", Settings.GetStoreIAs()) && Passed;
    Passed = EvaluateFalse("Read()", "incomplete file", "The value not in the file stays", Settings.GetStoreVetoes()) && Passed;
  }

  // Unknown nodes are ignored
  {
    const MString File = GetTemporaryFileName("Unknown.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, GetFileContent("<UnknownNode>1</UnknownNode>\n<MGeantOutputMode>6</MGeantOutputMode>\n"))) && Passed;
    MSettingsGeomega Settings(false);
    Passed = EvaluateTrue("Read()", "unknown node", "The file can be read", ReadInto(Settings, File)) && Passed;
    Passed = Evaluate("Read()", "unknown node", "The known value is used", Settings.GetMGeantOutputMode(), 6) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the automatic loading and the writing by the destructor
bool UTSettingsGeomega::TestAutoLoadAndDestructor()
{
  bool Passed = true;

  // Start without the file in the private HOME
  MString DefaultFile = "~/.geomega.cfg";
  MFile::ExpandFileName(DefaultFile);
  RemoveTemporaryFile(DefaultFile);

  // The destructor writes the settings into the default file
  {
    MSettingsGeomega Settings(false);
    FillSettings(Settings);
  }
  Passed = EvaluateTrue("~MSettingsGeomega()", "private HOME", "The destructor writes the default settings file", MFile::Exists(DefaultFile)) && Passed;

  // The constructor reads it again
  {
    DisableDefaultStreams();
    MSettingsGeomega Settings(true);
    EnableDefaultStreams();
    Passed = Evaluate("MSettingsGeomega(true)", "output mode", "The auto load uses the default settings file", Settings.GetMGeantOutputMode(), 3) && Passed;
    Passed = EvaluateVectorNear("MSettingsGeomega(true)", "position", "The auto load uses the default settings file", Settings.GetPosition(), MVector(1.25, -2.5, 3.0625), 1E-9) && Passed;
    Passed = EvaluateTrue("MSettingsGeomega(true)", "settings file", "The expanded file name is stored", Settings.GetSettingsFileName() == DefaultFile) && Passed;
  }

  // Without auto load it is not read
  {
    MSettingsGeomega Settings(false);
    Passed = Evaluate("MSettingsGeomega(false)", "output mode", "Without auto load the default is used", Settings.GetMGeantOutputMode(), 0) && Passed;
  }

  // The auto load without file uses the defaults
  RemoveTemporaryFile(DefaultFile);
  {
    DisableDefaultStreams();
    MSettingsGeomega Settings(true);
    EnableDefaultStreams();
    Passed = Evaluate("MSettingsGeomega(true)", "no settings file", "The default is used", Settings.GetMGeantOutputMode(), 0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the suffix of the file and the name of the settings file
bool UTSettingsGeomega::TestFileNames()
{
  bool Passed = true;

  // The suffix is added
  const MString NoSuffix = GetTemporaryFileName("NoSuffix");
  MSettingsGeomega Settings(false);
  FillSettings(Settings);
  Passed = EvaluateTrue("Write()", NoSuffix, "The file can be written", Settings.Write(NoSuffix)) && Passed;
  Passed = EvaluateFalse("Write()", NoSuffix, "No file without the suffix exists", MFile::Exists(NoSuffix)) && Passed;
  Passed = EvaluateTrue("Write()", NoSuffix, "The suffix cfg is added", MFile::Exists(NoSuffix + ".cfg")) && Passed;

  // The reading stores the name of the file
  MSettingsGeomega Reader(false);
  DisableDefaultStreams();
  Reader.Read(NoSuffix + ".cfg");
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetSettingsFileName()", NoSuffix, "The name of the read file is stored", Reader.GetSettingsFileName() == NoSuffix + ".cfg") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test repeated reads with one object
bool UTSettingsGeomega::TestReuse()
{
  bool Passed = true;

  const MString First = GetTemporaryFileName("ReuseFirst.cfg");
  const MString Second = GetTemporaryFileName("ReuseSecond.cfg");

  MSettingsGeomega A(false);
  A.SetMGeantOutputMode(11);
  A.SetPosition(MVector(1, 2, 3));
  Passed = EvaluateTrue("Write()", First, "The file can be written", A.Write(First)) && Passed;
  A.SetMGeantOutputMode(22);
  A.SetPosition(MVector(4, 5, 6));
  Passed = EvaluateTrue("Write()", Second, "The file can be written again by the same object", A.Write(Second)) && Passed;

  MSettingsGeomega B(false);
  DisableDefaultStreams();
  B.Read(First);
  EnableDefaultStreams();
  Passed = Evaluate("Read()", "first file", "The value of the first file is used", B.GetMGeantOutputMode(), 11) && Passed;
  DisableDefaultStreams();
  B.Read(Second);
  EnableDefaultStreams();
  Passed = Evaluate("Read()", "second file", "The value of the second file replaces it", B.GetMGeantOutputMode(), 22) && Passed;
  Passed = EvaluateVectorNear("Read()", "second file", "The value of the second file replaces it", B.GetPosition(), MVector(4, 5, 6), 1E-12) && Passed;
  DisableDefaultStreams();
  bool Failed = B.Read(GetTemporaryFileName("NotThere.cfg"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("Read()", "missing file after a good one", "The failure is reported", Failed) && Passed;
  Passed = Evaluate("Read()", "missing file after a good one", "The old value stays", B.GetMGeantOutputMode(), 22) && Passed;
  DisableDefaultStreams();
  B.Read(First);
  EnableDefaultStreams();
  Passed = Evaluate("Read()", "first file again", "The object can read again after a failure", B.GetMGeantOutputMode(), 11) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsGeomega Test;
  return Test.Run() == true ? 0 : 1;
}


// UTSettingsGeomega.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
