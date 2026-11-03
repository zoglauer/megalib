/*
 * MEndToEndRecord.h
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

#ifndef __MEndToEndRecord__
#define __MEndToEndRecord__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <utility>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A record of an end-to-end test: fields "key: value" in the order of setting, stored as comment lines "# key: value"
class MEndToEndRecord
{
  // public interface:
 public:
  //! Default constructor
  MEndToEndRecord() = default;
  //! Default destructor
  ~MEndToEndRecord() = default;

  //! Remove all fields
  void Clear() { m_Fields.clear(); }

  //! Read the file (the fields replace the current ones), return false if it cannot be read
  bool Read(const MString& FileName);
  //! Write the file into the directory of the file: the title, the fields, and the text which follows them, return false if it cannot be written
  bool Write(const MString& FileName, const MString& Title, const MString& Following = "") const;

  //! Set a field, an existing field is replaced
  void Set(const MString& Key, const MString& Value);
  //! Return a field, empty if it does not exist
  MString Get(const MString& Key) const;
  //! Return the fields in the order in which they were set
  const vector<pair<MString, MString>>& GetFields() const { return m_Fields; }

  //! Set a field from a line "# key: value" and return true, return false for any other line (the line of the columns is not a field)
  bool FromLine(const MString& Line);
  //! Return the fields as lines "# key: value"
  MString ToLines() const;

  // private members:
 private:
  //! The fields in the order of setting
  vector<pair<MString, MString>> m_Fields;
};

#endif


////////////////////////////////////////////////////////////////////////////////
