/*
 * MGUIImageDimensionsSpherical.cxx
 *
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
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
// MGUIImageDimensionsSpherical
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIImageDimensionsSpherical.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSystem.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIImageDimensionsSpherical)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIImageDimensionsSpherical::MGUIImageDimensionsSpherical(const TGWindow* Parent, 
                                                           const TGWindow* Main, 
                                                           MGUIDataMimrec* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIImageDimensionsSpherical::~MGUIImageDimensionsSpherical()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageDimensionsSpherical::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Image Dimensions");  

  AddSubTitle("Please enter the image-axis vector, dimensions \n and the number of bins for spherical images"); 

  m_AxisLayoutFirst = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
  m_AxisLayoutLast = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 20);

  m_DimensionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
  m_BinLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 20);

  m_ThetaImageAxis = new MGUIEEntry(this, "Theta of image-axis vector:", false, m_GUIData->GetAxisTheta(), true);
  m_ThetaImageAxis->SetEnabled(false);
  AddFrame(m_ThetaImageAxis, m_AxisLayoutFirst);

  m_PhiImageAxis = new MGUIEEntry(this, "Phi of image-axis vector:", false, m_GUIData->GetAxisPhi(), true);
  m_PhiImageAxis->SetEnabled(false);
  AddFrame(m_PhiImageAxis, m_AxisLayoutLast);

  m_ThetaDimension = new MGUIEMinMaxEntry(this,
                                          TString("Galactic latitude:"),
                                          false,
                                          TString("Minimum [°]: "),
                                          TString("Maximum [°]: "),
                                          m_GUIData->GetThetaMin(), 
                                          m_GUIData->GetThetaMax(),
                                          true,
                                          -90.0,
                                          90.0);
  AddFrame(m_ThetaDimension, m_DimensionLayout);

  m_ThetaBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsTheta(), true, 1);
  AddFrame(m_ThetaBins, m_BinLayout);

  m_PhiDimension = new MGUIEMinMaxEntry(this,
                                        TString("Galactic longitude:"),
                                        false,
                                        TString("Minimum [°]: "),
                                        TString("Maximum [°]: "),
                                        m_GUIData->GetPhiMin(), 
                                        m_GUIData->GetPhiMax(),
                                        true);
  AddFrame(m_PhiDimension, m_DimensionLayout);

  m_PhiBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsPhi(), true, 1);
  AddFrame(m_PhiBins, m_BinLayout);


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIImageDimensionsSpherical::OnApply()
{
	// The Apply button has been pressed

  if (m_ThetaImageAxis->IsDouble(-90.0, 90.0) == false) return false;
  // if (m_PhiImageAxis->IsDouble(0.0, 360.0) == false) return false;
  
  if (m_ThetaDimension->CheckRange(-90.0, 90.0, -90.0, 90.0, true) == false) return false;
  // if (m_PhiDimension->CheckRange(0.0, 360.0, 0.0, 360.0, true) == false) return false;
  
  if (m_PhiDimension->GetMaxValue() - m_PhiDimension->GetMinValue() > 360) {
    mgui<<"The galactic longitude axis is not allowed to show more than 360 degrees."<<error;
    return false;
  }

  if (m_ThetaBins->IsInt(0, 10000) == false) return false;
  if (m_PhiBins->IsInt(0, 10000) == false) return false;
  
  m_GUIData->SetAxisTheta(m_ThetaImageAxis->GetAsDouble());
  m_GUIData->SetAxisPhi(m_PhiImageAxis->GetAsDouble());
  
  m_GUIData->SetThetaMin(m_ThetaDimension->GetMinValue());
  m_GUIData->SetThetaMax(m_ThetaDimension->GetMaxValue());
  m_GUIData->SetPhiMin(m_PhiDimension->GetMinValue());
  m_GUIData->SetPhiMax(m_PhiDimension->GetMaxValue());
  
  m_GUIData->SetBinsTheta(m_ThetaBins->GetAsInt());
  m_GUIData->SetBinsPhi(m_PhiBins->GetAsInt());

	return true;
}


// MGUIImageDimensionsSpherical: the end...
////////////////////////////////////////////////////////////////////////////////
