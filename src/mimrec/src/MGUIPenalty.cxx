/*
 * MGUIPenalty.cxx
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
// MGUIPenalty
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIPenalty.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIPenalty)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPenalty::MGUIPenalty(const TGWindow* Parent, const TGWindow* Main, 
                         MSettingsImaging* Data)
 : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIPenalty::~MGUIPenalty()
{
  // standard destructor

  delete m_Label1;
  delete m_Label1Layout;
  delete m_Label2;
  delete m_Label2Layout;

  delete m_LabelFrame;
  delete m_LabelFrameLayout;

  delete m_RB[0];
  delete m_RB[1];
  delete m_RB[2];
  delete m_RBLayout;

  delete m_LabelPenaltyAlpha;
  delete m_LabelPenaltyAlphaLayout;

  delete m_PenaltyAlphaLayout;
  delete m_PenaltyAlphaInput;

  delete m_PenaltyAlphaFrame;
  delete m_PenaltyAlphaFrameLayout;


  delete m_CancelButton;
  delete m_CancelButtonLayout;

  delete m_OKButton;
  delete m_OKButtonLayout;

  delete m_ButtonFrame;
  delete m_ButtonFrameLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPenalty::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Penalty selection");  
 

  // ... continue with the label ...
  m_LabelFrame = new TGVerticalFrame(this, 100, 10, kRaisedFrame);
  m_LabelFrameLayout =
    new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 20);
  AddFrame(m_LabelFrame, m_LabelFrameLayout);


  m_Label1 = new TGLabel(m_LabelFrame, 
                         new TGString("Please select the penalty"));
  m_Label1Layout =
    new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 
                      0, 0, 2, 0);
  m_LabelFrame->AddFrame(m_Label1, m_Label1Layout);

  m_Label2 = new TGLabel(m_LabelFrame, 
                         new TGString("and its value for all iterations"));
  m_Label2Layout =
    new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsBottom, 
                      0, 0, 0, 2);
  m_LabelFrame->AddFrame(m_Label2, m_Label2Layout);


  // ... add the radiobutton selection ...
  m_RBLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,
                                 40, 2, 5, 0);
  
  m_RB[0] = new TGRadioButton(this, "None", 0);
  m_RB[1] = new TGRadioButton(this, "Square", 1);
  m_RB[2] = new TGRadioButton(this, "ME", 2);

  int i;
  for (i = 0; i <= 2; i++)
    {
      AddFrame(m_RB[i], m_RBLayout);
      m_RB[i]->Associate(this);
      if (i == m_GUIData->GetPenalty()) m_RB[i]->SetState(kButtonDown);
    } 


  // ALPHA:
  m_PenaltyAlphaFrame = new TGHorizontalFrame(this, 100, 25, kFixedSize);
  m_PenaltyAlphaFrameLayout =
    new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 10, 20, 2);
  AddFrame(m_PenaltyAlphaFrame, m_PenaltyAlphaFrameLayout);
  
  m_LabelPenaltyAlpha = new TGLabel(m_PenaltyAlphaFrame, new TGString("Alpha:"));
  m_LabelPenaltyAlphaLayout =
    new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 0, 3, 0);
  m_PenaltyAlphaFrame->AddFrame(m_LabelPenaltyAlpha, m_LabelPenaltyAlphaLayout);
  
  m_PenaltyAlphaLayout = new TGLayoutHints(kLHintsLeft, 5, 5, 0, 0);
  m_PenaltyAlphaBuffer = new TGTextBuffer(10);
  m_PenaltyAlphaBuffer->AddText(0, (char *) MakeSmartString(m_GUIData->GetPenaltyAlpha()).Data());
  m_PenaltyAlphaInput = new TGTextEntry(m_PenaltyAlphaFrame, m_PenaltyAlphaBuffer);
  m_PenaltyAlphaInput->Resize(50, m_PenaltyAlphaInput->GetDefaultHeight());
  m_PenaltyAlphaInput->SetAlignment(kTextCenterX);
  m_PenaltyAlphaFrame->AddFrame(m_PenaltyAlphaInput, m_PenaltyAlphaLayout);
  m_PenaltyAlphaInput->Layout();



  // ... and finally the Ok and Cancel-buttons
  m_ButtonFrame = new TGHorizontalFrame(this, 150, 25, kFixedSize);
  m_ButtonFrameLayout = 
    new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 
                      10, 10, 10, 8);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
    
  m_CancelButton = new TGTextButton(m_ButtonFrame, "Cancel", 2); 
  m_CancelButton->Associate(this);
  m_CancelButtonLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 5, 0, 0);
  m_ButtonFrame->AddFrame(m_CancelButton, m_CancelButtonLayout);

  m_OKButton = new TGTextButton(m_ButtonFrame, "Ok", 1); 
  m_OKButton->Associate(this);
  m_OKButtonLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 5, 0, 0, 0);
  m_ButtonFrame->AddFrame(m_OKButton, m_OKButtonLayout);


  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPenalty::ProcessMessage(long Message, long Parameter1, 
                                 long Parameter2)
{
  // Process the messages for this application

  int i;
  
  switch (GET_MSG(Message)) 
    {
    case kC_COMMAND:
      switch (GET_SUBMSG(Message)) 
        {
        case kCM_RADIOBUTTON:
          if (Parameter1 >= 0 && Parameter1 <= 2)
            {
              for (i = 0; i <= 2; i++)
                if (m_RB[i]->WidgetId() != Parameter1)
                  m_RB[i]->SetState(kButtonUp);
                else
                  m_RB[i]->SetState(kButtonDown);
            }
          break;

        case kCM_BUTTON:
          switch (Parameter1) 
            {
            case 1:
              for (i = 0; i <= 2; i++)
                {
                  if (m_RB[i]->GetState() == kButtonDown) 
                    m_GUIData->SetPenalty(i);
                }

              if (IsNumber(MString(m_PenaltyAlphaBuffer->GetString())) == true)
                m_GUIData->SetPenaltyAlpha(atof((char *) m_PenaltyAlphaBuffer->GetString()));
              else
                cout<<"no number!!"<<endl;

              CloseWindow();
              break;

            case 2:
              CloseWindow();
              break;

            default:
              break;
            }
    
        case kCM_MENUSELECT:
          break;
    
        case kCM_MENU:
          switch (Parameter1) 

            {
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



// MGUIPenalty: the end...
////////////////////////////////////////////////////////////////////////////////
