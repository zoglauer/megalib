/*
 * MGUIImageDimensionsCart.cxx                         
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
// MGUIImageDimensionsCart
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIImageDimensionsCart.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSystem.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIImageDimensionsCart)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIImageDimensionsCart::MGUIImageDimensionsCart(const TGWindow* Parent, 
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


MGUIImageDimensionsCart::~MGUIImageDimensionsCart()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIImageDimensionsCart::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Image Dimensions");  

  AddSubTitle("Please enter the dimensions \n and the number of bins for Cartesean images"); 

  m_DimensionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 0, 0);
  m_BinLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 0, 20);

  m_XDimension = new MGUIEMinMaxEntry(this,
                                      TString("x-axis:"),
                                       false,
                                      TString("Minimum [cm]: "),
                                      TString("Maximum [cm]: "),
                                      m_GUIData->GetXMin(), 
                                      m_GUIData->GetXMax());
  AddFrame(m_XDimension, m_DimensionLayout);

  m_XBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsX());
  AddFrame(m_XBins, m_BinLayout);

  m_YDimension = new MGUIEMinMaxEntry(this,
                                      TString("y-axis:"),
                                      false,
                                      TString("Minimum [cm]: "),
                                      TString("Maximum [cm]: "),
                                      m_GUIData->GetYMin(), 
                                      m_GUIData->GetYMax());
  AddFrame(m_YDimension, m_DimensionLayout);

  m_YBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsY());
  AddFrame(m_YBins, m_BinLayout);

 
  m_ZDimension = new MGUIEMinMaxEntry(this,
                                      TString("z-axis:"),
                                      false,
                                      TString("Minimum [cm]: "),
                                      TString("Maximum [cm]: "),
                                      m_GUIData->GetZMin(), 
                                      m_GUIData->GetZMax());
  AddFrame(m_ZDimension, m_DimensionLayout);
    
  m_ZBins = new MGUIEEntry(this, "Number of Bins:", false, m_GUIData->GetBinsZ());
  AddFrame(m_ZBins, m_BinLayout);


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIImageDimensionsCart::OnApply()
{
	// The Apply button has been pressed

  if (m_XDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
  if (m_YDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
  if (m_ZDimension->CheckRange(-100000.0, 100000.0, -100000.0, 100000000.0, true) == false) return false;
  
  if (m_XBins->IsInt(0, 10000) == false) return false;
  if (m_YBins->IsInt(0, 10000) == false) return false;
  if (m_ZBins->IsInt(0, 10000) == false) return false;
  
  m_GUIData->SetXMin(m_XDimension->GetMinValue());
  m_GUIData->SetXMax(m_XDimension->GetMaxValue());
  m_GUIData->SetYMin(m_YDimension->GetMinValue());
  m_GUIData->SetYMax(m_YDimension->GetMaxValue());
  m_GUIData->SetZMin(m_ZDimension->GetMinValue());
  m_GUIData->SetZMax(m_ZDimension->GetMaxValue());
  
  m_GUIData->SetBinsX(m_XBins->GetAsInt());
  m_GUIData->SetBinsY(m_YBins->GetAsInt());
  m_GUIData->SetBinsZ(m_ZBins->GetAsInt());
  
  return true;
}



// MGUIImageDimensionsCart: the end...
////////////////////////////////////////////////////////////////////////////////
