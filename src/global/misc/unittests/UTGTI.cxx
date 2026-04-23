/*
 * UTGTI.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
using namespace std;

// MEGAlib:
#include "MFile.h"
#include "MGTI.h"
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MGTI
class UTGTI : public MUnitTest
{
public:
  //! Default constructor
  UTGTI() : MUnitTest("UTGTI") {}
  //! Default destructor
  virtual ~UTGTI() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Create the temporary directory used by the tests
  bool PrepareTempDirectory();
  //! Write a GTI test file
  bool WriteTextFile(const MString& FileName, const MString& Content);
  //! Test constructor and reset behavior
  bool TestDefaultAndReset();
  //! Test loading representative GTI and BTI intervals
  bool TestLoadAndIsGood();
  //! Test adding a second GTI
  bool TestAdd();
  //! Test nested include loading
  bool TestIncludes();
  //! Test parser details such as EN handling and ignored malformed lines
  bool TestLoadParsingDetails();
  //! Test load failure fallback behavior
  bool TestLoadFailure();
};


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::Run()
{
  bool Passed = true;

  Passed = TestDefaultAndReset() && Passed;
  Passed = TestLoadAndIsGood() && Passed;
  Passed = TestAdd() && Passed;
  Passed = TestIncludes() && Passed;
  Passed = TestLoadParsingDetails() && Passed;
  Passed = TestLoadFailure() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::PrepareTempDirectory()
{
  MString Directory = "/tmp/UTGTI";
  MString Command = "mkdir -p " + Directory;

  return system(Command.Data()) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::WriteTextFile(const MString& FileName, const MString& Content)
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) {
    return false;
  }

  Out<<Content;
  Out.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestDefaultAndReset()
{
  bool Passed = true;

  MGTI GTI;
  Passed = EvaluateTrue("MGTI()", "default lower bound", "The default constructor creates an all-open interval including time zero", GTI.IsGood(MTime(0))) && Passed;
  Passed = EvaluateTrue("MGTI()", "default upper bound", "The default constructor creates an all-open interval including the configured upper edge", GTI.IsGood(MTime(2000000000))) && Passed;
  Passed = EvaluateFalse("MGTI()", "outside default interval", "Times above the default open interval are rejected", GTI.IsGood(MTime(2000000001))) && Passed;

  GTI.Reset(false);
  Passed = EvaluateFalse("Reset(false)", "empty intervals", "Reset(false) clears all intervals and rejects representative times", GTI.IsGood(MTime(100))) && Passed;

  GTI.Reset(true);
  Passed = EvaluateTrue("Reset(true)", "reopen interval", "Reset(true) restores the default open interval", GTI.IsGood(MTime(100))) && Passed;
  Passed = EvaluateFalse("Reset(true)", "outside reopened interval", "Reset(true) still keeps the configured upper bound", GTI.IsGood(MTime(2000000001))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestLoadAndIsGood()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "load temp dir", "The temporary directory for GTI load tests can be created", PrepareTempDirectory()) && Passed;

  MString FileName = "/tmp/UTGTI/basic.gti";
  MString Content =
    "GT 10 20\n"
    "BT 12 14\n"
    "GT 30 40\n"
    "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "basic file", "A representative GTI file can be written", WriteTextFile(FileName, Content)) && Passed;

  MGTI GTI;
  Passed = EvaluateTrue("Load()", "basic file", "Load() accepts a representative GTI file", GTI.Load(FileName)) && Passed;
  Passed = EvaluateFalse("IsGood()", "before first interval", "Times before the first good interval are rejected", GTI.IsGood(MTime(9))) && Passed;
  Passed = EvaluateTrue("IsGood()", "first good interval start", "The first good interval start is inclusive", GTI.IsGood(MTime(10))) && Passed;
  Passed = EvaluateTrue("IsGood()", "inside first good interval", "A representative time inside the first good interval is accepted", GTI.IsGood(MTime(11))) && Passed;
  Passed = EvaluateFalse("IsGood()", "bad interval start", "The bad interval start is excluded even inside a good interval", GTI.IsGood(MTime(12))) && Passed;
  Passed = EvaluateFalse("IsGood()", "bad interval stop", "The bad interval stop is excluded even inside a good interval", GTI.IsGood(MTime(14))) && Passed;
  Passed = EvaluateTrue("IsGood()", "after bad interval", "Times after the bad interval but still inside the good interval are accepted", GTI.IsGood(MTime(15))) && Passed;
  Passed = EvaluateTrue("IsGood()", "first good interval stop", "The first good interval stop is inclusive", GTI.IsGood(MTime(20))) && Passed;
  Passed = EvaluateFalse("IsGood()", "gap between good intervals", "Times in the gap between good intervals are rejected", GTI.IsGood(MTime(25))) && Passed;
  Passed = EvaluateTrue("IsGood()", "second good interval", "A representative time in the second good interval is accepted", GTI.IsGood(MTime(35))) && Passed;
  Passed = EvaluateFalse("IsGood()", "after second interval", "Times after the second good interval are rejected", GTI.IsGood(MTime(41))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestAdd()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "add temp dir", "The temporary directory for GTI add tests can be created", PrepareTempDirectory()) && Passed;

  MString FirstFile = "/tmp/UTGTI/first.gti";
  MString SecondFile = "/tmp/UTGTI/second.gti";

  Passed = EvaluateTrue("WriteTextFile()", "first add file", "The first GTI input file can be written", WriteTextFile(FirstFile, "GT 1 3\nBT 2 2\nEN\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "second add file", "The second GTI input file can be written", WriteTextFile(SecondFile, "GT 8 10\nBT 9 9\nEN\n")) && Passed;

  MGTI First;
  MGTI Second;
  Passed = EvaluateTrue("Load()", "first add file", "The first GTI file loads successfully", First.Load(FirstFile)) && Passed;
  Passed = EvaluateTrue("Load()", "second add file", "The second GTI file loads successfully", Second.Load(SecondFile)) && Passed;

  First.Add(Second);
  Passed = EvaluateTrue("Add()", "first preserved", "Add() preserves the original GTI intervals", First.IsGood(MTime(1))) && Passed;
  Passed = EvaluateFalse("Add()", "first bad interval preserved", "Add() preserves the original bad intervals", First.IsGood(MTime(2))) && Passed;
  Passed = EvaluateTrue("Add()", "second interval added", "Add() appends good intervals from the added GTI", First.IsGood(MTime(8))) && Passed;
  Passed = EvaluateFalse("Add()", "second bad interval added", "Add() appends bad intervals from the added GTI", First.IsGood(MTime(9))) && Passed;
  Passed = EvaluateTrue("Add()", "second interval stop", "Added good interval stops remain inclusive", First.IsGood(MTime(10))) && Passed;
  Passed = EvaluateFalse("Add()", "outside combined intervals", "Times outside both GTIs remain rejected", First.IsGood(MTime(11))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestIncludes()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "include temp dir", "The temporary directory for GTI include tests can be created", PrepareTempDirectory()) && Passed;

  MString IncludedFile = "/tmp/UTGTI/included.gti";
  MString MainFile = "/tmp/UTGTI/main.gti";

  Passed = EvaluateTrue("WriteTextFile()", "included file", "The included GTI file can be written", WriteTextFile(IncludedFile, "GT 70 80\nBT 75 76\nEN\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "main include file", "The main GTI file referencing the include can be written", WriteTextFile(MainFile, "IN included.gti\nGT 50 60\nEN\n")) && Passed;

  MGTI GTI;
  Passed = EvaluateTrue("Load()", "include file", "Load() resolves relative IN directives and loads nested GTIs", GTI.Load(MainFile)) && Passed;
  Passed = EvaluateTrue("IsGood()", "main interval", "The main file interval is loaded", GTI.IsGood(MTime(55))) && Passed;
  Passed = EvaluateTrue("IsGood()", "included interval", "The included file interval is loaded", GTI.IsGood(MTime(74))) && Passed;
  Passed = EvaluateFalse("IsGood()", "included bad interval", "Bad intervals from included files are loaded too", GTI.IsGood(MTime(75))) && Passed;
  Passed = EvaluateFalse("IsGood()", "outside included interval", "Times outside all included and local intervals are rejected", GTI.IsGood(MTime(81))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestLoadParsingDetails()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "parsing temp dir", "The temporary directory for GTI parsing-detail tests can be created", PrepareTempDirectory()) && Passed;

  {
    MString FileName = "/tmp/UTGTI/early_end.gti";
    MString Content =
      "GT 1 2\n"
      "EN\n"
      "GT 100 200\n";
    Passed = EvaluateTrue("WriteTextFile()", "early EN file", "A GTI file with an early EN marker can be written", WriteTextFile(FileName, Content)) && Passed;

    MGTI GTI;
    Passed = EvaluateTrue("Load()", "early EN file", "Load() accepts GTI files with an early EN marker", GTI.Load(FileName)) && Passed;
    Passed = EvaluateTrue("IsGood()", "early EN first interval", "Intervals before EN are loaded", GTI.IsGood(MTime(1))) && Passed;
    Passed = EvaluateFalse("IsGood()", "early EN ignored tail", "Intervals after EN are ignored", GTI.IsGood(MTime(150))) && Passed;
  }

  {
    MString FileName = "/tmp/UTGTI/malformed_lines.gti";
    MString Content =
      "GT 10 11\n"
      "GT 20\n"
      "BT 30 31 32\n"
      "XX 40 41\n"
      "BT 50 51\n"
      "EN\n";
    Passed = EvaluateTrue("WriteTextFile()", "malformed line file", "A GTI file containing malformed and unrelated lines can be written", WriteTextFile(FileName, Content)) && Passed;

    MGTI GTI;
    Passed = EvaluateTrue("Load()", "malformed line file", "Load() ignores malformed and unrelated lines while keeping valid intervals", GTI.Load(FileName)) && Passed;
    Passed = EvaluateTrue("IsGood()", "malformed line valid GT", "Valid GT lines in mixed files are still loaded", GTI.IsGood(MTime(10))) && Passed;
    Passed = EvaluateFalse("IsGood()", "malformed line ignored GT", "Malformed GT lines are ignored", GTI.IsGood(MTime(20))) && Passed;
    Passed = EvaluateFalse("IsGood()", "malformed line valid BT", "Valid BT lines in mixed files are still loaded", GTI.IsGood(MTime(50))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTGTI::TestLoadFailure()
{
  bool Passed = true;

  MGTI GTI;
  mout.Enable(false);
  mlog.Enable(false);
  mgui.Enable(false);
  __merr.Enable(false);
  Passed = EvaluateFalse("Load()", "missing file", "Load() reports failure for missing GTI files", GTI.Load("/tmp/UTGTI/does_not_exist.gti")) && Passed;
  __merr.Enable(true);
  mgui.Enable(true);
  mlog.Enable(true);
  mout.Enable(true);
  Passed = EvaluateTrue("Load()", "missing file fallback", "A failed load falls back to the default open interval", GTI.IsGood(MTime(12345))) && Passed;
  Passed = EvaluateFalse("Load()", "missing file fallback upper bound", "The failed-load fallback still keeps the configured upper bound", GTI.IsGood(MTime(2000000001))) && Passed;

  Passed = EvaluateTrue("PrepareTempDirectory()", "failure temp dir", "The temporary directory for GTI failure tests can be created", PrepareTempDirectory()) && Passed;

  MString MainFile = "/tmp/UTGTI/missing_include.gti";
  Passed = EvaluateTrue("WriteTextFile()", "missing include file", "A GTI file with a missing include can be written", WriteTextFile(MainFile, "IN does_not_exist.gti\nGT 1 2\nEN\n")) && Passed;

  MGTI IncludeFailure;
  mout.Enable(false);
  mlog.Enable(false);
  mgui.Enable(false);
  __merr.Enable(false);
  Passed = EvaluateFalse("Load()", "missing include", "Load() reports failure when a referenced include file cannot be loaded", IncludeFailure.Load(MainFile)) && Passed;
  __merr.Enable(true);
  mgui.Enable(true);
  mlog.Enable(true);
  mout.Enable(true);
  Passed = EvaluateFalse("IsGood()", "missing include partial state", "Load() keeps the intervals parsed before the failing include load without falling back to all-open", IncludeFailure.IsGood(MTime(100))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTGTI Test;

  return Test.Run() == true ? 0 : 1;
}
