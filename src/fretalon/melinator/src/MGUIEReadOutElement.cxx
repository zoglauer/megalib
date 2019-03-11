/*
 * MGUIEReadOutElement.cxx
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
// MGUIEReadOutElement
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEReadOutElement.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TColor.h"

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEReadOutElement)
#endif


//////////////////////////////////////////////////////////////////////////////


//! Standard constructor
MGUIEReadOutElement::MGUIEReadOutElement(TGCompositeFrame* Parent, const MReadOutElement& ROE, unsigned int ID)
  : TGTransientFrame(Parent), m_ROE(ROE), m_ID(ID)
{
  // Construct an instance of MGUIEReadOutElement

  // Use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_TextButton = new TGTextButton(this, m_ROE.ToString(), m_ID);
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 1, 1);
  // m_TextButton->SetStyle(1);
  // m_TextButton->ChangeOptions(m_TextButton->GetOptions() & ~kRaisedFrame);
  m_TextButton->SetTopMargin(-2);
  m_TextButton->SetBottomMargin(-2);
  m_TextButton->SetLeftMargin(5);
  m_TextButton->SetRightMargin(5);
  
  m_DefaultBackgroundColor = m_TextButton->GetDefaultFrameBackground();
  // SetQuality(0);

  
  AddFrame(m_TextButton, ButtonLayout);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MGUIEReadOutElement::~MGUIEReadOutElement()
{
  // Delete this instance of MGUIEReadOutElement
}


////////////////////////////////////////////////////////////////////////////////


//! Set a Quality factor representing the color of the elemnt
void MGUIEReadOutElement::SetQuality(double Quality) 
{ 
  m_Quality = Quality; 
  
  if (m_Quality == 0) {
    m_TextButton->ChangeBackground(m_DefaultBackgroundColor);    
  } else if (m_Quality == 1) {
    m_TextButton->ChangeBackground(gROOT->GetColor(kGreen+1)->GetPixel());
  } else if (m_Quality == 2) {
    m_TextButton->ChangeBackground(gROOT->GetColor(kYellow)->GetPixel());
  } else if (m_Quality == 3) {
    m_TextButton->ChangeBackground(gROOT->GetColor(kOrange+1)->GetPixel());
  } else {
    m_TextButton->ChangeBackground(gROOT->GetColor(kRed)->GetPixel());
  }
}


// MGUIEReadOutElement.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
