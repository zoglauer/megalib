/*
 * MGUILHAlgorithm.cxx
 *
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
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
// MGUILHAlgorithm
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUILHAlgorithm.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MLMLAlgorithms.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUILHAlgorithm)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUILHAlgorithm::MGUILHAlgorithm(const TGWindow* Parent, const TGWindow* Main, 
                                 MGUIDataMimrec* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUILHAlgorithm and bring it to the screen
  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUILHAlgorithm::~MGUILHAlgorithm()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUILHAlgorithm::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Likelihood algorithm");  
  AddSubTitle("Please choose the likelihood algorithm and the stop criterion"); 

  m_RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 20, 0, 0, 0);
  m_RBEntryLayout =  new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsCenterY, 100, 0, 0, 0);
  m_RBFrameLayout =  new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 2, 2);

  // Algorithm
  TGLabel* AlgorithmLabel = new TGLabel(this, "Choose a list-mode maximum-likelihood algorithm");
  TGLayoutHints* AlgorithmLabelLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 20, 20, 0, 0);
  AddFrame(AlgorithmLabel, AlgorithmLabelLayout);

  TGLayoutHints* RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 40, 0, 2, 0);

  m_EM = new TGRadioButton(this, "Classic EM (expectation maximization)", c_ClassicEM);
	m_EM->Associate(this);
  AddFrame(m_EM, RBLayout);


  TGHorizontalFrame* OSEMFrame = new TGHorizontalFrame(this);
  AddFrame(OSEMFrame, m_RBFrameLayout);

  m_OSEM = new TGRadioButton(OSEMFrame, "OS-EM (ordered-subsets EM) with this number of subsets: ", c_OSEM);
	m_OSEM->Associate(this);
  OSEMFrame->AddFrame(m_OSEM, m_RBLayout);

  m_SubSets = new MGUIEEntry(OSEMFrame, "", false, 0, true, 0);
  m_SubSets->SetValue(m_Data->GetOSEMSubSets());
  OSEMFrame->AddFrame(m_SubSets, m_RBEntryLayout);



  // Stop citeria
  // Label:
  m_StopLabel = new TGLabel(this, "Iterate until:");
  m_StopLabelLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 30, 5);
  AddFrame(m_StopLabel, m_StopLabelLayout);


  // ... iterations
  m_RBIterationsFrame = new TGCompositeFrame(this, 100, 100, kHorizontalFrame);
  AddFrame(m_RBIterationsFrame, m_RBFrameLayout);

  m_RBIterations = new TGRadioButton(m_RBIterationsFrame, "... this number of iterations is reached", c_Iterations);
  m_RBIterations->SetState(kButtonUp);
  m_RBIterations->Associate(this);
  m_RBIterationsFrame->AddFrame(m_RBIterations, m_RBLayout);

  m_Iterations = new MGUIEEntry(m_RBIterationsFrame, "", false, 0, true, 0);
  m_Iterations->SetValue(m_Data->GetNIterations());
  m_RBIterationsFrame->AddFrame(m_Iterations, m_RBEntryLayout);

	// ... stop criteria
  m_RBIncreaseFrame = new TGCompositeFrame(this, 100, 100, kHorizontalFrame);
  AddFrame(m_RBIncreaseFrame, m_RBFrameLayout);

  m_RBIncrease = new TGRadioButton(m_RBIncreaseFrame, "... the relative likelihood increase drops below", c_Increase);
  m_RBIncrease->SetState(kButtonUp);
  m_RBIncrease->Associate(this);
  m_RBIncreaseFrame->AddFrame(m_RBIncrease, m_RBLayout);

  m_Increase = new MGUIEEntry(m_RBIncreaseFrame, "", false, 0.0, true, 0.0);
  m_Increase->SetValue((double) m_Data->GetLHIncrease());
  m_RBIncreaseFrame->AddFrame(m_Increase, m_RBEntryLayout);

  if (m_Data->GetLHAlgorithm() == MLMLAlgorithms::c_ClassicEM) {
    ToggleRadioButtons(c_ClassicEM);
  } else if (m_Data->GetLHAlgorithm() == MLMLAlgorithms::c_OSEM) {
    ToggleRadioButtons(c_OSEM);
  }

  if (m_Data->GetLHStopCriteria() == MLMLAlgorithms::c_StopAfterIterations) {
		ToggleRadioButtons(c_Iterations);
  } else if (m_Data->GetLHStopCriteria() == MLMLAlgorithms::c_StopAfterLikelihoodIncrease) {
		ToggleRadioButtons(c_Increase);
	}

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUILHAlgorithm::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
{
  // Process the messages for this window

	bool Status = true;
	
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
      ToggleRadioButtons(Parameter1);
      break;      
    case kCM_BUTTON:
      switch (Parameter1) {
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
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUILHAlgorithm::OnApply()
{
	// The Apply button has been pressed

  if (m_EM->GetState() == kButtonDown) {
    m_Data->SetLHAlgorithm(MLMLAlgorithms::c_ClassicEM);
  } else if (m_OSEM->GetState() == kButtonDown) {
    m_Data->SetLHAlgorithm(MLMLAlgorithms::c_OSEM);
  }

  // We always set the data
  m_Data->SetOSEMSubSets(m_SubSets->GetAsInt());

	if (m_RBIterations->GetState() == kButtonDown) {
		m_Data->SetLHStopCriteria(MLMLAlgorithms::c_StopAfterIterations);
	} else if (m_RBIncrease->GetState() == kButtonDown) {
		m_Data->SetLHStopCriteria(MLMLAlgorithms::c_StopAfterLikelihoodIncrease);
	}

  // We always set the data
  m_Data->SetNIterations(m_Iterations->GetAsInt());
  m_Data->SetLHIncrease(m_Increase->GetAsDouble());

	return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUILHAlgorithm::ToggleRadioButtons(int WidgetID)
{
  // Toggle the radio buttons and the entry fields

  if (WidgetID == m_EM->WidgetId() || 
      WidgetID == m_OSEM->WidgetId()) {
    if (WidgetID == m_EM->WidgetId()) {
      m_EM->SetState(kButtonDown);
      m_OSEM->SetState(kButtonUp);
      m_SubSets->SetEnabled(false);
    } else if (WidgetID == m_OSEM->WidgetId()) {
      m_EM->SetState(kButtonUp);
      m_OSEM->SetState(kButtonDown);
      m_SubSets->SetEnabled(true);
    }
  } else {
    if (WidgetID == m_RBIncrease->WidgetId()) {
      m_RBIncrease->SetState(kButtonUp);
      mgui<<"This option is currently disabled"<<info;
//       m_RBIncrease->SetState(kButtonDown);
//       m_Increase->SetEnabled(true);
//       m_RBIterations->SetState(kButtonUp);
//       m_Iterations->SetEnabled(false);
    } else if (WidgetID == m_RBIterations->WidgetId()) {
      m_RBIncrease->SetState(kButtonUp);
      m_Increase->SetEnabled(false);
      m_RBIterations->SetState(kButtonDown);
      m_Iterations->SetEnabled(true);
    }
  } 
}


// MGUILHAlgorithm: the end...
////////////////////////////////////////////////////////////////////////////////
