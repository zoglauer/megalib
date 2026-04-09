/*
 * UTStreamBuffer.cxx
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
#include <ostream>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MFile.h"
#include "MStreamBuffer.h"
#include "MUnitTest.h"


//! Unit test class for the low-level MStreamBuffer
class UTStreamBuffer : public MUnitTest
{
public:
  //! Default constructor
  UTStreamBuffer() : MUnitTest("UTStreamBuffer") {}
  //! Default destructor
  virtual ~UTStreamBuffer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a dedicated temporary file name
  MString CreateFileName(const MString& Suffix) const;
  //! Read a complete file into a string
  MString ReadFile(const MString& FileName) const;
  //! Remove a temporary file
  void CleanFile(const MString& FileName) const;

  //! Test the direct MStreamBuffer API
  bool TestStreamBuffer();
};


////////////////////////////////////////////////////////////////////////////////


//! Return a dedicated temporary file name
MString UTStreamBuffer::CreateFileName(const MString& Suffix) const
{
  ostringstream FileName;
  FileName<<"/tmp/UTStreamBuffer_"<<Suffix<<"_"<<getpid()<<".txt";
  return FileName.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Read a complete file into a string
MString UTStreamBuffer::ReadFile(const MString& FileName) const
{
  ifstream In(FileName);
  ostringstream Content;
  Content<<In.rdbuf();
  return Content.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Remove a temporary file
void UTStreamBuffer::CleanFile(const MString& FileName) const
{
  if (MFile::Exists(FileName) == true) {
    MFile::Remove(FileName);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTStreamBuffer::Run()
{
  bool Passed = true;

  Passed = TestStreamBuffer() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the direct MStreamBuffer API
bool UTStreamBuffer::TestStreamBuffer()
{
  bool Passed = true;

  MString BasicFileName = CreateFileName("Basic");
  MString RejectionFileName = CreateFileName("Rejection");
  MString DisabledFileName = CreateFileName("Disabled");
  MString ShowOnceFileName = CreateFileName("ShowOnce");
  MString TimePrefixFileName = CreateFileName("TimePrefix");

  CleanFile(BasicFileName);
  CleanFile(RejectionFileName);
  CleanFile(DisabledFileName);
  CleanFile(ShowOnceFileName);
  CleanFile(TimePrefixFileName);

  {
    MStreamBuffer Buffer;
    Buffer.DumpToStdOut(false);
    Buffer.DumpToStdErr(false);
    Buffer.DumpToGui(false);
    ostream Stream(&Buffer);

    Passed = EvaluateTrue("Connect()", "new file", "Connecting a new file succeeds", Buffer.Connect(BasicFileName, false, false)) && Passed;
    Passed = EvaluateFalse("Connect()", "duplicate file", "Connecting the same file twice fails", Buffer.Connect(BasicFileName, false, false)) && Passed;
    Passed = EvaluateFalse("Disconnect()", "unknown file", "Disconnecting an unknown file fails", Buffer.Disconnect(CreateFileName("DoesNotExist"))) && Passed;

    Stream<<"Alpha";
    Buffer.show();
    Passed = Evaluate("show()", "plain output", "show() flushes the buffered line to the file", ReadFile(BasicFileName), MString("Alpha\n")) && Passed;

    Buffer.SetHeader("Header");
    Buffer.SetPrefix("  ");
    Stream<<"Line1\nLine2";
    Buffer.show();
    Passed = Evaluate("SetHeader()/SetPrefix()", "multi-line output", "Header and prefix are added to all shown lines", ReadFile(BasicFileName), MString("Alpha\nHeader\n  Line1\n  Line2\n")) && Passed;

    Stream<<"Line3";
    Buffer.show();
    Passed = Evaluate("show()", "header reset", "The header is shown again for each newly shown message block", ReadFile(BasicFileName), MString("Alpha\nHeader\n  Line1\n  Line2\nHeader\n  Line3\n")) && Passed;

    Passed = EvaluateTrue("Disconnect()", "known file", "Disconnecting a connected file succeeds", Buffer.Disconnect(BasicFileName)) && Passed;

    Stream<<"Ignored";
    Buffer.show();
    Passed = Evaluate("Disconnect()", "no file output", "After disconnecting no further output reaches the file", ReadFile(BasicFileName), MString("Alpha\nHeader\n  Line1\n  Line2\nHeader\n  Line3\n")) && Passed;
  }

  {
    MStreamBuffer Buffer;
    Buffer.DumpToStdOut(false);
    Buffer.DumpToStdErr(false);
    Buffer.DumpToGui(false);
    ostream Stream(&Buffer);

    Passed = EvaluateTrue("Connect()", "rejection file", "Connecting the rejection file succeeds", Buffer.Connect(RejectionFileName, false, false)) && Passed;

    Buffer.SetRejection("RejectMe");
    Stream<<"First";
    Buffer.show();
    Buffer.SetRejection("RejectMe");
    Stream<<"Second";
    Buffer.show();
    Passed = Evaluate("SetRejection()", "duplicate rejection", "Using the same rejection key twice suppresses the second message", ReadFile(RejectionFileName), MString("First\n")) && Passed;
  }

  {
    MStreamBuffer Buffer;
    Buffer.DumpToStdOut(false);
    Buffer.DumpToStdErr(false);
    Buffer.DumpToGui(false);
    ostream Stream(&Buffer);

    Buffer.Enable(false);
    Passed = EvaluateFalse("Connect()", "disabled stream", "A disabled stream cannot connect to a file", Buffer.Connect(DisabledFileName, false, false)) && Passed;

    Stream<<"Muted";
    Buffer.show();
    Passed = EvaluateFalse("Enable(false)", "muted file", "A disabled stream does not create the target file implicitly", MFile::Exists(DisabledFileName)) && Passed;
  }

  {
    MStreamBuffer Buffer;
    Buffer.DumpToStdOut(false);
    Buffer.DumpToStdErr(false);
    Buffer.DumpToGui(false);
    ostream Stream(&Buffer);

    Passed = EvaluateTrue("Connect()", "show once file", "Connecting the show-once file succeeds", Buffer.Connect(ShowOnceFileName, false, false)) && Passed;
    Buffer.ShowOnce();

    Stream<<"First";
    Buffer.show();
    Stream<<"Second";
    Buffer.show();

    Passed = Evaluate("ShowOnce()", "single emission", "ShowOnce disables the stream after the first shown message", ReadFile(ShowOnceFileName), MString("First\n")) && Passed;
  }

  {
    MStreamBuffer Buffer;
    Buffer.DumpToStdOut(false);
    Buffer.DumpToStdErr(false);
    Buffer.DumpToGui(false);
    ostream Stream(&Buffer);

    Passed = EvaluateTrue("Connect()", "time prefix file", "Connecting with time prefix succeeds", Buffer.Connect(TimePrefixFileName, false, true)) && Passed;
    Stream<<"Timed";
    Buffer.show();

    MString Content = ReadFile(TimePrefixFileName);
    Passed = EvaluateTrue("Connect(..., TimePrefix=true)", "contains payload", "Timed file output still contains the emitted message", Content.Contains("Timed\n")) && Passed;
    Passed = EvaluateTrue("Connect(..., TimePrefix=true)", "contains separator", "Timed file output prefixes the line with a timestamp separator", Content.Contains(":  ")) && Passed;
  }

  CleanFile(BasicFileName);
  CleanFile(RejectionFileName);
  CleanFile(DisabledFileName);
  CleanFile(ShowOnceFileName);
  CleanFile(TimePrefixFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTStreamBuffer Test;
  return Test.Run() == true ? 0 : 1;
}
