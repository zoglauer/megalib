/*
 * UTExecute.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Standard libs:
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


//! Discover and execute unit-test programs in parallel
class UTExecute
{
  // public interface:
 public:
  //! Create a runner for executables matching Prefix, excluding RunnerName
  UTExecute(const MString& Prefix, const MString& RunnerName, const MString& DashboardTitle);
  //! Default destructor
  virtual ~UTExecute();

  //! Execute all discovered unit tests or the requested subset
  int Execute(int argc, char** argv);


  // private methods:
 private:
  enum EStatus {
    c_StatusPending = 0,
    c_StatusRunning,
    c_StatusFailed,
    c_StatusPassed
  };

  MString StripExtension(const MString& Name) const;
  MString GetBinDirectory(const char* Argv0) const;
  MString GetTimingFile() const;
  MString NormalizeRequest(const MString& Input) const;
  MString ResolveRequest(const MString& Input) const;
  bool DiscoverTests();
  void LoadTimings(map<MString, double>& Timings) const;
  void SaveTimings(const map<MString, double>& Timings) const;
  bool BuildRequestedTests(int argc, char** argv, vector<MString>& RequestedTests) const;
  void SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const;
  bool LaunchTest(const MString& TestName, int& ChildPid, MString& OutputFile) const;
  MString ReadOutput(const MString& OutputFile) const;
  MString ExtractMetric(const MString& Output) const;
  MString FormatFailureMetric(const MString& Metric) const;
  MString FormatRuntime(double Seconds) const;
  MString WriteFailureReport(const vector<MString>& RequestedTests, const vector<EStatus>& Statuses,
                             const vector<MString>& Metrics, const vector<MString>& Outputs) const;


  // private members:
 private:
  MString m_Prefix;
  MString m_RunnerName;
  MString m_DashboardTitle;
  MString m_BinDirectory;
  MString m_TimingFile;
  map<MString, MString> m_AllTests;
};


////////////////////////////////////////////////////////////////////////////////


UTExecute::UTExecute(const MString& Prefix, const MString& RunnerName, const MString& DashboardTitle) :
  m_Prefix(Prefix),
  m_RunnerName(RunnerName),
  m_DashboardTitle(DashboardTitle)
{
}


////////////////////////////////////////////////////////////////////////////////


UTExecute::~UTExecute()
{
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::StripExtension(const MString& Name) const
{
  if (Name.EndsWith(".cxx") == true) return Name.GetSubString(0, Name.Length() - 4);
  return Name;
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::GetBinDirectory(const char* Argv0) const
{
  MString Executable = Argv0 != nullptr ? Argv0 : "";
  if (Executable.Contains("/") == true || Executable.Contains("\\") == true) {
    MString Directory = MFile::GetDirectoryName(Executable);
    if (Directory.IsEmpty() == false) return Directory;
  }

  const char* MEGAlib = getenv("MEGALIB");
  if (MEGAlib != nullptr && MEGAlib[0] != '\0') return MString(MEGAlib) + "/bin";

  return MFile::GetWorkingDirectory() + "/bin";
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::GetTimingFile() const
{
  const char* Home = getenv("HOME");
  MString BaseDirectory;
#ifdef __APPLE__
  if (Home != nullptr && Home[0] != '\0') BaseDirectory = MString(Home) + "/Library/Application Support";
#else
  const char* XdgConfigHome = getenv("XDG_CONFIG_HOME");
  if (XdgConfigHome != nullptr && XdgConfigHome[0] != '\0') {
    BaseDirectory = XdgConfigHome;
  } else if (Home != nullptr && Home[0] != '\0') {
    BaseDirectory = MString(Home) + "/.config";
  }
#endif
  if (BaseDirectory.IsEmpty() == true) return "";

  MString ConfigDirectory = BaseDirectory + "/MEGAlib";
  if (MFile::CreateDirectory(ConfigDirectory) == false) return "";
  return ConfigDirectory + "/" + m_RunnerName + ".timings";
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::NormalizeRequest(const MString& Input) const
{
  return StripExtension(MFile::GetBaseName(Input));
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ResolveRequest(const MString& Input) const
{
  MString Name = NormalizeRequest(Input);
  vector<MString> Candidates;
  if (Name.BeginsWith(m_Prefix) == true) {
    Candidates.push_back(Name);
  } else {
    Candidates.push_back(m_Prefix + Name);
    if (Name.BeginsWith("M") == true && Name.Length() > 1) {
      Candidates.push_back(m_Prefix + Name.GetSubString(1));
    }
  }

  for (const MString& Candidate : Candidates) {
    if (m_AllTests.find(Candidate) != m_AllTests.end()) return Candidate;
  }
  return Candidates.empty() == false ? Candidates.front() : Name;
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::DiscoverTests()
{
  m_AllTests.clear();
  DIR* Directory = opendir(m_BinDirectory.Data());
  if (Directory == nullptr) return false;

  dirent* Entry = nullptr;
  while ((Entry = readdir(Directory)) != nullptr) {
    MString Name = Entry->d_name;
    if (Name.BeginsWith(m_Prefix) == false || Name == m_RunnerName) continue;
    MString Path = m_BinDirectory + "/" + Name;
    if (MFile::IsExecutable(Path) == true) m_AllTests[Name] = Path;
  }
  closedir(Directory);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTExecute::LoadTimings(map<MString, double>& Timings) const
{
  Timings.clear();
  if (m_TimingFile.IsEmpty() == true) return;

  ifstream Input(m_TimingFile.Data());
  string Line;
  while (getline(Input, Line)) {
    if (Line.empty() == true || Line[0] == '#') continue;
    string Name;
    double Seconds = 0.0;
    stringstream Stream(Line);
    if ((Stream >> Name >> Seconds).fail() == false && Seconds >= 0.0) Timings[Name.c_str()] = Seconds;
  }
}


////////////////////////////////////////////////////////////////////////////////


void UTExecute::SaveTimings(const map<MString, double>& Timings) const
{
  if (m_TimingFile.IsEmpty() == true) return;
  ofstream Output(m_TimingFile.Data(), ios::trunc);
  if (Output.is_open() == false) return;

  Output<<"# "<<m_RunnerName<<" timing cache\n";
  Output<<"# test_name seconds\n";
  for (const auto& Entry : Timings) Output<<Entry.first<<" "<<Entry.second<<"\n";
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::BuildRequestedTests(int argc, char** argv, vector<MString>& RequestedTests) const
{
  RequestedTests.clear();
  if (argc == 1) {
    for (const auto& Entry : m_AllTests) RequestedTests.push_back(Entry.first);
    return true;
  }

  set<MString> Seen;
  for (int a = 1; a < argc; ++a) {
    MString Requested = ResolveRequest(argv[a]);
    if (Seen.insert(Requested).second == false) continue;
    if (m_AllTests.find(Requested) == m_AllTests.end()) {
      merr<<"Unknown unit test request: "<<argv[a]<<" -> "<<Requested<<show;
      return false;
    }
    RequestedTests.push_back(Requested);
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void UTExecute::SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const
{
  stable_sort(RequestedTests.begin(), RequestedTests.end(), [&](const MString& A, const MString& B) {
    const auto Ta = Timings.find(A);
    const auto Tb = Timings.find(B);
    const double TimeA = Ta != Timings.end() ? Ta->second : -1.0;
    const double TimeB = Tb != Timings.end() ? Tb->second : -1.0;
    return TimeA != TimeB ? TimeA > TimeB : A < B;
  });
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::LaunchTest(const MString& TestName, int& ChildPid, MString& OutputFile) const
{
  ChildPid = -1;
  const auto Test = m_AllTests.find(TestName);
  if (Test == m_AllTests.end()) return false;

  OutputFile = MFile::CreateTemporaryFile(m_RunnerName + ".log");
  if (OutputFile.IsEmpty() == true) return false;

  pid_t Pid = fork();
  if (Pid < 0) {
    MFile::Remove(OutputFile);
    OutputFile = "";
    return false;
  }
  if (Pid == 0) {
    int Output = open(OutputFile.Data(), O_WRONLY | O_TRUNC);
    if (Output < 0) _exit(127);
    dup2(Output, STDOUT_FILENO);
    dup2(Output, STDERR_FILENO);
    close(Output);
    execl(Test->second.Data(), Test->second.Data(), static_cast<char*>(nullptr));
    _exit(127);
  }

  ChildPid = Pid;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ReadOutput(const MString& OutputFile) const
{
  ifstream Input(OutputFile.Data());
  stringstream Buffer;
  Buffer<<Input.rdbuf();
  return Buffer.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ExtractMetric(const MString& Output) const
{
  string Text = Output.Data();
  size_t Passed = Text.rfind("Passed tests:");
  size_t Failed = Text.rfind("Failed tests:");
  if (Passed == string::npos || Failed == string::npos || Failed < Passed) return "done";

  size_t PassedEnd = Text.find('\n', Passed);
  size_t FailedEnd = Text.find('\n', Failed);
  return MString(Text.substr(Passed, PassedEnd - Passed).c_str()) + ", "
       + MString(Text.substr(Failed, FailedEnd - Failed).c_str());
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::FormatFailureMetric(const MString& Metric) const
{
  string Text = Metric.Data();
  auto ReadValue = [&](const char* Prefix) -> long {
    size_t Position = Text.find(Prefix);
    if (Position == string::npos) return -1;
    Position += strlen(Prefix);
    while (Position < Text.size() && isspace(static_cast<unsigned char>(Text[Position])) != 0) ++Position;
    char* End = nullptr;
    long Value = strtol(Text.c_str() + Position, &End, 10);
    return End == Text.c_str() + Position ? -1 : Value;
  };
  long Passed = ReadValue("Passed tests:");
  long Failed = ReadValue("Failed tests:");
  if (Passed < 0 || Failed < 0) return Metric;
  return MString(Failed) + "/" + Passed + " failed";
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::FormatRuntime(double Seconds) const
{
  ostringstream Output;
  Output.setf(ios::fixed);
  Output.precision(1);
  Output<<Seconds<<"s";
  return Output.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::WriteFailureReport(const vector<MString>& Tests, const vector<EStatus>& Statuses,
                                            const vector<MString>& Metrics, const vector<MString>& Outputs) const
{
  time_t Now = time(nullptr);
  tm LocalTime;
  localtime_r(&Now, &LocalTime);
  char TimeTag[32];
  strftime(TimeTag, sizeof(TimeTag), "%Y%m%dT%H%M%S", &LocalTime);

  MString FileName = m_RunnerName + "_failed_" + TimeTag + ".log";
  ofstream Report(FileName.Data(), ios::trunc);
  if (Report.is_open() == false) return "";

  Report<<m_RunnerName<<" failed unit-test output report\nCreated: "<<TimeTag<<"\n";
  for (size_t i = 0; i < Tests.size(); ++i) {
    if (Statuses[i] == c_StatusPassed) continue;
    Report<<"\n================================================================================\n";
    Report<<"Test: "<<Tests[i]<<"\nMetric: "<<Metrics[i]<<"\n";
    Report<<"--------------------------------------------------------------------------------\n";
    Report<<(Outputs[i].IsEmpty() == false ? Outputs[i] : "No output captured.\n");
    if (Outputs[i].IsEmpty() == false && Outputs[i].EndsWith("\n") == false) Report<<"\n";
  }
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


int UTExecute::Execute(int argc, char** argv)
{
  m_BinDirectory = GetBinDirectory(argc > 0 ? argv[0] : nullptr);
  m_TimingFile = GetTimingFile();
  if (DiscoverTests() == false || m_AllTests.empty() == true) {
    merr<<"No unit test executables found in "<<m_BinDirectory<<show;
    return 1;
  }

  map<MString, double> Timings;
  LoadTimings(Timings);
  vector<MString> Tests;
  if (BuildRequestedTests(argc, argv, Tests) == false) return 1;
  SortRequestedTests(Tests, Timings);

  vector<EStatus> Statuses(Tests.size(), c_StatusPending);
  vector<MString> Outputs(Tests.size(), "");
  vector<MString> OutputFiles(Tests.size(), "");
  vector<MString> Metrics(Tests.size(), "");
  vector<chrono::steady_clock::time_point> StartTimes(Tests.size());
  map<pid_t, size_t> PidToIndex;
  const chrono::steady_clock::time_point SuiteStart = chrono::steady_clock::now();
  unsigned int MaxParallel = 0;
#ifdef __APPLE__
  size_t CpuSize = sizeof(MaxParallel);
  sysctlbyname("hw.physicalcpu", &MaxParallel, &CpuSize, nullptr, 0);
#else
  MaxParallel = thread::hardware_concurrency();
#endif
  if (MaxParallel == 0) MaxParallel = 1;
  const bool UseTTY = isatty(STDOUT_FILENO) != 0;
  unsigned int Spinner = 0;

  size_t TerminalWidth = 100;
  if (UseTTY == true) {
    winsize Size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &Size) == 0 && Size.ws_col > 0) TerminalWidth = Size.ws_col;
  }

  auto Clip = [](const MString& Text, size_t Width) {
    if (Text.Length() <= Width) return Text;
    return Width > 3 ? Text.GetSubString(0, Width - 3) + "..." : Text.GetSubString(0, Width);
  };
  auto Render = [&]() {
    if (UseTTY == false) return;
    unsigned int Done = 0;
    unsigned int Running = 0;
    unsigned int Failed = 0;
    vector<MString> Left = {"", "", "", "", "Failed runs:"};
    vector<MString> Right = {"Running tests:"};
    for (size_t i = 0; i < Tests.size(); ++i) {
      if (Statuses[i] != c_StatusPending && Statuses[i] != c_StatusRunning) ++Done;
      if (Statuses[i] == c_StatusRunning) {
        ++Running;
        const char SpinnerChars[] = {'|', '/', '-', '\\'};
        Right.push_back(MString(SpinnerChars[Spinner % 4]) + " " + Tests[i]);
      } else if (Statuses[i] == c_StatusFailed) {
        ++Failed;
        Left.push_back(Tests[i] + " (" + FormatFailureMetric(Metrics[i]) + ")");
      }
    }
    Left[0] = MString("Done: ") + Done + "/" + Tests.size();
    Left[1] = MString("Running: ") + Running;
    Left[2] = MString("Failed: ") + Failed;
    if (Left.size() == 5) Left.push_back("none");
    if (Right.size() == 1) Right.push_back("none");
    while (Right.size() < MaxParallel + 1) Right.push_back("");

    const size_t ProgressWidth = 32;
    const size_t Filled = Tests.empty() ? ProgressWidth : Done * ProgressWidth / Tests.size();
    MString Bar = "[";
    for (size_t i = 0; i < ProgressWidth; ++i) Bar += i < Filled ? "#" : ".";
    Bar += "]";
    double Elapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - SuiteStart).count();
    MString Progress = MString("Progress: ") + Bar + "  " + Done + "/" + Tests.size() + "  " + FormatRuntime(Elapsed);

    size_t PaneWidth = 24;
    for (const MString& Line : Left) PaneWidth = max(PaneWidth, Line.Length());
    for (const MString& Line : Right) PaneWidth = max(PaneWidth, Line.Length());
    PaneWidth = max(PaneWidth, Progress.Length() / 2);
    const size_t MaximumPaneWidth = TerminalWidth > 7 ? (TerminalWidth - 7) / 2 : 1;
    PaneWidth = min(PaneWidth, MaximumPaneWidth);
    const size_t InnerWidth = PaneWidth * 2 + 5;
    auto Pad = [](const MString& Text, size_t Width) { return Text + MString(string(Width - Text.Length(), ' ')); };
    auto Border = [&]() { return MString("+") + MString(string(InnerWidth, '-')) + "+"; };
    auto Center = [&](const MString& Text) {
      MString Value = Clip(Text, InnerWidth);
      size_t LeftPad = (InnerWidth - Value.Length()) / 2;
      return MString("|") + MString(string(LeftPad, ' ')) + Value
           + MString(string(InnerWidth - LeftPad - Value.Length(), ' ')) + "|";
    };

    cout<<"\x1b[2J\x1b[H"<<Border()<<"\n"<<Center(m_DashboardTitle)<<"\n"<<Border()<<"\n";
    for (size_t i = 0; i < max(Left.size(), Right.size()); ++i) {
      MString L = Clip(i < Left.size() ? Left[i] : "", PaneWidth);
      MString R = Clip(i < Right.size() ? Right[i] : "", PaneWidth);
      cout<<"| "<<Pad(L, PaneWidth)<<" | "<<Pad(R, PaneWidth)<<" |\n";
    }
    cout<<Border()<<"\n"<<Center(Progress)<<"\n"<<Border()<<"\n"<<flush;
  };

  if (UseTTY == true) cout<<"\x1b[?25l"<<flush;
  Render();
  size_t Next = 0;
  size_t Running = 0;
  while (Next < Tests.size() || Running > 0) {
    while (Next < Tests.size() && Running < MaxParallel) {
      int Child = -1;
      if (LaunchTest(Tests[Next], Child, OutputFiles[Next]) == false) {
        Statuses[Next] = c_StatusFailed;
        Metrics[Next] = "Failed to launch test process";
        ++Next;
        continue;
      }
      PidToIndex[Child] = Next;
      StartTimes[Next] = chrono::steady_clock::now();
      Statuses[Next++] = c_StatusRunning;
      ++Running;
    }
    Render();

    int ChildStatus = 0;
    pid_t Child = waitpid(-1, &ChildStatus, WNOHANG);
    if (Child < 0) {
      if (errno == EINTR) continue;
      break;
    }
    if (Child == 0) {
      ++Spinner;
      usleep(1000000);
      continue;
    }
    const auto Found = PidToIndex.find(Child);
    if (Found == PidToIndex.end()) continue;
    const size_t Index = Found->second;
    PidToIndex.erase(Found);
    Outputs[Index] = ReadOutput(OutputFiles[Index]);
    MFile::Remove(OutputFiles[Index]);
    Metrics[Index] = ExtractMetric(Outputs[Index]);
    Timings[Tests[Index]] = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - StartTimes[Index]).count();
    Statuses[Index] = WIFEXITED(ChildStatus) != 0 && WEXITSTATUS(ChildStatus) == 0 ? c_StatusPassed : c_StatusFailed;
    --Running;
  }

  unsigned int Failed = 0;
  for (EStatus Status : Statuses) if (Status != c_StatusPassed) ++Failed;
  Render();
  if (UseTTY == true) cout<<"\x1b[?25h\n"<<flush;
  if (UseTTY == false) {
    for (size_t i = 0; i < Tests.size(); ++i) {
      cout<<(Statuses[i] == c_StatusPassed ? "PASS " : "")<<Tests[i]<<" ("<<Metrics[i]<<")\n";
    }
  }
  if (Failed > 0) {
    MString Report = WriteFailureReport(Tests, Statuses, Metrics, Outputs);
    cout<<"Failed unit-test output report: "<<(Report.IsEmpty() == false ? Report : "unable to write report file")<<"\n";
  }
  SaveTimings(Timings);
  return Failed == 0 ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  UTExecute Execute("UT", "UTExecute", "MEGAlib unit testing dashboard");
  return Execute.Execute(argc, argv);
}


////////////////////////////////////////////////////////////////////////////////
