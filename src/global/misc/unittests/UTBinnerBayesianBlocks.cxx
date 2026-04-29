/*
 * UTBinnerBayesianBlocks.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// MEGAlib:
#include "MBinnerBayesianBlocks.h"
#include "MExceptions.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerBayesianBlocks
class UTBinnerBayesianBlocks : public MUnitTest
{
public:
  UTBinnerBayesianBlocks() : MUnitTest("UTBinnerBayesianBlocks") {}
  virtual ~UTBinnerBayesianBlocks() {}

  virtual bool Run();
};

////////////////////////////////////////////////////////////////////////////////


bool UTBinnerBayesianBlocks::Run()
{
  bool Passed = true;
  Passed = EvaluateException<MExceptionTestFailed>("SetMinimumBinWidth()", "zero width", "A representative zero minimum bin width is rejected immediately", [&](){ MBinnerBayesianBlocks Invalid; Invalid.SetMinimumBinWidth(0.0); }) && Passed;

  DisableDefaultStreams();

  MBinnerBayesianBlocks Representative;
  Representative.SetMinMax(0.0, 10.0, false);
  Representative.SetMinimumBinWidth(2.0);
  Representative.AddUnsorted(vector<double>{1.0, 2.0, 8.0, 9.0});
  Passed = EvaluateTrue("GetBinEdges()", "representative output", "The Bayesian-block binner creates at least one representative bin edge", Representative.GetBinEdges().size() >= 2) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "representative first edge", "The Bayesian-block binner keeps the representative minimum edge", Representative.GetBinEdges().front(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetBinnedData()", "representative output", "The Bayesian-block binner creates one representative content entry per bin", Representative.GetBinnedData().size(), Representative.GetBinEdges().size() - 1) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "representative sum", "The Bayesian-block binner preserves the representative total counts", Representative.GetBinnedData()[0], 4.0, 1e-12) && Passed;

  MBinnerBayesianBlocks Interior;
  Interior.SetMinMax(-1.0, 9.0, false);
  Interior.SetMinimumBinWidth(1.5);
  Interior.AddUnsorted(vector<double>{0.125, 0.375, 4.625, 4.875, 7.25});
  Passed = EvaluateNear("GetBinEdges()", "interior first edge", "The Bayesian-block binner keeps the representative non-round first edge", Interior.GetBinEdges().front(), -1.0, 1e-12) && Passed;
  Passed = Evaluate("GetBinnedData()", "interior output", "The Bayesian-block binner keeps one representative content entry per non-round interior bin", Interior.GetBinnedData().size(), Interior.GetBinEdges().size() - 1) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "interior sum", "The Bayesian-block binner preserves the representative non-round total counts", Interior.GetBinnedData()[0], 5.0, 1e-12) && Passed;

  MBinnerBayesianBlocks WideBins;
  WideBins.SetMinMax(0.0, 10.0, false);
  WideBins.SetMinimumBinWidth(20.0);
  WideBins.AddUnsorted(vector<double>{1.0, 2.0, 3.0});
  Passed = EvaluateTrue("SetMinimumBinWidth()", "large width", "A large representative minimum bin width still produces a non-empty bin-edge vector", WideBins.GetBinEdges().size() >= 1) && Passed;
  Passed = EvaluateNear("SetMinimumBinWidth()", "large width last edge", "A large representative minimum bin width pushes the last edge beyond the requested maximum", WideBins.GetBinEdges().back(), 20.0, 1e-12) && Passed;

  MBinnerBayesianBlocks LowPrior;
  LowPrior.SetMinMax(0.0, 10.0, false);
  LowPrior.SetMinimumBinWidth(1.0);
  LowPrior.SetPrior(0.0);
  LowPrior.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 8.0, 8.1, 8.2});

  MBinnerBayesianBlocks HighPrior;
  HighPrior.SetMinMax(0.0, 10.0, false);
  HighPrior.SetMinimumBinWidth(1.0);
  HighPrior.SetPrior(100.0);
  HighPrior.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 8.0, 8.1, 8.2});
  Passed = Evaluate("SetPrior()", "representative monotonicity", "A larger representative prior does not create more bins than a smaller prior", HighPrior.GetBinEdges().size() <= LowPrior.GetBinEdges().size(), true) && Passed;

  MBinnerBayesianBlocks MinimumCounts;
  MinimumCounts.SetMinMax(0.0, 10.0, false);
  MinimumCounts.SetMinimumBinWidth(1.0);
  MinimumCounts.SetMinimumCountsPerBin(3.0);
  MinimumCounts.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 8.0});
  Passed = EvaluateTrue("SetMinimumCountsPerBin()", "representative merged bins", "The minimum-counts setting produces representative bins with at least the requested counts", MinimumCounts.GetBinnedData().size() >= 1) && Passed;
  if (MinimumCounts.GetBinnedData().size() > 0) {
    Passed = Evaluate("SetMinimumCountsPerBin()", "representative merged bins", "The first representative Bayesian block satisfies the minimum counts after merging", MinimumCounts.GetBinnedData()[0] >= 3.0, true) && Passed;
  }

  MBinnerBayesianBlocks Adapted;
  Adapted.SetMinMax(0.0, 10.0, true);
  Adapted.SetMinimumBinWidth(1.0);
  Adapted.AddUnsorted(vector<double>{2.0, 2.1, 7.9, 8.0});
  Passed = EvaluateNear("SetMinMax()", "adapted minimum", "Adaptive Bayesian-block binning keeps the representative in-range minimum", Adapted.GetBinEdges().front(), 2.0, 1e-12) && Passed;

  MBinnerBayesianBlocks AdaptedInterior;
  AdaptedInterior.SetMinMax(0.0, 10.0, true);
  AdaptedInterior.SetMinimumBinWidth(0.5);
  AdaptedInterior.AddUnsorted(vector<double>{1.125, 1.375, 8.625, 8.875});
  Passed = EvaluateNear("SetMinMax()", "adapted interior minimum", "Adaptive Bayesian-block binning keeps the representative non-round in-range minimum", AdaptedInterior.GetBinEdges().front(), 1.125, 1e-12) && Passed;

  EnableDefaultStreams();

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerBayesianBlocks Test;
  return Test.Run() == true ? 0 : 1;
}
