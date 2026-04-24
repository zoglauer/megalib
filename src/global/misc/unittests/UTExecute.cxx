/*
 * UTExecute.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <dirent.h>
#include <map>
#include <set>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;

// MEGAlib:
#include "MStreams.h"
#include "MString.h"


//! Execute all unit tests or the requested subset
class UTExecute
{
public:
  //! Default constructor
  UTExecute() {}
  //! Default destructor
  virtual ~UTExecute() {}

  //! Execute the requested tests
  int Execute(int argc, char** argv);

private:
  //! Return true if the text begins with the prefix
  bool BeginsWith(const MString& Text, const MString& Prefix) const;
  //! Return true if the text ends with the suffix
  bool EndsWith(const MString& Text, const MString& Suffix) const;
  //! Remove the directory from the path
  MString StripDirectory(const MString& Path) const;
  //! Remove a known source-file extension
  MString StripExtension(const MString& Name) const;
  //! Determine the bin directory from argv[0]
  MString GetBinDirectory(const char* Argv0) const;
  //! Return true if the path points to an executable file
  bool IsExecutableFile(const MString& Path) const;
  //! Normalize one command-line request
  MString NormalizeRequest(const MString& Input) const;
  //! Resolve one command-line request to a discovered test name
  MString ResolveRequest(const MString& Input) const;
  //! Discover all available UT* executables in the bin directory
  bool DiscoverTests();
  //! Build the list of tests which should be run
  bool BuildRequestedTests(int argc, char** argv, vector<MString>& RequestedTests) const;
  //! Run one test executable
  int RunTest(const MString& TestName) const;

  //! The bin directory containing the test executables
  MString m_BinDirectory;
  //! All discovered unit tests and their paths
  map<MString, MString> m_AllTests;
};


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::BeginsWith(const MString& Text, const MString& Prefix) const
{
  return Text.BeginsWith(Prefix);
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::EndsWith(const MString& Text, const MString& Suffix) const
{
  return Text.EndsWith(Suffix);
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::StripDirectory(const MString& Path) const
{
  size_t Slash = Path.Last('/');
  size_t Backslash = Path.Last('\\');
  size_t NPos = string::npos;

  if (Slash == NPos && Backslash == NPos) {
    return Path;
  }

  if (Slash == NPos || (Backslash != NPos && Backslash > Slash)) {
    Slash = Backslash;
  }

  return Path.GetSubString(Slash + 1);
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::StripExtension(const MString& Name) const
{
  if (EndsWith(Name, ".cxx") == true) {
    return Name.GetSubString(0, Name.Length() - 4);
  }

  return Name;
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::GetBinDirectory(const char* Argv0) const
{
  MString Executable = Argv0 != nullptr ? Argv0 : "";
  if (Executable.Contains("/") == true || Executable.Contains("\\") == true) {
    size_t Slash = Executable.Last('/');
    size_t Backslash = Executable.Last('\\');
    size_t NPos = string::npos;
    if (Slash == NPos || (Backslash != NPos && Backslash > Slash)) {
      Slash = Backslash;
    }
    if (Slash != NPos) {
      return Executable.GetSubString(0, Slash);
    }
  }

  char Cwd[4096];
  if (getcwd(Cwd, sizeof(Cwd)) != nullptr) {
    return MString(Cwd) + "/bin";
  }

  return "bin";
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::IsExecutableFile(const MString& Path) const
{
  struct stat Info;
  if (stat(Path.Data(), &Info) != 0) {
    return false;
  }

  if (S_ISREG(Info.st_mode) == 0) {
    return false;
  }

  return access(Path.Data(), X_OK) == 0;
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::NormalizeRequest(const MString& Input) const
{
  return StripExtension(StripDirectory(Input));
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ResolveRequest(const MString& Input) const
{
  MString Name = NormalizeRequest(Input);
  vector<MString> Candidates;

  if (BeginsWith(Name, "UT") == true) {
    Candidates.push_back(Name);
  } else {
    Candidates.push_back("UT" + Name);
    if (BeginsWith(Name, "M") == true && Name.Length() > 1) {
      Candidates.push_back("UT" + Name.GetSubString(1));
    }
  }

  for (vector<MString>::const_iterator It = Candidates.begin(); It != Candidates.end(); ++It) {
    if (m_AllTests.find(*It) != m_AllTests.end()) {
      return *It;
    }
  }

  if (Candidates.empty() == false) {
    return Candidates.front();
  }

  return Name;
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::DiscoverTests()
{
  m_AllTests.clear();

  DIR* Directory = opendir(m_BinDirectory.Data());
  if (Directory == nullptr) {
    return false;
  }

  dirent* Entry = nullptr;
  while ((Entry = readdir(Directory)) != nullptr) {
    MString Name = Entry->d_name;
    if (BeginsWith(Name, "UT") == false) {
      continue;
    }
    if (Name == "UTExecute") {
      continue;
    }

    MString Path = m_BinDirectory + "/" + Name;
    if (IsExecutableFile(Path) == false) {
      continue;
    }

    m_AllTests[Name] = Path;
  }

  closedir(Directory);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::BuildRequestedTests(int argc, char** argv, vector<MString>& RequestedTests) const
{
  RequestedTests.clear();

  if (argc == 1) {
    for (map<MString, MString>::const_iterator It = m_AllTests.begin(); It != m_AllTests.end(); ++It) {
      RequestedTests.push_back(It->first);
    }
    return true;
  }

  set<MString> Seen;
  for (int a = 1; a < argc; ++a) {
    MString Requested = ResolveRequest(argv[a]);
    if (Seen.insert(Requested).second == false) {
      continue;
    }

    if (m_AllTests.find(Requested) == m_AllTests.end()) {
      merr<<"Unknown unit test request: "<<argv[a]<<" -> "<<Requested<<show;
      return false;
    }

    RequestedTests.push_back(Requested);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int UTExecute::RunTest(const MString& TestName) const
{
  mout<<endl;
  mout<<"RUN "<<TestName<<show;

  map<MString, MString>::const_iterator Test = m_AllTests.find(TestName);
  if (Test == m_AllTests.end()) {
    mout<<"FAIL "<<TestName<<show;
    return 1;
  }

  int Status = system(Test->second.Data());
  if (Status == -1) {
    mout<<"FAIL "<<TestName<<show;
    return 1;
  }

  if (WIFEXITED(Status) != 0 && WEXITSTATUS(Status) == 0) {
    mout<<"PASS "<<TestName<<show;
    return 0;
  }

  mout<<"FAIL "<<TestName<<show;
  return 1;
}


////////////////////////////////////////////////////////////////////////////////


int UTExecute::Execute(int argc, char** argv)
{
  m_BinDirectory = GetBinDirectory(argc > 0 ? argv[0] : nullptr);
  if (DiscoverTests() == false || m_AllTests.empty() == true) {
    merr<<"No unit test executables found in "<<m_BinDirectory<<show;
    return 1;
  }

  vector<MString> RequestedTests;
  if (BuildRequestedTests(argc, argv, RequestedTests) == false) {
    return 1;
  }

  int Failed = 0;
  int Total = 0;
  for (vector<MString>::const_iterator It = RequestedTests.begin(); It != RequestedTests.end(); ++It) {
    ++Total;
    Failed += RunTest(*It);
  }

  mout<<endl;
  mout<<"Summary:"<<endl;
  mout<<"  Executed: "<<Total<<endl;
  mout<<"  Failed: "<<Failed<<show;

  return Failed == 0 ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  UTExecute Execute;
  return Execute.Execute(argc, argv);
}
