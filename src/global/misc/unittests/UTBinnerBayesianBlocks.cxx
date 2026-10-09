/*
 * UTBinnerBayesianBlocks.cxx
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

private:
  //! Compare the bin edges and the content of a binner with the expected values
  bool CheckBins(const MString& Function, const MString& Input, MBinnerBayesianBlocks& Binner, const vector<double>& Edges, const vector<double>& Counts);
};

////////////////////////////////////////////////////////////////////////////////


bool UTBinnerBayesianBlocks::CheckBins(const MString& Function, const MString& Input, MBinnerBayesianBlocks& Binner, const vector<double>& Edges, const vector<double>& Counts)
{
  bool Passed = true;

  // Enable the default streams - the binner prints a message when it fails
  DisableDefaultStreams();
  const vector<double> BinEdges = Binner.GetBinEdges();
  const vector<double> BinnedData = Binner.GetBinnedData();
  EnableDefaultStreams();

  Passed = EvaluateSize(Function, Input, "The Bayesian-block binner creates the expected number of bin edges", BinEdges.size(), Edges.size()) && Passed;
  Passed = EvaluateSize(Function, Input, "The Bayesian-block binner creates the expected number of content entries", BinnedData.size(), Counts.size()) && Passed;

  // Edges and counts are exactly representable
  if (BinEdges.size() == Edges.size()) {
    for (unsigned int i = 0; i < Edges.size(); ++i) {
      Passed = EvaluateNear(Function, Input + ", edge " + to_string(i), "The Bayesian-block binner places the bin edge at the expected position", BinEdges[i], Edges[i], 1e-12) && Passed;
    }
  }
  if (BinnedData.size() == Counts.size()) {
    for (unsigned int i = 0; i < Counts.size(); ++i) {
      Passed = EvaluateNear(Function, Input + ", bin " + to_string(i), "The Bayesian-block binner fills the bin with the expected counts", BinnedData[i], Counts[i], 1e-12) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerBayesianBlocks::Run()
{
  // Expected: maximum of sum_k N_k*(ln N_k - ln T_k) - Prior over all partitions of the cells, brute force with an independent script
  // Edges are Minimum + k*MinimumBinWidth up to the first edge >= Maximum, cells are [lower, upper)

  bool Passed = true;
  Passed = EvaluateException<MExceptionTestFailed>("SetMinimumBinWidth()", "zero width", "A representative zero minimum bin width is rejected immediately", [&](){ MBinnerBayesianBlocks Invalid; Invalid.SetMinimumBinWidth(0.0); }) && Passed;
  Passed = EvaluateException<MExceptionTestFailed>("SetMinimumBinWidth()", "negative width", "A negative minimum bin width is rejected immediately", [&](){ MBinnerBayesianBlocks Invalid; Invalid.SetMinimumBinWidth(-1.0); }) && Passed;

  // Cells [0,2) ... [8,10) hold 1, 1, 0, 0, 2 counts - one block: -7.665, next best {0, 8, 10}: -10.773
  MBinnerBayesianBlocks Representative;
  Representative.SetMinMax(0.0, 10.0, false);
  Representative.SetMinimumBinWidth(2.0);
  Representative.AddUnsorted(vector<double>{1.0, 2.0, 8.0, 9.0});
  Passed = CheckBins("Histogram()", "representative", Representative, vector<double>{0.0, 10.0}, vector<double>{4.0}) && Passed;

  // Cells of width 1.5 from -1, last edge 9.5 - one block: -7.710, next best: -10.721
  MBinnerBayesianBlocks Interior;
  Interior.SetMinMax(-1.0, 9.0, false);
  Interior.SetMinimumBinWidth(1.5);
  Interior.AddUnsorted(vector<double>{0.125, 0.375, 4.625, 4.875, 7.25});
  Passed = CheckBins("Histogram()", "interior", Interior, vector<double>{-1.0, 9.5}, vector<double>{5.0}) && Passed;

  // One cell [0,20) with three counts
  MBinnerBayesianBlocks WideBins;
  WideBins.SetMinMax(0.0, 10.0, false);
  WideBins.SetMinimumBinWidth(20.0);
  WideBins.AddUnsorted(vector<double>{1.0, 2.0, 3.0});
  Passed = CheckBins("SetMinimumBinWidth()", "large width", WideBins, vector<double>{0.0, 20.0}, vector<double>{3.0}) && Passed;

  // Three counts in [1,2) and three in [8,9) - {0, 1, 2, 8, 9, 10}: 1.592, next best {0, 1, 2, 3, 8, 9, 10}: 0.592
  MBinnerBayesianBlocks LowPrior;
  LowPrior.SetMinMax(0.0, 10.0, false);
  LowPrior.SetMinimumBinWidth(1.0);
  LowPrior.SetPrior(1.0);
  LowPrior.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 8.0, 8.1, 8.2});
  Passed = CheckBins("SetPrior()", "prior 1", LowPrior, vector<double>{0.0, 1.0, 2.0, 8.0, 9.0, 10.0}, vector<double>{0.0, 3.0, 0.0, 3.0, 0.0}) && Passed;

  // Same data with prior 100 - one block: -103.065, next best: -201.726
  MBinnerBayesianBlocks HighPrior;
  HighPrior.SetMinMax(0.0, 10.0, false);
  HighPrior.SetMinimumBinWidth(1.0);
  HighPrior.SetPrior(100.0);
  HighPrior.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 8.0, 8.1, 8.2});
  Passed = CheckBins("SetPrior()", "prior 100", HighPrior, vector<double>{0.0, 10.0}, vector<double>{6.0}) && Passed;

  // Cells [0,1) ... [5,6) hold 1, 1, 1, 6, 6, 6 counts, prior 4 - {0, 3, 6}: 24.252, one block: 22.308, {0, 2, 6}: 21.605
  vector<double> TwoDensities{0.5, 1.5, 2.5};
  for (unsigned int i = 0; i < 6; ++i) {
    TwoDensities.push_back(3.1 + 0.1*i);
    TwoDensities.push_back(4.1 + 0.1*i);
    TwoDensities.push_back(5.1 + 0.1*i);
  }

  MBinnerBayesianBlocks NoMerge;
  NoMerge.SetMinMax(0.0, 6.0, false);
  NoMerge.SetMinimumBinWidth(1.0);
  NoMerge.AddUnsorted(TwoDensities);
  Passed = CheckBins("Histogram()", "two densities", NoMerge, vector<double>{0.0, 3.0, 6.0}, vector<double>{3.0, 18.0}) && Passed;

  // Minimum of 3 counts - the first block has exactly 3
  MBinnerBayesianBlocks MinimumCountsInside;
  MinimumCountsInside.SetMinMax(0.0, 6.0, false);
  MinimumCountsInside.SetMinimumBinWidth(1.0);
  MinimumCountsInside.SetMinimumCountsPerBin(3.0);
  MinimumCountsInside.AddUnsorted(TwoDensities);
  Passed = CheckBins("SetMinimumCountsPerBin()", "minimum 3, just fulfilled", MinimumCountsInside, vector<double>{0.0, 3.0, 6.0}, vector<double>{3.0, 18.0}) && Passed;

  // Minimum of 4 counts - the first block has 3 and is merged into [0,6)
  MBinnerBayesianBlocks MinimumCountsOutside;
  MinimumCountsOutside.SetMinMax(0.0, 6.0, false);
  MinimumCountsOutside.SetMinimumBinWidth(1.0);
  MinimumCountsOutside.SetMinimumCountsPerBin(4.0);
  MinimumCountsOutside.AddUnsorted(TwoDensities);
  Passed = CheckBins("SetMinimumCountsPerBin()", "minimum 4, not fulfilled", MinimumCountsOutside, vector<double>{0.0, 6.0}, vector<double>{21.0}) && Passed;

  // Blocks [0,1) [1,2) [2,5) [5,6) [6,9) [9,10) hold {0, 9, 0, 9, 0, 1} counts, minimum 5 - empty ones merge right: {9, 9, 1}, the last one merges left: {9, 10}
  MBinnerBayesianBlocks MinimumCountsLast;
  MinimumCountsLast.SetMinMax(0.0, 10.0, false);
  MinimumCountsLast.SetMinimumBinWidth(1.0);
  MinimumCountsLast.SetPrior(1.0);
  MinimumCountsLast.SetMinimumCountsPerBin(5.0);
  MinimumCountsLast.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 5.0, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 9.5});
  Passed = CheckBins("SetMinimumCountsPerBin()", "last bin too small", MinimumCountsLast, vector<double>{0.0, 2.0, 10.0}, vector<double>{9.0, 10.0}) && Passed;

  // Blocks [0,1) [1,2) [2,9) [9,10) hold {0, 9, 0, 1} counts, minimum 20 - all merge into one bin of 10 counts
  MBinnerBayesianBlocks MinimumCountsAll;
  MinimumCountsAll.SetMinMax(0.0, 10.0, false);
  MinimumCountsAll.SetMinimumBinWidth(1.0);
  MinimumCountsAll.SetPrior(1.0);
  MinimumCountsAll.SetMinimumCountsPerBin(20.0);
  MinimumCountsAll.AddUnsorted(vector<double>{1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 9.5});
  Passed = CheckBins("SetMinimumCountsPerBin()", "all bins too small", MinimumCountsAll, vector<double>{0.0, 10.0}, vector<double>{10.0}) && Passed;

  // Cells [2,3) ... [7,8) hold 2, 0, 0, 0, 0, 1 counts - one block: -6.079, next best: -8.223
  MBinnerBayesianBlocks Adapted;
  Adapted.SetMinMax(0.0, 10.0, true);
  Adapted.SetMinimumBinWidth(1.0);
  Adapted.AddUnsorted(vector<double>{2.0, 2.1, 7.9, 8.0});
  Passed = CheckBins("SetMinMax()", "adapted", Adapted, vector<double>{2.0, 8.0}, vector<double>{3.0}) && Passed;

  // Cells of width 0.5 from 1.125 to 9.125 hold 2 counts in the first and last cell - {1.125, 1.625, 8.625, 9.125}: -6.455, one block: -6.773
  MBinnerBayesianBlocks AdaptedInterior;
  AdaptedInterior.SetMinMax(0.0, 10.0, true);
  AdaptedInterior.SetMinimumBinWidth(0.5);
  AdaptedInterior.AddUnsorted(vector<double>{1.125, 1.375, 8.625, 8.875});
  Passed = CheckBins("SetMinMax()", "adapted interior", AdaptedInterior, vector<double>{1.125, 1.625, 8.625, 9.125}, vector<double>{2.0, 0.0, 2.0}) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinnerBayesianBlocks Test;
  return Test.Run() == true ? 0 : 1;
}
