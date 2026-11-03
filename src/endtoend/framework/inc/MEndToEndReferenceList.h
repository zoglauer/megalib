/*
 * MEndToEndReferenceList.h
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

#ifndef __MEndToEndReferenceList__
#define __MEndToEndReferenceList__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// MEGAlib libs:
#include "MEndToEndRecord.h"
#include "MEndToEndReference.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A list of reference values of an end-to-end test together with the record of how they were made
class MEndToEndReferenceList
{
  // public interface:
 public:
  //! Default constructor
  MEndToEndReferenceList() = default;
  //! Default destructor
  ~MEndToEndReferenceList() = default;

  //! Remove the header and all references
  void Clear();

  //! Read the file (the header and the references replace the current ones), return false if it cannot be read
  bool Read(const MString& FileName);
  //! Write the file into the directory of the file, the references sorted by name, return false if it cannot be written
  bool Write(const MString& FileName) const;

  //! Return the record of how the references were made
  const MEndToEndRecord& GetHeader() const { return m_Header; }
  //! Return the record of how the references were made, to change it
  MEndToEndRecord& GetHeader() { return m_Header; }

  //! Set a reference, an existing one with the same name is replaced
  void SetReference(const MEndToEndReference& Reference) { m_References[Reference.GetName()] = Reference; }
  //! Return true if there is a reference with this name
  bool HasReference(const MString& Name) const;
  //! Return the reference with this name, all values are zero if there is none, thus check with HasReference first
  MEndToEndReference GetReference(const MString& Name) const;
  //! Return the number of references
  unsigned int GetNumberOfReferences() const { return m_References.size(); }

  //! Return the default name of the file of a test: $(MEGALIB)/resource/endtoend/<test>.references.tsv
  static MString GetFileName(const MString& TestName);

  // private members:
 private:
  //! The record of how the references were made
  MEndToEndRecord m_Header;
  //! The references by name
  map<MString, MEndToEndReference> m_References;
};

#endif


////////////////////////////////////////////////////////////////////////////////
