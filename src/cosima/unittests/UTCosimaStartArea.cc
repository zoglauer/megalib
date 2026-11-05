/*
 * UTCosimaStartArea.cc
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

// End-to-end test of the start areas in cosima:
// The same far-field beams (flux in ph/cm2/s) are simulated once with a spherical and once with a
// tube-like start area around the same detector. The detected rate must not depend on the start area,
// thus the time needed to collect the same number of triggers has to agree within statistics.


// Cosima:
#include "UTFixtureShared.h"

// MEGAlib:
#include "MGlobal.h"
#include "MProcessManager.h"
#include "MString.h"
#include "MSystem.h"
#include "MUnitTest.h"

// Standard lib:
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iostream>
#include <string>
#include <vector>
using namespace std;


//! Unit test class for the spherical and tube-like start areas
class UTCosimaStartArea : public MUnitTest
{
public:
  //! Default constructor
  UTCosimaStartArea() : MUnitTest("UTCosimaStartArea") {}
  //! Default destructor
  virtual ~UTCosimaStartArea() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Copy the input files into the temporary directory and set the number of triggers
  bool PrepareInputFiles(const MString& Directory);
  //! Run cosima on the given source file (without the extension) in the given directory, return its wait status (0: exited normally with 0)
  int RunCosima(const MString& Directory, const MString& Source, unsigned int Seed);
  //! Read the simulation end time and the number of events from the sim file, return false on failure
  bool ReadSimFile(const MString& FileName, double& EndTime, unsigned int& NEvents);

  //! The number of triggers per run: the time to reach them has a relative statistical uncertainty of 1/sqrt(N)
  //! 10000 -> ~1% per run, ~13 s run time; 100000 -> ~0.3% per run, ~90 s run time
  static const unsigned int c_NumberOfTriggers = 10000;
  //! The number of standard deviations the two simulations are allowed to deviate
  static const unsigned int c_NumberOfSigmas = 4;
};


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaStartArea::PrepareInputFiles(const MString& Directory)
{
  const char* Root = getenv("MEGALIB");
  if (Root == nullptr) {
    merr<<"Failed: MEGALIB is not set"<<endl;
    return false;
  }
  MString Data = MString(Root) + "/src/cosima/unittests/data/";

  // The geometry and the beam file are used as is
  for (const char* File : { "StartArea.geo.setup", "StartAreaBeam.dat" }) {
    ifstream In((Data + File).Data(), ios::binary);
    ofstream Out((Directory + "/" + File).Data(), ios::binary);
    if (In.is_open() == false || Out.is_open() == false) {
      merr<<"Failed: unable to copy "<<File<<endl;
      return false;
    }
    Out<<In.rdbuf();
  }

  // The source files get the number of triggers of this test
  for (const char* File : { "StartAreaSphere.source", "StartAreaTube.source" }) {
    ifstream In((Data + File).Data());
    ofstream Out((Directory + "/" + File).Data());
    if (In.is_open() == false || Out.is_open() == false) {
      merr<<"Failed: unable to copy "<<File<<endl;
      return false;
    }
    string Line;
    while (getline(In, Line)) {
      size_t Pos = Line.find(".Triggers");
      if (Pos != string::npos) {
        Line = Line.substr(0, Pos) + ".Triggers " + to_string(c_NumberOfTriggers);
      }
      Out<<Line<<endl;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int UTCosimaStartArea::RunCosima(const MString& Directory, const MString& Source, unsigned int Seed)
{
  // The run stays in the process group of the test: the test driver stops it with the test if the test times out; the time limit turns a hang into a failure
  return MProcessManager::Run("cosima", MString("-v 0 -s ") + Seed + " -z " + MSystem::GetShellQuoted(Source + ".source"), Directory + "/" + Source + ".log", Directory, 100);
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaStartArea::ReadSimFile(const MString& FileName, double& EndTime, unsigned int& NEvents)
{
  EndTime = -1;
  NEvents = 0;

  FILE* Pipe = popen((MString("gzip -dc ") + MSystem::GetShellQuoted(FileName)).Data(), "r");
  if (Pipe == nullptr) return false;

  char Buffer[4096];
  while (fgets(Buffer, sizeof(Buffer), Pipe) != nullptr) {
    string Line(Buffer);
    if (Line.compare(0, 3, "TE ") == 0) {
      EndTime = atof(Line.c_str() + 3);
    } else if (Line.compare(0, 2, "SE") == 0) {
      ++NEvents;
    }
  }
  pclose(Pipe);

  return EndTime > 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaStartArea::Run()
{
  bool Passed = true;

  // The preconditions: the input files below $MEGALIB, the temporary directory, and the copies of the input files
  if (RequireFixture(*this, Passed, "src/cosima/unittests/data/StartArea.geo.setup") == false) {
    Summarize();
    return Passed;
  }
  MString Directory = GetTemporaryDirectoryName("StartArea");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "StartArea", "The temporary directory can be created", PrepareTemporaryDirectory("StartArea")) && Passed;
  Passed = EvaluateTrue("PrepareInputFiles()", "StartArea", "The geometry, the beam file, and the source files are copied into it", PrepareInputFiles(Directory)) && Passed;
  if (Passed == false) {
    Summarize();
    return Passed;
  }

  // Fixed seeds make the test reproducible, the two runs are independent of each other
  auto Sphere = async(launch::async, [&]() { return RunCosima(Directory, "StartAreaSphere", 101); });
  auto Tube = async(launch::async, [&]() { return RunCosima(Directory, "StartAreaTube", 202); });
  int SphereStatus = Sphere.get();
  int TubeStatus = Tube.get();

  Passed = EvaluateTrue("RunCosima()", "StartAreaSphere.source", "Cosima exits normally with the spherical start area", SphereStatus == 0) && Passed;
  Passed = EvaluateTrue("RunCosima()", "StartAreaTube.source", "Cosima exits normally with the tube-like start area", TubeStatus == 0) && Passed;

  // For a fixed number of triggers N the observation time has a relative standard deviation of 1/sqrt(N),
  // the difference of two independent runs sqrt(2)/sqrt(N)
  const double RelativeSigma = sqrt(2.0/c_NumberOfTriggers);

  for (const char* Beam : { "FarFieldPointSource", "FarFieldAreaSource", "FarFieldFileZenithDependent" }) {
    double EndTimeSphere = 0, EndTimeTube = 0;
    unsigned int EventsSphere = 0, EventsTube = 0;
    MString SphereFile = Directory + "/StartAreaSphere_" + Beam + ".inc1.id1.sim.gz";
    MString TubeFile = Directory + "/StartAreaTube_" + Beam + ".inc1.id1.sim.gz";

    bool ReadSphere = ReadSimFile(SphereFile, EndTimeSphere, EventsSphere);
    bool ReadTube = ReadSimFile(TubeFile, EndTimeTube, EventsTube);
    Passed = EvaluateTrue("ReadSimFile()", Beam, "The sim file of the spherical start area can be read", ReadSphere) && Passed;
    Passed = EvaluateTrue("ReadSimFile()", Beam, "The sim file of the tube-like start area can be read", ReadTube) && Passed;
    if (ReadSphere == false || ReadTube == false) continue;

    Passed = Evaluate("ReadSimFile()", Beam, "The spherical start area stores the requested number of triggers", EventsSphere, c_NumberOfTriggers) && Passed;
    Passed = Evaluate("ReadSimFile()", Beam, "The tube-like start area stores the requested number of triggers", EventsTube, c_NumberOfTriggers) && Passed;

    double Mean = 0.5*(EndTimeSphere + EndTimeTube);
    Passed = EvaluateNear("Start area", Beam, "The time for the same number of triggers agrees between sphere and tube within 4 sigma",
                          EndTimeTube, EndTimeSphere, c_NumberOfSigmas*RelativeSigma*Mean) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTCosimaStartArea Test;
  return Test.Run() == true ? 0 : 1;
}
