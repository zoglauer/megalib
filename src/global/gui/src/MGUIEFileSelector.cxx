/*
 * MGUIEFileSelector.cxx
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
// MGUIEFileSelector
//
//
// This class is an elementary GUI-widget:
// It contains a text-label and a input-field.
// It can be checked, if the input is within a preselected range of values.
//
//
// Example:
//
// 
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEFileSelector.h"

// Standard libs:
#include "MStreams.h"

// ROOT libs:
#include <TGMsgBox.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MFile.h"
#include "MFileManager.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEFileSelector)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEFileSelector::MGUIEFileSelector(const TGWindow* Parent, MString Label, 
                                     MString FileName) :
  MGUIElement(Parent, kVerticalFrame)
{
  // Creates a frame containing two entry-boxes 
  //
  // Label:    The text of the label
  // FileName: Default name of the file

  m_Label = Label;
  m_FileName = FileName;
  m_FileTypes = 0;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEFileSelector::~MGUIEFileSelector()
{
  // default destructor

  if (MustCleanup() == kNoCleanup) {
    delete m_TextLabel;
    delete m_TextLabelLayout;

    delete m_InputLayout;
    delete m_Input;  

    delete m_ButtonFolderLayout;
    delete m_ButtonFolder;

    delete m_InputFrame;
    delete m_InputFrameLayout;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFileSelector::Create()
{
  // Create the label and the input-field.

  // label:
  if (m_Label != "") {
    m_TextLabel = new TGLabel(this, new TGString(m_Label));
    m_TextLabelLayout =
      new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsCenterY, 0, 30, 0, 0);
    AddFrame(m_TextLabel, m_TextLabelLayout);
  } else {
    m_TextLabel = 0;
    m_TextLabelLayout = 0;
  }

  m_InputFrame = new TGHorizontalFrame(this, 100, 40);
  if (m_Label != "") {
    m_InputFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 5, 0);
  } else {
    m_InputFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  }
  AddFrame(m_InputFrame, m_InputFrameLayout);

  // input-field
  m_InputLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  m_Input = new TGTextEntry(m_InputFrame, m_FileName);
  m_Input->SetAlignment(kTextRight);
  m_InputFrame->AddFrame(m_Input, m_InputLayout);
  m_Input->Layout();

  // input label:
  MString Icon = g_MEGAlibPath + "/resource/icons/folder.xpm";
  MFile::ExpandFileName(Icon);

  m_ButtonFolderLayout = new TGLayoutHints(kLHintsRight | kLHintsCenterY, 5, 0, 0, 0);
  m_ButtonFolder = new TGPictureButton(m_InputFrame, fClient->GetPicture(Icon), 99);
  m_ButtonFolder->Associate(this);
  m_InputFrame->AddFrame(m_ButtonFolder, m_ButtonFolderLayout);


  return;
}


////////////////////////////////////////////////////////////////////////////////


MString MGUIEFileSelector::GetFileName()
{
  // Return the name of the selected file

  return m_Input->GetText();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFileSelector::SetFileName(MString FileName)
{
  // Set the name of the selected file

  m_Input->SetText(FileName);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFileSelector::SetFileTypes(const char **FileTypes)
{
  // Set the filetypes

  m_FileTypes = FileTypes;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFileSelector::SetFileType(MString Name, MString Suffix)
{
  // Set one filetype

  int NTypes;
  const char** Types;

  if (m_FileTypes == 0) {
    Types = new const char*[6];
    Types[0] = StrDup(Name.Data());
    Types[1] = StrDup(Suffix.Data());
    Types[2] = "All files";
    Types[3] = "*";
    Types[4] = 0;
    Types[5] = 0;
    delete [] m_FileTypes;
    m_FileTypes = Types;
  } else {
    for (NTypes = 0; m_FileTypes[NTypes] != 0; NTypes += 2);
    NTypes += 2; // To account for the zeros

    Types = new const char*[NTypes+2];
    for (int i = 0; i < NTypes; i++) {
      Types[i] = m_FileTypes[i];
    }
    Types[NTypes-4] = StrDup(Name.Data());
    Types[NTypes-3] = StrDup(Suffix.Data());
    Types[NTypes-2] = "All files";
    Types[NTypes-1] = "*";
    Types[NTypes+0] = 0;
    Types[NTypes+1] = 0;
    delete [] m_FileTypes;
    m_FileTypes = Types;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEFileSelector::SetEnabled(bool flag)
{
	m_IsEnabled = flag;
	
  m_Input->SetEnabled(m_IsEnabled);

  if (flag == true) {
    m_ButtonFolder->SetState(kButtonUp);
  } else {
    m_ButtonFolder->SetState(kButtonDisabled);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEFileSelector::ProcessMessage(long Message, long Parameter1, 
                                       long Parameter2)
{
  // Process the messages for this application

  MFileManager *FM = new MFileManager();

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case 99:
        FM->SelectFileToLoad(m_FileName, m_FileTypes);
        m_Input->SetText(m_FileName);
        Layout();
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
  
  delete FM;

  return true;
}


// MGUIEFileSelector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
