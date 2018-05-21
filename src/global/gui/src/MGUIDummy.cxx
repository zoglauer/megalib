/*
 * MGUIDummy.cxx
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
// MGUIDummy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDummy.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIDummy)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDummy::MGUIDummy(const TGWindow* Parent, const TGWindow* Main, 
                     MGUIData* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIDummy and bring it to the screen

  m_Data = Data;

  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDummy::~MGUIDummy()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDummy::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Title");  
  AddSubTitle("Subtitle"); 

  // Add here ...


  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDummy::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
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


bool MGUIDummy::OnOk()
{
  // The Apply button has been pressed

  if (OnApply() == true) {
    CloseWindow();
    return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDummy::OnCancel()
{
  // The Apply button has been pressed

  CloseWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDummy::OnApply()
{
  // The Apply button has been pressed

  // Add here...

  return true;
}


// MGUIDummy: the end...
////////////////////////////////////////////////////////////////////////////////
