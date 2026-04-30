/*
 * UTRotationInterface.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MRotationInterface.h"
#include "MUnitTest.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for MRotationInterface
class UTRotationInterface : public MUnitTest
{
public:
  UTRotationInterface() : MUnitTest("UTRotationInterface") {}
  virtual ~UTRotationInterface() {}

  virtual bool Run();

private:
  bool TestDefaultsAndSetters();
  bool TestParseAndStream();
  bool TestBinaryRoundTrip();
  bool TestGalacticRotationCache();
};


////////////////////////////////////////////////////////////////////////////////


bool UTRotationInterface::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndSetters() && Passed;
  Passed = TestParseAndStream() && Passed;
  Passed = TestBinaryRoundTrip() && Passed;
  Passed = TestGalacticRotationCache() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRotationInterface::TestDefaultsAndSetters()
{
  bool Passed = true;

  MRotationInterface Rotation;
  Passed = EvaluateFalse("HasGalacticPointing()", "default", "Default construction starts without galactic pointing", Rotation.HasGalacticPointing()) && Passed;
  Passed = EvaluateFalse("HasDetectorRotation()", "default", "Default construction starts without detector rotation", Rotation.HasDetectorRotation()) && Passed;
  Passed = EvaluateFalse("HasHorizonPointing()", "default", "Default construction starts without horizon pointing", Rotation.HasHorizonPointing()) && Passed;
  Passed = EvaluateTrue("GetGalacticPointingXAxis()", "default", "Default galactic x axis is the representative cartesian x axis", Rotation.GetGalacticPointingXAxis().AreEqual(MVector(1.0, 0.0, 0.0), 1e-12)) && Passed;
  Passed = EvaluateTrue("GetGalacticPointingZAxis()", "default", "Default galactic z axis is the representative cartesian z axis", Rotation.GetGalacticPointingZAxis().AreEqual(MVector(0.0, 0.0, 1.0), 1e-12)) && Passed;

  Rotation.SetGalacticPointingXAxis(123.4, -21.5);
  Rotation.SetGalacticPointingZAxis(281.2, 52.25);
  Passed = EvaluateTrue("HasGalacticPointing()", "set by angles", "Setting representative galactic axes enables galactic pointing", Rotation.HasGalacticPointing()) && Passed;
  Passed = EvaluateNear("GetGalacticPointingXAxisLongitude()", "interior longitude", "Galactic x longitude round-trips representative interior values", Rotation.GetGalacticPointingXAxisLongitude()*c_Deg, 123.4, 1e-10) && Passed;
  Passed = EvaluateNear("GetGalacticPointingXAxisLatitude()", "interior latitude", "Galactic x latitude round-trips representative interior values", Rotation.GetGalacticPointingXAxisLatitude()*c_Deg, -21.5, 1e-10) && Passed;
  Passed = EvaluateNear("GetGalacticPointingZAxisLongitude()", "interior longitude", "Galactic z longitude round-trips representative interior values", Rotation.GetGalacticPointingZAxisLongitude()*c_Deg, 281.2, 1e-10) && Passed;
  Passed = EvaluateNear("GetGalacticPointingZAxisLatitude()", "interior latitude", "Galactic z latitude round-trips representative interior values", Rotation.GetGalacticPointingZAxisLatitude()*c_Deg, 52.25, 1e-10) && Passed;

  Rotation.SetDetectorPointingXAxis(47.5, 76.0);
  Rotation.SetDetectorPointingZAxis(128.0, 23.0);
  Passed = EvaluateTrue("HasDetectorRotation()", "set by angles", "Setting representative detector axes enables detector rotation", Rotation.HasDetectorRotation()) && Passed;
  Passed = EvaluateNear("GetDetectorRotationXAxis().Phi()", "interior phi", "Detector x phi round-trips representative interior values", Rotation.GetDetectorRotationXAxis().Phi()*c_Deg, 47.5, 1e-10) && Passed;
  Passed = EvaluateNear("GetDetectorRotationXAxis().Theta()", "interior theta", "Detector x theta round-trips representative interior values", Rotation.GetDetectorRotationXAxis().Theta()*c_Deg, 76.0, 1e-10) && Passed;

  Rotation.SetHorizonPointingXAxis(15.5, 32.25);
  Rotation.SetHorizonPointingZAxis(200.0, -18.0);
  Passed = EvaluateTrue("HasHorizonPointing()", "set by angles", "Setting representative horizon axes enables horizon pointing", Rotation.HasHorizonPointing()) && Passed;
  Passed = EvaluateNear("GetHorizonPointingXAxisAzimuthNorth()", "interior azimuth", "Horizon x azimuth round-trips representative interior values", Rotation.GetHorizonPointingXAxisAzimuthNorth()*c_Deg, 15.5, 1e-10) && Passed;
  Passed = EvaluateNear("GetHorizonPointingXAxisElevation()", "interior elevation", "Horizon x elevation round-trips representative interior values", Rotation.GetHorizonPointingXAxisElevation()*c_Deg, 32.25, 1e-10) && Passed;
  Passed = EvaluateNear("GetHorizonPointingZAxisAzimuthNorth()", "interior azimuth", "Horizon z azimuth round-trips representative interior values", Rotation.GetHorizonPointingZAxisAzimuthNorth()*c_Deg, 200.0, 1e-10) && Passed;
  Passed = EvaluateNear("GetHorizonPointingZAxisElevation()", "interior elevation", "Horizon z elevation round-trips representative interior values", Rotation.GetHorizonPointingZAxisElevation()*c_Deg, -18.0, 1e-10) && Passed;

  Rotation.Reset();
  Passed = EvaluateFalse("Reset()", "flags", "Reset clears galactic pointing again", Rotation.HasGalacticPointing()) && Passed;
  Passed = EvaluateFalse("Reset()", "flags", "Reset clears detector rotation again", Rotation.HasDetectorRotation()) && Passed;
  Passed = EvaluateFalse("Reset()", "flags", "Reset clears horizon pointing again", Rotation.HasHorizonPointing()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRotationInterface::TestParseAndStream()
{
  bool Passed = true;

  MRotationInterface Rotation;
  Passed = Evaluate("ParseLine()", "RX line", "ParseLine accepts representative detector x axis text", Rotation.ParseLine("RX 0.5 0.25 0.75"), 0) && Passed;
  Passed = Evaluate("ParseLine()", "RZ line", "ParseLine accepts representative detector z axis text", Rotation.ParseLine("RZ -0.5 0.75 0.25"), 0) && Passed;
  Passed = Evaluate("ParseLine()", "GX line", "ParseLine accepts representative galactic x axis text", Rotation.ParseLine("GX 123.5 -21.25"), 0) && Passed;
  Passed = Evaluate("ParseLine()", "GZ line", "ParseLine accepts representative galactic z axis text", Rotation.ParseLine("GZ 281.5 52.75"), 0) && Passed;
  Passed = Evaluate("ParseLine()", "HX line", "ParseLine accepts representative horizon x axis text", Rotation.ParseLine("HX 15.5 32.25"), 0) && Passed;
  Passed = Evaluate("ParseLine()", "HZ line", "ParseLine accepts representative horizon z axis text", Rotation.ParseLine("HZ 200.0 -18.0"), 0) && Passed;

  ostringstream Streamed;
  Rotation.Stream(Streamed);
  MString Text = Streamed.str();
  Passed = EvaluateTrue("Stream()", "contains RX", "Stream contains the representative detector x line", Text.Contains("RX 0.5 0.25 0.75")) && Passed;
  Passed = EvaluateTrue("Stream()", "contains RZ", "Stream contains the representative detector z line", Text.Contains("RZ -0.5 0.75 0.25")) && Passed;
  Passed = EvaluateTrue("Stream()", "contains GX", "Stream contains the representative galactic x line", Text.Contains("GX 123.5 -21.25")) && Passed;
  Passed = EvaluateTrue("Stream()", "contains GZ", "Stream contains the representative galactic z line", Text.Contains("GZ 281.5 52.75")) && Passed;
  Passed = EvaluateTrue("Stream()", "contains HX", "Stream contains the representative horizon x line", Text.Contains("HX 15.5 32.25")) && Passed;

  istringstream Input(Streamed.str());
  MString Line;
  MRotationInterface RoundTrip;
  while (Line.ReadLine(Input)) {
    if (Line.IsEmpty() == false) {
      Passed = Evaluate("ParseLine()", "round-trip stream line", "All representative streamed rotation lines can be parsed again", RoundTrip.ParseLine(Line), 0) && Passed;
    }
  }
  Passed = EvaluateNear("Stream()/ParseLine()", "horizon z azimuth", "Streaming and reparsing preserve the representative horizon z azimuth", RoundTrip.GetHorizonPointingZAxisAzimuthNorth()*c_Deg, Rotation.GetHorizonPointingZAxisAzimuthNorth()*c_Deg, 1e-10) && Passed;
  Passed = EvaluateNear("Stream()/ParseLine()", "horizon z elevation", "Streaming and reparsing preserve the representative horizon z elevation", RoundTrip.GetHorizonPointingZAxisElevation()*c_Deg, Rotation.GetHorizonPointingZAxisElevation()*c_Deg, 1e-10) && Passed;

  MRotationInterface Fast;
  Passed = Evaluate("ParseLine(..., true)", "fast GX", "Fast parsing accepts representative galactic text", Fast.ParseLine("GX 33.75 11.5", true), 0) && Passed;
  Passed = EvaluateNear("ParseLine(..., true)", "fast GX longitude", "Fast parsing stores the representative galactic longitude", Fast.GetGalacticPointingXAxisLongitude()*c_Deg, 33.75, 1e-10) && Passed;
  Passed = EvaluateNear("ParseLine(..., true)", "fast GX latitude", "Fast parsing stores the representative galactic latitude", Fast.GetGalacticPointingXAxisLatitude()*c_Deg, 11.5, 1e-10) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRotationInterface::TestBinaryRoundTrip()
{
  bool Passed = true;

  {
    MRotationInterface Original;
    Original.SetGalacticPointingXAxis(123.25, -21.75);
    Original.SetGalacticPointingZAxis(281.5, 52.125);
    Original.SetDetectorRotationXAxis(MVector(0.25, 0.5, 0.75));
    Original.SetDetectorRotationZAxis(MVector(-0.5, 0.75, 0.25));
    Original.SetHorizonPointingXAxis(15.5, 32.25);
    Original.SetHorizonPointingZAxis(200.0, -18.0);

    MBinaryStore Store;
    Passed = EvaluateTrue("ToBinary()", "32-bit", "32-bit binary output succeeds for representative interior data", Original.ToBinary(Store, 32)) && Passed;

    MRotationInterface Parsed;
    Passed = EvaluateTrue("ParseBinary()", "32-bit", "32-bit binary input succeeds for representative interior data", Parsed.ParseBinary(Store, true, true, true, 32)) && Passed;
    Passed = EvaluateNear("ParseBinary()", "32-bit galactic x longitude", "32-bit binary round-trips the representative galactic x longitude", Parsed.GetGalacticPointingXAxisLongitude()*c_Deg, 123.25, 1e-4) && Passed;
    Passed = EvaluateNear("ParseBinary()", "32-bit galactic x latitude", "32-bit binary round-trips the representative galactic x latitude", Parsed.GetGalacticPointingXAxisLatitude()*c_Deg, -21.75, 1e-4) && Passed;
    Passed = EvaluateTrue("ParseBinary()", "32-bit detector x vector", "32-bit binary round-trips the representative detector x axis", Parsed.GetDetectorRotationXAxis().AreEqual(MVector(0.25, 0.5, 0.75), 1e-6)) && Passed;
    Passed = EvaluateNear("ParseBinary()", "32-bit horizon z elevation", "32-bit binary round-trips the representative horizon z elevation", Parsed.GetHorizonPointingZAxisElevation()*c_Deg, -18.0, 1e-4) && Passed;
  }

  {
    MRotationInterface Original;
    Original.SetGalacticPointingXAxis(123.456789123, -21.654321987);
    Original.SetGalacticPointingZAxis(281.987654321, 52.123456789);
    Original.SetDetectorRotationXAxis(MVector(0.123456789, 0.234567891, 0.345678912));
    Original.SetDetectorRotationZAxis(MVector(-0.456789123, 0.567891234, 0.678912345));
    Original.SetHorizonPointingXAxis(15.987654321, 32.123456789);
    Original.SetHorizonPointingZAxis(200.876543219, -18.234567891);

    MBinaryStore Store;
    Passed = EvaluateTrue("ToBinary()", "64-bit", "64-bit binary output succeeds for representative interior data", Original.ToBinary(Store, 64)) && Passed;

    MRotationInterface Parsed;
    Passed = EvaluateTrue("ParseBinary()", "64-bit", "64-bit binary input succeeds for representative interior data", Parsed.ParseBinary(Store, true, true, true, 64)) && Passed;
    Passed = EvaluateNear("ParseBinary()", "64-bit galactic x longitude", "64-bit binary round-trips the representative galactic x longitude", Parsed.GetGalacticPointingXAxisLongitude()*c_Deg, 123.456789123, 1e-10) && Passed;
    Passed = EvaluateNear("ParseBinary()", "64-bit galactic x latitude", "64-bit binary round-trips the representative galactic x latitude", Parsed.GetGalacticPointingXAxisLatitude()*c_Deg, -21.654321987, 1e-10) && Passed;
    Passed = EvaluateNear("ParseBinary()", "64-bit galactic z latitude", "64-bit binary round-trips the representative galactic z latitude", Parsed.GetGalacticPointingZAxisLatitude()*c_Deg, 52.123456789, 1e-10) && Passed;
    Passed = EvaluateTrue("ParseBinary()", "64-bit detector z vector", "64-bit binary round-trips the representative detector z axis", Parsed.GetDetectorRotationZAxis().AreEqual(MVector(-0.456789123, 0.567891234, 0.678912345), 1e-12)) && Passed;
    Passed = EvaluateNear("ParseBinary()", "64-bit horizon x azimuth", "64-bit binary round-trips the representative horizon x azimuth", Parsed.GetHorizonPointingXAxisAzimuthNorth()*c_Deg, 15.987654321, 1e-10) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTRotationInterface::TestGalacticRotationCache()
{
  bool Passed = true;

  {
    MRotationInterface Rotation;
    Rotation.SetGalacticPointingXAxis(MVector(1.0, 0.0, 0.0));
    Rotation.SetGalacticPointingZAxis(MVector(0.0, 0.0, 1.0));
    Rotation.GetGalacticPointingRotationMatrix();

    Rotation.SetGalacticPointingXAxis(MVector(0.0, 1.0, 0.0));
    Rotation.SetGalacticPointingZAxis(MVector(0.0, 0.0, 1.0));

    Passed = EvaluateTrue("GetGalacticPointingRotationMatrix()", "vector setter cache refresh", "Changing galactic axes through representative vector setters refreshes the cached rotation matrix", Rotation.GetGalacticPointingRotationMatrix().GetX().AreEqual(MVector(0.0, 1.0, 0.0), 1e-12)) && Passed;
  }

  {
    MRotationInterface Source;
    Source.SetGalacticPointingXAxis(123.0, -21.0);
    Source.SetGalacticPointingZAxis(281.0, 52.0);

    MRotationInterface Target;
    Target.SetGalacticPointingXAxis(MVector(1.0, 0.0, 0.0));
    Target.SetGalacticPointingZAxis(MVector(0.0, 0.0, 1.0));
    Target.GetGalacticPointingRotationMatrix();
    Target.Set(Source);

    Passed = EvaluateTrue("Set(MRotationInterface&)", "cache refresh", "Copying representative galactic data into an already-cached target refreshes the cached rotation matrix", Target.GetGalacticPointingRotationMatrix().GetX().AreEqual(Source.GetGalacticPointingXAxis(), 1e-12)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTRotationInterface Test;
  return Test.Run() == true ? 0 : 1;
}
