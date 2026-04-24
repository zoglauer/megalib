/*
 * UTQualifiedIsotope.cxx
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
#include "MQualifiedIsotope.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MQualifiedIsotope
class UTQualifiedIsotope : public MUnitTest
{
public:
  //! Default constructor
  UTQualifiedIsotope() : MUnitTest("UTQualifiedIsotope") {}
  //! Default destructor
  virtual ~UTQualifiedIsotope() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test default state and rating helpers
  bool TestDefaultsAndRating();
  //! Test representative line storage and found flags
  bool TestLinesAndFlags();
  //! Test comparisons and deterministic formatting
  bool TestFormatting();
  //! Test out-of-bounds flag access behavior
  bool TestOutOfBounds();
};

MString g_UTQualifiedIsotopeBinary;


////////////////////////////////////////////////////////////////////////////////


bool UTQualifiedIsotope::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndRating() && Passed;
  Passed = TestLinesAndFlags() && Passed;
  Passed = TestFormatting() && Passed;
  Passed = TestOutOfBounds() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTQualifiedIsotope::TestDefaultsAndRating()
{
  bool Passed = true;

  MQualifiedIsotope Isotope;
  Passed = Evaluate("GetElement()", "default", "The default qualified isotope inherits the empty element name", Isotope.GetElement(), MString("")) && Passed;
  Passed = Evaluate("GetNucleons()", "default", "The default qualified isotope inherits zero nucleons", Isotope.GetNucleons(), 0U) && Passed;
  Passed = Evaluate("GetNLines()", "default", "The default qualified isotope starts without lines", Isotope.GetNLines(), 0U) && Passed;
  Passed = Evaluate("GetRating()", "default", "The default qualified isotope rating is initialized to -1", Isotope.GetRating(), -1.0) && Passed;

  Isotope.SetElement("Co");
  Isotope.SetNucleons(60);
  Passed = Evaluate("GetName()", "set identity", "The qualified isotope inherits the base isotope name formatting", Isotope.GetName(), MString("Co-60")) && Passed;

  Isotope.SetRating(3.5);
  Passed = Evaluate("SetRating()", "set rating", "SetRating stores the representative rating", Isotope.GetRating(), 3.5) && Passed;
  Isotope.AddRating(1.25);
  Passed = Evaluate("AddRating()", "add rating", "AddRating increments the representative rating", Isotope.GetRating(), 4.75) && Passed;
  Isotope.AddRating(-0.75);
  Passed = Evaluate("AddRating()", "negative add", "AddRating also supports decreasing the rating", Isotope.GetRating(), 4.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTQualifiedIsotope::TestLinesAndFlags()
{
  bool Passed = true;

  MQualifiedIsotope Isotope;
  Isotope.SetElement("Co");
  Isotope.SetNucleons(57);

  Isotope.AddLine(122.061, 0.856, "D");
  Passed = Evaluate("GetNLines()", "plain line", "AddLine stores a representative qualified-isotope line", Isotope.GetNLines(), 1U) && Passed;
  Passed = EvaluateNear("GetLineEnergy()", "plain line", "The inherited line energy is stored", Isotope.GetLineEnergy(0), 122.061, 1e-12) && Passed;
  Passed = Evaluate("GetLineFound()", "plain line", "Newly added lines start with found=false", Isotope.GetLineFound(0), false) && Passed;

  Isotope.SetLineFound(0, true);
  Passed = Evaluate("SetLineFound()", "true", "SetLineFound stores a representative true flag", Isotope.GetLineFound(0), true) && Passed;
  Isotope.SetLineFound(0, false);
  Passed = Evaluate("SetLineFound()", "false", "SetLineFound also stores false again", Isotope.GetLineFound(0), false) && Passed;

  Isotope.AddLine(136.474, 0.1068, "");
  Passed = Evaluate("GetNLines()", "second line", "A second representative line is appended", Isotope.GetNLines(), 2U) && Passed;
  Passed = Evaluate("GetLineFound()", "second line", "The second appended line gets its own found flag", Isotope.GetLineFound(1), false) && Passed;

  MQualifiedIsotope Filtered;
  Filtered.SetElement("Y");
  Filtered.SetNucleons(88);
  Filtered.AddLine(814.065, 0.1, "E", true);
  Passed = Evaluate("GetNLines()", "excluded filtered", "Excluded lines are skipped when requested", Filtered.GetNLines(), 0U) && Passed;
  Filtered.AddLine(898.042, 0.937, "D", true);
  Passed = Evaluate("GetNLines()", "excluded filtered remaining", "Non-excluded lines still load when filtering is enabled", Filtered.GetNLines(), 1U) && Passed;
  Passed = Evaluate("GetLineFound()", "excluded filtered remaining", "The found-flag list stays aligned with the stored lines after filtering", Filtered.GetLineFound(0), false) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTQualifiedIsotope::TestFormatting()
{
  bool Passed = true;

  MQualifiedIsotope NoRating;
  NoRating.SetElement("Cs");
  NoRating.SetNucleons(137);
  NoRating.AddLine(661.657, 0.851, "D");

  ostringstream StreamNoRating;
  StreamNoRating<<NoRating;
  Passed = Evaluate("operator<<", "no rating", "operator<< omits the rating section when the rating is non-positive", MString(StreamNoRating.str()), MString("Isotope Cs-137 with the following lines: 661.657 keV (0.851)\n")) && Passed;

  MQualifiedIsotope Rated;
  Rated.SetElement("Co");
  Rated.SetNucleons(60);
  Rated.SetRating(2.5);
  Rated.AddLine(1173.237, 0.9997, "D");
  Rated.AddLine(1332.501, 0.9999, "");

  ostringstream StreamRated;
  StreamRated<<Rated;
  Passed = Evaluate("operator<<", "with rating", "operator<< formats the representative qualified isotope deterministically", MString(StreamRated.str()), MString("Isotope Co-60 (rating:2.5) with the following lines: 1173.24 keV (0.9997), 1332.5 keV (0.9999)\n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTQualifiedIsotope::TestOutOfBounds()
{
  bool Passed = true;

  {
    MString LogFileName = "/tmp/UTQualifiedIsotope_setlinefound_fatal.log";
    MString Command = MString("\"") + g_UTQualifiedIsotopeBinary + "\" --fatal-set > " + LogFileName + " 2>&1";
    int Status = system(Command.Data());
    Passed = EvaluateTrue("SetLineFound()", "out of bounds status", "Out-of-bounds SetLineFound access aborts the child process", Status != 0) && Passed;

    ifstream In(LogFileName.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("SetLineFound()", "out of bounds message", "Out-of-bounds SetLineFound access reports the offending index and size", MString(Content).Contains("Index out of bounds: l=1 vs. size()=1")) && Passed;
  }

  {
    MString LogFileName = "/tmp/UTQualifiedIsotope_getlinefound_fatal.log";
    MString Command = MString("\"") + g_UTQualifiedIsotopeBinary + "\" --fatal-get > " + LogFileName + " 2>&1";
    int Status = system(Command.Data());
    Passed = EvaluateTrue("GetLineFound()", "out of bounds status", "Out-of-bounds GetLineFound access aborts the child process", Status != 0) && Passed;

    ifstream In(LogFileName.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("GetLineFound()", "out of bounds message", "Out-of-bounds GetLineFound access reports the offending index and size", MString(Content).Contains("Index out of bounds: l=1 vs. size()=1")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  g_UTQualifiedIsotopeBinary = MString(argv[0]);

  if (g_UTQualifiedIsotopeBinary.IsEmpty() == true) {
    g_UTQualifiedIsotopeBinary = "bin/UTQualifiedIsotope";
  }

  if (argc >= 2) {
    MQualifiedIsotope Isotope;
    Isotope.AddLine(122.061, 0.856, "D");

    MString Argument = argv[1];
    if (Argument == "--fatal-set") {
      Isotope.SetLineFound(1, true);
      return 1;
    }
    if (Argument == "--fatal-get") {
      Isotope.GetLineFound(1);
      return 1;
    }
  }

  UTQualifiedIsotope Test;

  return Test.Run() == true ? 0 : 1;
}
