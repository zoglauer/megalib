/*
 * MEndToEndReport.cxx
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

// Include the header:
#include "MEndToEndReport.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TBox.h"
#include "TError.h"
#include "TFile.h"
#include "TGraph.h"
#include "TLatex.h"
#include "TLine.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TStyle.h"

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"
#include "MSystem.h"


//! Standard constructor giving the name of the test
MEndToEndReport::MEndToEndReport(const MString& TestName) : m_TestName(TestName)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Add a key result: name, value, uncertainty, return it (valid until the next key result is added)
ETKeyResult& MEndToEndReport::AddKeyResult(const MString& Name, double Value, double Error)
{
  ETKeyResult Result;
  Result.m_Name = Name;
  Result.m_Value = Value;
  Result.m_Error = Error;
  m_KeyResults.push_back(Result);
  return m_KeyResults.back();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a key result which was measured before (e.g. read from a file), return it (valid until the next key result is added)
ETKeyResult& MEndToEndReport::AddKeyResult(const ETKeyResult& Result)
{
  m_KeyResults.push_back(Result);
  return m_KeyResults.back();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a comparison of a measured value with an expected value to the key results, the tolerance is the largest accepted difference, return it (valid until the next key result is added)
ETKeyResult& MEndToEndReport::AddComparison(const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  ETKeyResult& Result = AddKeyResult(Name, Measured.m_Value, Measured.m_Error);
  Result.m_HasReference = true;
  Result.m_Reference = Expected.m_Value;
  Result.m_ReferenceError = Expected.m_Error;
  Result.m_Tolerance = Tolerance;
  Result.m_IsStatistical = IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical);
  Result.m_Pull = GetScore(Measured, Expected, Tolerance, IsStatistical);

  // A comparison with a value which is not valid fails: the check of the test overwrites the outcome
  Result.m_IsValid = false;
  if (Measured.IsValid() == true && Expected.IsValid() == true && isfinite(Tolerance) == true) {
    Result.m_IsValid = true;
  }
  Result.m_Passed = false;
  if (Result.m_IsValid == true && fabs(Measured.m_Value - Expected.m_Value) <= Tolerance) {
    Result.m_Passed = true;
  }
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the statistics of the pulls of all key results which have a reference
ETPullStatistics MEndToEndReport::GetPullStatistics() const
{
  // Only valid statistical comparisons are Gaussian pulls
  ETPullStatistics Statistics;
  for (const ETKeyResult& Result: m_KeyResults) {
    if (Result.m_HasReference == true && Result.m_IsValid == true && Result.m_IsStatistical == true) {
      ++Statistics.m_NumberOfPulls;
      for (unsigned int s = 0; s < 3; ++s) {
        if (fabs(Result.m_Pull) <= s + 1.0) {
          ++Statistics.m_Inside[s];
        }
      }
    }
  }
  return Statistics;
}


////////////////////////////////////////////////////////////////////////////////


//! Read the key results which Write() stored in the file (empty if the file cannot be read)
vector<ETKeyResult> MEndToEndReport::ReadKeyResults(const MString& FileName)
{
  vector<ETKeyResult> Results;

  // A missing file is not an error: tests without key results write none
  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    return Results;
  }

  // One key result per line, the columns are separated by tabs
  const unsigned int NumberOfColumns = 11;
  for (const MString& Line: Content.Tokenize("\n")) {
    if (Line.BeginsWith("#") == true) {
      continue;
    }
    const vector<MString> Columns = Line.Tokenize("\t");
    if (Columns.size() != NumberOfColumns) {
      continue;
    }
    ETKeyResult Result;
    Result.m_Name = Columns[0];
    Result.m_Value = Columns[1].ToDouble();
    Result.m_Error = Columns[2].ToDouble();
    Result.m_HasReference = false;
    if (Columns[3].ToInt() != 0) {
      Result.m_HasReference = true;
    }
    Result.m_Reference = Columns[4].ToDouble();
    Result.m_ReferenceError = Columns[5].ToDouble();
    Result.m_Pull = Columns[6].ToDouble();
    Result.m_Tolerance = Columns[7].ToDouble();
    Result.m_IsValid = false;
    if (Columns[8].ToInt() != 0) {
      Result.m_IsValid = true;
    }
    Result.m_IsStatistical = false;
    if (Columns[9].ToInt() != 0) {
      Result.m_IsStatistical = true;
    }
    Result.m_Passed = false;
    if (Columns[10].ToInt() != 0) {
      Result.m_Passed = true;
    }
    Results.push_back(Result);
  }
  return Results;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a page with a title and a pad which is divided into columns x rows, return the pad: draw into pad->cd(1 ... columns*rows)
TPad* MEndToEndReport::AddPage(const MString& Title, unsigned int Columns, unsigned int Rows)
{
  unique_ptr<TCanvas> Page(new TCanvas(MString("SummaryPage") + m_NumberOfCreatedPages++, Title.Data(), c_PageWidth, c_PageHeight));
  // The header is a pad of its own: the texts which belong to the canvas itself are lost when a canvas read from a file is printed together with others
  Page->cd();
  TPad* Header = new TPad("Header", "Header", 0.0, c_BodyTop, 1.0, 1.0);
  Header->SetBorderMode(0);
  Header->SetBorderSize(0);
  Header->Draw();
  Header->cd();
  TLatex Label;
  Label.SetNDC();
  Label.SetTextFont(62);
  Label.SetTextSize(c_HeaderTitleSize);
  Label.DrawLatex(c_HeaderLeft, c_HeaderTitleY, Title.Data());
  Label.SetTextFont(42);
  Label.SetTextSize(c_HeaderTestSize);
  Label.SetTextColor(kGray + 2);
  Label.DrawLatex(c_HeaderLeft, c_HeaderTestY, (MString("Test: ") + m_TestName).Data());
  Page->cd();
  TPad* Body = new TPad("Body", "Body", 0.0, 0.0, 1.0, c_BodyTop);
  Body->Draw();
  Body->Divide(max(1U, Columns), max(1U, Rows));
  m_Pages.push_back(std::move(Page));
  return Body;
}


////////////////////////////////////////////////////////////////////////////////


//! Add pages with lines of text
void MEndToEndReport::AddTextPages(const MString& Title, const vector<MString>& Lines)
{
  unsigned int Start = 0;
  do {
    TPad* Body = AddPage(Title, 1, 1);
    Body->cd(1);
    gPad->SetFillColor(0);
    TLatex Text;
    Text.SetNDC();
    Text.SetTextFont(82);
    Text.SetTextSize(c_TextSize);
    for (unsigned int l = Start; l < Lines.size() && l < Start + c_TextLinesPerPage; ++l) {
      Text.DrawLatex(c_TextLeft, c_TextTop - c_TextPitch*(l - Start), Lines[l].Data());
    }
    Start += c_TextLinesPerPage;
  } while (Start < Lines.size());
}


////////////////////////////////////////////////////////////////////////////////


//! Create a histogram of values (owned by the report), draw it into the current pad with the options, return it
TH1D* MEndToEndReport::DrawHistogram(const MString& Title, const vector<double>& Values, unsigned int Bins, double Minimum, double Maximum, const MString& Options)
{
  TH1D* Histogram = KeepObject(unique_ptr<TH1D>(new TH1D(MString("SummaryHistogram") + m_Objects.size(), Title.Data(), Bins, Minimum, Maximum)));
  Histogram->SetDirectory(nullptr);
  for (double Value: Values) {
    Histogram->Fill(Value);
  }
  Histogram->SetStats(false);
  Histogram->SetLineColor(kBlack);
  Histogram->SetMaximum(1.4*Histogram->GetMaximum());
  Histogram->Draw(Options.Data());
  return Histogram;
}


////////////////////////////////////////////////////////////////////////////////


//! Write a row into the current pad with the measured and the expected value and their pull, on the color of the pull (rows count from the top)
void MEndToEndReport::DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  DrawComparison(Row, Label, Measured, Expected, GetStatisticalTolerance(Measured, Expected), true);
}


////////////////////////////////////////////////////////////////////////////////


//! Write a row for a comparison with a tolerance: as above, the score is the one of AddComparison
void MEndToEndReport::DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  const double Pull = GetScore(Measured, Expected, Tolerance, IsStatistical);
  const MString Text = GetComparisonText(Label, Measured, Expected, Tolerance, IsStatistical);

  // The row stays inside the frame of the histogram, the text shrinks to fit
  const double Left = gPad->GetLeftMargin() + 0.005;
  const double Right = 1.0 - gPad->GetRightMargin() - 0.005;
  const double Top = 1.0 - gPad->GetTopMargin() - 0.005 - c_RowPitch*Row;
  DrawPullBox(gPad, Left, Top - c_RowHeight, Right, Top, Text, Pull, c_RowTextSize);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text of a row of a comparison: a statistical pull is in sigma, the score of a comparison with a limit has no unit
MString MEndToEndReport::GetComparisonText(const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  const double Score = GetScore(Measured, Expected, Tolerance, IsStatistical);
  ostringstream Text;
  Text<<Label.Data()<<" "<<setprecision(4)<<Measured.m_Value<<"#pm"<<setprecision(2)<<Measured.m_Error<<", expected "<<setprecision(4)<<Expected.m_Value;
  if (IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical) == true) {
    Text<<"#pm"<<setprecision(2)<<Expected.m_Error<<": "<<fixed<<setprecision(1)<<Score<<"#sigma";
  } else {
    Text<<" (limit #pm"<<setprecision(2)<<Tolerance<<"): score "<<fixed<<setprecision(1)<<Score;
  }
  return Text.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Write a text line into the current pad: NDC coordinates, color index of ROOT
void MEndToEndReport::DrawLabel(double X, double Y, const MString& Text, int Color)
{
  TLatex Label;
  Label.SetNDC();
  Label.SetTextFont(42);
  Label.SetTextSize(c_RowTextSize);
  Label.SetTextColor(Color);
  Label.DrawLatex(X, Y, Text.Data());
}


////////////////////////////////////////////////////////////////////////////////


//! Add the page with the pulls of all comparisons and their statistics
void MEndToEndReport::AddPullsPage()
{
  // Only valid comparisons have a score which can be shown, the invalid ones fail in the table of the key results
  vector<const ETKeyResult*> Shown;
  for (const ETKeyResult& Result: m_KeyResults) {
    if (Result.m_HasReference == true && Result.m_IsValid == true) {
      Shown.push_back(&Result);
    }
  }
  if (Shown.empty() == true) {
    return;
  }

  TPad* Body = AddPage("Pulls and scores of the comparisons", 1, 1);
  Body->cd(1);
  gPad->SetRightMargin(c_PanelMargin);
  TH1D* Frame = KeepObject(unique_ptr<TH1D>(new TH1D("PullFrame", ";comparison;pull (#sigma) or score", Shown.size(), 0.5, Shown.size() + 0.5)));
  Frame->SetDirectory(nullptr);
  Frame->SetStats(false);
  Frame->SetMinimum(-c_PullAxisLimit);
  Frame->SetMaximum(c_PullAxisLimit);
  Frame->GetYaxis()->SetNdivisions(static_cast<int>(2*c_PullAxisLimit), kFALSE);
  Frame->Draw("AXIS");
  // A horizontal grid line for every sigma
  for (int Level = -c_PullGridMaximum; Level <= c_PullGridMaximum; ++Level) {
    if (Level != 0) {
      TLine* Grid = new TLine(0.5, Level, Shown.size() + 0.5, Level);
      Grid->SetLineColor(kGray);
      Grid->SetLineStyle(3);
      Grid->Draw();
    }
  }
  TLine* Zero = new TLine(0.5, 0.0, Shown.size() + 0.5, 0.0);
  Zero->SetLineColor(kGray + 1);
  Zero->Draw();
  TGraph* Graph = KeepObject(unique_ptr<TGraph>(new TGraph()));
  TGraph* ScoreGraph = KeepObject(unique_ptr<TGraph>(new TGraph()));
  for (unsigned int p = 0; p < Shown.size(); ++p) {
    TGraph* Target = ScoreGraph;
    if (Shown[p]->m_IsStatistical == true) {
      Target = Graph;
    }
    Target->SetPoint(Target->GetN(), p + 1, max(-c_PullEdge, min(c_PullEdge, Shown[p]->m_Pull)));
  }
  // ROOT does not draw a graph without points
  Graph->SetMarkerStyle(20);
  Graph->SetMarkerSize(0.8);
  if (Graph->GetN() > 0) {
    Graph->Draw("P SAME");
  }
  ScoreGraph->SetMarkerStyle(24);
  ScoreGraph->SetMarkerSize(0.8);
  if (ScoreGraph->GetN() > 0) {
    ScoreGraph->Draw("P SAME");
  }
  for (double Limit: { -c_PullLimit, c_PullLimit }) {
    TLine* Line = new TLine(0.5, Limit, Shown.size() + 0.5, Limit);
    Line->SetLineColor(kRed);
    Line->SetLineStyle(2);
    Line->Draw();
  }

  // The counts of the pulls which are inside one, two, and three sigma next to the Gaussian fractions
  const ETPullStatistics Statistics = GetPullStatistics();
  MString Header = "No statistical pulls";
  if (Statistics.m_NumberOfPulls > 0) {
    Header = MString("Fraction of the ") + Statistics.m_NumberOfPulls + " pulls inside:";
  }
  DrawPanelBox(gPad, c_PanelLeft, c_PanelTitleBottom, c_PanelRight, c_PanelTitleTop, Header, c_PanelTextSize);
  for (unsigned int s = 1; s <= 3 && Statistics.m_NumberOfPulls > 0; ++s) {
    ostringstream Text;
    Text<<"|pull| <= "<<s<<": "<<Statistics.m_Inside[s - 1]<<"/"<<Statistics.m_NumberOfPulls<<" ("<<fixed<<setprecision(1)<<100.0*Statistics.GetFraction(s).m_Value<<"%), Gauss "<<100.0*ETPullStatistics::c_GaussianFractions[s - 1]<<"%";
    const double Top = c_PanelRowTop - c_PanelRowPitch*(s - 1);
    DrawPanelBox(gPad, c_PanelLeft, Top - c_PanelRowHeight, c_PanelRight, Top, Text.str().c_str(), c_PanelTextSize);
  }
  // The pulls of observables which are measured from the same events are correlated: the counts have no significance
  if (Statistics.m_NumberOfPulls > 0) {
    DrawPanelBox(gPad, c_PanelLeft, c_PanelNoteBottom, c_PanelRight, c_PanelNoteTop, "Observables share events: guide only", c_PanelTextSize);
  }

  // The markers: pulls of the statistical comparisons and scores of the comparisons with a limit
  DrawLabel(c_PanelLeft, c_PanelLegendFirst, "filled: pull in standard errors");
  DrawLabel(c_PanelLeft, c_PanelLegendSecond, "open: score of a limit");
}


////////////////////////////////////////////////////////////////////////////////


//! Return the pages, in the order in which they were added
vector<TCanvas*> MEndToEndReport::GetPages() const
{
  vector<TCanvas*> Pages;
  for (const unique_ptr<TCanvas>& Page: m_Pages) {
    Pages.push_back(Page.get());
  }
  return Pages;
}


////////////////////////////////////////////////////////////////////////////////


//! Print the pages as one PDF (A4 landscape)
void MEndToEndReport::PrintPdf(const vector<TCanvas*>& Pages, const MString& FileName)
{
  gStyle->SetPaperSize(27.0, 19.1);
  // ROOT reports every page which it prints
  gErrorIgnoreLevel = kWarning;
  for (unsigned int p = 0; p < Pages.size(); ++p) {
    MString Option = FileName;
    if (Pages.size() > 1) {
      if (p == 0) {
        Option += "(";
      } else if (p + 1 == Pages.size()) {
        Option += ")";
      }
    }
    Pages[p]->Print(Option.Data(), "pdf");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write the title page, the key results, the pulls, and the pages of the test as PDF and as ROOT file with the canvases into the directory, nothing if there is nothing to write
void MEndToEndReport::Write(const MString& Geometry)
{
  // Nothing to write without a directory, key results, and pages
  if (m_Directory.IsEmpty() == true || (m_KeyResults.empty() == true && m_Pages.empty() == true)) {
    return;
  }
  if (MFile::CreateDirectory(m_Directory) == false) {
    merr<<"Unable to create the directory of the summary: "<<m_Directory<<show;
    return;
  }

  // The title page, the key results, and the pulls go in front of the pages of the test
  const unsigned int NumberOfTestPages = m_Pages.size();
  AddTitlePage(Geometry);
  AddKeyResultPages();
  AddPullsPage();
  rotate(m_Pages.begin(), m_Pages.begin() + NumberOfTestPages, m_Pages.end());

  WritePages();
  WriteKeyResults(m_Directory + "/" + m_TestName + ".keyresults.tsv");
  mout<<"The summary of the test is in "<<m_Directory<<"/"<<m_TestName<<".pdf"<<endl;

  // The pages go before the objects which they show
  m_Pages.clear();
  m_Objects.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the difference of a measured and an expected value in combined standard errors (10 if they differ without uncertainties, NaN if a value is not valid)
double MEndToEndReport::GetPull(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  // An infinite or NaN uncertainty must not hide a difference
  if (Measured.IsValid() == false || Expected.IsValid() == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  const double Sigma = sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
  if (Sigma > 0.0) {
    return (Measured.m_Value - Expected.m_Value)/Sigma;
  }
  if (Measured.m_Value == Expected.m_Value) {
    return 0.0;
  }
  return 10.0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the tolerance of a statistical comparison: the pull limit times the combined uncertainty, NaN if a value is not valid
double MEndToEndReport::GetStatisticalTolerance(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  if (Measured.IsValid() == false || Expected.IsValid() == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  return c_PullLimit*sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the comparison can be a statistical pull: the uncertainties are not zero and the tolerance is the pull limit times the combined uncertainty, if it was requested
bool MEndToEndReport::IsStatisticalComparison(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  if (IsStatistical == false || Measured.IsValid() == false || Expected.IsValid() == false) {
    return false;
  }

  // Without uncertainties the pull is zero or ten, whatever the difference
  const double Sigma = sqrt(Measured.m_Error*Measured.m_Error + Expected.m_Error*Expected.m_Error);
  if (Sigma <= 0.0) {
    return false;
  }

  // A tolerance which is not the pull limit (a systematic allowance, another number of sigma) is not seen by the pull
  const double RelativeMatch = 1e-6;
  if (fabs(Tolerance - c_PullLimit*Sigma) > RelativeMatch*Tolerance) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the score of a comparison: the pull if it is statistical, otherwise the difference in units of the tolerance divided by the pull limit, NaN if a value is not valid
double MEndToEndReport::GetScore(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  if (Measured.IsValid() == false || Expected.IsValid() == false || isfinite(Tolerance) == false) {
    return numeric_limits<double>::quiet_NaN();
  }
  if (IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical) == true) {
    return GetPull(Measured, Expected);
  }

  // The tolerance is the limit: the score is the pull limit at the limit like the pull
  const double Difference = Measured.m_Value - Expected.m_Value;
  if (Tolerance > 0.0) {
    return Difference*c_PullLimit/Tolerance;
  }
  if (Difference == 0.0) {
    return 0.0;
  }
  return 10.0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the ROOT color of a pull: dark green up to 1, light green to 2, yellow to 3, orange to 4, red above (predefined colors: the same in every process which reads the pages)
int MEndToEndReport::GetPullColor(double Pull)
{
  const double Size = fabs(Pull);
  if (Size <= 1.0) {
    return kGreen + 3;
  }
  if (Size <= 2.0) {
    return kGreen - 7;
  }
  if (Size <= 3.0) {
    return kYellow;
  }
  if (Size <= 4.0) {
    return kOrange + 1;
  }
  return kRed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the color of text on the color of the pull: white on dark green and red, black otherwise
int MEndToEndReport::GetPullTextColor(double Pull)
{
  const double Size = fabs(Pull);
  if (Size <= 1.0 || !(Size <= 4.0)) {
    return kWhite;
  }
  return kBlack;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the line "Geometry: path" for the title page, a path inside $MEGALIB is shown as ${MEGALIB}/...
MString MEndToEndReport::GetGeometryLine(const MString& Geometry)
{
  MString Path = Geometry;
  MString Home = "$(MEGALIB)";
  if (MFile::ExpandFileName(Home) == true) {
    const MString Prefix = Home + "/";
    if (Prefix.Length() > 1 && Path.BeginsWith(Prefix) == true) {
      Path.Replace(0, Prefix.Length() - 1, "${MEGALIB}");
    }
  }
  return MString("Geometry:  ") + Path;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the lines "Name: version" of MEGAlib, Geant4, ROOT, HEALPix, and cfitsio ("unknown" if a version cannot be determined), the temporary directory is for the output of pkg-config
vector<MString> MEndToEndReport::GetVersionLines() const
{
  vector<MString> Lines;
  Lines.push_back(MString("MEGAlib:   ") + ReadFirstLine("$(MEGALIB)/config/Version.txt"));
  Lines.push_back(MString("Geant4:    ") + ReadConfiguration("GEANT4"));
  Lines.push_back(MString("ROOT:      ") + gROOT->GetVersion());
  Lines.push_back(MString("HEALPix:   ") + GetPackageVersion("healpix_cxx"));
  Lines.push_back(MString("cfitsio:   ") + GetPackageVersion("cfitsio"));
  return Lines;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the text size (relative to the height of the pad, at most the maximum) with which the text fits into the width (NDC of the pad)
double MEndToEndReport::GetFittingTextSize(const TVirtualPad* Pad, const MString& Text, double Width, double MaximumSize)
{
  // A character of a proportional font has a fixed fraction of the text size as width
  const double PadWidth = Pad->GetWw()*Pad->GetAbsWNDC();
  const double PadHeight = Pad->GetWh()*Pad->GetAbsHNDC();
  const double Characters = max(1.0, static_cast<double>(Text.Length()));
  return min(MaximumSize, c_TextWidthFraction*Width*PadWidth/(Characters*c_CharacterWidth*PadHeight));
}


////////////////////////////////////////////////////////////////////////////////


//! Draw a box with a frame in NDC coordinates of the pad, the text shrinks to fit
void MEndToEndReport::DrawPanelBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double MaximumSize)
{
  TPaveText* Box = new TPaveText(Left, Bottom, Right, Top, "NDC");
  Box->SetBit(kCanDelete);
  Box->SetFillColor(0);
  Box->SetBorderSize(1);
  Box->SetTextFont(42);
  Box->SetTextSize(GetFittingTextSize(Pad, Text, Right - Left, MaximumSize));
  Box->SetTextAlign(12);
  Box->AddText(Text.Data());
  Box->Draw();
}


////////////////////////////////////////////////////////////////////////////////


//! Draw a box in NDC coordinates of the pad with the text on the color of the pull, the text shrinks to fit
void MEndToEndReport::DrawPullBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double Pull, double MaximumSize)
{
  TPaveText* Box = new TPaveText(Left, Bottom, Right, Top, "NDC");
  Box->SetBit(kCanDelete);
  Box->SetFillColor(GetPullColor(Pull));
  Box->SetBorderSize(0);
  Box->SetTextFont(42);
  Box->SetTextSize(GetFittingTextSize(Pad, Text, Right - Left, MaximumSize));
  Box->SetTextAlign(12);
  Box->SetTextColor(GetPullTextColor(Pull));
  Box->AddText(Text.Data());
  Box->Draw();
}


////////////////////////////////////////////////////////////////////////////////


//! Add the title page: date, version, geometry, number of comparisons, seeds
void MEndToEndReport::AddTitlePage(const MString& Geometry)
{
  unsigned int Compared = 0;
  unsigned int Failed = 0;
  for (const ETKeyResult& Result: m_KeyResults) {
    if (Result.m_HasReference == true || Result.m_Passed == false) {
      ++Compared;
      if (Result.m_Passed == false) {
        ++Failed;
      }
    }
  }
  vector<MString> Lines;
  ostringstream Time;
  const time_t Now = time(nullptr);
  Time<<put_time(localtime(&Now), "%Y-%m-%d %H:%M:%S");
  Lines.push_back(MString("Date:      ") + Time.str().c_str());
  for (const MString& Version: GetVersionLines()) {
    Lines.push_back(Version);
  }
  Lines.push_back(GetGeometryLine(Geometry));
  Lines.push_back("");
  if (Compared > 0) {
    Lines.push_back(MString("Compared with the reference or the expectation: ") + Compared + ", failed: " + Failed);
  }
  AddTextPages(m_TestName, Lines);
}


////////////////////////////////////////////////////////////////////////////////


//! Add the table of the key results, colored by pull
void MEndToEndReport::AddKeyResultPages()
{
  for (unsigned int Start = 0; Start < m_KeyResults.size(); Start += c_TableRowsPerPage) {
    TPad* Body = AddPage("Key results", 1, 1);
    Body->cd(1);
    TLatex Text;
    Text.SetNDC();
    Text.SetTextFont(82);
    Text.SetTextSize(c_TableTextSize);
    Text.SetTextColor(kBlack);
    Text.DrawLatex(c_ColumnName, c_TableHeaderY, "observable");
    Text.DrawLatex(c_ColumnMeasured, c_TableHeaderY, "measured");
    Text.DrawLatex(c_ColumnReference, c_TableHeaderY, "reference");
    Text.DrawLatex(c_ColumnPull, c_TableHeaderY, "score");
    Text.DrawLatex(c_ColumnResult, c_TableHeaderY, "result");
    for (unsigned int r = Start; r < m_KeyResults.size() && r < Start + c_TableRowsPerPage; ++r) {
      const ETKeyResult& Result = m_KeyResults[r];
      const double Y = c_TableTop - c_TablePitch*(r - Start);
      // The row has the color of its pull
      Text.SetTextColor(kBlack);
      if (Result.m_HasReference == true) {
        TBox Box;
        Box.SetFillColor(GetPullColor(Result.m_Pull));
        Box.DrawBox(0.0, Y - c_TableBandBelow, 1.0, Y + c_TableBandAbove);
        Text.SetTextColor(GetPullTextColor(Result.m_Pull));
      }
      MString Name = Result.m_Name;
      if (Name.Length() > c_TableNameLength) {
        Name = Name.GetSubString(0, c_TableNameLength - 3) + "...";
      }
      ostringstream Measured, Reference, Pull;
      Measured<<setprecision(4)<<Result.m_Value<<" +- "<<setprecision(2)<<Result.m_Error;
      Text.DrawLatex(c_ColumnName, Y, Name.Data());
      Text.DrawLatex(c_ColumnMeasured, Y, Measured.str().c_str());
      if (Result.m_HasReference == true) {
        Reference<<setprecision(4)<<Result.m_Reference<<" +- "<<setprecision(2)<<Result.m_ReferenceError;
        Pull<<fixed<<setprecision(1)<<Result.m_Pull;
        Text.DrawLatex(c_ColumnReference, Y, Reference.str().c_str());
        Text.DrawLatex(c_ColumnPull, Y, Pull.str().c_str());
      } else if (Result.m_Passed == false) {
        Text.DrawLatex(c_ColumnReference, Y, "no reference");
      }
      if (Result.m_IsValid == false) {
        Text.DrawLatex(c_ColumnResult, Y, "invalid");
      } else if (Result.m_Passed == false) {
        Text.DrawLatex(c_ColumnResult, Y, "FAILED");
      } else {
        Text.DrawLatex(c_ColumnResult, Y, "ok");
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write the key results as tab separated text: one line per key result, the first line names the columns
void MEndToEndReport::WriteKeyResults(const MString& FileName) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) {
    merr<<"Unable to write the key results: "<<FileName<<show;
    return;
  }
  Out<<"# name\tvalue\terror\thas_reference\treference\treference_error\tpull\ttolerance\tvalid\tstatistical\tpassed"<<endl;
  Out<<setprecision(17);
  for (const ETKeyResult& Result: m_KeyResults) {
    MString Name = Result.m_Name;
    Name.ReplaceAllInPlace("\t", " ");
    Out<<Name<<"\t"<<Result.m_Value<<"\t"<<Result.m_Error<<"\t"<<Result.m_HasReference<<"\t"<<Result.m_Reference<<"\t"<<Result.m_ReferenceError<<"\t"<<Result.m_Pull<<"\t"<<Result.m_Tolerance<<"\t"<<Result.m_IsValid<<"\t"<<Result.m_IsStatistical<<"\t"<<Result.m_Passed<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write the pages as PDF (A4 landscape) and into the ROOT file for the report of the test driver
void MEndToEndReport::WritePages() const
{
  const MString Base = m_Directory + "/" + m_TestName;
  for (unsigned int p = 0; p < m_Pages.size(); ++p) {
    m_Pages[p]->SetName(MString("SummaryPage") + (p + 1000));
  }
  PrintPdf(GetPages(), Base + ".pdf");
  TFile File((Base + ".summary.root").Data(), "RECREATE");
  if (File.IsZombie() == true) {
    merr<<"Unable to write the pages of the summary: "<<Base<<".summary.root"<<show;
    return;
  }
  for (const unique_ptr<TCanvas>& Page: m_Pages) {
    Page->Write();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the first line of a file, "unknown" if it cannot be read
MString MEndToEndReport::ReadFirstLine(const MString& FileName)
{
  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    return "unknown";
  }
  const vector<MString> Lines = Content.Tokenize("\n");
  if (Lines.empty() == true) {
    return "unknown";
  }
  return Lines[0];
}


////////////////////////////////////////////////////////////////////////////////


//! Return the value of the key in the configuration of the build (lines KEY=value), "unknown" if there is none
MString MEndToEndReport::ReadConfiguration(const MString& Key)
{
  MString Content;
  if (MFile::ReadTextFile("$(MEGALIB)/config/Configuration.txt", Content) == false) {
    return "unknown";
  }
  const MString Start = Key + "=";
  for (const MString& Line: Content.Tokenize("\n")) {
    if (Line.BeginsWith(Start) == true && Line.Length() > Start.Length()) {
      return Line.GetSubString(Start.Length());
    }
  }
  return "unknown";
}


////////////////////////////////////////////////////////////////////////////////


//! Return the version of a package which pkg-config knows, "unknown" if it does not
MString MEndToEndReport::GetPackageVersion(const MString& Package) const
{
  // The output goes into a file which belongs to this test: tests which run at the same time do not share it
  const MString FileName = m_Directory + "/." + m_TestName + "_" + Package + ".version";
  MString Version = "unknown";
  if (MSystem::RunProcess("pkg-config", MString("--modversion ") + Package, FileName, "", c_VersionTimeOut) == 0) {
    Version = ReadFirstLine(FileName);
  }
  MFile::Remove(FileName);
  return Version;
}


////////////////////////////////////////////////////////////////////////////////


// MEndToEndReport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
