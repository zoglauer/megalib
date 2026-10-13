/*
 * MEndToEndReport.h
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

#ifndef __MEndToEndReport__
#define __MEndToEndReport__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <array>
#include <cmath>
#include <memory>
#include <vector>
using namespace std;

// ROOT libs:
#include "TCanvas.h"
#include "TH1.h"
#include "TObject.h"
#include "TPad.h"
#include "TVirtualPad.h"

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A measured value with its uncertainty
class ETMeasuredValue
{
public:
  //! The value
  double m_Value = 0.0;
  //! The uncertainty (standard error)
  double m_Error = 0.0;

  //! Return true if the value and the uncertainty are finite and the uncertainty is not negative
  bool IsValid() const
  {
    if (isfinite(m_Value) == false || isfinite(m_Error) == false || m_Error < 0.0) {
      return false;
    }
    return true;
  }
};


//! A key result of a test for the summary: a measured value, and, if there is one, its reference
class ETKeyResult
{
public:
  //! The name: scenario and observable
  MString m_Name;
  //! The measured value
  double m_Value = 0.0;
  //! The uncertainty of the measured value
  double m_Error = 0.0;
  //! True if there is a reference
  bool m_HasReference = false;
  //! The reference value
  double m_Reference = 0.0;
  //! The uncertainty of the reference
  double m_ReferenceError = 0.0;
  //! The score of the comparison: the difference to the reference in combined standard errors for a statistical comparison, in units of the tolerance divided by the pull limit otherwise (zero without a reference, NaN if invalid)
  double m_Pull = 0.0;
  //! The largest difference to the reference which the test accepts
  double m_Tolerance = 0.0;
  //! True if the values and their uncertainties are finite (a comparison which is not valid fails)
  bool m_IsValid = true;
  //! True if the score is the difference in standard errors, which follows a Gaussian for statistical fluctuations alone: only these scores are part of the statistics of the pulls
  bool m_IsStatistical = true;
  //! True if the value is inside the tolerance of the reference: the outcome of the check of the test
  bool m_Passed = true;
};


//! The distribution of the pulls of all comparisons: are they distributed as expected for statistical fluctuations?
class ETPullStatistics
{
public:
  //! The fractions inside one, two, and three sigma which are expected for Gaussian pulls
  static constexpr array<double, 3> c_GaussianFractions = { 0.6827, 0.9545, 0.9973 };

  //! The number of pulls
  unsigned int m_NumberOfPulls = 0;
  //! The number of pulls with |pull| <= 1, 2, 3
  vector<unsigned int> m_Inside = { 0, 0, 0 };

  //! Return the fraction of the pulls inside the given number of sigma (1, 2, or 3), the uncertainty is zero
  ETMeasuredValue GetFraction(unsigned int Sigma) const
  {
    ETMeasuredValue Fraction;
    if (Sigma >= 1 && Sigma <= 3 && m_NumberOfPulls > 0) {
      Fraction.m_Value = static_cast<double>(m_Inside[Sigma - 1])/m_NumberOfPulls;
    }
    return Fraction;
  }
};


////////////////////////////////////////////////////////////////////////////////


//! The summary of an end-to-end test: key results and pages, written as PDF and as ROOT file with the canvases (for the report of the test driver)
//! * Pages are ROOT canvases of A4 landscape format with a header (title and test name) and a body pad which is divided into columns x rows
//! * The drawing functions draw into the current pad (gPad), the usual ROOT convention: cd() into the pad of the body first
//! * Write() adds the title page, the key results, and the pulls in front of the pages of the test
class MEndToEndReport
{
  // public interface:
 public:
  //! The comparisons are inside the tolerance if the pull is not larger than this number of combined standard errors
  static constexpr double c_PullLimit = 5.0;

  //! Standard constructor giving the name of the test, which is on every page and the name of the files
  MEndToEndReport(const MString& TestName);
  //! Default destructor
  ~MEndToEndReport() = default;

  //! Set the directory in which Write() writes the files
  void SetDirectory(const MString& Directory) { m_Directory = Directory; }

  // Key results:
  //! Add a key result: name, value, uncertainty, return it (valid until the next key result is added)
  ETKeyResult& AddKeyResult(const MString& Name, double Value, double Error);
  //! Add a key result which was measured before (e.g. read from a file), return it (valid until the next key result is added)
  ETKeyResult& AddKeyResult(const ETKeyResult& Result);
  //! Add a comparison to the key results with the tolerance (the largest accepted difference) and the kind of score (statistical: in standard errors, otherwise in units of the tolerance divided by the pull limit), return it (valid until the next key result is added)
  ETKeyResult& AddComparison(const MString& Name, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the key results
  const vector<ETKeyResult>& GetKeyResults() const { return m_KeyResults; }
  //! Return the statistics of the pulls of all valid statistical comparisons (comparisons without uncertainties and with a tolerance which is not the pull limit times the uncertainty are scored against their tolerance)
  ETPullStatistics GetPullStatistics() const;
  //! Read the key results which Write() stored in the file (empty if the file cannot be read)
  static vector<ETKeyResult> ReadKeyResults(const MString& FileName);

  // Pages:
  //! Add a page with a title and a pad which is divided into columns x rows, return the pad: draw into pad->cd(1 ... columns*rows)
  TPad* AddPage(const MString& Title, unsigned int Columns, unsigned int Rows);
  //! Add pages with lines of text
  void AddTextPages(const MString& Title, const vector<MString>& Lines);
  //! Keep an object which is drawn on a page until the summary is written, return it
  template<class T>
  T* KeepObject(unique_ptr<T> Object)
  {
    T* Raw = Object.get();
    m_Objects.push_back(unique_ptr<TObject>(Object.release()));
    return Raw;
  }
  //! Create a histogram of values (owned by the report), draw it into the current pad with the options, return it
  TH1D* DrawHistogram(const MString& Title, const vector<double>& Values, unsigned int Bins, double Minimum, double Maximum, const MString& Options = "HIST");
  //! Write a row into the current pad with the measured and the expected value and their pull, on the color of the pull (rows count from the top)
  static void DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Write a row for a comparison with a tolerance: as above, the score is the one of AddComparison
  static void DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the text of a row of a comparison: a statistical pull is in sigma, the score of a comparison with a limit has no unit
  static MString GetComparisonText(const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Write a text line into the current pad: NDC coordinates, color index of ROOT
  static void DrawLabel(double X, double Y, const MString& Text, int Color = 1);
  //! Add the page with the pulls of all comparisons and their statistics (nothing without comparisons)
  void AddPullsPage();
  //! Return the pages, in the order in which they were added
  vector<TCanvas*> GetPages() const;
  //! Print the pages as one PDF (A4 landscape)
  static void PrintPdf(const vector<TCanvas*>& Pages, const MString& FileName);

  // Output:
  //! Write the title page, the key results, the pulls, and the pages of the test as PDF, as ROOT file with the canvases, and the key results as tab separated text into the directory, nothing if there is nothing to write
  void Write(const MString& Geometry);

  // Helpers:
  //! Return the difference of a measured and an expected value in combined standard errors (10 if they differ without uncertainties, NaN if a value is not valid)
  static double GetPull(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Return the tolerance of a statistical comparison: the pull limit times the combined uncertainty, NaN if a value is not valid
  static double GetStatisticalTolerance(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Return true if the comparison can be a statistical pull: the uncertainties are not zero and the tolerance is the pull limit times the combined uncertainty, if it was requested
  static bool IsStatisticalComparison(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the score of a comparison: the pull if it is statistical, otherwise the difference in units of the tolerance divided by the pull limit, NaN if a value is not valid
  static double GetScore(const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the ROOT color of a pull: dark green up to 1, light green to 2, yellow to 3, orange to 4, red above (predefined colors: the same in every process which reads the pages)
  static int GetPullColor(double Pull);
  //! Return the color of text on the color of the pull: white on dark green and red, black otherwise
  static int GetPullTextColor(double Pull);
  //! Return the line "Geometry: path" for the title page, a path inside $MEGALIB is shown as ${MEGALIB}/...
  static MString GetGeometryLine(const MString& Geometry);
  //! Return the lines "Name: version" of MEGAlib, Geant4, ROOT, HEALPix, and cfitsio ("unknown" if a version cannot be determined)
  vector<MString> GetVersionLines() const;

  // private methods:
 private:
  //! Return the text size (relative to the height of the pad, at most the maximum) with which the text fits into the width (NDC of the pad)
  static double GetFittingTextSize(const TVirtualPad* Pad, const MString& Text, double Width, double MaximumSize);
  //! Draw a box with a frame in NDC coordinates of the pad, the text shrinks to fit
  static void DrawPanelBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double MaximumSize);
  //! Draw a box in NDC coordinates of the pad with the text on the color of the pull, the text shrinks to fit
  static void DrawPullBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double Pull, double MaximumSize);
  //! Add the title page: date, versions, geometry, number of comparisons
  void AddTitlePage(const MString& Geometry);
  //! Add the table of the key results, colored by pull
  void AddKeyResultPages();
  //! Write the key results as tab separated text
  void WriteKeyResults(const MString& FileName) const;
  //! Write the pages as PDF and into the ROOT file
  void WritePages() const;
  //! Return the first line of a file, "unknown" if it cannot be read
  static MString ReadFirstLine(const MString& FileName);
  //! Return the value of the key in the configuration of the build (lines KEY=value), "unknown" if there is none
  static MString ReadConfiguration(const MString& Key);
  //! Return the version of a package which pkg-config knows, "unknown" if it does not
  MString GetPackageVersion(const MString& Package) const;

  // private members:
 private:
  //! The width of a page in pixels (A4 landscape)
  static constexpr unsigned int c_PageWidth = 1123;
  //! The height of a page in pixels (A4 landscape)
  static constexpr unsigned int c_PageHeight = 794;
  //! The body of a page is below this height (NDC of the page), the header above
  static constexpr double c_BodyTop = 0.93;
  //! The left edge of the texts in the header (NDC of the header)
  static constexpr double c_HeaderLeft = 0.015;
  //! The height of the title in the header (NDC of the header)
  static constexpr double c_HeaderTitleY = 0.5;
  //! The size of the title in the header (NDC of the header)
  static constexpr double c_HeaderTitleSize = 0.4;
  //! The height of the test name in the header (NDC of the header)
  static constexpr double c_HeaderTestY = 0.1;
  //! The size of the test name in the header (NDC of the header)
  static constexpr double c_HeaderTestSize = 0.25;

  //! The distance of the rows of comparisons in a histogram (NDC of the pad)
  static constexpr double c_RowPitch = 0.075;
  //! The height of a row of a comparison (NDC of the pad)
  static constexpr double c_RowHeight = 0.07;
  //! The maximum text size of rows and labels (NDC of the pad)
  static constexpr double c_RowTextSize = 0.045;
  //! The width of a character of a proportional font relative to the text size
  static constexpr double c_CharacterWidth = 0.45;
  //! The fraction of the width which the text may use
  static constexpr double c_TextWidthFraction = 0.95;

  //! The number of lines of text of a page
  static constexpr unsigned int c_TextLinesPerPage = 28;
  //! The left edge of the lines of text (NDC of the pad)
  static constexpr double c_TextLeft = 0.03;
  //! The height of the first line of text (NDC of the pad)
  static constexpr double c_TextTop = 0.95;
  //! The distance between the lines of text (NDC of the pad)
  static constexpr double c_TextPitch = 0.033;
  //! The size of the text of text pages
  static constexpr double c_TextSize = 0.028;

  //! The number of rows of the table of the key results on a page
  static constexpr unsigned int c_TableRowsPerPage = 30;
  //! The column of the names (NDC of the pad)
  static constexpr double c_ColumnName = 0.01;
  //! The column of the measured values (NDC of the pad)
  static constexpr double c_ColumnMeasured = 0.44;
  //! The column of the reference values (NDC of the pad)
  static constexpr double c_ColumnReference = 0.68;
  //! The column of the pulls (NDC of the pad)
  static constexpr double c_ColumnPull = 0.82;
  //! The column of the results (NDC of the pad)
  static constexpr double c_ColumnResult = 0.91;
  //! The height of the header of the table (NDC of the pad)
  static constexpr double c_TableHeaderY = 0.96;
  //! The height of the first row of the table (NDC of the pad)
  static constexpr double c_TableTop = 0.925;
  //! The distance of the rows of the table (NDC of the pad)
  static constexpr double c_TablePitch = 0.0305;
  //! The colored band of a row starts this far below the text (NDC of the pad)
  static constexpr double c_TableBandBelow = 0.009;
  //! The colored band of a row ends this far above the text (NDC of the pad)
  static constexpr double c_TableBandAbove = 0.0215;
  //! The text size of the table
  static constexpr double c_TableTextSize = 0.02;
  //! The names in the table are shortened to this number of characters
  static constexpr unsigned int c_TableNameLength = 46;

  //! The range of the pulls on the pulls page
  static constexpr double c_PullAxisLimit = 8.0;
  //! The pulls beyond this value are drawn at the edge of the pulls page
  static constexpr double c_PullEdge = 7.9;
  //! The horizontal grid lines of the pulls page are at every integer up to this value
  static constexpr int c_PullGridMaximum = 7;
  //! The right margin of the pulls page which makes room for the statistics (NDC of the pad)
  static constexpr double c_PanelMargin = 0.38;
  //! The left edge of the statistics panel (NDC of the pad)
  static constexpr double c_PanelLeft = 0.64;
  //! The right edge of the statistics panel (NDC of the pad)
  static constexpr double c_PanelRight = 0.99;
  //! The bottom of the title of the statistics panel (NDC of the pad)
  static constexpr double c_PanelTitleBottom = 0.87;
  //! The top of the title of the statistics panel (NDC of the pad)
  static constexpr double c_PanelTitleTop = 0.95;
  //! The top of the first row of the statistics panel (NDC of the pad)
  static constexpr double c_PanelRowTop = 0.85;
  //! The height of the rows of the statistics panel (NDC of the pad)
  static constexpr double c_PanelRowHeight = 0.07;
  //! The distance of the rows of the statistics panel (NDC of the pad)
  static constexpr double c_PanelRowPitch = 0.085;
  //! The bottom of the note below the rows of the statistics panel (NDC of the pad)
  static constexpr double c_PanelNoteBottom = 0.50;
  //! The top of the note below the rows of the statistics panel (NDC of the pad)
  static constexpr double c_PanelNoteTop = 0.57;
  //! The height of the first line of the legend below the statistics panel (NDC of the pad)
  static constexpr double c_PanelLegendFirst = 0.40;
  //! The height of the second line of the legend below the statistics panel (NDC of the pad)
  static constexpr double c_PanelLegendSecond = 0.35;
  //! The maximum text size in the statistics panel
  static constexpr double c_PanelTextSize = 0.03;

  //! The time out of the version queries in seconds
  static constexpr unsigned int c_VersionTimeOut = 10;

  //! The name of the test
  MString m_TestName;
  //! The directory of the files
  MString m_Directory;
  //! The key results
  vector<ETKeyResult> m_KeyResults;
  //! The number of pages created so far: every canvas needs its own name
  unsigned int m_NumberOfCreatedPages = 0;
  //! The objects which are drawn on the pages (declared before the pages: the pages are deleted first)
  vector<unique_ptr<TObject>> m_Objects;
  //! The pages
  vector<unique_ptr<TCanvas>> m_Pages;
};

#endif


////////////////////////////////////////////////////////////////////////////////
