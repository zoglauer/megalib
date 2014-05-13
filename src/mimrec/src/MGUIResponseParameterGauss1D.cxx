/*
 * MGUIResponseParameterGauss1D.cxx
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
// MGUIResponseParameterGauss1D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterGauss1D.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIResponseParameterGauss1D)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterGauss1D::MGUIResponseParameterGauss1D(const TGWindow* Parent, 
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


MGUIResponseParameterGauss1D::~MGUIResponseParameterGauss1D()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseParameterGauss1D::Create()
{
  // Create the main window

  int Width = m_FontScaler*550;

  // We start with a name and an icon...
  SetWindowName("Response parameters Gauss 1D");  

  AddSubTitle("Parameters for a 1D Gaussian response approximation"); 

  TGLayoutHints* GaussiansLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 10);
  m_Gaussians = new MGUIEEntryList(this, "Set the 1D Gaussian approximations of the response:");
  m_Gaussians->Add("Compton across cone (e.g. 1 sigma of ARM width) [deg]:", m_GUIData->GetFitParameterComptonTransSphere(), true, 0.0, 180.0);
  m_Gaussians->Add("Compton along cone for tracked events (e.g. 1 sigma of SPD width) [deg]:", m_GUIData->GetFitParameterComptonLongSphere(), true, 0.0, 180.0);
  m_Gaussians->Add("Pairs [deg]:", m_GUIData->GetFitParameterPair(), true, 0.0, 180.0);
  m_Gaussians->SetWrapLength(Width - m_FontScaler*40);
  m_Gaussians->Create();
  AddFrame(m_Gaussians, GaussiansLayout);

  TGLayoutHints* CutOffLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 20);
  m_CutOff = new MGUIEEntryList(this, "Set the cut-off out to which distance the Gauss is calculated in sigma. A smaller cut-off increases the reconstruction speed, but deteriorates the image. If you use the maths approximations and/or 1-byte accuracy, then there will be an automatic cut-off at ~4.");
  m_CutOff->Add("Gaussian cut-off (suggested value: 2.5) [sigmas]:", m_GUIData->GetGauss1DCutOff(), true, 1.5);
  m_CutOff->SetWrapLength(Width - m_FontScaler*40);
  m_CutOff->Create();
  AddFrame(m_CutOff, CutOffLayout);

  TGLayoutHints* AbsorptionsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 20);
  m_Absorptions = new TGCheckButton(this, "Use absorption probabilities (very time consuming!!!) - they are always used for photo-effect (i.e. coded-mask) events", c_AbsorptionId);
  m_Absorptions->SetWrapLength(Width - m_FontScaler*40);
  AddFrame(m_Absorptions, AbsorptionsLayout);

  if (m_GUIData->GetUseAbsorptions() == true) {
    m_Absorptions->SetState(kButtonDown);
  } else {
    m_Absorptions->SetState(kButtonUp);
  }

  AddButtons();

  PositionWindow(Width, GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseParameterGauss1D::OnApply()
{
  // The Apply button has been pressed

  // First test the data
  if (m_Gaussians->IsModified() == true) {
    m_GUIData->SetFitParameterComptonTransSphere(m_Gaussians->GetAsDouble(0)); 
    m_GUIData->SetFitParameterComptonLongSphere(m_Gaussians->GetAsDouble(1)); 
    m_GUIData->SetFitParameterPair(m_Gaussians->GetAsDouble(2));
  }
  if (m_CutOff->IsModified() == true) {
    m_GUIData->SetGauss1DCutOff(m_CutOff->GetAsDouble(0)); 
  }

  if (m_Absorptions->GetState() == kButtonUp) {
    if (m_GUIData->GetUseAbsorptions() == true) m_GUIData->SetUseAbsorptions(false);
  } else {
    if (m_GUIData->GetUseAbsorptions() == false) m_GUIData->SetUseAbsorptions(true);
  }

  
	return true;
}


// MGUIResponseParameterGauss1D: the end...
////////////////////////////////////////////////////////////////////////////////
