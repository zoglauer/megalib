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
#include "MProcessManager.h"
#include "MSystem.h"


//! The number of pages created so far by all reports
unsigned int MEndToEndReport::m_NumberOfCreatedPages = 0;


////////////////////////////////////////////////////////////////////////////////


//! Standard constructor giving the name of the test
MEndToEndReport::MEndToEndReport(const MString& TestName) : m_TestName(TestName)
{
  // Intentionally left blank
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


//! Create a histogram of values, draw it into the current pad with the options, return it
TH1D* MEndToEndReport::DrawHistogram(const MString& Title, const vector<double>& Values, unsigned int Bins, double Minimum, double Maximum, const MString& Options)
{
  TH1D* Histogram = new TH1D("", Title.Data(), Bins, Minimum, Maximum);
  for (double Value: Values) {
    Histogram->Fill(Value);
  }
  Histogram->SetStats(false);
  Histogram->SetLineColor(kBlack);
  Histogram->SetMaximum(1.4*Histogram->GetMaximum());
  // The page owns a copy (ROOT takes it out of the current directory and lets the pad delete it), the original goes
  TH1D* Drawn = static_cast<TH1D*>(Histogram->DrawCopy(Options.Data()));
  delete Histogram;
  return Drawn;
}


////////////////////////////////////////////////////////////////////////////////


//! Write a row into the current pad with the measured and the expected value and their pull, on the color of the pull (rows count from the top)
void MEndToEndReport::DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected)
{
  DrawComparison(Row, Label, Measured, Expected, MEndToEndResultList::GetStatisticalTolerance(Measured, Expected), true);
}


////////////////////////////////////////////////////////////////////////////////


//! Write a row for a comparison with a tolerance: as above, the score is the one of AddComparison
void MEndToEndReport::DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical)
{
  const double Pull = MEndToEndResultList::GetScore(Measured, Expected, Tolerance, IsStatistical);
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
  const double Score = MEndToEndResultList::GetScore(Measured, Expected, Tolerance, IsStatistical);
  ostringstream Text;
  Text<<Label.Data()<<" "<<setprecision(4)<<Measured.m_Value<<"#pm"<<setprecision(2)<<Measured.m_Error<<", expected "<<setprecision(4)<<Expected.m_Value;
  if (MEndToEndResultList::IsStatisticalComparison(Measured, Expected, Tolerance, IsStatistical) == true) {
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
void MEndToEndReport::AddPullsPage(const MEndToEndResultList& Results)
{
  // Only valid comparisons have a score which can be shown, the invalid ones fail in the table of the key results
  vector<const MEndToEndResult*> Shown;
  for (const MEndToEndResult& Result: Results.GetResults()) {
    if (Result.HasReference() == true && Result.IsValid() == true) {
      Shown.push_back(&Result);
    }
  }
  if (Shown.empty() == true) {
    return;
  }

  TPad* Body = AddPage("Pulls and scores of the comparisons", 1, 1);
  Body->cd(1);
  gPad->SetRightMargin(c_PanelMargin);
  TH1D* Frame = new TH1D("", ";comparison;pull (#sigma) or score", Shown.size(), 0.5, Shown.size() + 0.5);
  Frame->SetStats(false);
  Frame->SetMinimum(-c_PullAxisLimit);
  Frame->SetMaximum(c_PullAxisLimit);
  Frame->GetYaxis()->SetNdivisions(static_cast<int>(2*c_PullAxisLimit), kFALSE);
  // The page owns a copy of the frame, the original goes
  Frame->DrawCopy("AXIS");
  delete Frame;
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
  // The page owns the graphs: a graph has no copy which is drawn, it gets the flag of the pad itself
  TGraph* Graph = new TGraph();
  Graph->SetBit(kCanDelete);
  TGraph* ScoreGraph = new TGraph();
  ScoreGraph->SetBit(kCanDelete);
  for (unsigned int p = 0; p < Shown.size(); ++p) {
    TGraph* Target = ScoreGraph;
    if (Shown[p]->IsStatistical() == true) {
      Target = Graph;
    }
    Target->SetPoint(Target->GetN(), p + 1, max(-c_PullEdge, min(c_PullEdge, Shown[p]->GetPull())));
  }
  // ROOT does not draw a graph without points - a graph which is not drawn belongs to nobody and is deleted here
  Graph->SetMarkerStyle(20);
  Graph->SetMarkerSize(0.8);
  if (Graph->GetN() > 0) {
    Graph->Draw("P SAME");
  } else {
    delete Graph;
  }
  ScoreGraph->SetMarkerStyle(24);
  ScoreGraph->SetMarkerSize(0.8);
  if (ScoreGraph->GetN() > 0) {
    ScoreGraph->Draw("P SAME");
  } else {
    delete ScoreGraph;
  }
  for (double Limit: { -MEndToEndResultList::c_PullLimit, MEndToEndResultList::c_PullLimit }) {
    TLine* Line = new TLine(0.5, Limit, Shown.size() + 0.5, Limit);
    Line->SetLineColor(kRed);
    Line->SetLineStyle(2);
    Line->Draw();
  }

  // The counts of the pulls which are inside one, two, and three sigma next to the Gaussian fractions
  const ETPullStatistics Statistics = Results.GetPullStatistics();
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


//! Print the pages as one PDF (US letter landscape), return false if the file was not written
bool MEndToEndReport::PrintPdf(const vector<TCanvas*>& Pages, const MString& FileName)
{
  // ROOT scales the canvas to fit the paper and caps the height of the page: this size keeps the 11 x 8.5 proportions of US letter
  gStyle->SetPaperSize(25.0, 19.3);
  // ROOT only reports that it cannot open the file: a file of an earlier print must not look like the file of this one
  if (MFile::Exists(FileName) == true) {
    MFile::Remove(FileName);
  }
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
  return MFile::Exists(FileName);
}


////////////////////////////////////////////////////////////////////////////////


//! Write the title page, the key results, the pulls, and the pages of the test as PDF and as ROOT file with the canvases into the directory, nothing if there is nothing to write
void MEndToEndReport::Write(const MString& Geometry, const MEndToEndResultList& Results)
{
  // Nothing to write without a directory, key results, and pages
  if (m_Directory.IsEmpty() == true || (Results.IsEmpty() == true && m_Pages.empty() == true)) {
    return;
  }
  if (MFile::CreateDirectory(m_Directory) == false) {
    merr<<"Unable to create the directory of the summary: "<<m_Directory<<show;
    return;
  }

  // The title page, the key results, and the pulls go in front of the pages of the test
  const unsigned int NumberOfTestPages = m_Pages.size();
  AddTitlePage(Geometry, Results);
  AddKeyResultPages(Results);
  AddPullsPage(Results);
  rotate(m_Pages.begin(), m_Pages.begin() + NumberOfTestPages, m_Pages.end());

  WritePages();
  mout<<"The summary of the test is in "<<m_Directory<<"/"<<m_TestName<<".pdf"<<endl;

  m_Pages.clear();
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
void MEndToEndReport::AddTitlePage(const MString& Geometry, const MEndToEndResultList& Results)
{
  unsigned int Compared = 0;
  unsigned int Failed = 0;
  for (const MEndToEndResult& Result: Results.GetResults()) {
    if (Result.HasReference() == true || Result.IsPassed() == false) {
      ++Compared;
      if (Result.IsPassed() == false) {
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
  for (const MString& Line: m_TitleLines) {
    Lines.push_back(Line);
  }
  Lines.push_back("");
  if (Compared > 0) {
    Lines.push_back(MString("Compared with the reference or the expectation: ") + Compared + ", failed: " + Failed);
  }
  AddTextPages(m_TestName, Lines);
}


////////////////////////////////////////////////////////////////////////////////


//! Add the table of the key results, colored by pull
void MEndToEndReport::AddKeyResultPages(const MEndToEndResultList& Results)
{
  const deque<MEndToEndResult>& KeyResults = Results.GetResults();
  for (unsigned int Start = 0; Start < KeyResults.size(); Start += c_TableRowsPerPage) {
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
    for (unsigned int r = Start; r < KeyResults.size() && r < Start + c_TableRowsPerPage; ++r) {
      const MEndToEndResult& Result = KeyResults[r];
      const double Y = c_TableTop - c_TablePitch*(r - Start);
      // The row has the color of its pull
      Text.SetTextColor(kBlack);
      if (Result.HasReference() == true) {
        TBox Box;
        Box.SetFillColor(GetPullColor(Result.GetPull()));
        Box.DrawBox(0.0, Y - c_TableBandBelow, 1.0, Y + c_TableBandAbove);
        Text.SetTextColor(GetPullTextColor(Result.GetPull()));
      }
      MString Name = Result.GetName();
      if (Name.Length() > c_TableNameLength) {
        Name = Name.GetSubString(0, c_TableNameLength - 3) + "...";
      }
      ostringstream Measured, Reference, Pull;
      Measured<<setprecision(4)<<Result.GetValue()<<" +- "<<setprecision(2)<<Result.GetError();
      Text.DrawLatex(c_ColumnName, Y, Name.Data());
      Text.DrawLatex(c_ColumnMeasured, Y, Measured.str().c_str());
      if (Result.HasReference() == true) {
        Reference<<setprecision(4)<<Result.GetReference()<<" +- "<<setprecision(2)<<Result.GetReferenceError();
        Pull<<fixed<<setprecision(1)<<Result.GetPull();
        Text.DrawLatex(c_ColumnReference, Y, Reference.str().c_str());
        Text.DrawLatex(c_ColumnPull, Y, Pull.str().c_str());
      } else if (Result.IsPassed() == false) {
        Text.DrawLatex(c_ColumnReference, Y, "no reference");
      }
      if (Result.IsValid() == false) {
        Text.DrawLatex(c_ColumnResult, Y, "invalid");
      } else if (Result.IsPassed() == false) {
        Text.DrawLatex(c_ColumnResult, Y, "FAILED");
      } else {
        Text.DrawLatex(c_ColumnResult, Y, "ok");
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write the pages as PDF (US letter landscape) and into the ROOT file for the report of the test driver
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


//! Return the version of a package which pkg-config knows, "unknown" if it does not
MString MEndToEndReport::GetPackageVersion(const MString& Package) const
{
  // The output goes into a file which belongs to this test: tests which run at the same time do not share it
  const MString FileName = m_Directory + "/." + m_TestName + "_" + Package + ".version";
  MString Version = "unknown";
  if (MProcessManager::Run("pkg-config", MString("--modversion ") + Package, FileName, "", c_VersionTimeOut) == 0) {
    Version = ReadFirstLine(FileName);
  }
  MFile::Remove(FileName);
  return Version;
}


// MEndToEndReport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
