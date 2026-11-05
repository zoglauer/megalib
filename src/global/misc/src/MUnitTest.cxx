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
#include <cctype>
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


//! Standard constructor
MUnitTest::MUnitTest(const MString& Name)
{
  m_Name = Name;
  m_TemporaryBaseName = CreateTemporaryDirectoryBaseName(Name);

  LoadLogDirectory();

  m_NumberOfPassedTests = 0;
  m_NumberOfFailedTests = 0;
  m_TemporaryRootLock = -1;
  m_NumberOfKnownFailures = 0;
  m_IsExpectedFailureModeActive = false;
  m_NumberOfNestedExpectedFailureModes = 0;
  m_NumberOfExpectedFailureModeChecks = 0;
  m_HasUnexpectedPass = false;

  // On the first call, create a private temporary root directory and redirect HOME and the working directory there:
  if (RedirectHomeDirectory() == false) {
    mout<<"ABORTED: HOME and the working directory are not a private temporary directory - no check was run"<<endl;
    // Clean up before exiting - the destructor will not run
    RemoveTemporaryDirectory();
    exit(1);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
  // Keep HOME and the working directory isolated until the process ends

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
  // Report an unfinished expected-failure check:
  if (m_IsExpectedFailureModeActive == true) {
    mout<<endl;
    mout<<"FAILED: ExpectedToFail()"<<endl;
    mout<<"   Description: EndExpectedFailureMode() was not reached"<<endl;
    mout<<endl;
    ++m_NumberOfFailedTests;
    m_IsExpectedFailureModeActive = false;
    m_NumberOfNestedExpectedFailureModes = 0;
  }

  mout<<"Unit test: "<<m_Name<<endl;
  mout<<"Passed tests: "<<m_NumberOfPassedTests<<endl;
  mout<<"Failed tests: "<<m_NumberOfFailedTests<<endl;
  if (m_NumberOfKnownFailures > 0) {
    mout<<"Known failures: "<<m_NumberOfKnownFailures<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Redirect HOME and the working directory to the private temporary root once per process
bool MUnitTest::RedirectHomeDirectory()
{
  // Keep the first test object's HOME and working directory for the process:
  static bool Redirected = false;
  if (Redirected == true) {
    return true;
  }

  // Use the private root for settings, output, and temporary files:
  const MString Root = GetTemporaryRootDirectory();
  if (Root.IsEmpty() == true) {
    merr<<"Error in MUnitTest::RedirectHomeDirectory: unable to create the private temporary root"<<endl;
    return false;
  }

  if (setenv("HOME", Root.Data(), 1) != 0) {
    merr<<"Error in MUnitTest::RedirectHomeDirectory: unable to set HOME"<<endl;
    return false;
  }

  // Check that tilde expansion follows HOME:
  MString Check = "~/.redirect";
  MFile::ExpandFileName(Check);
  if (Check.BeginsWith(Root) == false) {
    merr<<"Error in MUnitTest::RedirectHomeDirectory: ~ does not expand into the private temporary root: "<<Check<<endl;
    return false;
  }

  // Change the working directory through ROOT to update its cached path:
  if (gSystem->ChangeDirectory(Root) == false) {
    merr<<"Error in MUnitTest::RedirectHomeDirectory: unable to change into the private temporary root"<<endl;
    return false;
  }
  Redirected = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Begin tracking the check wrapped by ExpectedToFail
void MUnitTest::BeginExpectedFailureMode()
{
  // Ignore nested wrappers - the outer one already tracks the check
  if (m_IsExpectedFailureModeActive == true) {
    ++m_NumberOfNestedExpectedFailureModes;
    return;
  }

  m_IsExpectedFailureModeActive = true;
  m_NumberOfExpectedFailureModeChecks = 0;
  m_HasUnexpectedPass = false;
}


////////////////////////////////////////////////////////////////////////////////


//! End tracking and return true if exactly one wrapped check failed
bool MUnitTest::EndExpectedFailureMode()
{
  if (m_NumberOfNestedExpectedFailureModes > 0) {
    --m_NumberOfNestedExpectedFailureModes;
    return true;
  }

  // Check that tracking was started:
  if (m_IsExpectedFailureModeActive == false) {
    mout<<endl;
    mout<<"FAILED: ExpectedToFail()"<<endl;
    mout<<"   Description: EndExpectedFailureMode() without BeginExpectedFailureMode()"<<endl;
    mout<<endl;
    ++m_NumberOfFailedTests;
    return false;
  }

  m_IsExpectedFailureModeActive = false;

  if (m_NumberOfExpectedFailureModeChecks == 0) {
    mout<<endl;
    mout<<"FAILED: ExpectedToFail()"<<endl;
    mout<<"   Description: ExpectedToFail() must wrap at least one Evaluate*() call"<<endl;
    mout<<endl;
    ++m_NumberOfFailedTests;
    return false;
  }

  if (m_NumberOfExpectedFailureModeChecks > 1) {
    mout<<endl;
    mout<<"FAILED: ExpectedToFail()"<<endl;
    mout<<"   Description: ExpectedToFail() must wrap exactly one Evaluate*() call, not "<<m_NumberOfExpectedFailureModeChecks<<endl;
    mout<<endl;
    ++m_NumberOfFailedTests;
    return false;
  }

  return m_HasUnexpectedPass == false;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the last passed, failed, and known failure counts from a summary, return false if missing or invalid
//! The known failures are zero if the summary has no such line (it is printed only if there are any)
bool MUnitTest::ParseSummary(const MString& Output, unsigned int& Passed, unsigned int& Failed, unsigned int& KnownFailures)
{
  Passed = 0;
  Failed = 0;
  KnownFailures = 0;

  const string Text = Output.Data();
  const string PassedLabel = "Passed tests:";
  const string FailedLabel = "Failed tests:";
  const string KnownLabel = "Known failures:";
  const string::size_type PassedPosition = Text.rfind(PassedLabel);
  const string::size_type FailedPosition = Text.rfind(FailedLabel);
  if (PassedPosition == string::npos || FailedPosition == string::npos || FailedPosition < PassedPosition) {
    return false;
  }

  unsigned int Numbers[2] = { 0, 0 };
  if (ParseSummaryCount(Text, PassedPosition + PassedLabel.size(), Numbers[0]) == false) {
    return false;
  }
  if (ParseSummaryCount(Text, FailedPosition + FailedLabel.size(), Numbers[1]) == false) {
    return false;
  }

  // The known failures belong to the summary only after its failed tests
  unsigned int Known = 0;
  const string::size_type KnownPosition = Text.rfind(KnownLabel);
  if (KnownPosition != string::npos && KnownPosition > FailedPosition) {
    if (ParseSummaryCount(Text, KnownPosition + KnownLabel.size(), Known) == false) {
      return false;
    }
  }

  Passed = Numbers[0];
  Failed = Numbers[1];
  KnownFailures = Known;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the count of a summary line after its label: digits only, up to the end, a blank, or the comma of the one-line form
//! Return false if there is no count or it is larger than an unsigned int
bool MUnitTest::ParseSummaryCount(const string& Text, string::size_type Position, unsigned int& Count)
{
  Count = 0;

  // Skip the spaces and tabs after the label:
  while (Position < Text.size() && (Text[Position] == ' ' || Text[Position] == '\t')) {
    ++Position;
  }
  if (Position >= Text.size() || isdigit(static_cast<unsigned char>(Text[Position])) == 0) {
    return false;
  }
  unsigned long Value = 0;
  while (Position < Text.size() && isdigit(static_cast<unsigned char>(Text[Position])) != 0) {
    Value = 10*Value + static_cast<unsigned long>(Text[Position] - '0');
    if (Value > numeric_limits<unsigned int>::max()) {
      return false;
    }
    ++Position;
  }
  if (Position < Text.size() && Text[Position] != ',' && isspace(static_cast<unsigned char>(Text[Position])) == 0) {
    return false;
  }

  Count = static_cast<unsigned int>(Value);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Write complete text content to a test fixture file
bool MUnitTest::WriteTextFile(const MString& FileName, const MString& Content) const
{
  // Lock against temporary directory removal during file I/O
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
  if (Out.fail() == true) {
    merr<<"Error in MUnitTest::WriteTextFile: unable to write the complete content to the temporary file: "<<FileName<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate if text files have matching tokens on each line, ignoring whitespace between tokens
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

  // Read corresponding lines:
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    // A read error (e.g. the file is a directory) is not the end of the file:
    if (TestStream.bad() == true) {
      RegisterFailure(Function, Input, Description, MString("test file can be read: ") + TestFile.Data(), "read error in the test file");
      return false;
    }
    if (ReferenceStream.bad() == true) {
      RegisterFailure(Function, Input, Description, MString("reference file can be read: ") + ReferenceFile.Data(), "read error in the reference file");
      return false;
    }

    // Stop when both files end:
    if (GotTest == false && GotReference == false) {
      break;
    }

    ++LineNumber;

    // Report different line counts:
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

    // Report the first unequal line:
    if (LinesMatchNumerically(TestLine, ReferenceLine, MaximumLastDigitDifference) == false) {
      ostringstream Diff;
      Diff<<endl<<"      line "<<LineNumber<<":"
           <<endl<<"        expected:  "<<ReferenceLine
           <<endl<<"        test:      "<<TestLine;
      RegisterFailure(Function, Input, Description, "numerically equivalent files", Diff.str());
      return false;
    }
  }

  RegisterSuccess(Function, Input, Description);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if two lines have matching tokens, allowing differences in the last printed digits
bool MUnitTest::LinesMatchNumerically(const MString& TestLine, const MString& ReferenceLine,
                                      unsigned int MaximumLastDigitDifference) const
{
  // Split both lines at whitespace:
  istringstream TestStream(TestLine.ToString());
  istringstream ReferenceStream(ReferenceLine.ToString());

  MString TestToken;
  MString ReferenceToken;
  while (true) {
    const bool HasTestToken = static_cast<bool>(TestStream>>TestToken);
    const bool HasReferenceToken = static_cast<bool>(ReferenceStream>>ReferenceToken);

    // Check that both lines have the same number of tokens:
    if (HasTestToken == false || HasReferenceToken == false) {
      if (HasTestToken == HasReferenceToken) {
        return true;
      }
      return false;
    }

    // Accept identical tokens:
    if (TestToken == ReferenceToken) {
      continue;
    }

    // Compare differing tokens at their printed precision:
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
  // Lock against temporary directory removal during file I/O
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


//! Return a file path below the private temporary root without creating the file
MString MUnitTest::GetTemporaryFileName(const MString& Name) const
{
  // Lock against concurrent changes to the temporary root
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


//! Return a directory path below the private temporary root (empty name: root itself)
MString MUnitTest::GetTemporaryDirectoryName(const MString& Name) const
{
  // Lock against concurrent changes to the temporary root
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Accept an empty name for the root itself:
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


//! Remove and recreate a temporary directory (empty name: clear the root contents)
bool MUnitTest::PrepareTemporaryDirectory(const MString& Name) const
{
  // Lock against concurrent access during directory recreation
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Validate the directory path:
  const MString Directory = GetTemporaryDirectoryName(Name);
  if (IsSafeTemporaryPath(Directory, true) == false) {
    merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unsafe temporary directory path: "<<Directory<<endl;
    return false;
  }

  std::error_code Error;
  if (Name.IsEmpty() == true) {
    // Clear the root contents - keep its lock and working directory intact
    std::filesystem::directory_iterator Iterator(Directory.Data(), Error);
    if (Error.value() != 0) {
      merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to read the temporary root: "<<Directory<<endl;
      return false;
    }
    while (Iterator != std::filesystem::directory_iterator()) {
      std::filesystem::remove_all(Iterator->path(), Error);
      if (Error.value() != 0) {
        merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to clear the temporary root: "<<Directory<<endl;
        return false;
      }
      Iterator.increment(Error);
      if (Error.value() != 0) {
        merr<<"Error in MUnitTest::PrepareTemporaryDirectory: unable to read the temporary root: "<<Directory<<endl;
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


//! Remove a file inside the private temporary root
bool MUnitTest::RemoveTemporaryFile(const MString& FileName) const
{
  // Lock against concurrent access during path validation and removal
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Reject paths outside the private root:
  if (IsSafeTemporaryPath(FileName, false) == false) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: unsafe temporary file path: "<<FileName<<endl;
    return false;
  }

  // Do not follow symlinks: a dangling link is an entry which has to be removed too
  std::error_code Error;
  const std::filesystem::file_status Status = std::filesystem::symlink_status(FileName.Data(), Error);
  if (std::filesystem::exists(Status) == false) {
    // A missing entry is reported with an error code too, so distinguish it by the type
    if (Status.type() != std::filesystem::file_type::not_found) {
      merr<<"Error in MUnitTest::RemoveTemporaryFile: unable to check if temporary file exists: "<<FileName<<endl;
      return false;
    }
    return true;
  }

  // Reject directories:
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


//! Remove a directory and its contents inside the private temporary root (empty path: root itself)
bool MUnitTest::RemoveTemporaryDirectory(const MString& DirectoryName) const
{
  // Lock against concurrent access during path validation and removal
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Use the cached root if no path is given:
  MString Path = DirectoryName;
  if (DirectoryName.IsEmpty() == true) {
    Path = m_TemporaryRootDirectory;
  }
  if (Path.IsEmpty() == true) {
    return true;
  }

  // Validate the path, allowing the root itself for cleanup:
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

  // Clear the cached root when its stored path was removed:
  if (Path == m_TemporaryRootDirectory) {
    m_TemporaryRootDirectory = "";
    ReleaseTemporaryRootLock();
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create this object's private test directory below the configured log directory, or reuse it if it still exists
//! Use the test basename and a random suffix, and check that the directory stays inside the log directory
//! Hold a directory lock against test driver cleanup; leave HOME and the working directory unchanged
//! Return false if directory creation or path validation fails
bool MUnitTest::CreateTemporaryRootDirectory() const
{
  // Lock root creation - concurrent callers must share one directory
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Reuse the existing root:
  if (m_TemporaryRootDirectory.IsEmpty() == false) {
    if (MFile::IsDirectory(m_TemporaryRootDirectory) == true) {
      return true;
    }

    m_TemporaryRootDirectory = "";
    ReleaseTemporaryRootLock();
  }

  // Create a directory with a 10-character random suffix below the log directory:
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

  // Lock the root against cleanup by the test driver:
  m_TemporaryRootLock = open(m_TemporaryRootDirectory.Data(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
  if (m_TemporaryRootLock < 0 || flock(m_TemporaryRootLock, LOCK_EX | LOCK_NB) != 0) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: unable to lock the private temporary root: "<<m_TemporaryRootDirectory<<endl;
    ReleaseTemporaryRootLock();
    // The root is still empty: remove it, nothing protects it against the cleanup of the test driver
    std::filesystem::remove(m_TemporaryRootDirectory.Data(), Error);
    m_TemporaryRootDirectory = "";
    return false;
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


//! Return the randomized private temporary root
MString MUnitTest::GetTemporaryRootDirectory() const
{
  // Lock against concurrent changes to the temporary root
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Create the root if needed:
  if (CreateTemporaryRootDirectory() == false) {
    return "";
  }

  return m_TemporaryRootDirectory;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true for a plain file or directory name (empty name: allowed if AllowEmpty is true)
bool MUnitTest::IsValidTemporaryPathName(const MString& Name, bool AllowEmpty) const
{
  // Accept an empty name only when allowed:
  if (Name.IsEmpty() == true) {
    return AllowEmpty;
  }

  // Reject path components and embedded null characters:
  if (Name == "." || Name == ".." || Name.Contains("/") == true || Name.Contains("\\") == true ||
      Name.GetString().find('\0') != std::string::npos) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Build a directory basename from ASCII letters, digits, '_' and '-' (fallback: "UnitTest")
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


//! Read the log directory and the time out from the testing settings file
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
  m_SettingsTimeout = Settings.GetTimeout();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the path resolves inside the private temporary root (AllowRoot: accept the root itself)
bool MUnitTest::IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const
{
  // Reject empty paths:
  if (Path.IsEmpty() == true) {
    return false;
  }

  // Require an existing cached root - validation must not create one
  if (m_TemporaryRootDirectory.IsEmpty() == true) {
    return false;
  }

  std::error_code Error;
  const MString LogDirectory = GetLogDirectory();

  // Resolve path components and symlinks in the log directory:
  const std::filesystem::path LogPath = std::filesystem::weakly_canonical(LogDirectory.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Resolve the private root:
  const std::filesystem::path Root = std::filesystem::weakly_canonical(m_TemporaryRootDirectory.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Resolve path components and symlinks in the candidate path:
  const std::filesystem::path Candidate = std::filesystem::weakly_canonical(Path.Data(), Error);
  if (Error.value() != 0) {
    return false;
  }

  // Reject the log directory itself:
  if (Root == LogPath) {
    return false;
  }

  // Check that the root remains inside the log directory:
  if (IsPathContained(LogPath, Root) == false) {
    return false;
  }

  // Check that the path remains inside the private root:
  if (IsPathContained(Root, Candidate) == false) {
    return false;
  }

  // Accept the root itself only when allowed:
  if (AllowRoot == false && Candidate == Root) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if Child equals Parent or lies below it
bool MUnitTest::IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const
{
  // Compare path components - a shared string prefix does not imply containment
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


//! Evaluate if two text files have identical lines, ignoring LF/CRLF endings and a missing final newline
bool MUnitTest::EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& TestFile, const MString& ReferenceFile)
{
  // Open the files:
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

  // Read corresponding lines:
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    // A read error (e.g. the file is a directory) is not the end of the file:
    if (TestStream.bad() == true) {
      RegisterFailure(Function, Input, Description, MString("test file can be read: ") + TestFile.Data(), "read error in the test file");
      return false;
    }
    if (ReferenceStream.bad() == true) {
      RegisterFailure(Function, Input, Description, MString("reference file can be read: ") + ReferenceFile.Data(), "read error in the reference file");
      return false;
    }

    // Strip the carriage return from CRLF endings:
    if (GotTest == true && TestLine.EndsWith("\r")) {
      TestLine.RemoveLast(1);
    }
    if (GotReference == true && ReferenceLine.EndsWith("\r")) {
      ReferenceLine.RemoveLast(1);
    }

    // Stop when both files end:
    if (GotTest == false && GotReference == false) {
      break;
    }

    ++LineNumber;

    // Report different line counts:
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

    // Report the first unequal line, including whitespace differences:
    if (TestLine != ReferenceLine) {
      ostringstream Diff;
      Diff<<endl<<"      line "<<LineNumber<<":"
           <<endl<<"        expected:  "<<ReferenceLine
           <<endl<<"        test:      "<<TestLine;
      RegisterFailure(Function, Input, Description, "identical files", Diff.str());
      return false;
    }
  }

  RegisterSuccess(Function, Input, Description);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove source context lines from error messages
MString MUnitTest::RemoveErrorMessageContext(const MString& Text) const
{
  const MString Prefix = "!!!!! Error in file \"";

  // Copy all lines except the ones with the source context:
  MString Result;
  size_t Start = 0;
  while (Start < Text.Length()) {
    size_t End = Text.Index("\n", Start);
    size_t Size = (End == string::npos) ? Text.Length() - Start : End + 1 - Start;
    MString Line = Text.GetSubString(Start, Size);
    if (Line.BeginsWith(Prefix) == false) {
      Result.AppendInPlace(Line);
    }
    Start += Size;
  }

  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the larger of two values or NaN if one of them is NaN
double MUnitTest::GetMaximum(double First, double Second) const
{
  if (std::isnan(First) == true || std::isnan(Second) == true) {
    return numeric_limits<double>::quiet_NaN();
  }

  return (First > Second) ? First : Second;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate if two finite vectors have a distance of at most Tolerance
bool MUnitTest::EvaluateVectorNear(MString Function, MString Input, MString Description, const MVector& Output, const MVector& Truth, double Tolerance)
{
  const bool Finite = std::isfinite(Output.X()) && std::isfinite(Output.Y()) && std::isfinite(Output.Z()) &&
                      std::isfinite(Truth.X()) && std::isfinite(Truth.Y()) && std::isfinite(Truth.Z());
  const double Distance = (Output - Truth).Mag();

  if (Finite == false || std::isfinite(Tolerance) == false || Distance > Tolerance) {
    ostringstream ExpectedStream;
    ExpectedStream<<setprecision(numeric_limits<long double>::max_digits10)<<"("<<Truth.X()<<", "<<Truth.Y()<<", "<<Truth.Z()<<") +/- "<<Tolerance;
    ostringstream OutputStream;
    OutputStream<<setprecision(numeric_limits<long double>::max_digits10)<<"("<<Output.X()<<", "<<Output.Y()<<", "<<Output.Z()<<"), distance "<<Distance;
    RegisterFailure(Function, Input, Description, ExpectedStream.str(), OutputStream.str());
    return false;
  }

  RegisterSuccess(Function, Input, Description);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Evaluate if each pair of finite rotation matrix elements differs by at most Tolerance
bool MUnitTest::EvaluateRotationNear(MString Function, MString Input, MString Description, const MRotation& Output, const MRotation& Truth, double Tolerance)
{
  // Elements row by row: XX, YX, ZX, XY, YY, ZY, XZ, YZ, ZZ
  const vector<double> O = { Output.GetXX(), Output.GetYX(), Output.GetZX(), Output.GetXY(), Output.GetYY(), Output.GetZY(), Output.GetXZ(), Output.GetYZ(), Output.GetZZ() };
  const vector<double> T = { Truth.GetXX(), Truth.GetYX(), Truth.GetZX(), Truth.GetXY(), Truth.GetYY(), Truth.GetZY(), Truth.GetXZ(), Truth.GetYZ(), Truth.GetZZ() };

  // A NaN or infinite tolerance would accept every difference:
  bool Match = std::isfinite(Tolerance);
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

  RegisterSuccess(Function, Input, Description);
  return true;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
