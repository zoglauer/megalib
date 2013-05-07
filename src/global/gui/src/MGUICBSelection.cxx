/*
 * MGUICBSelection.cxx
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
// MGUICBSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUICBSelection.h"

// Standard libs:

// ROOT libs:
#include <TObjString.h>

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUICBSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUICBSelection::MGUICBSelection(const TGWindow* Parent, const TGWindow* Main, 
                                 MString Title, MString SubTitle)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUICBSelection and bring it to the screen
  
  m_Title = Title;
  m_SubTitle = SubTitle;

  m_LastPressedButton = 0;
  m_IsMultiple = true;

  m_MsgWindow = 0;
  m_List = new MGUIECBList(this, "", m_IsMultiple);
}


////////////////////////////////////////////////////////////////////////////////


MGUICBSelection::~MGUICBSelection()
{
  // Delete an instance of MGUICBSelection

  delete m_List;
  delete m_ListLayout;

  delete m_SelectNone;
  delete m_SelectAll;
}


////////////////////////////////////////////////////////////////////////////////


void MGUICBSelection::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName(m_Title);  

  AddSubTitle(m_SubTitle); 

  m_ListLayout = 
    new TGLayoutHints(kLHintsLeft | kLHintsTop | 
                      kLHintsExpandX | kLHintsExpandY, 20, 20, 0, 20);
  m_List->Create();
  AddFrame(m_List, m_ListLayout);

  
  AddButtons();

  m_SelectNone = new TGTextButton(m_ButtonFrame, "Select None", e_SelectNone);
  m_SelectNone->Associate(this);
  m_ButtonFrame->AddFrame(m_SelectNone, m_RightButtonLayout);

  m_SelectAll = new TGTextButton(m_ButtonFrame, "Select All", e_SelectAll);
  m_SelectAll->Associate(this);
  m_ButtonFrame->AddFrame(m_SelectAll, m_LeftButtonLayout);
  if (m_IsMultiple == false) {
    m_SelectAll->SetState(kButtonDisabled);
  }

  PositionWindow(GetDefaultWidth()+40, GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::ProcessMessage(long Message, long Parameter1, 
				       long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        OnOk();
        break;
        
      case e_Cancel:
        OnCancel();
        break;
        
      case e_SelectNone:
        OnSelectNone();
        break;
        
      case e_SelectAll:
        OnSelectAll();
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
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


void MGUICBSelection::Add(MString Title, int Selected)
{
  // Add a new check button:
  // 0: not selected
  // 1: selected

  m_List->Add(Title, Selected);
}


////////////////////////////////////////////////////////////////////////////////


void MGUICBSelection::Add(TObjArray Titles, TArrayI Selected)
{
  // Add a list of check button
  // The Titles array contains several TObjString's

  int i;
  MString Title;

  for (i = 0; i < Titles.GetLast()+1; i++) {
    Title = ((TObjString *) Titles.At(i))->GetString();
    if (Selected.GetSize() > i) {
      m_List->Add(Title, Selected.At(i));
    } else {
      m_List->Add(Title);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


TArrayI MGUICBSelection::GetSelected()
{
  // Return the selected buttons as integer 

  return m_List->GetSelected();
}


////////////////////////////////////////////////////////////////////////////////


void MGUICBSelection::SetMultiple(bool On)
{
  m_IsMultiple = On;

  m_List->SetMultiple(m_IsMultiple);
}


////////////////////////////////////////////////////////////////////////////////


void MGUICBSelection::EmitOkMessage(const TGWindow *MsgWindow, 
                                    unsigned int MsgID1, unsigned int MsgID2)
{
  // After pressing OK the messages MsgID1 and MsgID2 are sent to the window
  // MsgWindow

  m_MsgWindow = MsgWindow;
  m_MsgID1 = MsgID1;
  m_MsgID2 = MsgID2;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::IsOKPressed()
{
  // Return true if the last pressed button was OK

  if (m_LastPressedButton == 1) {
    return true;
  } else { 
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::OnOk()
{
  // The Ok button has been pressed

  m_LastPressedButton = 1;

  if (m_MsgWindow != 0) {
    SendMessage(m_MsgWindow, m_MsgID1, m_MsgID2, 1);
  } else {
    CloseWindow();
  }

  UnmapWindow();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::OnCancel()
{
  // The cancel button has been pressed

  m_LastPressedButton = 0;

  if (m_MsgWindow != 0) {
    SendMessage(m_MsgWindow, m_MsgID1, m_MsgID2, 0);
  } else {
    CloseWindow();
  }
  UnmapWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::OnSelectAll()
{
  // The select all button has been pressed

  m_LastPressedButton = 0;
  m_List->SelectAll();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUICBSelection::OnSelectNone()
{
  // The select none button has been pressed

  m_LastPressedButton = 0;
  m_List->SelectNone();

  return true;
}



// MGUICBSelection: the end...
////////////////////////////////////////////////////////////////////////////////
