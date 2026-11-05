/*
 * UTSourceEarthNoData.cc
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


// Cosima:
#include "MCOrientation.hh"
#include "MCRun.hh"
#include "MCSource.hh"
#include "UTSDHitShared.h"
#include "UTSourceShared.h"

// MEGAlib:
#include "MUnitTest.h"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <functional>
#include <string>
#include <vector>
using namespace std;


//! Unit test class for the Earth occultation of MCSource with a Galactic sky orientation file which has no Earth position
//! and does not loop (times 1000 s to 2000 s)
class UTSourceEarthNoData : public UTSourceTestBase
{
public:
  //! Default constructor
  UTSourceEarthNoData() : UTSourceTestBase("UTSourceEarthNoData") {}
  //! Default destructor
  virtual ~UTSourceEarthNoData() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Configure a far field source with Earth occultation, by default with the Earth occultation beam (as the parameter file does)
  bool ConfigureFarField(MCSource& S, int BeamType = MCSource::c_FarFieldEarthOccultation)
  {
    SilenceOutput();
    bool OK = S.SetParticleType(MCSource::c_Gamma);
    OK = S.SetSpectralType(MCSource::c_Monoenergetic) && OK;
    OK = S.SetEnergy(511*keV) && OK;
    OK = S.SetBeamType(MCSource::c_FarField, BeamType) && OK;
    OK = S.SetPosition() && OK;
    OK = S.SetFlux(1.0/(cm*cm*second)) && OK;
    OK = S.SetStartAreaType(MCSource::c_StartAreaSphere) && OK;
    OK = S.SetStartAreaParameters(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 30*cm) && OK;
    OK = S.SetEarthOccultation(0.0, false) && OK;
    OK = S.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  }
};


////////////////////////////////////////////////////////////////////////////////


bool UTSourceEarthNoData::Run()
{
  bool Passed = true;

  // The parser gets the file name relative to the private temporary root, which is the working directory of the test:
  // the parameter file format has no quoting, and the absolute path may contain blanks (e.g. the log directory)
  PrepareTemporaryDirectory("Sky");
  const MString SkyFile = GetTemporaryDirectoryName("Sky") + "/Sky.ori";
  Passed = EvaluateTrue("WriteTextFile()", "sky", "The orientation file can be written", WriteTextFile(SkyFile, "OG 1000 0 130 20 40\nOG 2000 0 130 20 40\n")) && Passed;
  Passed = EvaluateTrue("InitializeEnvironment()", "no Earth data", "The Geant4 environment with a Galactic sky can be created",
                        InitializeEnvironment("Dummy.OrientationSky Galactic File NoLoop Sky/Sky.ori\n")) && Passed;
  if (m_EnvironmentOK == false) {
    Summarize();
    return false;
  }

  // Outside of the time range of the orientation file there is no Earth position:
  {
    MCSource S("Out of range");
    Passed = EvaluateTrue("ConfigureFarField()", "out of range", "The source can be configured", ConfigureFarField(S)) && Passed;
    SilenceOutput();
    bool Generated = S.GeneratePosition(m_Gun);
    RestoreOutput();
    // The code reports "the time is not in the ori time range" and must then refuse to generate a photon (it used to continue with theta = phi = 0, i.e. a photon from the zenith)
    Passed = EvaluateFalse("GeneratePosition()", "out of range", "No start position is generated when the Earth position is not known", Generated) && Passed;
  }

  // Inside the time range, but the orientation file has no Earth position: must not access missing data
  {
    MCSource S("No Earth data");
    Passed = EvaluateTrue("ConfigureFarField()", "no Earth data", "The source can be configured", ConfigureFarField(S)) && Passed;
    SilenceOutput();
    S.CalculateNextEmission(1500*second, 1.0);
    RestoreOutput();
    Passed = EvaluateNear("GetNextEmission()", "no Earth data, lower", "The next emission is not before the orientation file starts (shown: the shortfall)", max(1000*second - S.GetNextEmission(), 0.0), 0.0, 0.0) && Passed;
    Passed = EvaluateNear("GetNextEmission()", "no Earth data, upper", "The next emission is not after the orientation file ends (shown: the excess)", max(S.GetNextEmission() - 2000*second, 0.0), 0.0, 0.0) && Passed;

    // The children inherit the random generators, thus seed them (the outcome must not depend on the random numbers)
    RandomGuard Guard(55);
    UTSDHitShared::ChildOutcome Position = UTSDHitShared::RunInChild([&S, this]() { SilenceOutput(); bool R = S.GeneratePosition(m_Gun); return R; });
    Passed = EvaluateTrue("GeneratePosition()", "no Earth data", "A missing Earth position does not give a start position (and does not crash, time out, or fail to fork)", Position == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;
    UTSDHitShared::ChildOutcome Particles = UTSDHitShared::RunInChild([&S, this]() { SilenceOutput(); bool R = S.GenerateParticles(m_Gun); return R; });
    Passed = EvaluateTrue("GenerateParticles()", "no Earth data", "A missing Earth position does not give a particle (and does not crash, time out, or fail to fork)", Particles == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;
  }

  // Regression test (A4, MCSource::GenerateParticles): without Earth position (outside the time range or no data in the file) the occultation
  // used to be skipped and the photon went through - the event must not be generated (or its energy must be zero, which the run manager skips)
  for (int InRange = 0; InRange < 2; ++InRange) {
    RandomGuard Guard(56 + InRange);
    MCSource S("Occultation without Earth");
    const MString Name = InRange == 1 ? "inside the range, no Earth data" : "outside the range";
    // An isotropic beam: its start position does not depend on the Earth, thus only the occultation in GenerateParticles() can reject the event
    Passed = EvaluateTrue("ConfigureFarField()", Name, "The source can be configured", ConfigureFarField(S, MCSource::c_FarFieldIsotropic)) && Passed;
    if (InRange == 1) {
      SilenceOutput();
      S.CalculateNextEmission(1500*second, 1.0);
      RestoreOutput();
    }
    UTSDHitShared::ChildOutcome Outcome = UTSDHitShared::RunInChild([&S, this]() {
      SilenceOutput();
      const bool Generated = S.GenerateParticles(m_Gun);
      return Generated == true && GetGunEnergy() > 0.0;
    });
    Passed = EvaluateTrue("GenerateParticles()", Name, "No photon passes when the Earth position is unknown", Outcome == UTSDHitShared::ChildOutcome::c_ReturnedFalse) && Passed;
  }

  // Regression test (A7, MCSource::CalculateNextEmission): the message for the end of the sky orientation used to print the stop time of the source's own orientation
  {
    MCSource S("Sky end");
    Passed = EvaluateTrue("ConfigureFarField()", "sky end", "The source can be configured", ConfigureFarField(S)) && Passed;
    // The MEGAlib stream is written to a log file, since it is not part of cout
    const MString Log = GetTemporaryFileName("SkyEnd.log");
    mout.DumpToStdOut(false);
    mout.Connect(Log);
    S.CalculateNextEmission(2000*second, 1.0);
    mout.Disconnect(Log);
    mout.DumpToStdOut(true);
    ifstream In(Log.Data());
    string Text((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("CalculateNextEmission()", "sky end", "The message about the exceeded sky orientation contains the stop time of the sky (2000 s)", Text.find("max time: 2000") != string::npos) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSourceEarthNoData Test;
  return Test.Run() == true ? 0 : 1;
}
