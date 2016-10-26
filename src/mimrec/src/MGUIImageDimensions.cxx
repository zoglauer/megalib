/*
 * MGUIImageDimensions.cxx
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
// MGUIImageDimensions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIImageDimensions.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MImage.h"
#include "MCoordinateSystem.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIImageDimensions)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIImageDimensions::MGUIImageDimensions(const TGWindow* Parent, 
                                         const TGWindow* Main, 
                                         MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIImageDimensions::~MGUIImageDimensions()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageDimensions::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Image Dimensions");  


  TGLayoutHints* AxisLayoutFirst = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
  TGLayoutHints* AxisLayoutLast = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 20);

  TGLayoutHints* DimensionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
  TGLayoutHints* BinLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 20);


  if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    AddSubTitle("Please enter dimensions and the number of bins\nfor images in spherical coordinates"); 

    m_XAxis = new MGUIEEntryList(this, "New X-axis vector:", MGUIEEntryList::c_SingleLine);
    m_XAxis->Add("", m_GUIData->GetImageRotationXAxis().X(), true);
    m_XAxis->Add("", m_GUIData->GetImageRotationXAxis().Y(), true);
    m_XAxis->Add("", m_GUIData->GetImageRotationXAxis().Z(), true);
    m_XAxis->Create();
    AddFrame(m_XAxis, AxisLayoutFirst);

    m_ZAxis = new MGUIEEntryList(this, "New Z-axis vector:", MGUIEEntryList::c_SingleLine);
    m_ZAxis->Add("", m_GUIData->GetImageRotationZAxis().X(), true);
    m_ZAxis->Add("", m_GUIData->GetImageRotationZAxis().Y(), true);
    m_ZAxis->Add("", m_GUIData->GetImageRotationZAxis().Z(), true);
    m_ZAxis->Create();
    AddFrame(m_ZAxis, AxisLayoutLast);

    m_ThetaDimension = new MGUIEMinMaxEntry(this,
                                            MString("Theta:"),
                                            false,
                                            MString("Minimum [deg]: "),
                                            MString("Maximum [deg]: "),
                                            m_GUIData->GetThetaMin(), 
                                            m_GUIData->GetThetaMax(),
                                            true,
                                            0.0,
                                            180.0);
    AddFrame(m_ThetaDimension, DimensionLayout);
  
    m_ThetaBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsTheta(), true, 1);
    AddFrame(m_ThetaBins, BinLayout);

    m_PhiDimension = new MGUIEMinMaxEntry(this,
                                          MString("Phi:"),
                                          false,
                                          MString("Minimum [deg]: "),
                                          MString("Maximum [deg]: "),
                                          m_GUIData->GetPhiMin(), 
                                          m_GUIData->GetPhiMax(),
                                          true);
    AddFrame(m_PhiDimension, DimensionLayout);
    
    m_PhiBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsPhi(), true, 1);
    AddFrame(m_PhiBins, BinLayout);
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    AddSubTitle("Please enter dimensions and the number of bins\nfor images in galactic coordinates"); 

    m_LatitudeDimension = new MGUIEMinMaxEntry(this,
                                            MString("Galactic latitude:"),
                                            false,
                                            MString("Minimum [deg]: "),
                                            MString("Maximum [deg]: "),
                                            m_GUIData->GetGalLatitudeMin(), 
                                            m_GUIData->GetGalLatitudeMax(),
                                            true,
                                            -90.0,
                                            90.0);
    AddFrame(m_LatitudeDimension, DimensionLayout);
  
    m_LatitudeBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsGalLatitude(), true, 1);
    AddFrame(m_LatitudeBins, BinLayout);

    m_LongitudeDimension = new MGUIEMinMaxEntry(this,
                                          MString("Galactic longitude:"),
                                          false,
                                          MString("Minimum [deg]: "),
                                          MString("Maximum [deg]: "),
                                          m_GUIData->GetGalLongitudeMin(), 
                                          m_GUIData->GetGalLongitudeMax(),
                                          true);
    AddFrame(m_LongitudeDimension, DimensionLayout);
    
    m_LongitudeBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsGalLongitude(), true, 1);
    AddFrame(m_LongitudeBins, BinLayout);
    
    TGHorizontalFrame* ProjectionFrame = new TGHorizontalFrame(this);
    TGLayoutHints* ProjectionFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 0, 0, 20, 20);
    AddFrame(ProjectionFrame, ProjectionFrameLayout);
    
    TGLabel* ProjectionLabel = new TGLabel(ProjectionFrame, "Please choose a projection:");
    TGLayoutHints* ProjectionLabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 0, 0);
    ProjectionFrame->AddFrame(ProjectionLabel, ProjectionLabelLayout);

    m_Projection = new TGComboBox(ProjectionFrame);
    m_Projection->AddEntry("No Projection", static_cast<int>(MImageProjection::c_None));
    m_Projection->AddEntry("Hammer projection", static_cast<int>(MImageProjection::c_Hammer));
    m_Projection->Associate(this);
    m_Projection->Select(static_cast<int>(m_GUIData->GetImageProjection()));
    m_Projection->SetHeight(m_FontScaler*18);
    m_Projection->SetWidth(m_FontScaler*120);
    TGLayoutHints* ProjectionLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 20, 20, 0, 0);
    ProjectionFrame->AddFrame(m_Projection, ProjectionLayout);
    
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {
    AddSubTitle("Please enter dimensions and the number of bins\nfor images in Cartesian coordinates"); 
    m_XDimension = new MGUIEMinMaxEntry(this,
                                        MString("x-axis:"),
                                        false,
                                        MString("Minimum [cm]: "),
                                        MString("Maximum [cm]: "),
                                        m_GUIData->GetXMin(), 
                                        m_GUIData->GetXMax());
    AddFrame(m_XDimension, DimensionLayout);
    
    m_XBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsX(), true, 1);
    AddFrame(m_XBins, BinLayout);

    m_YDimension = new MGUIEMinMaxEntry(this,
                                        MString("y-axis:"),
                                        false,
                                        MString("Minimum [cm]: "),
                                        MString("Maximum [cm]: "),
                                        m_GUIData->GetYMin(), 
                                        m_GUIData->GetYMax());
    AddFrame(m_YDimension, DimensionLayout);

    m_YBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsY(), true, 1);
    AddFrame(m_YBins, BinLayout);
    
 
    m_ZDimension = new MGUIEMinMaxEntry(this,
                                        MString("z-axis:"),
                                        false,
                                        MString("Minimum [cm]: "),
                                        MString("Maximum [cm]: "),
                                        m_GUIData->GetZMin(), 
                                        m_GUIData->GetZMax());
    AddFrame(m_ZDimension, DimensionLayout);
    
    m_ZBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsZ(), true, 1);
    AddFrame(m_ZBins, BinLayout);
  } else {
    merr<<"Unknown coordinate system ID: "<<m_GUIData->GetCoordinateSystem()<<fatal;
  }

  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIImageDimensions::OnApply()
{
  // The Apply button has been pressed

  if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {

    if (m_ThetaDimension->CheckRange(0.0, 180.0, 0.0, 180.0, true) == false) return false;
    if (m_PhiDimension->CheckRange(-360.0, 360.0, -360.0, 360.0, true) == false) return false;
    
    if (m_PhiDimension->GetMaxValue() - m_PhiDimension->GetMinValue() > 360) {
      mgui<<"The phi axis is not allowed to show more than 360 degrees."<<error;
      return false;
    }
    
    if (m_ThetaBins->IsInt(0, 1000000) == false) return false;
    if (m_PhiBins->IsInt(0, 1000000) == false) return false;

    if (m_ThetaBins->GetAsInt() == 0) {
      mgui<<"The theta axis must have at least one bin."<<error;
      return false;
    }
    if (m_PhiBins->GetAsInt() == 0) {
      mgui<<"The phi axis must have at least one bin."<<error;
      return false;
    }
    
    if (m_XAxis->IsModified() == true) {
      MVector XAxis(m_XAxis->GetAsDouble(0), m_XAxis->GetAsDouble(1), m_XAxis->GetAsDouble(2));
      XAxis.Unitize();
      m_GUIData->SetImageRotationXAxis(XAxis);
    }

    if (m_ZAxis->IsModified() == true) {
      MVector ZAxis(m_ZAxis->GetAsDouble(0), m_ZAxis->GetAsDouble(1), m_ZAxis->GetAsDouble(2));
      ZAxis.Unitize();
      m_GUIData->SetImageRotationZAxis(ZAxis);
    }

    if (m_ThetaDimension->IsModified() == true) {
      m_GUIData->SetThetaMin(m_ThetaDimension->GetMinValue());
      m_GUIData->SetThetaMax(m_ThetaDimension->GetMaxValue());
    }

    if (m_PhiDimension->IsModified() == true) {
      m_GUIData->SetPhiMin(m_PhiDimension->GetMinValue());
      m_GUIData->SetPhiMax(m_PhiDimension->GetMaxValue());
    }

    if (m_ThetaBins->IsModified() == true) {
      m_GUIData->SetBinsTheta(m_ThetaBins->GetAsInt());
    }

    if (m_PhiBins->IsModified() == true) {
      m_GUIData->SetBinsPhi(m_PhiBins->GetAsInt());
    }

  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {

    if (m_LatitudeDimension->CheckRange(-90.0, 90.0, -90.0, 90.0, true) == false) return false;
    //if (m_LongitudeDimension->CheckRange(0.0, 360.0, 0.0, 360.0, true) == false) return false;
    
    if (m_LongitudeDimension->GetMaxValue() - m_LongitudeDimension->GetMinValue() > 360) {
      mgui<<"The galactic longitude axis is not allowed to show more than 360 degrees."<<error;
      return false;
    }
    
    if (m_LatitudeBins->IsInt(0, 1000000) == false) return false;
    if (m_LongitudeBins->IsInt(0, 1000000) == false) return false;
    
    if (m_LatitudeBins->GetAsInt() == 0) {
      mgui<<"The latitude axis must have at least one bin."<<error;
      return false;
    }
    if (m_LongitudeBins->GetAsInt() == 0) {
      mgui<<"The longitude axis must have at least one bin."<<error;
      return false;
    }

    if (m_LatitudeDimension->IsModified() == true) {
      m_GUIData->SetGalLatitudeMin(m_LatitudeDimension->GetMinValue());
      m_GUIData->SetGalLatitudeMax(m_LatitudeDimension->GetMaxValue());
    }

    if (m_LongitudeDimension->IsModified() == true) {
      m_GUIData->SetGalLongitudeMin(m_LongitudeDimension->GetMinValue());
      m_GUIData->SetGalLongitudeMax(m_LongitudeDimension->GetMaxValue());
    }

    if (m_LatitudeBins->IsModified() == true) {
      m_GUIData->SetBinsGalLatitude(m_LatitudeBins->GetAsInt());
    }

    if (m_LongitudeBins->IsModified() == true) {
      m_GUIData->SetBinsGalLongitude(m_LongitudeBins->GetAsInt());
    }
    
    if (m_Projection->GetSelected() != static_cast<int>(m_GUIData->GetImageProjection())) {
      m_GUIData->SetImageProjection(static_cast<MImageProjection>(m_Projection->GetSelected()));
    }
    
  } else if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D) {

    if (m_XDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
    if (m_YDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
    if (m_ZDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
    
    if (m_XBins->IsInt(0, 10000) == false) return false;
    if (m_YBins->IsInt(0, 10000) == false) return false;
    if (m_ZBins->IsInt(0, 10000) == false) return false;
    
    if (m_XBins->GetAsInt() == 0) {
      mgui<<"The x axis must have at least one bin."<<error;
      return false;
    }
    if (m_YBins->GetAsInt() == 0) {
      mgui<<"The y axis must have at least one bin."<<error;
      return false;
    }
    if (m_ZBins->GetAsInt() == 0) {
      mgui<<"The z axis must have at least one bin."<<error;
      return false;
    }

    if (m_GUIData->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D) {
      if (m_XBins->GetAsInt() != 1 && m_YBins->GetAsInt() != 1 && m_ZBins->GetAsInt() != 1) {
        int Return = 0;
        MString Text = "In 2D Cartesian coordinates at least one dimension must have a bin size of 1!\n";
        Text += "Do you want to switch to 3D Cartesian coordinates?";
        new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), 
                     "Warning", Text, kMBIconExclamation, kMBYes | kMBNo, &Return);
        if (Return == kMBYes) {
          m_GUIData->SetCoordinateSystem(MCoordinateSystem::c_Cartesian3D);
        } else {
          return false;
        }
      }
    }

    if (m_XDimension->IsModified() == true) {
      m_GUIData->SetXMin(m_XDimension->GetMinValue());
      m_GUIData->SetXMax(m_XDimension->GetMaxValue());
    }
    if (m_YDimension->IsModified() == true) {
      m_GUIData->SetYMin(m_YDimension->GetMinValue());
      m_GUIData->SetYMax(m_YDimension->GetMaxValue());
    }
    if (m_ZDimension->IsModified() == true) {
      m_GUIData->SetZMin(m_ZDimension->GetMinValue());
      m_GUIData->SetZMax(m_ZDimension->GetMaxValue());
    }

    if (m_XBins->IsModified() == true) {
      m_GUIData->SetBinsX(m_XBins->GetAsInt());
    }
    if (m_YBins->IsModified() == true) {
      m_GUIData->SetBinsY(m_YBins->GetAsInt());
    }
    if (m_ZBins->IsModified() == true) {
      m_GUIData->SetBinsZ(m_ZBins->GetAsInt());
    }
  }

  

  return true;
}


// MGUIImageDimensions: the end...
////////////////////////////////////////////////////////////////////////////////
