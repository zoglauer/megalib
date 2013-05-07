/*
 * MGUISivanTracking.cxx
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
#include "MGUISivanTracking.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUISivanTracking)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISivanTracking::MGUISivanTracking(const TGWindow* Parent, const TGWindow* Main, 
                                     MSettingsSivan* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  m_Realism = m_GUIData->GetRealism();

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUISivanTracking::~MGUISivanTracking()
{
  // standard destructor

  delete m_RealRB[0];
  delete m_RealRB[1];
  delete m_RealRB[2];
    
  delete m_RBLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUISivanTracking::Create()
{
  // Pop up the dialog window

  SetWindowName("Raw data preparation");  

  AddSubTitle("Choose the degree of realism in your data");


  // Realism
  m_RealRB[0] = new TGRadioButton(this, "Ideal data", 30);
  m_RealRB[1] = new TGRadioButton(this, "Half noised data (positions and track-direction)", 31);
  m_RealRB[2] = new TGRadioButton(this, "Fully noised data (positions and energies)", 32);


  m_RBLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                                 35, 2, 5, 0);
  for (int i = 0; i <= 2; i++) {
    AddFrame(m_RealRB[i], m_RBLayout);
    m_RealRB[i]->Associate(this);
  }
  m_RealRB[m_Realism]->SetState(kButtonDown);


  AddButtons();

  PositionWindow(380, 250);

  MapSubwindows();
  MapWindow(); 
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISivanTracking::ProcessMessage(long msg, long param1, long)
{
  // We handle two messages the Ok- and the Cancel-Button 

	bool Status = true;

  int i;
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_RADIOBUTTON:
      if (param1 >= 30 && param1 <= 32) {
        for (i = 0; i <= 2; i++) {
          if (m_RealRB[i]->WidgetId() != param1) {
            m_RealRB[i]->SetState(kButtonUp);
          } else {
            m_RealRB[i]->SetState(kButtonDown);
            m_Realism = i;
          }
        }
      }
    case kCM_BUTTON:
      switch(param1) {
      case e_Ok:
				Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
				Status = OnApply();
        break;

      default:
        break;
      }
    default:
      break;
    }
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISivanTracking::OnApply()
{
	// The Apply button has been pressed

  m_GUIData->SetRealism(m_Realism);

	return true;
}


// MGUISivanTracking: the end...
////////////////////////////////////////////////////////////////////////////////
