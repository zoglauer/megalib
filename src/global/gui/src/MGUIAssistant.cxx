/*
 * MGUIAssistant.cxx
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
// MGUIAssistant
//
//
// This is the base class for most of the other Dialogs.
// It provides some methods all other dialogs need
//
// Important info:
// Always call a derived class via
// MDerived *d = new MDerived(...)
// and never use MDerived d(..) because it will simply always and ever crash.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIAssistant.h"

// Standard libs:
#include <stdlib.h>
#include <ctype.h>

// ROOT libs:
#include "TGLabel.h"
#include "TSystem.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


//#ifdef ___CLING___
//ClassImp(MGUIAssistant)
//#endif


////////////////////////////////////////////////////////////////////////////////


MGUIAssistant::MGUIAssistant() 
  : TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 480, 360)
{
  // standard constructor

  m_ParentWindow = (TGFrame *) gClient->GetRoot();
}


////////////////////////////////////////////////////////////////////////////////


MGUIAssistant::MGUIAssistant(const TGWindow *p, const TGWindow *main, unsigned int Type, unsigned int w, 
                       unsigned int h, unsigned int options) 
  : TGTransientFrame(p, main, w, h, options)
{
  m_ParentWindow = (TGFrame *) main;
  m_Type = Type;
  
  MString Path(g_MEGAlibPath + "/resource/icons/global/Icon.xpm");
  MFile::ExpandFileName(Path);
  SetIconPixmap(Path);

  m_SubTitleLabel = 0;
  m_SubTitleFirstLayout = 0;
  m_SubTitleMiddleLayout = 0;
  m_SubTitleLastLayout = 0;
  m_SubTitleOnlyLayout = 0;

  m_ButtonsAdded = false;
  m_SubTitleAdded = false;
  
  m_LabelFrame = 0;
  m_LabelFrameLayout = 0;
    
  m_ButtonFrame = 0;
  m_ButtonFrameLayout = 0;    

  m_BackText = "<< Back <<";
  m_BackButton = 0;
  m_BackButtonLayout = 0;

  m_CancelText = "Cancel";
  m_CancelButton = 0;
  m_CancelButtonLayout = 0;
    
  m_NextText = ">> Next >>";
  m_NextButton = 0;
  m_NextButtonLayout = 0;
    
  m_ButtonFrame = 0;
  m_ButtonFrameLayout = 0;    
}


////////////////////////////////////////////////////////////////////////////////


MGUIAssistant::~MGUIAssistant()
{
  m_ParentWindow = 0;

  //if (m_SubTitleAdded == true) {
  delete m_SubTitleLabel;
  delete m_SubTitleFirstLayout;
  delete m_SubTitleMiddleLayout;
  delete m_SubTitleLastLayout;
  delete m_SubTitleOnlyLayout;
  
  delete m_LabelFrame;
  delete m_LabelFrameLayout;
  //}

  delete m_BackButton;
  delete m_BackButtonLayout;

  delete m_CancelButton;
  delete m_CancelButtonLayout;
    
  delete m_NextButton;
  delete m_NextButtonLayout;
    
  delete m_ButtonFrame;
  delete m_ButtonFrameLayout;    
  
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::CloseWindow()
{
  // When the x is pressed, this function is called.

  DeleteWindow();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::PositionWindow(int Width, int Height, bool AllowResize)
{
  // This method positions the dialog window in the center of its parent window,
  // but (if possible) the whole window is shown on the screen.

  // First we compute the above-mentioned x and y coordinates ...
  int x = 0, y = 0;
  Window_t WindowDummy;
  gVirtualX->TranslateCoordinates(m_ParentWindow->GetId(), 
                                  GetParent()->GetId(),
                                  (m_ParentWindow->GetWidth() - Width) >> 1,
                                  (m_ParentWindow->GetHeight() - Height) >> 1,
                                  x, 
                                  y, 
                                  WindowDummy);

  // ... get the width and height of the display ...
  int xDisplay, yDisplay;
  unsigned int wDisplay, hDisplay;
  gVirtualX->GetGeometry(-1, xDisplay, yDisplay, wDisplay, hDisplay);

  // ... make sure that the whole dialog window is shown on the screen ...
  if (Width > (int) wDisplay) Width = wDisplay - 50;
  if (Height > (int) hDisplay) Height = hDisplay - 50;

  if (x + Width > (int) wDisplay)
    x = wDisplay - Width - 8;
  if (y + Height > (int) hDisplay)
    y = hDisplay - Height - 28;

  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  // ... and bring the dialog to the calculated position.
  MoveResize(x, y, Width, Height);

  if (AllowResize == true) {
    TGDimension size = GetSize();
    SetWMSize(size.fWidth, size.fHeight);
    SetWMSizeHints(size.fWidth, size.fHeight, size.fWidth, size.fHeight, 0, 0);
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::Header(MString SubTitle)
{
  // Add a label to the top of the window:

  if (m_SubTitleAdded == false) {
    m_LabelFrame = new TGVerticalFrame(this, 100, 10, kRaisedFrame);
    m_LabelFrameLayout =
      new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 30);
    AddFrame(m_LabelFrame, m_LabelFrameLayout);
    
    m_SubTitleLabel = new TObjArray();
    m_SubTitleFirstLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 5, 1);
    m_SubTitleMiddleLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 1, 1);
    m_SubTitleLastLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 1, 5);
    m_SubTitleOnlyLayout =
      new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 20, 20, 5, 5);
    
    bool First = true;
    int LabelIndex = 0;
    MString SubString;
    
    // (start a new line for each '\n')
    while (SubTitle.Contains('\n') == true) {
      SubString = SubTitle;
      SubString = SubString.Remove(SubString.First('\n'), SubString.Length() - SubString.First('\n'));
      m_SubTitleLabel->AddAt(new TGLabel(m_LabelFrame, 
        new TGString(SubString)), LabelIndex++);

      if (First == true) {
        m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
          m_SubTitleFirstLayout);
        First = false;
      } else {
        m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
          m_SubTitleMiddleLayout);
      }
      
      SubTitle.Replace(0, SubTitle.First('\n')+1, "");
    }
    
    m_SubTitleLabel->AddAt(new TGLabel(m_LabelFrame, 
                                       new TGString(SubTitle)), LabelIndex++);

    if (First == true) {
      m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
        m_SubTitleOnlyLayout);
    } else {
      m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
        m_SubTitleLastLayout);
    }
  } else {
    Error("void MGUIAssistant::AddSubTitle(MString SubTitle)",
          "SubTitle has already been added!");
  }

  m_SubTitleAdded = true;


  return;
}


////////////////////////////////////////////////////////////////////////////////


  void SetFooterText(MString Back, MString Next, MString Cancel);


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::Footer()
{
  // Add the Ok- and Cancel-Buttons

  if (m_ButtonsAdded == false) {
    m_ButtonFrame = new TGHorizontalFrame(this, 150, 25);
    m_ButtonFrameLayout = 
      new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
                        5, 5, 30, 10);
    AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
    
    
    m_BackButton = new TGTextButton(m_ButtonFrame, m_BackText, c_Back);
    if (m_Type == c_First) {
      m_BackButton->SetState(kButtonDisabled);
      //m_BackButton->SetText("");
      //m_BackButton->ChangeOptions(0);
    }
    m_BackButton->Associate(this);
    m_BackButtonLayout = 
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 15, 0, 0);
    m_ButtonFrame->AddFrame(m_BackButton, m_BackButtonLayout);
    
    m_CancelButton = new TGTextButton(m_ButtonFrame, m_CancelText, c_Cancel); 
    m_CancelButton->Associate(this);
    m_CancelButtonLayout = 
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 15, 15, 0, 0);
    m_ButtonFrame->AddFrame(m_CancelButton, m_CancelButtonLayout);

    
    if (m_Type == c_Last) {
      m_NextText = "Finish";
    }

    m_NextButton = new TGTextButton(m_ButtonFrame, m_NextText, c_Next); 
    m_NextButton->Associate(this);
    m_NextButtonLayout = 
      new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 15, 5, 0, 0);
    m_ButtonFrame->AddFrame(m_NextButton, m_NextButtonLayout);
  }

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIAssistant::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_Back:
        OnBack();
        break;
      case c_Next:
        OnNext();
        break;
      case c_Cancel:
        OnCancel();
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
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::OnBack()
{
  // The back button has been pressed

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::OnNext()
{
  // The next button has been pressed

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::OnCancel()
{
  // The cancel button has been pressed

  m_ParentWindow->MapWindow();

  CloseWindow();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::SetTextBack(MString String)
{
  m_BackText = String;
  if (m_BackButton != 0) {
    m_BackButton->SetText(m_BackText.Data());
    m_BackButton->Layout();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::SetTextNext(MString String)
{
  m_NextText = String;
  if (m_NextButton != 0) {
    m_NextButton->SetText(m_NextText.Data());
    m_NextButton->Layout();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIAssistant::SetTextCancel(MString String)
{
  m_CancelText = String;
  if (m_CancelButton != 0) {
    m_CancelButton->SetText(m_CancelText.Data());
    m_CancelButton->Layout();
  }
}


// MGUIAssistant: the end...
////////////////////////////////////////////////////////////////////////////////
