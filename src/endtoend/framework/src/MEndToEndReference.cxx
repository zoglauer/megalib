/*
 * MEndToEndReference.cxx
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
#include "MEndToEndReference.h"

// Standard libs:
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Set the reference from a line of the file (five or eight tab separated columns), return false and leave the reference as it is if the line has another number of columns
bool MEndToEndReference::FromLine(const MString& Line)
{
  const unsigned int MinimumColumns = 5;
  const unsigned int MaximumColumns = 8;
  const vector<MString> Columns = Line.Tokenize("\t");
  if (Columns.size() != MinimumColumns && Columns.size() != MaximumColumns) {
    merr<<"A reference has "<<MinimumColumns<<" or "<<MaximumColumns<<" tab separated columns, but this line has "<<Columns.size()<<": "<<Line<<show;
    return false;
  }

  // Every number is a complete number in its range: the mean is finite, the errors and the floors are not negative, the calibration is positive, there is at least one run
  MEndToEndReference Reference(Columns[0]);
  bool Valid = Columns[1].IsNumber() && Columns[2].IsNumber() && Columns[3].IsNumber();
  Valid = Valid && Columns[4].Is<unsigned int>();
  if (Valid == true) {
    Reference.m_Mean = Columns[1].ToDouble();
    Reference.m_Error = Columns[2].ToDouble();
    Reference.m_Scatter = Columns[3].ToDouble();
    Reference.m_Runs = Columns[4].ToUnsignedInt();
    Valid = Reference.m_Error >= 0.0 && Reference.m_Scatter >= 0.0 && Reference.m_Runs > 0;
  }
  if (Valid == true && Columns.size() == MaximumColumns) {
    Valid = Columns[5].IsNumber() && Columns[6].IsNumber() && Columns[7].IsNumber();
    if (Valid == true) {
      Reference.m_Calibration = Columns[5].ToDouble();
      Reference.m_RelativeFloor = Columns[6].ToDouble();
      Reference.m_AbsoluteFloor = Columns[7].ToDouble();
      Valid = Reference.m_Calibration > 0.0 && Reference.m_RelativeFloor >= 0.0 && Reference.m_AbsoluteFloor >= 0.0;
    }
  }
  if (Valid == false) {
    merr<<"A reference has a number which is not complete or not in its range: "<<Line<<show;
    return false;
  }

  *this = Reference;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the line of the file: all columns with 17 digits, a tab in the name is replaced by a space
MString MEndToEndReference::ToLine() const
{
  MString Name = m_Name;
  Name.ReplaceAllInPlace("\t", " ");
  
  ostringstream Line;
  Line<<setprecision(17)<<Name.Data()<<"\t"<<m_Mean<<"\t"<<m_Error<<"\t"<<m_Scatter<<"\t"<<m_Runs<<"\t"<<m_Calibration<<"\t"<<m_RelativeFloor<<"\t"<<m_AbsoluteFloor;
  
  return Line.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the names of the columns of the line
MString MEndToEndReference::GetColumnNames()
{
  return "Name, Mean, Error (of the mean), Scatter (of the runs), Runs, Calibration of the bootstrap error, Relative floor, Absolute floor";
}


// MEndToEndReference.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
