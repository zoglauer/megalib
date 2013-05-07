/*
 * MGUIETransceivers.cxx
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
// MGUIETransceivers
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIETransceivers.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIETransceiver.h"

////////////////////////////////////////////////////////////////////////////////


MGUIETransceivers::MGUIETransceivers(const TGWindow *Parent, MString Label, bool Emphasize) :
  MGUIElement(Parent, kVerticalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_IsEmphasized = Emphasize;
  m_Label = Label;
  m_IsEnabled = true;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIETransceivers::~MGUIETransceivers()
{
  // Destruct this instance of MGUIETransceivers
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceivers::Create()
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

  // Transceivers:
  m_TransceiversLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10);
  m_Transceivers = new TGCanvas(this, 200, 200);
  // Since we do not create the element in the constructor, 
  // we have initialze some pointers:
  m_Container = new TGCompositeFrame(m_Transceivers->GetViewPort(), 50, 50);
  m_Transceivers->SetContainer(m_Container);
  //m_Transceivers->Associate(this);
  AddFrame(m_Transceivers, m_TransceiversLayout);

  m_TransceiverLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 10, 10, 10, 10);
  for (unsigned int i = 0; i < m_TransceiverList.size(); ++i) {
    m_Container->AddFrame(m_TransceiverList[i], m_TransceiverLayout);
  }

  // Buttons:
  m_ButtonFrameLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 5, 5, 10, 10);
  m_ButtonFrame = new TGCompositeFrame(this, 0, 0, kHorizontalFrame);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);

  m_ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 0, 0);
  m_AddButton = new TGTextButton(m_ButtonFrame, "Add Transceiver", e_Add);
  m_AddButton->Associate(this);
  m_ButtonFrame->AddFrame(m_AddButton, m_ButtonLayout);
  m_RemoveButton = new TGTextButton(m_ButtonFrame, "Remove Transceiver", e_Remove); 
  m_RemoveButton->Associate(this);
  m_ButtonFrame->AddFrame(m_RemoveButton, m_ButtonLayout);

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceivers::Associate(TGCompositeFrame* w)
{
  // Set a window to which all messages are sent

  m_MessageWindow = w;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIETransceivers::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  // cout<<"TransceiverS Msg: "<<Message<<"!"<<Parameter1<<"!"<<Parameter2<<endl;

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
      m_Transceivers->GetViewPort()->SetVPos(-Parameter1);
      break;
    default:
      break;
    }
    break;
  case kC_HSCROLL:
    switch (GET_SUBMSG(Message)) {
    case kSB_SLIDERPOS:
      // Scroll the viewport to the new position (horizontal)
      m_Transceivers->GetViewPort()->SetHPos(-Parameter1);
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



void MGUIETransceivers::Add(MTransceiverTcpIp* Transceiver)
{
  MGUIETransceiver* T = new MGUIETransceiver(m_Container, Transceiver);

  m_TransceiverList.push_back(T);
  m_Container->AddFrame(T, m_TransceiverLayout);
  T->MapSubwindows();
  T->MapWindow();
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


vector<MTransceiverTcpIp*> MGUIETransceivers::GetMarked()
{
  vector<MTransceiverTcpIp*> Marked;

  for (unsigned int t = 0; t < m_TransceiverList.size(); ++t) {
    if (m_TransceiverList[t] == 0) continue;
    if (m_TransceiverList[t]->IsMarked() == true) {
      Marked.push_back(m_TransceiverList[t]->GetTransceiver());
    }
  }

  return Marked;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIETransceivers::Remove(MTransceiverTcpIp* Transceiver)
{
  //

  MGUIETransceiver* T = 0;

  for (unsigned int t = 0; t < m_TransceiverList.size(); ++t) {
    if (m_TransceiverList[t] == 0) continue;
    if (m_TransceiverList[t]->GetTransceiver() == Transceiver) {
      T = m_TransceiverList[t];
      m_TransceiverList[t] = 0;
      break;
    }
  }

  if (T != 0) {
    cout<<"Removing transceiver "<<Transceiver->GetName()<<endl;
    T->UnmapWindow();
    m_Container->RemoveFrame(T);
    delete T;
  }

  Layout();
}


////////////////////////////////////////////////////////////////////////////////




// MGUIETransceivers.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
