/*
 * MGUIEEntryList.cxx
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
// MGUIEEntryList
//
// This class implements a list of check buttons
//
//
// Usage example (when used in a class like MGUIAnalysisModes):
//
// MGUIEEntryList *m_CBList;
//
// m_CBList = new MGUIEEntryList(this, "Coordinate-System:");
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
#include "MGUIEEntryList.h"

// Standard libs:
#include "MStreams.h"
#include <MStreams.h>

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MGUIEEntry.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEEntryList)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEEntryList::MGUIEEntryList(const TGWindow* Parent, MString Label, int Mode) :
  MGUIElement(Parent)
{
  // Creates a frame containing a label and several checkbuttons:
  //
  // Parent:    parent frame
  // Label:     label, decribing the content of the check buttons

  m_Label = Label;

  m_EntryList = new TObjArray();

  m_Mode = Mode;
  m_Size = m_FontScaler*85;

  m_TextLabel = 0;
  m_TextLabelLayout = 0;

  m_EntryLayout = 0;

  if (m_Mode == c_SingleLine) {
    ChangeOptions(kHorizontalFrame);
  } else {
    ChangeOptions(kVerticalFrame);
  }
}


////////////////////////////////////////////////////////////////////////////////


MGUIEEntryList::~MGUIEEntryList()
{
  // default destructor

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;

    m_EntryList->Delete();
    delete m_EntryLayout;
  }
  delete m_EntryList;     
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::Create()
{
  // Create the GUI element

  int i;

  // The label:
  if (m_Label.IsEmpty() == false) {
    m_TextLabel = new TGLabel(this, new TGString(m_Label));
    m_TextLabel->SetWrapLength(m_WrapLength);
    m_TextLabelLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 2, 0);
    AddFrame(m_TextLabel, m_TextLabelLayout);
  }
  
  // Create the list of text entries:
  if (m_Mode == c_SingleLine) {
    m_EntryLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, m_FontScaler*5, 0, 0, 0);
    for (i = m_EntryList->GetLast(); i >= 0; i--) {
      AddFrame((MGUIEEntry *) m_EntryList->At(i), m_EntryLayout);
    }
  } else {
    m_EntryLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, m_FontScaler*20, 0, 2, 0);
    for (i = 0; i < m_EntryList->GetLast()+1; i++) {
      AddFrame((MGUIEEntry *) m_EntryList->At(i), m_EntryLayout);
    }
  }



  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntryList::IsModified()
{
  //! Return true if the content has been modified 

  for (int i = m_EntryList->GetLast(); i >= 0; i--) {
    if (((MGUIEEntry *) m_EntryList->At(i))->IsModified() == true) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetEnabled(bool Flag)
{
  // 

  m_IsEnabled = Flag;

  for (int i = m_EntryList->GetLast(); i >= 0; i--) {
    ((MGUIEEntry *) m_EntryList->At(i))->SetEnabled(Flag);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntryList::ProcessMessage(long Message, long Parameter1, 
                                   long Parameter2)
{
  // Process the messages for this application, mainly the scollbar moves:

//   switch (GET_MSG(Message)) {
//   case kC_VSCROLL:
//     switch (GET_SUBMSG(Message)) {
//     case kSB_SLIDERPOS:
//       // Scroll the viewport to the new position (vertical)
//       GetViewPort()->SetVPos(-Parameter1);
//       break;
//     default:
//       break;
//     }
//     break;
//   case kC_HSCROLL:
//     switch (GET_SUBMSG(Message)) {
//     case kSB_SLIDERPOS:
//       // Scroll the viewport to the new position (horizontal)
//       GetViewPort()->SetHPos(-Parameter1);
//       break;
//     default:
//       break;
//     }
//     break;
//   default:
//     break;
//   }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::Add(MString Label, long Value, bool Limits, long Min, long Max)
{
  MGUIEEntry* Entry = new MGUIEEntry(this, Label, false, Value, Limits, Min, Max);
  Entry->SetEntryFieldSize(m_Size);

  m_EntryList->AddLast(Entry);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::Add(MString Label, double Value, bool Limits, double Min, double Max)
{
  MGUIEEntry* Entry = new MGUIEEntry(this, Label, false, Value, Limits, Min, Max);
  Entry->SetEntryFieldSize(m_Size);

  m_EntryList->AddLast(Entry);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::Add(MString Label, MString Value)
{
  MGUIEEntry* Entry = new MGUIEEntry(this, Label, false, Value);
  Entry->SetEntryFieldSize(m_Size);

  m_EntryList->AddLast(Entry);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetEntryFieldSize(int Size)
{
  // Set the size of the entry field

  m_Size = Size;

  for (int e = 0; e < GetNEntrys(); e++) {
    ((MGUIEEntry *) (m_EntryList->At(e)))->SetEntryFieldSize(Size);
  }
}


////////////////////////////////////////////////////////////////////////////////


int MGUIEEntryList::GetNEntrys()
{
  return m_EntryList->GetLast() + 1;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntryList::IsInt(int Entry, long Min, long Max)
{
  //

  if (Entry == -1) {
    // Check all entries:
    for (int e = 0; e < GetNEntrys(); e++) {
      if (((MGUIEEntry *) (m_EntryList->At(e)))->IsInt(Min, Max) == false) {
        return false;
      }
    }
    return true;
  }

  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("bool MGUIEEntryList::IsInt(int Entry, long Min, long Max)",
          "Index out of bounds!");
    return false;
  }
  
  return ((MGUIEEntry *) (m_EntryList->At(Entry)))->IsInt(Min, Max);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEEntryList::IsDouble(int Entry, double Min, double Max)
{
  //

  if (Entry == -1) {
    // Check all entries:
    for (int e = 0; e < GetNEntrys(); e++) {
      if (((MGUIEEntry *) (m_EntryList->At(e)))->IsDouble(Min, Max) == false) {
        return false;
      }
    }
    return true;
  }

  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("bool MGUIEEntryList::IsDouble(int Entry, double Min, double Max)",
          "Index out of bounds!");
    return false;
  }
  
  return ((MGUIEEntry *) (m_EntryList->At(Entry)))->IsDouble(Min, Max);
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEEntryList::GetAsString(int Entry)
{
  //

  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("MString MGUIEEntryList::GetAsString(int Entry)",
          "Index out of bounds!");
    return "";
  }

  return ((MGUIEEntry *) (m_EntryList->At(Entry)))->GetAsString();
}


////////////////////////////////////////////////////////////////////////////////


long MGUIEEntryList::GetAsInt(int Entry)
{
  //
  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("long MGUIEEntryList::GetAsInt(int Entry)",
          "Index out of bounds!");
    return 0;
  }

  return ((MGUIEEntry *) (m_EntryList->At(Entry)))->GetAsInt();
}


////////////////////////////////////////////////////////////////////////////////


double MGUIEEntryList::GetAsDouble(int Entry)
{
  //

  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("double MGUIEEntryList::GetAsDouble(int Entry)",
          "Index out of bounds!");
    return 0.0;
  }

  return ((MGUIEEntry *) (m_EntryList->At(Entry)))->GetAsDouble();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetValue(int Entry, double Value)
{
  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("MGUIEEntryList::SetValue(double Value)",
          "Index out of bounds!");
    return;
  }

  ((MGUIEEntry *) (m_EntryList->At(Entry)))->SetValue(Value);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetValue(int Entry, long Value)
{
  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("MGUIEEntryList::SetValue(long Value)",
          "Index out of bounds!");
    return;
  }

  ((MGUIEEntry *) (m_EntryList->At(Entry)))->SetValue(Value);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetValue(int Entry, MString Value)
{
  if (Entry < 0 || Entry >= GetNEntrys()) {
    Error("MGUIEEntryList::SetValue(MString Value)",
          "Index out of bounds!");
    return;
  }

  ((MGUIEEntry *) (m_EntryList->At(Entry)))->SetValue(Value);
}

////////////////////////////////////////////////////////////////////////////////


void MGUIEEntryList::SetWrapLength(int WrapLength)
{
  // Set the wrap length of all text

  MGUIElement::SetWrapLength(WrapLength);

  if (m_TextLabel != 0) {
    m_TextLabel->SetWrapLength(m_WrapLength);
  }

}


////////////////////////////////////////////////////////////////////////////////



// MGUIEEntryList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
