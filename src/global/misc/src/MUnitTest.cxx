/*
 * MUnitTest.cxx
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


// Include the header:
#include "MUnitTest.h"

// Standard libs:
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

// POSIX libs:
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:
#include "MFile.h"
#include "MRotation.h"
#include "MSettingsTesting.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MUnitTest)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MUnitTest::MUnitTest(const MString& Name)
{
  m_Name = Name;
  m_TemporaryBaseName = CreateTemporaryDirectoryBaseName(Name);

  LoadLogDirectory();

  m_NumberOfPassedTests = 0;
  m_NumberOfFailedTests = 0;
  m_TemporaryRootLock = -1;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
  if (m_TemporaryRootDirectory.IsEmpty() == true) {
    ReleaseTemporaryRootLock();
    return;
  }

  // Keep the files of a failed test for inspection
  if (m_NumberOfFailedTests > 0) {
    mout<<"The files of the failed test "<<m_Name<<" are kept for inspection: "<<m_TemporaryRootDirectory<<endl;
    ReleaseTemporaryRootLock();
    return;
  }

  if (RemoveTemporaryDirectory() == false) {
    merr<<"Error in MUnitTest::~MUnitTest: unable to remove the private temporary root: "<<m_TemporaryRootDirectory<<endl;
  }
  ReleaseTemporaryRootLock();
}


////////////////////////////////////////////////////////////////////////////////


//! Summarize the test run
void MUnitTest::Summarize()
{
  mout<<"Unit test: "<<m_Name<<endl;
  mout<<"Passed tests: "<<m_NumberOfPassedTests<<endl;
  mout<<"Failed tests: "<<m_NumberOfFailedTests<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the numbers of passed and failed tests from the output of Summarize (the last "Passed tests: N" followed by "Failed tests: M"), return false if there are none
bool MUnitTest::ParseSummary(const MString& Output, unsigned int& Passed, unsigned int& Failed)
{
  Passed = 0;
  Failed = 0;

  const string Text = Output.Data();
  const string PassedLabel = "Passed tests:";
  const string FailedLabel = "Failed tests:";
  const string::size_type PassedPosition = Text.rfind(PassedLabel);
  const string::size_type FailedPosition = Text.rfind(FailedLabel);
  if (PassedPosition == string::npos || FailedPosition == string::npos || FailedPosition < PassedPosition) {
    return false;
  }

  // The number follows the label after optional blanks
  unsigned int Numbers[2] = { 0, 0 };
  const string::size_type Positions[2] = { PassedPosition + PassedLabel.size(), FailedPosition + FailedLabel.size() };
  for (unsigned int n = 0; n < 2; ++n) {
    string::size_type Position = Positions[n];
    while (Position < Text.size() && (Text[Position] == ' ' || Text[Position] == '\t')) {
      ++Position;
    }
    char* End = nullptr;
    const unsigned long Value = strtoul(Text.c_str() + Position, &End, 10);
    if (End == Text.c_str() + Position) {
      return false;
    }
    Numbers[n] = static_cast<unsigned int>(Value);
  }

  Passed = Numbers[0];
  Failed = Numbers[1];
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Write complete text content to a test fixture file
bool MUnitTest::WriteTextFile(const MString& FileName, const MString& Content) const
{
  // Lock to prevent a concurrent teardown of the temporary directory
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  if (IsSafeTemporaryPath(FileName, false) == false) {
    merr<<"Error in MUnitTest::WriteTextFile: unsafe temporary file path: "<<FileName<<endl;
    return false;
  }

  ofstream Out(FileName.Data());
  if (Out.is_open() == false) {
    merr<<"Error in MUnitTest::WriteTextFile: unable to open temporary file: "<<FileName<<endl;
    return false;
  }

  Out<<Content;
  Out.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate that two text files are equivalent, numbers may differ in the last digits
bool MUnitTest::EvaluateFilesNumericallyEquivalent(MString Function, MString Input, MString Description,
                                                   const MString& TestFile, const MString& ReferenceFile,
                                                   unsigned int MaximumLastDigitDifference)
{
  // Open the files
  ifstream TestStream(TestFile.Data());
  if (TestStream.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("test file opens: ") + TestFile.Data(),
                    "cannot open test file");
    return false;
  }

  ifstream ReferenceStream(ReferenceFile.Data());
  if (ReferenceStream.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("reference file opens: ") + ReferenceFile.Data(),
                    "cannot open reference file");
    return false;
  }

  // Read both files in synchronized so line count and line content are checked together
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    // Reaching the end of both files at the same time completes the comparison
    if (GotTest == false && GotReference == false) {
      break;
    }

    ++LineNumber;

    // If only one read succeeded, the files contain a different number of lines
    if (GotTest != GotReference) {
      ostringstream LengthOutput;
      if (GotTest == false) {
        LengthOutput<<"test file is shorter (ends at line "<<LineNumber<<")";
      } else {
        LengthOutput<<"test file is longer (reference ends at line "<<LineNumber<<")";
      }
      RegisterFailure(Function, Input, Description + MString(" (line count)"),
                      "same number of lines", LengthOutput.str());
      return false;
    }

    // Stop at the first unequal line and report both complete lines for diagnosis
    if (LinesMatchNumerically(TestLine, ReferenceLine, MaximumLastDigitDifference) == false) {
      ostringstream Diff;
      Diff<<endl<<"      line "<<LineNumber<<":"
           <<endl<<"        expected:  "<<ReferenceLine
           <<endl<<"        test:      "<<TestLine;
      RegisterFailure(Function, Input, Description, "numerically equivalent files", Diff.str());
      return false;
    }
  }

  RegisterSuccess();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if two lines have the same tokens, numbers may differ in the last digits
bool MUnitTest::LinesMatchNumerically(const MString& TestLine, const MString& ReferenceLine,
                                      unsigned int MaximumLastDigitDifference) const
{
  // Stream extraction splits both lines at whitespace and ignores whitespace differences
  istringstream TestStream(TestLine.ToString());
  istringstream ReferenceStream(ReferenceLine.ToString());

  MString TestToken;
  MString ReferenceToken;
  while (true) {
    const bool HasTestToken = static_cast<bool>(TestStream>>TestToken);
    const bool HasReferenceToken = static_cast<bool>(ReferenceStream>>ReferenceToken);

    // If either stream is exhausted, both must be exhausted to have equal token counts
    if (HasTestToken == false || HasReferenceToken == false) {
      if (HasTestToken == HasReferenceToken) {
        return true;
      }
      return false;
    }

    // Identical tokens match directly, including non-numeric and integer-only tokens
    if (TestToken == ReferenceToken) {
      continue;
    }

    // Differing tokens must be floating-point numbers matching within their printed precision
    if (TestToken.AreNumbersNumericallyMatching(ReferenceToken, MaximumLastDigitDifference) == false) {
      return false;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read complete text content from a test fixture file, return an empty string on failure
MString MUnitTest::ReadTextFile(const MString& FileName) const
{
  // Keep validation and reading atomic with respect to concurrent teardown, i.e.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  if (IsSafeTemporaryPath(FileName, false) == false) {
    merr<<"Error in MUnitTest::ReadTextFile: unsafe temporary file path: "<<FileName<<endl;
    return "";
  }

  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    merr<<"Error in MUnitTest::ReadTextFile: unable to open temporary file: "<<FileName<<endl;
    return "";
  }

  return Content;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a process-local temporary file name for this test, the file is not created
MString MUnitTest::GetTemporaryFileName(const MString& Name) const
{
  // Lock to keep the randomized temporary root stable
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Use plain names directly below the private root
  if (IsValidTemporaryPathName(Name) == false) {
    merr<<"Error in MUnitTest::GetTemporaryFileName: invalid temporary file name: "<<Name<<endl;
    return "";
  }

  const MString Root = GetTemporaryRootDirectory();
  if (Root.IsEmpty() == true) {
    merr<<"Error in MUnitTest::GetTemporaryFileName: unable to create the randomized temporary root"<<endl;
    return "";
  }

  return Root + "/" + Name;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a process-local temporary directory name for this test, the directory is not created
MString MUnitTest::GetTemporaryDirectoryName(const MString& Name) const
{
  // Lock to keep the randomized temporary root stable
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // An empty name selects the private root itself. Named directories are
  // direct children of that root.
  if (IsValidTemporaryPathName(Name, true) == false) {
    merr<<"Error in MUnitTest::GetTemporaryDirectoryName: invalid temporary directory name: "<<Name<<endl;
    return "";
  }

  const MString Root = GetTemporaryRootDirectory();
  if (Root.IsEmpty() == true) {
    merr<<"Error in MUnitTest::GetTemporaryDirectoryName: unable to create the randomized temporary root"<<endl;
    return "";
  }
  if (Name.IsEmpty() == true) {
    return Root;
  }

  return Root + "/" + Name;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove and recreate a process-local temporary directory for this test
bool MUnitTest::PrepareTemporaryDirectory(const MString& Name) const
{
  // Keep validation and recreation atomic with respect to concurrent access, i.e.
  // make sure no other temporary-path operation runs while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Recreate only directories generated below this test's private root.
  const MString Directory = GetTemporaryDirectoryName(Name);
  if (IsSafeTemporaryPath(Directory, true) == false) {
    merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unsafe temporary directory path: "<<Directory<<endl;
    return false;
  }

  std::error_code Error;
  if (Name.IsEmpty() == true) {
    // The root itself: only clear its content, a new directory would not have the lock which the test driver respects
    for (const std::filesystem::directory_entry& Entry : std::filesystem::directory_iterator(Directory.Data(), Error)) {
      std::filesystem::remove_all(Entry.path(), Error);
      if (Error.value() != 0) {
        merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to clear the temporary root: "<<Directory<<endl;
        return false;
      }
    }
    return true;
  }

  std::filesystem::remove_all(Directory.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to remove existing temporary directory: "<<Directory<<endl;
    return false;
  }

  if (std::filesystem::create_directories(Directory.Data(), Error) == false || Error.value() != 0) {
    merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to create temporary directory: "<<Directory<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove a temporary file only if it is inside this test's private temporary root
bool MUnitTest::RemoveTemporaryFile(const MString& FileName) const
{
  // Keep validation and removal atomic with respect to concurrent access, i.e.
  // make sure no other temporary-path operation runs while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Refuse paths outside this test's randomized private root, including
  // paths which escape via ".." or symlinks.
  if (IsSafeTemporaryPath(FileName, false) == false) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: unsafe temporary file path: "<<FileName<<endl;
    return false;
  }

  std::error_code Error;
  if (std::filesystem::exists(FileName.Data(), Error) == false) {
    if (Error.value() != 0) {
      merr<<"Error in MUnitTest::RemoveTemporaryFile: unable to check if temporary file exists: "<<FileName<<endl;
      return false;
    }
    return true;
  }

  // Directories must be removed only through RemoveTemporaryDirectory().
  if (MFile::IsDirectory(FileName) == true) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: file is actually a directory: "<<FileName<<endl;
    return false;
  }

  std::filesystem::remove(FileName.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: unable to remove temporary file: "<<FileName<<endl;
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Recursively remove a temporary directory only if it is inside this test's private temporary root
bool MUnitTest::RemoveTemporaryDirectory(const MString& DirectoryName) const
{
  // Keep validation and removal atomic with respect to concurrent access, i.e.
  // make sure no other temporary-path operation runs while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // If no directory name is given, uses m_TemporaryRootDirectory.
  MString Path = DirectoryName;
  if (DirectoryName.IsEmpty() == true) {
    Path = m_TemporaryRootDirectory;
  }
  if (Path.IsEmpty() == true) {
    return true;
  }

  // Recursive deletion is permitted only inside this test's randomized
  // private root. Passing the private root itself is allowed for teardown.
  if (IsSafeTemporaryPath(Path, true) == false) {
    merr<<"Error in MUnitTest::RemoveTemporaryDirectory: unsafe temporary directory path: "<<Path<<endl;
    return false;
  }

  std::error_code Error;
  std::filesystem::remove_all(Path.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::RemoveTemporaryDirectory: unable to remove temporary directory: "<<Path<<endl;
    return false;
  }

  // Clear the cached root on the normal generated-path teardown. Equivalent
  // spellings such as a trailing slash self-heal on the next root access.
  if (Path == m_TemporaryRootDirectory) {
    m_TemporaryRootDirectory = "";
    ReleaseTemporaryRootLock();
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create this test's randomized private temporary root if necessary
bool MUnitTest::CreateTemporaryRootDirectory() const
{
  // Serialize lazy root creation, i.e. make sure concurrent callers share one
  // randomized temporary root instead of creating competing directories.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Reuse the existing randomized root if it still exists.
  if (m_TemporaryRootDirectory.IsEmpty() == false) {
    if (MFile::IsDirectory(m_TemporaryRootDirectory) == true) {
      return true;
    }

    m_TemporaryRootDirectory = "";
    ReleaseTemporaryRootLock();
  }

  // MFile creates the directory atomically below the log directory
  // and adds the default 10-character random component.
  const MString LogDirectory = GetLogDirectory();
  if (MFile::CreateDirectory(LogDirectory) == false) {
    return false;
  }
  m_TemporaryRootDirectory = MFile::CreateTemporaryDirectory(m_TemporaryBaseName, 10, LogDirectory);
  if (m_TemporaryRootDirectory.IsEmpty() == true) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: unable to create the private temporary root for "<<m_Name<<endl;
    return false;
  }

  std::error_code Error;
  const std::filesystem::path LogPath =
    std::filesystem::weakly_canonical(LogDirectory.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: unable to resolve the log directory"<<endl;
    m_TemporaryRootDirectory = "";
    return false;
  }

  const std::filesystem::path Root =
    std::filesystem::weakly_canonical(m_TemporaryRootDirectory.Data(), Error);
  if (Error.value() != 0 || IsPathContained(LogPath, Root) == false || Root == LogPath) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: generated root is not safely contained in the log directory: "<<m_TemporaryRootDirectory<<endl;
    m_TemporaryRootDirectory = "";
    return false;
  }

  // Lock the root as long as it exists: the test driver does not remove the roots of running tests
  m_TemporaryRootLock = open(m_TemporaryRootDirectory.Data(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
  if (m_TemporaryRootLock >= 0) {
    flock(m_TemporaryRootLock, LOCK_EX | LOCK_NB);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Release the lock on the temporary root
void MUnitTest::ReleaseTemporaryRootLock() const
{
  if (m_TemporaryRootLock >= 0) {
    close(m_TemporaryRootLock);
    m_TemporaryRootLock = -1;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the randomized private temporary root for this test
MString MUnitTest::GetTemporaryRootDirectory() const
{
  // Serialize lazy root access, i.e. make sure concurrent callers observe one
  // stable randomized temporary root.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // All generated temporary paths are rooted below this randomized directory.
  if (CreateTemporaryRootDirectory() == false) {
    return "";
  }

  return m_TemporaryRootDirectory;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true only for a plain child file or directory name without path components
bool MUnitTest::IsValidTemporaryPathName(const MString& Name, bool AllowEmpty) const
{
  // Empty names are permitted only when the caller explicitly requests
  // the test's private temporary root.
  if (Name.IsEmpty() == true) {
    return AllowEmpty;
  }

  // Temporary names must not contain path components. This rejects
  // traversal attempts such as "../home/andreas" before a path is built.
  if (Name == "." || Name == ".." || Name.Contains("/") == true || Name.Contains("\\") == true ||
      Name.GetString().find('\0') != std::string::npos) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the basename of the temporary directory: the acceptable characters of the name, or "UnitTest" if there are none
MString MUnitTest::CreateTemporaryDirectoryBaseName(const MString& Name) const
{
  // Keep the acceptable characters of the name
  MString BaseName;
  for (unsigned int c = 0; c < Name.Length(); ++c) {
    const char Character = Name[c];
    if ((Character >= 'a' && Character <= 'z') ||
        (Character >= 'A' && Character <= 'Z') ||
        (Character >= '0' && Character <= '9') ||
        Character == '_' || Character == '-') {
      BaseName += Character;
    }
  }

  // Use a default name if nothing is left
  if (BaseName.IsEmpty() == true) {
    return "UnitTest";
  }

  return BaseName;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the log directory from the testing settings file
void MUnitTest::LoadLogDirectory()
{
  // Create the settings file with the defaults if it does not exist
  MSettingsTesting Settings;
  MString FileName = Settings.GetSettingsFileName();
  MFile::ExpandFileName(FileName);
  if (MFile::Exists(FileName) == true) {
    Settings.Read(FileName);
  } else {
    MFile::CreateDirectory(MFile::GetDirectoryName(FileName));
    Settings.Write(FileName);
  }

  m_LogDirectory = Settings.GetLogDirectory();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true only if the path resolves inside this test's randomized temporary root
bool MUnitTest::IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const
{
  // An empty path must never reach a guarded filesystem operation.
  if (Path.IsEmpty() == true) {
    return false;
  }

  // Validation must never create a new temporary root as a side effect.
  // Only paths generated earlier by this test are eligible for access.
  if (m_TemporaryRootDirectory.IsEmpty() == true) {
    return false;
  }

  std::error_code Error;
  const MString LogDirectory = GetLogDirectory();

  // Resolve the log directory. This normalizes "." and ".."
  // and resolves symlinks before any containment decision is made.
  const std::filesystem::path LogPath = std::filesystem::weakly_canonical(LogDirectory.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Resolve this test's randomized private root.
  const std::filesystem::path Root = std::filesystem::weakly_canonical(m_TemporaryRootDirectory.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Resolve currently visible traversal and symlink escapes. The later
  // remove() and remove_all() calls remove symlinks themselves instead of
  // following them during deletion, which keeps the removal path safe even
  // if a symlink changes after this check.
  const std::filesystem::path Candidate = std::filesystem::weakly_canonical(Path.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Never permit an operation on the log directory itself.
  if (Root == LogPath) {
    return false;
  }

  // The randomized private root must itself remain inside the system
  // temporary directory.
  if (IsPathContained(LogPath, Root) == false) {
    return false;
  }

  // The path must be inside the private root - this rejects /tmp/root/../../home
  if (IsPathContained(Root, Candidate) == false) {
    return false;
  }

  // File removal must not remove the root. Directory removal can permit
  // this explicitly for final test cleanup.
  if (AllowRoot == false && Candidate == Root) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if Child is Parent or is contained below Parent
bool MUnitTest::IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const
{
  // A lexical relative path avoids string-prefix mistakes such as treating
  // "/tmp/test-other" as a child of "/tmp/test" and tolerates trailing
  // separators after canonicalization.
  const std::filesystem::path Relative = Child.lexically_normal().lexically_relative(Parent.lexically_normal());
  if (Relative.empty() == true) {
    return false;
  }
  if (Relative == ".") {
    return true;
  }

  const std::filesystem::path::const_iterator First = Relative.begin();
  if (First != Relative.end() && *First != "..") {
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate that two text files contain exactly the same lines
bool MUnitTest::EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& TestFile, const MString& ReferenceFile)
{
  // Open both files independently so failures identify which input is unavailable
  ifstream TestStream(TestFile.Data());
  if (TestStream.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("test file opens: ") + TestFile.Data(),
                    "cannot open test file");
    return false;
  }

  ifstream ReferenceStream(ReferenceFile.Data());
  if (ReferenceStream.is_open() == false) {
    RegisterFailure(Function, Input, Description,
                    MString("reference file opens: ") + ReferenceFile.Data(),
                    "cannot open reference file");
    return false;
  }

  // Read both files in lockstep so line count and line content are checked together
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    // Remove the carriage-return component of CRLF line endings
    if (GotTest == true && TestLine.EndsWith("\r")) {
      TestLine.RemoveLast(1);
    }
    if (GotReference == true && ReferenceLine.EndsWith("\r")) {
      ReferenceLine.RemoveLast(1);
    }

    // Reaching the end of both files at the same time completes the comparison
    if (GotTest == false && GotReference == false) {
      break;
    }

    ++LineNumber;

    // If only one read succeeded, the files contain a different number of lines
    if (GotTest != GotReference) {
      ostringstream LengthOutput;
      if (GotTest == false) {
        LengthOutput<<"test file is shorter (ends at line "<<LineNumber<<")";
      } else {
        LengthOutput<<"test file is longer (reference ends at line "<<LineNumber<<")";
      }
      RegisterFailure(Function, Input, Description + MString(" (line count)"),
                      "same number of lines", LengthOutput.str());
      return false;
    }

    // Exact comparison preserves all characters within the line, including whitespace
    // Stop at the first unequal line and report both complete lines for diagnosis
    if (TestLine != ReferenceLine) {
      ostringstream Diff;
      Diff<<endl<<"      line "<<LineNumber<<":"
           <<endl<<"        expected:  "<<ReferenceLine
           <<endl<<"        test:      "<<TestLine;
      RegisterFailure(Function, Input, Description, "identical files", Diff.str());
      return false;
    }
  }

  RegisterSuccess();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate two vectors within a given tolerance
bool MUnitTest::EvaluateVectorNear(MString Function, MString Input, MString Description, const MVector& Output, const MVector& Truth, double Tolerance)
{
  const bool Finite = std::isfinite(Output.X()) && std::isfinite(Output.Y()) && std::isfinite(Output.Z()) &&
                      std::isfinite(Truth.X()) && std::isfinite(Truth.Y()) && std::isfinite(Truth.Z());
  const double Distance = (Output - Truth).Mag();

  if (Finite == false || Distance > Tolerance) {
    ostringstream ExpectedStream;
    ExpectedStream<<setprecision(numeric_limits<long double>::max_digits10)<<"("<<Truth.X()<<", "<<Truth.Y()<<", "<<Truth.Z()<<") +/- "<<Tolerance;
    ostringstream OutputStream;
    OutputStream<<setprecision(numeric_limits<long double>::max_digits10)<<"("<<Output.X()<<", "<<Output.Y()<<", "<<Output.Z()<<"), distance "<<Distance;
    RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
    return false;
  }

  RegisterSuccess();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate two rotation matrices within a given tolerance
bool MUnitTest::EvaluateRotationNear(MString Function, MString Input, MString Description, const MRotation& Output, const MRotation& Truth, double Tolerance)
{
  // Elements row by row: XX, YX, ZX, XY, YY, ZY, XZ, YZ, ZZ
  const vector<double> O = { Output.GetXX(), Output.GetYX(), Output.GetZX(), Output.GetXY(), Output.GetYY(), Output.GetZY(), Output.GetXZ(), Output.GetYZ(), Output.GetZZ() };
  const vector<double> T = { Truth.GetXX(), Truth.GetYX(), Truth.GetZX(), Truth.GetXY(), Truth.GetYY(), Truth.GetZY(), Truth.GetXZ(), Truth.GetYZ(), Truth.GetZZ() };

  bool Match = true;
  double LargestDifference = 0.0;
  for (unsigned int i = 0; i < 9; ++i) {
    if (std::isfinite(O[i]) == false || std::isfinite(T[i]) == false) {
      Match = false;
    } else {
      LargestDifference = std::max(LargestDifference, fabs(O[i] - T[i]));
      if (fabs(O[i] - T[i]) > Tolerance) {
        Match = false;
      }
    }
  }

  if (Match == false) {
    auto Format = [](const vector<double>& V) {
      ostringstream Stream;
      Stream<<setprecision(numeric_limits<long double>::max_digits10)<<"("<<V[0]<<"/"<<V[1]<<"/"<<V[2]<<", "<<V[3]<<"/"<<V[4]<<"/"<<V[5]<<", "<<V[6]<<"/"<<V[7]<<"/"<<V[8]<<")";
      return Stream.str();
    };
    RegisterFailure(Function, Input, Description, Format(T) + " +/- " + to_string(Tolerance), Format(O) + ", largest difference " + to_string(LargestDifference));
    return false;
  }

  RegisterSuccess();
  return true;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
