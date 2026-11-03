/*
 * MEndToEndReference.h
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


#ifndef __MEndToEndReference__
#define __MEndToEndReference__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
using namespace std;

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A reference value of an observable: 
//! The result of a campaign of independent runs, with the line which stores it in the file of the references
class MEndToEndReference
{
  // public interface:
 public:
  //! Default constructor
  MEndToEndReference() = default;
  //! Standard constructor giving the name of the observable: scenario and observable
  explicit MEndToEndReference(const MString& Name) : m_Name(Name) {}
  //! Default destructor
  ~MEndToEndReference() = default;

  //! Set the name: scenario and observable
  void SetName(const MString& Name) { m_Name = Name; }
  //! Return the name: scenario and observable
  const MString& GetName() const { return m_Name; }
  
  //! Set the mean of the runs
  void SetMean(double Mean) { m_Mean = Mean; }
  //! Return the mean of the runs
  double GetMean() const { return m_Mean; }
  
  //! Set the standard error of the mean
  void SetError(double Error) { m_Error = Error; }
  //! Return the standard error of the mean
  double GetError() const { return m_Error; }
  
  //! Set the scatter (standard deviation) of the single runs
  void SetScatter(double Scatter) { m_Scatter = Scatter; }
  //! Return the scatter (standard deviation) of the single runs
  double GetScatter() const { return m_Scatter; }
  
  //! Set the number of runs
  void SetRuns(unsigned int Runs) { m_Runs = Runs; }
  //! Return the number of runs
  unsigned int GetRuns() const { return m_Runs; }
  
  //! Set the factor by which the bootstrap error of a single run is multiplied: the scatter of the runs divided by their mean bootstrap error
  void SetCalibration(double Calibration) { m_Calibration = Calibration; }
  //! Return the factor by which the bootstrap error of a single run is multiplied
  double GetCalibration() const { return m_Calibration; }
  
  //! Set the smallest accepted difference as fraction of the mean: a reference which is much more precise than a change which does not matter
  void SetRelativeFloor(double RelativeFloor) { m_RelativeFloor = RelativeFloor; }
  //! Return the smallest accepted difference as fraction of the mean
  double GetRelativeFloor() const { return m_RelativeFloor; }
  
  //! Set the smallest accepted difference in the unit of the observable
  void SetAbsoluteFloor(double AbsoluteFloor) { m_AbsoluteFloor = AbsoluteFloor; }
  //! Return the smallest accepted difference in the unit of the observable
  double GetAbsoluteFloor() const { return m_AbsoluteFloor; }

  //! Set the reference from a line of the file (five or eight tab separated columns), return false and leave the reference as it is if the line has another number of columns
  bool FromLine(const MString& Line);
  //! Return the line of the file: all columns with 17 digits, a tab in the name is replaced by a space
  MString ToLine() const;
  
  //! Return the names of the columns of the line
  static MString GetColumnNames();

  // private members:
 private:
  //! The name: scenario and observable
  MString m_Name;
  //! The mean of the runs
  double m_Mean = 0.0;
  //! The standard error of the mean
  double m_Error = 0.0;
  //! The scatter (standard deviation) of the single runs
  double m_Scatter = 0.0;
  //! The number of runs
  unsigned int m_Runs = 0;
  //! The factor by which the bootstrap error of a single run is multiplied: the scatter of the runs divided by their mean bootstrap error
  double m_Calibration = 1.0;
  //! The smallest accepted difference as fraction of the mean
  double m_RelativeFloor = 0.0;
  //! The smallest accepted difference in the unit of the observable
  double m_AbsoluteFloor = 0.0;
};

#endif


////////////////////////////////////////////////////////////////////////////////
