/*
* MGUIEReadOutElementView.cxx
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


// Include the header:
#include "MGUIEReadOutElementView.h"

// Standard libs:
#include "MStreams.h"

// ROOT libs:
#include <TGMsgBox.h>
#include <TObjString.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEReadOutElementView)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Standard constructor
MGUIEReadOutElementView::MGUIEReadOutElementView(const TGWindow* Parent) :
  TGCanvas(Parent, 50, 50, 0)
{
  m_Parent = (TGWindow *) Parent;

  // Since we do not create the element in the constructor, 
  // we have initialze some pointers:
  m_Container = new TGCompositeFrame(GetViewPort(), 50, 50);
  SetContainer(m_Container);

  m_Associate = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MGUIEReadOutElementView::~MGUIEReadOutElementView()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear all the read-out elements
void MGUIEReadOutElementView::Clear()
{
  m_ROEs.clear(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Create or re-create the view
void MGUIEReadOutElementView::Create()
{
  // Create the GUI element
  
  TGCompositeFrame
  
  UnmapWindow();
  m_Container->RemoveAll();
  
  for (unsigned int c = 0; c < m_ROEButtons.size(); ++c) {
    delete m_ROEButtons[c];
  }
  m_ROEButtons.clear();
  
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 15, 0, 0);
  for (unsigned int c = 0; c < m_ROEs.size(); ++c) {
    MGUIEReadOutElement* ROEButton = new MGUIEReadOutElement(m_Container, *m_ROEs[c], m_ROEIDs[c]);
    ROEButton->Associate(m_Associate);
    m_Container->AddFrame(ROEButton, ButtonLayout);
    m_ROEButtons.push_back(ROEButton);
  }

  // Give this element the default size of its content:
  Resize(m_Container->GetDefaultWidth(), m_Container->GetDefaultHeight()); 
  
  MapSubwindows();
  MapWindow();  
  Layout();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


//! Process the messages for this UI, mainly the scollbar moves:
bool MGUIEReadOutElementView::ProcessMessage(long Message, long Parameter1, 
                                  long Parameter2)
{
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
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


void MGUIEReadOutElementView::Associate(TGWindow* Associate)
{
  m_Associate = Associate;
}


// MGUIEReadOutElementView.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
