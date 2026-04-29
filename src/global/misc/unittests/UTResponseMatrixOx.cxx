/*
 * UTResponseMatrixOx.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MResponseMatrixO1.h"
#include "UTResponseMatrixOrderShared.h"


//! Unit test class for MResponseMatrixOx
class UTResponseMatrixOx : public MUnitTest
{
public:
  UTResponseMatrixOx() : MUnitTest("UTResponseMatrixOx") {}
  virtual ~UTResponseMatrixOx() {}

  virtual bool Run();

private:
  //! Test helper exposing protected MResponseMatrixOx functionality
  class ResponseMatrixOxTest : public MResponseMatrixO1
  {
  public:
    ResponseMatrixOxTest() : MResponseMatrixO1() {}
    virtual ~ResponseMatrixOxTest() {}

    using MResponseMatrixOx::AreIncreasing;
    using MResponseMatrixOx::FindBin;
    using MResponseMatrixOx::FindBinCentered;
    using MResponseMatrixOx::IsIncreasing;
  };
};


////////////////////////////////////////////////////////////////////////////////


bool UTResponseMatrixOx::Run()
{
  bool Passed = true;

  vector<float> Increasing{0.0f, 1.0f, 2.0f};
  vector<float> NonIncreasing{0.0f, 1.0f, 1.0f};

  ResponseMatrixOxTest Matrix;
  Passed = Evaluate("AreValuesCentered()", "default constructor", "Representative response matrices are value-centered by default", Matrix.AreValuesCentered(), true) && Passed;
  Matrix.SetValuesCenteredFlag(false);
  Passed = Evaluate("SetValuesCenteredFlag()", "representative flag", "The centered-values flag can be changed explicitly", Matrix.AreValuesCentered(), false) && Passed;
  Matrix.SetValuesCenteredFlag(true);

  Passed = Evaluate("IsIncreasing()", "increasing axis", "Increasing axes are recognized correctly", Matrix.IsIncreasing(Increasing), true) && Passed;
  Passed = Evaluate("IsIncreasing()", "non-increasing axis", "Non-increasing axes are rejected correctly", Matrix.IsIncreasing(NonIncreasing), false) && Passed;

  Passed = Evaluate("FindBin()", "interior value", "FindBin locates a representative value in the first bin", Matrix.FindBin(Increasing, 0.25f), 0) && Passed;
  Passed = Evaluate("FindBin()", "underflow", "FindBin returns -1 below the first axis edge", Matrix.FindBin(Increasing, -0.1f), -1) && Passed;
  Passed = Evaluate("FindBin()", "overflow", "FindBin returns the axis size at or above the last edge", Matrix.FindBin(Increasing, 2.0f), 3) && Passed;
  Passed = Evaluate("FindBinCentered()", "first center", "FindBinCentered locates a representative first-bin center", Matrix.FindBinCentered(Increasing, 0.5f), 0) && Passed;
  Passed = Evaluate("FindBinCentered()", "underflow", "FindBinCentered returns -1 below the centered first bin", Matrix.FindBinCentered(Increasing, 0.1f), -1) && Passed;

  Passed = Evaluate("AreIncreasing()", "ordered axes", "Representative ordered axis selections are accepted", Matrix.AreIncreasing(3, 1, 2, 3), true) && Passed;
  Passed = Evaluate("AreIncreasing()", "duplicate axis selection", "Duplicate axis selections are rejected", Matrix.AreIncreasing(3, 1, 1, 3), false) && Passed;
  Passed = Evaluate("AreIncreasing()", "out-of-range selection", "Out-of-range axis selections are rejected", Matrix.AreIncreasing(3, 1, 4, 0), false) && Passed;

  Matrix.SetName("Representative");
  Matrix.SetAxis(Increasing);
  Matrix.SetAxisNames("X1");
  Matrix.SetBinContent(0, 5.0f);
  Matrix.SetBinContent(1, 1.0f);
  Matrix.SetSimulatedEvents(42);
  Matrix.SetFarFieldStartArea(3.5);
  Matrix.SetSpectrum("Mono", {2.0, 7.0});
  MString Statistics = Matrix.GetStatistics();
  Passed = EvaluateTrue("GetStatistics()", "representative string", "The statistics string includes the representative matrix name", Statistics.Contains("Representative")) && Passed;
  Passed = EvaluateTrue("GetStatistics()", "representative axes", "The statistics string includes the representative axis name", Statistics.Contains("X1")) && Passed;
  Passed = EvaluateTrue("GetStatistics()", "representative sum", "The statistics string includes the representative sum", Statistics.Contains("Sum:                    6")) && Passed;

  Passed = EvaluateTrue("PrepareResponseMatrixTempDirectory()", "read setup", "The temporary response-matrix directory exists for the base-class read test", PrepareResponseMatrixTempDirectory()) && Passed;
  MString FileName = "/tmp/UTResponseMatrix/UTResponseMatrixOx.rsp";
  Passed = Evaluate("Write()", "base read setup", "A representative order-1 matrix can be written for the base-class read test", Matrix.Write(FileName, true), true) && Passed;
  ResponseMatrixOxTest ReadBack;
  Passed = Evaluate("Read()", "base class", "The base response-matrix read implementation initializes the representative derived matrix", ReadBack.Read(FileName), true) && Passed;
  Passed = EvaluateNear("Read()", "base class content", "The base response-matrix read implementation restores the representative first-bin content", ReadBack.GetBinContent(0), 5.0, 1e-6) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTResponseMatrixOx Test;
  return Test.Run() == true ? 0 : 1;
}
