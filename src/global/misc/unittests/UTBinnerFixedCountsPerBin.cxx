/*
 * UTBinnerFixedCountsPerBin.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MBinnerFixedCountsPerBin.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerFixedCountsPerBin
class UTBinnerFixedCountsPerBin : public MUnitTest
{
public:
  UTBinnerFixedCountsPerBin() : MUnitTest("UTBinnerFixedCountsPerBin") {}
  virtual ~UTBinnerFixedCountsPerBin() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerFixedCountsPerBin::Run()
{
  bool Passed = true;

  MBinnerFixedCountsPerBin Default;
  Default.SetCountsPerBin(3);
  Default.SetMinMax(0.0, 10.0, false);
  Default.Add(1.0);
  Default.Add(2.0);
  Default.Add(3.0);
  Default.Add(4.0);
  Passed = EvaluateSize("GetBinEdges()", "representative histogram", "The fixed-count binner creates the expected representative number of edges", Default.GetBinEdges().size(), 3UL) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "first edge", "The fixed-count binner stores the representative first edge", Default.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "split edge", "The fixed-count binner stores the representative split edge", Default.GetBinEdges()[1], 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "last edge", "The fixed-count binner stores the representative last edge", Default.GetBinEdges()[2], 10.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetBinnedData()", "representative histogram", "The fixed-count binner creates the expected representative number of bins", Default.GetBinnedData().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "first bin", "The fixed-count binner stores the representative first-bin counts", Default.GetBinnedData()[0], 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "second bin", "The fixed-count binner stores the representative second-bin counts", Default.GetBinnedData()[1], 1.0, 1e-12) && Passed;

  MBinnerFixedCountsPerBin Weighted;
  Weighted.SetCountsPerBin(3);
  Weighted.SetMinMax(0.0, 10.0, false);
  Weighted.Add(1.0, 1.5);
  Weighted.Add(2.0, 1.5);
  Weighted.Add(11.0, 10.0);
  Passed = EvaluateNear("Add()", "weighted representative content", "The fixed-count binner accumulates representative weighted content and ignores out-of-range values", Weighted.GetBinnedData()[0], 3.0, 1e-12) && Passed;

  MBinnerFixedCountsPerBin Adapted;
  Adapted.SetCountsPerBin(2);
  Adapted.SetMinMax(0.0, 10.0, true);
  Adapted.Add(2.0);
  Adapted.Add(4.0);
  Adapted.Add(6.0);
  Passed = EvaluateNear("SetMinMax()", "adapted minimum", "Adaptive fixed-count binning uses the representative in-range minimum", Adapted.GetBinEdges()[0], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted split edge", "Adaptive fixed-count binning stores the representative adaptive split edge", Adapted.GetBinEdges()[1], 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted maximum", "Adaptive fixed-count binning uses the representative in-range maximum", Adapted.GetBinEdges()[2], 6.0, 1e-12) && Passed;

  MBinnerFixedCountsPerBin SingleElement;
  SingleElement.SetCountsPerBin(3);
  SingleElement.SetMinMax(0.0, 10.0, false);
  SingleElement.Add(4.0);
  Passed = EvaluateSize("GetBinEdges()", "single element", "A single representative element below the count threshold keeps the configured min/max edges only", SingleElement.GetBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "single element maximum edge", "A single representative element below the count threshold keeps the configured maximum edge", SingleElement.GetBinEdges()[1], 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "single element content", "A single representative element remains in the only populated fixed-count bin", SingleElement.GetBinnedData()[0], 1.0, 1e-12) && Passed;

  MBinnerFixedCountsPerBin AdaptedEmpty;
  AdaptedEmpty.SetCountsPerBin(2);
  AdaptedEmpty.SetMinMax(0.0, 10.0, true);
  Passed = EvaluateNear("SetMinMax()", "adapted empty minimum", "Adaptive fixed-count binning without representative data keeps the configured minimum", AdaptedEmpty.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted empty maximum", "Adaptive fixed-count binning without representative data keeps the configured maximum", AdaptedEmpty.GetBinEdges()[1], 10.0, 1e-12) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerFixedCountsPerBin Test;
  return Test.Run() == true ? 0 : 1;
}
