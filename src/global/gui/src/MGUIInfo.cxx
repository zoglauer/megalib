/*
 * MGUIInfo.cxx
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


#include "MGUIInfo.h"

#ifdef ___CINT___
ClassImp(MGUIInfo)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIInfo::MGUIInfo(const TGWindow *p, const TGWindow *main)
  : MGUIDialog(p, main)
{
  // standard constructor

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIInfo::~MGUIInfo()
{
  // standard destructor



  delete m_OKButton;
  delete m_OKButtonLayout;

  delete m_ButtonFrame;
  delete m_ButtonFrameLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIInfo::Create()
{
   // Create the main window

  // We start with a name and an icon...
  SetWindowName("About...");  
 
  AddSubTitle("About MIWorks & MEGAlib");


  // The information
  MString Info("MIWorks development version 0.81 (Feb. 2001)\n\n"
               "GeoMega development version 0.85 (12-Mar-2001)\n\n"
               "If you encounter bugs or have enhancement request,\n"
               "please mail a full report to\n\n"
               "zog@mpe.mpg.de\n\n\n"
               "(C) by Andreas Zoglauer and the MEGA-team\n"
               "All rights reserved");

  m_InfoFrame = new TGVerticalFrame(this, 100, 10);
  m_InfoFrameLayout =
    new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 30);
  AddFrame(m_InfoFrame, m_InfoFrameLayout);
  
  m_InfoText = new TObjArray();
  m_InfoTextLayout =
    new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 0, 0, 1, 1);
  
  int LabelIndex = 0;
  MString SubString;
  
  while (Info.Contains("\n") == true) {
    SubString = Info;
    
    m_InfoText->AddAt(new TGLabel(m_InfoFrame, 
                                  new TGString(SubString.Remove(SubString.First('\n')+1, 
                                                                SubString.Length() - SubString.First('\n')- 1))), LabelIndex++);
    m_InfoFrame->AddFrame((TGLabel *) m_InfoText->At(LabelIndex-1), 
                          m_InfoTextLayout);
    
    Info.Replace(0, Info.First('\n')+1, "");
  }
    
  m_InfoText->AddAt(new TGLabel(m_InfoFrame, 
                                     new TGString(Info)), LabelIndex++);
  m_InfoFrame->AddFrame((TGLabel *) m_InfoText->At(LabelIndex-1), 
                         m_InfoTextLayout);



  // ... and finally the Ok and Cancel-buttons
  m_ButtonFrame = new TGHorizontalFrame(this, 150, 25, kFixedSize);
  m_ButtonFrameLayout = 
   new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
         10, 10, 10, 8);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);

  m_OKButton = new TGTextButton(m_ButtonFrame, "OK", 3); 
  m_OKButton->Associate(this);
  m_OKButtonLayout = 
   new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 5, 0, 0, 0);
  m_ButtonFrame->AddFrame(m_OKButton, m_OKButtonLayout);


  // Let's resize and position the window, 
  PositionWindow(400, 350);
  
  // make it non-resizeable...
  TGDimension size = GetSize();
  SetWMSize(size.fWidth, size.fHeight);
  SetWMSizeHints(size.fWidth, size.fHeight, size.fWidth, size.fHeight, 0, 0);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIInfo::ProcessMessage(long Message, long Parameter1, 
                                long Parameter2)
{
  // Process the messages for this dialog-box
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) { 
    case kCM_BUTTON:
      switch (Parameter1) {
      case 3:
        CloseWindow();
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
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIInfo::CloseWindow()
{
  // Close the window

  delete this;
}


// MGUIInfo: the end...
////////////////////////////////////////////////////////////////////////////////
