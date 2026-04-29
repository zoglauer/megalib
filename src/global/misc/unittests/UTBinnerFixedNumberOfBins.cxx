/*
 * UTBinnerFixedNumberOfBins.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MBinnerFixedNumberOfBins.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerFixedNumberOfBins
class UTBinnerFixedNumberOfBins : public MUnitTest
{
public:
  UTBinnerFixedNumberOfBins() : MUnitTest("UTBinnerFixedNumberOfBins") {}
  virtual ~UTBinnerFixedNumberOfBins() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerFixedNumberOfBins::Run()
{
  bool Passed = true;

  MBinnerFixedNumberOfBins Default;
  Default.SetNumberOfBins(2);
  Default.SetMinMax(0.0, 4.0, false);
  Default.Add(0.5);
  Default.Add(1.5);
  Default.Add(3.5);
  Passed = EvaluateSize("GetBinEdges()", "representative histogram", "The fixed-number binner creates the expected representative number of bin edges", Default.GetBinEdges().size(), 3UL) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "first edge", "The fixed-number binner stores the representative first edge", Default.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "middle edge", "The fixed-number binner stores the representative middle edge", Default.GetBinEdges()[1], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "last edge", "The fixed-number binner stores the representative last edge", Default.GetBinEdges()[2], 4.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetBinnedData()", "representative histogram", "The fixed-number binner creates the expected representative number of bins", Default.GetBinnedData().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "first bin", "The fixed-number binner stores the representative first-bin counts", Default.GetBinnedData()[0], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "second bin", "The fixed-number binner stores the representative second-bin counts", Default.GetBinnedData()[1], 1.0, 1e-12) && Passed;

  MBinnerFixedNumberOfBins Interior;
  Interior.SetNumberOfBins(3);
  Interior.SetMinMax(-1.5, 4.5, false);
  Interior.Add(-0.75);
  Interior.Add(0.25);
  Interior.Add(2.25);
  Interior.Add(3.75);
  Passed = EvaluateNear("GetBinEdges()", "interior first edge", "The fixed-number binner stores the representative non-round first edge", Interior.GetBinEdges()[0], -1.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "interior split edge", "The fixed-number binner stores the representative non-round interior split edge", Interior.GetBinEdges()[1], 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "interior middle bin", "The fixed-number binner stores representative non-round interior content in the middle bin", Interior.GetBinnedData()[1], 1.0, 1e-12) && Passed;

  MBinnerFixedNumberOfBins Clamped;
  Clamped.SetNumberOfBins(0);
  Clamped.SetMinMax(0.0, 4.0, false);
  Clamped.Add(1.0);
  Passed = EvaluateSize("SetNumberOfBins()", "zero bins", "SetNumberOfBins clamps a representative zero request to one bin", Clamped.GetBinnedData().size(), 1UL) && Passed;

  MBinnerFixedNumberOfBins Aligned;
  Aligned.SetNumberOfBins(2);
  Aligned.AlignBins(true);
  Aligned.SetMinMax(0.2, 3.8, false);
  Aligned.Add(0.5);
  Aligned.Add(2.5);
  Passed = EvaluateNear("AlignBins()", "aligned minimum", "AlignBins shifts the representative minimum to the lower integer", Aligned.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("AlignBins()", "aligned middle", "AlignBins keeps the representative integer-width midpoint", Aligned.GetBinEdges()[1], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("AlignBins()", "aligned maximum", "AlignBins shifts the representative maximum to the next integer", Aligned.GetBinEdges()[2], 4.0, 1e-12) && Passed;

  MBinnerFixedNumberOfBins Adapted;
  Adapted.SetNumberOfBins(2);
  Adapted.SetMinMax(0.0, 10.0, true);
  Adapted.Add(2.0);
  Adapted.Add(8.0);
  Passed = EvaluateNear("SetMinMax()", "adapted minimum", "Adaptive fixed-number binning uses the representative in-range minimum", Adapted.GetBinEdges()[0], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted maximum", "Adaptive fixed-number binning uses the representative in-range maximum", Adapted.GetBinEdges()[2], 8.0, 1e-12) && Passed;

  MBinnerFixedNumberOfBins AdaptedEmpty;
  AdaptedEmpty.SetNumberOfBins(2);
  AdaptedEmpty.SetMinMax(0.0, 10.0, true);
  Passed = EvaluateNear("SetMinMax()", "adapted empty minimum", "Adaptive fixed-number binning without representative data keeps the configured minimum", AdaptedEmpty.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted empty maximum", "Adaptive fixed-number binning without representative data keeps the configured maximum", AdaptedEmpty.GetBinEdges()[2], 10.0, 1e-12) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerFixedNumberOfBins Test;
  return Test.Run() == true ? 0 : 1;
}
