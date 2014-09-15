/*
 * MGUIEModule.cxx
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
// MGUIEModule
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEModule.h"

// Standard libs:

// ROOT libs:
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEModule)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEModule::MGUIEModule(const TGWindow* Parent, unsigned int ID, MModule* Module)
  : MGUIElement(Parent, kHorizontalFrame), m_ID(ID), m_Module(Module)
{
  // Construct an instance of MGUIEModule and bring it to the screen

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEModule::~MGUIEModule()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEModule::Create()
{
  // Create the GUI element

  // The title
  TGLabel* Title;
  if (m_Module != 0) {
    Title = new TGLabel(this, m_Module->GetName());
  } else {
    Title = new TGLabel(this, "Add a new module");
  }
  TGLayoutHints* TitleLayout = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 0, 0);
  AddFrame(Title, TitleLayout);

  // The buttons:
  
  MString ChangeText;
  if (m_Module == 0) {
    ChangeText = "Add";
  } else {
    ChangeText = "Change";    
  }

  m_ChangeButton = new TGTextButton(this, ChangeText, 600+m_ID); 
  m_ChangeButton->ChangeOptions(m_ChangeButton->GetOptions() | kFixedWidth);
  m_ChangeButton->SetLeftMargin(5*m_FontScaler);
  m_ChangeButton->SetRightMargin(5*m_FontScaler);
  m_ChangeButton->SetWidth(60*m_FontScaler);
  TGLayoutHints* ChangeButtonLayout = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 10, 0, 0, 0);
  AddFrame(m_ChangeButton, ChangeButtonLayout);
 
  m_RemoveButton = new TGTextButton(this, "Remove", 400+m_ID); 
  m_RemoveButton->ChangeOptions(m_ChangeButton->GetOptions() | kFixedWidth);
  m_RemoveButton->SetLeftMargin(5*m_FontScaler);
  m_RemoveButton->SetRightMargin(5*m_FontScaler);
  m_RemoveButton->SetWidth(60*m_FontScaler);
  TGLayoutHints* RemoveButtonLayout = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 10, 0, 0, 0);
  AddFrame(m_RemoveButton, RemoveButtonLayout);
  if (m_Module == 0) {
    m_RemoveButton->SetState(kButtonDisabled);
  }
 
  m_OptionsButton = new TGTextButton(this, "Options", 500+m_ID); 
  m_OptionsButton->ChangeOptions(m_ChangeButton->GetOptions() | kFixedWidth);
  m_OptionsButton->SetLeftMargin(5*m_FontScaler);
  m_OptionsButton->SetRightMargin(5*m_FontScaler);
  m_OptionsButton->SetWidth(60*m_FontScaler);
  TGLayoutHints* OptionsButtonLayout = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 30, 0, 0, 0);
  AddFrame(m_OptionsButton, OptionsButtonLayout);
  if (m_Module == 0 || (m_Module != 0 && m_Module->HasOptionsGUI() == false)) {
    m_OptionsButton->SetState(kButtonDisabled);
  }


  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEModule::Associate(const TGWindow* Window)
{
  // Transfer button clicks to this window

  m_ChangeButton->Associate(Window);
  m_RemoveButton->Associate(Window);
  m_OptionsButton->Associate(Window);
}


// MGUIEModule: the end...
////////////////////////////////////////////////////////////////////////////////
