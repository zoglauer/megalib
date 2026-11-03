/*
 * MEndToEndFingerprint.h
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

#ifndef __MEndToEndFingerprint__
#define __MEndToEndFingerprint__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MEndToEndRecord.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The environment which the results of an end-to-end test depend on, and the names of the environment variables of a campaign
class MEndToEndFingerprint
{
  // public interface:
 public:
  //! Return the keys of the fingerprint: Geant4, ROOT, the geometry, and the scenarios
  static vector<MString> GetKeys();
  //! Return the hash of a text (MString::GetHash(): FNV-1a, 64 bit, the same on every machine) as text: a value of the fingerprint
  static MString GetHash(const MString& Text);
  //! Return the hash of a geometry file and of all files which the geometry reader includes while it reads it (MDGeometry::GetHash()), "unknown" if the geometry cannot be read
  //! The geometry is only read, not built (no volume tree, no cross sections), and the streams are left as they were
  static MString GetFilesHash(const MString& FileName);
  //! Return the descriptions of the keys of the fingerprint in which the record differs from the current environment, empty if the record matches
  //! A key which the record does not have counts as different, and so does a key which the environment does not have
  static vector<MString> Compare(const MEndToEndRecord& Recorded, const map<MString, MString>& Current);

  //! Return the name of the environment variable with the base seed of the random seeds of a test: a campaign sets it to make its runs reproducible
  static MString GetSeedVariable() { return "MEGALIB_END_TO_END_SEED"; }
  //! Return the name of the environment variable which says that the test is a run of a reference campaign: it measures the key results but does not compare them with the references it replaces
  static MString GetCampaignVariable() { return "MEGALIB_END_TO_END_CAMPAIGN"; }
};

#endif


////////////////////////////////////////////////////////////////////////////////
