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
#include "MEndToEndResultList.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The summary of an end-to-end test: key results and pages, written as PDF and as ROOT file with the canvases (for the report of the test driver)
//! * Pages are ROOT canvases of US letter landscape format with a header (title and test name) and a body pad which is divided into columns x rows
//! * The drawing functions draw into the current pad (gPad), the usual ROOT convention: cd() into the pad of the body first
//! * Write() adds the title page, the key results, and the pulls in front of the pages of the test
class MEndToEndReport
{
  // public interface:
 public:
  //! Standard constructor giving the name of the test, which is on every page and the name of the files
  MEndToEndReport(const MString& TestName);
  //! Default destructor
  ~MEndToEndReport() = default;

  //! Set the directory in which Write() writes the files
  void SetDirectory(const MString& Directory) { m_Directory = Directory; }
  //! Return the directory in which Write() writes the files
  const MString& GetDirectory() const { return m_Directory; }
  
  //! Add a line to the title page below the default version and geometry list
  void AddTitleLine(const MString& Line) { m_TitleLines.push_back(Line); }
  //! Return the lines which were added to the title page
  const vector<MString>& GetTitleLines() const { return m_TitleLines; }

  
  // Pages:
  
  //! Add a page with a title and a pad which is divided into columns x rows, return the pad: draw into pad->cd(1 ... columns*rows)
  TPad* AddPage(const MString& Title, unsigned int Columns, unsigned int Rows);
  //! Add pages with lines of text
  void AddTextPages(const MString& Title, const vector<MString>& Lines);
  //! Create a histogram of values, draw it into the current pad with the options, return it
  TH1D* DrawHistogram(const MString& Title, const vector<double>& Values, unsigned int Bins, double Minimum, double Maximum, const MString& Options = "HIST");
  //! Write a row into the current pad with the measured and the expected value and their pull, on the color of the pull (rows count from the top)
  static void DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected);
  //! Write a row for a comparison with a tolerance: as above, the score is the one of AddComparison
  static void DrawComparison(unsigned int Row, const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Return the text of a row of a comparison: a statistical pull is in sigma, the score of a comparison with a limit has no unit
  static MString GetComparisonText(const MString& Label, const ETMeasuredValue& Measured, const ETMeasuredValue& Expected, double Tolerance, bool IsStatistical);
  //! Write a text line into the current pad: NDC coordinates, color index of ROOT
  static void DrawLabel(double X, double Y, const MString& Text, int Color = 1);
  //! Add the page with the pulls of all comparisons of the results and their statistics (nothing without comparisons)
  void AddPullsPage(const MEndToEndResultList& Results);
  //! Return the pages, in the order in which they were added
  vector<TCanvas*> GetPages() const;
  
  //! The width of a page in pixels (US letter landscape: 11 inch at 96 dpi)
  static constexpr unsigned int c_PageWidth = 1056;
  //! The height of a page in pixels: ROOT takes 4 pixels of the width and 28 pixels of the height for the frame of the canvas, also in batch mode, so that the PDF has the 11 x 8.5 proportions of US letter
  static constexpr unsigned int c_PageHeight = 841;
  //! Print the pages as one PDF (US letter landscape), return false if the file was not written
  static bool PrintPdf(const vector<TCanvas*>& Pages, const MString& FileName);


  // Output:

  //! Write the title page, the key results, the pulls, and the pages of the test as PDF and as ROOT file with the canvases into the directory, nothing if there is nothing to write
  void Write(const MString& Geometry, const MEndToEndResultList& Results);


  // Helpers:

  //! Return the ROOT color of a pull: dark green up to 1, light green to 2, yellow to 3, orange to 4, red above (predefined colors: the same in every process which reads the pages)
  static int GetPullColor(double Pull);
  //! Return the color of text on the color of the pull: white on dark green and red, black otherwise
  static int GetPullTextColor(double Pull);
  //! Return the line "Geometry: path" for the title page, a path inside $MEGALIB is shown as ${MEGALIB}/...
  static MString GetGeometryLine(const MString& Geometry);
  //! Return the lines "Name: version" of MEGAlib, Geant4, ROOT, HEALPix, and cfitsio ("unknown" if a version cannot be determined)
  vector<MString> GetVersionLines() const;

  //! Return the first line of a file, "unknown" if it cannot be read
  static MString ReadFirstLine(const MString& FileName);
  //! Return the value of the key in the configuration of the build (lines KEY=value), "unknown" if there is none
  static MString ReadConfiguration(const MString& Key);

  // private methods:
 private:
  //! Return the text size (relative to the height of the pad, at most the maximum) with which the text fits into the width (NDC of the pad)
  static double GetFittingTextSize(const TVirtualPad* Pad, const MString& Text, double Width, double MaximumSize);
  //! Draw a box with a frame in NDC coordinates of the pad, the text shrinks to fit
  static void DrawPanelBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double MaximumSize);
  //! Draw a box in NDC coordinates of the pad with the text on the color of the pull, the text shrinks to fit
  static void DrawPullBox(TVirtualPad* Pad, double Left, double Bottom, double Right, double Top, const MString& Text, double Pull, double MaximumSize);
  //! Add the title page: date, versions, geometry, number of comparisons
  void AddTitlePage(const MString& Geometry, const MEndToEndResultList& Results);
  //! Add the table of the key results, colored by pull
  void AddKeyResultPages(const MEndToEndResultList& Results);
  //! Write the pages as PDF and into the ROOT file
  void WritePages() const;
  //! Return the version of a package which pkg-config knows, "unknown" if it does not
  MString GetPackageVersion(const MString& Package) const;

  // private members:
 private:
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
  //! The lines which are added to the title page
  vector<MString> m_TitleLines;
  //! The number of pages created so far by all reports: ROOT deletes a canvas when another canvas gets its name, every canvas needs its own name
  static unsigned int m_NumberOfCreatedPages;
  //! The pages
  vector<unique_ptr<TCanvas>> m_Pages;
};

#endif


////////////////////////////////////////////////////////////////////////////////
