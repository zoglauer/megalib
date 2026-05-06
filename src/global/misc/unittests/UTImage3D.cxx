/*
 * UTImage3D.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH3.h>
#include <TList.h>

// MEGAlib:
#include "MImage3D.h"
#include "MUnitTest.h"


class TestImage3D : public MImage3D
{
public:
  TestImage3D()
    : MImage3D(),
      m_TestZTitle("z-Axis"),
      m_TestZMin(0.0),
      m_TestZMax(1.0),
      m_TestZNBins(1) {}

  TestImage3D(MString Title, double* IA,
              MString xTitle, double xMin, double xMax, int xNBins,
              MString yTitle, double yMin, double yMax, int yNBins,
              MString zTitle, double zMin, double zMax, int zNBins,
              MString vTitle = "", int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ) :
    MImage3D(Title, IA, xTitle, xMin, xMax, xNBins,
             yTitle, yMin, yMax, yNBins,
             zTitle, zMin, zMax, zNBins, vTitle, Spectrum, DrawOption),
    m_TestZTitle(zTitle),
    m_TestZMin(zMin),
    m_TestZMax(zMax),
    m_TestZNBins(zNBins) {}

  virtual MImage* Clone()
  {
    TestImage3D* I =
      new TestImage3D(m_Title, m_IA,
                      m_xTitle, m_xMin, m_xMax, m_xNBins,
                      m_yTitle, m_yMin, m_yMax, m_yNBins,
                      m_TestZTitle, m_TestZMin, m_TestZMax, m_TestZNBins,
                      m_vTitle, m_Spectrum, m_DrawOption);
    I->Normalize(m_Normalize);
    return I;
  }

  TH1* GetHistogramPointer() const { return m_Histogram; }
  TCanvas* GetCanvasPointer() const { return m_Canvas; }
  int GetXNBinsRaw() const { return m_xNBins; }
  int GetYNBinsRaw() const { return m_yNBins; }
  int GetZNBinsRaw() const { return m_TestZNBins; }
  double GetArrayValueRaw(int Index) const { return m_IA[Index]; }

  virtual void SetZAxis(MString zTitle, double zMin, double zMax, int zNBins)
  {
    MImage3D::SetZAxis(zTitle, zMin, zMax, zNBins);
    m_TestZTitle = zTitle;
    m_TestZMin = zMin;
    m_TestZMax = zMax;
    m_TestZNBins = zNBins;
  }

  void ForceEmpty()
  {
    if (m_IA != nullptr) {
      delete [] m_IA;
      m_IA = nullptr;
    }
    SetXAxis("x-Axis", 0, 1, 0);
    SetYAxis("y-Axis", 0, 1, 0);
    SetZAxis("z-Axis", 0, 1, 0);
  }

  void ForceNullArray()
  {
    if (m_IA != nullptr) {
      delete [] m_IA;
      m_IA = nullptr;
    }
  }

private:
  MString m_TestZTitle;
  double m_TestZMin;
  double m_TestZMax;
  int m_TestZNBins;
};


static TCanvas* FindCanvasByName(const MString& Name)
{
  return dynamic_cast<TCanvas*>(gROOT->FindObject(Name.Data()));
}


static TH1* GetFirstHistogramPrimitive(TCanvas* Canvas)
{
  if (Canvas == nullptr || Canvas->GetListOfPrimitives() == nullptr) {
    return nullptr;
  }

  TList* Primitives = Canvas->GetListOfPrimitives();
  for (int i = 0; i < Primitives->GetSize(); ++i) {
    TH1* Histogram = dynamic_cast<TH1*>(Primitives->At(i));
    if (Histogram != nullptr) {
      return Histogram;
    }
  }

  return nullptr;
}


//! Unit test class for MImage3D
class UTImage3D : public MUnitTest
{
public:
  UTImage3D() : MUnitTest("UTImage3D") {}
  virtual ~UTImage3D() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTImage3D::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    TestImage3D Default;
    Passed = Evaluate("GetDimensions()", "default constructor", "The default 3D image reports the representative three-dimensional image shape", Default.GetDimensions(), 3U) && Passed;
    Passed = EvaluateFalse("IsCreated()", "default constructor", "A default 3D image starts without a representative created histogram", Default.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "default constructor", "A default 3D image reports that no representative canvas exists yet", Default.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "default constructor", "The default 3D image configures one representative entry for the default 1x1x1 image", Default.GetNEntries(), 1) && Passed;
    Passed = Evaluate("GetXNBinsRaw()", "default constructor", "The default 3D image configures one representative x bin", Default.GetXNBinsRaw(), 1) && Passed;
    Passed = Evaluate("GetYNBinsRaw()", "default constructor", "The default 3D image configures one representative y bin", Default.GetYNBinsRaw(), 1) && Passed;
    Passed = Evaluate("GetZNBinsRaw()", "default constructor", "The default 3D image configures one representative z bin", Default.GetZNBinsRaw(), 1) && Passed;
    Passed = EvaluateNear("GetAverage()", "default constructor", "The default 3D image average is zero for the representative zero-filled default entry", Default.GetAverage(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArrayValueRaw()", "default constructor", "The default 3D image backing array is initialized with a representative zero value", Default.GetArrayValueRaw(0), 0.0, 1e-12) && Passed;

    vector<double> Coordinate;
    double Maximum = 99.0;
    Default.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "default constructor maximum value", "DetermineMaximum returns the representative zero-valued default maximum", Maximum, 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "default constructor coordinate count", "DetermineMaximum returns three representative coordinates for the default 3D image", Coordinate.size(), 3UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor x coordinate", "DetermineMaximum returns the representative default x-bin center", Coordinate[0], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor y coordinate", "DetermineMaximum returns the representative default y-bin center", Coordinate[1], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "default constructor z coordinate", "DetermineMaximum returns the representative default z-bin center", Coordinate[2], 0.5, 1e-12) && Passed;

    Default.Reset();
    Passed = EvaluateNear("Reset()", "default constructor pre-display average", "Reset is safely callable on a representative 3D image before any histogram has been created", Default.GetAverage(), 0.0, 1e-12) && Passed;

    DisableDefaultStreams();
    Default.SaveAs("/tmp/UTImage3D/does_not_exist.png");
    EnableDefaultStreams();
    Passed = EvaluateTrue("SaveAs()", "default constructor no-canvas path", "SaveAs is safely callable on a representative 3D image without a canvas", true) && Passed;
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Image("Representative3DImage", Values,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    Passed = EvaluateNear("GetAverage()", "standard constructor", "The standard 3D constructor stores representative backing data for averaging", Image.GetAverage(), 4.5, 1e-12) && Passed;
    Passed = Evaluate("GetNEntries()", "standard constructor", "The standard 3D constructor reports the representative number of 3D entries", Image.GetNEntries(), 8) && Passed;

    vector<double> Coordinate;
    double Maximum = 0.0;
    Image.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum value", "DetermineMaximum finds the representative largest stored 3D value", Maximum, 8.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative positive coordinate count", "DetermineMaximum returns three representative coordinates for a 3D image", Coordinate.size(), 3UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum x coordinate", "DetermineMaximum reports the representative x-bin center of the 3D maximum", Coordinate[0], 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum y coordinate", "DetermineMaximum reports the representative y-bin center of the 3D maximum", Coordinate[1], 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative positive maximum z coordinate", "DetermineMaximum reports the representative z-bin center of the 3D maximum", Coordinate[2], 1.5, 1e-12) && Passed;
  }

  {
    double Values[12] = {1.0, 2.0, 3.0,
                         4.0, 5.0, 6.0,
                         7.0, 8.0, 9.0,
                         10.0, 11.0, 12.0};
    TestImage3D Asymmetric("Asymmetric3DImage", Values,
                           "X Axis", 0.0, 3.0, 3,
                           "Y Axis", 0.0, 2.0, 2,
                           "Z Axis", 0.0, 2.0, 2,
                           "Counts");
    Passed = Evaluate("GetNEntries()", "asymmetric constructor", "The asymmetric 3D constructor reports the representative number of 3D entries", Asymmetric.GetNEntries(), 12) && Passed;
    Asymmetric.Display();
    TH3* Hist = dynamic_cast<TH3*>(Asymmetric.GetHistogramPointer());
    Passed = EvaluateTrue("Display()", "representative TH3 type", "Display stores the representative 3D image in a TH3 histogram", Hist != nullptr) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative asymmetric bin (1,1,1)", "Display stores the representative first asymmetric 3D bin with the expected x-major stride", Hist->GetBinContent(1, 1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (3,1,1)", "Display stores the representative last bin of the first asymmetric 3D x-row with the expected x-major stride", Hist->GetBinContent(3, 1, 1), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (1,2,1)", "Display stores the representative first bin of the second asymmetric 3D y-row with the expected x-major stride", Hist->GetBinContent(1, 2, 1), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (1,1,2)", "Display stores the representative first bin of the second asymmetric 3D z-slice with the expected x-major stride", Hist->GetBinContent(1, 1, 2), 7.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative asymmetric bin (3,2,2)", "Display stores the representative last asymmetric 3D bin with the expected x-major stride", Hist->GetBinContent(3, 2, 2), 12.0, 1e-12) && Passed;
    }
  }

  {
    double NegativeValues[8] = {-9.0, -2.0, -7.0, -3.0, -8.0, -4.0, -6.0, -5.0};
    TestImage3D Negative("Negative3DImage", NegativeValues,
                         "X Axis", 0.0, 2.0, 2,
                         "Y Axis", 0.0, 2.0, 2,
                         "Z Axis", 0.0, 2.0, 2,
                         "Counts");
    vector<double> Coordinate;
    double Maximum = 0.0;
    Negative.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum value", "DetermineMaximum finds the representative 3D maximum even when all values are negative", Maximum, -2.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative all-negative coordinate count", "DetermineMaximum still returns three representative coordinates for all-negative 3D data", Coordinate.size(), 3UL) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum x coordinate", "DetermineMaximum reports the representative x-bin center of the all-negative 3D maximum", Coordinate[0], 1.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum y coordinate", "DetermineMaximum reports the representative y-bin center of the all-negative 3D maximum", Coordinate[1], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative all-negative maximum z coordinate", "DetermineMaximum reports the representative z-bin center of the all-negative 3D maximum", Coordinate[2], 0.5, 1e-12) && Passed;
  }

  {
    double FirstIsMaximum[8] = {-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0};
    TestImage3D First("FirstMaximum3DImage", FirstIsMaximum,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    vector<double> Coordinate;
    double Maximum = 0.0;
    First.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum value", "DetermineMaximum keeps the representative first 3D element as the maximum when nothing exceeds it", Maximum, -1.0, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum x coordinate", "DetermineMaximum keeps the representative first-bin x coordinate when the maximum is at (0,0,0)", Coordinate[0], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum y coordinate", "DetermineMaximum keeps the representative first-bin y coordinate when the maximum is at (0,0,0)", Coordinate[1], 0.5, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineMaximum()", "representative first-bin maximum z coordinate", "DetermineMaximum keeps the representative first-bin z coordinate when the maximum is at (0,0,0)", Coordinate[2], 0.5, 1e-12) && Passed;
  }

  {
    TestImage3D Empty;
    Empty.ForceEmpty();
    vector<double> Coordinate;
    double Maximum = 17.0;
    Empty.DetermineMaximum(Maximum, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative forced-empty maximum value", "DetermineMaximum returns zero for a representative empty 3D image", Maximum, 0.0, 1e-12) && Passed;
    Passed = EvaluateSize("DetermineMaximum()", "representative forced-empty coordinate count", "DetermineMaximum leaves the representative coordinate list empty for an empty 3D image", Coordinate.size(), 0UL) && Passed;

    TestImage3D NullData;
    NullData.ForceNullArray();
    DisableDefaultStreams();
    NullData.Display();
    EnableDefaultStreams();
    Passed = EvaluateFalse("Display()", "representative null-data created flag", "Display rejects a representative 3D image without backing data without marking the image as created", NullData.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "representative null-data canvas", "Display rejects a representative 3D image without backing data without creating a canvas", NullData.CanvasExists()) && Passed;
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Image("Display3DImage", Values,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative first display", "Display creates a representative 3D histogram and marks the image as created", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("Display()", "representative histogram created", "Display creates a representative ROOT 3D histogram", Image.GetHistogramPointer() != nullptr) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative canvas exists", "Display creates a representative 3D canvas that can be found afterwards", Image.CanvasExists()) && Passed;

    TH3* Hist = dynamic_cast<TH3*>(Image.GetHistogramPointer());
    Passed = EvaluateTrue("Display()", "representative TH3 type", "Display stores the representative image in a TH3 histogram", Hist != nullptr) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative first display bin (1,1,1)", "Display fills the representative first 3D histogram bin from the backing array", Hist->GetBinContent(1, 1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative first display bin (2,2,2)", "Display fills the representative last 3D histogram bin from the backing array", Hist->GetBinContent(2, 2, 2), 8.0, 1e-12) && Passed;
    }

    Image.SetTitle("Display3DImageUpdated");
    Image.Display(Image.GetCanvasPointer());
    if (Hist != nullptr) {
      Passed = Evaluate("SetTitle()", "representative live histogram title update", "SetTitle updates the representative 3D histogram title after display", MString(Hist->GetTitle()), MString("Display3DImageUpdated")) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display bin (1,1,1)", "Repeated Display keeps the representative first 3D histogram bin unchanged", Hist->GetBinContent(1, 1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display bin (2,2,2)", "Repeated Display keeps the representative last 3D histogram bin unchanged", Hist->GetBinContent(2, 2, 2), 8.0, 1e-12) && Passed;
    }
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Projection("Projection3DImage", Values,
                           "X Axis", 0.0, 2.0, 2,
                           "Y Axis", 0.0, 2.0, 2,
                           "Z Axis", 0.0, 2.0, 2,
                           "Counts");
    Projection.Display();
    MString MainCanvasName = Projection.GetCanvasPointer()->GetName();
    TCanvas* XYCanvas = FindCanvasByName(MainCanvasName + "_XY");
    TCanvas* XZCanvas = FindCanvasByName(MainCanvasName + "_XZ");
    TCanvas* YZCanvas = FindCanvasByName(MainCanvasName + "_YZ");
    Passed = EvaluateTrue("Display()", "representative XY projection canvas", "Display creates the representative XY projection canvas", XYCanvas != nullptr) && Passed;
    Passed = EvaluateTrue("Display()", "representative XZ projection canvas", "Display creates the representative XZ projection canvas", XZCanvas != nullptr) && Passed;
    Passed = EvaluateTrue("Display()", "representative YZ projection canvas", "Display creates the representative YZ projection canvas", YZCanvas != nullptr) && Passed;
    TH1* XYHist = GetFirstHistogramPrimitive(XYCanvas);
    TH1* XZHist = GetFirstHistogramPrimitive(XZCanvas);
    TH1* YZHist = GetFirstHistogramPrimitive(YZCanvas);
    Passed = EvaluateTrue("Display()", "representative XY projection histogram", "Display creates the representative XY projection histogram", XYHist != nullptr) && Passed;
    Passed = EvaluateTrue("Display()", "representative XZ projection histogram", "Display creates the representative XZ projection histogram", XZHist != nullptr) && Passed;
    Passed = EvaluateTrue("Display()", "representative YZ projection histogram", "Display creates the representative YZ projection histogram", YZHist != nullptr) && Passed;
    if (XYHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative XY projection bin (1,1)", "The representative XY projection sums over z for the first bin", XYHist->GetBinContent(1, 1), 6.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative XY projection bin (2,2)", "The representative XY projection sums over z for the last bin", XYHist->GetBinContent(2, 2), 12.0, 1e-12) && Passed;
    }
    if (XZHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative XZ projection bin (1,1)", "The representative XZ projection sums over y for the first bin", XZHist->GetBinContent(1, 1), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative XZ projection bin (2,2)", "The representative XZ projection sums over y for the last bin", XZHist->GetBinContent(2, 2), 14.0, 1e-12) && Passed;
    }
    if (YZHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative YZ projection bin (1,1)", "The representative YZ projection sums over x for the first bin", YZHist->GetBinContent(1, 1), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative YZ projection bin (2,2)", "The representative YZ projection sums over x for the last bin", YZHist->GetBinContent(2, 2), 15.0, 1e-12) && Passed;
    }

    double UpdatedValues[8] = {21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0};
    Projection.SetImageArray(UpdatedValues);
    XYCanvas = FindCanvasByName(MainCanvasName + "_XY");
    XZCanvas = FindCanvasByName(MainCanvasName + "_XZ");
    YZCanvas = FindCanvasByName(MainCanvasName + "_YZ");
    XYHist = GetFirstHistogramPrimitive(XYCanvas);
    XZHist = GetFirstHistogramPrimitive(XZCanvas);
    YZHist = GetFirstHistogramPrimitive(YZCanvas);
    if (XYHist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated XY projection bin (1,1)", "SetImageArray updates the representative XY projection first bin without requiring a follow-up Display()", XYHist->GetBinContent(1, 1), 46.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated XY projection bin (2,2)", "SetImageArray updates the representative XY projection last bin without requiring a follow-up Display()", XYHist->GetBinContent(2, 2), 52.0, 1e-12) && Passed;
    }
    if (XZHist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated XZ projection bin (1,1)", "SetImageArray updates the representative XZ projection first bin without requiring a follow-up Display()", XZHist->GetBinContent(1, 1), 44.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated XZ projection bin (2,2)", "SetImageArray updates the representative XZ projection last bin without requiring a follow-up Display()", XZHist->GetBinContent(2, 2), 54.0, 1e-12) && Passed;
    }
    if (YZHist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated YZ projection bin (1,1)", "SetImageArray updates the representative YZ projection first bin without requiring a follow-up Display()", YZHist->GetBinContent(1, 1), 43.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated YZ projection bin (2,2)", "SetImageArray updates the representative YZ projection last bin without requiring a follow-up Display()", YZHist->GetBinContent(2, 2), 55.0, 1e-12) && Passed;
    }
    Projection.Display(Projection.GetCanvasPointer());
    XYCanvas = FindCanvasByName(MainCanvasName + "_XY");
    XZCanvas = FindCanvasByName(MainCanvasName + "_XZ");
    YZCanvas = FindCanvasByName(MainCanvasName + "_YZ");
    XYHist = GetFirstHistogramPrimitive(XYCanvas);
    XZHist = GetFirstHistogramPrimitive(XZCanvas);
    YZHist = GetFirstHistogramPrimitive(YZCanvas);
    if (XYHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated XY projection bin (1,1)", "Repeated Display updates the representative XY projection first bin", XYHist->GetBinContent(1, 1), 46.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated XY projection bin (2,2)", "Repeated Display updates the representative XY projection last bin", XYHist->GetBinContent(2, 2), 52.0, 1e-12) && Passed;
    }
    if (XZHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated XZ projection bin (1,1)", "Repeated Display updates the representative XZ projection first bin", XZHist->GetBinContent(1, 1), 44.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated XZ projection bin (2,2)", "Repeated Display updates the representative XZ projection last bin", XZHist->GetBinContent(2, 2), 54.0, 1e-12) && Passed;
    }
    if (YZHist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated YZ projection bin (1,1)", "Repeated Display updates the representative YZ projection first bin", YZHist->GetBinContent(1, 1), 43.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated YZ projection bin (2,2)", "Repeated Display updates the representative YZ projection last bin", YZHist->GetBinContent(2, 2), 55.0, 1e-12) && Passed;
    }
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Reconfigured("ReconfiguredZAxis3D", Values,
                             "X Axis", 0.0, 2.0, 2,
                             "Y Axis", 0.0, 2.0, 2,
                             "Z Axis", 0.0, 2.0, 2,
                             "Counts");
    Reconfigured.SetZAxis("Configured Z", -2.0, 4.0, 4);
    Passed = Evaluate("GetNEntries()", "post-construction SetZAxis", "A representative post-construction SetZAxis call updates the number of 3D entries", Reconfigured.GetNEntries(), 16) && Passed;
    Passed = Evaluate("GetZNBinsRaw()", "post-construction SetZAxis", "A representative post-construction SetZAxis call updates the raw z-bin count", Reconfigured.GetZNBinsRaw(), 4) && Passed;
  }

  {
    double Values[8] = {2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0};
    TestImage3D Normalized("Normalized3DImage", Values,
                           "X Axis", 0.0, 2.0, 2,
                           "Y Axis", 0.0, 2.0, 2,
                           "Z Axis", 0.0, 2.0, 2,
                           "Counts");
    Normalized.Normalize(true);
    Normalized.Display();

    TH3* Hist = dynamic_cast<TH3*>(Normalized.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Normalize()", "representative normalized maximum", "Display normalizes a representative 3D image so the maximum becomes one", Hist->GetMaximum(), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized first bin", "Display rescales the representative first normalized 3D histogram bin by the maximum", Hist->GetBinContent(1, 1, 1), 0.125, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized last bin", "Display rescales the representative last normalized 3D histogram bin by the maximum", Hist->GetBinContent(2, 2, 2), 1.0, 1e-12) && Passed;
    }

    Normalized.Normalize(false);
    Normalized.Display(Normalized.GetCanvasPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Normalize()", "representative normalization toggle-off first bin", "Display stops rescaling the representative first 3D histogram bin after Normalize(false)", Hist->GetBinContent(1, 1, 1), 2.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalization toggle-off last bin", "Display stops rescaling the representative last 3D histogram bin after Normalize(false)", Hist->GetBinContent(2, 2, 2), 16.0, 1e-12) && Passed;
    }

    double ZeroValues[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    TestImage3D ZeroNormalized("ZeroNormalized3DImage", ZeroValues,
                               "X Axis", 0.0, 2.0, 2,
                               "Y Axis", 0.0, 2.0, 2,
                               "Z Axis", 0.0, 2.0, 2,
                               "Counts");
    ZeroNormalized.Normalize(true);
    ZeroNormalized.Display();
    TH3* ZeroHist = dynamic_cast<TH3*>(ZeroNormalized.GetHistogramPointer());
    if (ZeroHist != nullptr) {
      Passed = EvaluateNear("Normalize()", "representative normalized zero maximum", "Display keeps the representative normalized 3D histogram at zero when the maximum is not positive", ZeroHist->GetMaximum(), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized zero first bin", "Display leaves the representative first normalized 3D histogram bin at zero when scaling is skipped", ZeroHist->GetBinContent(1, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Normalize()", "representative normalized zero last bin", "Display leaves the representative last normalized 3D histogram bin at zero when scaling is skipped", ZeroHist->GetBinContent(2, 2, 2), 0.0, 1e-12) && Passed;
    }
    Normalized.Reset();
    if (Hist != nullptr) {
      Passed = EvaluateNear("Reset()", "representative post-display reset first bin", "Reset zeros the representative first 3D histogram bin after display", Hist->GetBinContent(1, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Reset()", "representative post-display reset last bin", "Reset zeros the representative last 3D histogram bin after display", Hist->GetBinContent(2, 2, 2), 0.0, 1e-12) && Passed;
    }
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Image("SetImageArray3D", Values,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    Image.Display();
    double UpdatedValues[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    Image.SetImageArray(UpdatedValues);

    TH3* Hist = dynamic_cast<TH3*>(Image.GetHistogramPointer());
    Passed = EvaluateNear("SetImageArray()", "representative redisplay-free average", "SetImageArray updates the representative 3D backing array immediately", Image.GetAverage(), 4.5, 1e-12) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated bin (1,1,1)", "SetImageArray updates the representative first displayed 3D histogram bin", Hist->GetBinContent(1, 1, 1), 8.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated bin (2,2,2)", "SetImageArray updates the representative last displayed 3D histogram bin", Hist->GetBinContent(2, 2, 2), 1.0, 1e-12) && Passed;
    }

    Image.SetImageArray(nullptr);
    Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr average", "SetImageArray(nullptr) zeros the representative 3D backing array of an existing image", Image.GetAverage(), 0.0, 1e-12) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (1,1,1)", "SetImageArray(nullptr) zeros the representative first displayed 3D histogram bin", Hist->GetBinContent(1, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative explicit nullptr bin (2,2,2)", "SetImageArray(nullptr) zeros the representative last displayed 3D histogram bin", Hist->GetBinContent(2, 2, 2), 0.0, 1e-12) && Passed;
    }
  }

  {
    double NonFiniteValues[8] = {1.0, numeric_limits<double>::quiet_NaN(), numeric_limits<double>::infinity(), 4.0,
                                 5.0, 6.0, 7.0, 8.0};
    TestImage3D Image("NonFinite3DImage", NonFiniteValues,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    DisableDefaultStreams();
    Image.Display();
    EnableDefaultStreams();

    TH3* Hist = dynamic_cast<TH3*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative finite bin before non-finite entries", "Display keeps the representative first finite 3D histogram bin", Hist->GetBinContent(1, 1, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative NaN bin", "Display leaves the representative NaN 3D histogram bin at zero", Hist->GetBinContent(2, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative Inf bin", "Display leaves the representative infinite 3D histogram bin at zero", Hist->GetBinContent(1, 2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative final finite bin after non-finite entries", "Display keeps the representative last finite 3D histogram bin", Hist->GetBinContent(2, 2, 2), 8.0, 1e-12) && Passed;
    }

    double UpdatedValues[8] = {9.0, numeric_limits<double>::infinity(), numeric_limits<double>::quiet_NaN(), 12.0,
                               13.0, 14.0, 15.0, 16.0};
    DisableDefaultStreams();
    Image.SetImageArray(UpdatedValues);
    EnableDefaultStreams();
    if (Hist != nullptr) {
      Passed = EvaluateNear("SetImageArray()", "representative updated Inf bin", "SetImageArray leaves the representative infinite 3D histogram bin at zero", Hist->GetBinContent(2, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated finite bin", "SetImageArray still updates the representative finite 3D histogram bin after an infinite entry", Hist->GetBinContent(1, 1, 1), 9.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated NaN bin", "SetImageArray leaves the representative NaN 3D histogram bin at zero", Hist->GetBinContent(1, 2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray()", "representative updated last bin", "SetImageArray still updates the representative last 3D histogram bin after a NaN entry", Hist->GetBinContent(2, 2, 2), 16.0, 1e-12) && Passed;
    }

    double RedisplayValues[8] = {21.0, numeric_limits<double>::infinity(), numeric_limits<double>::quiet_NaN(), 24.0,
                                 25.0, 26.0, 27.0, 28.0};
    Image.SetImageArray(RedisplayValues);
    DisableDefaultStreams();
    Image.Display(Image.GetCanvasPointer());
    EnableDefaultStreams();
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative repeated display finite bin after non-finite update", "Repeated Display keeps the representative finite 3D histogram bin after non-finite updates", Hist->GetBinContent(1, 1, 1), 21.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display Inf bin", "Repeated Display clears the representative infinite 3D histogram bin instead of leaving stale content behind", Hist->GetBinContent(2, 1, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display NaN bin", "Repeated Display clears the representative NaN 3D histogram bin instead of leaving stale content behind", Hist->GetBinContent(1, 2, 1), 0.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative repeated display final finite bin", "Repeated Display keeps the representative last finite 3D histogram bin after non-finite updates", Hist->GetBinContent(2, 2, 2), 28.0, 1e-12) && Passed;
    }
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Image("SaveAs3DImage", Values,
                      "X Axis", 0.0, 2.0, 2,
                      "Y Axis", 0.0, 2.0, 2,
                      "Z Axis", 0.0, 2.0, 2,
                      "Counts");
    Image.Display();
    MString FileName = "/tmp/UTImage3D_existing_canvas.png";
    unlink(FileName.Data());
    Image.SaveAs(FileName);
    struct stat Info;
    Passed = EvaluateTrue("SaveAs()", "representative existing canvas path", "SaveAs creates the representative 3D output file when a canvas exists", stat(FileName.Data(), &Info) == 0 && S_ISREG(Info.st_mode) != 0) && Passed;
  }

  {
    double Values[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    TestImage3D Original("Clone3DSource", Values,
                         "X Axis", 0.0, 2.0, 2,
                         "Y Axis", 0.0, 2.0, 2,
                         "Z Axis", 0.0, 2.0, 2,
                         "Counts");
    Original.Normalize(true);
    MImage* CloneBase = Original.Clone();
    TestImage3D* Clone = dynamic_cast<TestImage3D*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative clone type", "Clone returns a representative object of the 3D image type", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative cloned dimensions", "Clone preserves the representative 3D image dimensionality", Clone->GetDimensions(), 3U) && Passed;
      Passed = EvaluateNear("Clone()", "representative cloned average", "Clone preserves the representative 3D backing data", Clone->GetAverage(), 4.5, 1e-12) && Passed;
      Clone->Display();
      Passed = EvaluateTrue("Clone()", "representative cloned display created", "A representative 3D clone can still be displayed after cloning", Clone->IsCreated()) && Passed;
      TH3* CloneHist = dynamic_cast<TH3*>(Clone->GetHistogramPointer());
      if (CloneHist != nullptr) {
        Passed = EvaluateNear("Clone()", "representative cloned normalized maximum", "Clone preserves the representative Normalize flag in observable 3D display behavior", CloneHist->GetMaximum(), 1.0, 1e-12) && Passed;
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
  UTImage3D Test;
  return Test.Run() == true ? 0 : 1;
}
