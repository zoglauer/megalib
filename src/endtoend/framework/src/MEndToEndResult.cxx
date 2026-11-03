/*
 * MEndToEndResult.cxx
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
#include "MEndToEndResult.h"

// Standard libs:
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Set the result from a line of the file (twelve tab separated columns), return false and leave the result as it is if the line has another number of columns
bool MEndToEndResult::FromLine(const MString& Line)
{
  const unsigned int NumberOfColumns = 12;
  const vector<MString> Columns = Line.Tokenize("\t");
  if (Columns.size() != NumberOfColumns) {
    merr<<"A key result has "<<NumberOfColumns<<" tab separated columns, but this line has "<<Columns.size()<<": "<<Line<<show;
    return false;
  }

  // Every number is a complete finite number, the flags are 0 or 1, and no uncertainty or tolerance is negative
  MEndToEndResult Result(Columns[0]);
  bool Valid = true;
  for (unsigned int c: { 1, 2, 4, 5, 6, 7 }) {
    Valid = Valid && Columns[c].IsNumber();
  }
  for (unsigned int c: { 3, 8, 9, 10, 11 }) {
    Valid = Valid && Columns[c].Is<bool>();
  }
  if (Valid == true) {
    Result.m_Value = Columns[1].ToDouble();
    Result.m_Error = Columns[2].ToDouble();
    Result.m_HasReference = (Columns[3].ToInt() != 0);
    Result.m_Reference = Columns[4].ToDouble();
    Result.m_ReferenceError = Columns[5].ToDouble();
    Result.m_Pull = Columns[6].ToDouble();
    Result.m_Tolerance = Columns[7].ToDouble();
    Result.m_IsValid = (Columns[8].ToInt() != 0);
    Result.m_IsStatistical = (Columns[9].ToInt() != 0);
    Result.m_Passed = (Columns[10].ToInt() != 0);
    Result.m_FromReferences = (Columns[11].ToInt() != 0);
    Valid = Result.m_Error >= 0.0 && Result.m_ReferenceError >= 0.0 && Result.m_Tolerance >= 0.0;
  }
  if (Valid == false) {
    merr<<"A key result has a number which is not complete or not in its range: "<<Line<<show;
    return false;
  }

  *this = Result;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the line of the file: all columns with 17 digits, a tab in the name is replaced by a space
MString MEndToEndResult::ToLine() const
{
  MString Name = m_Name;
  Name.ReplaceAllInPlace("\t", " ");

  ostringstream Line;
  Line<<setprecision(17)<<Name.Data()<<"\t"<<m_Value<<"\t"<<m_Error<<"\t"<<m_HasReference<<"\t"<<m_Reference<<"\t"<<m_ReferenceError<<"\t"<<m_Pull<<"\t"<<m_Tolerance<<"\t"<<m_IsValid<<"\t"<<m_IsStatistical<<"\t"<<m_Passed<<"\t"<<m_FromReferences;

  return Line.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Return the names of the columns of the line
MString MEndToEndResult::GetColumnNames()
{
  return "Name, Value, Error (of the value), Has reference, Reference, Error (of the reference), Pull, Tolerance, Valid, Statistical, Passed, From references";
}


// MEndToEndResult.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
