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

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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
  m_TextButton->SetStyle(1);
  m_TextButton->SetTopMargin(-2);
  m_TextButton->SetBottomMargin(-2);
  AddFrame(m_TextButton, ButtonLayout);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MGUIEReadOutElement::~MGUIEReadOutElement()
{
  // Delete this instance of MGUIEReadOutElement
}


// MGUIEReadOutElement.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
