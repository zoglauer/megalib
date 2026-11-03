/*
 * MEndToEndFingerprint.cxx
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
#include "MEndToEndFingerprint.h"

// MEGAlib libs:
#include "MDGeometry.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


//! Return the keys of the fingerprint: Geant4, ROOT, the geometry, and the scenarios
vector<MString> MEndToEndFingerprint::GetKeys()
{
  return { "geant4", "root", "geometry", "scenarios" };
}


////////////////////////////////////////////////////////////////////////////////


//! Return the hash of a text (MString::GetHash()) as text
MString MEndToEndFingerprint::GetHash(const MString& Text)
{
  return MString(Text.GetHash());
}


////////////////////////////////////////////////////////////////////////////////


//! Return the hash of a geometry file and of all files which the geometry reader includes while it reads it, "unknown" if the geometry cannot be read
//! The geometry is only read, not built (no volume tree, no cross sections), and the streams are left as they were
MString MEndToEndFingerprint::GetFilesHash(const MString& FileName)
{
  // Switch off the output of the reader - a geometry which cannot be read is reported as unknown - and switch it back to what it was
  MDGeometry Geometry;
  const bool OutEnabled = mout.IsEnabled();
  const bool LogEnabled = mlog.IsEnabled();
  const bool GuiEnabled = mgui.IsEnabled();
  mout.Enable(false);
  mlog.Enable(false);
  mgui.Enable(false);
  const bool Read = Geometry.ScanSetupFile(FileName, false, false, false);
  mout.Enable(OutEnabled);
  mlog.Enable(LogEnabled);
  mgui.Enable(GuiEnabled);
  if (Read == false) {
    return "unknown";
  }
  const long Hash = Geometry.GetHash();
  if (Hash == 0) {
    return "unknown";
  }
  return MString(Hash);
}


////////////////////////////////////////////////////////////////////////////////


//! Return the descriptions of the keys of the fingerprint in which the record differs from the current environment, empty if the record matches
vector<MString> MEndToEndFingerprint::Compare(const MEndToEndRecord& Recorded, const map<MString, MString>& Current)
{
  vector<MString> Differences;
  for (const MString& Key: GetKeys()) {
    const MString Then = Recorded.Get(Key);
    MString Now = "unknown";
    const auto Found = Current.find(Key);
    if (Found != Current.end()) {
      Now = Found->second;
    }
    if (Then.IsEmpty() == true) {
      Differences.push_back(Key + " is not recorded in the references (now " + Now + ")");
    } else if (Then != Now) {
      Differences.push_back(Key + ": references " + Then + ", now " + Now);
    }
  }
  return Differences;
}


// MEndToEndFingerprint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
