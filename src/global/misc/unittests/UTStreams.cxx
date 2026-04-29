/*
 * UTStreams.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdio>
#include <fstream>
#include <list>
#include <ostream>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MFile.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for the MStreams wrapper and global streams
class UTStreams : public MUnitTest
{
public:
  //! Default constructor
  UTStreams() : MUnitTest("UTStreams") {}
  //! Default destructor
  virtual ~UTStreams() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a dedicated temporary file name
  MString CreateFileName(const MString& Suffix) const;
  //! Read a complete file into a string
  MString ReadFile(const MString& FileName) const;
  //! Remove a temporary file
  void CleanFile(const MString& FileName) const;
  //! Count non-overlapping occurrences of a pattern
  unsigned int CountOccurrences(const MString& Text, const MString& Pattern) const;

  //! Emit a deprecated warning from a stable source location
  static void EmitDeprecatedDuplicate();
  //! Emit a second deprecated warning from a different source location
  static void EmitDeprecatedUnique();
  //! Emit an implementation-limit warning from a stable source location
  static void EmitImplementationDuplicate();
  //! Emit a second implementation-limit warning from a different source location
  static void EmitImplementationUnique();
  //! Emit a merr message using the production macro
  static void EmitMerrShow();
  //! Emit mdebug messages at all verbosity levels
  static void EmitDebugSeries();

  //! Test the direct MStreams wrapper API
  bool TestStreams();
  //! Test global streams and macros as they are used in MEGAlib
  bool TestGlobalStreamsAndMacros();
};


////////////////////////////////////////////////////////////////////////////////


//! Return a dedicated temporary file name
MString UTStreams::CreateFileName(const MString& Suffix) const
{
  ostringstream FileName;
  FileName<<"/tmp/UTStreams_"<<Suffix<<"_"<<getpid()<<".txt";
  return FileName.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Emit a deprecated warning from a stable source location
void UTStreams::EmitDeprecatedDuplicate()
{
  mdep<<"Deprecated duplicate"<<show;
}


////////////////////////////////////////////////////////////////////////////////


//! Emit a second deprecated warning from a different source location
void UTStreams::EmitDeprecatedUnique()
{
  mdep<<"Deprecated unique"<<show;
}


////////////////////////////////////////////////////////////////////////////////


//! Emit an implementation-limit warning from a stable source location
void UTStreams::EmitImplementationDuplicate()
{
  mimp<<"Implementation duplicate"<<show;
}


////////////////////////////////////////////////////////////////////////////////


//! Emit a second implementation-limit warning from a different source location
void UTStreams::EmitImplementationUnique()
{
  mimp<<"Implementation unique"<<show;
}


////////////////////////////////////////////////////////////////////////////////


//! Emit a merr message using the production macro
void UTStreams::EmitMerrShow()
{
  merr<<"Macro problem"<<show;
}


////////////////////////////////////////////////////////////////////////////////


//! Emit mdebug messages at all verbosity levels
void UTStreams::EmitDebugSeries()
{
  mdebug1<<"Debug1"<<endl;
  mdebug2<<"Debug2"<<endl;
  mdebug3<<"Debug3"<<endl;
  mdebug4<<"Debug4"<<endl;
  mdebug5<<"Debug5"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a complete file into a string
MString UTStreams::ReadFile(const MString& FileName) const
{
  ifstream In(FileName);
  ostringstream Content;
  Content<<In.rdbuf();
  return Content.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Remove a temporary file
void UTStreams::CleanFile(const MString& FileName) const
{
  if (MFile::Exists(FileName) == true) {
    MFile::Remove(FileName);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Count non-overlapping occurrences of a pattern
unsigned int UTStreams::CountOccurrences(const MString& Text, const MString& Pattern) const
{
  if (Pattern.IsEmpty() == true) return 0;

  unsigned int Occurrences = 0;
  size_t Position = 0;
  while ((Position = Text.GetString().find(Pattern.GetString(), Position)) != string::npos) {
    ++Occurrences;
    Position += Pattern.Length();
  }

  return Occurrences;
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTStreams::Run()
{
  bool Passed = true;

  Passed = TestStreams() && Passed;
  Passed = TestGlobalStreamsAndMacros() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the direct MStreams wrapper API
bool UTStreams::TestStreams()
{
  bool Passed = true;

  MString WrapperFileName = CreateFileName("Wrapper");
  MString LogOnlyFileName = CreateFileName("LogOnly");

  CleanFile(WrapperFileName);
  CleanFile(LogOnlyFileName);

  {
    MStreams Stream;
    Stream.DumpToStdOut(false);
    Stream.DumpToStdErr(false);
    Stream.DumpToGui(false);

    Passed = EvaluateTrue("MStreams::Connect()", "wrapper file", "The wrapper can connect to a file", Stream.Connect(WrapperFileName, false, false)) && Passed;

    Stream.SetHeader("WrapperHeader");
    Stream.SetPrefix(">> ");
    Stream<<"First"<<"\n"<<"Second"<<show;
    Stream<<"Third"<<show;
    Stream.printf("Value=%d\n", 17);
    Stream<<warn<<"Warned";
    Stream<<"Fourth"<<show;
    Stream.Disconnect(WrapperFileName);

    Passed = Evaluate("MStreams", "wrapper output", "The wrapper forwards header, prefix, manipulators and printf output", ReadFile(WrapperFileName), MString("WrapperHeader\n>> First\n>> Second\nWrapperHeader\n>> Third\nWrapperHeader\n>> Value=17\n>> \nWrapperHeader\n>> WarnedFourth\n")) && Passed;
  }

  {
    MStreams Stream;
    Stream.DumpToStdOut(false);
    Stream.DumpToStdErr(false);
    Stream.DumpToGui(false);
    Passed = EvaluateTrue("MStreams::Connect()", "logonly file", "The wrapper can connect to a second file", Stream.Connect(LogOnlyFileName, false, false)) && Passed;

    Stream<<"OnlyToFile"<<logonly;
    Stream<<"Shown"<<show;
    Stream.Disconnect(LogOnlyFileName);

    Passed = Evaluate("logonly()", "file logging", "logonly flushes to the file without breaking subsequent show calls", ReadFile(LogOnlyFileName), MString("OnlyToFile\nShown\n")) && Passed;
  }

  CleanFile(WrapperFileName);
  CleanFile(LogOnlyFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test global streams and macros as they are used in MEGAlib
bool UTStreams::TestGlobalStreamsAndMacros()
{
  bool Passed = true;

  MString MoutFileName = CreateFileName("GlobalMout");
  MString MlogFileName = CreateFileName("GlobalMlog");
  MString MguiFileName = CreateFileName("GlobalMgui");
  MString MerrFileName = CreateFileName("GlobalMerr");
  MString MdepFileName = CreateFileName("GlobalMdep");
  MString MimpFileName = CreateFileName("GlobalMimp");
  MString AppendFileName = CreateFileName("Append");
  MString MultiAFileName = CreateFileName("MultiA");
  MString MultiBFileName = CreateFileName("MultiB");

  list<MString> FileNames;
  FileNames.push_back(MoutFileName);
  FileNames.push_back(MlogFileName);
  FileNames.push_back(MguiFileName);
  FileNames.push_back(MerrFileName);
  FileNames.push_back(MdepFileName);
  FileNames.push_back(MimpFileName);
  FileNames.push_back(AppendFileName);
  FileNames.push_back(MultiAFileName);
  FileNames.push_back(MultiBFileName);
  for (list<MString>::iterator I = FileNames.begin(); I != FileNames.end(); ++I) {
    CleanFile(*I);
  }

  int OriginalVerbosity = g_Verbosity;

  mout.DumpToStdOut(false);
  mout.DumpToStdErr(false);
  mout.DumpToGui(false);
  mlog.DumpToStdOut(false);
  mlog.DumpToStdErr(false);
  mlog.DumpToGui(false);
  mgui.DumpToStdOut(false);
  mgui.DumpToStdErr(false);
  mgui.DumpToGui(false);
  __merr.DumpToStdOut(false);
  __merr.DumpToStdErr(false);
  __merr.DumpToGui(false);
  __mdep.DumpToStdOut(false);
  __mdep.DumpToStdErr(false);
  __mdep.DumpToGui(false);
  __mimp.DumpToStdOut(false);
  __mimp.DumpToStdErr(false);
  __mimp.DumpToGui(false);

  mout.Enable(true);
  mlog.Enable(true);
  mgui.Enable(true);
  __merr.Enable(true);
  __mdep.Enable(true);
  __mimp.Enable(true);

  {
    Passed = EvaluateTrue("mout.Connect()", "global mout", "The global mout stream can connect to a file", mout.Connect(MoutFileName, false, false)) && Passed;
    mout<<"Hello"<<endl;
    mout<<"World"<<show;
    mout.Disconnect(MoutFileName);
    Passed = Evaluate("mout", "endl and show", "mout supports the dominant endl/show emission paths used in MEGAlib", ReadFile(MoutFileName), MString("Hello\nWorld\n")) && Passed;
  }

  {
    Passed = EvaluateTrue("mgui.Connect()", "global mgui", "The global mgui stream can connect to a file", mgui.Connect(MguiFileName, false, false)) && Passed;
    mgui<<"GuiInfo"<<info;
    mgui<<"GuiError"<<error;
    mgui.Disconnect(MguiFileName);
    Passed = Evaluate("mgui", "info and error", "mgui forwards info and error manipulators to the underlying stream", ReadFile(MguiFileName), MString("GuiInfo\nGuiError\n")) && Passed;
  }

  {
    Passed = EvaluateTrue("__merr.Connect()", "global merr", "The internal error stream can connect to a file", __merr.Connect(MerrFileName, false, false)) && Passed;
    EmitMerrShow();
    __merr.Disconnect(MerrFileName);

    MString Content = ReadFile(MerrFileName);
    Passed = EvaluateTrue("merr", "message text", "The merr macro writes the emitted message", Content.Contains("Macro problem")) && Passed;
#ifdef NDEBUG
    Passed = EvaluateFalse("merr", "debug file context", "In release builds merr does not prepend debug source context", Content.Contains("UTStreams.cxx")) && Passed;
    Passed = EvaluateFalse("merr", "debug function context", "In release builds merr does not prepend debug function context", Content.Contains("UTStreams_EmitMerrShow")) && Passed;
#else
    Passed = EvaluateTrue("merr", "file context", "The merr macro writes source file context", Content.Contains("UTStreams.cxx")) && Passed;
    Passed = EvaluateTrue("merr", "function context", "The merr macro writes function context", Content.Contains("UTStreams_EmitMerrShow")) && Passed;
#endif
  }

  {
    g_Verbosity = 1;
    Passed = EvaluateTrue("__mdep.Connect()", "deprecated file", "The deprecated stream can connect to a file", __mdep.Connect(MdepFileName, false, false)) && Passed;
    EmitDeprecatedDuplicate();
    EmitDeprecatedDuplicate();
    EmitDeprecatedUnique();
    __mdep.Disconnect(MdepFileName);

    MString Content = ReadFile(MdepFileName);
    Passed = Evaluate("mdep", "duplicate suppression", "The deprecation macro suppresses repeated messages from the same source location", CountOccurrences(Content, "Deprecated duplicate"), 1U) && Passed;
    Passed = Evaluate("mdep", "unique source", "A deprecation from a different source location is still emitted", CountOccurrences(Content, "Deprecated unique"), 1U) && Passed;
    Passed = EvaluateTrue("mdep", "header formatting", "The deprecation macro prefixes the output with a deprecation header", Content.Contains("Deprecated use of function")) && Passed;
  }

  {
    g_Verbosity = 1;
    Passed = EvaluateTrue("__mimp.Connect()", "implementation file", "The implementation-limit stream can connect to a file", __mimp.Connect(MimpFileName, false, false)) && Passed;
    EmitImplementationDuplicate();
    EmitImplementationDuplicate();
    EmitImplementationUnique();
    __mimp.Disconnect(MimpFileName);

    MString Content = ReadFile(MimpFileName);
    Passed = Evaluate("mimp", "duplicate suppression", "The implementation-limit macro suppresses repeated messages from the same source location", CountOccurrences(Content, "Implementation duplicate"), 1U) && Passed;
    Passed = Evaluate("mimp", "unique source", "A different implementation-limit source location is still emitted", CountOccurrences(Content, "Implementation unique"), 1U) && Passed;
    Passed = EvaluateTrue("mimp", "header formatting", "The implementation-limit macro prefixes the output with a limitation header", Content.Contains("Implementation limitation")) && Passed;
  }

  {
    Passed = EvaluateTrue("mlog.Connect()", "global mlog", "The global log stream can connect to a file", mlog.Connect(MlogFileName, false, false)) && Passed;
    g_Verbosity = 0;
    EmitDebugSeries();
    g_Verbosity = 5;
    EmitDebugSeries();
    mlog.Disconnect(MlogFileName);

    MString Content = ReadFile(MlogFileName);
    Passed = Evaluate("mdebug", "verbosity gated", "mdebug messages are suppressed when the verbosity is too low and emitted when it is high enough", Content, MString("Debug1\nDebug2\nDebug3\nDebug4\nDebug5\n")) && Passed;
  }

  {
    ofstream Out(AppendFileName);
    Out<<"Existing\n";
    Out.close();

    Passed = EvaluateTrue("MStreams::Connect(..., Append=true)", "append file", "Connecting in append mode succeeds", mout.Connect(AppendFileName, true, false)) && Passed;
    mout<<"Appended"<<endl;
    mout.Disconnect(AppendFileName);
    Passed = Evaluate("Connect(..., Append=true)", "append content", "Append mode preserves the old file content and adds new output", ReadFile(AppendFileName), MString("Existing\nAppended\n")) && Passed;
  }

  {
    Passed = EvaluateTrue("MStreams::Connect()", "multi file A", "The same stream can connect to a first file", mout.Connect(MultiAFileName, false, false)) && Passed;
    Passed = EvaluateTrue("MStreams::Connect()", "multi file B", "The same stream can connect to a second file", mout.Connect(MultiBFileName, false, false)) && Passed;
    mout<<"Both"<<endl;
    Passed = EvaluateTrue("MStreams::Disconnect()", "remove first file", "Disconnecting the first of multiple files succeeds", mout.Disconnect(MultiAFileName)) && Passed;
    mout<<"OnlyB"<<endl;
    mout.Disconnect(MultiBFileName);

    Passed = Evaluate("Disconnect()", "first file removed", "After disconnecting one of multiple files, the disconnected file no longer receives output", ReadFile(MultiAFileName), MString("Both\n")) && Passed;
    Passed = Evaluate("Disconnect()", "remaining file still active", "After disconnecting one file, the remaining connected file still receives output", ReadFile(MultiBFileName), MString("Both\nOnlyB\n")) && Passed;
  }

  g_Verbosity = OriginalVerbosity;

  for (list<MString>::iterator I = FileNames.begin(); I != FileNames.end(); ++I) {
    CleanFile(*I);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTStreams Test;
  return Test.Run() == true ? 0 : 1;
}
