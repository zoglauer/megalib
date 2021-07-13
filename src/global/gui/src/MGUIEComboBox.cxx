/*
 * MGUIEComboBox.cxx
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
// MGUIEComboBox
//
//
// This class is an elementary GUI-widget:
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEComboBox.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>
#include <TObjString.h>

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEComboBox)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEComboBox::MGUIEComboBox(const TGWindow* Parent, MString Label, bool Emphasize, unsigned int Id) :
  MGUIElement(Parent, kHorizontalFrame)
{
  // Creates a frame containing a label and an entry-box 
  //
  // Parent:   parent Window, where this frame is contained
  // Label:    text of the label

  m_Id = Id;

  m_Label = Label;

  m_IsEmphasized = Emphasize;
  m_IsEnabled = true;

  m_ComboBoxSize = 100;
  m_ComboBox = 0;
  m_Selected = -1;
}


////////////////////////////////////////////////////////////////////////////////


MGUIEComboBox::~MGUIEComboBox()
{
  // Destruct this instance of MGUIEComboBox
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEComboBox::Create()
{
  // Create the label and the input-field.

  // Label:
  m_TextLabel = new TGLabel(this, new TGString(m_Label));
  if (m_IsEmphasized == true) {
    m_TextLabel->SetTextFont(m_EmphasizedFont);
  }  
  m_TextLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 0, 0);
  AddFrame(m_TextLabel, m_TextLabelLayout);

  m_ComboBoxLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 0, 0, 0, 0);
  m_ComboBox = new TGComboBox(this, m_Id);
  m_ComboBox->Resize(m_ComboBoxSize, 18);
  AddFrame(m_ComboBox, m_ComboBoxLayout);

  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  AddAll();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEComboBox::SetComboBoxSize(int Size)
{
  // Set the size of the combo box

  m_ComboBoxSize = Size;
  if (m_ComboBox != 0) {
    m_ComboBox->Resize(m_ComboBoxSize, 18);
  }
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIEComboBox::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  // cout<<"MGUIEComboBox: Msg: "<<Message<<"!"<<Parameter1<<"!"<<Parameter2<<endl;

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


void MGUIEComboBox::Add(MString Entry, bool IsSelected)
{
  m_Names.push_back(Entry);
  if (IsSelected == true) {
    m_Selected = m_Names.size();
  }
  if (m_ComboBox != 0) {
    m_ComboBox->AddEntry(Entry, m_Names.size());
    if (IsSelected == true) {
      m_ComboBox->Select(m_Selected);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEComboBox::Remove(MString Entry)
{

  if (Entry == "") return;

  // Find entry
  bool Found = false;
  unsigned int n = 0;
  for (n = 0; n < m_Names.size(); ++n) {
    if (m_Names[n] == Entry) {
      Found = true;
      break;
    }
  }

  if (Found == false) return;

  cout<<m_ComboBox->GetSelected()<<"!"<<n+1<<endl;
  if (m_ComboBox->GetSelected() == (int) (n+1)) {
    m_ComboBox->Select(100);
  }

  m_ComboBox->RemoveEntry(n+1);
  m_Names[n] = "";
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEComboBox::AddAll()
{
  if (m_ComboBox != 0) {
    for (unsigned int n = 0; n < m_Names.size(); ++n) {
      if (m_Names[n] != "") {
        m_ComboBox->AddEntry(m_Names[n], n);    
      }
    }
    if (m_Selected != -1) {
      m_ComboBox->Select(m_Selected);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEComboBox::GetSelected()
{
  if (m_ComboBox != 0) {
    return m_Names[m_ComboBox->GetSelected()-1];
  }

  return "";
}


////////////////////////////////////////////////////////////////////////////////



// MGUIEComboBox.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
