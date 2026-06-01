/*
 * UTAtmosphericAbsorption.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MAtmosphericAbsorption.h"
#include "MFile.h"
#include "MUnitTest.h"

// Standard libs:


//! Unit test class for MAtmosphericAbsorption
class UTAtmosphericAbsorption : public MUnitTest
{
public:
  UTAtmosphericAbsorption() : MUnitTest("UTAtmosphericAbsorption") {}
  virtual ~UTAtmosphericAbsorption() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTAtmosphericAbsorption::Run()
{
  bool Passed = true;

  const MString TempDirectory = GetTemporaryDirectoryName();
  if (PrepareTemporaryDirectory() == false) {
    Summarize();
    return false;
  }

  const MString AbsorptionFile = TempDirectory + "/atmospheric.abs";
  MString AbsorptionContent =
    "IP LIN\n"
    "XA 0 10\n"
    "YA 0 20\n"
    "ZA 100 200\n"
    "AP 0 0 100 0.10\n"
    "AP 10 0 100 0.50\n"
    "AP 0 20 100 0.30\n"
    "AP 10 20 100 0.70\n"
    "AP 0 0 200 0.20\n"
    "AP 10 0 200 0.60\n"
    "AP 0 20 200 0.40\n"
    "AP 10 20 200 0.80\n"
    "EN\n";
  Passed = EvaluateTrue("WriteTextFile()", "fixture", "The representative atmospheric absorption fixture can be written", WriteTextFile(AbsorptionFile, AbsorptionContent)) && Passed;

  MAtmosphericAbsorption Absorption;
  Passed = Evaluate("MAtmosphericAbsorption()", "construction", "A representative atmospheric-absorption instance can be constructed", true, true) && Passed;
  Passed = EvaluateTrue("MFile::Exists()", "fixture", "The representative atmospheric absorption fixture exists", MFile::Exists(AbsorptionFile)) && Passed;
  Passed = EvaluateTrue("Read()", "fixture", "MAtmosphericAbsorption reads the representative absorption fixture", Absorption.Read(AbsorptionFile)) && Passed;

  // MEGAlib callers currently pass altitude, azimuth, energy in this order.
  Passed = EvaluateNear("GetTransmissionProbability()", "interior", "GetTransmissionProbability returns the representative interpolated transmission probability",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 150.0), 0.45, 1e-12) && Passed;
  Passed = Evaluate("GetTransmissionProbability()", "lower corner", "GetTransmissionProbability returns the representative lower-corner transmission probability",
                    Absorption.GetTransmissionProbability(0.0, 0.0, 100.0), 0.10) && Passed;
  Passed = Evaluate("GetTransmissionProbability()", "upper corner", "GetTransmissionProbability returns the representative upper-corner transmission probability",
                    Absorption.GetTransmissionProbability(10.0, 20.0, 200.0), 0.80) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateNear("GetTransmissionProbability()", "below altitude", "GetTransmissionProbability clamps altitude below the tabulated range to the minimum altitude",
                        Absorption.GetTransmissionProbability(-1.0, 10.0, 150.0), 0.25, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "above altitude", "GetTransmissionProbability clamps altitude above the tabulated range to the maximum altitude",
                        Absorption.GetTransmissionProbability(11.0, 10.0, 150.0), 0.65, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "below azimuth", "GetTransmissionProbability clamps azimuth below the tabulated range to the minimum azimuth",
                        Absorption.GetTransmissionProbability(5.0, -1.0, 150.0), 0.35, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "above azimuth", "GetTransmissionProbability clamps azimuth above the tabulated range to the maximum azimuth",
                        Absorption.GetTransmissionProbability(5.0, 25.0, 150.0), 0.55, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "below energy", "GetTransmissionProbability clamps energy below the tabulated range to the minimum energy",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 50.0), 0.40, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "above energy", "GetTransmissionProbability clamps energy above the tabulated range to the maximum energy",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 250.0), 0.50, 1e-12) && Passed;
  EnableDefaultStreams();
  Passed = EvaluateTrue("Read()", "reuse", "MAtmosphericAbsorption can be re-read on the same instance", Absorption.Read(AbsorptionFile)) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "reuse", "Re-reading the representative fixture preserves the representative interpolated transmission probability",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 150.0), 0.45, 1e-12) && Passed;

  DisableDefaultStreams();
  MAtmosphericAbsorption Missing;
  Passed = EvaluateFalse("Read()", "missing file", "MAtmosphericAbsorption rejects a missing absorption file", Missing.Read(TempDirectory + "/missing.abs")) && Passed;
  EnableDefaultStreams();

  const MString MalformedFile = TempDirectory + "/malformed.abs";
  MString MalformedContent =
    "IP LIN\n"
    "XA 0\n"
    "YA 0 20\n"
    "ZA 100 200\n"
    "AP 0 0 100 0.10\n";
  Passed = EvaluateTrue("WriteTextFile()", "malformed fixture", "The malformed atmospheric absorption fixture can be written", WriteTextFile(MalformedFile, MalformedContent)) && Passed;

  DisableDefaultStreams();
  MAtmosphericAbsorption Malformed;
  Passed = EvaluateFalse("Read()", "malformed file", "MAtmosphericAbsorption rejects a representative malformed absorption file", Malformed.Read(MalformedFile)) && Passed;
  EnableDefaultStreams();

  MAtmosphericAbsorption Recovery;
  DisableDefaultStreams();
  Passed = EvaluateFalse("Read()", "recover setup", "MAtmosphericAbsorption rejects the malformed file before recovery", Recovery.Read(MalformedFile)) && Passed;
  EnableDefaultStreams();
  Passed = EvaluateTrue("Read()", "recover", "MAtmosphericAbsorption can recover after a failed read on the same instance", Recovery.Read(AbsorptionFile)) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "recover", "A failed read does not prevent the representative interpolated probability from being reloaded",
                        Recovery.GetTransmissionProbability(5.0, 10.0, 150.0), 0.45, 1e-12) && Passed;

  Passed = EvaluateTrue("MFile::Remove()", "malformed cleanup", "The representative malformed atmospheric absorption file can be removed", MFile::Remove(MalformedFile)) && Passed;

  Passed = EvaluateTrue("MFile::Remove()", "fixture cleanup", "The representative atmospheric absorption fixture can be removed", MFile::Remove(AbsorptionFile)) && Passed;
  Passed = EvaluateFalse("MFile::Exists()", "fixture cleanup", "The representative atmospheric absorption fixture is gone after cleanup", MFile::Exists(AbsorptionFile)) && Passed;
  Passed = EvaluateTrue("RemoveTemporaryDirectory()", "temp cleanup", "The atmospheric-absorption temp directory can be removed", RemoveTemporaryDirectory()) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTAtmosphericAbsorption Test;
  return Test.Run() == true ? 0 : 1;
}
