/*
 * MGUIResponseParameterGaussianByUncertainties.cxx
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
// MGUIResponseParameterGaussianByUncertainties
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterGaussianByUncertainties.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIResponseParameterGaussianByUncertainties)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterGaussianByUncertainties::MGUIResponseParameterGaussianByUncertainties(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterGaussianByUncertainties::~MGUIResponseParameterGaussianByUncertainties()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseParameterGaussianByUncertainties::Create()
{
  // Create the main window

  int Width = 400*m_FontScaler;

  // We start with a name and an icon...
  SetWindowName("Response parameters Gaussian by uncertainties");  

  AddSubTitle("Parameters for a 1D Gaussian response approximation\ncalculated by the given uncertainties"); 

  TGLayoutHints* IncreaseLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 20);
  m_Increase = new MGUIEEntry(this, "Increase the calculated uncertainty by this amount of degrees. Having a larger cone width makes the images a bit smoother but sacrificies sensitivity.", false, m_GUIData->GetGaussianByUncertaintiesIncrease(), true, 0.0, 20.0);
  m_Increase->SetEntryFieldSize(m_FontScaler*60);
  m_Increase->SetWrapLength(Width - m_FontScaler*100);
  cout<<"Set WL: "<<Width- m_FontScaler*400<<endl;
  AddFrame(m_Increase, IncreaseLayout);

  AddButtons();

  PositionWindow(Width, GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseParameterGaussianByUncertainties::OnApply()
{
  // The Apply button has been pressed

  // First test the data
  if (m_Increase->IsModified() == true) {
    m_GUIData->SetGaussianByUncertaintiesIncrease(m_Increase->GetAsDouble()); 
  }

  return true;
}


// MGUIResponseParameterGaussianByUncertainties: the end...
////////////////////////////////////////////////////////////////////////////////
