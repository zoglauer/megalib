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
#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;


////////////////////////////////////////////////////////////////////////////////


static bool BeginsWith(const string& Text, const string& Prefix)
{
  return Text.size() >= Prefix.size() && Text.compare(0, Prefix.size(), Prefix) == 0;
}


////////////////////////////////////////////////////////////////////////////////


static bool EndsWith(const string& Text, const string& Suffix)
{
  return Text.size() >= Suffix.size() &&
         Text.compare(Text.size() - Suffix.size(), Suffix.size(), Suffix) == 0;
}


////////////////////////////////////////////////////////////////////////////////


static string StripDirectory(const string& Path)
{
  string::size_type Slash = Path.find_last_of("/\\");
  if (Slash == string::npos) {
    return Path;
  }

  return Path.substr(Slash + 1);
}


////////////////////////////////////////////////////////////////////////////////


static string StripExtension(const string& Name)
{
  if (EndsWith(Name, ".cxx") == true) {
    return Name.substr(0, Name.size() - 4);
  }

  return Name;
}


////////////////////////////////////////////////////////////////////////////////


static string GetBinDirectory(const char* Argv0)
{
  string Executable = Argv0 != nullptr ? Argv0 : "";
  if (Executable.find('/') != string::npos || Executable.find('\\') != string::npos) {
    string::size_type Slash = Executable.find_last_of("/\\");
    if (Slash != string::npos) {
      return Executable.substr(0, Slash);
    }
  }

  char Cwd[4096];
  if (getcwd(Cwd, sizeof(Cwd)) != nullptr) {
    return string(Cwd) + "/bin";
  }

  return "bin";
}


////////////////////////////////////////////////////////////////////////////////


static bool IsExecutableFile(const string& Path)
{
  struct stat Info;
  if (stat(Path.c_str(), &Info) != 0) {
    return false;
  }

  if (S_ISREG(Info.st_mode) == 0) {
    return false;
  }

  return access(Path.c_str(), X_OK) == 0;
}


////////////////////////////////////////////////////////////////////////////////


static string NormalizeRequest(const string& Input)
{
  return StripExtension(StripDirectory(Input));
}


////////////////////////////////////////////////////////////////////////////////


static string ResolveRequest(const string& Input, const map<string, string>& AllTests)
{
  string Name = NormalizeRequest(Input);
  vector<string> Candidates;

  if (BeginsWith(Name, "UT") == true) {
    Candidates.push_back(Name);
  } else {
    Candidates.push_back("UT" + Name);
    if (BeginsWith(Name, "M") == true && Name.size() > 1) {
      Candidates.push_back("UT" + Name.substr(1));
    }
  }

  for (vector<string>::const_iterator It = Candidates.begin(); It != Candidates.end(); ++It) {
    if (AllTests.find(*It) != AllTests.end()) {
      return *It;
    }
  }

  return Candidates.empty() == false ? Candidates.front() : Name;
}


////////////////////////////////////////////////////////////////////////////////


static map<string, string> DiscoverTests(const string& BinDirectory)
{
  map<string, string> Tests;

  DIR* Directory = opendir(BinDirectory.c_str());
  if (Directory == nullptr) {
    return Tests;
  }

  dirent* Entry = nullptr;
  while ((Entry = readdir(Directory)) != nullptr) {
    string Name = Entry->d_name;
    if (BeginsWith(Name, "UT") == false) {
      continue;
    }
    if (Name == "UTExecute") {
      continue;
    }

    string Path = BinDirectory + "/" + Name;
    if (IsExecutableFile(Path) == false) {
      continue;
    }

    Tests[Name] = Path;
  }

  closedir(Directory);
  return Tests;
}


////////////////////////////////////////////////////////////////////////////////


static int RunTest(const string& TestName, const string& TestPath)
{
  cout<<endl;
  cout<<"RUN "<<TestName<<endl;

  int Status = system(TestPath.c_str());
  if (Status == -1) {
    cout<<"FAIL "<<TestName<<endl;
    return 1;
  }

  if (WIFEXITED(Status) != 0 && WEXITSTATUS(Status) == 0) {
    cout<<"PASS "<<TestName<<endl;
    return 0;
  }

  cout<<"FAIL "<<TestName<<endl;
  return 1;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  string BinDirectory = GetBinDirectory(argc > 0 ? argv[0] : nullptr);
  map<string, string> AllTests = DiscoverTests(BinDirectory);

  if (AllTests.empty() == true) {
    cerr<<"No unit test executables found in "<<BinDirectory<<endl;
    return 1;
  }

  vector<string> RequestedTests;
  if (argc == 1) {
    for (map<string, string>::const_iterator It = AllTests.begin(); It != AllTests.end(); ++It) {
      RequestedTests.push_back(It->first);
    }
  } else {
    set<string> Seen;
    for (int a = 1; a < argc; ++a) {
      string Requested = ResolveRequest(argv[a], AllTests);
      if (Seen.insert(Requested).second == false) {
        continue;
      }

      if (AllTests.find(Requested) == AllTests.end()) {
        cerr<<"Unknown unit test request: "<<argv[a]<<" -> "<<Requested<<endl;
        return 1;
      }

      RequestedTests.push_back(Requested);
    }
  }

  int Failed = 0;
  int Total = 0;
  for (vector<string>::const_iterator It = RequestedTests.begin(); It != RequestedTests.end(); ++It) {
    ++Total;
    Failed += RunTest(*It, AllTests[*It]);
  }

  cout<<endl;
  cout<<"Summary:"<<endl;
  cout<<"  Executed: "<<Total<<endl;
  cout<<"  Failed: "<<Failed<<endl;

  return Failed == 0 ? 0 : 1;
}
