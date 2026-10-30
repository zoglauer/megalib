#include "MGUICADConverter.h"

#include "MGUIGeomegaMain.h"
#include "MGlobal.h"
#include <TGButton.h>
#include <TGFileDialog.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TSystem.h>
#include <TTimer.h>

#include <cerrno>
#include <cmath>
#include <csignal>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

#ifdef ___CLING___
ClassImp(MGUICADConverter)
#endif

namespace {
enum EControlIDs {
  c_SelectFile = 1001, c_SelectDest, c_Convert, c_ViewGeomega, c_Cancel
};

bool IsFile(const TString& Path, int Mode)
{
  struct stat Info;
  return stat(Path.Data(), &Info) == 0 && S_ISREG(Info.st_mode) &&
         access(Path.Data(), Mode) == 0;
}

TString AbsolutePath(const TString& Path)
{
  // File dialogs supply literal paths: do not expand '$' or shell syntax.
  if (Path.BeginsWith("/")) return Path;
  return TString(gSystem->WorkingDirectory()) + "/" + Path;
}

std::string Number(double Value)
{
  std::ostringstream Stream;
  Stream.imbue(std::locale::classic());
  Stream << std::setprecision(17) << Value;
  return Stream.str();
}
}

MGUICADConverter::MGUICADConverter(const TGWindow* Parent, MGUIGeomegaMain* Main,
                                 UInt_t Width, UInt_t Height)
  : TGTransientFrame(Parent, Main, Width, Height), fMain(Main)
{
  SetCleanup(kDeepCleanup);
  SetWindowName("STEP to Geomega");
  auto Expand = new TGLayoutHints(kLHintsExpandX, 6, 6, 4, 4);
  auto Left = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 6, 6, 4, 4);

  auto Source = new TGHorizontalFrame(this);
  fSourcePath = new TGTextEntry(Source, "Select a STEP file...");
  fSourcePath->SetEnabled(kFALSE);
  Source->AddFrame(fSourcePath, Expand);
  fBtnSelectFile = new TGTextButton(Source, "STEP file...", c_SelectFile);
  fBtnSelectFile->Associate(this);
  Source->AddFrame(fBtnSelectFile, Left);
  AddFrame(Source, Expand);

  auto Destination = new TGHorizontalFrame(this);
  fDestPath = new TGTextEntry(Destination, "Destination defaults to the source directory");
  fDestPath->SetEnabled(kFALSE);
  Destination->AddFrame(fDestPath, Expand);
  fBtnDestBrowse = new TGTextButton(Destination, "Destination...", c_SelectDest);
  fBtnDestBrowse->Associate(this);
  Destination->AddFrame(fBtnDestBrowse, Left);
  AddFrame(Destination, Expand);

  auto Formats = new TGHorizontalFrame(this);
  fCheckGeo = new TGCheckButton(Formats, "Geomega (.geo.setup + meshes)");
  fCheckObj = new TGCheckButton(Formats, "OBJ");
  fCheckStl = new TGCheckButton(Formats, "STL");
  fCheckGeo->SetOn(kTRUE);
  Formats->AddFrame(fCheckGeo, Left);
  Formats->AddFrame(fCheckObj, Left);
  Formats->AddFrame(fCheckStl, Left);
  AddFrame(Formats, Expand);

  auto AddNumber = [&](const char* Label, double Default, double Maximum) {
    auto Row = new TGHorizontalFrame(this);
    Row->AddFrame(new TGLabel(Row, Label), Left);
    auto Entry = new TGNumberEntry(Row, Default, 10, -1, TGNumberFormat::kNESReal,
                                  TGNumberFormat::kNEANonNegative,
                                  Maximum > 0 ? TGNumberFormat::kNELLimitMinMax : TGNumberFormat::kNELLimitMin,
                                  0, Maximum);
    Row->AddFrame(Entry, new TGLayoutHints(kLHintsRight, 6, 6, 4, 4));
    AddFrame(Row, Expand);
    return Entry;
  };
  fNumAngDef = AddNumber("Angular deflection [degrees, > 0 to 180]", 20, 180);
  fNumMaxDev = AddNumber("Maximum deviation [mm, 0 = automatic]", 0, 0);
  fNumMinVol = AddNumber("Minimum solid volume [mm^3, 0 = keep all]", 0, 0);
  AddFrame(new TGLabel(this, "Geomega uses cm. Exported materials are Aluminium placeholders; edit before simulation."), Left);
  AddFrame(new TGLabel(this, "Existing outputs with the same model name will be replaced."), Left);

  auto Actions = new TGHorizontalFrame(this);
  fBtnConvert = new TGTextButton(Actions, "Convert", c_Convert);
  fBtnCancel = new TGTextButton(Actions, "Cancel conversion", c_Cancel);
  fBtnViewGeomega = new TGTextButton(Actions, "View in Geomega", c_ViewGeomega);
  for (auto Button : {fBtnConvert, fBtnCancel, fBtnViewGeomega}) {
    Button->Associate(this);
    Actions->AddFrame(Button, Expand);
  }
  fBtnConvert->SetEnabled(kFALSE);
  fBtnCancel->SetEnabled(kFALSE);
  fBtnViewGeomega->SetEnabled(kFALSE);
  AddFrame(Actions, Expand);

  fStatusLabel = new TGLabel(this, "Select a STEP file to begin.");
  AddFrame(fStatusLabel, Left);
  fLogView = new TGTextView(this, Width - 20, 220);
  AddFrame(fLogView, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 6, 6, 4, 6));
  fTimer = new TTimer(this, 250, kTRUE);
  MapSubwindows();
  Resize(Width, Height);
  Layout();
  CenterOnParent();
  MapWindow();
}

MGUICADConverter::~MGUICADConverter()
{
  delete fTimer;
  // Application shutdown must not leave an unattended converter or a zombie.
  if (IsConverting()) {
    kill(fChildPid, SIGKILL);
    while (waitpid(fChildPid, nullptr, 0) < 0 && errno == EINTR) { }
  }
}

void MGUICADConverter::SetStatus(const TString& Status)
{
  fStatusLabel->SetText(Status.Data());
  Layout();
}

void MGUICADConverter::SetBusy(bool Busy)
{
  fBtnSelectFile->SetEnabled(!Busy);
  fBtnDestBrowse->SetEnabled(!Busy);
  fBtnConvert->SetEnabled(!Busy && !fInputFilePath.IsNull());
  fBtnCancel->SetEnabled(Busy);
  for (auto Check : {fCheckGeo, fCheckObj, fCheckStl}) Check->SetEnabled(!Busy);
  for (auto Entry : {fNumAngDef, fNumMaxDev, fNumMinVol}) Entry->SetState(!Busy);
}

bool MGUICADConverter::SetInputFile(const TString& FileName)
{
  if (IsConverting()) return false;
  TString Extension = FileName;
  Extension.ToLower();
  if ((!Extension.EndsWith(".step") && !Extension.EndsWith(".stp")) ||
      !IsFile(FileName, R_OK)) {
    SetStatus("Select a readable .step or .stp file.");
    return false;
  }
  fInputFilePath = AbsolutePath(FileName);
  fSourcePath->SetText(fInputFilePath);
  fOutputFilePath = "";
  fBtnViewGeomega->SetEnabled(kFALSE);
  SetOutputDirectory(gSystem->DirName(fInputFilePath));
  fBtnConvert->SetEnabled(kTRUE);
  SetStatus("Ready to convert.");
  return true;
}

bool MGUICADConverter::SetOutputDirectory(const TString& Directory)
{
  if (IsConverting() || Directory.IsNull()) return false;
  fOutputDir = AbsolutePath(Directory);
  fDestPath->SetText(fOutputDir);
  fOutputFilePath = "";
  fBtnViewGeomega->SetEnabled(kFALSE);
  return true;
}

void MGUICADConverter::OnSelectFile()
{
  if (IsConverting()) return;
  static const char* Types[] = {"STEP files", "*.step", "STEP files", "*.stp",
                               "STEP files", "*.STEP", "STEP files", "*.STP",
                               "All files", "*", nullptr, nullptr};
  TGFileInfo Info;
  Info.fFileTypes = Types;
  Info.fIniDir = StrDup(fInputFilePath.IsNull() ? "." : gSystem->DirName(fInputFilePath));
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  if (Info.fFilename != nullptr) SetInputFile(Info.fFilename);
}

void MGUICADConverter::OnSelectDest()
{
  if (IsConverting()) return;
  TGFileInfo Info;
  Info.fIniDir = StrDup(fOutputDir.IsNull() ? "." : fOutputDir.Data());
  new TGFileDialog(gClient->GetRoot(), this, kDOpen, &Info);
  // fIniDir is also set on Cancel; only fFilename indicates acceptance.
  if (Info.fFilename != nullptr) SetOutputDirectory(Info.fFilename);
}

TString MGUICADConverter::FindConverter() const
{
  const char* Override = gSystem->Getenv("MEGALIB_CAD_CONVERTER");
  if (Override != nullptr && Override[0] != '\0') {
    return IsFile(Override, X_OK) ? AbsolutePath(Override) : TString("");
  }
  TString Base = g_MEGAlibPath.Data();
  gSystem->ExpandPathName(Base);
  for (const char* Relative : {"/bin/cad_converter",
       "/src/geomega/cad-conversion-dev/cad_converter/install/linux-release/bin/cad_converter",
       "/src/geomega/cad-conversion-dev/cad_converter/build/linux-release/cad_converter"}) {
    TString Candidate = Base + Relative;
    if (IsFile(Candidate, X_OK)) return Candidate;
  }
  char* Found = gSystem->Which(gSystem->Getenv("PATH"), "cad_converter", kExecutePermission);
  TString Result = Found == nullptr ? "" : Found;
  delete [] Found;
  return Result;
}

void MGUICADConverter::OnConvert()
{
  if (IsConverting()) return;
  fOutputFilePath = "";
  fBtnViewGeomega->SetEnabled(kFALSE);
  if (!IsFile(fInputFilePath, R_OK)) {
    SetStatus("Select a readable STEP file first.");
    return;
  }
  const double Angle = fNumAngDef->GetNumber();
  const double Deviation = fNumMaxDev->GetNumber();
  const double Volume = fNumMinVol->GetNumber();
  if (!std::isfinite(Angle) || Angle <= 0 || Angle > 180 ||
      !std::isfinite(Deviation) || Deviation < 0 || !std::isfinite(Volume) || Volume < 0) {
    SetStatus("Invalid settings: angle must be > 0 and <= 180; deviation and volume >= 0.");
    return;
  }
  TString Converter = FindConverter();
  if (Converter.IsNull()) {
    SetStatus("cad_converter not found. Run make cad-converter or set MEGALIB_CAD_CONVERTER.");
    return;
  }
  gSystem->mkdir(fOutputDir, kTRUE);
  struct stat Info;
  if (stat(fOutputDir, &Info) != 0 || !S_ISDIR(Info.st_mode) || access(fOutputDir, W_OK | X_OK) != 0) {
    SetStatus("The output directory cannot be created or is not writable.");
    return;
  }

  bool Geo = fCheckGeo->IsOn();
  const bool Obj = fCheckObj->IsOn();
  const bool Stl = fCheckStl->IsOn();
  if (!Geo && !Obj && !Stl) { Geo = true; fCheckGeo->SetOn(kTRUE); }
  TString Stem = gSystem->BaseName(fInputFilePath);
  if (Stem.Last('.') > 0) Stem.Remove(Stem.Last('.')); // also handles .STEP/.STP
  TString Prefix = fOutputDir + "/" + Stem;
  fExpectedOutputs.clear();
  std::vector<std::string> Args = {Converter.Data(), "--input", fInputFilePath.Data(),
    "--output-dir", fOutputDir.Data(), "--angular-deflection", Number(Angle),
    "--min-volume", Number(Volume)};
  if (Deviation > 0) { Args.push_back("--max-deviation"); Args.push_back(Number(Deviation)); }
  if (Geo) {
    Args.push_back("--geo");
    fOutputFilePath = Prefix + ".geo.setup";
    fExpectedOutputs.emplace_back(fOutputFilePath.Data());
  }
  if (Obj) { Args.push_back("--obj"); fExpectedOutputs.emplace_back((Prefix + ".obj").Data()); }
  if (Stl) { Args.push_back("--stl"); fExpectedOutputs.emplace_back((Prefix + ".stl").Data()); }

  // A separate log for each run preserves diagnostics even after closing the UI.
  std::string Template = std::string(Prefix.Data()) + ".conversion-XXXXXX";
  std::vector<char> LogName(Template.begin(), Template.end());
  LogName.push_back('\0');
  int LogFd = mkstemp(LogName.data());
  if (LogFd < 0) {
    fOutputFilePath = "";
    SetStatus("Cannot create conversion log in the output directory.");
    return;
  }
  fLogFilePath = LogName.data();
  fLogOffset = 0;
  fPendingLine.clear();
  fHadWarnings = false;
  fCancelTicks = 0;
  fLogView->Clear();
  fLogView->AddLine(("Converter: " + Converter).Data());
  fLogView->AddLine(("Log: " + fLogFilePath).Data());

  // Pass literal argv elements, never a shell command (spaces/quotes are safe).
  std::vector<char*> Argv;
  for (auto& Arg : Args) Argv.push_back(&Arg[0]);
  Argv.push_back(nullptr);
  posix_spawn_file_actions_t Actions;
  int Error = posix_spawn_file_actions_init(&Actions);
  const bool ActionsInitialized = Error == 0;
  if (Error == 0) Error = posix_spawn_file_actions_adddup2(&Actions, LogFd, STDOUT_FILENO);
  if (Error == 0) Error = posix_spawn_file_actions_adddup2(&Actions, LogFd, STDERR_FILENO);
  if (Error == 0) Error = posix_spawn_file_actions_addclose(&Actions, LogFd);
  pid_t Child = -1;
  if (Error == 0) Error = posix_spawn(&Child, Converter.Data(), &Actions, nullptr, Argv.data(), environ);
  if (ActionsInitialized) posix_spawn_file_actions_destroy(&Actions);
  close(LogFd);
  if (Error != 0) {
    fOutputFilePath = "";
    SetStatus(TString("Cannot start converter: ") + std::strerror(Error));
    return;
  }
  fChildPid = Child;
  SetBusy(true);
  SetStatus("Converting STEP... (large assemblies can take several minutes)");
  fTimer->Start(250, kFALSE);
}

void MGUICADConverter::ReadLog(bool Finished)
{
  std::ifstream Log(fLogFilePath.Data(), std::ios::binary);
  if (!Log) return;
  Log.seekg(fLogOffset);
  char Buffer[8192];
  do {
    Log.read(Buffer, sizeof(Buffer));
    const auto Count = Log.gcount();
    fLogOffset += Count;
    fPendingLine.append(Buffer, static_cast<size_t>(Count));
    size_t End;
    while ((End = fPendingLine.find('\n')) != std::string::npos) {
      std::string Line = fPendingLine.substr(0, End);
      if (Line.find("WARNING") != std::string::npos || Line.find("ERROR") != std::string::npos) fHadWarnings = true;
      fLogView->AddLine(Line.c_str());
      fPendingLine.erase(0, End + 1);
    }
  } while (Finished && Log);
  if (Finished && !fPendingLine.empty()) {
    if (fPendingLine.find("WARNING") != std::string::npos ||
        fPendingLine.find("ERROR") != std::string::npos) fHadWarnings = true;
    fLogView->AddLine(fPendingLine.c_str());
    fPendingLine.clear();
  }
  fLogView->ShowBottom();
}

Bool_t MGUICADConverter::HandleTimer(TTimer*)
{
  if (!IsConverting()) return kFALSE;
  ReadLog();
  int Status = 0;
  const pid_t Result = waitpid(fChildPid, &Status, WNOHANG);
  if (Result == 0 || (Result < 0 && errno == EINTR)) {
    // Give SIGTERM two seconds, then enforce cancellation.
    if (fCancelTicks > 0 && ++fCancelTicks >= 9) kill(fChildPid, SIGKILL);
    return kTRUE;
  }
  fTimer->TurnOff();
  fChildPid = -1;
  ReadLog(true);
  SetBusy(false);
  bool Success = Result > 0 && WIFEXITED(Status) && WEXITSTATUS(Status) == 0 && fCancelTicks == 0;
  if (Success) {
    for (const auto& Path : fExpectedOutputs) {
      struct stat Info;
      if (!IsFile(Path.c_str(), R_OK) || stat(Path.c_str(), &Info) != 0 || Info.st_size == 0) Success = false;
    }
  }
  if (Success) {
    fBtnViewGeomega->SetEnabled(!fOutputFilePath.IsNull());
    SetStatus(fHadWarnings ? "Conversion finished with warnings. Review the log before using the geometry."
                          : "Conversion complete. Output files are ready.");
  } else {
    fOutputFilePath = "";
    SetStatus(fCancelTicks > 0 ? "Conversion cancelled. Partial outputs may remain; see the log."
                              : "Conversion failed or output is missing. See the log below.");
    if (Result > 0 && WIFEXITED(Status)) fLogView->AddLine(Form("Converter exit code: %d", WEXITSTATUS(Status)));
    if (Result > 0 && WIFSIGNALED(Status)) fLogView->AddLine(Form("Converter terminated by signal: %d", WTERMSIG(Status)));
  }
  if (fCloseWhenFinished) {
    fCloseWhenFinished = false;
    UnmapWindow();
  }
  return kTRUE;
}

void MGUICADConverter::OnCancel()
{
  if (!IsConverting() || fCancelTicks > 0) return;
  fCancelTicks = 1;
  kill(fChildPid, SIGTERM);
  fBtnCancel->SetEnabled(kFALSE);
  SetStatus("Cancelling conversion...");
}

void MGUICADConverter::CloseWindow()
{
  if (IsConverting()) {
    fCloseWhenFinished = true;
    OnCancel();
  } else {
    // The main window owns this dialog and reuses its options and log.
    UnmapWindow();
  }
}

void MGUICADConverter::OnViewGeomega()
{
  if (IsConverting() || !fBtnViewGeomega->IsEnabled() || fOutputFilePath.IsNull()) return;
  if (!IsFile(fOutputFilePath, R_OK) || fMain == nullptr ||
      !fMain->ViewGeometryFile(fOutputFilePath.Data())) {
    SetStatus("Cannot load the converted geometry. See Geomega's diagnostic output.");
    return;
  }
  SetStatus("Converted geometry loaded in Geomega.");
}

Bool_t MGUICADConverter::ProcessMessage(Long_t Message, Long_t Parameter1, Long_t)
{
  if (GET_MSG(Message) == kC_COMMAND && GET_SUBMSG(Message) == kCM_BUTTON) {
    switch (Parameter1) {
    case c_SelectFile: OnSelectFile(); break;
    case c_SelectDest: OnSelectDest(); break;
    case c_Convert: OnConvert(); break;
    case c_ViewGeomega: OnViewGeomega(); break;
    case c_Cancel: OnCancel(); break;
    }
  }
  return kTRUE;
}
