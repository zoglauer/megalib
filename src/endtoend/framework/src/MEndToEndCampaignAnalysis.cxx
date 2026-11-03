/*
 * MEndToEndCampaignAnalysis.cxx
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
#include "MEndToEndCampaignAnalysis.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TF1.h"
#include "TLine.h"
#include "TMath.h"

// MEGAlib libs:
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MStatistics.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MEndToEndCampaignAnalysis::MEndToEndCampaignAnalysis() : m_MinimumRuns(30)
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Read the key results and the environment of a run of the test which are in the directory, return false if the key results cannot be read
bool MEndToEndCampaignAnalysis::ReadRun(const MString& Directory, const MString& TestName, unsigned int FailedChecks, ETCampaignRun& Run)
{
  Run = ETCampaignRun();
  Run.m_FailedChecks = FailedChecks;

  if (Run.m_Results.Read(Directory + "/" + TestName + ".keyresults.tsv") == false || Run.m_Results.IsEmpty() == true) {
    merr<<"The run has no key results, or they are not readable: "<<Directory<<"/"<<TestName<<".keyresults.tsv"<<show;
    return false;
  }

  // The environment is the header of the file of the fingerprint, a missing file is a finding of the analysis
  MEndToEndRecord Environment;
  if (MFile::Exists(Directory + "/" + TestName + ".fingerprint.tsv") == true && Environment.Read(Directory + "/" + TestName + ".fingerprint.tsv") == true) {
    for (const MString& Key: MEndToEndFingerprint::GetKeys()) {
      Run.m_Fingerprint[Key] = Environment.Get(Key);
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the runs: the observables, the findings, and the leave-one-out validation
void MEndToEndCampaignAnalysis::Analyze()
{
  m_Observables.clear();
  m_Findings.clear();
  m_Coverage = ETCampaignCoverage();
  m_Fingerprint.clear();

  CheckRuns();
  CollectObservables();
  CalculateStatistics();
  ValidateLeaveOneOut();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if there is a finding which is an error
bool MEndToEndCampaignAnalysis::HasErrors() const
{
  for (const ETCampaignFinding& Finding: m_Findings) {
    if (Finding.m_Severity == MCampaignSeverity::c_Error) {
      return true;
    }
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the report of the analysis as PDF: the findings, the table of the observables, the leave-one-out validation, and the ratios of scatter and bootstrap error, return false if the file cannot be written
bool MEndToEndCampaignAnalysis::WriteReport(const MString& TestName, const MString& FileName) const
{
  MString Expanded = FileName;
  if (MFile::ExpandFileName(Expanded) == false || MFile::CreateDirectory(MFile::GetDirectoryName(Expanded)) == false) {
    merr<<"Unable to create the directory of the report of the campaign: "<<FileName<<show;
    return false;
  }

  MEndToEndReport Report(TestName);

  // The summary and the findings
  unsigned int NumberOfErrors = 0;
  for (const ETCampaignFinding& Finding: m_Findings) {
    if (Finding.m_Severity == MCampaignSeverity::c_Error) {
      ++NumberOfErrors;
    }
  }
  const unsigned int LineLength = 105;
  vector<MString> Lines;
  Lines.push_back(MString("Runs:         ") + GetNumberOfRuns() + " (at least " + m_MinimumRuns + " are needed)");
  Lines.push_back(MString("Observables:  ") + m_Observables.size());
  Lines.push_back(MString("Findings:     ") + NumberOfErrors + " errors, " + (m_Findings.size() - NumberOfErrors) + " warnings");
  for (const MString& Key: MEndToEndFingerprint::GetKeys()) {
    const auto Found = m_Fingerprint.find(Key);
    if (Found != m_Fingerprint.end()) {
      Lines.push_back(Key + ": " + Found->second);
    }
  }
  Lines.push_back("");
  for (const ETCampaignFinding& Finding: m_Findings) {
    MString Line = "WARNING  " + Finding.m_Message;
    if (Finding.m_Severity == MCampaignSeverity::c_Error) {
      Line = "ERROR    " + Finding.m_Message;
    }
    if (Line.Length() > LineLength) {
      Line = Line.GetSubString(0, LineLength - 3) + "...";
    }
    Lines.push_back(Line);
  }
  Report.AddTextPages("Reference campaign: summary and findings", Lines);

  // The table of the observables
  const unsigned int NameLength = 46;
  vector<MString> Table;
  Table.push_back("observable                                       mean         scatter      ratio   drift");
  for (const ETCampaignObservable& Observable: m_Observables) {
    MString Name = Observable.m_Name;
    if (Name.Length() > NameLength) {
      Name = Name.GetSubString(0, NameLength - 3) + "...";
    }
    ostringstream Row;
    Row<<left<<setw(NameLength + 2)<<Name.Data()<<setw(13)<<setprecision(6)<<Observable.m_Mean<<setw(13)<<Observable.m_Scatter<<setw(8)<<fixed<<setprecision(2)<<Observable.m_Ratio<<setw(6)<<setprecision(1)<<Observable.m_Drift;
    Table.push_back(Row.str().c_str());
  }
  Report.AddTextPages("Reference campaign: observables (scatter of the runs, ratio to the bootstrap error, drift of the halves)", Table);

  // The leave-one-out validation: the pulls and Student's t distribution
  if (m_Coverage.m_Pulls.empty() == false) {
    TPad* Body = Report.AddPage("Reference campaign: every run against the mean of the others (leave one out)", 2, 1);
    Body->cd(1);
    const unsigned int Bins = 64;
    const double Range = 8.0;
    TH1D* Histogram = Report.DrawHistogram("pulls;(run - mean of the others)/scatter;comparisons", m_Coverage.m_Pulls, Bins, -Range, Range);
    Histogram->SetLineColor(kBlack);
    TF1* Student = new TF1("", "[0]*TMath::Student(x, [1])", -Range, Range);
    Student->SetParameters(m_Coverage.m_Pulls.size()*2.0*Range/Bins, m_Coverage.m_DegreesOfFreedom);
    Student->SetLineColor(kRed);
    Student->Draw("SAME");
    Body->cd(2);
    Report.DrawLabel(0.05, 0.92, MString("comparisons: ") + m_Coverage.m_NumberOfPulls + ", Student t with " + m_Coverage.m_DegreesOfFreedom + " degrees of freedom");
    for (unsigned int s = 0; s < m_Coverage.m_Sigmas.size(); ++s) {
      Report.DrawLabel(0.05, 0.82 - 0.08*s, MString("beyond ") + MString(m_Coverage.m_Sigmas[s], 2U) + " sigma: " + m_Coverage.m_Outside[s] + ", expected " + MString(m_Coverage.m_Expected[s], 3U));
    }
  }

  // The ratios of the scatter and the bootstrap error
  vector<double> Ratios;
  for (const ETCampaignObservable& Observable: m_Observables) {
    if (Observable.m_Ratio > 0.0) {
      Ratios.push_back(Observable.m_Ratio);
    }
  }
  if (Ratios.empty() == false) {
    TPad* Body = Report.AddPage("Reference campaign: scatter of the runs divided by the bootstrap error of a run (expected between the red lines)", 1, 1);
    Body->cd(1);
    TH1D* Histogram = Report.DrawHistogram("ratios;scatter / bootstrap error;observables", Ratios, 40, 0.0, 3.0);
    for (double Limit: { c_MinimumRatio, c_MaximumRatio }) {
      TLine* Line = new TLine(Limit, 0.0, Limit, Histogram->GetMaximum());
      Line->SetLineColor(kRed);
      Line->Draw();
    }
  }

  if (MEndToEndReport::PrintPdf(Report.GetPages(), Expanded) == false) {
    merr<<"Unable to write the report of the campaign: "<<FileName<<show;
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Make the references of the test from the analysis: mean, standard error, scatter, and calibration of the observables, and the record with the reason
//! The floors are zero: the statistics alone set the tolerance, a floor for a difference which does not matter is set per observable in the file
MEndToEndReferenceList MEndToEndCampaignAnalysis::MakeReferences(const MString& TestName, const MString& Reason) const
{
  MEndToEndReferenceList References;

  // The record: how the references were made
  ostringstream Time;
  const time_t Now = time(nullptr);
  Time<<put_time(localtime(&Now), "%Y-%m-%d");
  References.GetHeader().Set("test", TestName);
  References.GetHeader().Set("date", Time.str().c_str());
  References.GetHeader().Set("reason", Reason);
  References.GetHeader().Set("runs", MString(GetNumberOfRuns()));
  References.GetHeader().Set("megalib", MEndToEndReport::ReadFirstLine("$(MEGALIB)/config/Version.txt"));
  for (const MString& Key: MEndToEndFingerprint::GetKeys()) {
    const auto Found = m_Fingerprint.find(Key);
    if (Found != m_Fingerprint.end()) {
      References.GetHeader().Set(Key, Found->second);
    }
  }

  // The references: the calibration is the ratio of the scatter and the bootstrap error, one if it is not defined
  for (const ETCampaignObservable& Observable: m_Observables) {
    MEndToEndReference Reference(Observable.m_Name);
    Reference.SetMean(Observable.m_Mean);
    Reference.SetError(Observable.m_Error);
    Reference.SetScatter(Observable.m_Scatter);
    Reference.SetRuns(Observable.m_Values.size());
    Reference.SetCalibration(1.0);
    if (Observable.m_Ratio > 0.0) {
      Reference.SetCalibration(Observable.m_Ratio);
    }
    References.SetReference(Reference);
  }
  return References;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a finding
void MEndToEndCampaignAnalysis::AddFinding(MCampaignSeverity Severity, const MString& Message)
{
  ETCampaignFinding Finding;
  Finding.m_Severity = Severity;
  Finding.m_Message = Message;
  m_Findings.push_back(Finding);
}


////////////////////////////////////////////////////////////////////////////////


//! Check the runs: the number, the failed checks, and the environments, and collect the observables of the first run
void MEndToEndCampaignAnalysis::CheckRuns()
{
  if (m_Runs.empty() == true) {
    AddFinding(MCampaignSeverity::c_Error, "There are no runs");
    return;
  }
  if (m_Runs.size() < m_MinimumRuns) {
    AddFinding(MCampaignSeverity::c_Error, MString("There are ") + GetNumberOfRuns() + " runs, at least " + m_MinimumRuns + " are needed");
  }

  // Every check of every run has to pass: the references are for a test which works
  for (unsigned int r = 0; r < m_Runs.size(); ++r) {
    if (m_Runs[r].m_FailedChecks > 0) {
      AddFinding(MCampaignSeverity::c_Error, MString("Run ") + r + " has " + m_Runs[r].m_FailedChecks + " failed checks");
    }
  }

  // All runs have to be made in the same environment, and it has to be known
  bool Same = true;
  for (unsigned int r = 1; r < m_Runs.size(); ++r) {
    if (m_Runs[r].m_Fingerprint != m_Runs[0].m_Fingerprint) {
      Same = false;
      AddFinding(MCampaignSeverity::c_Error, MString("Run ") + r + " was made in another environment than run 0");
    }
  }
  if (Same == true) {
    m_Fingerprint = m_Runs[0].m_Fingerprint;
  }

  // Every key of the fingerprint has to be known
  MString Unknown;
  for (const MString& Key: MEndToEndFingerprint::GetKeys()) {
    const auto Found = m_Runs[0].m_Fingerprint.find(Key);
    if (Found == m_Runs[0].m_Fingerprint.end() || Found->second.IsEmpty() == true || Found->second == "unknown") {
      Unknown += (Unknown.IsEmpty() == true ? "" : ", ") + Key;
    }
  }
  if (Unknown.IsEmpty() == false) {
    AddFinding(MCampaignSeverity::c_Error, MString("The runs did not record their environment completely, unknown: ") + Unknown);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Collect the values of the observables over the runs, errors for observables which are missing or invalid in a run
void MEndToEndCampaignAnalysis::CollectObservables()
{
  if (m_Runs.empty() == true) {
    return;
  }

  // The names of the observables are the ones of the first run, sorted
  set<MString> Names;
  for (const MEndToEndResult& Result: m_Runs[0].m_Results.GetResults()) {
    if (Result.IsFromReferences() == true) {
      Names.insert(Result.GetName());
    }
  }
  if (Names.empty() == true) {
    AddFinding(MCampaignSeverity::c_Error, "The first run has no observable which is compared with references");
    return;
  }

  // Every run needs every observable once, with finite values
  map<MString, ETCampaignObservable> Collected;
  set<MString> Excluded;
  for (const MString& Name: Names) {
    Collected[Name].m_Name = Name;
  }
  for (unsigned int r = 0; r < m_Runs.size(); ++r) {
    set<MString> Seen;
    for (const MEndToEndResult& Result: m_Runs[r].m_Results.GetResults()) {
      if (Result.IsFromReferences() == false) {
        continue;
      }
      if (Names.count(Result.GetName()) == 0) {
        AddFinding(MCampaignSeverity::c_Error, MString("The observable ") + Result.GetName() + " is in run " + r + " but not in run 0");
        continue;
      }
      if (Seen.insert(Result.GetName()).second == false) {
        AddFinding(MCampaignSeverity::c_Error, MString("The observable ") + Result.GetName() + " is twice in run " + r);
        Excluded.insert(Result.GetName());
        continue;
      }
      if (isfinite(Result.GetValue()) == false || isfinite(Result.GetError()) == false) {
        AddFinding(MCampaignSeverity::c_Error, MString("The observable ") + Result.GetName() + " is not finite in run " + r);
        Excluded.insert(Result.GetName());
        continue;
      }
      if (Result.GetError() < 0.0) {
        AddFinding(MCampaignSeverity::c_Error, MString("The observable ") + Result.GetName() + " has a negative uncertainty in run " + r);
        Excluded.insert(Result.GetName());
        continue;
      }
      Collected[Result.GetName()].m_Values.push_back(Result.GetValue());
      Collected[Result.GetName()].m_Errors.push_back(Result.GetError());
    }
    for (const MString& Name: Names) {
      if (Seen.count(Name) == 0) {
        AddFinding(MCampaignSeverity::c_Error, MString("The observable ") + Name + " is missing in run " + r);
        Excluded.insert(Name);
      }
    }
  }

  // An observable which is not complete has no statistics
  for (const pair<const MString, ETCampaignObservable>& Entry: Collected) {
    if (Excluded.count(Entry.first) == 0) {
      m_Observables.push_back(Entry.second);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the statistics of the observables and add the warnings for them
void MEndToEndCampaignAnalysis::CalculateStatistics()
{
  for (ETCampaignObservable& Observable: m_Observables) {
    const unsigned int N = Observable.m_Values.size();
    Observable.m_Mean = MStatistics::GetMean(Observable.m_Values);
    Observable.m_Scatter = MStatistics::GetStandardDeviation(Observable.m_Values);
    Observable.m_Error = Observable.m_Scatter/sqrt(max(1.0, static_cast<double>(N)));
    Observable.m_Median = MStatistics::GetMedian(Observable.m_Values);
    Observable.m_MeanBootstrapError = MStatistics::GetMean(Observable.m_Errors);

    // The robust scatter: median absolute deviation
    vector<double> Deviations;
    for (double Value: Observable.m_Values) {
      Deviations.push_back(fabs(Value - Observable.m_Median));
    }
    Observable.m_RobustScatter = MStatistics::c_MedianAbsoluteDeviationFactor*MStatistics::GetMedian(Deviations);

    CalculateRatio(Observable);
    FindOutliers(Observable);
    CalculateDrift(Observable);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the ratio of the scatter and the bootstrap error of the observable, warn if it is outside the expected range
void MEndToEndCampaignAnalysis::CalculateRatio(ETCampaignObservable& Observable)
{
  if (Observable.m_Scatter > 0.0 && Observable.m_MeanBootstrapError > 0.0) {
    Observable.m_Ratio = Observable.m_Scatter/Observable.m_MeanBootstrapError;
    if (Observable.m_Ratio < c_MinimumRatio || Observable.m_Ratio > c_MaximumRatio) {
      AddFinding(MCampaignSeverity::c_Warning, Observable.m_Name + ": the scatter of the runs is " + MString(Observable.m_Ratio, 3U) + " times the bootstrap error, expected between " + MString(c_MinimumRatio, 2U) + " and " + MString(c_MaximumRatio, 2U));
    }
  } else if (Observable.m_Values.size() >= 2 && Observable.m_Scatter == 0.0 && Observable.m_MeanBootstrapError > 0.0) {
    AddFinding(MCampaignSeverity::c_Warning, Observable.m_Name + ": the runs do not scatter although the bootstrap error is " + MString(Observable.m_MeanBootstrapError, 3U));
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Warn about the runs of the observable which are far from the others
void MEndToEndCampaignAnalysis::FindOutliers(const ETCampaignObservable& Observable)
{
  if (Observable.m_RobustScatter <= 0.0) {
    return;
  }
  for (unsigned int r = 0; r < Observable.m_Values.size(); ++r) {
    if (fabs(Observable.m_Values[r] - Observable.m_Median) > c_OutlierLimit*Observable.m_RobustScatter) {
      AddFinding(MCampaignSeverity::c_Warning, Observable.m_Name + ": run " + r + " (" + MString(Observable.m_Values[r], 4U) + ") is an outlier, the median is " + MString(Observable.m_Median, 4U));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Calculate the drift of the observable (the first against the last half of the runs), warn if it is large
void MEndToEndCampaignAnalysis::CalculateDrift(ETCampaignObservable& Observable)
{
  // The scatter within the halves is the one which counts
  const unsigned int Half = Observable.m_Values.size()/2;
  if (Half < 2 || Observable.m_Scatter <= 0.0) {
    return;
  }
  const vector<double> First(Observable.m_Values.begin(), Observable.m_Values.begin() + Half);
  const vector<double> Last(Observable.m_Values.end() - Half, Observable.m_Values.end());
  const double Difference = MStatistics::GetMean(Last) - MStatistics::GetMean(First);
  const double ErrorOfDifference = sqrt((MStatistics::GetStandardDeviation(First)*MStatistics::GetStandardDeviation(First) + MStatistics::GetStandardDeviation(Last)*MStatistics::GetStandardDeviation(Last))/Half);
  if (ErrorOfDifference > 0.0) {
    Observable.m_Drift = Difference/ErrorOfDifference;
  } else if (Difference != 0.0) {
    Observable.m_Drift = Difference*numeric_limits<double>::infinity();
  }
  if (fabs(Observable.m_Drift) > c_DriftLimit) {
    AddFinding(MCampaignSeverity::c_Warning, Observable.m_Name + ": the first and the last half of the runs differ by " + MString(Observable.m_Drift, 3U) + " standard errors");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Compare every run with the mean of all the others
void MEndToEndCampaignAnalysis::ValidateLeaveOneOut()
{
  // The limits are 1 to the pull limit sigma
  const unsigned int NumberOfLimits = static_cast<unsigned int>(MEndToEndResultList::c_PullLimit);
  for (unsigned int s = 1; s <= NumberOfLimits; ++s) {
    m_Coverage.m_Sigmas.push_back(s);
  }
  m_Coverage.m_Outside.assign(NumberOfLimits, 0);
  m_Coverage.m_Expected.assign(NumberOfLimits, 0.0);

  // The pull of a run is (value - mean of the others)/(scatter of the others*sqrt(1 + 1/(N - 1))): Student's t distributed with N - 2 degrees of freedom
  for (const ETCampaignObservable& Observable: m_Observables) {
    const unsigned int N = Observable.m_Values.size();
    if (N < 4) {
      continue;
    }
    unsigned int NumberOfPulls = 0;
    for (unsigned int r = 0; r < N; ++r) {
      vector<double> Others;
      for (unsigned int o = 0; o < N; ++o) {
        if (o != r) {
          Others.push_back(Observable.m_Values[o]);
        }
      }
      const double Scatter = MStatistics::GetStandardDeviation(Others);
      if (Scatter <= 0.0) {
        continue;
      }
      const double Pull = (Observable.m_Values[r] - MStatistics::GetMean(Others))/(Scatter*sqrt(1.0 + 1.0/Others.size()));
      m_Coverage.m_Pulls.push_back(Pull);
      m_Coverage.m_DegreesOfFreedom = N - 2;
      ++NumberOfPulls;
      for (unsigned int s = 0; s < NumberOfLimits; ++s) {
        if (fabs(Pull) > m_Coverage.m_Sigmas[s]) {
          ++m_Coverage.m_Outside[s];
        }
      }
    }
    m_Coverage.m_NumberOfPulls += NumberOfPulls;
    for (unsigned int s = 0; s < NumberOfLimits; ++s) {
      m_Coverage.m_Expected[s] += NumberOfPulls*2.0*(1.0 - TMath::StudentI(m_Coverage.m_Sigmas[s], N - 2.0));
    }
  }

  // The runs are independent: a pull beyond the limit of the tests is a finding
  if (m_Coverage.m_Outside.empty() == false && m_Coverage.m_Outside.back() > 0) {
    AddFinding(MCampaignSeverity::c_Warning, MString(m_Coverage.m_Outside.back()) + " of " + m_Coverage.m_NumberOfPulls + " comparisons of a run with the others are beyond " + MString(m_Coverage.m_Sigmas.back(), 2U) + " sigma, expected " + MString(m_Coverage.m_Expected.back(), 3U));
  }
}


////////////////////////////////////////////////////////////////////////////////




// MEndToEndCampaignAnalysis.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
