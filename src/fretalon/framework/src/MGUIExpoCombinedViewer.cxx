/*
 * MGUIExpoCombinedViewer.cxx
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
#include "MGUIExpoCombinedViewer.h"

// Standard libs:
#include <iomanip>
#include <string>
#include <locale>
using namespace std;

// ROOT libs:
#include <KeySymbols.h>
#include <TApplication.h>
#include <TGPicture.h>
#include <TStyle.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TString.h>
#include <TGClient.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"
#include "MGUIEFileSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExpoCombinedViewer)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoCombinedViewer::MGUIExpoCombinedViewer() : MGUIDialog(gClient->GetRoot(), gClient->GetRoot())
{
  // No deep clean-up allowed in this function!
  SetCleanup(kNoCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoCombinedViewer::~MGUIExpoCombinedViewer()
{
  // No automatic cleanup so we have to do it ourselves
  // TODO: Cleanup
  
  RemoveExpos();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Expo - combined viewer");  

  m_MainTab = new TGTab(this, m_FontScaler*900, m_FontScaler*550);
  m_MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 20, 0);
  AddFrame(m_MainTab, m_MainTabLayout);
  

  // Start & Exit buttons
  // Frame around the buttons:
  TGHorizontalFrame* ButtonFrame = new TGHorizontalFrame(this, 150, 25);
  TGLayoutHints* ButtonFrameLayout =  new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10, 10, 10, 10);
  AddFrame(ButtonFrame, ButtonFrameLayout);
  
  // The buttons itself
  TGTextButton* UpdateButton = new TGTextButton(ButtonFrame, "Update", c_Update); 
  UpdateButton->Associate(this);
  TGLayoutHints* UpdateButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 40, 0, 0, 0);
  ButtonFrame->AddFrame(UpdateButton, UpdateButtonLayout);
  
  TGTextButton* ResetButton = new TGTextButton(ButtonFrame, "     Reset     ", c_Reset); 
  ResetButton->Associate(this);
  TGLayoutHints* ResetButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  ButtonFrame->AddFrame(ResetButton, ResetButtonLayout);
  
  TGTextButton* StopButton = new TGTextButton(ButtonFrame, "     Print     ", c_Print); 
  StopButton->Associate(this);
  TGLayoutHints* StopButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 20, 0, 0, 0);
  ButtonFrame->AddFrame(StopButton, StopButtonLayout);  
  

  // Give this element the default size of its content:
  Resize(m_FontScaler*900, m_FontScaler*580); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_Update:
        Status = OnUpdate();
        break;
      case c_Reset:
        Status = OnReset();
        break;
      case c_Print:
        Status = OnPrint();
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::RemoveExpos()
{
  //! Remove all expos
  
  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_MainTab->RemoveTab(0, false);
  }
  m_Expos.clear();

  if (IsMapped() == true) {
    MapSubwindows();
    MapWindow();  
    Layout();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::ShowExpos()
{
  //! Show all expos
  
  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_Expos[d]->ReparentWindow(m_MainTab);
    m_Expos[d]->Create();
    m_MainTab->AddTab(m_Expos[d]->GetTabTitle(), m_Expos[d]);
  }

  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::NeedsUpdate()
{
  //! Return true if we need an update
 
  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    if (m_Expos[d]->NeedsUpdate() == true) {
      return true;
    }
  }
  
  return false;
}
 
  
////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::OnUpdate()
{
  //! Update all tabs

  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_Expos[d]->Update();
  }
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::CloseWindow()
{
  // When the x is pressed, this function is called.
  // We do not delete it automatically!

  UnmapWindow();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::OnReset()
{
  // The Apply button has been pressed

  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_Expos[d]->Reset();
  }
  OnUpdate();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::OnPrint()
{
  // The Apply button has been pressed

  MString TabTitle = m_Expos[m_MainTab->GetCurrent()]->GetTabTitle();
  MString FileName;
  for (char c : TabTitle.GetString()) if (isalnum(c)) FileName += c;
  FileName += ".";
  FileName += MTime().GetShortString();
  FileName += ".pdf";
  
  m_Expos[m_MainTab->GetCurrent()]->Print(FileName);

  return true;
}


// MGUIExpoCombinedViewer: the end...
////////////////////////////////////////////////////////////////////////////////
