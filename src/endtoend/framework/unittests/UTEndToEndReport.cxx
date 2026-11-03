/*
 * UTEndToEndReport.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


// Standard libs:
#include <cmath>
#include <cstdlib>
#include <limits>
using namespace std;

// ROOT libs:
#include "TApplication.h"
#include "TError.h"
#include "TFile.h"
#include "TPad.h"
#include "TROOT.h"

// MEGAlib libs:
#include "MEndToEndReport.h"
#include "MEndToEndResultList.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndReport
class UTEndToEndReport : public MUnitTest
{
public:
  UTEndToEndReport() : MUnitTest("UTEndToEndReport") {}
  virtual ~UTEndToEndReport() {}

  virtual bool Run();

private:
  bool TestPullColors();
  bool TestComparisonText();
  bool TestWrite();
  bool TestVersions();
  bool TestGeometryLine();
  bool TestTwoReports();
  bool TestOwnership();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::Run()
{
  bool Passed = true;
  Passed = TestPullColors() && Passed;
  Passed = TestComparisonText() && Passed;
  Passed = TestWrite() && Passed;
  Passed = TestVersions() && Passed;
  Passed = TestGeometryLine() && Passed;
  Passed = TestTwoReports() && Passed;
  Passed = TestOwnership() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestPullColors()
{
  bool Passed = true;

  // Dark green to 1, light green to 2, yellow to 3, orange to 4, red above, the limits belong to the lower color
  const int Dark = MEndToEndReport::GetPullColor(0.0);
  const int Light = MEndToEndReport::GetPullColor(1.5);
  const int Yellow = MEndToEndReport::GetPullColor(2.5);
  const int Orange = MEndToEndReport::GetPullColor(3.5);
  const int Red = MEndToEndReport::GetPullColor(4.5);
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "dark green, upper limit", "A pull of 1 is dark green", MEndToEndReport::GetPullColor(1.0), Dark) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "dark green, negative", "A pull of -1 is dark green", MEndToEndReport::GetPullColor(-1.0), Dark) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "light green, just above 1", "A pull just above 1 is light green", MEndToEndReport::GetPullColor(1.001), Light) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "light green, upper limit", "A pull of 2 is light green", MEndToEndReport::GetPullColor(2.0), Light) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "light green, negative", "A pull of -1.5 is light green", MEndToEndReport::GetPullColor(-1.5), Light) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "yellow, just above 2", "A pull just above 2 is yellow", MEndToEndReport::GetPullColor(2.001), Yellow) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "yellow, upper limit", "A pull of 3 is yellow", MEndToEndReport::GetPullColor(3.0), Yellow) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "orange, just above 3", "A pull just above 3 is orange", MEndToEndReport::GetPullColor(-3.001), Orange) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "orange, upper limit", "A pull of 4 is orange", MEndToEndReport::GetPullColor(4.0), Orange) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "red, just above 4", "A pull just above 4 is red", MEndToEndReport::GetPullColor(4.001), Red) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullColor()", "red, large", "A pull of -50 is red", MEndToEndReport::GetPullColor(-50.0), Red) && Passed;
  Passed = EvaluateTrue("MEndToEndReport::GetPullColor()", "different colors", "The five colors are different", Dark != Light && Light != Yellow && Yellow != Orange && Orange != Red && Dark != Red) && Passed;

  // White text on the dark green and the red, black on the others
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "zero", "Text on dark green is white", MEndToEndReport::GetPullTextColor(0.0), static_cast<int>(kWhite)) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "one", "Text on a pull of 1 is white", MEndToEndReport::GetPullTextColor(-1.0), static_cast<int>(kWhite)) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "just above one", "Text on light green is black", MEndToEndReport::GetPullTextColor(1.001), static_cast<int>(kBlack)) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "four", "Text on orange is black", MEndToEndReport::GetPullTextColor(4.0), static_cast<int>(kBlack)) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "just above four", "Text on red is white", MEndToEndReport::GetPullTextColor(4.001), static_cast<int>(kWhite)) && Passed;
  Passed = Evaluate("MEndToEndReport::GetPullTextColor()", "NaN", "Text on the red of a NaN pull is white", MEndToEndReport::GetPullTextColor(numeric_limits<double>::quiet_NaN()), static_cast<int>(kWhite)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestComparisonText()
{
  bool Passed = true;

  // A statistical comparison of 1.25 +- 0.3 with 1.0 +- 0.4 and a limited comparison of 0.3 with 0 and the tolerance 0.5
  ETMeasuredValue Small, Reference, Distance, NoDistance;
  Small.m_Value = 1.25;
  Small.m_Error = 0.3;
  Reference.m_Value = 1.0;
  Reference.m_Error = 0.4;
  Distance.m_Value = 0.3;

  // The text of a statistical pull has the unit sigma, the text of a score of a limit has none: the score is 0.3*5/0.5 = 3.0
  const MString PullText = MEndToEndReport::GetComparisonText("w", Small, Reference, MEndToEndResultList::GetStatisticalTolerance(Small, Reference), true);
  Passed = EvaluateTrue("MEndToEndReport::GetComparisonText()", "pull", "A statistical pull is written in sigma", PullText.EndsWith("0.5#sigma")) && Passed;
  const MString ScoreText = MEndToEndReport::GetComparisonText("w", Distance, NoDistance, 0.5, false);
  Passed = EvaluateTrue("MEndToEndReport::GetComparisonText()", "score", "The score of a limit is written without sigma", ScoreText.EndsWith("score 3.0")) && Passed;
  Passed = EvaluateFalse("MEndToEndReport::GetComparisonText()", "no sigma", "The score of a limit has no sigma", ScoreText.Contains("sigma")) && Passed;

  return Passed;
}
////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestWrite()
{
  bool Passed = true;

  // The test has two key results and one page: the title page, the key results, the pulls, and the page make four pages
  const MString Directory = GetTemporaryDirectoryName("write");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "write", "The directory for the summary can be created", MFile::CreateDirectory(Directory)) && Passed;
  {
    MEndToEndReport Report("Probe");
    Report.SetDirectory(Directory);
    MEndToEndResultList Results;
    ETMeasuredValue Measured, Expected;
    Measured.m_Value = 10.2;
    Measured.m_Error = 0.3;
    Expected.m_Value = 10.0;
    Expected.m_Error = 0.5;
    Results.AddComparison("S/O", Measured, Expected, MEndToEndResultList::c_PullLimit*sqrt(0.3*0.3 + 0.5*0.5), true);
    Results.AddResult("Free", 1.0, 0.1);
    TPad* Body = Report.AddPage("Page", 1, 1);
    Body->cd(1);
    Report.DrawHistogram("Test;x;y", vector<double>{ 1.0, 2.0, 2.0 }, 4, 0.0, 4.0);
    Passed = EvaluateSize("MEndToEndReport::GetPages()", "before", "The page of the test is the only page before writing", Report.GetPages().size(), 1) && Passed;
    DisableDefaultStreams();
    Report.Write("Max.geo.setup", Results);
    EnableDefaultStreams();
  }
  Passed = EvaluateFalse("MEndToEndReport::Write()", "no key results file", "The report writes the pages, the key results are written by the results", MFile::Exists(Directory + "/Probe.keyresults.tsv")) && Passed;
  Passed = EvaluateTrue("MEndToEndReport::Write()", "PDF", "The summary PDF exists", MFile::Exists(Directory + "/Probe.pdf")) && Passed;
  Passed = EvaluateTrue("MEndToEndReport::Write()", "ROOT file", "The ROOT file with the canvases exists", MFile::Exists(Directory + "/Probe.summary.root")) && Passed;
  TFile File((Directory + "/Probe.summary.root").Data());
  Passed = EvaluateTrue("MEndToEndReport::Write()", "open", "The ROOT file can be opened", File.IsZombie() == false) && Passed;
  if (File.IsZombie() == false) {
    Passed = Evaluate("MEndToEndReport::Write()", "pages", "Title page, key results, pulls, and the page of the test", File.GetListOfKeys()->GetEntries(), 4) && Passed;
  }

  // Without pages and key results nothing is written
  const MString EmptyDirectory = GetTemporaryDirectoryName("empty");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "empty", "The directory for the empty summary can be created", MFile::CreateDirectory(EmptyDirectory)) && Passed;
  {
    MEndToEndReport Report("Probe");
    Report.SetDirectory(EmptyDirectory);
    DisableDefaultStreams();
    Report.Write("Max.geo.setup", MEndToEndResultList());
    EnableDefaultStreams();
  }
  Passed = EvaluateFalse("MEndToEndReport::Write()", "empty", "A report without pages and key results writes no summary", MFile::Exists(EmptyDirectory + "/Probe.pdf")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestVersions()
{
  bool Passed = true;

  const MString Directory = GetTemporaryDirectoryName("versions");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "versions", "The directory for the versions can be created", MFile::CreateDirectory(Directory)) && Passed;
  MEndToEndReport Report("Probe");
  Report.SetDirectory(Directory);
  const vector<MString> Versions = Report.GetVersionLines();
  Passed = EvaluateSize("MEndToEndReport::GetVersionLines()", "number", "MEGAlib, Geant4, ROOT, HEALPix, and cfitsio", Versions.size(), 5) && Passed;
  if (Versions.size() == 5) {
    Passed = EvaluateTrue("MEndToEndReport::GetVersionLines()", "MEGAlib", "The MEGAlib version is read from the version file, not the default of the library", Versions[0].BeginsWith("MEGAlib:") == true && Versions[0].Contains("unknown") == false && Versions[0].Contains("1.00.00") == false) && Passed;
    Passed = EvaluateTrue("MEndToEndReport::GetVersionLines()", "ROOT", "The ROOT version is known", Versions[2].BeginsWith("ROOT:") == true && Versions[2].Contains("unknown") == false) && Passed;
  }
  Passed = EvaluateFalse("MEndToEndReport::GetVersionLines()", "temporary files", "pkg-config leaves no files in the directory", MFile::Exists(Directory + "/.Probe_cfitsio.version") || MFile::Exists(Directory + "/.Probe_healpix_cxx.version")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestGeometryLine()
{
  bool Passed = true;

  // The environment variable is changed and restored: setenv and unsetenv are POSIX, the C++ standard library has no equivalent
  const char* Old = getenv("MEGALIB");
  MString OldValue;
  if (Old != nullptr) {
    OldValue = Old;
  }

  setenv("MEGALIB", "/opt/MEGAlib", 1);
  Passed = Evaluate("MEndToEndReport::GetGeometryLine()", "inside", "A geometry inside MEGALIB is shown relative to it", MEndToEndReport::GetGeometryLine("/opt/MEGAlib/resource/Max.geo.setup"), MString("Geometry:  ${MEGALIB}/resource/Max.geo.setup")) && Passed;
  Passed = Evaluate("MEndToEndReport::GetGeometryLine()", "similar name", "A path which only starts like MEGALIB keeps its full path", MEndToEndReport::GetGeometryLine("/opt/MEGAlib2/Max.geo.setup"), MString("Geometry:  /opt/MEGAlib2/Max.geo.setup")) && Passed;
  Passed = Evaluate("MEndToEndReport::GetGeometryLine()", "outside", "A geometry outside MEGALIB keeps its full path", MEndToEndReport::GetGeometryLine("/home/user/My.geo.setup"), MString("Geometry:  /home/user/My.geo.setup")) && Passed;
  // ROOT reports the variable which is not defined on stderr
  unsetenv("MEGALIB");
  const int OldErrorLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;
  const MString WithoutMegalib = MEndToEndReport::GetGeometryLine("/opt/MEGAlib/Max.geo.setup");
  gErrorIgnoreLevel = OldErrorLevel;
  Passed = Evaluate("MEndToEndReport::GetGeometryLine()", "no MEGALIB", "Without MEGALIB the full path is shown", WithoutMegalib, MString("Geometry:  /opt/MEGAlib/Max.geo.setup")) && Passed;

  if (Old != nullptr) {
    setenv("MEGALIB", OldValue.Data(), 1);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestTwoReports()
{
  bool Passed = true;

  // ROOT deletes a canvas when a new canvas gets its name: the pages of reports which exist at the same time need different names
  const int Before = gROOT->GetListOfCanvases()->GetSize();
  {
    MEndToEndReport First("First");
    MEndToEndReport Second("Second");
    First.AddPage("First", 1, 1);
    Second.AddPage("Second", 1, 1);
    Passed = Evaluate("MEndToEndReport::AddPage()", "canvases", "The pages of two reports are two canvases", gROOT->GetListOfCanvases()->GetSize(), Before + 2) && Passed;
    if (gROOT->GetListOfCanvases()->GetSize() == Before + 2) {
      Passed = EvaluateTrue("MEndToEndReport::AddPage()", "names", "The pages of two reports have different names", MString(First.GetPages()[0]->GetName()) != MString(Second.GetPages()[0]->GetName())) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndReport::TestOwnership()
{
  bool Passed = true;

  // A histogram which is drawn while a ROOT file is the current directory belongs to the page, not to the file: the file deletes what it owns
  // when it closes, and the page would print a deleted histogram
  const MString Directory = GetTemporaryDirectoryName("ownership");
  Passed = EvaluateTrue("MFile::CreateDirectory()", "ownership", "The directory for the file can be created", MFile::CreateDirectory(Directory)) && Passed;
  MEndToEndReport Report("Owner");
  TPad* Body = nullptr;
  TH1D* Histogram = nullptr;
  {
    TFile File((Directory + "/open.root").Data(), "RECREATE");
    Passed = EvaluateTrue("TFile", "open", "The file can be opened", File.IsZombie() == false) && Passed;
    Body = Report.AddPage("Owned", 1, 1);
    Body->cd(1);
    Histogram = Report.DrawHistogram("Owned;x;y", vector<double>{ 1.0, 2.0, 2.0 }, 4, 0.0, 4.0);
    Passed = EvaluateTrue("MEndToEndReport::DrawHistogram()", "not in the file", "The histogram is not registered in the open file", File.GetList()->FindObject(Histogram) == nullptr) && Passed;
    File.Close();
  }
  // A deleted histogram is removed from the primitives of its pad: it has to be there still
  Passed = EvaluateTrue("MEndToEndReport::DrawHistogram()", "survives the file", "The histogram is still a primitive of the page after the file closed", Body->cd(1)->GetListOfPrimitives()->FindObject(Histogram) != nullptr) && Passed;
  Passed = EvaluateTrue("MEndToEndReport::DrawHistogram()", "owned by the page", "The page deletes the histogram with itself", Histogram->TestBit(kCanDelete) == true) && Passed;

  // The frame and the graphs of the pulls page are owned by the page too
  MEndToEndResultList Results;
  ETMeasuredValue Measured, Expected;
  Measured.m_Value = 10.2;
  Measured.m_Error = 0.3;
  Expected.m_Value = 10.0;
  Expected.m_Error = 0.5;
  Results.AddComparison("S/O", Measured, Expected, MEndToEndResultList::c_PullLimit*sqrt(0.3*0.3 + 0.5*0.5), true);
  // A comparison with a tolerance which is no pull goes into the second graph: both graphs are drawn
  Results.AddComparison("S/T", Measured, Expected, 1.0, false);
  {
    TFile File((Directory + "/pulls.root").Data(), "RECREATE");
    Report.AddPullsPage(Results);
    Passed = EvaluateTrue("MEndToEndReport::AddPullsPage()", "not in the file", "The frame of the pulls page is not registered in the open file", File.GetList()->GetSize() == 0) && Passed;
    File.Close();
  }
  TPad* PullsPage = Report.GetPages().back();
  unsigned int OwnedPrimitives = 0;
  TIter Next(PullsPage->cd(1)->GetListOfPrimitives());
  while (TObject* Primitive = Next()) {
    if (Primitive->InheritsFrom("TH1") == true || Primitive->InheritsFrom("TGraph") == true) {
      if (Primitive->TestBit(kCanDelete) == true) {
        ++OwnedPrimitives;
      }
    }
  }
  Passed = Evaluate("MEndToEndReport::AddPullsPage()", "owned", "The frame and the two graphs of the pulls page are owned by the page", OwnedPrimitives, 3U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc.
  if (MGlobal::Initialize("UTEndToEndReport", "unit test of the end-to-end report") == false) {
    return 1;
  }

  // ROOT runs without graphics - the canvases of the pages are written without a display
  TApplication ROOT("ROOT", 0, 0);
  gROOT->SetBatch(true);

  UTEndToEndReport Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndReport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
