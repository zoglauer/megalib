/*
 * UTBinner.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT:
#include <TCanvas.h>
#include <TH1.h>
#include <TROOT.h>

// MEGAlib:
#include "MBinner.h"
#include "MUnitTest.h"


//! Unit test class for MBinner
class UTBinner : public MUnitTest
{
public:
  UTBinner() : MUnitTest("UTBinner") {}
  virtual ~UTBinner() {}

  virtual bool Run();

private:
  //! Return the current number of ROOT canvases
  static int GetCanvasCount();
  //! Delete canvases until the requested count is reached
  static void CleanupCanvases(int TargetCount);
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinner::Run()
{
  bool Passed = true;

  MBinnedData EmptyData;
  Passed = EvaluateNear("MBinnedData()", "default axis value", "The default binned-data axis value is initialized to zero", EmptyData.m_AxisValue, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MBinnedData()", "default data value", "The default binned-data payload is initialized to zero", EmptyData.m_DataValue, 0.0, 1e-12) && Passed;

  MBinnedData WeightedData(3.5, 2.5);
  Passed = EvaluateNear("MBinnedData()", "weighted axis value", "The weighted binned-data constructor stores the representative axis value", WeightedData.m_AxisValue, 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("MBinnedData()", "weighted data value", "The weighted binned-data constructor stores the representative payload", WeightedData.m_DataValue, 2.5, 1e-12) && Passed;

  MBinnedData Lower(1.0, 5.0);
  MBinnedData Higher(2.0, 7.0);
  Passed = Evaluate("operator<()", "representative ordering", "MBinnedData compares by representative axis value", Lower < Higher, true) && Passed;
  Passed = Evaluate("operator>()", "representative ordering", "MBinnedData compares by representative axis value", Higher > Lower, true) && Passed;

  vector<MBinnedData> UnsortedData{MBinnedData(3.0), MBinnedData(1.0), MBinnedData(2.0)};
  sort(UnsortedData.begin(), UnsortedData.end(), SortBinnedData);
  Passed = EvaluateNear("SortBinnedData()", "first sorted value", "SortBinnedData orders representative axis values increasingly", UnsortedData[0].m_AxisValue, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("SortBinnedData()", "last sorted value", "SortBinnedData orders representative axis values increasingly", UnsortedData[2].m_AxisValue, 3.0, 1e-12) && Passed;

  MBinner Default;
  Passed = EvaluateSize("GetBinEdges()", "default constructor", "The default binner exposes two representative default bin edges", Default.GetBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "default minimum", "The default binner uses the representative default minimum", Default.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinEdges()", "default maximum", "The default binner uses the representative default maximum", Default.GetBinEdges()[1], 1000.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetBinnedData()", "default constructor", "The default binner exposes one representative bin", Default.GetBinnedData().size(), 1UL) && Passed;
  Passed = EvaluateNear("GetBinnedData()", "default constructor", "The default binner starts with zero representative content", Default.GetBinnedData()[0], 0.0, 1e-12) && Passed;

  TH1D* Histogram = Default.GetHistogram("Representative", "Axis", "Counts");
  Passed = EvaluateTrue("GetHistogram()", "default constructor", "GetHistogram returns a representative ROOT histogram", Histogram != 0) && Passed;
  if (Histogram != 0) {
    Passed = Evaluate("GetHistogram()", "representative title", "GetHistogram stores the representative histogram title", MString(Histogram->GetTitle()), MString("Representative")) && Passed;
    Passed = EvaluateNear("GetHistogram()", "representative content", "GetHistogram stores the representative default bin content", Histogram->GetBinContent(1), 0.0, 1e-12) && Passed;
    delete Histogram;
  }

  TH1D* NormalizedHistogram = Default.GetNormalizedHistogram("RepresentativeNormalized", "Axis", "Rate");
  Passed = EvaluateTrue("GetNormalizedHistogram()", "default constructor", "GetNormalizedHistogram returns a representative ROOT histogram", NormalizedHistogram != 0) && Passed;
  if (NormalizedHistogram != 0) {
    Passed = EvaluateNear("GetNormalizedHistogram()", "representative content", "GetNormalizedHistogram keeps zero representative content at zero", NormalizedHistogram->GetBinContent(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetNormalizedHistogram()", "representative error", "GetNormalizedHistogram normalizes the representative default bin error by width", NormalizedHistogram->GetBinError(1), 0.001, 1e-12) && Passed;
    delete NormalizedHistogram;
  }

  MBinner NormalizedWeighted;
  NormalizedWeighted.SetMinMax(0.0, 4.0, false);
  NormalizedWeighted.Add(1.0, 4.0);
  TH1D* NormalizedWeightedHistogram = NormalizedWeighted.GetNormalizedHistogram("RepresentativeNormalizedWeighted", "Axis", "Rate");
  Passed = EvaluateTrue("GetNormalizedHistogram()", "weighted content", "GetNormalizedHistogram returns a representative histogram for weighted content", NormalizedWeightedHistogram != 0) && Passed;
  if (NormalizedWeightedHistogram != 0) {
    Passed = EvaluateNear("GetNormalizedHistogram()", "weighted content", "GetNormalizedHistogram divides representative weighted content by bin width", NormalizedWeightedHistogram->GetBinContent(1), 1.0, 1e-12) && Passed;
    delete NormalizedWeightedHistogram;
  }

  MBinner Weighted;
  Weighted.Add(10.0, 2.0);
  Weighted.Add(20.0, 3.0);
  Passed = EvaluateNear("Add()", "representative weighted sum", "Add accumulates representative weighted content into the single default bin", Weighted.GetBinnedData()[0], 5.0, 1e-12) && Passed;

  MBinner Unsorted;
  Unsorted.SetMinMax(0.0, 40.0, false);
  Unsorted.AddUnsorted(vector<double>{30.0, 10.0, 20.0});
  Passed = EvaluateNear("AddUnsorted()", "representative sum", "AddUnsorted accumulates three representative unit entries", Unsorted.GetBinnedData()[0], 3.0, 1e-12) && Passed;

  MBinner Adapted;
  Adapted.Add(2.0);
  Adapted.Add(8.0);
  Adapted.SetMinMax(0.0, 10.0, true);
  Passed = EvaluateNear("SetMinMax()", "adapted minimum", "Adaptive min/max uses the representative in-range minimum", Adapted.GetBinEdges()[0], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted maximum", "Adaptive min/max uses the representative in-range maximum", Adapted.GetBinEdges()[1], 8.0, 1e-12) && Passed;

  MBinner AdaptedEmpty;
  AdaptedEmpty.SetMinMax(0.0, 10.0, true);
  Passed = EvaluateNear("SetMinMax()", "adapted empty minimum", "Adaptive min/max without representative data keeps the configured minimum", AdaptedEmpty.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "adapted empty maximum", "Adaptive min/max without representative data keeps the configured maximum", AdaptedEmpty.GetBinEdges()[1], 10.0, 1e-12) && Passed;

  MBinner InvalidMinMax;
  InvalidMinMax.Add(5.0);
  InvalidMinMax.SetMinMax(0.0, 10.0, false);
  InvalidMinMax.SetMinMax(10.0, 0.0, false);
  Passed = EvaluateNear("SetMinMax()", "invalid minimum remains unchanged", "Invalid min/max input leaves the representative minimum unchanged", InvalidMinMax.GetBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinMax()", "invalid maximum remains unchanged", "Invalid min/max input leaves the representative maximum unchanged", InvalidMinMax.GetBinEdges()[1], 10.0, 1e-12) && Passed;

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    Default.DrawNormalizedHistogram("RepresentativeDraw", "Axis", "Rate");
    Passed = Evaluate("DrawNormalizedHistogram()", "representative display", "DrawNormalizedHistogram creates a representative ROOT canvas", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  Default.Clear();
  Passed = EvaluateSize("Clear()", "representative state", "Clear removes representative stored values and resets to one default bin on demand", Default.GetBinnedData().size(), 1UL) && Passed;
  Passed = EvaluateNear("Clear()", "representative content", "Clear resets representative content to zero", Default.GetBinnedData()[0], 0.0, 1e-12) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTBinner::GetCanvasCount()
{
  return gROOT != 0 && gROOT->GetListOfCanvases() != 0 ? gROOT->GetListOfCanvases()->GetSize() : 0;
}


////////////////////////////////////////////////////////////////////////////////


void UTBinner::CleanupCanvases(int TargetCount)
{
  while (gROOT != 0 && gROOT->GetListOfCanvases() != 0 && gROOT->GetListOfCanvases()->GetSize() > TargetCount) {
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    if (Canvas == 0) {
      break;
    }
    delete Canvas;
  }
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinner Test;
  return Test.Run() == true ? 0 : 1;
}
