/*
 * MGUIModuleSelector.cxx
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


// Include the header:
#include "MGUIModuleSelector.h"

// Standard libs:
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

// ROOT libs:
#include <KeySymbols.h>
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModule.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIModuleSelector)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIModuleSelector::MGUIModuleSelector(MSupervisor* Data, unsigned int Position) 
  : TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 320, 240), m_Data(Data), m_Position(Position)
{
  // standard constructor

  m_List = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIModuleSelector::~MGUIModuleSelector()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIModuleSelector::Create()
{
  
  // We start with a name and an icon...
  SetWindowName("Module selector");  

  // Main label
  const TGFont* lFont = gClient->GetFont("-*-helvetica-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
  if (!lFont) lFont = gClient->GetResourcePool()->GetDefaultFont();
  FontStruct_t LargeFont = lFont->GetFontStruct();

  TGLabel* MainLabel = new TGLabel(this, "Choose a module:");
  MainLabel->SetTextFont(LargeFont);
  TGLayoutHints* MainLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  AddFrame(MainLabel, MainLabelLayout);


  //
  vector<MModule*> Modules = m_Data->ReturnPossibleVolumes(m_Position);
  if (Modules.size() > 0) {
    m_List = new MGUIERBList(this, "The following modules are available: ");
    for (unsigned int m = 0; m < Modules.size(); ++m) {
      m_List->Add(Modules[m]->GetName());
    }
    m_List->Create();
    TGLayoutHints* ListLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 20, 20, 20, 20);
    AddFrame(m_List, ListLayout);
  } else {
    TGLabel* NoModulesLabel = new TGLabel(this, "No modules available fullfilling all requirements!");
    TGLayoutHints* NoModulesLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 20, 20, 20, 20);
    AddFrame(NoModulesLabel, NoModulesLayout);
  }


  // OK and cancel buttons
  // Frame around the buttons:
  TGHorizontalFrame* ButtonFrame = new TGHorizontalFrame(this, 150, 25);
  TGLayoutHints* ButtonFrameLayout =  new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10, 10, 10, 10);
  AddFrame(ButtonFrame, ButtonFrameLayout);
  
  // The buttons itself
  TGTextButton* OKButton = new TGTextButton(ButtonFrame, "OK", e_Ok); 
  OKButton->Associate(this);
  TGLayoutHints* OKButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 20, 0, 0, 0);
  ButtonFrame->AddFrame(OKButton, OKButtonLayout);
  
  TGTextButton* CancelButton = new TGTextButton(ButtonFrame, "     Cancel     ", e_Cancel); 
  CancelButton->Associate(this);
  TGLayoutHints* CancelButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  ButtonFrame->AddFrame(CancelButton, CancelButtonLayout);

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIModuleSelector::CloseWindow()
{
  // When the x is pressed, this function is called.

  OnCancel();
}



////////////////////////////////////////////////////////////////////////////////


bool MGUIModuleSelector::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
        Status = OnApply();
        break;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIModuleSelector::OnOk()
{
  // The OK button has been pressed

  if (OnApply() == true) {
    UnmapWindow();
    return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIModuleSelector::OnCancel()
{
  // The Cancel button has been pressed

  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIModuleSelector::OnApply()
{
  // The Apply button has been pressed

  if (m_List != 0 && m_List->GetNEntries() > 0) {
    if (m_List->GetSelected() < 0) {
      mgui<<"Please select a module or press cancel"<<show;
      return false;
    }
    
    vector<MModule*> Modules = m_Data->ReturnPossibleVolumes(m_Position);
    m_Data->SetModule(Modules[m_List->GetSelected()], m_Position);
  }

  UnmapWindow();

  return true;
}


// MGUIModuleSelector: the end...
////////////////////////////////////////////////////////////////////////////////
