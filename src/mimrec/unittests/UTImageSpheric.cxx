/*
 * UTImageSpheric.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <cmath>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH2.h>

// MEGAlib:
#include "MImageSpheric.h"
#include "MUnitTest.h"


class TestImageSpheric : public MImageSpheric
{
public:
  TestImageSpheric() : MImageSpheric(), m_TestProjection(MImageProjection::c_None) {}

  TestImageSpheric(MString Title, double* IA,
                   MString xTitle, double xMin, double xMax, int xNBins,
                   MString yTitle, double yMin, double yMax, int yNBins,
                   MString vTitle = "", int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ) :
    MImageSpheric(Title, IA, xTitle, xMin, xMax, xNBins,
                  yTitle, yMin, yMax, yNBins, vTitle, Spectrum, DrawOption),
    m_TestProjection(MImageProjection::c_None) {}

  virtual MImage* Clone()
  {
    TestImageSpheric* I =
      new TestImageSpheric(m_Title, m_IA,
                           m_xTitle, m_xMin, m_xMax, m_xNBins,
                           m_yTitle, m_yMin, m_yMax, m_yNBins,
                           m_vTitle, m_Spectrum, m_DrawOption);
    I->SetProjection(m_TestProjection);
    I->Normalize(m_Normalize);
    return I;
  }

  void SetProjection(MImageProjection Projection)
  {
    m_TestProjection = Projection;
    MImageSpheric::SetProjection(Projection);
  }

  TH1* GetHistogramPointer() const { return m_Histogram; }
  TCanvas* GetCanvasPointer() const { return m_Canvas; }
  double GetArrayValueRaw(int Index) const { return m_IA[Index]; }
  MImageProjection GetProjection() const { return m_TestProjection; }
  void InvalidateArray() { m_IA = nullptr; }

private:
  MImageProjection m_TestProjection;
};


static int CountNonZeroBins(TH2* Hist)
{
  if (Hist == nullptr) {
    return -1;
  }

  int Count = 0;
  for (int x = 1; x <= Hist->GetNbinsX(); ++x) {
    for (int y = 1; y <= Hist->GetNbinsY(); ++y) {
      if (Hist->GetBinContent(x, y) != 0.0) {
        ++Count;
      }
    }
  }

  return Count;
}


//! Unit test class for MImageSpheric
class UTImageSpheric : public MUnitTest
{
public:
  UTImageSpheric() : MUnitTest("UTImageSpheric") {}
  virtual ~UTImageSpheric() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTImageSpheric::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    TestImageSpheric Default;
    Passed = Evaluate("GetDimensions()", "default constructor", "The default spherical image reports the representative two-dimensional image shape", Default.GetDimensions(), 2U) && Passed;
    Passed = EvaluateFalse("IsCreated()", "default constructor", "A default spherical image starts without a representative created histogram", Default.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "default constructor", "A default spherical image reports that no representative canvas exists yet", Default.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "default constructor", "The default spherical image configures one representative entry for the default 1x1 image", Default.GetNEntries(), 1) && Passed;
    Passed = EvaluateNear("GetAverage()", "default constructor", "The default spherical image average is zero for the representative zero-filled default entry", Default.GetAverage(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArrayValueRaw()", "default constructor", "The default spherical image backing array is initialized with a representative zero value", Default.GetArrayValueRaw(0), 0.0, 1e-12) && Passed;
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageSpheric Image("SphericNoneImage", Values,
                           "Phi", 0.0, 3.0, 3,
                           "Theta", 0.0, 2.0, 2,
                           "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();

    Passed = EvaluateTrue("Display()", "representative created flag", "Display creates a representative spherical histogram and marks the image as created", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative canvas exists", "Display creates a representative spherical canvas that can be found afterwards", Image.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "representative asymmetric image entries", "A representative asymmetric spherical image reports the correct entry count", Image.GetNEntries(), 6) && Passed;

    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    Passed = EvaluateTrue("Display()", "representative TH2 type", "Display stores the representative spherical image in a TH2 histogram", Hist != nullptr) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative bin (1,1)", "Display stores the representative first spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(1, 2), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (2,1)", "Display stores the representative second spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(2, 2), 2.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (3,1)", "Display stores the representative third spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(3, 2), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (1,2)", "Display stores the representative fourth spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(1, 1), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (2,2)", "Display stores the representative fifth spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(2, 1), 5.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (3,2)", "Display stores the representative sixth spherical bin with the expected inverted y-axis mapping", Hist->GetBinContent(3, 1), 6.0, 1e-12) && Passed;
    }
    const char* SaveFile = "/tmp/UTImageSpheric_existing_canvas.png";
    unlink(SaveFile);
    Image.SaveAs(SaveFile);
    struct stat Stat = {};
    Passed = EvaluateTrue("SaveAs()", "representative none save", "SaveAs writes the representative spherical image to a file when a canvas exists", stat(SaveFile, &Stat) == 0) && Passed;
    Image.Reset();
    if (Hist != nullptr) {
      Passed = Evaluate("Reset()", "representative none reset clears bins", "Reset zeroes the unprojected spherical histogram after display", CountNonZeroBins(Hist), 0) && Passed;
    }
    Image.SetImageArray(nullptr);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative none nullptr clears bins", "SetImageArray(nullptr) zeros all bins in the unprojected spherical image", CountNonZeroBins(Hist), 0) && Passed;
    }
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageSpheric Image("SphericHammerImage", Values,
                           "Phi", 0.0, 360.0, 2,
                           "Theta", 0.0, 180.0, 2,
                           "Counts");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative hammer histogram", "Display creates a representative Hammer-projected spherical histogram", Image.GetHistogramPointer() != nullptr) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative hammer canvas", "Display creates a representative Hammer-projected spherical canvas", Image.CanvasExists()) && Passed;
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateTrue("Display()", "representative hammer binning", "The representative Hammer histogram has a finer grid than the source image", Hist->GetNbinsX() > Image.GetNEntries()) && Passed;
      Passed = EvaluateTrue("Display()", "representative hammer nonzero bins", "The representative Hammer histogram has projected content after display", CountNonZeroBins(Hist) > 0) && Passed;
    }
    double HammerUpdated[4] = {0.0, 0.0, 0.0, 10.0};
    Image.SetImageArray(HammerUpdated);
    if (Hist != nullptr) {
      Passed = EvaluateTrue("SetImageArray()", "representative hammer updated bins", "SetImageArray(newData) updates the projected Hammer histogram", Hist->GetMaximum() > 0.0) && Passed;
    }
    Image.SetImageArray(nullptr);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative hammer nullptr clears bins", "SetImageArray(nullptr) zeros all bins in the Hammer-projected spherical image", CountNonZeroBins(Hist), 0) && Passed;
    }
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageSpheric Image("SphericUpdateImage", Values,
                           "Phi", 0.0, 2.0, 2,
                           "Theta", 0.0, 2.0, 2,
                           "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());

    double UpdatedValues[4] = {9.0, numeric_limits<double>::quiet_NaN(),
                               numeric_limits<double>::infinity(), 12.0};
    Image.SetImageArray(UpdatedValues);
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated first bin", "SetImageArray updates the representative first spherical bin", Hist->GetBinContent(1, 2), 9.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative cleared NaN bin", "SetImageArray clears the representative NaN spherical bin", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative cleared Inf bin", "SetImageArray clears the representative infinite spherical bin", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated last bin", "SetImageArray updates the representative last spherical bin", Hist->GetBinContent(2, 1), 12.0, 1e-12) && Passed;
    }

    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display(Image.GetCanvasPointer());
    Passed = EvaluateTrue("Display()", "representative hammer redisplay", "A representative spherical image can be redrawn in Hammer projection after data updates", Image.GetHistogramPointer() != nullptr) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageSpheric Image("SphericHammerNonFinite", Values,
                           "Phi", 0.0, 360.0, 2,
                           "Theta", 0.0, 180.0, 2,
                           "Counts");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    double Invalid[4] = {numeric_limits<double>::quiet_NaN(),
                         numeric_limits<double>::infinity(),
                         numeric_limits<double>::quiet_NaN(),
                         numeric_limits<double>::infinity()};
    Image.SetImageArray(Invalid);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative hammer non-finite cleared", "SetImageArray() clears all Hammer bins when the representative spherical data is non-finite", CountNonZeroBins(Hist), 0) && Passed;
    }
  }

  {
    double Values[4] = {2.0, 4.0, 6.0, 8.0};
    TestImageSpheric Original("SphericCloneImage", Values,
                              "Phi", 0.0, 2.0, 2,
                              "Theta", 0.0, 2.0, 2,
                              "Counts");
    Original.SetProjection(MImageProjection::c_None);
    Original.Normalize(true);
    MImage* CloneBase = Original.Clone();
    TestImageSpheric* Clone = dynamic_cast<TestImageSpheric*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative clone type", "Clone returns a representative object of the spherical image type", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative cloned projection", "Clone preserves the representative spherical projection mode", static_cast<int>(Clone->GetProjection()), static_cast<int>(MImageProjection::c_None)) && Passed;
      Clone->Display();
      TH2* CloneHist = dynamic_cast<TH2*>(Clone->GetHistogramPointer());
      if (CloneHist != nullptr) {
        Passed = EvaluateNear("Clone()", "representative cloned normalized maximum", "Clone preserves the representative spherical normalization setting", CloneHist->GetMaximum(), 1.0, 1e-12) && Passed;
      }
    }
    delete CloneBase;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageSpheric Image("SphericNullDisplay", Values,
                           "Phi", 0.0, 2.0, 2,
                           "Theta", 0.0, 2.0, 2,
                           "Counts");
    Image.InvalidateArray();
    Image.Display();
    Passed = EvaluateFalse("Display()", "representative null backing data", "Display returns early and does not mark the representative spherical image as created when the backing data is missing", Image.IsCreated()) && Passed;
  }

  {
    double Values[4] = {9.0, 1.0, 2.0, 3.0};
    TestImageSpheric Image("SphericDetermineMaximum", Values,
                           "Phi", 0.0, 2.0, 2,
                           "Theta", 0.0, 2.0, 2,
                           "Counts");
    double MaxValue = -1.0;
    vector<double> Coordinate;
    Image.DetermineMaximum(MaxValue, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum", "DetermineMaximum finds the representative first-bin maximum in a spherical image", MaxValue, 9.0, 1e-12) && Passed;
    Passed = Evaluate("DetermineMaximum()", "representative first-bin coordinate count", "DetermineMaximum returns two coordinates for the representative spherical image", static_cast<int>(Coordinate.size()), 2) && Passed;
    if (Coordinate.size() == 2) {
      Passed = EvaluateNear("DetermineMaximum()", "representative first-bin x coordinate", "DetermineMaximum returns the representative x bin center for the spherical image maximum", Coordinate[0], 0.5, 1e-12) && Passed;
      Passed = EvaluateNear("DetermineMaximum()", "representative first-bin y coordinate", "DetermineMaximum returns the representative y bin center for the spherical image maximum", Coordinate[1], 0.5, 1e-12) && Passed;
    }

    Image.InvalidateArray();
    Coordinate.push_back(123.0);
    Image.DetermineMaximum(MaxValue, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative null maximum", "DetermineMaximum returns zero when the representative spherical backing data is missing", MaxValue, 0.0, 1e-12) && Passed;
    Passed = Evaluate("DetermineMaximum()", "representative null coordinate count", "DetermineMaximum clears the coordinate vector when the representative spherical backing data is missing", static_cast<int>(Coordinate.size()), 0) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTImageSpheric Test;
  return Test.Run() == true ? 0 : 1;
}
