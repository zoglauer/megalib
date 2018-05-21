/*
 * MGUIEText.cxx
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
// MGUIEText
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEText.h"

// Standard libs:

// ROOT libs:
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEText)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEText::MGUIEText(const TGWindow* Parent, MString Text, 
                     unsigned int Alignment, bool IsBold, bool IsItalic)
  : MGUIElement(Parent, kVerticalFrame)
{
  // Construct an instance of MGUIEText and bring it to the screen

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_Text = Text;
  m_Alignment = Alignment;
  m_IsBold = IsBold;
  m_IsItalic = IsItalic;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEText::~MGUIEText()
{
  // kDeepCleanup is activated so only delete the array
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEText::SetText(MString Text)
{
  // Set th etxt of this GUI element
  
  m_Text = Text;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEText::SetAlignment(unsigned int Alignment)
{
  // Set the alignment of this GUI element
  
  m_Alignment = Alignment;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEText::Create()
{
  // Create the GUI element

  MString SubString, Text = m_Text;

  TGLayoutHints* LabelLayout = 0;
  if (m_Alignment == c_Left) {
    LabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 1, 1);
  } else if (m_Alignment == c_Right) {
    LabelLayout = new TGLayoutHints(kLHintsRight | kLHintsTop, 0, 0, 1, 1);
  } else {
    LabelLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 0, 0, 1, 1);
  }    

  // Label:
  FontStruct_t LabelFont;
  if (m_IsBold == true) {
    if (m_IsItalic == true) {
      LabelFont = MGUIDefaults::GetInstance()->GetItalicBoldFont()->GetFontStruct();
    } else {
      LabelFont = MGUIDefaults::GetInstance()->GetNormalBoldFont()->GetFontStruct();
    }
  } else {
    if (m_IsItalic == true) {
      LabelFont = MGUIDefaults::GetInstance()->GetItalicMediumFont()->GetFontStruct();
    } else {
      LabelFont = MGUIDefaults::GetInstance()->GetNormalMediumFont()->GetFontStruct();
    }
  }

  TGLabel* Label = 0;
  while (Text.Contains("\n") == true) {
    SubString = Text;
    
    Label = new TGLabel(this, new TGString(SubString.Remove(SubString.First('\n'))));
    Label->SetTextFont(LabelFont);
    AddFrame(Label, LabelLayout);
    
    Text.Replace(0, Text.First('\n')+1, "");
  }
    
  Label = new TGLabel(this, new TGString(Text));
  Label->SetTextFont(LabelFont);
  AddFrame(Label, LabelLayout);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


// MGUIEText: the end...
////////////////////////////////////////////////////////////////////////////////
