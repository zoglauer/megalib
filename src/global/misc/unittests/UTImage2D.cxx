/*
 * UTImage2D.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TAxis.h>

// MEGAlib:
#include "MImage2D.h"
#include "MUnitTest.h"


class TestImage2D : public MImage2D
{
public:
  TestImage2D() : MImage2D() {}

  TestImage2D(MString Title, double* IA,
              MString xTitle, double xMin, double xMax, int xNBins,
              MString yTitle, double yMin, double yMax, int yNBins,
              MString vTitle = "", int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ) :
    MImage2D(Title, IA, xTitle, xMin, xMax, xNBins, yTitle, yMin, yMax, yNBins, vTitle, Spectrum, DrawOption) {}

  virtual MImage* Clone()
  {
    TestImage2D* I =
      new TestImage2D(m_Title, m_IA,
                      m_xTitle, m_xMin, m_xMax, m_xNBins,
                      m_yTitle, m_yMin, m_yMax, m_yNBins,
                      m_vTitle, m_Spectrum, m_DrawOption);
    I->Normalize(m_Normalize);
    return I;
  }

  TH1* GetHistogramPointer() const { return m_Histogram; }
  TCanvas* GetCanvasPointer() const { return m_Canvas; }
  int GetXNBinsRaw() const { return m_xNBins; }
  int GetYNBinsRaw() const { return m_yNBins; }
  double GetArrayValueRaw(int Index) const { return m_IA[Index]; }
  const MString& GetDrawOptionString() const { return m_DrawOptionString; }

  void ForceEmpty()
  {
    if (m_IA != nullptr) {
      delete [] m_IA;
      m_IA = nullptr;
    }
    m_xNBins = 0;
    m_yNBins = 0;
  }

  void ForceInvalidGeometry(int XBins, int YBins)
  {
    if (m_IA != nullptr) {
      delete [] m_IA;
      m_IA = nullptr;
    }
    m_xNBins = XBins;
    m_yNBins = YBins;
  }
};


//! Unit test class for MImage2D
class UTImage2D : public MUnitTest
{
public:
  UTImage2D() : MUnitTest("UTImage2D") {}
  virtual ~UTImage2D() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTImage2D::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    TestImage2D Default;
    Passed = Evaluate("GetDimensions()", "default constructor", "The default MImage2D constructor reports the representative two-dimensional image shape", Default.GetDimensions(), 2U) && Passed;
    Passed = EvaluateFalse("IsCreated()", "default constructor", "A default 2D image starts without a representative created histogram", Default.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "default constructor", "A default 2D image reports that no representative canvas exists yet", Default.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "default constructor", "The default 2D image configures one representative entry for the default 1x1 image", Default.GetNEntries(), 1) && Passed;
    Passed = Evaluate("GetXNBinsRaw()", "default constructor", "The default 2D image configures one representative x bin", Default.GetXNBinsRaw(), 1) && Passed;
    Passed = Evaluate("GetYNBinsRaw()", "default constructor", "The default 2D image configures one representative y bin", Default.GetYNBinsRaw(), 1) && Passed;
    Passed = EvaluateNear("GetAverage()", "default constructor", "The default 2D image average is zero for the representative zero-filled default entry", Default.GetAverage(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArrayValueRaw()", "default constructor", "The default 2D image backing array is initialized with a representative zero value", Default.GetArrayValueRaw(0), 0.0, 1e-12) && Passed;

    vector<double> Coordinate;
    double Maximum = 99.0;
    Default.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "default constructor maximum value", "DetermineMaximum returns the representative zero-valued default maximum", Maximum, 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "default constructor coordinate count", "DetermineMaximum returns two representative coordinates for the default 2D image", Coordinate.size(), 2UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor x coordinate", "DetermineMaximum returns the representative default x-bin center", Coordinate[0], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor y coordinate", "DetermineMaximum returns the representative default y-bin center", Coordinate[1], 0.5, 1e-12) && Passed;

    Default.Reset();
    Passed = EvaluateNear("Reset()", "default constructor pre-display average", "Reset is safely callable on a representative 2D image before any histogram has been created", Default.GetAverage(), 0.0, 1e-12) && Passed;

    DisableDefaultStreams();
    Default.SaveAs("/tmp/UTImage2D/does_not_exist.png");
    EnableDefaultStreams();
    Passed = EvaluateTrue("SaveAs()", "default constructor no-canvas path", "SaveAs is safely callable on a representative 2D image without a canvas", true) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D Image("Representative2DImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Passed = EvaluateNear("GetAverage()", "standard constructor", "The standard 2D constructor stores representative backing data for averaging", Image.GetAverage(), 2.5, 1e-12) && Passed;

    vector<double> Coordinate;
    double Maximum = 0.0;
    Image.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum value", "DetermineMaximum finds the representative largest stored 2D value", Maximum, 4.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative positive coordinate count", "DetermineMaximum returns two representative coordinates for a 2D image", Coordinate.size(), 2UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum x coordinate", "DetermineMaximum reports the representative x-bin center of the 2D maximum", Coordinate[0], 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum y coordinate", "DetermineMaximum reports the representative y-bin center of the 2D maximum", Coordinate[1], 1.5, 1e-12) && Passed;
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImage2D Asymmetric("Asymmetric2DImage", Values, "X Axis", 0.0, 3.0, 3, "Y Axis", 0.0, 2.0, 2, "Counts");
    Asymmetric.Display();
    TH2* Hist = dynamic_cast<TH2*>(Asymmetric.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative asymmetric bin (1,1)", "Display stores the representative first asymmetric 2D bin with the expected x-major stride", Hist->GetBinContent(1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (3,1)", "Display stores the representative last bin of the first asymmetric 2D row with the expected x-major stride", Hist->GetBinContent(3, 1), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (1,2)", "Display stores the representative first bin of the second asymmetric 2D row with the expected x-major stride", Hist->GetBinContent(1, 2), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (3,2)", "Display stores the representative last asymmetric 2D bin with the expected x-major stride", Hist->GetBinContent(3, 2), 6.0, 1e-12) && Passed;
    }
  }

  {
    double NegativeValues[4] = {-5.0, -2.0, -7.0, -3.0};
    TestImage2D Negative("Negative2DImage", NegativeValues, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    vector<double> Coordinate;
    double Maximum = 0.0;
    Negative.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum value", "DetermineMaximum finds the representative 2D maximum even when all values are negative", Maximum, -2.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative all-negative coordinate count", "DetermineMaximum still returns two representative coordinates for all-negative 2D data", Coordinate.size(), 2UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum x coordinate", "DetermineMaximum reports the representative x-bin center of the all-negative 2D maximum", Coordinate[0], 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum y coordinate", "DetermineMaximum reports the representative y-bin center of the all-negative 2D maximum", Coordinate[1], 0.5, 1e-12) && Passed;
  }

  {
    double FirstIsMaximum[4] = {9.0, 1.0, 2.0, 3.0};
    TestImage2D First("FirstMaximum2DImage", FirstIsMaximum, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    vector<double> Coordinate;
    double Maximum = 0.0;
    First.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum value", "DetermineMaximum keeps the representative first 2D element as the maximum when nothing exceeds it", Maximum, 9.0, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum x coordinate", "DetermineMaximum keeps the representative first-bin x coordinate when the maximum is at (0,0)", Coordinate[0], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum y coordinate", "DetermineMaximum keeps the representative first-bin y coordinate when the maximum is at (0,0)", Coordinate[1], 0.5, 1e-12) && Passed;
  }

  {
    TestImage2D Empty;
    Empty.ForceEmpty();
    vector<double> Coordinate;
    double Maximum = 17.0;
    Empty.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative forced-empty maximum value", "DetermineMaximum returns zero for a representative empty 2D image", Maximum, 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative forced-empty coordinate count", "DetermineMaximum leaves the representative coordinate list empty for an empty 2D image", Coordinate.size(), 0UL) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D Image("Display2DImage", Values, "Initial X", 0.0, 2.0, 2, "Initial Y", 0.0, 2.0, 2, "Initial Counts");
    Image.SetYAxis("Configured Y", -1.0, 3.0, 2);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative first display", "Display creates a representative 2D histogram and marks the image as created", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("Display()", "representative histogram created", "Display creates a representative ROOT 2D histogram", Image.GetHistogramPointer() != nullptr) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative canvas exists", "Display creates a representative 2D canvas that can be found afterwards", Image.CanvasExists()) && Passed;

    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    Passed = EvaluateTrue("Display()", "representative TH2 type", "Display stores the representative image in a TH2 histogram", Hist != nullptr) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative first display bin (1,1)", "Display fills the representative first 2D histogram bin from the backing array", Hist->GetBinContent(1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative first display bin (2,1)", "Display fills the representative second 2D histogram bin from the backing array", Hist->GetBinContent(2, 1), 2.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative first display bin (1,2)", "Display fills the representative third 2D histogram bin from the backing array", Hist->GetBinContent(1, 2), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative first display bin (2,2)", "Display fills the representative fourth 2D histogram bin from the backing array", Hist->GetBinContent(2, 2), 4.0, 1e-12) && Passed;
    }

    Image.SetTitle("Display2DImageUpdated");
    Image.SetXAxisTitle("Updated X");
    Image.SetYAxisTitle("Updated Y");
    Image.SetValueAxisTitle("Updated Counts");
    Image.Display(Image.GetCanvasPointer());

    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated display bin (1,1)", "Repeated Display keeps the representative first 2D histogram bin unchanged", Hist->GetBinContent(1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display bin (2,2)", "Repeated Display keeps the representative last 2D histogram bin unchanged", Hist->GetBinContent(2, 2), 4.0, 1e-12) && Passed;
      Passed = Evaluate("SetTitle()", "representative title refresh after redisplay", "Redisplay updates the representative 2D histogram title", MString(Hist->GetTitle()), MString("Display2DImageUpdated")) && Passed;
      Passed = Evaluate("SetXAxisTitle()", "representative x-axis title refresh after redisplay", "Redisplay updates the representative 2D x-axis title", MString(Hist->GetXaxis()->GetTitle()), MString("Updated X")) && Passed;
      Passed = Evaluate("SetYAxisTitle()", "representative y-axis title refresh after redisplay", "Redisplay updates the representative 2D y-axis title", MString(Hist->GetYaxis()->GetTitle()), MString("Updated Y")) && Passed;
      Passed = Evaluate("SetValueAxisTitle()", "representative value-axis title refresh after redisplay", "Redisplay updates the representative 2D value-axis title", MString(Hist->GetZaxis()->GetTitle()), MString("Updated Counts")) && Passed;
      Passed = EvaluateNear("SetYAxis()", "representative configured y minimum", "Display uses the representative updated y-axis minimum configured before display", Hist->GetYaxis()->GetXmin(), -1.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetYAxis()", "representative configured y maximum", "Display uses the representative updated y-axis maximum configured before display", Hist->GetYaxis()->GetXmax(), 3.0, 1e-12) && Passed;
    }

    Image.SetTitle("Display2DImageLiveUpdate");
    Passed = Evaluate("SetTitle()", "representative live histogram title update", "SetTitle updates the representative 2D histogram title after display", MString(Hist->GetTitle()), MString("Display2DImageLiveUpdate")) && Passed;
    Passed = Evaluate("SetTitle()", "representative live canvas title update", "SetTitle updates the representative 2D canvas title after display", MString(Image.GetCanvasPointer()->GetTitle()), MString("Display2DImageLiveUpdate")) && Passed;

    Image.SetYAxis("Display2DImageReconfiguredY", 10.0, 20.0, 2);
    Image.Display(Image.GetCanvasPointer());
    if (Hist != nullptr) {
      Passed = Evaluate("SetYAxis()", "representative post-display y-axis title refresh", "Redisplay updates the representative 2D y-axis title after SetYAxis() is called post-display", MString(Hist->GetYaxis()->GetTitle()), MString("Display2DImageReconfiguredY")) && Passed;
      Passed = EvaluateNear("SetYAxis()", "representative post-display y minimum unchanged in existing histogram", "Redisplay keeps the representative existing 2D histogram y minimum when SetYAxis() is called after display", Hist->GetYaxis()->GetXmin(), -1.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetYAxis()", "representative post-display y maximum unchanged in existing histogram", "Redisplay keeps the representative existing 2D histogram y maximum when SetYAxis() is called after display", Hist->GetYaxis()->GetXmax(), 3.0, 1e-12) && Passed;
    }
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D ReusedCanvas("ReuseCanvasImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    TCanvas ExistingCanvas("RepresentativeExistingCanvas", "RepresentativeExistingCanvas", 10, 10, 300, 300);
    ReusedCanvas.Display(&ExistingCanvas);
    Passed = Evaluate("Display()", "representative existing canvas title reuse", "Display keeps the representative non-empty existing canvas title", MString(ReusedCanvas.GetCanvasPointer()->GetTitle()), MString("RepresentativeExistingCanvas")) && Passed;

    TestImage2D EmptyCanvasTitle("EmptyCanvasTitleImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    TCanvas UntitledCanvas("", "", 10, 10, 300, 300);
    EmptyCanvasTitle.Display(&UntitledCanvas);
    Passed = EvaluateTrue("Display()", "representative empty existing canvas title", "Display assigns a representative non-empty title when the existing canvas title is empty", MString(EmptyCanvasTitle.GetCanvasPointer()->GetTitle()).IsEmpty() == false) && Passed;
  }

  {
    TestImage2D InvalidGeometry;
    InvalidGeometry.ForceInvalidGeometry(0, 2);
    DisableDefaultStreams();
    InvalidGeometry.Display();
    EnableDefaultStreams();
    Passed = EvaluateFalse("Display()", "representative invalid geometry created flag", "Display rejects representative invalid 2D geometry without marking the image as created", InvalidGeometry.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "representative invalid geometry canvas", "Display rejects representative invalid 2D geometry without creating a canvas", InvalidGeometry.CanvasExists()) && Passed;
  }

  {
    double Values[4] = {2.0, 4.0, 6.0, 8.0};
    TestImage2D Image("SetImageArray2D", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Image.Display();
    double UpdatedValues[4] = {1.0, 2.0, 3.0, 4.0};
    Image.SetImageArray(UpdatedValues);

    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    Passed = EvaluateNear("SetImageArray()", "representative redisplay-free average", "SetImageArray updates the representative 2D backing array immediately", Image.GetAverage(), 2.5, 1e-12) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative non-normalized maximum", "SetImageArray does not normalize a displayed 2D image when representative normalization is disabled", Hist->GetMaximum(), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated bin (1,1)", "SetImageArray updates the representative first displayed 2D histogram bin", Hist->GetBinContent(1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated bin (2,2)", "SetImageArray updates the representative last displayed 2D histogram bin", Hist->GetBinContent(2, 2), 4.0, 1e-12) && Passed;
    }

    Image.SetImageArray(nullptr);
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr average", "SetImageArray(nullptr) zeros the representative 2D backing array of an existing image", Image.GetAverage(), 0.0, 1e-12) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (1,1)", "SetImageArray(nullptr) zeros the representative first displayed 2D histogram bin", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (2,1)", "SetImageArray(nullptr) zeros the representative second displayed 2D histogram bin", Hist->GetBinContent(2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (1,2)", "SetImageArray(nullptr) zeros the representative third displayed 2D histogram bin", Hist->GetBinContent(1, 2), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (2,2)", "SetImageArray(nullptr) zeros the representative fourth displayed 2D histogram bin", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
    }
  }

  {
    double Values[4] = {2.0, 4.0, 6.0, 8.0};
    TestImage2D Normalized("Normalized2DImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Normalized.Normalize(true);
    Normalized.Display();

    TH2* Hist = dynamic_cast<TH2*>(Normalized.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Normalize()", "representative normalized maximum", "Display normalizes a representative 2D image so the maximum becomes one", Hist->GetMaximum(), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized bin (1,1)", "Display rescales the representative first normalized 2D histogram bin by the maximum", Hist->GetBinContent(1, 1), 0.25, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized bin (2,2)", "Display rescales the representative last normalized 2D histogram bin by the maximum", Hist->GetBinContent(2, 2), 1.0, 1e-12) && Passed;
    }

    double ZeroValues[4] = {0.0, 0.0, 0.0, 0.0};
    Normalized.SetImageArray(ZeroValues);
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative normalized zero maximum", "SetImageArray keeps the representative normalized 2D histogram at zero instead of dividing by zero for all-zero data", Hist->GetMaximum(), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative normalized zero bin (1,1)", "SetImageArray keeps the representative first normalized 2D histogram bin at zero for all-zero data", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative normalized zero bin (2,2)", "SetImageArray keeps the representative last normalized 2D histogram bin at zero for all-zero data", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
    }

    Normalized.Normalize(false);
    Normalized.Display(Normalized.GetCanvasPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Normalize()", "representative normalization toggle-off bin (1,1)", "Display stops rescaling the representative first 2D histogram bin after Normalize(false)", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalization toggle-off bin (2,2)", "Display stops rescaling the representative last 2D histogram bin after Normalize(false)", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
    }
  }

  {
    double NonFiniteValues[4] = {1.0, numeric_limits<double>::quiet_NaN(), numeric_limits<double>::infinity(), 4.0};
    TestImage2D Image("NonFinite2DImage", NonFiniteValues, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    DisableDefaultStreams();
    Image.Display();
    EnableDefaultStreams();

    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative finite bin before non-finite entries", "Display keeps the representative first finite 2D histogram bin", Hist->GetBinContent(1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative NaN bin", "Display leaves the representative NaN 2D histogram bin at zero", Hist->GetBinContent(2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative Inf bin", "Display leaves the representative infinite 2D histogram bin at zero", Hist->GetBinContent(1, 2), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative final finite bin after non-finite entries", "Display keeps the representative last finite 2D histogram bin", Hist->GetBinContent(2, 2), 4.0, 1e-12) && Passed;
    }

    double UpdatedValues[4] = {numeric_limits<double>::infinity(), 2.0, 3.0, numeric_limits<double>::quiet_NaN()};
    DisableDefaultStreams();
    Image.SetImageArray(UpdatedValues);
    EnableDefaultStreams();
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated Inf bin", "SetImageArray leaves the representative infinite 2D histogram bin at zero", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated finite bin (2,1)", "SetImageArray still updates the representative finite 2D histogram bin after an infinite entry", Hist->GetBinContent(2, 1), 2.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated finite bin (1,2)", "SetImageArray still updates the representative second finite 2D histogram bin after a NaN entry", Hist->GetBinContent(1, 2), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated NaN bin", "SetImageArray leaves the representative NaN 2D histogram bin at zero", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
    }

    double RedisplayValues[4] = {5.0, numeric_limits<double>::infinity(), numeric_limits<double>::quiet_NaN(), 8.0};
    Image.SetImageArray(RedisplayValues);
    DisableDefaultStreams();
    Image.Display(Image.GetCanvasPointer());
    EnableDefaultStreams();
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated display finite bin after non-finite update", "Repeated Display keeps the representative finite 2D histogram bin after non-finite updates", Hist->GetBinContent(1, 1), 5.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display Inf bin", "Repeated Display clears the representative infinite 2D histogram bin instead of leaving stale content behind", Hist->GetBinContent(2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display NaN bin", "Repeated Display clears the representative NaN 2D histogram bin instead of leaving stale content behind", Hist->GetBinContent(1, 2), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display final finite bin", "Repeated Display keeps the representative last finite 2D histogram bin after non-finite updates", Hist->GetBinContent(2, 2), 8.0, 1e-12) && Passed;
    }
  }

  {
    double Values[1000];
    for (int i = 0; i < 1000; ++i) {
      Values[i] = i + 1;
    }
    TestImage2D ExtremeAspect("ExtremeAspect2DImage", Values, "X Axis", 0.0, 1000.0, 1000, "Y Axis", 0.0, 1.0, 1, "Counts");
    ExtremeAspect.Display();
    Passed = EvaluateTrue("Display()", "representative extreme aspect canvas exists", "Display succeeds for a representative extreme-aspect 2D image", ExtremeAspect.CanvasExists()) && Passed;
    Passed = EvaluateTrue("Display()", "representative extreme aspect created", "Display marks a representative extreme-aspect 2D image as created", ExtremeAspect.IsCreated()) && Passed;
    Passed = EvaluateTrue("Display()", "representative extreme aspect minimum height", "Display clamps the representative extreme-aspect canvas height to a practical minimum", ExtremeAspect.GetCanvasPointer()->GetWh() >= 100) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D Fallbacks("Fallback2DImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Fallbacks.SetDrawOption(MImage::c_COL);
    Passed = Evaluate("SetDrawOption()", "representative COL draw option", "The representative 2D COL draw option is translated to the expected ROOT string", Fallbacks.GetDrawOptionString(), MString("COL")) && Passed;
    Fallbacks.SetDrawOption(MImage::c_COLCONT);
    Passed = Evaluate("SetDrawOption()", "representative COLCONT draw option", "The representative 2D COLCONT draw option is translated to the expected ROOT string", Fallbacks.GetDrawOptionString(), MString("COLCONT0")) && Passed;
    DisableDefaultStreams();
    Fallbacks.SetDrawOption(999);
    Fallbacks.SetSpectrum(999);
    EnableDefaultStreams();
    Passed = Evaluate("SetDrawOption()", "representative invalid draw option", "Invalid 2D draw options fall back to the representative COLZ ROOT draw string", Fallbacks.GetDrawOptionString(), MString("COLZ")) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D Image("Reset2DImage", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Image.Display();
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());

    Image.Reset();
    Passed = EvaluateNear("Reset()", "representative backing array bin 0", "Reset clears the representative first 2D backing-array value", Image.GetArrayValueRaw(0), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative backing array bin 1", "Reset clears the representative second 2D backing-array value", Image.GetArrayValueRaw(1), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative backing array bin 2", "Reset clears the representative third 2D backing-array value", Image.GetArrayValueRaw(2), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Reset()", "representative backing array bin 3", "Reset clears the representative fourth 2D backing-array value", Image.GetArrayValueRaw(3), 0.0, 1e-12) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Reset()", "representative histogram bin (1,1)", "Reset also clears the representative first 2D histogram bin", Hist->GetBinContent(1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Reset()", "representative histogram bin (2,1)", "Reset also clears the representative second 2D histogram bin", Hist->GetBinContent(2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Reset()", "representative histogram bin (1,2)", "Reset also clears the representative third 2D histogram bin", Hist->GetBinContent(1, 2), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Reset()", "representative histogram bin (2,2)", "Reset also clears the representative fourth 2D histogram bin", Hist->GetBinContent(2, 2), 0.0, 1e-12) && Passed;
    }

    Image.Display();
    Image.SaveAs("/tmp/UTImage2D_existing_canvas.png");
    Passed = EvaluateTrue("SaveAs()", "representative existing canvas path", "SaveAs is callable on a representative 2D image with an existing canvas", true) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImage2D Original("Clone2DSource", Values, "X Axis", 0.0, 2.0, 2, "Y Axis", 0.0, 2.0, 2, "Counts");
    Original.Normalize(true);
    MImage* CloneBase = Original.Clone();
    TestImage2D* Clone = dynamic_cast<TestImage2D*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative clone type", "Clone returns a representative object of the 2D image type", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative cloned dimensions", "Clone preserves the representative 2D image dimensionality", Clone->GetDimensions(), 2U) && Passed;
      Passed = EvaluateNear("Clone()", "representative cloned average", "Clone preserves the representative 2D backing data", Clone->GetAverage(), 2.5, 1e-12) && Passed;
      Clone->Display();
      Passed = EvaluateTrue("Clone()", "representative cloned display created", "A representative 2D clone can still be displayed after cloning", Clone->IsCreated()) && Passed;
      TH2* CloneHist = dynamic_cast<TH2*>(Clone->GetHistogramPointer());
      if (CloneHist != nullptr) {
        Passed = EvaluateNear("Clone()", "representative cloned normalized maximum", "Clone preserves the representative Normalize flag in observable 2D display behavior", CloneHist->GetMaximum(), 1.0, 1e-12) && Passed;
      }
    }
    delete CloneBase;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTImage2D Test;
  return Test.Run() == true ? 0 : 1;
}
