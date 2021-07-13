/*
 * MGUIPQSelection.cxx
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
// MGUIPQSelection.cxx
//
// This is a specialized diAlog-box, which displays some point-sources in a
// list-box. Via the buttons add, modify and delete the content of this box
// can be handled.  
//
//
// Example:
//
// new MGUIPQSelection(gClient->GetRoot(), this, m_GUIData);
//
// Nothing else is needed.
//
////////////////////////////////////////////////////////////////////////////////


#include "MGUIPQSelection.h"

#ifdef ___CLING___
ClassImp(MGUIPQSelection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIPQSelection::MGUIPQSelection(const TGWindow* Parent, const TGWindow* Main, 
                                 MSettingsMimrec* GUIData)
  : MGUIDialog(Parent, Main)
{
  // Creates the pointsource selection window
  //
  // p       - parent window
  // main    - main window
  // GUIData - all the GUIData

  m_GUIData = GUIData;
  m_NEntries = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIPQSelection::~MGUIPQSelection()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPQSelection::Create()
{
  // Create the main window


  // We start with a name and an icon...
  SetWindowName("Pointsource deselection");  
  SetIconPixmap("MimrecIcon.xpm"); 


  // ... continue with the label ...
  m_LabelFrame = new TGVerticalFrame(this, 100*m_FontScaler, 10*m_FontScaler, kRaisedFrame);
  m_LabelFrameLayout = new TGLayoutHints(kLHintsExpandX, 10*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler, 20*m_FontScaler);
  AddFrame(m_LabelFrame, m_LabelFrameLayout);

  m_Label1 = new TGLabel(m_LabelFrame, new TGString("Enter some point sources"));
  m_Label1Layout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsTop, 0, 0, 2*m_FontScaler, 0);
  m_LabelFrame->AddFrame(m_Label1, m_Label1Layout);

  m_Label2 = new TGLabel(m_LabelFrame, new TGString("which should be deleted"));
  m_Label2Layout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsBottom, 0, 0, 0, 2*m_FontScaler);
  m_LabelFrame->AddFrame(m_Label2, m_Label2Layout);

  
  // Add/Delete buttons:
  m_AddDeleteFrame = new TGHorizontalFrame(this, 150*m_FontScaler, 25*m_FontScaler, kFixedSize);
  m_AddDeleteFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 10*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler, 5*m_FontScaler);
  AddFrame(m_AddDeleteFrame, m_AddDeleteFrameLayout);
    
  m_AddButton = new TGTextButton(m_AddDeleteFrame, "Add PQ", 1002); 
  m_AddButton->Associate(this);
  m_AddButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 10*m_FontScaler, 5*m_FontScaler, 0, 0);
  m_AddDeleteFrame->AddFrame(m_AddButton, m_AddButtonLayout);
    
  m_ModifyButton = new TGTextButton(m_AddDeleteFrame, "Modify PQ", 1003); 
  m_ModifyButton->Associate(this);
  m_ModifyButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5*m_FontScaler, 5*m_FontScaler, 0, 0);
  m_AddDeleteFrame->AddFrame(m_ModifyButton, m_ModifyButtonLayout);

  m_DeleteButton = new TGTextButton(m_AddDeleteFrame, "Delete PQ", 1004); 
  m_DeleteButton->Associate(this);
  m_DeleteButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 5*m_FontScaler, 10*m_FontScaler, 0, 0);
  m_AddDeleteFrame->AddFrame(m_DeleteButton, m_DeleteButtonLayout);


  // ... add some list-boxes:
  m_ListBoxFrame = new TGHorizontalFrame(this, 150*m_FontScaler, 175*m_FontScaler, kFixedSize);
  m_ListBoxFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsCenterX, 10*m_FontScaler, 10*m_FontScaler, 5*m_FontScaler, 10*m_FontScaler);
  AddFrame(m_ListBoxFrame, m_ListBoxFrameLayout);

  m_ListBoxLayout = new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 10*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler, 8*m_FontScaler);
  m_ListBoxLong = new TGListBox(m_ListBoxFrame, 21);

  m_ListBoxFrame->AddFrame(m_ListBoxLong, m_ListBoxLayout);
  m_ListBoxLong->Associate(this);
  m_ListBoxLong->Resize(150*m_FontScaler, 172*m_FontScaler);

  m_GUIData->GetDeselectedPointSources()->Compress();
  for (int i = 0; i < m_GUIData->GetDeselectedPointSources()->GetLast()+1; i++) {
    m_ListBoxLong->AddEntry(((MPointSource *) (m_GUIData->GetDeselectedPointSources()->At(i)))->ToString(), m_NEntries++);
  }
 

  // ... add the use-PQs check-button
  m_UsePQsCB = new TGCheckButton(this, "Apply these deselections", 999);
  m_UsePQsCBLayout = new TGLayoutHints(kLHintsLeft, 20*m_FontScaler, 0*m_FontScaler, 5*m_FontScaler, 5*m_FontScaler);
  AddFrame(m_UsePQsCB, m_UsePQsCBLayout);
  m_UsePQsCB->SetState((m_GUIData->GetApplyDeselectedPointSources() == 1) ?  kButtonDown : kButtonUp);


  // ... and finally the Ok and Cancel-buttons
  m_ButtonFrame = new TGHorizontalFrame(this, 150*m_FontScaler, 25*m_FontScaler, kFixedSize);
  m_ButtonFrameLayout = new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10*m_FontScaler, 10*m_FontScaler, 10*m_FontScaler, 8*m_FontScaler);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);
    
  m_CancelButton = new TGTextButton(m_ButtonFrame, "Cancel", e_Cancel); 
  m_CancelButton->Associate(this);
  m_CancelButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 5*m_FontScaler, 0, 0);
  m_ButtonFrame->AddFrame(m_CancelButton, m_CancelButtonLayout);

  m_OKButton = new TGTextButton(m_ButtonFrame, "Ok", e_Ok); 
  m_OKButton->Associate(this);
  m_OKButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 5*m_FontScaler, 0, 0, 0);
  m_ButtonFrame->AddFrame(m_OKButton, m_OKButtonLayout);


  PositionWindow(300, 380, false);
  
  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPQSelection::ProcessMessage(long Message, long Parameter1, 
                                       long Parameter2)
{
  // Process the messages for this application

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_RADIOBUTTON:
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
        
      case 1002: // Add
        AddPointSource();
        break;
        
      case 1003: // Modify
        ModifyPointSource();
        break;
        
      case 1004: // Delete
        DeletePointSource();
        break;
        
      default:
        break;
      }
      
    case kCM_MENUSELECT:
      break;
      
    case kCM_MENU:
      switch (Parameter1) {
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


bool MGUIPQSelection::OnCancel()
{
  // The Apply button has been pressed

  m_GUIData->GetDeselectedPointSources()->Compress();
  CloseWindow();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIPQSelection::OnApply()
{
  // The Apply button has been pressed

  m_GUIData->GetDeselectedPointSources()->Compress();
  m_GUIData->SetApplyDeselectedPointSources((m_UsePQsCB->GetState() == kButtonDown) ? 1 : 0);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPQSelection::AddPointSource()
{
  // Add a pointsource to the list

  int NEntries = 4;
  MString Labels[4] = {"Name", "Longitude", "Latitude", "Radius"};
  MString Inputs[4] = {"", "", "", ""};

  //MGUIMultiInput *Input = 
  new MGUIMultiInput(gClient->GetRoot(), this, "Point source data", 
                     "Please enter the data of a point source", 
                     NEntries, Labels, Inputs);
  
  if (Inputs[0].CompareTo("") == true || Inputs[1].CompareTo("") == true 
      || Inputs[2].CompareTo("") == true || Inputs[3].CompareTo("") == true) {
    m_GUIData->GetDeselectedPointSources()->
      AddLast(new MPointSource(Inputs[0], 
                               atof(Inputs[2]), 
                               atof(Inputs[1]), 
                               atof(Inputs[3])));
    
    m_ListBoxLong->AddEntry(((MPointSource *) 
      (m_GUIData->GetDeselectedPointSources()->At(m_NEntries++)))->ToString(), 
      m_NEntries);
    m_ListBoxLong->SetTopEntry(m_NEntries);
    
    MapSubwindows();
    MapWindow();
    Layout();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPQSelection::ModifyPointSource()
{
  // Modify a pointsource of the list:

  // if no entry has been selected:
  int Selected = m_ListBoxLong->GetSelected();

  if (Selected == -1) {
    new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Warning", 
                 "Please select a point source or press Cancel", 
                 kMBIconExclamation, kMBOk);
    return;
  } else {
    int NEntries = 4;
    MString Labels[4] = {"Name", "Longitude", "Latitude", "Radius"};
    MString Inputs[4];

    MPointSource *PS = 
      ((MPointSource *) (m_GUIData->GetDeselectedPointSources()->At(Selected)));
    Inputs[0] = MString(PS->GetName());
    char Text[100];
    sprintf(Text, "%f", PS->GetLatitude());
    Inputs[2] = MString(Text);
    sprintf(Text, "%f", PS->GetLongitude());
    Inputs[1] = MString(Text);
    sprintf(Text, "%f", PS->GetSigma());
    Inputs[3] = MString(Text);

    //MGUIMultiInput *Input = 
    new MGUIMultiInput(gClient->GetRoot(), this, "Point source data", 
                       "Please enter the data of a point source", 
                       NEntries, Labels, Inputs);
    
    if (Inputs[0].CompareTo("") == true || Inputs[1].CompareTo("") == true 
        || Inputs[2].CompareTo("") == true || Inputs[3].CompareTo("") == true) {

      m_GUIData->GetDeselectedPointSources()->RemoveAt(Selected);
      m_GUIData->GetDeselectedPointSources()->
        AddAt(new MPointSource(Inputs[0], 
                               atof(Inputs[2]), 
                               atof(Inputs[1]), 
                               atof(Inputs[3])), Selected);
      
      m_ListBoxLong->RemoveEntry(Selected);
      m_ListBoxLong->AddEntry(((MPointSource *) 
        (m_GUIData->GetDeselectedPointSources()->At(Selected)))->ToString(), 
        Selected);
      m_ListBoxLong->SetTopEntry(Selected);

      MapSubwindows();
      MapWindow();
      Layout();
    }
    
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIPQSelection::DeletePointSource()
{
  // Delete the selected pointsource

  // if no entry has been selected:
  if (m_ListBoxLong->GetSelected() == -1) {
    new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Warning", 
                 "Please select a point source or press Cancel", 
                 kMBIconExclamation, kMBOk);
    return;
  } else {
    m_GUIData->GetDeselectedPointSources()->
      RemoveAt(m_ListBoxLong->GetSelected());
    
    m_ListBoxLong->RemoveEntry(m_ListBoxLong->GetSelected());

    MapSubwindows();
    MapWindow();
    Layout();

    return;
  }
}



// MGUIPQSelection: the end...
////////////////////////////////////////////////////////////////////////////////
