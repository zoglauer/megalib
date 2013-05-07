/*
 * MGUIRBSelection.cxx
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
// MGUIRBSelection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIRBSelection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIRBSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIRBSelection::MGUIRBSelection(const TGWindow* Parent, const TGWindow* Main, 
                                 MString Title, MString SubTitle)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIRBSelection and bring it to the screen
  
  m_Title = Title;
  m_SubTitle = SubTitle;

  m_LastPressedButton = 0;

  m_MsgWindow = 0;
  m_List = new MGUIERBList(this, "");
}


////////////////////////////////////////////////////////////////////////////////


MGUIRBSelection::~MGUIRBSelection()
{
  // Delete an instance of MGUIRBSelection

  delete m_List;
  delete m_ListLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRBSelection::Create()
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


  // and bring it to the screen.
	PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
	MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}



////////////////////////////////////////////////////////////////////////////////


void MGUIRBSelection::Add(MString Title, int Selected)
{
  // Add a new check button:
  // 0: not selected
  // 1: selected

  m_List->Add(Title, Selected);
}


////////////////////////////////////////////////////////////////////////////////


int MGUIRBSelection::GetSelected()
{
  // Return the selected buttons as integer 

  return m_List->GetSelected();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRBSelection::SetSelected(int Selected)
{
  // Set the selected button 

  m_List->SetSelected(Selected);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRBSelection::EmitOkMessage(const TGWindow *MsgWindow, 
                                    unsigned int MsgID1, unsigned int MsgID2)
{
  // After pressing OK the messages MsgID1 and MsgID2 are sent to the window
  // MsgWindow

  m_MsgWindow = MsgWindow;
  m_MsgID1 = MsgID1;
  m_MsgID2 = MsgID2;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIRBSelection::IsOKPressed()
{
  // Return true if the last pressed button was OK

  if (m_LastPressedButton == 1) {
    return true;
  } else { 
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIRBSelection::OnOk()
{
	//

	if (m_MsgWindow != 0) {
		SendMessage(m_MsgWindow, m_MsgID1, m_MsgID2, GetSelected());
	}
	CloseWindow();
	
	return true;
}


// MGUIRBSelection: the end...
////////////////////////////////////////////////////////////////////////////////
