/*
 * UTMIsotope.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MIsotope.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MIsotope
class UTMIsotope : public MUnitTest
{
public:
  //! Default constructor
  UTMIsotope() : MUnitTest("UTMIsotope") {}
  //! Default destructor
  virtual ~UTMIsotope() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test basic state and naming helpers
  bool TestBasics();
  //! Test line storage and per-line getters
  bool TestLines();
  //! Test default-line selection rules
  bool TestDefaultLine();
  //! Test comparisons and string formatting
  bool TestComparisonAndFormatting();
  //! Test out-of-bounds getter behavior
  bool TestOutOfBounds();
};

MString g_UTMIsotopeBinary;


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestLines() && Passed;
  Passed = TestDefaultLine() && Passed;
  Passed = TestComparisonAndFormatting() && Passed;
  Passed = TestOutOfBounds() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::TestBasics()
{
  bool Passed = true;

  MIsotope Isotope;
  Passed = Evaluate("GetElement()", "default", "The default isotope has an empty element name", Isotope.GetElement(), MString("")) && Passed;
  Passed = Evaluate("GetNucleons()", "default", "The default isotope has zero nucleons", Isotope.GetNucleons(), 0U) && Passed;
  Passed = Evaluate("GetName()", "default", "The default isotope name is empty when no element or nucleons are set", Isotope.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetNLines()", "default", "The default isotope has no decay lines", Isotope.GetNLines(), 0U) && Passed;
  Passed = Evaluate("GetDefaultLine()", "default", "The default isotope has no default decay line", Isotope.GetDefaultLine(), -1) && Passed;

  Isotope.SetElement("Cs");
  Passed = Evaluate("GetElement()", "set element", "SetElement stores the element symbol", Isotope.GetElement(), MString("Cs")) && Passed;
  Passed = Evaluate("GetName()", "element only", "GetName returns the element when nucleons are not set", Isotope.GetName(), MString("Cs")) && Passed;

  Isotope.SetNucleons(137);
  Passed = Evaluate("GetNucleons()", "set nucleons", "SetNucleons stores the nucleon count", Isotope.GetNucleons(), 137U) && Passed;
  Passed = Evaluate("GetName()", "element and nucleons", "GetName formats element and nucleons deterministically", Isotope.GetName(), MString("Cs-137")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::TestLines()
{
  bool Passed = true;

  MIsotope Isotope;
  Isotope.SetElement("Co");
  Isotope.SetNucleons(60);

  Isotope.AddLine(1173.228, 0.9985, "");
  Passed = Evaluate("GetNLines()", "plain line", "AddLine stores a representative decay line", Isotope.GetNLines(), 1U) && Passed;
  Passed = EvaluateNear("GetLineEnergy()", "plain line", "GetLineEnergy returns the stored representative energy", Isotope.GetLineEnergy(0), 1173.228, 1e-12) && Passed;
  Passed = EvaluateNear("GetLineBranchingRatio()", "plain line", "GetLineBranchingRatio returns the stored representative branching ratio", Isotope.GetLineBranchingRatio(0), 0.9985, 1e-12) && Passed;
  Passed = Evaluate("GetLineExcludeFlag()", "plain line", "A line without flags is not marked excluded", Isotope.GetLineExcludeFlag(0), false) && Passed;

  Isotope.AddLine(1332.492, 0.9998, "E");
  Passed = Evaluate("GetNLines()", "excluded line", "AddLine stores excluded lines when ExcludeExcludedLines is false", Isotope.GetNLines(), 2U) && Passed;
  Passed = Evaluate("GetLineExcludeFlag()", "excluded line", "The E flag marks a stored line as excluded", Isotope.GetLineExcludeFlag(1), true) && Passed;

  Isotope.AddLine(2505.0, 0.01, "E", true);
  Passed = Evaluate("GetNLines()", "skip excluded line", "AddLine can skip excluded lines when requested", Isotope.GetNLines(), 2U) && Passed;

  Isotope.AddLine(2150.0, 0.05, "D");
  Passed = Evaluate("GetNLines()", "default line flag", "AddLine stores lines with default flags", Isotope.GetNLines(), 3U) && Passed;
  Passed = EvaluateNear("GetLineEnergy()", "default line flag", "Lines added after excluded/skipped inputs keep their stored order", Isotope.GetLineEnergy(2), 2150.0, 1e-12) && Passed;
  Passed = Evaluate("GetLineExcludeFlag()", "default line flag", "Default lines are not excluded unless explicitly flagged", Isotope.GetLineExcludeFlag(2), false) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::TestDefaultLine()
{
  bool Passed = true;

  MIsotope Empty;
  Passed = Evaluate("GetDefaultLine()", "no lines", "GetDefaultLine returns -1 when no lines exist", Empty.GetDefaultLine(), -1) && Passed;

  MIsotope Strongest;
  Strongest.AddLine(100.0, 0.1, "");
  Strongest.AddLine(200.0, 0.3, "");
  Strongest.AddLine(300.0, 0.2, "");
  Passed = Evaluate("GetDefaultLine()", "strongest line", "Without D flags the strongest non-excluded branching ratio is chosen", Strongest.GetDefaultLine(), 1) && Passed;

  MIsotope DefaultPreferred;
  DefaultPreferred.AddLine(100.0, 0.9, "");
  DefaultPreferred.AddLine(200.0, 0.1, "D");
  Passed = Evaluate("GetDefaultLine()", "default flag", "A non-excluded D line is preferred over stronger non-default lines", DefaultPreferred.GetDefaultLine(), 1) && Passed;

  MIsotope ExcludedDefault;
  ExcludedDefault.AddLine(100.0, 0.2, "");
  ExcludedDefault.AddLine(200.0, 0.9, "DE");
  ExcludedDefault.AddLine(300.0, 0.5, "");
  Passed = Evaluate("GetDefaultLine()", "excluded default flag", "Excluded D lines are ignored and the strongest remaining line is chosen", ExcludedDefault.GetDefaultLine(), 2) && Passed;

  MIsotope AllExcluded;
  AllExcluded.AddLine(100.0, 0.2, "E");
  AllExcluded.AddLine(200.0, 0.9, "DE");
  Passed = Evaluate("GetDefaultLine()", "all excluded", "GetDefaultLine returns -1 when all stored lines are excluded", AllExcluded.GetDefaultLine(), -1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::TestComparisonAndFormatting()
{
  bool Passed = true;

  MIsotope A;
  A.SetElement("Cs");
  A.SetNucleons(137);
  A.AddLine(661.657, 0.851, "");

  MIsotope B;
  B.SetElement("Cs");
  B.SetNucleons(137);
  B.AddLine(32.0, 0.05, "");

  MIsotope C;
  C.SetElement("Ba");
  C.SetNucleons(133);

  MIsotope D;
  D.SetElement("Cs");
  D.SetNucleons(138);

  Passed = Evaluate("operator==", "same isotope id", "operator== compares element and nucleon identity", A == B, true) && Passed;
  Passed = Evaluate("operator==", "different isotope id", "operator== rejects different isotope identities", A == C, false) && Passed;
  Passed = Evaluate("operator<", "different nucleons", "operator< sorts primarily by nucleon count", C < A, true) && Passed;
  Passed = Evaluate("operator<", "same nucleons element", "operator< sorts secondarily by element name", C < D, true) && Passed;

  Passed = Evaluate("ToString()", "with lines", "ToString formats isotope identity and representative lines deterministically", A.ToString(), MString("Isotope Cs-137 with the following lines: 661.657 keV (0.851)")) && Passed;

  ostringstream Stream;
  Stream<<A;
  Passed = Evaluate("operator<<", "stream", "operator<< streams the same deterministic text as ToString()", MString(Stream.str()), A.ToString()) && Passed;

  MIsotope NameOnly;
  NameOnly.SetElement("U");
  Passed = Evaluate("ToString()", "no lines", "ToString omits the line list when no decay lines are stored", NameOnly.ToString(), MString("Isotope U")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTMIsotope::TestOutOfBounds()
{
  bool Passed = true;

  MIsotope Isotope;
  Isotope.AddLine(661.657, 0.851, "");

  __merr.Enable(false);
  Passed = EvaluateNear("GetLineEnergy()", "out of bounds", "Out-of-bounds energy access returns zero after reporting the error", Isotope.GetLineEnergy(1), 0.0, 1e-12) && Passed;
  __merr.Enable(true);

  {
    MString LogFileName = "/tmp/UTMIsotope_branching_fatal.log";
    MString Command = MString("\"") + g_UTMIsotopeBinary + "\" --fatal-branching > " + LogFileName + " 2>&1";
    int Status = system(Command.Data());
    Passed = EvaluateTrue("GetLineBranchingRatio()", "out of bounds status", "Out-of-bounds branching-ratio access aborts the child process", Status != 0) && Passed;

    ifstream In(LogFileName.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("GetLineBranchingRatio()", "out of bounds message", "Out-of-bounds branching-ratio access reports the offending index and size", MString(Content).Contains("Index out of bounds: i=1 vs. size()=1")) && Passed;
  }

  {
    MString LogFileName = "/tmp/UTMIsotope_exclude_fatal.log";
    MString Command = MString("\"") + g_UTMIsotopeBinary + "\" --fatal-exclude > " + LogFileName + " 2>&1";
    int Status = system(Command.Data());
    Passed = EvaluateTrue("GetLineExcludeFlag()", "out of bounds status", "Out-of-bounds exclude-flag access aborts the child process", Status != 0) && Passed;

    ifstream In(LogFileName.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("GetLineExcludeFlag()", "out of bounds message", "Out-of-bounds exclude-flag access reports the offending index and size", MString(Content).Contains("Index out of bounds: i=1 vs. size()=1")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  g_UTMIsotopeBinary = MString(argv[0]);

  if (g_UTMIsotopeBinary.IsEmpty() == true) {
    g_UTMIsotopeBinary = "bin/UTMIsotope";
  }

  if (argc >= 2) {
    MIsotope Isotope;
    Isotope.AddLine(661.657, 0.851, "");

    MString Argument = argv[1];
    if (Argument == "--fatal-branching") {
      Isotope.GetLineBranchingRatio(1);
      return 1;
    }
    if (Argument == "--fatal-exclude") {
      Isotope.GetLineExcludeFlag(1);
      return 1;
    }
  }

  UTMIsotope Test;

  return Test.Run() == true ? 0 : 1;
}
