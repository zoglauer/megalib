/*
 * MGUIAnimation.cxx
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
// MGUIAnimation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIAnimation.h"

// Standard libs:

// ROOT libs:
#include "TGNumberEntry.h"

// MEGAlib libs:
#include "MImager.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIAnimation)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIAnimation::MGUIAnimation(const TGWindow* Parent, const TGWindow* Main, 
                             MSettingsImaging* Data, bool& OkPressed)
  : MGUIDialog(Parent, Main), m_OkPressed(OkPressed)
{
  // standard constructor

  m_GUIData = Data;
  m_OkPressed = false;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIAnimation::~MGUIAnimation()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAnimation::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Animation");  

  AddSubTitle("Create an animated gif from either backprojections or the iteration process"); 
  
  TGLayoutHints* FrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 40, 20, 2, 2);
  TGLayoutHints* MainLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 20, 20, 10, 2);
  TGLayoutHints* RadioButtonLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 0, 0, 0, 0);

  TGLabel* IterationsLabel = new TGLabel(this, "Create an animated gif from ...");
  AddFrame(IterationsLabel, MainLayout);
  
  TGHorizontalFrame* IterationsFrame = new TGHorizontalFrame(this);
  AddFrame(IterationsFrame, FrameLayout);
  
  m_Iterations = new TGRadioButton(IterationsFrame, "Iterations", c_Iterations);
  m_Iterations->Associate(this);
  IterationsFrame->AddFrame(m_Iterations, RadioButtonLayout);
  
  TGHorizontalFrame* BackprojectionsFrame = new TGHorizontalFrame(this);
  AddFrame(BackprojectionsFrame, FrameLayout);

  m_Backprojections = new TGRadioButton(BackprojectionsFrame, "Backprojections and create a frame after each ", c_Backprojections);
  m_Backprojections->Associate(this);
  BackprojectionsFrame->AddFrame(m_Backprojections, RadioButtonLayout);
  
  m_SnapshotTime = new TGNumberEntry(BackprojectionsFrame, m_GUIData->GetAnimationFrameTime());
  BackprojectionsFrame->AddFrame(m_SnapshotTime, RadioButtonLayout);

  if (m_GUIData->GetAnimationMode() == MImager::c_AnimateIterations) {
    EnableRadioButton(c_Iterations);
  } else if (m_GUIData->GetAnimationMode() == MImager::c_AnimateBackprojections) {
    EnableRadioButton(c_Backprojections);
  }
  
  TGLabel* BackprojectionPostLabel = new TGLabel(BackprojectionsFrame, " seconds of observation time.");
  BackprojectionsFrame->AddFrame(BackprojectionPostLabel, RadioButtonLayout);
  
  
  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 2);
  m_FileSelector = new MGUIEFileSelector(this, "Store the animated gif in the following file:", m_GUIData->GetAnimationFileName());
  AddFrame(m_FileSelector, FileSelectorLayout);

  AddButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  fClient->WaitFor(this);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIAnimation::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
      EnableRadioButton(Parameter1);
      break;
      
    default:
      break;
    }
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

  return Status;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAnimation::EnableRadioButton(long Parameter1)
{
  
  if (Parameter1 == c_Iterations) {
    m_Iterations->SetState(kButtonDown);
    m_Backprojections->SetState(kButtonUp);
    m_SnapshotTime->SetState(false);
  } else {
    m_Iterations->SetState(kButtonUp);
    m_Backprojections->SetState(kButtonDown);
    m_SnapshotTime->SetState(true);    
  }
} 
 

////////////////////////////////////////////////////////////////////////////////


bool MGUIAnimation::OnApply()
{
  // The Apply button has been pressed

  if (m_Iterations->GetState() == kButtonDown) {
    if (m_GUIData->GetAnimationMode() != MImager::c_AnimateIterations) m_GUIData->SetAnimationMode(MImager::c_AnimateIterations);
  } else if (m_Backprojections->GetState() == kButtonDown) {
    if (m_GUIData->GetAnimationMode() != MImager::c_AnimateBackprojections) m_GUIData->SetAnimationMode(MImager::c_AnimateBackprojections);
  }

  if (m_SnapshotTime->GetNumber() != m_GUIData->GetAnimationFrameTime()) {
    m_GUIData->SetAnimationFrameTime(m_SnapshotTime->GetNumber());
  }
  
  if (m_FileSelector->GetFileName() != m_GUIData->GetAnimationFileName()) {
    m_GUIData->SetAnimationFileName(m_FileSelector->GetFileName());
  }
  
  m_OkPressed = true;

  return true;
}


// MGUIAnimation: the end...
////////////////////////////////////////////////////////////////////////////////
