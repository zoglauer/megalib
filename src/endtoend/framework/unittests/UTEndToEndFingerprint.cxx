/*
 * UTEndToEndFingerprint.cxx
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


// Standard libs:
using namespace std;

// MEGAlib libs:
#include "MDGeometry.h"
#include "MEndToEndFingerprint.h"
#include "MFile.h"
#include "MUnitTest.h"


//! Unit test class for MEndToEndFingerprint
class UTEndToEndFingerprint : public MUnitTest
{
public:
  UTEndToEndFingerprint() : MUnitTest("UTEndToEndFingerprint") {}
  virtual ~UTEndToEndFingerprint() {}

  virtual bool Run();

private:
  bool TestHash();
  bool TestGeometryHash();
  //! Write a small geometry (name, version, sphere, the materials, and the volumes which are included) into the directory
  bool WriteGeometry(const MString& Directory, const MString& Volumes, const MString& Main);
  bool TestCompare();
};


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndFingerprint::Run()
{
  bool Passed = true;
  Passed = TestHash() && Passed;
  Passed = TestGeometryHash() && Passed;
  Passed = TestCompare() && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndFingerprint::TestHash()
{
  bool Passed = true;

  // The test vectors of FNV-1a with 64 bits (from the specification of the hash) as signed 64 bit numbers: the unsigned values of the specification are cbf29ce484222325, af63dc4c8601ec8c, and 85944171f73967e8
  Passed = Evaluate("MEndToEndFingerprint::GetHash()", "empty", "The hash of the empty text is the offset of FNV-1a", MEndToEndFingerprint::GetHash(""), MString("-3750763034362895579")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetHash()", "a", "The hash of a", MEndToEndFingerprint::GetHash("a"), MString("-5808556873153909620")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetHash()", "foobar", "The hash of foobar", MEndToEndFingerprint::GetHash("foobar"), MString("-8821353812377114648")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetSeedVariable()", "name", "The variable with the base seed", MEndToEndFingerprint::GetSeedVariable(), MString("MEGALIB_END_TO_END_SEED")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetCampaignVariable()", "name", "The variable which marks a campaign run", MEndToEndFingerprint::GetCampaignVariable(), MString("MEGALIB_END_TO_END_CAMPAIGN")) && Passed;
  Passed = EvaluateTrue("MEndToEndFingerprint::GetHash()", "different", "A different text has a different hash", MEndToEndFingerprint::GetHash("foobar") != MEndToEndFingerprint::GetHash("foobaz")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetHash()", "same", "The same text has the same hash", MEndToEndFingerprint::GetHash("foobar"), MEndToEndFingerprint::GetHash("foobar")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Write a small geometry (name, version, sphere, the materials, and the volumes which are included) into the directory
bool UTEndToEndFingerprint::WriteGeometry(const MString& Directory, const MString& Volumes, const MString& Main)
{
  if (PrepareTemporaryDirectory(MFile::GetBaseName(Directory)) == false) {
    return false;
  }
  if (WriteTextFile(Directory + "/Volumes.geo", "Volume World\nWorld.Material Vacuum\nWorld.Visibility 0\nWorld.Shape BRIK 1000 1000 1000\nWorld.Mother 0\n" + Volumes) == false) {
    return false;
  }
  return WriteTextFile(Directory + "/Main.geo", "Name Mini\nVersion 1.0\nSurroundingSphere 100 0 0 0 100\nInclude $(MEGALIB)/resource/examples/geomega/materials/Materials.geo\n" + Main);
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndFingerprint::TestGeometryHash()
{
  bool Passed = true;

  const MString Directory = GetTemporaryDirectoryName("files_hash");
  Passed = EvaluateTrue("WriteGeometry()", "first", "The geometry can be written", WriteGeometry(Directory, "// first", "Include Volumes.geo // the volumes\n")) && Passed;
  const MString First = MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo");
  Passed = EvaluateTrue("MEndToEndFingerprint::GetFilesHash()", "geometry", "A geometry which can be read has a hash", First != "unknown") && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "repeat", "The same files give the same hash", MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo"), First) && Passed;
  // The reader is silenced while it reads, and the streams are left as they were: streams which are disabled stay disabled
  DisableDefaultStreams();
  MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo");
  bool StillDisabled = false;
  if (mout.IsEnabled() == false && mlog.IsEnabled() == false && mgui.IsEnabled() == false) {
    StillDisabled = true;
  }
  EnableDefaultStreams();
  Passed = EvaluateTrue("MEndToEndFingerprint::GetFilesHash()", "streams", "Streams which were disabled are still disabled after the geometry was read", StillDisabled) && Passed;

  // The hash of the fingerprint is the one of the geometry
  MDGeometry Geometry;
  Passed = Evaluate("MDGeometry::GetHash()", "not read", "Without a geometry the hash is zero", Geometry.GetHash(), 0L) && Passed;
  DisableDefaultStreams();
  const bool Read = Geometry.ScanSetupFile(Directory + "/Main.geo");
  EnableDefaultStreams();
  Passed = EvaluateTrue("MDGeometry::ScanSetupFile()", "geometry", "The geometry can be read", Read) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "geometry hash", "The hash is the hash of the geometry as text", First, MString(Geometry.GetHash())) && Passed;

  // An include with a trailing comment is read by the geometry reader, so a change in the file changes the hash
  Passed = EvaluateTrue("WriteTextFile()", "second", "The included file can be changed", WriteTextFile(Directory + "/Volumes.geo", "Volume World\nWorld.Material Vacuum\nWorld.Visibility 0\nWorld.Shape BRIK 1000 1000 1000\nWorld.Mother 0\n// second")) && Passed;
  Passed = EvaluateTrue("MEndToEndFingerprint::GetFilesHash()", "included", "A change of an included file changes the hash", MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo") != First) && Passed;

  // Includes which the reader does not read do not change the hash
  Passed = EvaluateTrue("WriteGeometry()", "not read", "The geometry with includes which are not read can be written", WriteGeometry(Directory, "// first", "Include Volumes.geo\n// Include Other.geo\nBeginComment\nInclude Other.geo\nEndComment\nExit\nInclude Other.geo\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "other", "The file which is not read can be written", WriteTextFile(Directory + "/Other.geo", "Name Other")) && Passed;
  const MString Ignored = MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo");
  Passed = EvaluateTrue("WriteTextFile()", "other changed", "The file which is not read can be changed", WriteTextFile(Directory + "/Other.geo", "Name Changed")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "not read", "A change of a file which is commented out or after Exit does not change the hash", MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo"), Ignored) && Passed;

  // Only the contents of the files matter, not their names or where they are
  const MString Moved = GetTemporaryDirectoryName("files_hash_moved");
  Passed = EvaluateTrue("WriteGeometry()", "moved", "The same geometry can be written to another directory", WriteGeometry(Moved, "// first", "Include Volumes.geo\n")) && Passed;
  Passed = EvaluateTrue("WriteGeometry()", "original", "The geometry can be written again", WriteGeometry(Directory, "// first", "Include Volumes.geo\n")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "moved", "The same files in another directory give the same hash", MEndToEndFingerprint::GetFilesHash(Moved + "/Main.geo"), MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo")) && Passed;

  // A geometry which cannot be read has no hash
  Passed = EvaluateTrue("WriteGeometry()", "missing include", "The geometry with a missing include can be written", WriteGeometry(Directory, "// first", "Include Missing.geo\n")) && Passed;
  DisableDefaultStreams();
  const MString Unreadable = MEndToEndFingerprint::GetFilesHash(Directory + "/Main.geo");
  const MString None = MEndToEndFingerprint::GetFilesHash(Directory + "/None.geo");
  EnableDefaultStreams();
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "missing include", "A geometry with a missing include is unknown", Unreadable, MString("unknown")) && Passed;
  Passed = Evaluate("MEndToEndFingerprint::GetFilesHash()", "missing file", "A geometry file which does not exist is unknown", None, MString("unknown")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTEndToEndFingerprint::TestCompare()
{
  bool Passed = true;

  map<MString, MString> Current = { { "geant4", "10.2.3" }, { "root", "6.40.04" }, { "geometry", "00000000000000aa" }, { "scenarios", "00000000000000bb" } };
  Passed = EvaluateSize("MEndToEndFingerprint::GetKeys()", "number", "Geant4, ROOT, geometry, and scenarios", MEndToEndFingerprint::GetKeys().size(), 4) && Passed;

  MEndToEndRecord References;
  References.Set("geant4", "10.2.3");
  References.Set("root", "6.40.04");
  References.Set("geometry", "00000000000000aa");
  References.Set("scenarios", "00000000000000bb");
  References.Set("megalib", "4.04.00");
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "match", "The record matches the environment", MEndToEndFingerprint::Compare(References, Current).size(), 0) && Passed;

  // The version of MEGAlib is only information: other fields are not compared
  Current["megalib"] = "5.00.00";
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "megalib", "A different MEGAlib version is no difference", MEndToEndFingerprint::Compare(References, Current).size(), 0) && Passed;

  Current["geant4"] = "11.2.0";
  vector<MString> Differences = MEndToEndFingerprint::Compare(References, Current);
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "geant4", "A different Geant4 version is one difference", Differences.size(), 1) && Passed;
  if (Differences.size() == 1) {
    Passed = Evaluate("MEndToEndFingerprint::Compare()", "geant4 text", "The difference names both versions", Differences[0], MString("geant4: references 10.2.3, now 11.2.0")) && Passed;
  }

  Current["scenarios"] = "00000000000000cc";
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "two", "Two different fields are two differences", MEndToEndFingerprint::Compare(References, Current).size(), 2) && Passed;

  // A field which the record does not have, and a field which the environment does not have
  MEndToEndRecord Unrecorded;
  Unrecorded.Set("geant4", "10.2.3");
  Differences = MEndToEndFingerprint::Compare(Unrecorded, { { "geant4", "10.2.3" } });
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "unrecorded", "ROOT, geometry, and scenarios are not recorded", Differences.size(), 3) && Passed;
  if (Differences.size() == 3) {
    Passed = Evaluate("MEndToEndFingerprint::Compare()", "unrecorded text", "The difference says that the field is not recorded", Differences[0], MString("root is not recorded in the references (now unknown)")) && Passed;
  }
  Passed = EvaluateSize("MEndToEndFingerprint::Compare()", "unknown now", "A field which the environment does not have is unknown and differs", MEndToEndFingerprint::Compare(References, { }).size(), 4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize global MEGAlib variables, especially mgui, etc. - the geometry needs the materials of MEGAlib, which fails without MEGALIB
  if (MGlobal::Initialize("UTEndToEndFingerprint", "unit test of the end-to-end fingerprint") == false) {
    return 1;
  }

  UTEndToEndFingerprint Test;
  if (Test.Run() == true) {
    return 0;
  }
  return 1;
}


// UTEndToEndFingerprint.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
