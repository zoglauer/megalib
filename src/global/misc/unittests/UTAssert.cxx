/*
 * UTAssert.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cerrno>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

// MEGAlib:
#include "MAssert.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MAssert
class UTAssert : public MUnitTest
{
public:
  //! Default constructor
  UTAssert() : MUnitTest("UTAssert") {}
  //! Default destructor
  virtual ~UTAssert() {}

  //! Run all tests
  virtual bool Run();

  //! Helper entry point for the null-argument assertion check
  static int NullAssertionCheck()
  {
    MAssert::AssertionFailed(nullptr, nullptr, 42, nullptr);
    return 1;
  }

  //! Helper entry point for the normal formatting assertion check
  static int NormalAssertionCheck()
  {
    MAssert::AssertionFailed("Alpha == Beta", "UTAssert.cxx", 4242, "NormalAssertionCheck");
    return 1;
  }

  //! Helper entry point for the null-function assertion check
  static int NullFunctionAssertionCheck()
  {
    MAssert::AssertionFailed("Beta != Gamma", "UTAssert.cxx", 4243, nullptr);
    return 1;
  }

  //! Helper entry point for the macro success check
  static int MacroSuccessCheck()
  {
    massert(true);
    return 0;
  }

  //! Helper entry point for the macro assertion check
  static int MacroAssertionCheck()
  {
    massert(1 == 2);
    return 1;
  }

  //! Helper entry point for the compound macro assertion check
  static int CompoundMacroAssertionCheck()
  {
    const int x = 0;
    const int y = 1;
    const int z = 2;
    massert(x > 0 && y != z);
    (void)x;
    (void)y;
    (void)z;
    return 1;
  }

private:
  typedef int (*TCheckFunction)();

  //! Run a helper function in a child process and capture its output
  static int RunChild(TCheckFunction Function, const MString& LogFileName)
  {
    pid_t Child = fork();
    if (Child < 0) {
      return -1;
    }

    if (Child == 0) {
      int Log = open(LogFileName.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log < 0) {
        _exit(127);
      }
      (void)dup2(Log, STDOUT_FILENO);
      (void)dup2(Log, STDERR_FILENO);
      close(Log);
      int ReturnValue = Function();
      _exit(ReturnValue);
    }

    int Status = 0;
    while (waitpid(Child, &Status, 0) < 0) {
      if (errno != EINTR) {
        return -1;
      }
    }

    if (WIFEXITED(Status) != 0) {
      return WEXITSTATUS(Status);
    }
    if (WIFSIGNALED(Status) != 0) {
      return 128 + WTERMSIG(Status);
    }
    return -1;
  }

};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTAssert::Run()
{
  bool Passed = true;
  const MString NullLog = GetTemporaryFileName("null.log");
  const MString NormalLog = GetTemporaryFileName("normal.log");
  const MString MacroLog = GetTemporaryFileName("macro.log");
  const MString CompoundMacroLog = GetTemporaryFileName("macro_compound.log");

  MFile::Remove(NullLog);
  MFile::Remove(NormalLog);
  MFile::Remove(MacroLog);
  MFile::Remove(CompoundMacroLog);

  int Status = RunChild(NullAssertionCheck, NullLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "null status", "The null-safe assertion helper aborts the child process", Status != 0) && Passed;
  MString NullOutput = ReadTextFile(NullLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "null message", "The null-safe assertion helper prints placeholder text for null inputs", NullOutput.Contains("[unknown file name]") && NullOutput.Contains("[unknown condition]") && NullOutput.Contains("in function") == false) && Passed;

  Status = RunChild(NormalAssertionCheck, NormalLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "normal status", "The normal assertion helper aborts the child process", Status != 0) && Passed;
  MString NormalOutput = ReadTextFile(NormalLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "normal message", "The normal assertion helper prints the provided assertion text and line number", NormalOutput.Contains("Alpha == Beta") && NormalOutput.Contains("UTAssert.cxx") && NormalOutput.Contains("NormalAssertionCheck") && NormalOutput.Contains("4242")) && Passed;

  const MString NullFunctionLog = GetTemporaryFileName("null_function.log");
  MFile::Remove(NullFunctionLog);
  Status = RunChild(NullFunctionAssertionCheck, NullFunctionLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "null function status", "The assertion helper aborts when the function name is unavailable", Status != 0) && Passed;
  MString NullFunctionOutput = ReadTextFile(NullFunctionLog);
  Passed = EvaluateTrue("MAssert::AssertionFailed()", "null function message", "The assertion helper omits the function text when it is unavailable", NullFunctionOutput.Contains("Beta != Gamma") && NullFunctionOutput.Contains("UTAssert.cxx") && NullFunctionOutput.Contains("4243") && NullFunctionOutput.Contains("in function") == false) && Passed;

  const MString MacroSuccessLog = GetTemporaryFileName("macro_success.log");
  MFile::Remove(MacroSuccessLog);
  Status = RunChild(MacroSuccessCheck, MacroSuccessLog);
  Passed = EvaluateTrue("massert(true)", "macro success status", "A true assertion expression must not abort", Status == 0) && Passed;
  MString MacroSuccessOutput = ReadTextFile(MacroSuccessLog);
  Passed = EvaluateTrue("massert(true)", "macro success message", "A successful assertion should not emit diagnostics", MacroSuccessOutput.IsEmpty()) && Passed;

  Status = RunChild(MacroAssertionCheck, MacroLog);
#ifdef NDEBUG
  Passed = EvaluateTrue("massert()", "macro status", "A false assertion expression is compiled out when NDEBUG is defined", Status == 1) && Passed;
  MString MacroOutput = ReadTextFile(MacroLog);
  Passed = EvaluateTrue("massert()", "macro message", "A compiled-out assertion should not emit diagnostics", MacroOutput.IsEmpty()) && Passed;
#else
  Passed = EvaluateTrue("massert()", "macro status", "The macro forwards failures to the assertion helper", Status != 0) && Passed;
  MString MacroOutput = ReadTextFile(MacroLog);
  Passed = EvaluateTrue("massert()", "macro message", "The macro stringizes a non-trivial failed expression", MacroOutput.Contains("1 == 2")) && Passed;
  Passed = EvaluateTrue("massert()", "macro function name", "The macro reports the enclosing function name", MacroOutput.Contains("MacroAssertionCheck")) && Passed;
#endif

  Status = RunChild(CompoundMacroAssertionCheck, CompoundMacroLog);
#ifdef NDEBUG
  Passed = EvaluateTrue("massert()", "compound macro status", "A false compound assertion is compiled out when NDEBUG is defined", Status == 1) && Passed;
  MString CompoundMacroOutput = ReadTextFile(CompoundMacroLog);
  Passed = EvaluateTrue("massert()", "compound macro message", "A compiled-out compound assertion should not emit diagnostics", CompoundMacroOutput.IsEmpty()) && Passed;
#else
  Passed = EvaluateTrue("massert()", "compound macro status", "A failing compound assertion aborts the child process", Status != 0) && Passed;
  MString CompoundMacroOutput = ReadTextFile(CompoundMacroLog);
  Passed = EvaluateTrue("massert()", "compound macro message", "The macro stringizes a compound expression without expansion artifacts", CompoundMacroOutput.Contains("x > 0 && y != z")) && Passed;
  Passed = EvaluateTrue("massert()", "compound macro function name", "The compound macro failure reports the enclosing function name", CompoundMacroOutput.Contains("CompoundMacroAssertionCheck")) && Passed;
#endif

  MFile::Remove(NullLog);
  MFile::Remove(NormalLog);
  MFile::Remove(NullFunctionLog);
  MFile::Remove(MacroSuccessLog);
  MFile::Remove(MacroLog);
  MFile::Remove(CompoundMacroLog);

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int, char**)
{
  UTAssert Test;
  return Test.Run() == true ? 0 : 1;
}
