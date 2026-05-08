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
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <dirent.h>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
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
  //! Determine the timing cache file in the user's config directory
  MString GetTimingFile() const;
  //! Ensure a directory exists, creating parents as needed
  bool EnsureDirectoryExists(const MString& Path) const;
  //! Return true if the path points to an executable file
  bool IsExecutableFile(const MString& Path) const;
  //! Normalize one command-line request
  MString NormalizeRequest(const MString& Input) const;
  //! Resolve one command-line request to a discovered test name
  MString ResolveRequest(const MString& Input) const;
  //! Discover all available UT* executables in the bin directory
  bool DiscoverTests();
  //! Load prior runtimes from the timing cache
  bool LoadTimings(map<MString, double>& Timings) const;
  //! Persist runtimes to the timing cache
  bool SaveTimings(const map<MString, double>& Timings) const;
  //! Build the list of tests which should be run
  bool BuildRequestedTests(int argc, char** argv, vector<MString>& RequestedTests) const;
  //! Sort requested tests so the slowest previous runs are started first
  void SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const;
  //! Launch a test executable and capture its output
  bool LaunchTest(const MString& TestName, pid_t& ChildPid, MString& OutputFile) const;
  //! Read the captured output file
  MString ReadOutput(const MString& OutputFile) const;
  //! Extract the final metric line from a captured test output
  MString ExtractMetric(const MString& Output) const;
  //! Format a concise failure label from the captured metric line
  MString FormatFailureMetric(const MString& Metric) const;
  //! Format a runtime with fixed width for the dashboard
  MString FormatRuntime(double Seconds) const;

  //! The bin directory containing the test executables
  MString m_BinDirectory;
  //! All discovered unit tests and their paths
  map<MString, MString> m_AllTests;
  //! File containing the runtime cache
  MString m_TimingFile;
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


MString UTExecute::GetTimingFile() const
{
  const char* Home = getenv("HOME");

  MString BaseDir;
#ifdef __APPLE__
  if (Home != nullptr && Home[0] != '\0') {
    BaseDir = MString(Home) + "/Library/Application Support";
  } else {
    BaseDir = "/tmp";
  }
#else
  const char* XdgConfigHome = getenv("XDG_CONFIG_HOME");
  if (XdgConfigHome != nullptr && XdgConfigHome[0] != '\0') {
    BaseDir = XdgConfigHome;
  } else if (Home != nullptr && Home[0] != '\0') {
    BaseDir = MString(Home) + "/.config";
  } else {
    BaseDir = "/tmp";
  }
#endif

  MString ConfigDir = BaseDir + "/MEGAlib";
  if (EnsureDirectoryExists(ConfigDir) == false) {
    return "";
  }

  return ConfigDir + "/UTExecute.timings";
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::EnsureDirectoryExists(const MString& Path) const
{
  if (Path == "") {
    return false;
  }

  string Current;
  string Input = Path.Data();
  size_t Start = 0;
  if (Input[0] == '/') {
    Current = "/";
    Start = 1;
  }

  while (Start <= Input.size()) {
    size_t End = Input.find('/', Start);
    string Part = Input.substr(Start, End == string::npos ? string::npos : End - Start);
    if (Part.empty() == false) {
      if (Current.empty() == false && Current.back() != '/') {
        Current += "/";
      }
      Current += Part;
      if (mkdir(Current.c_str(), 0755) != 0 && errno != EEXIST) {
        return false;
      }
    }
    if (End == string::npos) {
      break;
    }
    Start = End + 1;
  }

  return true;
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


bool UTExecute::LoadTimings(map<MString, double>& Timings) const
{
  Timings.clear();

  if (m_TimingFile == "") {
    return true;
  }

  ifstream Input(m_TimingFile.Data());
  if (Input.is_open() == false) {
    return true;
  }

  string Line;
  while (getline(Input, Line)) {
    if (Line.empty() == true || Line[0] == '#') {
      continue;
    }
    stringstream Stream(Line);
    string Name;
    double Time = 0.0;
    if ((Stream >> Name >> Time).fail()) {
      continue;
    }
    if (Time < 0.0) {
      continue;
    }
    Timings[MString(Name.c_str())] = Time;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::SaveTimings(const map<MString, double>& Timings) const
{
  if (m_TimingFile == "") {
    return true;
  }

  ofstream Output(m_TimingFile.Data(), ios::trunc);
  if (Output.is_open() == false) {
    return true;
  }

  Output << "# UTExecute timing cache" << '\n';
  Output << "# test_name seconds" << '\n';
  for (map<MString, double>::const_iterator It = Timings.begin(); It != Timings.end(); ++It) {
    Output << It->first.Data() << ' ' << It->second << '\n';
  }

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


void UTExecute::SortRequestedTests(vector<MString>& RequestedTests, const map<MString, double>& Timings) const
{
  stable_sort(RequestedTests.begin(), RequestedTests.end(), [&](const MString& A, const MString& B) {
    map<MString, double>::const_iterator Ta = Timings.find(A);
    map<MString, double>::const_iterator Tb = Timings.find(B);
    double TimeA = Ta != Timings.end() ? Ta->second : -1.0;
    double TimeB = Tb != Timings.end() ? Tb->second : -1.0;
    if (TimeA != TimeB) {
      return TimeA > TimeB;
    }
    return A < B;
  });
}


////////////////////////////////////////////////////////////////////////////////


bool UTExecute::LaunchTest(const MString& TestName, pid_t& ChildPid, MString& OutputFile) const
{
  ChildPid = -1;
  OutputFile = "";

  map<MString, MString>::const_iterator Test = m_AllTests.find(TestName);
  if (Test == m_AllTests.end()) {
    return false;
  }

  char Template[] = "/tmp/UTExecute_XXXXXX";
  int FD = mkstemp(Template);
  if (FD < 0) {
    return false;
  }
  close(FD);
  OutputFile = Template;

  pid_t Pid = fork();
  if (Pid < 0) {
    unlink(OutputFile.Data());
    OutputFile = "";
    return false;
  }

  if (Pid == 0) {
    int Out = open(OutputFile.Data(), O_WRONLY | O_TRUNC);
    if (Out < 0) {
      _exit(127);
    }
    dup2(Out, STDOUT_FILENO);
    dup2(Out, STDERR_FILENO);
    close(Out);

    execl(Test->second.Data(), Test->second.Data(), (char*) nullptr);
    _exit(127);
  }

  ChildPid = Pid;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ReadOutput(const MString& OutputFile) const
{
  ifstream Input(OutputFile.Data());
  if (Input.is_open() == false) {
    return "";
  }

  stringstream Buffer;
  Buffer << Input.rdbuf();
  return Buffer.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::ExtractMetric(const MString& Output) const
{
  string Text = Output.Data();
  size_t PassedPos = Text.rfind("Passed tests:");
  size_t FailedPos = Text.rfind("Failed tests:");
  if (PassedPos != string::npos && FailedPos != string::npos && FailedPos > PassedPos) {
    size_t PassedEnd = Text.find('\n', PassedPos);
    size_t FailedEnd = Text.find('\n', FailedPos);
    MString PassedLine = Text.substr(PassedPos, PassedEnd == string::npos ? string::npos : PassedEnd - PassedPos).c_str();
    MString FailedLine = Text.substr(FailedPos, FailedEnd == string::npos ? string::npos : FailedEnd - FailedPos).c_str();
    return PassedLine + ", " + FailedLine;
  }

  return "done";
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::FormatFailureMetric(const MString& Metric) const
{
  string Text = Metric.Data();
  size_t PassedPos = Text.find("Passed tests:");
  size_t FailedPos = Text.find("Failed tests:");
  if (PassedPos == string::npos || FailedPos == string::npos || FailedPos < PassedPos) {
    return Metric;
  }

  auto ParseValue = [&](size_t Pos, const char* Prefix) -> long {
    size_t Start = Text.find(Prefix, Pos);
    if (Start == string::npos) {
      return -1;
    }
    Start += strlen(Prefix);
    while (Start < Text.size() && isspace(static_cast<unsigned char>(Text[Start])) != 0) {
      ++Start;
    }
    char* End = nullptr;
    long Value = strtol(Text.c_str() + Start, &End, 10);
    if (End == Text.c_str() + Start) {
      return -1;
    }
    return Value;
  };

  long Passed = ParseValue(PassedPos, "Passed tests:");
  long Failed = ParseValue(FailedPos, "Failed tests:");
  if (Passed < 0 || Failed < 0) {
    return Metric;
  }

  return MString(Failed) + "/" + Passed + " failed";
}


////////////////////////////////////////////////////////////////////////////////


MString UTExecute::FormatRuntime(double Seconds) const
{
  ostringstream Out;
  Out.setf(ios::fixed);
  Out.precision(1);
  Out << Seconds << "s";
  return Out.str().c_str();
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

  vector<MString> RequestedTests;
  if (BuildRequestedTests(argc, argv, RequestedTests) == false) {
    return 1;
  }
  SortRequestedTests(RequestedTests, Timings);

  const int c_StatusPending = 0;
  const int c_StatusRunning = 1;
  const int c_StatusFailed = 2;
  const int c_StatusPassed = 3;

  vector<int> Statuses(RequestedTests.size(), c_StatusPending);
  vector<MString> Outputs(RequestedTests.size(), "");
  vector<MString> OutputFiles(RequestedTests.size(), "");
  map<pid_t, size_t> PidToIndex;
  vector<MString> Metrics(RequestedTests.size(), "");
  vector<chrono::steady_clock::time_point> StartTimes(RequestedTests.size());
  vector<double> Durations(RequestedTests.size(), 0.0);
  chrono::steady_clock::time_point SuiteStart = chrono::steady_clock::now();
  unsigned int MaxParallel = 1;
  bool UseTTY = isatty(STDOUT_FILENO) != 0;
  struct TerminalSession
  {
    bool Enabled = false;
    TerminalSession(bool IsEnabled) : Enabled(IsEnabled)
    {
      if (Enabled == true) {
        cout << "\x1b[?25l" << flush;
      }
    }
    ~TerminalSession()
    {
      if (Enabled == true) {
        cout << "\x1b[?25h" << flush;
      }
    }
  } TtySession(UseTTY);
  unsigned int SpinnerIndex = 0;
  const char SpinnerChars[] = {'|', '/', '-', '\\'};

  size_t TerminalWidth = 100;
  if (UseTTY == true) {
    winsize WinSize;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &WinSize) == 0 && WinSize.ws_col > 0) {
      TerminalWidth = WinSize.ws_col;
    }
  }
  if (TerminalWidth < 80) {
    TerminalWidth = 80;
  }

  auto Render = [&]() {
    if (UseTTY == false) {
      return;
    }

    vector<MString> LeftLines;
    vector<MString> RightLines;
    unsigned int Done = 0;
    unsigned int RunningCount = 0;
    unsigned int FailedCount = 0;
    const size_t ProgressBarWidth = 32;

    for (size_t i = 0; i < RequestedTests.size(); ++i) {
      if (Statuses[i] != c_StatusPending && Statuses[i] != c_StatusRunning) {
        ++Done;
      }
      if (Statuses[i] == c_StatusRunning) {
        ++RunningCount;
      } else if (Statuses[i] == c_StatusFailed) {
        ++FailedCount;
      }
    }

    LeftLines.push_back(MString("Done: ") + Done + "/" + RequestedTests.size());
    LeftLines.push_back(MString("Running: ") + RunningCount);
    LeftLines.push_back(MString("Failed: ") + FailedCount);
    LeftLines.push_back("");
    LeftLines.push_back("Failed runs:");
    for (size_t i = 0; i < RequestedTests.size(); ++i) {
      if (Statuses[i] == c_StatusFailed) {
        MString Line = RequestedTests[i];
        if (Metrics[i].Length() > 0) {
          Line += MString(" (") + FormatFailureMetric(Metrics[i]) + ")";
        }
        LeftLines.push_back(Line);
      }
    }
    if (LeftLines.size() == 5) {
      LeftLines.push_back("none");
    }

    RightLines.push_back("Running tests:");
    bool AnyRunning = false;
    for (size_t i = 0; i < RequestedTests.size(); ++i) {
      if (Statuses[i] == c_StatusRunning) {
        AnyRunning = true;
        MString Line = MString(SpinnerChars[SpinnerIndex % 4]) + " " + RequestedTests[i];
        if (Metrics[i].Length() > 0) {
          Line += MString(" (") + Metrics[i] + ")";
        }
        RightLines.push_back(Line);
      }
    }
    if (AnyRunning == false) {
      RightLines.push_back("none");
    }
    while (RightLines.size() < MaxParallel + 1) {
      RightLines.push_back("");
    }

    size_t Filled = RequestedTests.empty() == true ? ProgressBarWidth : (Done * ProgressBarWidth) / RequestedTests.size();
    MString Bar = "[";
    for (size_t i = 0; i < ProgressBarWidth; ++i) {
      Bar += (i < Filled ? "#" : ".");
    }
    Bar += "]";
    chrono::steady_clock::time_point Now = chrono::steady_clock::now();
    double Elapsed = chrono::duration_cast<chrono::duration<double>>(Now - SuiteStart).count();
    MString Progress = MString("Progress: ") + Bar + MString("  ") + Done + "/" + RequestedTests.size()
                     + MString("  ") + FormatRuntime(Elapsed);

    size_t LeftRequiredWidth = 0;
    for (size_t i = 0; i < LeftLines.size(); ++i) {
      if (LeftLines[i].Length() > LeftRequiredWidth) {
        LeftRequiredWidth = LeftLines[i].Length();
      }
    }

    size_t RightRequiredWidth = 0;
    for (size_t i = 0; i < RightLines.size(); ++i) {
      if (RightLines[i].Length() > RightRequiredWidth) {
        RightRequiredWidth = RightLines[i].Length();
      }
    }

    size_t MaxInnerWidth = TerminalWidth > 2 ? TerminalWidth - 2 : TerminalWidth;
    size_t PaneWidth = LeftRequiredWidth > RightRequiredWidth ? LeftRequiredWidth : RightRequiredWidth;
    if (PaneWidth < 24) {
      PaneWidth = 24;
    }
    if (PaneWidth * 2 + 5 > MaxInnerWidth) {
      if (MaxInnerWidth > 3) {
        PaneWidth = (MaxInnerWidth > 5) ? (MaxInnerWidth - 5) / 2 : 1;
      } else {
        PaneWidth = 1;
      }
    }
    size_t MinPaneForProgress = Progress.Length() / 2;
    if (MinPaneForProgress > PaneWidth) {
      PaneWidth = MinPaneForProgress;
    }
    size_t LeftWidth = PaneWidth;
    size_t RightWidth = PaneWidth;
    size_t InnerWidth = LeftWidth + RightWidth + 5;

    auto Clip = [&](const MString& Text, size_t Width) -> MString {
      if (Text.Length() <= Width) {
        return Text;
      }
      if (Width <= 3) {
        return Text.GetSubString(0, Width);
      }
      return Text.GetSubString(0, Width - 3) + "...";
    };

    auto Center = [&](const MString& Text, size_t Width) -> MString {
      MString Clipped = Clip(Text, Width);
      if (Clipped.Length() >= Width) {
        return Clipped;
      }
      size_t PadLeft = (Width - Clipped.Length()) / 2;
      size_t PadRight = Width - Clipped.Length() - PadLeft;
      return MString(string(PadLeft, ' ')) + Clipped + MString(string(PadRight, ' '));
    };

    auto Border = [&]() -> MString {
      return MString("+") + MString(string(InnerWidth, '-')) + "+";
    };

    auto MakeRow = [&](const MString& Left, const MString& Right) -> MString {
      MString LeftClipped = Clip(Left, LeftWidth);
      MString RightClipped = Clip(Right, RightWidth);
      return MString("| ") + LeftClipped + MString(string(LeftWidth - LeftClipped.Length(), ' '))
           + MString(" | ") + RightClipped + MString(string(RightWidth - RightClipped.Length(), ' '))
           + MString(" |");
    };

    auto MakeCenteredRow = [&](const MString& Text) -> MString {
      if (InnerWidth <= 2) {
        return MString("|") + Text + MString("|");
      }
      return MString("|") + Center(Text, InnerWidth) + MString("|");
    };

    size_t MaxLines = LeftLines.size() > RightLines.size() ? LeftLines.size() : RightLines.size();
    cout << "\x1b[2J\x1b[H";
    cout << Border() << '\n';
    cout << MakeCenteredRow("MEGAlib unit testing dashboard") << '\n';
    cout << Border() << '\n';
    for (size_t i = 0; i < MaxLines; ++i) {
      MString Left = i < LeftLines.size() ? LeftLines[i] : "";
      MString Right = i < RightLines.size() ? RightLines[i] : "";
      cout << MakeRow(Left, Right) << '\n';
    }
    cout << Border() << '\n';
    cout << MakeCenteredRow(Progress) << '\n';
    cout << Border() << '\n';
    cout.flush();
  };

  Render();

#ifdef __APPLE__
  unsigned int PhysicalCpuCount = 0;
  size_t CpuSize = sizeof(PhysicalCpuCount);
  if (sysctlbyname("hw.physicalcpu", &PhysicalCpuCount, &CpuSize, nullptr, 0) == 0 && PhysicalCpuCount > 0) {
    MaxParallel = PhysicalCpuCount;
  } else
#endif
  {
  long CpuCount = sysconf(_SC_NPROCESSORS_ONLN);
  if (CpuCount > 0) {
    MaxParallel = static_cast<unsigned int>(CpuCount);
    }
  }

  size_t NextParallel = 0;
  size_t Running = 0;
  while (NextParallel < RequestedTests.size() || Running > 0) {
    while (NextParallel < RequestedTests.size() && Running < MaxParallel) {
      pid_t ChildPid = -1;
      MString OutputFile;
      if (LaunchTest(RequestedTests[NextParallel], ChildPid, OutputFile) == false) {
        Statuses[NextParallel] = c_StatusFailed;
        Metrics[NextParallel] = "Failed to launch test process";
        ++NextParallel;
        Render();
        continue;
      }
      PidToIndex[ChildPid] = NextParallel;
      OutputFiles[NextParallel] = OutputFile;
      StartTimes[NextParallel] = chrono::steady_clock::now();
      ++NextParallel;
      ++Running;
      Statuses[NextParallel - 1] = c_StatusRunning;
      Render();
    }

    int ChildStatus = 0;
    pid_t Child = waitpid(-1, &ChildStatus, WNOHANG);
    if (Child < 0) {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
    if (Child == 0) {
      ++SpinnerIndex;
      Render();
      usleep(1000000);
      continue;
    }

    map<pid_t, size_t>::iterator It = PidToIndex.find(Child);
    if (It != PidToIndex.end()) {
      size_t Index = It->second;
      PidToIndex.erase(It);
      Outputs[Index] = ReadOutput(OutputFiles[Index]);
      unlink(OutputFiles[Index].Data());
      Metrics[Index] = ExtractMetric(Outputs[Index]);
      chrono::steady_clock::time_point EndTime = chrono::steady_clock::now();
      double Seconds = chrono::duration_cast<chrono::duration<double>>(EndTime - StartTimes[Index]).count();
      if (Seconds >= 0.0) {
        Durations[Index] = Seconds;
        Timings[RequestedTests[Index]] = Seconds;
      }
      if (WIFEXITED(ChildStatus) != 0 && WEXITSTATUS(ChildStatus) == 0) {
        Statuses[Index] = c_StatusPassed;
      } else {
        Statuses[Index] = c_StatusFailed;
      }
      --Running;
      Render();
    }
  }

  int Failed = 0;
  int Total = 0;
  for (size_t i = 0; i < RequestedTests.size(); ++i) {
    ++Total;
    if (Statuses[i] != c_StatusPassed) {
      ++Failed;
    }
  }

  if (UseTTY == false) {
    for (size_t i = 0; i < RequestedTests.size(); ++i) {
      if (Statuses[i] == c_StatusPassed) {
        cout << "PASS " << RequestedTests[i];
      } else {
        cout << RequestedTests[i];
      }
      if (Metrics[i].Length() > 0) {
        if (Statuses[i] == c_StatusPassed) {
          cout << " (" << Metrics[i] << ")";
        } else {
          cout << " (" << FormatFailureMetric(Metrics[i]) << ")";
        }
      }
      cout << '\n';
    }
  } else {
    Render();
    cout << '\n';
  }

  SaveTimings(Timings);

  return Failed == 0 ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  UTExecute Execute;
  return Execute.Execute(argc, argv);
}
