/*
 * UTFileDecay.cxx
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
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MFileDecay.h"
#include "MString.h"
#include "MUnitTest.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the reader of decay files MFileDecay
class UTFileDecay : public MUnitTest
{
public:
  //! Default constructor
  UTFileDecay() : MUnitTest("UTFileDecay") {}
  //! Default destructor
  virtual ~UTFileDecay() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test a new reader
  bool TestConstruction();
  //! Test reading a typical file
  bool TestOpen();
  //! Test the version and type lines
  bool TestHeader();
  //! Test lines which are no complete decay lines
  bool TestIncompleteLines();
  //! Test files which cannot be read or which have no decays
  bool TestFailures();
  //! Test reading several files with one reader
  bool TestReuse();
  //! Test the decay file of the examples
  bool TestExampleFile();

  //! Write the content into a file of the test and open it with the reader, the streams are silenced
  bool OpenContent(MFileDecay& Reader, const MString& Name, const MString& Content);
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTFileDecay::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestOpen() && Passed;
  Passed = TestHeader() && Passed;
  Passed = TestIncompleteLines() && Passed;
  Passed = TestFailures() && Passed;
  Passed = TestReuse() && Passed;
  Passed = TestExampleFile() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the content into a file of the test and open it with the reader, the streams are silenced
bool UTFileDecay::OpenContent(MFileDecay& Reader, const MString& Name, const MString& Content)
{
  const MString FileName = GetTemporaryFileName(Name);
  if (WriteTextFile(FileName, Content) == false) {
    return false;
  }

  DisableDefaultStreams();
  bool Opened = Reader.Open(FileName);
  EnableDefaultStreams();
  Reader.Close();

  return Opened;
}


////////////////////////////////////////////////////////////////////////////////


//! Test a new reader
bool UTFileDecay::TestConstruction()
{
  bool Passed = true;

  MFileDecay Reader;
  Passed = EvaluateSize("MFileDecay()", "default", "A new reader has no energies", Reader.GetEnergies().size(), 0) && Passed;
  Passed = EvaluateSize("MFileDecay()", "default", "A new reader has no energy errors", Reader.GetEnergyErrors().size(), 0) && Passed;
  Passed = EvaluateFalse("MFileDecay()", "default", "A new reader is not open", Reader.IsOpen()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading a typical file
bool UTFileDecay::TestOpen()
{
  bool Passed = true;

  MString Content;
  Content += "# A decay file\n";
  Content += "\n";
  Content += "Type Decay\n";
  Content += "Version 1\n";
  Content += "\n";
  Content += "DE 511.0 1.0\n";
  Content += "DE    1460.82    2.5\n";
  Content += "DE 0.125 0\n";
  Content += "DE 1e3 1.5e-1\n";

  MFileDecay Reader;
  Passed = EvaluateTrue("Open()", "four decays", "The file is read", OpenContent(Reader, "Typical.decay", Content)) && Passed;

  vector<double> Energies = Reader.GetEnergies();
  vector<double> Errors = Reader.GetEnergyErrors();
  Passed = EvaluateSize("GetEnergies()", "four decays", "There are four energies", Energies.size(), 4) && Passed;
  Passed = EvaluateSize("GetEnergyErrors()", "four decays", "There are four energy errors", Errors.size(), 4) && Passed;
  if (Energies.size() == 4 && Errors.size() == 4) {
    // The values are exact in double precision or parsed with full precision
    Passed = EvaluateNear("GetEnergies()", "DE 511.0 1.0", "The first energy is 511 keV", Energies[0], 511.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "DE 511.0 1.0", "The first error is 1 keV", Errors[0], 1.0, 1e-15) && Passed;
    Passed = EvaluateNear("GetEnergies()", "DE 1460.82 2.5 with several spaces", "The second energy is 1460.82 keV", Energies[1], 1460.82, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "DE 1460.82 2.5 with several spaces", "The second error is 2.5 keV", Errors[1], 2.5, 1e-15) && Passed;
    Passed = EvaluateNear("GetEnergies()", "DE 0.125 0", "The third energy is 0.125 keV", Energies[2], 0.125, 1e-15) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "DE 0.125 0", "The third error is zero", Errors[2], 0.0, 0.0) && Passed;
    Passed = EvaluateNear("GetEnergies()", "DE 1e3 1.5e-1", "The exponent notation is read: 1000 keV", Energies[3], 1000.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "DE 1e3 1.5e-1", "The exponent notation is read: 0.15 keV", Errors[3], 0.15, 1e-15) && Passed;
  }

  // Repeated reads of the getters give the same result
  Passed = EvaluateSize("GetEnergies()", "second call", "The energies stay", Reader.GetEnergies().size(), 4) && Passed;

  // A single decay
  MFileDecay Single;
  Passed = EvaluateTrue("Open()", "one decay", "The file is read", OpenContent(Single, "Single.decay", "Type Decay\nVersion 1\nDE 661.657 0.5\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "one decay", "There is one energy", Single.GetEnergies().size(), 1) && Passed;
  if (Single.GetEnergies().size() == 1 && Single.GetEnergyErrors().size() == 1) {
    Passed = EvaluateNear("GetEnergies()", "DE 661.657 0.5", "The energy is 661.657 keV", Single.GetEnergies()[0], 661.657, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "DE 661.657 0.5", "The error is 0.5 keV", Single.GetEnergyErrors()[0], 0.5, 1e-15) && Passed;
  }

  // A file without a new line at the end
  MFileDecay NoNewLine;
  Passed = EvaluateTrue("Open()", "no new line at the end", "The file is read", OpenContent(NoNewLine, "NoNewLine.decay", "Type Decay\nVersion 1\nDE 511 1\nDE 1022 2")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "no new line at the end", "The last line counts", NoNewLine.GetEnergies().size(), 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the version and type lines
bool UTFileDecay::TestHeader()
{
  bool Passed = true;

  // Without the header lines the file is a decay file of version 1
  MFileDecay NoHeader;
  Passed = EvaluateTrue("Open()", "no header", "A file without version and type is read", OpenContent(NoHeader, "NoHeader.decay", "DE 511 1\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "no header", "There is one energy", NoHeader.GetEnergies().size(), 1) && Passed;

  MFileDecay OnlyVersion;
  Passed = EvaluateTrue("Open()", "only version 1", "A file without type is read", OpenContent(OnlyVersion, "OnlyVersion.decay", "Version 1\nDE 511 1\n")) && Passed;
  MFileDecay OnlyType;
  Passed = EvaluateTrue("Open()", "only type", "A file without version is read", OpenContent(OnlyType, "OnlyType.decay", "Type Decay\nDE 511 1\n")) && Passed;

  // The header after the decays
  MFileDecay HeaderLast;
  Passed = EvaluateTrue("Open()", "header after the decays", "The position of the header does not matter", OpenContent(HeaderLast, "HeaderLast.decay", "DE 511 1\nVersion 1\nType Decay\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "header after the decays", "There is one energy", HeaderLast.GetEnergies().size(), 1) && Passed;

  // Only version 1 exists
  MFileDecay Version2;
  Passed = EvaluateFalse("Open()", "Version 2", "A newer version is rejected", OpenContent(Version2, "Version2.decay", "Type Decay\nVersion 2\nDE 511 1\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "Version 2", "No energies are read", Version2.GetEnergies().size(), 0) && Passed;
  MFileDecay Version0;
  Passed = EvaluateFalse("Open()", "Version 0", "Version zero is rejected", OpenContent(Version0, "Version0.decay", "Type Decay\nVersion 0\nDE 511 1\n")) && Passed;
  MFileDecay VersionNegative;
  Passed = EvaluateFalse("Open()", "Version -1", "A negative version is rejected", OpenContent(VersionNegative, "VersionNegative.decay", "Type Decay\nVersion -1\nDE 511 1\n")) && Passed;

  // Another type
  MFileDecay OtherType;
  Passed = EvaluateFalse("Open()", "Type TRA", "Another file type is rejected", OpenContent(OtherType, "OtherType.decay", "Type TRA\nVersion 1\nDE 511 1\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "Type TRA", "No energies are read", OtherType.GetEnergies().size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test lines which are no complete decay lines
bool UTFileDecay::TestIncompleteLines()
{
  bool Passed = true;

  // Only lines with the keyword and exactly two values are decays
  MString Content;
  Content += "Type Decay\n";
  Content += "Version 1\n";
  Content += "DE 100 1\n";
  Content += "DE 200\n";
  Content += "DE\n";
  Content += "DX 300 3\n";
  Content += "de 400 4\n";
  Content += "# DE 500 5\n";
  Content += "DE 600 6\n";

  MFileDecay Reader;
  Passed = EvaluateTrue("Open()", "incomplete lines", "The file is read", OpenContent(Reader, "Incomplete.decay", Content)) && Passed;
  vector<double> Energies = Reader.GetEnergies();
  Passed = EvaluateSize("GetEnergies()", "incomplete lines", "Only the two complete decay lines count", Energies.size(), 2) && Passed;
  if (Energies.size() == 2) {
    Passed = EvaluateNear("GetEnergies()", "incomplete lines", "The first decay is the one at 100 keV", Energies[0], 100.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergies()", "incomplete lines", "The second decay is the one at 600 keV", Energies[1], 600.0, 1e-12) && Passed;
  }
  Passed = EvaluateSize("GetEnergyErrors()", "incomplete lines", "There are as many errors as energies", Reader.GetEnergyErrors().size(), Energies.size()) && Passed;

  // The format of the example file: a comment behind the values
  MFileDecay Comment;
  bool Opened = OpenContent(Comment, "Comment.decay", "Type Decay\nVersion 1\nDE    511.0    1.0         // e+ annihilation\n");
  Passed = EvaluateTrue("Open()", "comment behind the values", "The decay line with a trailing comment is read", Opened) && Passed;
  Passed = EvaluateSize("GetEnergies()", "comment behind the values", "There is one energy", Comment.GetEnergies().size(), 1) && Passed;

  // A decay line which is commented out
  MFileDecay CommentedOut;
  Passed = EvaluateTrue("Open()", "decay commented out", "The file is read", OpenContent(CommentedOut, "CommentedOut.decay", "Type Decay\nVersion 1\n// DE 211.03 1.0\nDE 511 1\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "decay commented out", "The decay which is commented out does not count", CommentedOut.GetEnergies().size(), 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test files which cannot be read or which have no decays
bool UTFileDecay::TestFailures()
{
  bool Passed = true;

  // No file
  MFileDecay Missing;
  DisableDefaultStreams();
  bool Opened = Missing.Open(GetTemporaryFileName("DoesNotExist.decay"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "missing file", "A missing file is a failure", Opened) && Passed;
  Passed = EvaluateSize("GetEnergies()", "missing file", "There are no energies", Missing.GetEnergies().size(), 0) && Passed;

  // An empty file
  MFileDecay Empty;
  Passed = EvaluateFalse("Open()", "empty file", "A file without content is a failure", OpenContent(Empty, "Empty.decay", "")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "empty file", "There are no energies", Empty.GetEnergies().size(), 0) && Passed;

  // Only a header
  MFileDecay Header;
  Passed = EvaluateFalse("Open()", "only header", "A file without decays is a failure", OpenContent(Header, "Header.decay", "Type Decay\nVersion 1\n")) && Passed;

  // Only incomplete decay lines
  MFileDecay Incomplete;
  Passed = EvaluateFalse("Open()", "only incomplete decays", "A file without complete decay lines is a failure", OpenContent(Incomplete, "OnlyIncomplete.decay", "Type Decay\nVersion 1\nDE 511\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test reading several files with one reader
bool UTFileDecay::TestReuse()
{
  bool Passed = true;

  MFileDecay Reader;
  Passed = EvaluateTrue("Open()", "first file", "The first file is read", OpenContent(Reader, "First.decay", "Type Decay\nVersion 1\nDE 100 1\nDE 200 2\nDE 300 3\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "first file", "There are three energies", Reader.GetEnergies().size(), 3) && Passed;

  // A second file replaces the decays of the first one
  Passed = EvaluateTrue("Open()", "second file", "The second file is read", OpenContent(Reader, "Second.decay", "Type Decay\nVersion 1\nDE 400 4\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "second file", "There is one energy", Reader.GetEnergies().size(), 1) && Passed;
  Passed = EvaluateSize("GetEnergyErrors()", "second file", "There is one energy error", Reader.GetEnergyErrors().size(), 1) && Passed;
  if (Reader.GetEnergies().size() == 1) {
    Passed = EvaluateNear("GetEnergies()", "second file", "The energy is the one of the second file", Reader.GetEnergies()[0], 400.0, 1e-12) && Passed;
  }

  // A file without decays leaves nothing of the file before
  Passed = EvaluateFalse("Open()", "file without decays after a good one", "The file without decays is a failure", OpenContent(Reader, "ThirdEmpty.decay", "Type Decay\nVersion 1\n")) && Passed;
  Passed = EvaluateSize("GetEnergies()", "file without decays after a good one", "No energies of the file before stay", Reader.GetEnergies().size(), 0) && Passed;

  // Failures before the decays are read
  Passed = EvaluateTrue("Open()", "good file again", "The file is read", OpenContent(Reader, "Fourth.decay", "Type Decay\nVersion 1\nDE 400 4\n")) && Passed;
  bool WrongVersion = OpenContent(Reader, "FifthVersion.decay", "Type Decay\nVersion 2\nDE 500 5\n");
  Passed = EvaluateFalse("Open()", "wrong version after a good file", "The wrong version is a failure", WrongVersion) && Passed;
  // Defect REV-D-3: A failed Open (missing file, wrong type, wrong version) keeps the decays of the file read before
  Passed = ExpectedToFail(EvaluateSize("GetEnergies()", "wrong version after a good file", "No energies of the file before stay", Reader.GetEnergies().size(), 0)) && Passed;

  Passed = EvaluateTrue("Open()", "good file again", "The file is read", OpenContent(Reader, "Sixth.decay", "Type Decay\nVersion 1\nDE 400 4\n")) && Passed;
  DisableDefaultStreams();
  bool Missing = Reader.Open(GetTemporaryFileName("DoesNotExist.decay"));
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "missing file after a good file", "The missing file is a failure", Missing) && Passed;
  Passed = ExpectedToFail(EvaluateSize("GetEnergies()", "missing file after a good file", "No energies of the file before stay", Reader.GetEnergies().size(), 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the decay file of the examples
bool UTFileDecay::TestExampleFile()
{
  bool Passed = true;

  MString FileName = "$(MEGALIB)/resource/examples/response/Decays.decay";
  MFile::ExpandFileName(FileName);
  Passed = EvaluateTrue("Exists()", "Decays.decay", "The example decay file exists", MFile::Exists(FileName)) && Passed;

  // The example has one active line: "DE 511.0 1.0 // e+ annihilation", all others are commented out
  MFileDecay Reader;
  DisableDefaultStreams();
  bool Opened = Reader.Open(FileName);
  EnableDefaultStreams();
  Reader.Close();
  Passed = EvaluateTrue("Open()", "Decays.decay", "The example decay file is read", Opened) && Passed;
  Passed = EvaluateSize("GetEnergies()", "Decays.decay", "The example has one decay", Reader.GetEnergies().size(), 1) && Passed;
  if (Reader.GetEnergies().size() == 1 && Reader.GetEnergyErrors().size() == 1) {
    Passed = EvaluateNear("GetEnergies()", "Decays.decay", "The decay is the 511 keV line", Reader.GetEnergies()[0], 511.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetEnergyErrors()", "Decays.decay", "The energy window is 1 keV", Reader.GetEnergyErrors()[0], 1.0, 1e-15) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileDecay Test;
  return Test.Run() == true ? 0 : 1;
}


// UTFileDecay.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
