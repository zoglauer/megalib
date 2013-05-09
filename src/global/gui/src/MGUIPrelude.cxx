/*
 * MGUIPrelude.cxx
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
// MGUIPrelude
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIPrelude.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIPrelude)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPrelude::MGUIPrelude(MString Title, MString SubTitle, MString Text, MString OKButton, MString CancelButton)
  : MGUIDialog(gClient->GetRoot(), gClient->GetRoot(), 700, 400)
{
  // Construct an instance of MGUIPrelude and bring it to the screen

  m_Title = Title;
  m_SubTitle = SubTitle;
  m_Text = Text;
  m_OKButtonText = OKButton;
  m_CancelButtonText = CancelButton;
  
  m_IsOKed = false;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIPrelude::~MGUIPrelude()
{
  // Delete an instance of MGUIPrelude
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPrelude::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName(m_Title);  
  AddSubTitle(m_SubTitle); 

  m_TextView = new TGTextView(this, 10, 10);
  m_TextView->LoadBuffer(m_Text);
  m_TextLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 20, 20, 3, 3);
  AddFrame(m_TextView, m_TextLayout);

  m_ButtonFrame = new TGHorizontalFrame(this, 150, 25, kFixedSize);
  m_ButtonFrameLayout = 
    new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
                      10, 10, 10, 8);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
  
  m_ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 5, 0, 0);

  if (m_CancelButtonText != "") {
    m_CancelButton = new TGTextButton(m_ButtonFrame, m_CancelButtonText, c_Cancel); 
    m_CancelButton->Associate(this);
    m_ButtonFrame->AddFrame(m_CancelButton, m_ButtonLayout);
  }
  
  m_OKButton = new TGTextButton(m_ButtonFrame, m_OKButtonText, c_Ok); 
  m_OKButton->Associate(this);
  m_ButtonFrame->AddFrame(m_OKButton, m_ButtonLayout);

  PositionWindow(700, 500, false);

  // and bring it to the screen.
  MapSubwindows();
  Layout();
  MapWindow();  
 
  gClient->WaitForUnmap(this);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPrelude::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
{
  // Process the messages for this window

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_Cancel:
        Status = OnCancel();
        break;

      case c_Ok:
        Status = OnOK();
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


void MGUIPrelude::CloseWindow()
{
  // When the x is pressed, this function is called.

  OnCancel();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPrelude::OnOK()
{
  // The OK/Accept button has been pressed

  m_IsOKed = true;

  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPrelude::OnCancel()
{
  // The Cancel/Decline button has been pressed

  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPrelude::IsOKed()
{
  // Return true if the user has pressed the Accept button and not the decline 

  return m_IsOKed;
}


// MGUIPrelude: the end...
////////////////////////////////////////////////////////////////////////////////
