/*
 * MReadOutFileFormat.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MReadOutFileFormat
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutFileFormat.h"

// Standard libs:
#include <cctype>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MExceptions.h"
#include "MTokenizer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutFileFormat)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The keyword of the original single-unit format, "UH"
const MString MReadOutFileFormat::c_DefaultKeyword = "UH";


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutFileFormat::MReadOutFileFormat()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutFileFormat::~MReadOutFileFormat()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear all data
void MReadOutFileFormat::Clear()
{
  m_ReadOutKeywords.clear();
  m_ReadOutElementTypes.clear();
  m_ReadOutDataTypes.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this is a valid read-out keyword: "U" followed by letters or digits, not starting with "UF"
bool MReadOutFileFormat::IsValidKeyword(const MString& Keyword)
{
  // The readers recognize their keywords by prefix, "U" is reserved for read-outs except "UF"
  if (Keyword.Length() < 2) return false;
  if (Keyword[0] != 'U') return false;
  if (Keyword.BeginsWith("UF") == true) return false;
  for (size_t c = 1; c < Keyword.Length(); ++c) {
    if (isalnum(Keyword[c]) == 0) return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a definition of a read-out unit
//! Returns false if the keyword is invalid or already in use
bool MReadOutFileFormat::AddReadOutUnit(const MString& ReadOutKeyword, const MString& ReadOutElementType, const MString& ReadOutDataType)
{
  if (IsValidKeyword(ReadOutKeyword) == false) {
    merr<<"Invalid read-out keyword \""<<ReadOutKeyword<<"\": it must start with \"U\" followed by letters or digits, and cannot start with \"UF\"."<<endl;
    return false;
  }
  if (ReadOutElementType.Length() == 0 || ReadOutDataType.Length() == 0) {
    merr<<"A read-out unit needs a read-out element and a read-out data type."<<endl;
    return false;
  }
  for (const MString& K: m_ReadOutKeywords) {
    if (K == ReadOutKeyword) {
      merr<<"The read-out unit keyword \""<<ReadOutKeyword<<"\" is already used by this file."<<endl;
      return false;
    }
  }

  m_ReadOutKeywords.push_back(ReadOutKeyword);
  m_ReadOutElementTypes.push_back(ReadOutElementType);
  m_ReadOutDataTypes.push_back(ReadOutDataType);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a definition of a read-out unit without keyword
//! That will use "UH" as the first keyword, "U1", "U2" for latter
bool MReadOutFileFormat::AddReadOutUnit(const MString& ReadOutElementType, const MString& ReadOutDataType)
{
  return AddReadOutUnit(CreateKeyword(), ReadOutElementType, ReadOutDataType);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the "UF" line and store the definition
//! Accepts "UF <element> <data>" and "UF <keyword> <element> <data>"
bool MReadOutFileFormat::ParseUF(const MString& Line)
{
  MTokenizer T;
  T.Analyze(Line);

  MString Keyword;
  MString ElementType;
  MString DataType;
  if (T.GetNTokens() == 3) {
    Keyword = c_DefaultKeyword;
    ElementType = T.GetTokenAt(1);
    DataType = T.GetTokenAt(2);
  } else if (T.GetNTokens() == 4) {
    Keyword = T.GetTokenAt(1);
    ElementType = T.GetTokenAt(2);
    DataType = T.GetTokenAt(3);
  } else {
    merr<<"Unable to parse UF line: "<<Line<<endl;
    return false;
  }

  // If we read multiple included files, the keyword line can appear multiple times
  const unsigned int Existing = FindByKeyword(Keyword);
  if (Existing != g_UnsignedIntNotDefined) {
    if (m_ReadOutElementTypes[Existing] == ElementType && m_ReadOutDataTypes[Existing] == DataType) return true;
    merr<<"Found an existing read-out keyword with a different read-out element and data type."<<endl;
    return false;
  }

  return AddReadOutUnit(Keyword, ElementType, DataType);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the read-out unit of this line, or g_UnsignedIntNotDefined if none is found
unsigned int MReadOutFileFormat::FindByLine(const MString& Line) const
{
  for (unsigned int u = 0; u < m_ReadOutKeywords.size(); ++u) {
    const MString& Keyword = m_ReadOutKeywords[u];
    if (Line.BeginsWith(Keyword) == false) continue;
    // The line length must be longer than the keyword + space
    if (Line.Length() <= Keyword.Length() + 1 ) continue;
    // The must be a space after the keyword - then we have found it
    if (Line[Keyword.Length()] == ' ') {
      return u;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the read-out unit with this read-out keyword, or g_UnsignedIntNotDefined if it does not exist
unsigned int MReadOutFileFormat::FindByKeyword(const MString& ReadOutKeyword) const
{
  for (unsigned int u = 0; u < m_ReadOutKeywords.size(); ++u) {
    if (m_ReadOutKeywords[u] == ReadOutKeyword) {
      return u;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the read-out unit defined by these types, or g_UnsignedIntNotDefined if none is found
unsigned int MReadOutFileFormat::FindByTypes(const MString& ReadOutElementType, const MString& ReadOutDataType) const
{
  for (unsigned int u = 0; u < m_ReadOutKeywords.size(); ++u) {
    if (m_ReadOutElementTypes[u] == ReadOutElementType && m_ReadOutDataTypes[u] == ReadOutDataType) {
      return u;
    }
  }

  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a new keyword in this order: "UH", "U1", "U2", ...
MString MReadOutFileFormat::CreateKeyword() const
{
  // First check if the defauklt keyword, UH, is not yet used
  if (FindByKeyword(c_DefaultKeyword) == g_UnsignedIntNotDefined) {
    return c_DefaultKeyword;
  }

  // Then try to find one from the U1, U2, .... list
  for (unsigned int i = 1; ; ++i) {
    MString Candidate = "U";
    Candidate += i;
    if (FindByKeyword(Candidate) == g_UnsignedIntNotDefined) {
      return Candidate;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the keyword of read-out unit i
const MString& MReadOutFileFormat::GetKeyword(unsigned int i) const
{
  if (i < m_ReadOutKeywords.size()) return m_ReadOutKeywords[i];

  throw MExceptionIndexOutOfBounds(0, m_ReadOutKeywords.size(), i);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the read-out element type of read-out unit i
const MString& MReadOutFileFormat::GetElementType(unsigned int i) const
{
  if (i < m_ReadOutElementTypes.size()) return m_ReadOutElementTypes[i];

  throw MExceptionIndexOutOfBounds(0, m_ReadOutElementTypes.size(), i);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the read-out data type of read-out unit i
const MString& MReadOutFileFormat::GetDataType(unsigned int i) const
{
  if (i < m_ReadOutDataTypes.size()) return m_ReadOutDataTypes[i];

  throw MExceptionIndexOutOfBounds(0, m_ReadOutDataTypes.size(), i);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the UF line of read-out unit i
MString MReadOutFileFormat::GetUFLine(unsigned int i) const
{
  if (i >= m_ReadOutKeywords.size()) {
    throw MExceptionIndexOutOfBounds(0, m_ReadOutKeywords.size(), i);
  }

  ostringstream os;
  os<<"UF "<<m_ReadOutKeywords[i]<<" "<<m_ReadOutElementTypes[i]<<" "<<m_ReadOutDataTypes[i];

  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump a the content of this read-out file format
MString MReadOutFileFormat::ToString() const
{
  ostringstream os;
  os<<"Read-out format with "<<m_ReadOutKeywords.size()<<" read-out unit(s)";
  for (unsigned int u = 0; u < m_ReadOutKeywords.size(); ++u) {
    os<<endl<<"  "<<m_ReadOutKeywords[u]<<": "<<m_ReadOutElementTypes[u]<<" "<<m_ReadOutDataTypes[u];
  }

  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text
ostream& operator<<(ostream& os, const MReadOutFileFormat& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutFileFormat.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
