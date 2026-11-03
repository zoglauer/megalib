/*
 * MEndToEndRecord.cxx
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
#include "MEndToEndRecord.h"

// Standard libs:
#include <fstream>
using namespace std;

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Read the file (the fields replace the current ones), return false if it cannot be read
bool MEndToEndRecord::Read(const MString& FileName)
{
  Clear();

  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    merr<<"Unable to read the record: "<<FileName<<show;
    return false;
  }
  for (const MString& Line: Content.Tokenize("\n")) {
    FromLine(Line);
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the file into the directory of the file: the title, the fields, and the text which follows them, return false if it cannot be written
bool MEndToEndRecord::Write(const MString& FileName, const MString& Title, const MString& Following) const
{
  MString Expanded = FileName;
  if (MFile::ExpandFileName(Expanded) == false) {
    merr<<"Unable to expand the name of the file of the record: "<<FileName<<show;
    return false;
  }
  if (MFile::CreateDirectory(MFile::GetDirectoryName(Expanded)) == false) {
    merr<<"Unable to create the directory of the file of the record: "<<Expanded<<show;
    return false;
  }

  ofstream Out(Expanded.Data());
  if (Out.is_open() == false) {
    merr<<"Unable to write the file of the record: "<<Expanded<<show;
    return false;
  }

  Out<<"# "<<Title<<endl;
  Out<<ToLines();
  Out<<Following;
  if (Out.good() == false) {
    merr<<"Unable to write the file of the record: "<<Expanded<<show;
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set a field, an existing field is replaced
void MEndToEndRecord::Set(const MString& Key, const MString& Value)
{
  for (pair<MString, MString>& Field: m_Fields) {
    if (Field.first == Key) {
      Field.second = Value;
      return;
    }
  }
  m_Fields.push_back(make_pair(Key, Value));
}


////////////////////////////////////////////////////////////////////////////////


//! Return a field, empty if it does not exist
MString MEndToEndRecord::Get(const MString& Key) const
{
  for (const pair<MString, MString>& Field: m_Fields) {
    if (Field.first == Key) {
      return Field.second;
    }
  }
  return "";
}


////////////////////////////////////////////////////////////////////////////////


//! Set a field from a line "# key: value" and return true, return false for any other line
bool MEndToEndRecord::FromLine(const MString& Line)
{
  if (Line.BeginsWith("#") == false || Line.Contains(": ") == false) {
    return false;
  }
  const unsigned int Colon = Line.Index(": ");
  if (Colon <= 2) {
    return false;
  }
  const MString Key = Line.GetSubString(2, Colon - 2);
  if (Key == "columns") {
    return false;
  }
  Set(Key, Line.GetSubString(Colon + 2));
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the fields as lines "# key: value"
MString MEndToEndRecord::ToLines() const
{
  MString Lines;
  for (const pair<MString, MString>& Field: m_Fields) {
    Lines += MString("# ") + Field.first + ": " + Field.second + "\n";
  }
  return Lines;
}


// MEndToEndRecord.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
