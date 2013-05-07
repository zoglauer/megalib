/*
 * MGUISessionRealta.cxx                                   v0.9  12/01/2001
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
// MGUISessionRealta.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUISessionRealta.h"

// Standard libs:
#include <iostream>

// ROOT libs:
#include <MString.h>
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MGUIControlCenter.h"
#include "MGUINetwork.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUISessionRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISessionRealta::MGUISessionRealta(MGUIMainFrame *Parent)
  : MGUISession(Parent)
{
  // default constructor
  m_GUIData = new MSettingsRealta();
  //m_Realta = new MInterfaceRealta(m_GUIData);

  //m_ControlCenter = new MGUIControlCenter(gClient->GetRoot(), m_Parent, m_GUIData);

  CreateMenuBar();
  SetButtonText("Realta");
  SetButtonToolTipText("Real Time Analysis");

  CreateCenterFrame();
}


////////////////////////////////////////////////////////////////////////////////


MGUISessionRealta::~MGUISessionRealta()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////



void MGUISessionRealta::CreateMenuBar()
{
  //cout<<"MGUISessionRealta"<<endl;

  //MGUISession::CreateMenuBar();

  m_MenuAnalysis = new TGPopupMenu(fClient->GetRoot());
  m_MenuAnalysis->AddEntry("Network", M_REALTA_NETWORK);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Store input as ...", M_REALTA_STORAGE);
  m_MenuAnalysis->Associate(this);
  m_MenuBar->AddPopup("Analysis", m_MenuAnalysis, m_MenuBarItemLayoutLeft);
}


////////////////////////////////////////////////////////////////////////////////


void MGUISessionRealta::CreateCenterFrame()
{
  // 

  m_PictureLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsCenterY | kLHintsExpandY,
                                      0, 0, 0, 0);
  m_PictureButton = new TGPictureButton(m_CenterFrame, fClient->GetPicture("resource/icons/Realta.xpm"));
  m_PictureButton->SetState(kButtonDisabled);
  m_PictureButton->ChangeOptions(0);
  m_PictureButton->Associate(this);          
  m_CenterFrame->AddFrame(m_PictureButton, m_PictureLayout);
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MGUISessionRealta::ProcessMessage(Long_t Message, Long_t Parameter1, Long_t)
{
  // Process the messages for this application

  //cout<<"MGUISessionRealta::ProcessMessage"<<Message<<", "<<Parameter1<<endl;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
	      
      default:
        break;
      }
      
    case kCM_MENU:
      switch (Parameter1) {

      case IF_OPEN:
        m_GUIData->SetCurrentFile(SearchFile());
        break;
        
      case M_REALTA_NETWORK:
        cout<<"Starting network!"<<endl;
        new MGUINetwork(gClient->GetRoot(), m_Parent, m_GUIData);
        break;
        
      case M_REALTA_STORAGE:
        cout<<"Searching file!"<<endl;
        break;

      case IF_EXIT:
        m_GUIData->SaveData();
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
  
  return MGUISession::ProcessMessage(Message, Parameter1, 0);
}


////////////////////////////////////////////////////////////////////////////////


void MGUISessionRealta::Launch()
{
  // This method is called after pressing the MEGA-button in the 
  // M.I.Works main GUI:

  m_ControlCenter->DisplayWindow();
  //m_Realta->AnalyzeEvents();
}


////////////////////////////////////////////////////////////////////////////////


Char_t** MGUISessionRealta::GetFileTypes()
{
  // 

  Char_t **Types = new Char_t*[6];
  
  Types[0] = "Event files";
  Types[1] = "*.evta";
  Types[2] = "All files";
  Types[3] = "*";
  Types[4] = 0;
  Types[5] = 0;

  return Types;
}


////////////////////////////////////////////////////////////////////////////////


void MGUISessionRealta::SetFileName(MString Filename)
{
  // Do nothing here ...

  m_GUIData->SetCurrentFile(Filename);
}


////////////////////////////////////////////////////////////////////////////////


void MGUISessionRealta::Activate()
{
  // Before the session is displayed, this fuction has to be called
}


////////////////////////////////////////////////////////////////////////////////



// MGUISessionRealta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
