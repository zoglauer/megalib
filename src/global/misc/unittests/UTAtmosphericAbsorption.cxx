/*
 * UTAtmosphericAbsorption.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <cerrno>
#include <fstream>
#include <sys/stat.h>
using namespace std;

// MEGAlib:
#include "MAtmosphericAbsorption.h"
#include "MFile.h"
#include "MUnitTest.h"


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

  const MString TempDirectory = "/tmp/UTAtmosphericAbsorption";
  if (mkdir(TempDirectory.Data(), 0777) != 0 && errno != EEXIST) {
    Summarize();
    return false;
  }

  const MString AbsorptionFile = TempDirectory + "/atmospheric.abs";
  {
    ofstream Out(AbsorptionFile.Data());
    Passed = EvaluateTrue("ofstream::is_open()", "fixture", "The representative atmospheric absorption fixture can be opened for writing", Out.is_open() == true) && Passed;
    if (Out.is_open() == false) {
      Summarize();
      return false;
    }
    Out << "IP LIN\n";
    Out << "XA 0 10\n";
    Out << "YA 0 20\n";
    Out << "ZA 100 200\n";
    Out << "AP 0 0 100 0.10\n";
    Out << "AP 10 0 100 0.50\n";
    Out << "AP 0 20 100 0.30\n";
    Out << "AP 10 20 100 0.70\n";
    Out << "AP 0 0 200 0.20\n";
    Out << "AP 10 0 200 0.60\n";
    Out << "AP 0 20 200 0.40\n";
    Out << "AP 10 20 200 0.80\n";
    Out << "EN\n";
    Out.close();
  }

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
  Passed = Evaluate("GetTransmissionProbability()", "below altitude", "GetTransmissionProbability returns zero below the modeled altitude range",
                    Absorption.GetTransmissionProbability(-1.0, 10.0, 150.0), 0.40) && Passed;
  Passed = Evaluate("GetTransmissionProbability()", "above altitude", "GetTransmissionProbability returns zero above the modeled altitude range",
                    Absorption.GetTransmissionProbability(11.0, 10.0, 150.0), 0.50) && Passed;
  Passed = Evaluate("GetTransmissionProbability()", "below azimuth", "GetTransmissionProbability returns zero below the modeled azimuth range",
                    Absorption.GetTransmissionProbability(5.0, -1.0, 150.0), 0.30) && Passed;
  Passed = Evaluate("GetTransmissionProbability()", "above azimuth", "GetTransmissionProbability returns zero above the modeled azimuth range",
                    Absorption.GetTransmissionProbability(5.0, 25.0, 150.0), 0.60) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "below energy", "GetTransmissionProbability clamps energy below the tabulated range to the minimum energy",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 50.0), 0.40, 1e-12) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "above energy", "GetTransmissionProbability clamps energy above the tabulated range to the maximum energy",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 250.0), 0.50, 1e-12) && Passed;
  Passed = EvaluateTrue("Read()", "reuse", "MAtmosphericAbsorption can be re-read on the same instance", Absorption.Read(AbsorptionFile)) && Passed;
  Passed = EvaluateNear("GetTransmissionProbability()", "reuse", "Re-reading the representative fixture preserves the representative interpolated transmission probability",
                        Absorption.GetTransmissionProbability(5.0, 10.0, 150.0), 0.45, 1e-12) && Passed;

  DisableDefaultStreams();
  MAtmosphericAbsorption Missing;
  Passed = EvaluateFalse("Read()", "missing file", "MAtmosphericAbsorption rejects a missing absorption file", Missing.Read(TempDirectory + "/missing.abs")) && Passed;
  EnableDefaultStreams();

  const MString MalformedFile = TempDirectory + "/malformed.abs";
  {
    ofstream Out(MalformedFile.Data());
    Passed = EvaluateTrue("ofstream::is_open()", "malformed fixture", "The malformed atmospheric absorption fixture can be opened for writing", Out.is_open() == true) && Passed;
    if (Out.is_open() == false) {
      Summarize();
      return false;
    }
    Out << "IP LIN\n";
    Out << "XA 0\n";
    Out << "YA 0 20\n";
    Out << "ZA 100 200\n";
    Out << "AP 0 0 100 0.10\n";
    Out.close();
  }

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
  Passed = EvaluateTrue("rmdir()", "temp cleanup", "The atmospheric-absorption temp directory can be removed", rmdir(TempDirectory.Data()) == 0) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTAtmosphericAbsorption Test;
  return Test.Run() == true ? 0 : 1;
}
