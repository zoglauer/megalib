/*
 * MGUILicense.cxx
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
// MGUILicense
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUILicense.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUILicense)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUILicense::MGUILicense(const TGWindow* Parent, const TGWindow* Main, 
                         MString FileName, bool ShowButtons)
  : MGUIDialog(Parent, Main, 700, 400)
{
  // Construct an instance of MGUILicense and bring it to the screen

  m_FileName = FileName;
  m_IsAccepted = false;
  m_ShowButtons = ShowButtons;
}


////////////////////////////////////////////////////////////////////////////////


MGUILicense::~MGUILicense()
{
  // Delete an instance of MGUILicense

  delete m_Text;
  delete m_TextLayout;

  delete m_DeclineButton;
  delete m_AcceptButton;
  delete m_ButtonLayout;

  delete m_ButtonFrame;
  delete m_ButtonFrameLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUILicense::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("License agreement");  
  AddSubTitle("Please read the following license agreement very carefully\n"
              "You have to accept the license agreement to use the software"); 

  m_Text = new TGTextView(this, 10, 10);
  m_Text->LoadFile(m_FileName);
  m_TextLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 20, 20, 3, 3);
  AddFrame(m_Text, m_TextLayout);

  m_ButtonFrame = new TGHorizontalFrame(this, 150, 25, kFixedSize);
  m_ButtonFrameLayout = 
    new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
                      10, 10, 10, 8);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
  
  m_ButtonLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 5, 0, 0);

  m_DeclineButton = new TGTextButton(m_ButtonFrame, "Decline", c_Decline); 
  m_DeclineButton->Associate(this);
  m_ButtonFrame->AddFrame(m_DeclineButton, m_ButtonLayout);
  
  m_AcceptButton = new TGTextButton(m_ButtonFrame, "Accept", c_Accept); 
  m_AcceptButton->Associate(this);
  m_ButtonFrame->AddFrame(m_AcceptButton, m_ButtonLayout);

  PositionWindow(700, 500, false);

  // and bring it to the screen.
  MapSubwindows();
  Layout();
  MapWindow();  
 
  gClient->WaitForUnmap(this);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUILicense::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
{
  // Process the messages for this window

	bool Status = true;
	
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_Decline:
        Status = OnDecline();
        break;

      case c_Accept:
				Status = OnAccept();
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


bool MGUILicense::OnAccept()
{
	// The Accept button has been pressed

  m_IsAccepted = true;

  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUILicense::OnDecline()
{
	// The Decline button has been pressed

	UnmapWindow();

	return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUILicense::IsAccepted()
{
  // Return true if the user has pressed the Accept button and not the decline 

  return m_IsAccepted;
}


// MGUILicense: the end...
////////////////////////////////////////////////////////////////////////////////
