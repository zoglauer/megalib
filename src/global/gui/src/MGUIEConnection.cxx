/*
 * MGUIEConnection.cxx
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
// MGUIEConnection
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEConnection.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEConnection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEConnection::MGUIEConnection(const TGWindow* Parent, MConnection* Connection)
  : MGUIElement(Parent, kRaisedFrame | kSunkenFrame | kVerticalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_Connection = Connection;
  m_TransceiverNames.resize(0);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEConnection::~MGUIEConnection()
{
  // Destruct this instance of MGUIEConnection
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnection::Create()
{
  // Create the label and the input-field.

  // Label:
  m_Label = new TGCheckButton(this, m_Connection->GetName(), e_Id);
  m_LabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 5, 0);
  AddFrame(m_Label, m_LabelLayout);


  m_ComboLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 0, 5, 0);

  // The Host field
  m_Type = new MGUIEComboBox(this, "Objects to transmit:", false);
  m_Type->SetComboBoxSize(150);
  m_Type->Add("None");
  m_Type->Add("All events");
  m_Type->Add("Uncalibrated events");
  m_Type->Add("Calibrated events");
  m_Type->Add("Physical events");
  m_Type->Add("Housekeeping data");
  m_Type->Add("Commands");
  m_Type->Add("All", true);
  m_Type->Add("All except commands");
  m_Type->Create();
  AddFrame(m_Type, m_ComboLayout);

  m_Start = new MGUIEComboBox(this, "Receive from:", false);
  m_Start->SetComboBoxSize(150);
  m_Start->Create();
  AddFrame(m_Start, m_ComboLayout);

  m_Stop = new MGUIEComboBox(this, "Transmit to:", false);
  m_Stop->SetComboBoxSize(150);
  m_Stop->Create();
  AddFrame(m_Stop, m_ComboLayout);

  for (unsigned int n = 0; n < m_TransceiverNames.size(); ++n) {
    m_Start->Add(m_TransceiverNames[n]);
    m_Stop->Add(m_TransceiverNames[n]);
  }

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEConnection::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  // cout<<"Msg: "<<Message<<"!"<<Parameter1<<"!"<<Parameter2<<endl;

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
  default:
    break;
  }

  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEConnection::IsMarked()
{
  return (m_Label->GetState() == kButtonDown) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////


MConnection* MGUIEConnection::GetConnection()
{
  // Return the underlaying connection# 

  return m_Connection;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnection::AddTransceiverName(MString Name)
{
  // Add the name of a receiver two the two connection combo boxes

  m_TransceiverNames.push_back(Name);
  m_Start->Add(m_TransceiverNames.back());
  m_Stop->Add(m_TransceiverNames.back());
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEConnection::RemoveTransceiverName(MString Name)
{
  // Remove a name of a receiver from the two connection combo boxes

  // Find entry
  bool Found = false;
  unsigned int n = 0;
  for (n = 0; n < m_TransceiverNames.size(); ++n) {
    if (m_TransceiverNames[n] == Name) {
      Found = true;
      break;
    }
  }

  if (Found == false) return;

  m_Start->Remove(m_TransceiverNames[n]);
  m_Stop->Remove(m_TransceiverNames[n]);
  m_TransceiverNames[n] = "";
}


////////////////////////////////////////////////////////////////////////////////




// MGUIEConnection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
