/*
 * UTSettingsRevan.cxx
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
#include "MSettingsRevan.h"
#include "MString.h"
#include "MUnitTest.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the revan settings MSettingsRevan
//! Attention: The destructor writes the settings file in HOME, MUnitTest redirects HOME to its private directory
class UTSettingsRevan : public MUnitTest
{
public:
  //! Default constructor
  UTSettingsRevan() : MUnitTest("UTSettingsRevan") {}
  //! Default destructor
  virtual ~UTSettingsRevan() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the default values, also on memory with patterns
  bool TestDefaults();
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

  //! Check that the values of MSettingsRevan itself are the default values
  bool CheckDefaults(MSettingsRevan& Settings, const MString& Input);
  //! Set the values of MSettingsRevan itself, and some of its base classes, to values which are not the defaults
  void FillSettings(MSettingsRevan& Settings) const;
  //! Check that the values are the ones of FillSettings - without the spectrum before flag
  bool CheckFilled(MSettingsRevan& Settings, const MString& Input);
  //! Read the file into the settings with silenced streams
  bool ReadSilently(MSettingsRevan& Settings, const MString& FileName);
  //! Return the file content for the given nodes
  MString GetFileContent(const MString& Nodes) const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTSettingsRevan::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
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


//! Check that the values of MSettingsRevan itself are the default values
bool UTSettingsRevan::CheckDefaults(MSettingsRevan& Settings, const MString& Input)
{
  bool Passed = true;

  const MString Function = "MSettingsRevan()";

  Passed = Evaluate(Function, Input, "The default number of bins of the exported spectrum is 100", Settings.GetExportSpectrumBins(), 100) && Passed;
  Passed = EvaluateFalse(Function, Input, "The exported spectrum is not logarithmic by default", Settings.GetExportSpectrumLog()) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum of the exported spectrum is 0", Settings.GetExportSpectrumMin(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default maximum of the exported spectrum is 1000", Settings.GetExportSpectrumMax(), 1000.0, 0.0) && Passed;
  Passed = Evaluate(Function, Input, "The default file name of the exported spectrum is Export.dat", Settings.GetExportSpectrumFileName(), MString("Export.dat")) && Passed;

  Passed = EvaluateTrue(Function, Input, "The spectrum before the reconstruction is shown by default", Settings.GetSpectrumBefore()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum after the reconstruction is not shown by default", Settings.GetSpectrumAfter()) && Passed;
  Passed = EvaluateTrue(Function, Input, "The spectrum is sorted by instrument by default", Settings.GetSpectrumSortByInstrument()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum is not sorted by detector type by default", Settings.GetSpectrumSortByDetectorType()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum is not sorted by named detector by default", Settings.GetSpectrumSortByNamedDetector()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum is not sorted by detector by default", Settings.GetSpectrumSortByDetector()) && Passed;
  Passed = EvaluateTrue(Function, Input, "The spectra are combined by default", Settings.GetSpectrumCombine()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The total deposit is not used by default", Settings.GetSpectrumTotalDeposit()) && Passed;
  Passed = EvaluateTrue(Function, Input, "The spectrum goes to the screen by default", Settings.GetSpectrumOutputToScreen()) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum does not go to a file by default", Settings.GetSpectrumOutputToFile()) && Passed;
  Passed = Evaluate(Function, Input, "The default number of bins of the spectrum is 100", Settings.GetSpectrumBins(), 100) && Passed;
  Passed = EvaluateFalse(Function, Input, "The spectrum is not logarithmic by default", Settings.GetSpectrumLog()) && Passed;
  Passed = EvaluateNear(Function, Input, "The default minimum of the spectrum is 0", Settings.GetSpectrumMin(), 0.0, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input, "The default maximum of the spectrum is 1000", Settings.GetSpectrumMax(), 1000.0, 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the values of MSettingsRevan itself, and some of its base classes, to values which are not the defaults
void UTSettingsRevan::FillSettings(MSettingsRevan& Settings) const
{
  Settings.SetExportSpectrumBins(250);
  Settings.SetExportSpectrumLog(true);
  Settings.SetExportSpectrumMin(12.5);
  Settings.SetExportSpectrumMax(2500.25);
  Settings.SetExportSpectrumFileName("/some/where/MyExport.dat");

  Settings.SetSpectrumAfter(true);
  Settings.SetSpectrumSortByInstrument(false);
  Settings.SetSpectrumSortByDetectorType(true);
  Settings.SetSpectrumSortByNamedDetector(true);
  Settings.SetSpectrumSortByDetector(true);
  Settings.SetSpectrumCombine(false);
  Settings.SetSpectrumTotalDeposit(true);
  Settings.SetSpectrumOutputToScreen(false);
  Settings.SetSpectrumOutputToFile(true);
  Settings.SetSpectrumBins(333);
  Settings.SetSpectrumLog(true);
  Settings.SetSpectrumMin(0.125);
  Settings.SetSpectrumMax(8000.5);

  // The event reconstruction and the spectral options
  Settings.SetCSRAlgorithm(4);
  Settings.SetCoincidenceWindow(0.5);
  Settings.SetLensCenter(MVector(1.25, -2.5, 3.0625));
  Settings.AddElectronTrackingDetector("Tracker");
  Settings.SetSpectralPoissonLimit(17);
}


////////////////////////////////////////////////////////////////////////////////


//! Check that the values are the ones of FillSettings - without the spectrum before flag
bool UTSettingsRevan::CheckFilled(MSettingsRevan& Settings, const MString& Input)
{
  bool Passed = true;

  const MString Function = "Read()";
  const MString Description = "The value of the file is used";

  // All numbers are exact binary fractions
  Passed = Evaluate(Function, Input + ": export spectrum bins", Description, Settings.GetExportSpectrumBins(), 250) && Passed;
  Passed = EvaluateTrue(Function, Input + ": export spectrum logarithmic", Description, Settings.GetExportSpectrumLog()) && Passed;
  Passed = EvaluateNear(Function, Input + ": export spectrum minimum", Description, Settings.GetExportSpectrumMin(), 12.5, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": export spectrum maximum", Description, Settings.GetExportSpectrumMax(), 2500.25, 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": export spectrum file name", Description, Settings.GetExportSpectrumFileName(), MString("/some/where/MyExport.dat")) && Passed;

  Passed = EvaluateTrue(Function, Input + ": spectrum after", Description, Settings.GetSpectrumAfter()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": sort by instrument", Description, Settings.GetSpectrumSortByInstrument()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": sort by detector type", Description, Settings.GetSpectrumSortByDetectorType()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": sort by named detector", Description, Settings.GetSpectrumSortByNamedDetector()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": sort by detector", Description, Settings.GetSpectrumSortByDetector()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": combine", Description, Settings.GetSpectrumCombine()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": total deposit", Description, Settings.GetSpectrumTotalDeposit()) && Passed;
  Passed = EvaluateFalse(Function, Input + ": output to screen", Description, Settings.GetSpectrumOutputToScreen()) && Passed;
  Passed = EvaluateTrue(Function, Input + ": output to file", Description, Settings.GetSpectrumOutputToFile()) && Passed;
  Passed = Evaluate(Function, Input + ": spectrum bins", Description, Settings.GetSpectrumBins(), 333) && Passed;
  Passed = EvaluateTrue(Function, Input + ": spectrum logarithmic", Description, Settings.GetSpectrumLog()) && Passed;
  Passed = EvaluateNear(Function, Input + ": spectrum minimum", Description, Settings.GetSpectrumMin(), 0.125, 0.0) && Passed;
  Passed = EvaluateNear(Function, Input + ": spectrum maximum", Description, Settings.GetSpectrumMax(), 8000.5, 0.0) && Passed;

  // The base classes are part of the file
  Passed = Evaluate(Function, Input + ": Compton sequence algorithm", "The event reconstruction settings are read", Settings.GetCSRAlgorithm(), 4) && Passed;
  Passed = EvaluateNear(Function, Input + ": coincidence window", "The event reconstruction settings are read", Settings.GetCoincidenceWindow(), 0.5, 0.0) && Passed;
  Passed = EvaluateVectorNear(Function, Input + ": lens center", "The event reconstruction settings are read", Settings.GetLensCenter(), MVector(1.25, -2.5, 3.0625), 0.0) && Passed;
  Passed = Evaluate(Function, Input + ": electron tracking detectors", "The event reconstruction settings are read", Settings.GetNElectronTrackingDetectors(), 1U) && Passed;
  Passed = Evaluate(Function, Input + ": spectral Poisson limit", "The spectral options are read", Settings.GetSpectralPoissonLimit(), 17) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the file into the settings with silenced streams
bool UTSettingsRevan::ReadSilently(MSettingsRevan& Settings, const MString& FileName)
{
  DisableDefaultStreams();
  bool Result = Settings.Read(FileName);
  EnableDefaultStreams();

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the file content for the given nodes
MString UTSettingsRevan::GetFileContent(const MString& Nodes) const
{
  return MString("<RevanConfigurationFile>\n") + Nodes + "</RevanConfigurationFile>\n";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the default values, also on memory with patterns
bool UTSettingsRevan::TestDefaults()
{
  bool Passed = true;

  {
    MSettingsRevan Settings(false);
    Passed = CheckDefaults(Settings, "no auto load") && Passed;
    Passed = Evaluate("MSettingsRevan()", "no auto load", "The settings file is the one in HOME", Settings.GetSettingsFileName(), MString("~/.revan.cfg")) && Passed;
    // The base classes have their defaults
    Passed = Evaluate("MSettingsRevan()", "no auto load", "The event reconstruction settings have their defaults", Settings.GetCSRAlgorithm(), 1) && Passed;
    Passed = Evaluate("MSettingsRevan()", "no auto load", "The number of jobs is one", Settings.GetNJobs(), 1U) && Passed;
  }

  // On memory with patterns: 0xA5 shows numbers and false flags which are not set, 0x00 shows true flags which are not set
  {
    alignas(MSettingsRevan) unsigned char Block[sizeof(MSettingsRevan)];
    FillMemory(Block, sizeof(Block), 0xA5);
    MSettingsRevan* Pattern = new (Block) MSettingsRevan(false);
    Passed = CheckDefaults(*Pattern, "memory with 0xA5") && Passed;
    Pattern->~MSettingsRevan();
  }
  {
    alignas(MSettingsRevan) unsigned char Block[sizeof(MSettingsRevan)];
    FillMemory(Block, sizeof(Block), 0x00);
    MSettingsRevan* Pattern = new (Block) MSettingsRevan(false);
    Passed = CheckDefaults(*Pattern, "memory with 0x00") && Passed;
    Pattern->~MSettingsRevan();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters and getters
bool UTSettingsRevan::TestGettersSetters()
{
  bool Passed = true;

  MSettingsRevan Settings(false);

  for (int Bins : { 50, 1, 0, -1, 2147483647 }) {
    Settings.SetExportSpectrumBins(Bins);
    Passed = Evaluate("SetExportSpectrumBins()", Bins, "The number of bins is stored", Settings.GetExportSpectrumBins(), Bins) && Passed;
    Settings.SetSpectrumBins(Bins - 1);
    Passed = Evaluate("SetSpectrumBins()", Bins - 1, "The number of bins is stored", Settings.GetSpectrumBins(), Bins - 1) && Passed;
    Passed = Evaluate("SetSpectrumBins()", Bins - 1, "The number of bins of the exported spectrum is not changed", Settings.GetExportSpectrumBins(), Bins) && Passed;
  }

  for (double Value : { 123.456, 0.0, -1.5, 1E30 }) {
    Settings.SetExportSpectrumMin(Value);
    Passed = EvaluateNear("SetExportSpectrumMin()", Value, "The value is stored", Settings.GetExportSpectrumMin(), Value, 0.0) && Passed;
    Settings.SetExportSpectrumMax(2*Value);
    Passed = EvaluateNear("SetExportSpectrumMax()", 2*Value, "The value is stored", Settings.GetExportSpectrumMax(), 2*Value, 0.0) && Passed;
    Settings.SetSpectrumMin(3*Value);
    Passed = EvaluateNear("SetSpectrumMin()", 3*Value, "The value is stored", Settings.GetSpectrumMin(), 3*Value, 0.0) && Passed;
    Settings.SetSpectrumMax(4*Value);
    Passed = EvaluateNear("SetSpectrumMax()", 4*Value, "The value is stored", Settings.GetSpectrumMax(), 4*Value, 0.0) && Passed;
    Passed = EvaluateNear("SetSpectrumMax()", 4*Value, "The minimum of the exported spectrum is not changed", Settings.GetExportSpectrumMin(), Value, 0.0) && Passed;
    Passed = EvaluateNear("SetSpectrumMax()", 4*Value, "The maximum of the exported spectrum is not changed", Settings.GetExportSpectrumMax(), 2*Value, 0.0) && Passed;
    Passed = EvaluateNear("SetSpectrumMax()", 4*Value, "The minimum of the spectrum is not changed", Settings.GetSpectrumMin(), 3*Value, 0.0) && Passed;
  }

  for (const MString& Name : { MString("Other.dat"), MString(""), MString("/with/path and space.dat") }) {
    Settings.SetExportSpectrumFileName(Name);
    Passed = Evaluate("SetExportSpectrumFileName()", Name, "The name is stored", Settings.GetExportSpectrumFileName(), Name) && Passed;
  }

  // Each flag on its own: all others keep the opposite value
  for (bool Flag : { true, false, true }) {
    Settings.SetExportSpectrumLog(Flag);
    Settings.SetSpectrumBefore(!Flag);
    Settings.SetSpectrumAfter(Flag);
    Settings.SetSpectrumSortByInstrument(!Flag);
    Settings.SetSpectrumSortByDetectorType(Flag);
    Settings.SetSpectrumSortByNamedDetector(!Flag);
    Settings.SetSpectrumSortByDetector(Flag);
    Settings.SetSpectrumOutputToScreen(!Flag);
    Settings.SetSpectrumOutputToFile(Flag);
    Settings.SetSpectrumCombine(!Flag);
    Settings.SetSpectrumTotalDeposit(Flag);
    Settings.SetSpectrumLog(!Flag);
    Passed = Evaluate("SetExportSpectrumLog()", Flag, "The flag is stored", Settings.GetExportSpectrumLog(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumBefore()", !Flag, "The flag is stored", Settings.GetSpectrumBefore(), !Flag) && Passed;
    Passed = Evaluate("SetSpectrumAfter()", Flag, "The flag is stored", Settings.GetSpectrumAfter(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumSortByInstrument()", !Flag, "The flag is stored", Settings.GetSpectrumSortByInstrument(), !Flag) && Passed;
    Passed = Evaluate("SetSpectrumSortByDetectorType()", Flag, "The flag is stored", Settings.GetSpectrumSortByDetectorType(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumSortByNamedDetector()", !Flag, "The flag is stored", Settings.GetSpectrumSortByNamedDetector(), !Flag) && Passed;
    Passed = Evaluate("SetSpectrumSortByDetector()", Flag, "The flag is stored", Settings.GetSpectrumSortByDetector(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumOutputToScreen()", !Flag, "The flag is stored", Settings.GetSpectrumOutputToScreen(), !Flag) && Passed;
    Passed = Evaluate("SetSpectrumOutputToFile()", Flag, "The flag is stored", Settings.GetSpectrumOutputToFile(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumCombine()", !Flag, "The flag is stored", Settings.GetSpectrumCombine(), !Flag) && Passed;
    Passed = Evaluate("SetSpectrumTotalDeposit()", Flag, "The flag is stored", Settings.GetSpectrumTotalDeposit(), Flag) && Passed;
    Passed = Evaluate("SetSpectrumLog()", !Flag, "The flag is stored", Settings.GetSpectrumLog(), !Flag) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a write and read round trip with typical values
bool UTSettingsRevan::TestRoundTrip()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("RoundTrip.cfg");

  MSettingsRevan Source(false);
  FillSettings(Source);
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;
  Passed = EvaluateTrue("Write()", FileName, "The file exists", MFile::Exists(FileName)) && Passed;

  MSettingsRevan Target(false);
  Passed = EvaluateTrue("Read()", FileName, "The file can be read", ReadSilently(Target, FileName)) && Passed;
  Passed = CheckFilled(Target, "round trip") && Passed;

  // The other way around for the flags: the defaults are read over the filled values
  const MString Defaults = GetTemporaryFileName("Defaults.cfg");
  MSettingsRevan DefaultSource(false);
  Passed = EvaluateTrue("Write()", Defaults, "The file can be written", DefaultSource.Write(Defaults)) && Passed;
  MSettingsRevan DefaultTarget(false);
  FillSettings(DefaultTarget);
  Passed = EvaluateTrue("Read()", Defaults, "The file can be read", ReadSilently(DefaultTarget, Defaults)) && Passed;
  Passed = CheckDefaults(DefaultTarget, "defaults read over other values") && Passed;

  // The flag for the spectrum before the reconstruction: false is not the default
  const MString Before = GetTemporaryFileName("Before.cfg");
  MSettingsRevan BeforeSource(false);
  BeforeSource.SetSpectrumBefore(false);
  Passed = EvaluateTrue("Write()", Before, "The file can be written", BeforeSource.Write(Before)) && Passed;
  MSettingsRevan BeforeTarget(false);
  Passed = EvaluateTrue("Read()", Before, "The file can be read", ReadSilently(BeforeTarget, Before)) && Passed;
  // Defect REV-D-14: The flag is written as node "SpectrumBefore)" with a closing bracket and read as "SpectrumBefore", so it is never restored
  Passed = ExpectedToFail(EvaluateFalse("Read()", "spectrum before false", "The value of the file is used", BeforeTarget.GetSpectrumBefore())) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("Write()", "spectrum before", "No node name has a bracket", ReadTextFile(Before).Contains("SpectrumBefore)"))) && Passed;

  // The special mode and the saving of the origin information come only from the command line and are not part of the file
  MSettingsRevan Special(false);
  Special.SetSpecialMode(true);
  Special.SetSaveOI(true);
  const MString SpecialFile = GetTemporaryFileName("Special.cfg");
  Passed = EvaluateTrue("Write()", SpecialFile, "The file can be written", Special.Write(SpecialFile)) && Passed;
  Passed = EvaluateFalse("Write()", SpecialFile, "The file has no special mode", ReadTextFile(SpecialFile).Contains("SpecialMode")) && Passed;
  Passed = EvaluateFalse("Write()", SpecialFile, "The file has no save origin information flag", ReadTextFile(SpecialFile).Contains("SaveOI")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a write and read round trip with edge cases
bool UTSettingsRevan::TestRoundTripEdgeCases()
{
  bool Passed = true;

  const MString FileName = GetTemporaryFileName("Edge.cfg");

  MSettingsRevan Source(false);
  Source.SetExportSpectrumBins(-2147483647);
  Source.SetSpectrumBins(2147483647);
  Source.SetExportSpectrumMin(0.123456789012345);
  Source.SetExportSpectrumMax(-9876.54321098765);
  Source.SetSpectrumMin(1.23456789012345E-20);
  Source.SetSpectrumMax(1E30);
  Source.SetExportSpectrumFileName("Name with spaces & <special> characters.dat");
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;

  MSettingsRevan Target(false);
  Passed = EvaluateTrue("Read()", FileName, "The file can be read", ReadSilently(Target, FileName)) && Passed;
  Passed = Evaluate("Read()", "negative number of bins", "The value of the file is used", Target.GetExportSpectrumBins(), -2147483647) && Passed;
  Passed = Evaluate("Read()", "largest number of bins", "The value of the file is used", Target.GetSpectrumBins(), 2147483647) && Passed;
  // The numbers are written with 15 significant digits
  Passed = EvaluateNear("Read()", "precise minimum", "The value has 15 digits", Target.GetExportSpectrumMin(), 0.123456789012345, 1E-15) && Passed;
  Passed = EvaluateNear("Read()", "negative maximum", "The value has 15 digits", Target.GetExportSpectrumMax(), -9876.54321098765, 1E-10) && Passed;
  Passed = EvaluateNear("Read()", "tiny minimum", "The value has 15 digits", Target.GetSpectrumMin(), 1.23456789012345E-20, 1E-34) && Passed;
  Passed = EvaluateNear("Read()", "huge maximum", "Large values are not lost", Target.GetSpectrumMax(), 1E30, 1E16) && Passed;
  Passed = Evaluate("Read()", "name with special characters", "The name is identical", Target.GetExportSpectrumFileName(), MString("Name with spaces & <special> characters.dat")) && Passed;

  // An empty name
  const MString Empty = GetTemporaryFileName("Empty.cfg");
  MSettingsRevan EmptySource(false);
  EmptySource.SetExportSpectrumFileName("");
  Passed = EvaluateTrue("Write()", Empty, "The file can be written", EmptySource.Write(Empty)) && Passed;
  MSettingsRevan EmptyTarget(false);
  ReadSilently(EmptyTarget, Empty);
  Passed = Evaluate("Read()", "empty name", "An empty name stays empty", EmptyTarget.GetExportSpectrumFileName(), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the paths in the settings file
bool UTSettingsRevan::TestPathCleaning()
{
  bool Passed = true;

  // The MEGAlib path goes as variable into the file
  MString Expanded = "$(MEGALIB)/some/file.dat";
  MFile::ExpandFileName(Expanded);
  const MString FileName = GetTemporaryFileName("Path.cfg");
  MSettingsRevan Source(false);
  Source.SetExportSpectrumFileName(Expanded);
  Passed = EvaluateTrue("Write()", FileName, "The file can be written", Source.Write(FileName)) && Passed;
  Passed = EvaluateTrue("Write()", FileName, "The path has the variable", ReadTextFile(FileName).Contains("<ExportSpectrumFileName>$(MEGALIB)/some/file.dat</ExportSpectrumFileName>")) && Passed;

  // Other paths are not touched
  const MString Other = GetTemporaryFileName("OtherPath.cfg");
  MSettingsRevan OtherSource(false);
  OtherSource.SetExportSpectrumFileName("/not/megalib/file.dat");
  Passed = EvaluateTrue("Write()", Other, "The file can be written", OtherSource.Write(Other)) && Passed;
  Passed = EvaluateTrue("Write()", Other, "The path is not changed", ReadTextFile(Other).Contains("<ExportSpectrumFileName>/not/megalib/file.dat</ExportSpectrumFileName>")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the reading of incomplete and wrong files
bool UTSettingsRevan::TestReadFailures()
{
  bool Passed = true;

  // No file: the values of a failed read have to stay
  {
    MSettingsRevan Settings(false);
    FillSettings(Settings);
    Passed = EvaluateFalse("Read()", "missing file", "A missing file is a failure", ReadSilently(Settings, GetTemporaryFileName("DoesNotExist.cfg"))) && Passed;
    Passed = CheckFilled(Settings, "missing file") && Passed;
  }

  // Empty file
  {
    const MString File = GetTemporaryFileName("EmptyFile.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, "")) && Passed;
    MSettingsRevan Settings(false);
    FillSettings(Settings);
    Passed = EvaluateFalse("Read()", "empty file", "An empty file is a failure", ReadSilently(Settings, File)) && Passed;
    Passed = CheckFilled(Settings, "empty file") && Passed;
  }

  // Settings of another program
  {
    const MString File = GetTemporaryFileName("WrongProgram.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, "<GeomegaConfigurationFile>\n<SpectrumBins>5</SpectrumBins>\n</GeomegaConfigurationFile>\n")) && Passed;
    MSettingsRevan Settings(false);
    FillSettings(Settings);
    Passed = EvaluateFalse("Read()", "settings of another program", "The master node is checked", ReadSilently(Settings, File)) && Passed;
    Passed = CheckFilled(Settings, "settings of another program") && Passed;
  }

  // An incomplete file only changes what it contains
  {
    const MString File = GetTemporaryFileName("Incomplete.cfg");
    Passed = EvaluateTrue("WriteTextFile()", File, "The fixture can be written", WriteTextFile(File, GetFileContent("<SpectrumBins>5</SpectrumBins>\n<SpectrumBefore>false</SpectrumBefore>\n<UnknownNode>1</UnknownNode>\n<CSRAlgorithm>3</CSRAlgorithm>\n"))) && Passed;
    MSettingsRevan Settings(false);
    Passed = EvaluateTrue("Read()", "incomplete file", "The file can be read", ReadSilently(Settings, File)) && Passed;
    Passed = Evaluate("Read()", "incomplete file", "The value in the file is used", Settings.GetSpectrumBins(), 5) && Passed;
    Passed = EvaluateFalse("Read()", "incomplete file", "The node SpectrumBefore is read", Settings.GetSpectrumBefore()) && Passed;
    Passed = Evaluate("Read()", "incomplete file", "The event reconstruction value in the file is used", Settings.GetCSRAlgorithm(), 3) && Passed;
    Passed = Evaluate("Read()", "incomplete file", "The value not in the file stays", Settings.GetExportSpectrumBins(), 100) && Passed;
    Passed = EvaluateTrue("Read()", "incomplete file", "The value not in the file stays", Settings.GetSpectrumCombine()) && Passed;
    Passed = EvaluateNear("Read()", "incomplete file", "The value not in the file stays", Settings.GetSpectrumMax(), 1000.0, 0.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the automatic loading and the writing by the destructor
bool UTSettingsRevan::TestAutoLoadAndDestructor()
{
  bool Passed = true;

  // Start without the file in the private HOME
  MString DefaultFile = "~/.revan.cfg";
  MFile::ExpandFileName(DefaultFile);
  RemoveTemporaryFile(DefaultFile);

  // The destructor writes the settings into the default file
  {
    MSettingsRevan Settings(false);
    FillSettings(Settings);
  }
  Passed = EvaluateTrue("~MSettingsRevan()", "private HOME", "The destructor writes the default settings file", MFile::Exists(DefaultFile)) && Passed;

  // The constructor reads it again
  {
    DisableDefaultStreams();
    MSettingsRevan Settings(true);
    EnableDefaultStreams();
    Passed = CheckFilled(Settings, "auto load") && Passed;
    Passed = Evaluate("MSettingsRevan(true)", "settings file", "The expanded file name is stored", Settings.GetSettingsFileName(), DefaultFile) && Passed;
  }

  // Without auto load it is not read
  {
    MSettingsRevan Settings(false);
    Passed = CheckDefaults(Settings, "no auto load with a settings file") && Passed;
  }

  // The destructor writes the default file also after another file was read
  {
    const MString Other = GetTemporaryFileName("OtherSettings.cfg");
    MSettingsRevan Source(false);
    Source.SetSpectrumBins(77);
    Source.Write(Other);
    RemoveTemporaryFile(DefaultFile);
    {
      MSettingsRevan Settings(false);
      ReadSilently(Settings, Other);
      Passed = Evaluate("GetSettingsFileName()", Other, "The name of the read file is stored", Settings.GetSettingsFileName(), Other) && Passed;
    }
    Passed = EvaluateTrue("~MSettingsRevan()", "after reading another file", "The destructor writes the default settings file", MFile::Exists(DefaultFile)) && Passed;
    Passed = EvaluateTrue("~MSettingsRevan()", "after reading another file", "The values of the other file are in the default settings file", ReadTextFile(DefaultFile).Contains("<SpectrumBins>77</SpectrumBins>")) && Passed;
  }

  // The auto load without file uses the defaults
  RemoveTemporaryFile(DefaultFile);
  {
    DisableDefaultStreams();
    MSettingsRevan Settings(true);
    EnableDefaultStreams();
    Passed = CheckDefaults(Settings, "auto load without settings file") && Passed;
  }
  RemoveTemporaryFile(DefaultFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the suffix of the file and the name of the settings file
bool UTSettingsRevan::TestFileNames()
{
  bool Passed = true;

  // The suffix is added
  const MString NoSuffix = GetTemporaryFileName("NoSuffix");
  MSettingsRevan Settings(false);
  FillSettings(Settings);
  Passed = EvaluateTrue("Write()", NoSuffix, "The file can be written", Settings.Write(NoSuffix)) && Passed;
  Passed = EvaluateFalse("Write()", NoSuffix, "No file without the suffix exists", MFile::Exists(NoSuffix)) && Passed;
  Passed = EvaluateTrue("Write()", NoSuffix, "The suffix cfg is added", MFile::Exists(NoSuffix + ".cfg")) && Passed;

  // The reading stores the name of the file
  MSettingsRevan Reader(false);
  ReadSilently(Reader, NoSuffix + ".cfg");
  Passed = Evaluate("GetSettingsFileName()", NoSuffix, "The name of the read file is stored", Reader.GetSettingsFileName(), NoSuffix + ".cfg") && Passed;

  // The master node is the one of revan
  Passed = EvaluateTrue("Write()", NoSuffix, "The master node is RevanConfigurationFile", ReadTextFile(NoSuffix + ".cfg").Contains("<RevanConfigurationFile>")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test repeated reads with one object
bool UTSettingsRevan::TestReuse()
{
  bool Passed = true;

  const MString Filled = GetTemporaryFileName("ReuseFilled.cfg");
  const MString Defaults = GetTemporaryFileName("ReuseDefaults.cfg");
  {
    MSettingsRevan Source(false);
    FillSettings(Source);
    Source.Write(Filled);
    MSettingsRevan DefaultSource(false);
    DefaultSource.Write(Defaults);
  }

  // One object reads the two files several times: nothing accumulates, e.g. in the list of the electron tracking detectors
  MSettingsRevan Settings(false);
  ReadSilently(Settings, Filled);
  ReadSilently(Settings, Filled);
  Passed = CheckFilled(Settings, "filled file twice") && Passed;
  ReadSilently(Settings, Defaults);
  Passed = CheckDefaults(Settings, "default file after the filled file") && Passed;
  Passed = Evaluate("Read()", "default file after the filled file", "The electron tracking detectors are removed", Settings.GetNElectronTrackingDetectors(), 0U) && Passed;
  ReadSilently(Settings, Filled);
  Passed = CheckFilled(Settings, "filled file after the default file") && Passed;

  // A failed read in between changes nothing
  Passed = EvaluateFalse("Read()", "missing file in between", "A missing file is a failure", ReadSilently(Settings, GetTemporaryFileName("DoesNotExist.cfg"))) && Passed;
  Passed = CheckFilled(Settings, "after the missing file") && Passed;

  // Writing the same object twice gives the same file
  const MString Again = GetTemporaryFileName("ReuseAgain.cfg");
  Settings.Write(Again);
  const MString First = ReadTextFile(Again);
  Settings.Write(Again);
  Passed = EvaluateTrue("Write()", "second write", "The file does not change", ReadTextFile(Again) == First) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsRevan Test;
  return Test.Run() == true ? 0 : 1;
}


// UTSettingsRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
