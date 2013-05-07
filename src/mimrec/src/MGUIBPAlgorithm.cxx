/*
 * MGUIBPAlgorithm.cxx
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
// MGUIBPAlgorithm
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIBPAlgorithm.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIBPAlgorithm)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIBPAlgorithm::MGUIBPAlgorithm(const TGWindow* Parent, const TGWindow* Main, 
                                 MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_Data = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIBPAlgorithm::~MGUIBPAlgorithm()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIBPAlgorithm::Create()
{
   // Create the main window

  // We start with a name and an icon...
  SetWindowName("Backprojection algorithm selection");  

  AddSubTitle("Please select the algorithm\n"
              "responsible for all single backprojections");


  TGLayoutHints* SingleLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsCenterX | kLHintsTop, 20, 20, 10, 10);

  m_BPList = new MGUIERBList(this, "Backprojection type:", false);
  m_BPList->Add("Point");
  m_BPList->Add("Lines");
  m_BPList->Add("Area");
  m_BPList->SetSelected(m_Data->GetBPAlgorithm());
  m_BPList->Create();
  AddFrame(m_BPList, SingleLayout);


  AddButtons();

  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIBPAlgorithm::OnApply()
{
	// The Apply button has been pressed

  m_Data->SetBPAlgorithm(m_BPList->GetSelected());

  return true;
}


// MGUIBPAlgorithm: the end...
////////////////////////////////////////////////////////////////////////////////
