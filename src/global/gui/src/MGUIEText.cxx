/*
 * MGUIEText.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
