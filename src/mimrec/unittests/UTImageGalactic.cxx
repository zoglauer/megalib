/*
 * UTImageGalactic.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fstream>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TCanvas.h>
#include <TH2.h>
#include <TH2D.h>

// MEGAlib:
#include "MBinnerFISBEL.h"
#include "MImageGalactic.h"
#include "MUnitTest.h"


class TestImageGalactic : public MImageGalactic
{
public:
  TestImageGalactic() : MImageGalactic(), m_TestProjection(MImageProjection::c_None) {}

  TestImageGalactic(MString Title, double* IA,
                    MString xTitle, double xMin, double xMax, int xNBins,
                    MString yTitle, double yMin, double yMax, int yNBins,
                    MString vTitle = "", int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ,
                    MString SourceCatalog = "") :
    MImageGalactic(Title, IA, xTitle, xMin, xMax, xNBins,
                   yTitle, yMin, yMax, yNBins, vTitle, Spectrum, DrawOption, SourceCatalog),
    m_TestProjection(MImageProjection::c_None) {}

  virtual MImage* Clone()
  {
    TestImageGalactic* I =
      new TestImageGalactic(m_Title, m_IA,
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
    MImageGalactic::SetProjection(Projection);
  }

  TH1* GetHistogramPointer() const { return m_Histogram; }
  TCanvas* GetCanvasPointer() const { return m_Canvas; }
  double GetArrayValueRaw(int Index) const { return m_IA[Index]; }
  MImageProjection GetProjection() const { return m_TestProjection; }
  void InvalidateArray() { m_IA = nullptr; }
  void RunAddNamedSources() { AddNamedSources(); }

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


//! Unit test class for MImageGalactic
class UTImageGalactic : public MUnitTest
{
public:
  UTImageGalactic() : MUnitTest("UTImageGalactic") {}
  virtual ~UTImageGalactic() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTImageGalactic::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    TestImageGalactic Default;
    Passed = Evaluate("GetDimensions()", "default constructor", "A default galactic image reports the representative two-dimensional image shape", Default.GetDimensions(), 2U) && Passed;
    Passed = EvaluateFalse("IsCreated()", "default constructor", "A default galactic image starts without a representative created histogram", Default.IsCreated()) && Passed;
    Passed = EvaluateFalse("CanvasExists()", "default constructor", "A default galactic image reports that no representative canvas exists yet", Default.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "default constructor", "A default galactic image configures one representative entry for the default 1x1 image", Default.GetNEntries(), 1) && Passed;
    Passed = EvaluateNear("GetAverage()", "default constructor", "The default galactic image average is zero for the representative zero-filled default entry", Default.GetAverage(), 0.0, 1e-12) && Passed;
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageGalactic Image("GalacticNoneImage", Values,
                            "Lon", 0.0, 360.0, 3,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();

    Passed = EvaluateTrue("Display()", "representative created flag", "Display creates a representative galactic histogram and marks the image as created", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative canvas exists", "Display creates a representative galactic canvas that can be found afterwards", Image.CanvasExists()) && Passed;
    Passed = Evaluate("GetNEntries()", "representative asymmetric image entries", "A representative asymmetric galactic image reports the correct entry count", Image.GetNEntries(), 6) && Passed;

    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    Passed = EvaluateTrue("Display()", "representative TH2 type", "Display stores the representative galactic image in a TH2 histogram", Hist != nullptr) && Passed;
    if (Hist != nullptr) {
      Passed = EvaluateNear("Display()", "representative bin (1,1)", "Display stores the representative first galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(3, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (2,1)", "Display stores the representative second galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(2, 1), 2.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (3,1)", "Display stores the representative third galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(1, 1), 3.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (1,2)", "Display stores the representative fourth galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(3, 2), 4.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (2,2)", "Display stores the representative fifth galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(2, 2), 5.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative bin (3,2)", "Display stores the representative sixth galactic bin with the expected inverted x-axis mapping", Hist->GetBinContent(1, 2), 6.0, 1e-12) && Passed;
      Passed = EvaluateTrue("Display()", "representative non-zero bins", "The representative galactic histogram has rendered content after display", CountNonZeroBins(Hist) > 0) && Passed;
    }

    Image.SetImageArray((double*) nullptr);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative none nullptr clears bins", "SetImageArray(nullptr) zeros the representative galactic histogram in unprojected mode", CountNonZeroBins(Hist), 0) && Passed;
    }

    const char* SaveFile = "/tmp/UTImageGalactic_existing_canvas.png";
    unlink(SaveFile);
    Image.SaveAs(SaveFile);
    struct stat Stat = {};
    Passed = EvaluateTrue("SaveAs()", "representative none save", "SaveAs writes the representative galactic image to a file when a canvas exists", stat(SaveFile, &Stat) == 0) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticNoneRedisplayImage", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Image.Display(Image.GetCanvasPointer());
      Passed = EvaluateNear("Display()", "representative none redisplay first bin", "Redisplay on the same canvas keeps the representative unprojected galactic first bin", Hist->GetBinContent(2, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("Display()", "representative none redisplay last bin", "Redisplay on the same canvas keeps the representative unprojected galactic last bin", Hist->GetBinContent(1, 2), 4.0, 1e-12) && Passed;
    }
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageGalactic Image("GalacticNoneCatalogImage", Values,
                            "Lon", 0.0, 360.0, 3,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, "resource/catalogs/BrightMeVSources.scat");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative none catalog", "Display can add named sources from a valid catalog in unprojected mode", Image.IsCreated()) && Passed;
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageGalactic Image("GalacticNoneCatalogWideImage", Values,
                            "Lon", 0.0, 300.0, 3,
                            "Lat", -60.0, 60.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, "resource/catalogs/Crab.scat");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative none wide catalog", "Display can add named sources from a valid catalog in the alternate unprojected axis-division path", Image.IsCreated()) && Passed;
  }

  {
    const char* WrapCatalog = "/tmp/UTImageGalactic_wrap.scat";
    {
      ofstream Out(WrapCatalog);
      Out << "Type scat\n";
      Out << "Version 2\n";
      Out << "PS MO 0 400 1.0 WrapPos\n";
      Out << "PS MO 0 -20 1.0 WrapNeg\n";
      Out << "PS MO 200 400 1.0 OutOfRange\n";
      Out << "EN\n";
    }

    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageGalactic Image("GalacticWrapNoneImage", Values,
                            "Lon", 0.0, 300.0, 3,
                            "Lat", -60.0, 60.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, WrapCatalog);
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative wrap none", "Display can add named sources that require longitude wrapping in unprojected mode", Image.IsCreated()) && Passed;
    unlink(WrapCatalog);
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticExternalCanvas", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    TCanvas Canvas("GalacticExternalCanvas", "GalacticExternalCanvas", 20, 20, 500, 500);
    Image.SetProjection(MImageProjection::c_None);
    Image.Display(&Canvas);
    Passed = EvaluateTrue("Display()", "representative external canvas", "Display can render the representative galactic image on an external canvas", Image.IsCreated()) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative external canvas exists", "Display with an external canvas keeps the representative galactic canvas alive", Image.CanvasExists()) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticExternalCanvasEmptyTitle", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    TCanvas Canvas("GalacticExternalCanvasEmptyTitle", "", 20, 20, 500, 500);
    Image.SetProjection(MImageProjection::c_None);
    Image.Display(&Canvas);
    Passed = EvaluateTrue("Display()", "representative external empty title", "Display creates a representative galactic canvas title when the external canvas title is empty", Image.IsCreated()) && Passed;
  }

  {
    TH2D Hist("GalacticInputHist", "GalacticInputHist", 3, 0.0, 360.0, 2, -90.0, 90.0);
    Hist.GetXaxis()->SetTitle("Input Lon");
    Hist.GetYaxis()->SetTitle("Input Lat");
    Hist.GetZaxis()->SetTitle("Input Counts");
    Hist.SetBinContent(1, 1, 1.0);
    Hist.SetBinContent(2, 1, 2.0);
    Hist.SetBinContent(3, 1, 3.0);
    Hist.SetBinContent(1, 2, 4.0);
    Hist.SetBinContent(2, 2, 5.0);
    Hist.SetBinContent(3, 2, 6.0);

    TestImageGalactic Image;
    Image.SetImageArray(&Hist);
    Passed = Evaluate("SetImageArray(TH2D*)", "representative histogram entry count", "SetImageArray(TH2D*) updates the representative galactic entry count", Image.GetNEntries(), 6) && Passed;
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    TH2* Output = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Output != nullptr) {
      Passed = EvaluateNear("SetImageArray(TH2D*)", "representative histogram first bin", "SetImageArray(TH2D*) copies the representative first input histogram bin", Output->GetBinContent(3, 1), 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray(TH2D*)", "representative histogram last bin", "SetImageArray(TH2D*) copies the representative last input histogram bin", Output->GetBinContent(1, 2), 6.0, 1e-12) && Passed;
    }
  }

  {
    TestImageGalactic Image("GalacticInputHistNull", nullptr,
                            "Lon", 0.0, 360.0, 3,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetImageArray(static_cast<TH2D*>(nullptr));
    Passed = EvaluateFalse("SetImageArray(TH2D*)", "representative histogram nullptr", "SetImageArray(TH2D*) gracefully rejects a null histogram input", Image.IsCreated()) && Passed;
  }

  {
    TH2D Hist("GalacticInputHistRedisplay", "GalacticInputHistRedisplay", 3, 0.0, 360.0, 2, -90.0, 90.0);
    Hist.SetBinContent(1, 1, 1.0);
    Hist.SetBinContent(2, 1, 2.0);
    Hist.SetBinContent(3, 1, 3.0);
    Hist.SetBinContent(1, 2, 4.0);
    Hist.SetBinContent(2, 2, 5.0);
    Hist.SetBinContent(3, 2, 6.0);

    TestImageGalactic Image("GalacticInputHistRedisplayImage",
                            nullptr,
                            "Lon", 0.0, 360.0, 3,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetImageArray(&Hist);
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Hist.SetBinContent(1, 1, 10.0);
    Hist.SetBinContent(2, 1, 20.0);
    Hist.SetBinContent(3, 1, 30.0);
    Hist.SetBinContent(1, 2, 40.0);
    Hist.SetBinContent(2, 2, 50.0);
    Hist.SetBinContent(3, 2, 60.0);
    Image.SetImageArray(&Hist);
    TH2* Output = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Output != nullptr) {
      Passed = EvaluateNear("SetImageArray(TH2D*)", "representative redisplay first bin", "SetImageArray(TH2D*) updates the representative galactic histogram after display", Output->GetBinContent(3, 1), 10.0, 1e-12) && Passed;
      Passed = EvaluateNear("SetImageArray(TH2D*)", "representative redisplay last bin", "SetImageArray(TH2D*) updates the representative galactic histogram after display", Output->GetBinContent(1, 2), 60.0, 1e-12) && Passed;
    }
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerColCont0Image", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONT, "resource/catalogs/Crab.scat");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative hammer COLCONT0", "Display creates a representative Hammer galactic image with the COLCONT0 draw option", Image.IsCreated()) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerColCont0ZImage", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, "resource/catalogs/Crab.scat");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative hammer COLCONT0Z", "Display creates a representative Hammer galactic image with the COLCONT0Z draw option", Image.IsCreated()) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerBirdImage", Values,
                            "Lon", 0.0, 300.0, 2,
                            "Lat", -10.0, 70.0, 2,
                            "Counts", MImage::c_Bird, MImage::c_COLCONTZ, "resource/catalogs/Crab.scat");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative hammer bird", "Display creates a representative Hammer galactic image with the alternate text-color spectrum branch", Image.IsCreated()) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerNegativeLongitudeImage", Values,
                            "Lon", -180.0, 180.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Bird, MImage::c_COLCONTZ, "resource/catalogs/BrightMeVSources.scat");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative hammer negative longitude", "Display can render a Hammer image that exercises the negative-longitude label normalization path", Image.IsCreated()) && Passed;
  }

  {
    const char* WrapCatalog = "/tmp/UTImageGalactic_wrap_hammer.scat";
    {
      ofstream Out(WrapCatalog);
      Out << "Type scat\n";
      Out << "Version 2\n";
      Out << "PS MO 0 400 1.0 WrapPos\n";
      Out << "PS MO 0 -20 1.0 WrapNeg\n";
      Out << "PS MO 200 400 1.0 OutOfRange\n";
      Out << "EN\n";
    }

    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticWrapHammerImage", Values,
                            "Lon", 0.0, 300.0, 2,
                            "Lat", -10.0, 70.0, 2,
                            "Counts", MImage::c_Bird, MImage::c_COLCONTZ, WrapCatalog);
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative wrap hammer", "Display can add named sources that require longitude wrapping in Hammer mode", Image.IsCreated()) && Passed;
    unlink(WrapCatalog);
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerImage", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetProjection(MImageProjection::c_Hammer);
    Image.Display();

    Passed = EvaluateTrue("Display()", "representative hammer histogram", "Display creates a representative Hammer-projected galactic histogram", Image.GetHistogramPointer() != nullptr) && Passed;
    Passed = EvaluateTrue("CanvasExists()", "representative hammer canvas", "Display creates a representative Hammer-projected galactic canvas", Image.CanvasExists()) && Passed;
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateTrue("Display()", "representative hammer binning", "The representative Hammer histogram has a finer grid than the source image", Hist->GetNbinsX() > Image.GetNEntries()) && Passed;
      Passed = EvaluateTrue("Display()", "representative hammer content", "The representative Hammer histogram has projected content after display", CountNonZeroBins(Hist) > 0) && Passed;
    }

    double UpdatedValues[4] = {0.0, 0.0, 0.0, 10.0};
    Image.SetImageArray(UpdatedValues);
    if (Hist != nullptr) {
      Passed = EvaluateTrue("SetImageArray()", "representative hammer update", "SetImageArray(newData) updates the representative Hammer-projected galactic histogram", Hist->GetMaximum() > 0.0) && Passed;
      Passed = EvaluateTrue("SetImageArray()", "representative hammer update content", "SetImageArray(newData) leaves projected Hammer content in the representative galactic histogram", CountNonZeroBins(Hist) > 0) && Passed;
    }

    Image.Display(Image.GetCanvasPointer());
    if (Hist != nullptr) {
      Passed = EvaluateTrue("Display()", "representative hammer redisplay", "A representative Hammer galactic image can be redrawn on the same canvas after data updates", Image.IsCreated()) && Passed;
    }

    double InvalidValues[4] = {numeric_limits<double>::quiet_NaN(),
                               numeric_limits<double>::infinity(),
                               numeric_limits<double>::quiet_NaN(),
                               numeric_limits<double>::infinity()};
    Image.SetImageArray(InvalidValues);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative hammer non-finite clears bins", "SetImageArray() clears all projected Hammer bins when the representative galactic data is non-finite", CountNonZeroBins(Hist), 0) && Passed;
    }

    Image.SetImageArray((double*) nullptr);
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative hammer nullptr clears bins", "SetImageArray(nullptr) zeros the representative galactic histogram in Hammer mode", CountNonZeroBins(Hist), 0) && Passed;
    }
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticHammerUnknownProjection", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, "resource/catalogs/Crab.scat");
    Image.SetProjection(static_cast<MImageProjection>(123));
    Image.RunAddNamedSources();
  }

  {
    const char* EmptyCatalog = "/tmp/UTImageGalactic_empty.scat";
    {
      ofstream Out(EmptyCatalog);
      Out << "Type scat\n";
      Out << "Version 2\n";
      Out << "EN\n";
    }

    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticEmptyCatalog", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, EmptyCatalog);
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative empty catalog", "Display tolerates an empty source catalog without crashing", Image.IsCreated()) && Passed;
    unlink(EmptyCatalog);
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticMissingCatalog", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts", MImage::c_Viridis, MImage::c_COLCONTZ, "resource/catalogs/does_not_exist.scat");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("Display()", "representative missing catalog", "Display tolerates a missing source catalog without crashing", Image.IsCreated()) && Passed;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticInvalidGeometry", Values,
                            "Lon", 0.0, 0.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateFalse("Display()", "representative invalid geometry", "Display returns early and does not mark the representative galactic image as created when the geometry is invalid", Image.IsCreated()) && Passed;
  }

  {
    vector<double> Data;
    Data.push_back(1.0);
    Data.push_back(2.0);
    Data.push_back(3.0);
    Data.push_back(4.0);
    Data.push_back(5.0);
    Data.push_back(6.0);

    TestImageGalactic Image("GalacticFISBELImage", nullptr,
                            "Lon", 0.0, 360.0, 1,
                            "Lat", -90.0, 90.0, 1,
                            "Counts");
    Image.SetFISBEL(Data, 0.0);
    Passed = Evaluate("SetFISBEL()", "representative FISBEL entry count", "SetFISBEL updates the representative galactic entry count", Image.GetNEntries(), 12) && Passed;
    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    Passed = EvaluateTrue("SetFISBEL()", "representative FISBEL created", "SetFISBEL can be displayed in the representative galactic image", Image.IsCreated()) && Passed;
    Image.SetImageArray((double*) nullptr);
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = Evaluate("SetImageArray()", "representative FISBEL nullptr clears bins", "SetImageArray(nullptr) zeros the representative galactic histogram after SetFISBEL", CountNonZeroBins(Hist), 0) && Passed;
    }
  }

  {
    vector<double> EmptyData;
    TestImageGalactic Image("GalacticFISBELEmpty",
                            nullptr,
                            "Lon", 0.0, 360.0, 1,
                            "Lat", -90.0, 90.0, 1,
                            "Counts");
    Image.SetFISBEL(EmptyData, 0.0);
    Passed = EvaluateFalse("SetFISBEL()", "representative empty FISBEL", "SetFISBEL leaves the representative galactic image undisplayed when the input data is empty", Image.IsCreated()) && Passed;
  }

  {
    double Values[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    TestImageGalactic Image("GalacticCloneImage", Values,
                            "Lon", 0.0, 360.0, 3,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    Image.SetProjection(MImageProjection::c_None);
    Image.Normalize(true);

    MImage* CloneBase = Image.Clone();
    TestImageGalactic* Clone = dynamic_cast<TestImageGalactic*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative clone type", "Clone returns a representative object of the galactic image type", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative cloned projection", "Clone preserves the representative galactic projection mode", static_cast<int>(Clone->GetProjection()), static_cast<int>(MImageProjection::c_None)) && Passed;
      Clone->Display();
      TH2* CloneHist = dynamic_cast<TH2*>(Clone->GetHistogramPointer());
      if (CloneHist != nullptr) {
        Passed = EvaluateNear("Clone()", "representative cloned normalized maximum", "Clone preserves the representative galactic normalization setting", CloneHist->GetMaximum(), 1.0, 1e-12) && Passed;
      }
    }
    delete CloneBase;
  }

  {
    double Values[4] = {2.0, 4.0, 6.0, 8.0};
    TestImageGalactic Original("GalacticHammerCloneImage", Values,
                               "Lon", 0.0, 360.0, 2,
                               "Lat", -90.0, 90.0, 2,
                               "Counts");
    Original.SetProjection(MImageProjection::c_Hammer);
    Original.Normalize(true);

    MImage* CloneBase = Original.Clone();
    TestImageGalactic* Clone = dynamic_cast<TestImageGalactic*>(CloneBase);
    Passed = EvaluateTrue("Clone()", "representative hammer clone type", "Clone returns a representative object of the galactic image type in Hammer mode", Clone != nullptr) && Passed;
    if (Clone != nullptr) {
      Passed = Evaluate("Clone()", "representative hammer cloned projection", "Clone preserves the representative galactic Hammer projection mode", static_cast<int>(Clone->GetProjection()), static_cast<int>(MImageProjection::c_Hammer)) && Passed;
      Clone->Display();
      TH2* CloneHist = dynamic_cast<TH2*>(Clone->GetHistogramPointer());
      if (CloneHist != nullptr) {
        Passed = EvaluateNear("Clone()", "representative hammer normalized maximum", "Clone preserves the representative galactic Hammer normalization setting", CloneHist->GetMaximum(), 1.0, 1e-12) && Passed;
      }
    }
    delete CloneBase;
  }

  {
    double Values[4] = {1.0, 2.0, 3.0, 4.0};
    TestImageGalactic Image("GalacticDetermineReset", Values,
                            "Lon", 0.0, 360.0, 2,
                            "Lat", -90.0, 90.0, 2,
                            "Counts");
    double MaxValue = -1.0;
    vector<double> Coordinate;
    Image.DetermineMaximum(MaxValue, Coordinate);
    Passed = EvaluateNear("DetermineMaximum()", "representative maximum", "DetermineMaximum finds the representative maximum in a galactic image", MaxValue, 4.0, 1e-12) && Passed;
    Passed = Evaluate("DetermineMaximum()", "representative coordinate count", "DetermineMaximum returns two coordinates for the representative galactic image", static_cast<int>(Coordinate.size()), 2) && Passed;

    Image.SetProjection(MImageProjection::c_None);
    Image.Display();
    TH2* Hist = dynamic_cast<TH2*>(Image.GetHistogramPointer());
    if (Hist != nullptr) {
      Passed = EvaluateTrue("Reset()", "representative displayed reset", "Reset clears the representative galactic histogram after display", Hist->GetMaximum() > 0.0) && Passed;
      Image.Reset();
      Passed = Evaluate("Reset()", "representative reset clears bins", "Reset zeroes the representative galactic histogram after display", CountNonZeroBins(Hist), 0) && Passed;
    }

    TestImageGalactic NullImage("GalacticNullDisplay",
                               Values,
                               "Lon", 0.0, 360.0, 2,
                               "Lat", -90.0, 90.0, 2,
                               "Counts");
    NullImage.InvalidateArray();
    NullImage.Display();
    Passed = EvaluateFalse("Display()", "representative null backing data", "Display returns early and does not mark the representative galactic image as created when the backing data is missing", NullImage.IsCreated()) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTImageGalactic Test;
  return Test.Run() == true ? 0 : 1;
}
