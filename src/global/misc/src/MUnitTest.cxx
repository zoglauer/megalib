/*
 * MUnitTest.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MUnitTest.h"

// Standard libs:
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>


// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MUnitTest)
#endif


////////////////////////////////////////////////////////////////////////////////


const MString MUnitTest::c_FallbackTemporaryBaseName = "AUnitTest";


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MUnitTest::MUnitTest(const MString& Name)
{
  m_Name = Name;
  m_TemporaryBaseName = Name;
  if (IsValidTemporaryBaseName(m_TemporaryBaseName) == false) {
    merr<<"Warning in MUnitTest::MUnitTest: unit-test name cannot be used as a temporary directory name: "
        <<m_Name<<". Using "<<c_FallbackTemporaryBaseName<<" instead."<<endl;
    m_TemporaryBaseName = c_FallbackTemporaryBaseName;
  }
  m_NumberOfPassedTests = 0;
  m_NumberOfFailedTests = 0;
  
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MUnitTest::~MUnitTest()
{
  if (m_TemporaryRootDirectory.IsEmpty() == true) return;

  if (RemoveTemporaryDirectory() == false) {
    merr<<"Error in MUnitTest::~MUnitTest: unable to remove the private temporary root: "<<m_TemporaryRootDirectory<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MUnitTest::Summarize()
{
  mout<<"Unit test: "<<m_Name<<endl;
  mout<<"Passed tests: "<<m_NumberOfPassedTests<<endl;
  mout<<"Failed tests: "<<m_NumberOfFailedTests<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::WriteTextFile(const MString& FileName, const MString& Content) const
{
  // Keep validation and writing atomic with respect to concurrent teardown, i.e.
  // make sure the temporary directory is not removed while we execute this function.
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


bool MUnitTest::EvaluateFilesNumericallyEquivalent(MString Function, MString Input, MString Description,
                                                   const MString& TestFile, const MString& ReferenceFile,
                                                   unsigned int MaximumLastDigitDifference)
{
  //! Open both files independently so failures identify which input is unavailable
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

  //! Read both files in lockstep so line count and line content are checked together
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    //! Reaching the end of both files at the same time completes the comparison
    if (GotTest == false && GotReference == false) break;

    ++LineNumber;

    //! If only one read succeeded, the files contain a different number of lines
    if (GotTest != GotReference) {
      ostringstream LengthOutput;
      if (GotTest == false) {
        LengthOutput << "test file is shorter (ends at line " << LineNumber << ")";
      } else {
        LengthOutput << "test file is longer (reference ends at line " << LineNumber << ")";
      }
      RegisterFailure(Function, Input, Description + MString(" (line count)"),
                      "same number of lines", LengthOutput.str());
      return false;
    }

    //! Stop at the first unequal line and report both complete lines for diagnosis
    if (LinesMatchNumerically(TestLine, ReferenceLine, MaximumLastDigitDifference) == false) {
      ostringstream Diff;
      Diff << "\n      line " << LineNumber << ":"
           << "\n        expected:  " << ReferenceLine
           << "\n        test:      " << TestLine;
      RegisterFailure(Function, Input, Description, "numerically equivalent files", Diff.str());
      return false;
    }
  }

  RegisterSuccess();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::LinesMatchNumerically(const MString& TestLine, const MString& ReferenceLine,
                                      unsigned int MaximumLastDigitDifference) const
{
  //! Stream extraction splits both lines at whitespace and ignores whitespace differences
  istringstream TestStream(TestLine.ToString());
  istringstream ReferenceStream(ReferenceLine.ToString());

  MString TestToken;
  MString ReferenceToken;
  while (true) {
    const bool HasTestToken = static_cast<bool>(TestStream >> TestToken);
    const bool HasReferenceToken = static_cast<bool>(ReferenceStream >> ReferenceToken);

    //! If either stream is exhausted, both must be exhausted to have equal token counts
    if (HasTestToken == false || HasReferenceToken == false) return HasTestToken == HasReferenceToken;

    //! Identical tokens match directly, including non-numeric and integer-only tokens
    if (TestToken == ReferenceToken) continue;

    //! Differing tokens must be floating-point numbers matching within their printed precision
    if (TestToken.AreNumbersNumericallyMatching(ReferenceToken, MaximumLastDigitDifference) == false) return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MUnitTest::ReadTextFile(const MString& FileName) const
{
  // Keep validation and reading atomic with respect to concurrent teardown, i.e.
  // make sure the temporary directory is not removed while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  if (IsSafeTemporaryPath(FileName, false) == false) {
    merr<<"Error in MUnitTest::ReadTextFile: unsafe temporary file path: "<<FileName<<endl;
    return "";
  }

  ifstream In(FileName.Data());
  if (In.is_open() == false) {
    merr<<"Error in MUnitTest::ReadTextFile: unable to open temporary file: "<<FileName<<endl;
    return "";
  }

  ostringstream Buffer;
  Buffer<<In.rdbuf();

  return Buffer.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString MUnitTest::GetTemporaryFileName(const MString& Name) const
{
  // Keep path generation atomic with respect to concurrent teardown, i.e.
  // make sure the randomized temporary root remains stable while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Files must use plain names and always live directly below the private root.
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


MString MUnitTest::GetTemporaryDirectoryName(const MString& Name) const
{
  // Keep path generation atomic with respect to concurrent teardown, i.e.
  // make sure the randomized temporary root remains stable while we execute this function.
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
  if (Name.IsEmpty() == true) return Root;

  return Root + "/" + Name;
}


////////////////////////////////////////////////////////////////////////////////


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
  if (std::filesystem::is_directory(FileName.Data(), Error) == true) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: file is actually a directory: "<<FileName<<endl;
    return false;
  }
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::RemoveTemporaryFile: unable to inspect temporary file: "<<FileName<<endl;
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


bool MUnitTest::RemoveTemporaryDirectory(const MString& DirectoryName) const
{
  // Keep validation and removal atomic with respect to concurrent access, i.e.
  // make sure no other temporary-path operation runs while we execute this function.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // If no directory name is given, uses m_TemporaryRootDirectory.
  const MString Path = DirectoryName.IsEmpty() == true ? m_TemporaryRootDirectory : DirectoryName;
  if (Path.IsEmpty() == true) return true;

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
  if (Path == m_TemporaryRootDirectory) m_TemporaryRootDirectory = "";
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::CreateTemporaryRootDirectory() const
{
  // Serialize lazy root creation, i.e. make sure concurrent callers share one
  // randomized temporary root instead of creating competing directories.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // Reuse the existing randomized root if it still exists.
  if (m_TemporaryRootDirectory.IsEmpty() == false) {
    std::error_code Error;
    if (std::filesystem::is_directory(m_TemporaryRootDirectory.Data(), Error) == true && Error.value() == 0) {
      return true;
    }

    m_TemporaryRootDirectory = "";
  }

  // The test name becomes part of the randomized directory name and must
  // therefore be a plain name without path components.
  if (IsValidTemporaryBaseName(m_TemporaryBaseName) == false) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: invalid temporary directory basename: "<<m_TemporaryBaseName<<endl;
    return false;
  }

  // MFile creates the directory atomically below the system temporary
  // directory and adds the default 10-character random component.
  m_TemporaryRootDirectory = MFile::CreateTemporaryDirectory(m_TemporaryBaseName);
  if (m_TemporaryRootDirectory.IsEmpty() == true) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: unable to create the private temporary root for "<<m_Name<<endl;
    return false;
  }

  std::error_code Error;
  const MString SystemTemporaryDirectory = gSystem->TempDirectory();
  const std::filesystem::path TemporaryDirectory =
    std::filesystem::weakly_canonical(SystemTemporaryDirectory.Data(), Error);
  if (Error.value() != 0) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: unable to resolve the system temporary directory"<<endl;
    m_TemporaryRootDirectory = "";
    return false;
  }

  const std::filesystem::path Root =
    std::filesystem::weakly_canonical(m_TemporaryRootDirectory.Data(), Error);
  if (Error.value() != 0 || IsPathContained(TemporaryDirectory, Root) == false || Root == TemporaryDirectory) {
    merr<<"Error in MUnitTest::CreateTemporaryRootDirectory: generated root is not safely contained in the system temporary directory: "<<m_TemporaryRootDirectory<<endl;
    m_TemporaryRootDirectory = "";
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MUnitTest::GetTemporaryRootDirectory() const
{
  // Serialize lazy root access, i.e. make sure concurrent callers observe one
  // stable randomized temporary root.
  lock_guard<recursive_mutex> Lock(m_TemporaryPathMutex);

  // All generated temporary paths are rooted below this randomized directory.
  if (CreateTemporaryRootDirectory() == false) return "";

  return m_TemporaryRootDirectory;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::IsValidTemporaryPathName(const MString& Name, bool AllowEmpty) const
{
  // Empty names are permitted only when the caller explicitly requests
  // the test's private temporary root.
  if (Name.IsEmpty() == true) return AllowEmpty;

  // Temporary names must not contain path components. This rejects
  // traversal attempts such as "../home/andreas" before a path is built.
  if (Name == "." || Name == ".." || Name.Contains("/") == true || Name.Contains("\\") == true ||
      Name.GetString().find('\0') != std::string::npos) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::IsValidTemporaryBaseName(const MString& Name) const
{
  if (Name.IsEmpty() == true) return false;

  for (const char Character: Name.GetString()) {
    if ((Character >= 'a' && Character <= 'z') ||
        (Character >= 'A' && Character <= 'Z') ||
        (Character >= '0' && Character <= '9') ||
        Character == '_' || Character == '-') {
      continue;
    }
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::IsSafeTemporaryPath(const MString& Path, bool AllowRoot) const
{
  // An empty path must never reach a guarded filesystem operation.
  if (Path.IsEmpty() == true) return false;

  // Validation must never create a new temporary root as a side effect.
  // Only paths generated earlier by this test are eligible for access.
  if (m_TemporaryRootDirectory.IsEmpty() == true) return false;

  std::error_code Error;
  const MString SystemTemporaryDirectory = gSystem->TempDirectory();

  // Resolve the system temporary directory. This normalizes "." and ".."
  // and resolves symlinks before any containment decision is made.
  const std::filesystem::path TemporaryDirectory = std::filesystem::weakly_canonical(SystemTemporaryDirectory.Data(), Error);
  if (Error.value() != 0) return false;

  // Resolve this test's randomized private root.
  const std::filesystem::path Root = std::filesystem::weakly_canonical(m_TemporaryRootDirectory.Data(), Error);
  if (Error.value() != 0) return false;

  // Resolve currently visible traversal and symlink escapes. The later
  // remove() and remove_all() calls remove symlinks themselves instead of
  // following them during deletion, which keeps the removal path safe even
  // if a symlink changes after this check.
  const std::filesystem::path Candidate = std::filesystem::weakly_canonical(Path.Data(), Error);
  if (Error.value() != 0) return false;

  // Never permit an operation on the system temporary directory itself.
  if (Root == TemporaryDirectory) return false;

  // The randomized private root must itself remain inside the system
  // temporary directory.
  if (IsPathContained(TemporaryDirectory, Root) == false) return false;

  // The requested path must resolve to the private root or one of its
  // descendants. Paths such as "/tmp/root/../../home/andreas" fail here.
  if (IsPathContained(Root, Candidate) == false) return false;

  // File removal must not remove the root. Directory removal can permit
  // this explicitly for final test cleanup.
  if (AllowRoot == false && Candidate == Root) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::IsPathContained(const std::filesystem::path& Parent, const std::filesystem::path& Child) const
{
  // A lexical relative path avoids string-prefix mistakes such as treating
  // "/tmp/test-other" as a child of "/tmp/test" and tolerates trailing
  // separators after canonicalization.
  const std::filesystem::path Relative = Child.lexically_normal().lexically_relative(Parent.lexically_normal());
  if (Relative.empty() == true) return false;
  if (Relative == ".") return true;

  const std::filesystem::path::const_iterator First = Relative.begin();
  return First != Relative.end() && *First != "..";
}


////////////////////////////////////////////////////////////////////////////////


bool MUnitTest::EvaluateFilesIdentical(MString Function, MString Input, MString Description, const MString& TestFile, const MString& ReferenceFile)
{
  //! Open both files independently so failures identify which input is unavailable
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


  //! Read both files in lockstep so line count and line content are checked together
  unsigned int LineNumber = 0;
  MString TestLine;
  MString ReferenceLine;

  while (true) {
    const bool GotTest = static_cast<bool>(TestLine.ReadLine(TestStream));
    const bool GotReference = static_cast<bool>(ReferenceLine.ReadLine(ReferenceStream));

    //! Remove the carriage-return component of CRLF line endings
    if (GotTest == true && TestLine.EndsWith("\r")) TestLine.RemoveLast(1);
    if (GotReference == true && ReferenceLine.EndsWith("\r")) ReferenceLine.RemoveLast(1);

    //! Reaching the end of both files at the same time completes the comparison
    if (GotTest == false && GotReference == false) break;

    ++LineNumber;

    //! If only one read succeeded, the files contain a different number of lines
    if (GotTest != GotReference) {
      ostringstream LengthOutput;
      if (GotTest == false) {
        LengthOutput << "test file is shorter (ends at line " << LineNumber << ")";
      } else {
        LengthOutput << "test file is longer (reference ends at line " << LineNumber << ")";
      }
      RegisterFailure(Function, Input, Description + MString(" (line count)"),
                      "same number of lines", LengthOutput.str());
      return false;
    }

    //! Exact comparison preserves all characters within the line, including whitespace
    //! Stop at the first unequal line and report both complete lines for diagnosis
    if (TestLine != ReferenceLine) {
      ostringstream Diff;
      Diff << "\n      line " << LineNumber << ":"
           << "\n        expected:  " << ReferenceLine
           << "\n        test:      " << TestLine;
      RegisterFailure(Function, Input, Description, "identical files", Diff.str());
      return false;
    }
  }

  RegisterSuccess();
  return true;
}


// MUnitTest.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
