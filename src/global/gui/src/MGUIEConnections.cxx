/*
 * MGUIEConnections.cxx
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
// MGUIEConnections
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEConnections.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIEConnection.h"

////////////////////////////////////////////////////////////////////////////////


MGUIEConnections::MGUIEConnections(const TGWindow* Parent, MString Label, bool Emphasize) :
  MGUIElement(Parent, kVerticalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_Label = Label;

  m_IsEmphasized = Emphasize;
  m_IsEnabled = true;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEConnections::~MGUIEConnections()
{
  // Destruct this instance of MGUIEConnections
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::Create()
{
  // Create the label and the input-field.

  // Label:
  m_TextLabel = new TGLabel(this, new TGString(m_Label));
  if (m_IsEmphasized == true) {
    m_TextLabel->SetTextFont(m_EmphasizedFont);
  }    
  m_TextLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterX, 5, 5, 5, 5);
  AddFrame(m_TextLabel, m_TextLabelLayout);

  // Connections:
  m_ConnectionsLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10);
  m_Connections = new TGCanvas(this, 200, 200);
  // Since we do not create the element in the constructor, 
  // we have initialze some pointers:
  m_Container = new TGCompositeFrame(m_Connections->GetViewPort(), 50, 50);
  m_Connections->SetContainer(m_Container);
  //m_Connections->Associate(this);
  AddFrame(m_Connections, m_ConnectionsLayout);

  m_ConnectionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 10, 10, 10, 10);
  for (unsigned int i = 0; i < m_ConnectionList.size(); ++i) {
    m_Container->AddFrame(m_ConnectionList[i], m_ConnectionLayout);
  }

  // Buttons:
  m_ButtonFrameLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 5, 10, 10);
  m_ButtonFrame = new TGCompositeFrame(this, 0, 0, kHorizontalFrame);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);

  m_ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 0, 0);
  m_AddButton = new TGTextButton(m_ButtonFrame, "Add Connection", e_Add);
  m_AddButton->Associate(this);
  m_ButtonFrame->AddFrame(m_AddButton, m_ButtonLayout);
  m_RemoveButton = new TGTextButton(m_ButtonFrame, "Remove Connection", e_Remove); 
  m_RemoveButton->Associate(this);
  m_ButtonFrame->AddFrame(m_RemoveButton, m_ButtonLayout);

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::Associate(TGCompositeFrame* w)
{
  // Set a window to which all messages are sent

  m_MessageWindow = w;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEConnections::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_CHECKBUTTON:
      break;
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  case kC_VSCROLL:
    switch (GET_SUBMSG(Message)) {
    case kSB_SLIDERPOS:
      // Scroll the viewport to the new position (vertical)
      m_Connections->GetViewPort()->SetVPos(-Parameter1);
      break;
    default:
      break;
    }
    break;
  case kC_HSCROLL:
    switch (GET_SUBMSG(Message)) {
    case kSB_SLIDERPOS:
      // Scroll the viewport to the new position (horizontal)
      m_Connections->GetViewPort()->SetHPos(-Parameter1);
      break;
    default:
      break;
    }
    break;  default:
    break;
  }
  
  return m_MessageWindow->ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::Add(MConnection* Connection)
{
  MGUIEConnection* T = new MGUIEConnection(m_Container, Connection);

  m_ConnectionList.push_back(T);
  m_Container->AddFrame(T, m_ConnectionLayout);
  T->MapSubwindows();
  T->MapWindow();
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


vector<MConnection*> MGUIEConnections::GetMarked()
{
  vector<MConnection*> Marked;

  for (unsigned int t = 0; t < m_ConnectionList.size(); ++t) {
    if (m_ConnectionList[t] == 0) continue;
    if (m_ConnectionList[t]->IsMarked() == true) {
      Marked.push_back(m_ConnectionList[t]->GetConnection());
    }
  }

  return Marked;
}

////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::Remove(MConnection* Connection)
{
  //

  MGUIEConnection* T = 0;

  for (unsigned int t = 0; t < m_ConnectionList.size(); ++t) {
    if (m_ConnectionList[t] == 0) continue;
    if (m_ConnectionList[t]->GetConnection() == Connection) {
      T = m_ConnectionList[t];
      m_ConnectionList[t] = 0;
      break;
    }
  }

  if (T != 0) {
    cout<<"Removing connection "<<Connection->GetName()<<endl;
    T->UnmapWindow();
    m_Container->RemoveFrame(T);
    delete T;
  }

  Layout();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::AddTransceiverName(MString Name)
{

  for (unsigned int c = 0; c < m_ConnectionList.size(); ++c) {
    cout<<"void MGUIEConnections::AddTransceiverName(MString Name)"<<Name<<endl;
    m_ConnectionList[c]->AddTransceiverName(Name);
  }

  m_TransceiverNames.push_back(Name);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnections::RemoveTransceiverName(MString Name)
{
  for (unsigned int c = 0; c < m_ConnectionList.size(); ++c) {
    m_ConnectionList[c]->RemoveTransceiverName(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////



// MGUIEConnections.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
