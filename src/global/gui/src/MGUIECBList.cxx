/*
 * MGUIECBList.cxx
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
// MGUIECBList
//
// This class implements a list of check buttons
//
//
// Usage example (when used in a class like MGUIAnalysisModes):
//
// MGUIECBList *m_CBList;
//
// m_CBList = new MGUIECBList(this, "Coordinate-System:");
// m_CBList->Add("Spherical");
// m_CBList->Add("Cartesean 2D");
// m_CBList->Add("Cartesean 3D");
// m_CBList->SetSelectedBitEncoded(2); // Button 2 is activated
// m_CBList->Create();
// m_CBListLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | 
//                                    kLHintsExpandX, 20, 20, 10, 20);
//  
// AddFrame(m_CBList, m_CBListLayout);
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIECBList.h"

// Standard libs:
#include "MStreams.h"

// ROOT libs:
#include <TGMsgBox.h>
#include <TObjString.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIECBList)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIECBList::MGUIECBList(const TGWindow *Parent, MString Label, bool IsMultiple) :
  TGCanvas(Parent, 50, 50, 0)
{
  // Creates a frame containing a label and several checkbuttons:
  //
  // Parent:    parent frame
  // Label:     label, decribing the content of the check buttons

  m_Parent = (TGWindow *) Parent;
  m_Label = Label;

  m_IsMultiple = IsMultiple;
  m_CBList = new TObjArray();
  m_NamesList = new TObjArray();

  // Since we do not create the element in the constructor, 
  // we have initialze some pointers:
  SetContainer(new TGCompositeFrame(GetViewPort(), 50, 50));

  m_CBLayout = 0;
  m_TextLabel = 0;
  m_TextLabelLayout = 0;

  m_IsEnabled = true;
  m_WrapLength = 800;

  m_Associate = 0;
}


////////////////////////////////////////////////////////////////////////////////


MGUIECBList::~MGUIECBList()
{
  // default destructor

  m_Parent = 0;

  if (MustCleanup() == kNoCleanup) {
    m_CBList->Delete();
    m_NamesList->Delete();

    delete m_CBLayout;
  
    delete m_TextLabel;
    delete m_TextLabelLayout;
  }
  delete m_CBList;
  delete m_NamesList;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::Create()
{
  // Create the GUI element

  int i;

  // The label:
  if (m_Label.IsEmpty() == false) {
    m_TextLabel = new TGLabel(GetContainer(), new TGString(m_Label));
    m_TextLabel->SetWrapLength(m_WrapLength);
    m_TextLabelLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 0, 0);
    AddFrame(m_TextLabel, m_TextLabelLayout);
  }
  
  // The checkbuttons:
  m_CBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 2, 0);
  for (i = 0; i < m_CBList->GetLast()+1; i++) {
    ((TGCheckButton *) m_CBList->At(i))->SetWrapLength(m_WrapLength);
    AddFrame((TGCheckButton *) m_CBList->At(i), m_CBLayout);
  }

  // Give this element the default size of its content:
  Resize(GetContainer()->GetDefaultWidth(), GetContainer()->GetDefaultHeight()); 
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIECBList::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

  int i;
  TGCheckButton *CB; 

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_CHECKBUTTON:
      if (m_IsMultiple == true) break;
      if (Parameter1 >= 0 && Parameter1 <= m_CBList->GetLast()) {
        for (i = 0; i <= m_CBList->GetLast(); i++) {
          CB = (TGCheckButton *) m_CBList->At(i);
          if (CB->GetState() == kButtonDisabled) continue;

          if (CB->WidgetId() != Parameter1) {
            CB->SetState(kButtonUp);
          } else {
            CB->SetState(kButtonDown);
          }
        }
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
      GetViewPort()->SetVPos(-Parameter1);
      break;
    default:
      break;
    }
    break;
  case kC_HSCROLL:
    switch (GET_SUBMSG(Message)) {
    case kSB_SLIDERPOS:
      // Scroll the viewport to the new position (horizontal)
      GetViewPort()->SetHPos(-Parameter1);
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


void MGUIECBList::Associate(TGWindow* Associate)
{
  m_Associate = Associate;
  
  for (int i = 0; i < m_CBList->GetLast()+1; i++) {
    ((TGCheckButton *) (m_CBList->At(i)))->Associate(Associate);
  }
}


////////////////////////////////////////////////////////////////////////////////


TArrayI MGUIECBList::GetSelected()
{
  // Get an array containing the selected checkbuttons:
  // 0: not selcted
  // 1: selected

  int i;
  TArrayI Selected(m_CBList->GetLast()+1);
  for (i = 0; i < m_CBList->GetLast()+1; i++) {
    Selected[i] = (((TGCheckButton *) (m_CBList->At(i)))->GetState() == kButtonUp) ? 0 : 1; 
  }

  return Selected;
}


////////////////////////////////////////////////////////////////////////////////


int MGUIECBList::GetSelected(int i)
{
  // Return 1, if position i exists and is selected, otherwise 0

  if (i >= 0 && i < m_CBList->GetLast()+1) {
    return (((TGCheckButton *) (m_CBList->At(i)))->GetState() == kButtonUp) ? 0 : 1;
  }

  return 0;
}



////////////////////////////////////////////////////////////////////////////////


MString MGUIECBList::GetName(int i) const
{
  // Return the label name at position i

  if (i >= 0 && i < m_CBList->GetLast()+1) {
    return ((TObjString*) m_NamesList->At(i))->GetString().Data();
  }

  return "";
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIECBList::IsSelected(int i)
{
  // Return true, if position i exists and is selected, otherwise false

  if (i >= 0 && i < m_CBList->GetLast()+1) {
    return (((TGCheckButton *) (m_CBList->At(i)))->GetState() == kButtonUp) ? false : true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SetSelected(int i, bool Selected)
{
  // Set button i as selected (true) or not selcted (false)
  
  if (i >= 0 && i < m_CBList->GetLast()+1) {
    if (Selected == true) {
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonDown);
    } else {
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonUp);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SetSelectedBitEncoded(int Selected)
{
  // Set the selected buttons as an integer
  // Selected is an integer, if one spilts it up into bits, it contains 0 and 1,
  // Now 0 mains selected and 1 mains not selected:
  // The first checkbutton is the last bit
 
  if (m_CBList->GetLast()+1 == 0) return;

  int i = m_CBList->GetLast();
  int L = (int) pow(2.0, m_CBList->GetLast());
  
  do {
    if (Selected%L != Selected) {
      Selected -= L;
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonDown);
    } else {
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonUp);
    }
    L /= 2;
    i--;
  } while (L >= 1);
}


////////////////////////////////////////////////////////////////////////////////


int MGUIECBList::GetSelectedBitEncoded()
{
  // Get the selected buttons as an integer
  // If one spilts the returned interger up into bits, it contains 0 and 1,
  // Now 0 mains selected and 1 mains not selected:
  // The first checkbutton is the last bit

  int i;
  int pow = 1;
  int bMode = 0;
  TArrayI aMode = GetSelected();

  // Convert the array from bits to an integer:
  for (i = 1; i <= aMode.GetSize(); i++) { 
    bMode += aMode[i-1]*pow;
    pow *= 2;
  }

  return bMode;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::Add(MString CBLabel, int Selected)
{
  // Add a Checkbutton to the GUI element
  
  TGCheckButton *CB = 
    new TGCheckButton(GetContainer(), CBLabel, m_CBList->GetLast()+1);
  (Selected == 0) ? CB->SetState(kButtonUp) : CB->SetState(kButtonDown);
  CB->Associate(this);
  if (m_Associate != 0) {
    CB->Associate(m_Associate);
  }
  CB->SetWrapLength(m_WrapLength);
  m_CBList->AddLast(CB);

  m_NamesList->AddLast(new TObjString(CBLabel));
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::Add(MString CBLabel, bool Selected)
{
  // Add a Checkbutton to the GUI element
  
  TGCheckButton *CB = new TGCheckButton(GetContainer(), CBLabel, m_CBList->GetLast()+1);
  (Selected == false) ? CB->SetState(kButtonUp) : CB->SetState(kButtonDown);
  CB->Associate(this);
  if (m_Associate != 0) {
    CB->Associate(m_Associate);
  }
  CB->SetWrapLength(m_WrapLength);
  m_CBList->AddLast(CB);

  m_NamesList->AddLast(new TObjString(CBLabel));
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SetEnabled(bool flag)
{
  m_IsEnabled = flag;
  
  for (int i = 0; i < m_CBList->GetLast()+1; i++) {
    if (m_IsEnabled == true) {
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonUp); 
    } else {
      ((TGCheckButton *) (m_CBList->At(i)))->SetState(kButtonDisabled); 
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SetMultiple(bool IsMultiple)
{
  // True if multiple selctions are allowed 

  m_IsMultiple = IsMultiple;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SelectAll()
{
  // Set all check button to selected

  if (m_IsMultiple == false) {
    return;
  }

  int i;
  for (i = 0; i < m_CBList->GetLast()+1; i++) {
    ((TGCheckButton *) m_CBList->At(i))->SetState(kButtonDown);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SelectNone()
{
  // Set all check buttons to not selected

  int i;
  for (i = 0; i < m_CBList->GetLast()+1; i++) {
    ((TGCheckButton *) m_CBList->At(i))->SetState(kButtonUp);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIECBList::SetWrapLength(int WrapLength)
{
  // Set the wrap length of all text

  m_WrapLength = WrapLength;

  for (int i = 0; i < m_CBList->GetLast()+1; i++) {
    ((TGCheckButton *) (m_CBList->At(i)))->SetWrapLength(m_WrapLength);
  }

  if (m_TextLabel != 0) {
    m_TextLabel->SetWrapLength(m_WrapLength);
  }

}


// MGUIECBList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
