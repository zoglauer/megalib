/*
 * MGUIMultiInput.cxx
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
// MGUIMultiInput.cxx
//
// This (modal) dialog-box contains several (= NEntries) lines of entry boxes.
// The entered text can be found in the pointer "Input". After pressing Ok or
// Cancel the dialog-box is delete except the "Input"-pointer 
//
//
// Example:
//
//  int NEntries = 3;
//  MString Labels[3] = {"First Name", "Last Name", "Age"};
//  MString Inputs[3] = {"Bill", "Gates", "?"};
//
//  MGUIMultiInput *MI = 
//    new MGUIMultiInput(gClient->GetRoot(), this, "ID", 
//    "Please enter your identification\n or press Cancel to exit", 
//    NEntries, Labels, Inputs);
//
//  cout<<"Your ID is: "<<Inputs[0]<<" "<<Inputs[1]<<" ("<<Inputs[2]<<")"<<endl;
//
////////////////////////////////////////////////////////////////////////////////


#include "MGUIMultiInput.h"

#ifdef ___CINT___
ClassImp(MGUIMultiInput)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMultiInput::MGUIMultiInput(const TGWindow *p, const TGWindow *main, 
                               MString Title, MString SubTitle, int NEntries,
                               MString *Label, MString *Input)
  : MGUIDialog(p, main)
{
  // Creates a multi input dialog box
  //
  // p         - parent window
  // main      - main window
  // Title     - title of the window
  // Subtitle  - text of the description box: if it contains linebreaks "\n"
  //             than a multi line description-box is displayed 
  // NEntries  - number of input fields
  // Label     - description of the entry fields
  // Input     - default and returned text of the entry fields: if all fields
  //             are empty, return has been pressed. 

  m_Title = Title;
  m_SubTitle = SubTitle;
  m_NEntries = NEntries;
  m_Label = Label;
  m_Input = Input;
  m_NameLabel = new TObjArray(4);
  m_NameEntry = new TObjArray(4);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIMultiInput::~MGUIMultiInput()
{
  // Standard destructor:
  // The Label-string-pointer is delete, but NOT the input-pointer!

  delete m_SubTitleLabel;
  delete m_SubTitleLabelLayout;

  delete m_LabelFrame;
  delete m_LabelFrameLayout;

  m_NameLabel->Delete();
  delete m_NameLabel;
  delete m_NameLabelLayout;

  delete m_AddLabelsFrame;
  delete m_AddLabelsFrameLayout;

  m_NameEntry->Delete();
  delete m_NameEntry;
  delete m_NameEntryLayout;

  delete m_AddEntryFrame;
  delete m_AddEntryFrameLayout;

  delete m_AddFrame;
  delete m_AddFrameLayout;

  delete m_CancelButton;
  delete m_CancelButtonLayout;

  delete m_OKButton;
  delete m_OKButtonLayout;

  delete m_ButtonFrame;
  delete m_ButtonFrameLayout;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMultiInput::Create()
{
  // Create the main window

	int i;

  // We start with a name and an icon...
  SetWindowName(m_Title);  
  SetIconPixmap("MimrecIcon.xpm"); 

  // ... continue with the label ...
  // (start a new line for each "\n")
  m_LabelFrame = new TGVerticalFrame(this, 100, 10, kRaisedFrame);
  m_LabelFrameLayout =
    new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 20);
  AddFrame(m_LabelFrame, m_LabelFrameLayout);

  m_SubTitleLabel = new TObjArray();
  m_SubTitleLabelLayout =
    new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 0, 0, 1, 1);

  int LabelIndex = 0;
  MString SubString;

  while (m_SubTitle.Contains("\n") == true) {
    SubString = m_SubTitle;

    m_SubTitleLabel->AddAt(new TGLabel(m_LabelFrame, 
      new TGString(SubString.Remove(SubString.First('\n')+1, SubString.Length() - SubString.First('\n')- 1))), LabelIndex++);
    m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                           m_SubTitleLabelLayout);
    
    m_SubTitle.Replace(0, m_SubTitle.First('\n')+1, "");
  }
  
  m_SubTitleLabel->AddAt(new TGLabel(m_LabelFrame, 
                                     new TGString(m_SubTitle)), LabelIndex++);
  m_LabelFrame->AddFrame((TGLabel *) m_SubTitleLabel->At(LabelIndex-1), 
                         m_SubTitleLabelLayout);
  

  // entry-fields:
  m_AddFrame = new TGHorizontalFrame(this, 150, 155, kFixedSize);
  m_AddFrameLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 
                      20, 20, 10, 8);
  AddFrame(m_AddFrame, m_AddFrameLayout);

  // i) the labels
  m_AddLabelsFrame = new TGVerticalFrame(m_AddFrame, 150, 155, kFixedSize);
  m_AddLabelsFrameLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 
                      0, 0, 0, 0);
  m_AddFrame->AddFrame(m_AddLabelsFrame, m_AddLabelsFrameLayout);


  m_NameLabelLayout = new TGLayoutHints(kLHintsLeft, 0, 0, 5, 5);
  for (i = 0; i < m_NEntries; i++) {
    m_NameLabel->AddAt(new TGLabel(m_AddLabelsFrame, 
                                   new TGString(m_Label[i])), i);
    m_AddLabelsFrame->AddFrame((TGLabel *) m_NameLabel->At(i), 
                               m_NameLabelLayout);
  }

  // ii) the entry boxes
  m_AddEntryFrame = new TGVerticalFrame(m_AddFrame, 150, 155, kFixedSize);
  m_AddEntryFrameLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 
                      0, 0, 0, 0);
  m_AddFrame->AddFrame(m_AddEntryFrame, m_AddEntryFrameLayout);

  m_NameEntryLayout = 
    new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 
                      0, 0, 2, 2);

  for (i = 0; i < m_NEntries; i++) {
    m_NameEntry->AddAt(new TGTextEntry(m_AddEntryFrame, m_Input[i], 100), i);
    m_AddEntryFrame->AddFrame((TGTextEntry *) m_NameEntry->At(i), 
                              m_NameEntryLayout);
  }


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


  PositionWindow(300, 130 + m_NEntries*20 + (m_SubTitleLabel->GetLast()+1)*18);
  
  // make the window non-resizeable...
  TGDimension size = GetSize();
  SetWMSize(size.fWidth, size.fHeight);
  SetWMSizeHints(size.fWidth, size.fHeight, size.fWidth, size.fHeight, 0, 0);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();

  fClient->WaitFor(this);

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMultiInput::ProcessMessage(long Message, long Parameter1, 
                                      long Parameter2)
{
  // Process the messages for this application

	int i;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
      break;
      
    case kCM_BUTTON:
      switch (Parameter1) {
      case 1: // Ok
        // Copy the text of the text-entries of the input-pointer
        for (i = 0; i < m_NEntries; i++) {
          m_Input[i].Replace(0, m_Input[i].Length(), 
                             ((TGTextEntry *) m_NameEntry->At(i))->GetText());
        }
        CloseWindow();
        break;
        
      case 2: // Cancel
        // Delete the text of the input-pointer
        for (i = 0; i < m_NEntries; i++) {
          m_Input[i].Replace(0, m_Input[i].Length(), "");
        }
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


// MGUIMultiInput: the end...
////////////////////////////////////////////////////////////////////////////////
