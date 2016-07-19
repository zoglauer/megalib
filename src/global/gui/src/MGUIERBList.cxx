/*
 * MGUIERBList.cxx
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
// MGUIERBList
//
// This class implements a list of radio buttons
//
//
// Usage example (when used in a class like MGUIDummy):
//
// MGUIERBList *m_System;
//
// m_System = new MGUIERBList(this, "Coordinate-System:");
// m_System->Add("Spherical");
// m_System->Add("Cartesean 2D");
// m_System->Add("Cartesean 3D");
// m_System->SetSelected(m_GUIData->GetCoordinateSystem());
// m_System->Create();
// m_SystemLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | 
//                                    kLHintsExpandX, 20, 20, 10, 20);
//  
// AddFrame(m_System, m_SystemLayout);
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIERBList.h"

// Standard libs:
#include "MStreams.h"

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIERBList)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIERBList::MGUIERBList(const TGWindow* Parent, MString Label, 
                         bool Emphasize, int Mode) :
  MGUIElement(Parent)
{
  // Creates a frame containing a label a slider and its min, max and current value:

  m_IsEmphasized = Emphasize;
  m_Label = Label;
  m_Mode = Mode;

  if (m_Mode == c_SingleLine) {
    ChangeOptions(kHorizontalFrame);
  }

  m_RBList = new TObjArray();
  m_Selected = -1;

  m_TextLabel = 0;
  m_TextLabelLayout = 0;

  m_Associate = 0;
}


////////////////////////////////////////////////////////////////////////////////


MGUIERBList::~MGUIERBList()
{
  // default destructor

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;

    m_RBList->Delete();
    delete m_RBLayout;
  }

  delete m_RBList;
}

////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::Create()
{
  // Create the GUI element

  // The label:
  if (m_Label.IsEmpty() == false) {
    m_TextLabel = new TGLabel(this, new TGString(m_Label));
    if (m_IsEmphasized == true) {
      m_TextLabel->SetTextFont(m_EmphasizedFont);
    }    
    m_TextLabel->SetWrapLength(m_WrapLength);
    m_TextLabelLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 0, 0);
    AddFrame(m_TextLabel, m_TextLabelLayout);
  }

  m_RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 0, 2, 0);

  int i;
  for (i = 0; i < m_RBList->GetLast()+1; i++) {
    if (m_Selected == i) {
      ((TGRadioButton *) m_RBList->At(i))->SetState(kButtonDown);
    } 
    ((TGRadioButton *) m_RBList->At(i))->SetWrapLength(m_WrapLength);
    AddFrame((TGRadioButton *) m_RBList->At(i), m_RBLayout);
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIERBList::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application
  int i;
  TGRadioButton *RB; 

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
      if (Parameter1 >= 0 && Parameter1 <= m_RBList->GetLast()) {
        m_Selected = Parameter1;
        for (i = 0; i <= m_RBList->GetLast(); i++) {
          RB = (TGRadioButton *) m_RBList->At(i);
          if (RB->GetState() == kButtonDisabled) continue;

          if (RB->WidgetId() != Parameter1) {
            RB->SetState(kButtonUp);
          } else {
            RB->SetState(kButtonDown);
          }
        }
      }
      break;

    default:
      break;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::Associate(TGWindow* Associate)
{
  m_Associate = Associate;
  
  for (int i = 0; i < m_RBList->GetLast()+1; i++) {
    ((TGRadioButton *) (m_RBList->At(i)))->Associate(Associate);
  }
}

////////////////////////////////////////////////////////////////////////////////


int MGUIERBList::GetSelected()
{
  // Get the selected radiobutton

  return m_Selected;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::SetSelected(int Selected)
{
  // Set the selcted radiobutton

  m_Selected = Selected;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::Add(MString RBLabel, int Selected)
{
  // Add a Radiobutton to the GUI element

  TGRadioButton* RB = new TGRadioButton(this, RBLabel, m_RBList->GetLast()+1);
  RB->Associate(this);
  if (m_Associate != 0) {
    RB->Associate(m_Associate);
  }
  if (Selected == 1) {
    SetSelected(m_RBList->GetLast()-1);
  }
  RB->SetWrapLength(m_WrapLength);
  m_RBList->AddLast(RB);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::SetEnabled(bool flag)
{
  m_IsEnabled = flag;
  
  for (int i = 0; i < m_RBList->GetLast()+1; i++) {
    if (m_IsEnabled == true) {
      if (m_Selected == i) {
        ((TGRadioButton *) (m_RBList->At(i)))->SetState(kButtonDown); 
      } else {
        ((TGRadioButton *) (m_RBList->At(i)))->SetState(kButtonUp); 
      }
    } else {
      ((TGRadioButton *) (m_RBList->At(i)))->SetState(kButtonDisabled); 
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIERBList::SetWrapLength(int WrapLength)
{
  // Set the wrap length of all text

  MGUIElement::SetWrapLength(WrapLength);

  for (int i = 0; i < m_RBList->GetLast()+1; i++) {
    ((TGRadioButton *) (m_RBList->At(i)))->SetWrapLength(m_WrapLength);
  }

  if (m_TextLabel != 0) {
    m_TextLabel->SetWrapLength(m_WrapLength);
  }

}


// MGUIERBList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
