/*
 * MEndToEndCampaignAnalysis.h
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

#ifndef __MEndToEndCampaignAnalysis__
#define __MEndToEndCampaignAnalysis__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MEndToEndReferenceList.h"
#include "MEndToEndReport.h"
#include "MEndToEndResultList.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The severity of a finding of a campaign: an error blocks the references, a warning has to be read
enum class MCampaignSeverity { c_Warning, c_Error };


//! One run of the campaign: the key results of the test, the number of its failed checks, and its environment
class ETCampaignRun
{
public:
  //! The key results of the run
  MEndToEndResultList m_Results;
  //! The number of checks of the run which failed
  unsigned int m_FailedChecks = 0;
  //! The environment of the run (the fingerprint: Geant4, ROOT, geometry, scenarios)
  map<MString, MString> m_Fingerprint;
};


//! A finding of the analysis of a campaign
class ETCampaignFinding
{
public:
  //! The severity
  MCampaignSeverity m_Severity = MCampaignSeverity::c_Warning;
  //! The description
  MString m_Message;
};


//! The statistics of an observable over the runs of a campaign
class ETCampaignObservable
{
public:
  //! The name: scenario and observable
  MString m_Name;
  //! The values of the runs
  vector<double> m_Values;
  //! The bootstrap errors of the runs
  vector<double> m_Errors;
  //! The mean of the values
  double m_Mean = 0.0;
  //! The scatter (standard deviation) of the values
  double m_Scatter = 0.0;
  //! The standard error of the mean
  double m_Error = 0.0;
  //! The median of the values
  double m_Median = 0.0;
  //! The robust scatter (1.4826 times the median absolute deviation) of the values
  double m_RobustScatter = 0.0;
  //! The mean of the bootstrap errors of the runs
  double m_MeanBootstrapError = 0.0;
  //! The scatter divided by the mean bootstrap error (zero if either of them is zero): the calibration of the bootstrap error
  double m_Ratio = 0.0;
  //! The difference of the means of the first and the second half of the runs in standard errors of the difference (from the scatter within the halves): a drift of the results
  double m_Drift = 0.0;
};


//! The leave-one-out validation of the campaign: every run is compared with the mean of all the others
class ETCampaignCoverage
{
public:
  //! The number of sigma of the limits: 1, 2, ... up to the number of the largest limit
  vector<double> m_Sigmas;
  //! The number of the comparisons of runs with the mean of the others
  unsigned int m_NumberOfPulls = 0;
  //! The number of the comparisons which are beyond the limits
  vector<unsigned int> m_Outside;
  //! The number which is expected beyond the limits for Student's t distribution of independent runs
  vector<double> m_Expected;
  //! All the comparisons (pulls)
  vector<double> m_Pulls;
  //! The degrees of freedom of the pulls: the number of runs minus two (of the observable with the fewest runs, zero without pulls)
  unsigned int m_DegreesOfFreedom = 0;
};


//! The analysis of a campaign of independent runs of an end-to-end test: statistics of the observables, quality gates, and the references which follow from them
//! * Only the key results which are compared with references (MEndToEndResult::IsFromReferences()) are observables of the campaign
//! * Errors (too few runs, a failed check, a missing or invalid value, different environments) block the references, warnings have to be read
class MEndToEndCampaignAnalysis
{
  // public interface:
 public:
  //! Default constructor
  MEndToEndCampaignAnalysis();
  //! Default destructor
  ~MEndToEndCampaignAnalysis() = default;

  //! Set the smallest number of runs which makes references
  void SetMinimumRuns(unsigned int MinimumRuns) { m_MinimumRuns = MinimumRuns; }
  //! Add a run
  void AddRun(const ETCampaignRun& Run) { m_Runs.push_back(Run); }
  //! Return the number of runs
  unsigned int GetNumberOfRuns() const { return m_Runs.size(); }

  //! Read the key results and the environment of a run of the test which are in the directory, return false if the key results cannot be read
  static bool ReadRun(const MString& Directory, const MString& TestName, unsigned int FailedChecks, ETCampaignRun& Run);

  //! Analyze the runs: the observables, the findings, and the leave-one-out validation
  void Analyze();

  //! Return the observables, sorted by name
  const vector<ETCampaignObservable>& GetObservables() const { return m_Observables; }
  //! Return the findings of the analysis
  const vector<ETCampaignFinding>& GetFindings() const { return m_Findings; }
  //! Return true if there is a finding which is an error
  bool HasErrors() const;
  //! Return the leave-one-out validation
  const ETCampaignCoverage& GetCoverage() const { return m_Coverage; }
  //! Return the environment which all runs have (empty if they differ or there are no runs)
  const map<MString, MString>& GetFingerprint() const { return m_Fingerprint; }

  //! Write the report of the analysis as PDF: the findings, the table of the observables, the leave-one-out validation, and the ratios of scatter and bootstrap error, return false if the file cannot be written
  bool WriteReport(const MString& TestName, const MString& FileName) const;
  //! Make the references of the test from the analysis: mean, standard error, scatter, and calibration of the observables, and the record with the reason
  //! The floors are zero: the statistics alone set the tolerance, a floor for a difference which does not matter is set per observable in the file
  MEndToEndReferenceList MakeReferences(const MString& TestName, const MString& Reason) const;

  // private methods:
 private:
  //! Add a finding
  void AddFinding(MCampaignSeverity Severity, const MString& Message);
  //! Check the runs: the number, the failed checks, and the environments, and collect the observables of the first run
  void CheckRuns();
  //! Collect the values of the observables over the runs, errors for observables which are missing or invalid in a run
  void CollectObservables();
  //! Calculate the statistics of the observables and add the warnings for them
  void CalculateStatistics();
  //! Calculate the ratio of the scatter and the bootstrap error of the observable, warn if it is outside the expected range
  void CalculateRatio(ETCampaignObservable& Observable);
  //! Warn about the runs of the observable which are far from the others
  void FindOutliers(const ETCampaignObservable& Observable);
  //! Calculate the drift of the observable (the first against the last half of the runs), warn if it is large
  void CalculateDrift(ETCampaignObservable& Observable);
  //! Compare every run with the mean of all the others
  void ValidateLeaveOneOut();

  // private members:
 private:
  //! The smallest expected ratio of the scatter and the mean bootstrap error
  static constexpr double c_MinimumRatio = 0.7;
  //! The largest expected ratio of the scatter and the mean bootstrap error
  static constexpr double c_MaximumRatio = 1.4;
  //! A run is an outlier if it is farther than this number of robust scatters from the median
  static constexpr double c_OutlierLimit = 6.0;
  //! The halves of the runs differ if their means differ by more than this number of standard errors of the difference
  static constexpr double c_DriftLimit = 4.0;
  //! The smallest number of runs which makes references
  unsigned int m_MinimumRuns;
  //! The runs
  vector<ETCampaignRun> m_Runs;
  //! The observables
  vector<ETCampaignObservable> m_Observables;
  //! The findings
  vector<ETCampaignFinding> m_Findings;
  //! The leave-one-out validation
  ETCampaignCoverage m_Coverage;
  //! The environment which all runs have
  map<MString, MString> m_Fingerprint;
};

#endif


////////////////////////////////////////////////////////////////////////////////
