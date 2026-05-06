/*
 * UTImage.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TAxis.h>

// MEGAlib:
#include "MImage.h"
#include "MUnitTest.h"


class TestImage : public MImage
{
public:
  TestImage() : MImage() {}

  TestImage(MString Title, double* IA,
            MString xTitle, double xMin, double xMax, int xNBins,
            MString vTitle = "", int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ) :
    MImage(Title, IA, xTitle, xMin, xMax, xNBins, vTitle, Spectrum, DrawOption) {}

  virtual MImage* Clone()
  {
    TestImage* I =
      new TestImage(m_Title, m_IA,
                    m_xTitle, m_xMin, m_xMax, m_xNBins,
                    m_vTitle, m_Spectrum, m_DrawOption);
    I->Normalize(m_Normalize);
    return I;
  }

  using MImage::MakeCanvasTitle;

  TH1* GetHistogramPointer() const { return m_Histogram; }
  TCanvas* GetCanvasPointer() const { return m_Canvas; }
  const MString& GetDrawOptionString() const { return m_DrawOptionString; }
  int GetID() const { return m_ID; }
  int GetXNBinsRaw() const { return m_xNBins; }
  double GetArrayValueRaw(int Index) const { return m_IA[Index]; }
};


//! Unit test class for MImage
class UTImage : public MUnitTest
{
public:
  UTImage() : MUnitTest("UTImage") {}
  virtual ~UTImage() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTImage::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    TestImage Default;
    Passed = Evaluate("GetTitle()", "default constructor", "The default MImage constructor sets the representative default title", MString(Default.GetTitle()), MString("Title here")) && Passed;
    Passed = Evaluate("GetDimensions()", "default constructor", "The base image reports one representative dimension", Default.GetDimensions(), 1U) && Passed;
    Passed = EvaluateFalse("IsCreated()", "default constructor", "A default image starts without a created histogram", Default.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "default constructor", "A default image reports that no representative canvas exists yet", Default.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "default constructor", "The default constructor configures one representative entry for the default 1D image", Default.GetNEntries(), 1) && Passed;
    Passed = Evaluate("GetXNBinsRaw()", "default constructor", "The default constructor still configures one representative x bin", Default.GetXNBinsRaw(), 1) && Passed;
    Passed = EvaluateNear("GetAverage()", "default constructor", "The default image average is zero when no representative entries are configured", Default.GetAverage(), 0.0, 1e-12) && Passed;

    vector<double> Coordinate;
    double Maximum = 99.0;
    Default.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "default constructor maximum value", "DetermineMaximum returns the representative zero-valued default maximum", Maximum, 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "default constructor coordinate count", "DetermineMaximum returns one representative coordinate for the default 1D image", Coordinate.size(), 1UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor maximum coordinate", "DetermineMaximum returns the representative default x-bin center", Coordinate[0], 0.5, 1e-12) && Passed;

    Default.Reset();
    Passed = EvaluateNear("Reset()", "default constructor pre-display average", "Reset is safely callable on a representative image before any histogram has been created", Default.GetAverage(), 0.0, 1e-12) && Passed;

    DisableDefaultStreams();
    Default.DrawCopy();
    EnableDefaultStreams();
    Passed = EvaluateTrue("DrawCopy()", "default constructor", "The not-yet-implemented DrawCopy path is still callable on a representative image", true) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage Image("RepresentativeImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);

    Passed = Evaluate("GetTitle()", "standard constructor", "The standard MImage constructor keeps the representative title", MString(Image.GetTitle()), MString("RepresentativeImage")) && Passed;
    Passed = EvaluateNear("GetAverage()", "standard constructor", "The standard MImage constructor stores representative data for averaging", Image.GetAverage(), 2.0, 1e-12) && Passed;
    Passed = Evaluate("GetDrawOptionString()", "COLZ", "The representative COLZ draw option is translated to the expected ROOT string", Image.GetDrawOptionString(), MString("COLZ")) && Passed;
    Passed = Evaluate("MakeCanvasTitle()", "stable instance id", "The representative canvas title uses the stable per-instance image id after the expected title sanitization", Image.MakeCanvasTitle(), MString("RepresentativeImage___") + Image.GetID()) && Passed;

    TestImage Sanitized("Title with spaces:/?#!", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    Passed = Evaluate("MakeCanvasTitle()", "representative title sanitization", "MakeCanvasTitle replaces representative disallowed title characters with underscores", Sanitized.MakeCanvasTitle(), MString("Title_with_spaces________") + Sanitized.GetID()) && Passed;

    vector<double> Coordinate;
    double Maximum = 0.0;
    Image.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum value", "DetermineMaximum finds the representative largest stored value", Maximum, 3.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative positive maximum coordinate count", "DetermineMaximum returns one representative coordinate for a one-dimensional image", Coordinate.size(), 1UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum coordinate", "DetermineMaximum reports the representative bin center of the maximum", Coordinate[0], 2.5, 1e-12) && Passed;
  }

  {
    double NegativeValues[3] = {-5.0, -2.0, -7.0};
    TestImage Negative("NegativeImage", NegativeValues, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    vector<double> Coordinate;
    double Maximum = 0.0;
    Negative.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum value", "DetermineMaximum finds the representative maximum even when all values are negative", Maximum, -2.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative all-negative coordinate count", "DetermineMaximum still returns one representative coordinate for all-negative data", Coordinate.size(), 1UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum coordinate", "DetermineMaximum reports the representative bin center of the all-negative maximum", Coordinate[0], 1.5, 1e-12) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage Image("DisplayImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    Image.SetTitle("DisplayImageUpdated");
    Image.SetXAxisTitle("EnergyUpdated");
    Image.SetValueAxisTitle("CountsUpdated");
    Image.SetDrawOption(MImage::c_COLCONTZ);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative first display", "Display creates a representative histogram and marks the image as created", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("Display()", "representative histogram created", "Display creates a representative ROOT histogram", Image.GetHistogramPointer() != nullptr) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative canvas exists", "Display creates a representative canvas that can be found afterwards", Image.CanvasExists()) && Passed;

    TH1* Hist = Image.GetHistogramPointer();
    Passed = Evaluate("SetTitle()", "representative title after display", "Display uses the representative updated image title for the histogram", MString(Hist->GetTitle()), MString("DisplayImageUpdated")) && Passed;
    Passed = Evaluate("SetXAxisTitle()", "representative x-axis title after display", "Display uses the representative updated x-axis title", MString(Hist->GetXaxis()->GetTitle()), MString("EnergyUpdated")) && Passed;
    Passed = Evaluate("SetValueAxisTitle()", "representative value-axis title after display", "Display uses the representative updated value-axis title", MString(Hist->GetYaxis()->GetTitle()), MString("CountsUpdated")) && Passed;
    Passed = Evaluate("SetDrawOption()", "representative COLCONTZ draw option", "The representative COLCONTZ draw option is translated to the expected ROOT string", Image.GetDrawOptionString(), MString("COLCONT0Z")) && Passed;
    Passed = EvaluateNear("Display()", "representative first display bin 1", "Display fills the representative first histogram bin from the backing array", Hist->GetBinContent(1), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("Display()", "representative first display bin 2", "Display fills the representative second histogram bin from the backing array", Hist->GetBinContent(2), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("Display()", "representative first display bin 3", "Display fills the representative third histogram bin from the backing array", Hist->GetBinContent(3), 3.0, 1e-12) && Passed;

    Image.Display(Image.GetCanvasPointer());
    Passed = EvaluateNear("Display()", "representative repeated display bin 1", "Repeated Display does not accumulate representative histogram content", Hist->GetBinContent(1), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("Display()", "representative repeated display bin 2", "Repeated Display keeps the representative second histogram bin unchanged", Hist->GetBinContent(2), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("Display()", "representative repeated display bin 3", "Repeated Display keeps the representative third histogram bin unchanged", Hist->GetBinContent(3), 3.0, 1e-12) && Passed;

    Image.SetTitle("DisplayImageLiveUpdate");
    Passed = Evaluate("SetTitle()", "representative live histogram title update", "SetTitle updates the representative histogram title after display", MString(Hist->GetTitle()), MString("DisplayImageLiveUpdate")) && Passed;
    Passed = Evaluate("SetTitle()", "representative live canvas title update", "SetTitle updates the representative canvas title after display", MString(Image.GetCanvasPointer()->GetTitle()), MString("DisplayImageLiveUpdate")) && Passed;

    Image.Reset();
    Passed = EvaluateNear("Reset()", "representative backing array bin 0", "Reset clears the representative first backing-array value", Image.GetArrayValueRaw(0), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative backing array bin 1", "Reset clears the representative second backing-array value", Image.GetArrayValueRaw(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative backing array bin 2", "Reset clears the representative third backing-array value", Image.GetArrayValueRaw(2), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative histogram bin 1", "Reset also clears the representative first histogram bin", Hist->GetBinContent(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative histogram bin 2", "Reset also clears the representative second histogram bin", Hist->GetBinContent(2), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative histogram bin 3", "Reset also clears the representative third histogram bin", Hist->GetBinContent(3), 0.0, 1e-12) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage Image("MutatorImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    Image.Display();
    double UpdatedValues[3] = {4.0, 5.0, 6.0};
    Image.SetImageArray(UpdatedValues);
    Passed = EvaluateNear("SetImageArray()", "representative redisplay-free average", "SetImageArray updates the representative backing array immediately", Image.GetAverage(), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative histogram bin 1 after update", "SetImageArray updates the representative first displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(1), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative histogram bin 2 after update", "SetImageArray updates the representative second displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(2), 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative histogram bin 3 after update", "SetImageArray updates the representative third displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(3), 6.0, 1e-12) && Passed;

    Image.SetImageArray(nullptr);
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr average", "SetImageArray(nullptr) zeros the representative backing array of an existing image", Image.GetAverage(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr histogram bin 1", "SetImageArray(nullptr) zeros the representative first displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr histogram bin 2", "SetImageArray(nullptr) zeros the representative second displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(2), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr histogram bin 3", "SetImageArray(nullptr) zeros the representative third displayed histogram bin", Image.GetHistogramPointer()->GetBinContent(3), 0.0, 1e-12) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage AxisImage("AxisImage", Values, "Axis", 0.0, 4.0, 4, "Intensity", MImage::c_Viridis, MImage::c_COLZ);
    AxisImage.SetXAxis("AxisUpdated", -2.0, 2.0, 4);
    AxisImage.Display();
    TH1* Hist = AxisImage.GetHistogramPointer();
    Passed = Evaluate("SetXAxis()", "representative x-axis title after post-construction update", "SetXAxis updates the representative x-axis title before display", MString(Hist->GetXaxis()->GetTitle()), MString("AxisUpdated")) && Passed;
    Passed = EvaluateNear("SetXAxis()", "representative x minimum after post-construction update", "SetXAxis updates the representative x-axis minimum before display", Hist->GetXaxis()->GetXmin(), -2.0, 1e-12) && Passed;
    Passed = EvaluateNear("SetXAxis()", "representative x maximum after post-construction update", "SetXAxis updates the representative x-axis maximum before display", Hist->GetXaxis()->GetXmax(), 2.0, 1e-12) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage ReusedCanvas("ReuseCanvasImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    TCanvas ExistingCanvas("RepresentativeExistingCanvas1D", "RepresentativeExistingCanvas1D", 10, 10, 300, 300);
    ReusedCanvas.Display(&ExistingCanvas);
    Passed = Evaluate("Display()", "representative existing canvas title reuse", "Display keeps the representative non-empty existing 1D canvas title", MString(ReusedCanvas.GetCanvasPointer()->GetTitle()), MString("RepresentativeExistingCanvas1D")) && Passed;

    TestImage EmptyCanvasTitle("EmptyCanvasTitleImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    TCanvas UntitledCanvas("", "", 10, 10, 300, 300);
    EmptyCanvasTitle.Display(&UntitledCanvas);
    Passed = EvaluateTrue("Display()", "representative empty existing canvas title", "Display assigns a representative non-empty title when the existing 1D canvas title is empty", MString(EmptyCanvasTitle.GetCanvasPointer()->GetTitle()).IsEmpty() == false) && Passed;
  }

  {
    double Values[2] = {2.0, 4.0};
    TestImage Normalized("NormalizedImage", Values, "Axis", 0.0, 2.0, 2, "Intensity", MImage::c_Viridis, MImage::c_COLZ);
    Normalized.Normalize(true);
    Normalized.Display();
    Passed = EvaluateNear("Normalize()", "representative normalized maximum", "Display normalizes a representative image so the maximum becomes one", Normalized.GetHistogramPointer()->GetMaximum(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("Normalize()", "representative normalized first bin", "Display rescales the representative first normalized histogram bin by the maximum", Normalized.GetHistogramPointer()->GetBinContent(1), 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("Normalize()", "representative normalized second bin", "Display rescales the representative second normalized histogram bin by the maximum", Normalized.GetHistogramPointer()->GetBinContent(2), 1.0, 1e-12) && Passed;

    double ZeroValues[2] = {0.0, 0.0};
    TestImage ZeroNormalized("ZeroNormalizedImage", ZeroValues, "Axis", 0.0, 2.0, 2, "Intensity", MImage::c_Viridis, MImage::c_COLZ);
    ZeroNormalized.Normalize(true);
    ZeroNormalized.Display();
    Passed = EvaluateNear("Normalize()", "representative normalized zero maximum", "Display keeps the representative normalized histogram at zero when the maximum is not positive", ZeroNormalized.GetHistogramPointer()->GetMaximum(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Normalize()", "representative normalized zero first bin", "Display leaves the representative first normalized histogram bin at zero when scaling is skipped", ZeroNormalized.GetHistogramPointer()->GetBinContent(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Normalize()", "representative normalized zero second bin", "Display leaves the representative second normalized histogram bin at zero when scaling is skipped", ZeroNormalized.GetHistogramPointer()->GetBinContent(2), 0.0, 1e-12) && Passed;

    Normalized.Normalize(false);
    Normalized.Display(Normalized.GetCanvasPointer());
    Passed = EvaluateNear("Normalize()", "representative normalization toggle-off first bin", "Display stops rescaling the representative first histogram bin after Normalize(false)", Normalized.GetHistogramPointer()->GetBinContent(1), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("Normalize()", "representative normalization toggle-off second bin", "Display stops rescaling the representative second histogram bin after Normalize(false)", Normalized.GetHistogramPointer()->GetBinContent(2), 4.0, 1e-12) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage Fallbacks("FallbackImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    Fallbacks.SetDrawOption(MImage::c_COL);
    Passed = Evaluate("SetDrawOption()", "representative COL draw option", "The representative COL draw option is translated to the expected ROOT string", Fallbacks.GetDrawOptionString(), MString("COL")) && Passed;
    Fallbacks.SetDrawOption(MImage::c_COLCONT);
    Passed = Evaluate("SetDrawOption()", "representative COLCONT draw option", "The representative COLCONT draw option is translated to the expected ROOT string", Fallbacks.GetDrawOptionString(), MString("COLCONT0")) && Passed;
    DisableDefaultStreams();
    Fallbacks.SetDrawOption(999);
    Fallbacks.SetSpectrum(999);
    EnableDefaultStreams();
    Passed = Evaluate("SetDrawOption()", "representative invalid draw option", "Invalid draw options fall back to the representative COLZ ROOT draw string", Fallbacks.GetDrawOptionString(), MString("COLZ")) && Passed;
  }

  {
    double Values[3] = {1.0, 2.0, 3.0};
    TestImage Saved("SavedImage", Values, "Energy", 0.0, 3.0, 3, "Counts", MImage::c_Viridis, MImage::c_COLZ);
    DisableDefaultStreams();
    Saved.SaveAs("/tmp/UTImage/does_not_exist.png");
    EnableDefaultStreams();
    Passed = EvaluateTrue("SaveAs()", "representative no-canvas path", "SaveAs is safely callable on a representative image without a canvas", true) && Passed;

    Saved.Display();
    MString FileName = "/tmp/UTImage_existing_canvas.png";
    ::remove(FileName.Data());
    Saved.SaveAs(FileName);
    struct stat Info;
    Passed = EvaluateTrue("SaveAs()", "representative existing canvas path", "SaveAs creates the representative output file when a canvas exists", stat(FileName.Data(), &Info) == 0 && S_ISREG(Info.st_mode) != 0) && Passed;
  }

  {
    double Values[2] = {4.0, 8.0};
    TestImage Original("CloneSource", Values, "Axis", 0.0, 2.0, 2, "Intensity", MImage::c_Viridis, MImage::c_COLCONTZ);
    Original.Normalize(true);
    MImage* CloneBase = Original.Clone();
    TestImage* Clone = dynamic_cast<TestImage*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative derived clone type", "Clone returns a representative object of the same concrete type", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative cloned title", "Clone preserves the representative image title", MString(Clone->GetTitle()), MString("CloneSource")) && Passed;
      Passed = EvaluateNear("Clone()", "representative cloned average", "Clone preserves representative backing data", Clone->GetAverage(), 6.0, 1e-12) && Passed;
      Passed = Evaluate("MakeCanvasTitle()", "representative cloned stable id", "Each representative clone gets its own stable image id for canvas titles after the expected title sanitization", Clone->MakeCanvasTitle(), MString("CloneSource___") + Clone->GetID()) && Passed;
      Clone->Display();
      Passed = EvaluateNear("Clone()", "representative cloned normalized maximum", "Clone preserves the representative Normalize flag in observable display behavior", Clone->GetHistogramPointer()->GetMaximum(), 1.0, 1e-12) && Passed;
    }
    delete CloneBase;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTImage Test;
  return !Test.Run();
}
