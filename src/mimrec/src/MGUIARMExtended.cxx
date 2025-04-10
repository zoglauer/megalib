/*
 * MGUIARMExtended.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUIARMExtended
//
//
// Dialog box, which provides entry-boxes for ARM-parameter (theta, phi, radius)
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIARMExtended.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MARMFitter.h"
#include "MGUIEText.h"
#include "MCoordinateSystem.h"
#include "MGUIEEntry.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIARMExtended)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIARMExtended::MGUIARMExtended(const TGWindow* p, const TGWindow* main, MSettingsMimrec* Data, bool& OkPressed)
  : MGUIDialog(p, main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_Settings = Data;
  m_OkPressed = false;

  m_ThetaIsX = nullptr;
  m_PhiIsY = nullptr;
  m_RadiusIsZ = nullptr;
  m_Distance = nullptr;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIARMExtended::~MGUIARMExtended()
{
  // standard destructor --- deep clean up activated!
}


////////////////////////////////////////////////////////////////////////////////


void MGUIARMExtended::Create()
{
  // Create the main window

  double Scaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  double WrapLength = 600*Scaler;

  SetWindowName("Angular Resolution Measure");
  AddSubTitle("Choose the source location, histogramming, and fitting options for creating the\nAngular Resolution Measure (ARM) for Compton events");

  TGLayoutHints* SectionFirstLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 50*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);
  TGLayoutHints* SectionTopLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 50*Scaler, 50*Scaler, 30*Scaler, 2*Scaler);
  TGLayoutHints* SectionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 50*Scaler, 50*Scaler, 2*Scaler, 2*Scaler);

  TGLayoutHints* LeftLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 10*Scaler, 0, 0);
  TGLayoutHints* RightLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 10*Scaler, 0, 0, 0);

  MString TestPositionIntro;
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    TestPositionIntro += "Source location in spheric coordinates:\n";
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    TestPositionIntro += "Source location in Galactic coordinates:\n";
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    TestPositionIntro += "Source location in Cartesian coordinates:\n";
  }
  TestPositionIntro += "Change the coordiante system in the menu Selections -> Coordinate System:";
  TGLabel* TestPositionLabel = new TGLabel(this, TestPositionIntro);
  TestPositionLabel->SetWrapLength(WrapLength);
  AddFrame(TestPositionLabel, SectionFirstLayout);

  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {

    m_ThetaIsX = new MGUIEEntry(this, "Theta (0 - 180 deg):", false, m_Settings->GetTPTheta());
    AddFrame(m_ThetaIsX, SectionLayout);
    m_PhiIsY = new MGUIEEntry(this, "Phi (-180 - 360 deg):", false, m_Settings->GetTPPhi());
    AddFrame(m_PhiIsY, SectionLayout);
    
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    
    m_ThetaIsX = new MGUIEEntry(this, "Latitude (-90 - 90 deg):", false, m_Settings->GetTPGalLatitude());
    AddFrame(m_ThetaIsX, SectionLayout);
    m_PhiIsY = new MGUIEEntry(this, "Longitude (0 - 360 deg):", false, m_Settings->GetTPGalLongitude());
    AddFrame(m_PhiIsY, SectionLayout);

  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {

    m_ThetaIsX = new MGUIEEntry(this, "x [cm]:", false, m_Settings->GetTPX());
    AddFrame(m_ThetaIsX, SectionLayout);
    m_PhiIsY = new MGUIEEntry(this, "y [cm]:", false, m_Settings->GetTPY());
    AddFrame(m_PhiIsY, SectionLayout);
    m_RadiusIsZ = new MGUIEEntry(this, "z [cm]:", false, m_Settings->GetTPZ());
    AddFrame(m_RadiusIsZ, SectionLayout);

  }


  // Fitting section:
  MString HistogramIntro;
  HistogramIntro += "Histogram options:";
  TGLabel* HistogramIntroLabel = new TGLabel(this, HistogramIntro);
  HistogramIntroLabel->SetWrapLength(WrapLength);
  AddFrame(HistogramIntroLabel, SectionTopLayout);

  m_Distance = new MGUIEEntry(this, "Maximum ARM value in histogram (0 - 180 deg):", false, m_Settings->GetTPDistanceTrans(), true, 0.0, 180.0);
  AddFrame(m_Distance, SectionLayout);

  // All histogram types have the number of bins
  m_Bins = new MGUIEEntry(this, "Number of bins in histogram (odd numbers preferred):", false, m_Settings->GetHistBinsARMGamma(), true, 1);
  AddFrame(m_Bins, SectionLayout);

  m_OptimizedBinning = new TGCheckButton(this, "Optimize the binning, i.e., make bins larger until there are at least 50 counts in one bin if possible", m_OptimizedBinningID);
  m_OptimizedBinning->Associate(this);
  m_OptimizedBinning->SetState(m_Settings->GetOptimizeBinningARMGamma() ? kButtonDown : kButtonUp);
  AddFrame(m_OptimizedBinning, SectionLayout);




  // Fitting section:
  MString FittingIntro;
  FittingIntro += "Fitting options:\n";
  FittingIntro += "Zero fits means only the ARM is created, one fit does exactly that, multiple fits calculates also the uncertainty of the FWHM from bootstrapping the data and re-fitting. Concerning the fit function, start with asymmetric Gauss + Lorentz + Lorentz.";
  TGLabel* FittingIntroLabel = new TGLabel(this, FittingIntro);
  FittingIntroLabel->SetWrapLength(WrapLength);
  //FittingIntroLabel->ChangeOptions(FittingIntroLabel->GetOptions() | kRaisedFrame);
  AddFrame(FittingIntroLabel, SectionTopLayout);

  m_NumberOfFits = new MGUIEEntry(this, "Number of fits:", false, (int) m_Settings->GetNumberOfFitsARMGamma(), true, 0);
  AddFrame(m_NumberOfFits, SectionLayout);

  TGHorizontalFrame* FitFunctionFrame = new TGHorizontalFrame(this);
  AddFrame(FitFunctionFrame, SectionLayout);

  TGLabel* FitFunctionLabel = new TGLabel(FitFunctionFrame, "Fit function:");
  FitFunctionFrame->AddFrame(FitFunctionLabel, LeftLayout);

  MARMFitter Fitter;
  vector<MARMFitFunctionID> FitFunctions = Fitter.GetListOfFitFunctions();

  m_FitFunctions = new TGComboBox(FitFunctionFrame);
  for (unsigned int f = 0; f < FitFunctions.size(); ++f) {
    m_FitFunctions->AddEntry(Fitter.GetARMFitFunctionName(FitFunctions[f]), f);
  }
  m_FitFunctions->Select(m_Settings->GetFitFunctionIDARMGamma());
  m_FitFunctions->Associate(this);
  m_FitFunctions->SetHeight(Scaler*18);
  m_FitFunctions->SetWidth(Scaler*475);
  FitFunctionFrame->AddFrame(m_FitFunctions, RightLayout);

  m_UseUnbinnedFitting = new TGCheckButton(this, "Use unbinned fitting. This works better for a lower number of counts, but is slower for a large number of counts.", m_UseUnbinnedFittingID);
  m_UseUnbinnedFitting->SetWrapLength(WrapLength);
  m_UseUnbinnedFitting->Associate(this);
  m_UseUnbinnedFitting->SetState(m_Settings->GetUseUnbinnedFittingARMGamma() ? kButtonDown : kButtonUp);
  AddFrame(m_UseUnbinnedFitting, SectionLayout);



  MString ARMComment;
  ARMComment += "Some comments on the ARM:\n";
  ARMComment += "1. The ARM is the angular resolution of the detector, not of point sources in the final image. The angular resolution achieved in images depends on the ARM, the number of counts, and the imaging algorithm. It is also not a measure for the localization accuracy of a point source. This is a function of the number of counts in the core of the ARM. The ARM is also just a proxy for how good two close by point sources can be resolved. That depends on the shape of the ARM, the imaging algorithm, and the number of counts.\n";
  ARMComment += "2. The ARM depends on the used event selections. Therefore, always quote the event selections when quoting an ARM. A typical event selection is to use a +-3 sigma cut on the (calibration) line you are using and then quoting the effective area calculated by using all events falling withing [-FWHM, +FWHM] of the ARM.\n";
  ARMComment += "3. If you are fitting the ARM, the FWHM uncertainty is only reliable if the fit is OK. If the fit is not good, you will have an additional systematic uncertainty which is not included in the FWHM uncertainty. For a good fit, try to use an ARM window of 2-4 FWHM'es, and try to start with the asymmetric-gaussian + Lorentz + Lorentz fit function. If the fit still fails, make the ARM window smaller.";

  TGLabel* ARMCommentsLabel = new TGLabel(this, ARMComment);
  ARMCommentsLabel->SetWrapLength(WrapLength);
  //ARMCommentsLabel->ChangeOptions(ARMCommentsLabel->GetOptions() | kRaisedFrame);
  AddFrame(ARMCommentsLabel, SectionTopLayout);


  AddButtons();
  
  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();

  fClient->WaitFor(this);
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIARMExtended::ProcessMessage(long Message, long Parameter1,
                                        long Parameter2)
{
  // Process the messages for this application
  
  return MGUIDialog::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIARMExtended::OnApply()
{
  // First test the data (m_RadiusIsZ has not to be checked!)
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    if (m_ThetaIsX->IsDouble(0, 180) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    if (m_ThetaIsX->IsDouble(-90, 90) == false || 
        m_PhiIsY->IsDouble(-360, 360) == false || 
        m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    if (m_Distance->IsDouble(-180, 180) == false) {
      return false;
    }
  }
  
  // Otherwise update the data:
  if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    m_Settings->SetTPTheta(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPPhi(m_PhiIsY->GetAsDouble());
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    m_Settings->SetTPGalLatitude(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPGalLongitude(m_PhiIsY->GetAsDouble());
  } else if (m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    m_Settings->SetTPX(m_ThetaIsX->GetAsDouble());
    m_Settings->SetTPY(m_PhiIsY->GetAsDouble());
    m_Settings->SetTPZ(m_RadiusIsZ->GetAsDouble());
  }

  m_Settings->SetTPDistanceTrans(m_Distance->GetAsDouble());
  m_Settings->SetHistBinsARMGamma(m_Bins->GetAsInt());
  
  m_Settings->SetOptimizeBinningARMGamma(m_OptimizedBinning->GetState() == kButtonDown ? true : false);
  m_Settings->SetNumberOfFitsARMGamma(m_NumberOfFits->GetAsInt());
  m_Settings->SetFitFunctionIDARMGamma(m_FitFunctions->GetSelected());
  m_Settings->SetUseUnbinnedFittingARMGamma(m_UseUnbinnedFitting->GetState() == kButtonDown ? true : false);

  m_OkPressed = true;
  
  return true;
}


// MGUIARMExtended: the end...
////////////////////////////////////////////////////////////////////////////////
