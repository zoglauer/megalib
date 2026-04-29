/*
 * UTBinnerFISBEL.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// ROOT:
#include <TCanvas.h>
#include <TROOT.h>

// MEGAlib:
#include "MBinnerFISBEL.h"
#include "MExceptions.h"
#include "MUnitTest.h"


//! Unit test class for MBinnerFISBEL
class UTBinnerFISBEL : public MUnitTest
{
public:
  UTBinnerFISBEL() : MUnitTest("UTBinnerFISBEL") {}
  virtual ~UTBinnerFISBEL() {}

  virtual bool Run();

private:
  //! Return the current number of ROOT canvases
  static int GetCanvasCount();
  //! Delete canvases until the requested count is reached
  static void CleanupCanvases(int TargetCount);
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinnerFISBEL::Run()
{
  bool Passed = true;

  MBinnerFISBEL Default;
  Passed = Evaluate("GetNBins()", "default constructor", "The default FISBEL binner starts with zero representative bins", Default.GetNBins(), 0U) && Passed;

  MBinnerFISBEL Single(1);
  Passed = Evaluate("GetNBins()", "single bin", "The one-bin FISBEL binner stores the representative number of bins", Single.GetNBins(), 1U) && Passed;
  Passed = EvaluateNear("GetLongitudeShift()", "single bin", "The one-bin FISBEL binner starts with zero representative longitude shift", Single.GetLongitudeShift(), 0.0, 1e-12) && Passed;
  Passed = EvaluateSize("GetLongitudeBins()", "single bin", "The one-bin FISBEL binner stores one representative longitude collar", Single.GetLongitudeBins().size(), 1UL) && Passed;
  Passed = Evaluate("GetLongitudeBins()", "single bin", "The one-bin FISBEL binner stores one representative longitude bin in its collar", Single.GetLongitudeBins()[0], 1U) && Passed;
  Passed = EvaluateSize("GetLatitudeBinEdges()", "single bin", "The one-bin FISBEL binner stores two representative latitude edges", Single.GetLatitudeBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateNear("GetLatitudeBinEdges()", "first edge", "The one-bin FISBEL binner stores the representative first latitude edge in radians", Single.GetLatitudeBinEdges()[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetLatitudeBinEdges()", "last edge", "The one-bin FISBEL binner stores the representative last latitude edge in radians", Single.GetLatitudeBinEdges()[1], c_Pi, 1e-12) && Passed;
  Passed = Evaluate("FindBin()", "north pole", "The one-bin FISBEL binner maps the representative north pole into its only bin", Single.FindBin(0.0, 0.0), 0U) && Passed;
  Passed = Evaluate("FindBin()", "wrapped longitude", "The one-bin FISBEL binner maps wrapped representative longitudes into its only bin", Single.FindBin(0.5*c_Pi, 5.0*c_Pi), 0U) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin theta", "The one-bin FISBEL binner returns the representative theta center in radians", Single.GetBinCenters(0)[0], 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "single bin phi", "The one-bin FISBEL binner returns the representative phi center in radians", Single.GetBinCenters(0)[1], 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "single bin count", "The one-bin FISBEL binner returns one representative vector center", Single.GetAllBinCenters().size(), 1UL) && Passed;
  Passed = EvaluateNear("GetAllBinCenters()", "single bin z", "The one-bin FISBEL binner returns the representative north-pole vector", Single.GetAllBinCenters()[0].Z(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetDrawingAxisBinEdges()", "single bin axes", "The one-bin FISBEL binner returns two representative drawing-axis vectors", Single.GetDrawingAxisBinEdges().size(), 2UL) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("FindBin()", "theta underflow", "FindBin rejects representative theta underflow", [&](){ Single.FindBin(-0.1, 0.0); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBinCenters()", "out of bounds", "GetBinCenters rejects representative out-of-bounds bin requests", [&](){ Single.GetBinCenters(1); }) && Passed;

  MBinnerFISBEL Shifted;
  Shifted.Create(1, 30.0*c_Rad);
  Passed = EvaluateNear("Create()", "shifted longitude", "Create stores the representative longitude shift in radians", Shifted.GetLongitudeShift(), 30.0*c_Rad, 1e-12) && Passed;
  Passed = EvaluateNear("GetBinCenters()", "shifted single bin phi", "A shifted one-bin FISBEL binner returns the representative shifted phi center", Shifted.GetBinCenters(0)[1], 30.0*c_Rad, 1e-12) && Passed;

  MBinnerFISBEL FourBins;
  FourBins.Create(4);
  Passed = Evaluate("GetNBins()", "four bins", "Create stores the representative four-bin count", FourBins.GetNBins(), 4U) && Passed;
  Passed = Evaluate("GetAllBinCenters()", "four bins", "The four-bin FISBEL binner returns one representative vector per bin", FourBins.GetAllBinCenters().size(), 4UL) && Passed;
  Passed = Evaluate("FindBin()", "equator", "The four-bin FISBEL binner maps a representative equatorial direction into a valid bin", FourBins.FindBin(0.5*c_Pi, 0.25*c_Pi) < FourBins.GetNBins(), true) && Passed;
  vector<double> InteriorCenter = FourBins.GetBinCenters(1);
  Passed = EvaluateTrue("GetBinCenters()", "four bins interior theta", "The four-bin FISBEL binner returns a representative non-polar interior theta center", InteriorCenter[0] > 0.0 && InteriorCenter[0] < c_Pi) && Passed;
  Passed = EvaluateTrue("GetBinCenters()", "four bins interior phi", "The four-bin FISBEL binner returns a representative nontrivial interior phi center", InteriorCenter[1] >= 0.0 && InteriorCenter[1] <= 2.0*c_Pi) && Passed;
  Passed = Evaluate("FindBin()", "interior direction", "The four-bin FISBEL binner maps a representative nontrivial interior direction into the expected bin", FourBins.FindBin(InteriorCenter[0], InteriorCenter[1]), 1U) && Passed;

  vector<unsigned int> LongitudeBins = FourBins.GetLongitudeBins();
  vector<double> LatitudeEdges = FourBins.GetLatitudeBinEdges();
  MBinnerFISBEL Copied;
  Copied.Set(LongitudeBins, LatitudeEdges, FourBins.GetNBins(), FourBins.GetLongitudeShift());
  Passed = Evaluate("operator==()", "copied binning", "Set reproduces the representative FISBEL binning exactly", Copied == FourBins, true) && Passed;
  Passed = Evaluate("operator!=", "different longitude shift", "FISBEL binners with a different representative longitude shift compare unequal", Shifted != Single, true) && Passed;

  MBinnerFISBEL ShiftedCopied;
  ShiftedCopied.Set(LongitudeBins, LatitudeEdges, FourBins.GetNBins(), 30.0*c_Rad);
  Passed = EvaluateNear("Set()", "shifted longitude", "Set stores the representative longitude shift exactly", ShiftedCopied.GetLongitudeShift(), 30.0*c_Rad, 1e-12) && Passed;

  {
    bool WasBatch = gROOT->IsBatch();
    gROOT->SetBatch(true);
    int BeforeCanvases = GetCanvasCount();
    DisableDefaultStreams();
    Single.View(vector<double>{1.0});
    EnableDefaultStreams();
    Passed = Evaluate("View()", "representative display", "View creates a representative ROOT canvas", GetCanvasCount(), BeforeCanvases + 1) && Passed;
    CleanupCanvases(BeforeCanvases);
    gROOT->SetBatch(WasBatch);
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTBinnerFISBEL::GetCanvasCount()
{
  return gROOT != 0 && gROOT->GetListOfCanvases() != 0 ? gROOT->GetListOfCanvases()->GetSize() : 0;
}


////////////////////////////////////////////////////////////////////////////////


void UTBinnerFISBEL::CleanupCanvases(int TargetCount)
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
  UTBinnerFISBEL Test;
  return Test.Run() == true ? 0 : 1;
}
