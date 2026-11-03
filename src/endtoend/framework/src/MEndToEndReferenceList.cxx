/*
 * MEndToEndReferenceList.cxx
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
#include "MEndToEndReferenceList.h"

// MEGAlib libs:
#include "MFile.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Remove the header and all references
void MEndToEndReferenceList::Clear()
{
  m_Header.Clear();
  m_References.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Read the file (the header and the references replace the current ones), return false if it cannot be read
bool MEndToEndReferenceList::Read(const MString& FileName)
{
  Clear();

  MString Content;
  if (MFile::ReadTextFile(FileName, Content) == false) {
    merr<<"Unable to read the file with the references: "<<FileName<<show;
    return false;
  }

  for (const MString& Line: Content.Tokenize("\n")) {
    // The header: lines "# key: value", other comments and the line of the columns are ignored
    if (Line.BeginsWith("#") == true) {
      m_Header.FromLine(Line);
      continue;
    }

    // A reference: a line which is not one is ignored
    MEndToEndReference Reference;
    if (Reference.FromLine(Line) == true) {
      SetReference(Reference);
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the file into the directory of the file, the references sorted by name, return false if it cannot be written
bool MEndToEndReferenceList::Write(const MString& FileName) const
{
  MString Table = MString("# columns: ") + MEndToEndReference::GetColumnNames() + "\n";
  for (const pair<const MString, MEndToEndReference>& Entry: m_References) {
    Table += Entry.second.ToLine() + "\n";
  }
  return m_Header.Write(FileName, "MEGAlib end-to-end references - do not edit, make them with a reference campaign", Table);
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if there is a reference with this name
bool MEndToEndReferenceList::HasReference(const MString& Name) const
{
  if (m_References.find(Name) == m_References.end()) {
    return false;
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the reference with this name, all values are zero if there is none
MEndToEndReference MEndToEndReferenceList::GetReference(const MString& Name) const
{
  const auto Found = m_References.find(Name);
  if (Found == m_References.end()) {
    return MEndToEndReference(Name);
  }
  return Found->second;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the default name of the file of a test: $(MEGALIB)/resource/endtoend/<test>.references.tsv
MString MEndToEndReferenceList::GetFileName(const MString& TestName)
{
  return MString("$(MEGALIB)/resource/endtoend/") + TestName + ".references.tsv";
}


// MEndToEndReferenceList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
