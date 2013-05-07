/*
 * MGUIResponseParameterUACR.cxx
 *
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
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
// MGUIResponseParameterUACR
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIResponseParameterUACR.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIResponseParameterUACR)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIResponseParameterUACR::MGUIResponseParameterUACR(const TGWindow* Parent, 
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


MGUIResponseParameterUACR::~MGUIResponseParameterUACR()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIResponseParameterUACR::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Parameter for an UACR");  

  AddSubTitle("Enter the fit parameter for the response\nof Compton- and pair-events"); 

  m_SelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 2, 2);

  m_ComptonLong = 
    new MGUIEEntry(this, "Compton - longitudinal (e.g. 1 sigma of SPD width) [°]:", false, 
                   m_GUIData->GetFitParameterComptonLongSphere());
  m_ComptonTrans =
    new MGUIEEntry(this, "Compton - transversal (e.g. 1 sigma of ARM width) [°]:", false, 
                   m_GUIData->GetFitParameterComptonTransSphere());
  m_Pair = 
    new MGUIEEntry(this, "Pair - circular [°]:", false, 
                   m_GUIData->GetFitParameterPair());


  AddFrame(m_ComptonLong, m_SelectorLayout);
  AddFrame(m_ComptonTrans, m_SelectorLayout);
  AddFrame(m_Pair, m_SelectorLayout);

  m_AbsorptionsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 20);
  m_Absorptions = new TGCheckButton(this, "Use absorption probabilities (time consuming!!!)", c_AbsorptionId);
  AddFrame(m_Absorptions, m_AbsorptionsLayout);

  if (m_GUIData->GetUseAbsorptions() == true) {
    m_Absorptions->SetState(kButtonDown);
  } else {
    m_Absorptions->SetState(kButtonUp);
  }

  m_FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 20, 2);
  m_FileSelector = new MGUIEFileSelector(this, "Choose a file for the fit-parameters (if the box is not empty, then use the file)", 
                                        m_GUIData->GetFitParameterSphereFile());

  AddFrame(m_FileSelector, m_FileSelectorLayout);


  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIResponseParameterUACR::OnApply()
{
  // The Apply button has been pressed

  // First test the data
  if (m_ComptonLong->IsDouble(0, 180) == false || 
      m_ComptonTrans->IsDouble(0, 180) == false || 
      m_Pair->IsDouble(0, 180) == false) {
    return false;
  }
  
  m_GUIData->SetFitParameterComptonLongSphere(m_ComptonLong->GetAsDouble()); 
  m_GUIData->SetFitParameterComptonTransSphere(m_ComptonTrans->GetAsDouble()); 
  m_GUIData->SetFitParameterPair(m_Pair->GetAsDouble()); 
  
  if (m_Absorptions->GetState() == kButtonUp) {
    m_GUIData->SetUseAbsorptions(false);
  } else {
    m_GUIData->SetUseAbsorptions(true);
  }
  
  m_GUIData->SetFitParameterSphereFile(m_FileSelector->GetFileName());
  if (m_FileSelector->GetFileName().CompareTo(TString("")) != 0) {
    m_GUIData->SetUseFitParameterSphereFile(true);
  } else {
    m_GUIData->SetUseFitParameterSphereFile(false);
  }
  
	return true;
}


// MGUIResponseParameterUACR: the end...
////////////////////////////////////////////////////////////////////////////////
