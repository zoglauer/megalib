/*
 * MGUIMain.cxx
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
#include "MGUIMain.h"

// ROOT libs:
#include <TGFileDialog.h>
#include <TStyle.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MSystem.h"
#include "MStreams.h"
#include "MInterface.h"
#include "MSettings.h"
#include "MFile.h"

// Standard libs:
#include <fstream>
using namespace std;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMain::MGUIMain(unsigned int Width, unsigned int Height, MInterface* Interface, MSettings* Data)
  : TGMainFrame(gClient->GetRoot(), Width, Height, kVerticalFrame)
{
  // Default constructor:

  m_BaseInterface = Interface;
  m_BaseData = Data;

  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


MGUIMain::~MGUIMain()
{
  // Standard destructor - in all normal exit cases the session have already 
  // been deleted in CloseWindow
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::Create()
{
  // Create some important features of the main window

  MString Path(g_MEGAlibPath + "/resource/icons/global/Icon.xpm");
  MFile::ExpandFileName(Path);
  SetIconPixmap(Path);

  // In the beginning we build the menus and define their layout, ...	
  m_MenuBarItemLayoutLeft = 
    new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  m_MenuBarItemLayoutRight = 
    new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  m_MenuBarLayout = new TGLayoutHints
    (kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  m_MenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);
  AddFrame(m_MenuBar, m_MenuBarLayout);
  
  m_MenuGeometryHistory = new TGPopupMenu(gClient->GetRoot());

  m_MenuFile = new TGPopupMenu(gClient->GetRoot());
  m_MenuFile->AddEntry("Open ...", c_Open);
  AddGeometryToFileMenu();
  m_MenuFile->AddSeparator();
  m_MenuFile->AddEntry("Load configuration", c_LoadConfig);
  m_MenuFile->AddEntry("Save configuration", c_SaveConfig);
  m_MenuFile->AddSeparator();
  m_MenuFile->AddEntry("Exit", c_Exit);

  m_MenuBar->AddPopup("File", m_MenuFile, m_MenuBarItemLayoutLeft);
  m_MenuFile->Associate(this);


  m_MenuInfo = new TGPopupMenu(fClient->GetRoot());
  m_MenuInfo->AddEntry("About", c_About);
  m_MenuInfo->Associate(this);
  // m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayoutRight);

  // The tool bar:
  m_ToolBar = new MGUIEFlatToolBar(this, 600, 50, kHorizontalFrame | kRaisedFrame);
  m_ToolBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  AddFrame(m_ToolBar, m_ToolBarLayout);


  // The status bar  
  m_StatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);

  m_StatusBarFile = new MGUIEStatusBar(this, "File", true, 70);
  m_StatusBarFile->Add("Active:", MGUIEStatusBar::c_Max);

  m_StatusBarGeo = new MGUIEStatusBar(this, "Geometry", true, 70);
  m_StatusBarGeo->Add("Active:", MGUIEStatusBar::c_Max);

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::AddGeometryToFileMenu()
{
  m_MenuFile->AddSeparator();
  m_MenuFile->AddEntry("Load geometry", c_LoadGeometry);
  m_MenuFile->AddPopup("Load previous geometry", m_MenuGeometryHistory);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::UpdateFileHistory()
{
  // Update the file history in the menu "File"
  // --- the same algorithm is used for UpdateGeometryHistory()

  if (m_BaseData != 0) {

    bool First = false;
    if (m_MenuFile->GetEntry(c_FileHistory0) == 0) {
      First = true;
    }

    for (int i = c_FileHistory0; i <= c_FileHistory19; ++i) {
      if (m_MenuFile->GetEntry(i) != 0) {
        m_MenuFile->DeleteEntry(i);
      }
    }
    
    if (First == true && m_BaseData->GetNFileHistories() > 0) {
      m_MenuFile->AddSeparator();
    }

    MString CommonPrefix = "";
    if (m_BaseData->GetNFileHistories() > 1) {
      bool FoundCommon = true; 
      size_t CharacterID = 0;
      do {
        if (m_BaseData->GetFileHistoryAt(0).Length() < CharacterID) break;
        MString Test = m_BaseData->GetFileHistoryAt(0).GetSubString(0, CharacterID);
        for (unsigned int i = 0; i < m_BaseData->GetNFileHistories(); ++i) {
          if (m_BaseData->GetFileHistoryAt(i).BeginsWith(Test) == false) {
            FoundCommon = false;
            break;
          }
        }
        if (FoundCommon == true) {
          CommonPrefix = Test;
          CharacterID++;
        }
      }
      while (FoundCommon == true);
    }

    // If the last character is not "/" then remove the last character:
    while (CommonPrefix.EndsWith("/") == false && CommonPrefix.Length() > 0) {
      CommonPrefix.Remove(CommonPrefix.Length()-1, 1);
    }

    for (unsigned int i = 0; i < m_BaseData->GetNFileHistories(); ++i) {
      MString Name = m_BaseData->GetFileHistoryAt(i);
      if (CommonPrefix.Length() > 7) {
        Name.Replace(0, CommonPrefix.Length(), "... ");
      }
      m_MenuFile->AddEntry(Name, i + c_FileHistory0);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::UpdateGeometryHistory()
{
  // Update the file history in the menu "File"
  // --- the same algorithm is used for UpdateFileHistory()

  if (m_BaseData != 0) {

    for (int i = c_GeometryHistory0; i <= c_GeometryHistory19; ++i) {
      if (m_MenuGeometryHistory->GetEntry(i) != 0) {
        m_MenuGeometryHistory->DeleteEntry(i);
      }
    }

    MString CommonPrefix = "";
    if (m_BaseData->GetNGeometryHistories() > 1) {
      bool FoundCommon = true; 
      size_t CharacterID = 0;
      do {
        if (m_BaseData->GetGeometryHistoryAt(0).Length() < CharacterID) break;
        MString Test = m_BaseData->GetGeometryHistoryAt(0).GetSubString(0, CharacterID);
        for (unsigned int i = 0; i < m_BaseData->GetNGeometryHistories(); ++i) {
          if (m_BaseData->GetGeometryHistoryAt(i).BeginsWith(Test) == false) {
            FoundCommon = false;
            break;
          }
        }
        if (FoundCommon == true) {
          CommonPrefix = Test;
          CharacterID++;
        }
      }
      while (FoundCommon == true);
    }

    // If the last character is not "/" then remove the last character:
    while (CommonPrefix.EndsWith("/") == false && CommonPrefix.Length() > 0) {
      CommonPrefix.Remove(CommonPrefix.Length()-1, 1);
    }

    for (unsigned int i = 0; i < m_BaseData->GetNGeometryHistories(); ++i) {
      MString Name = m_BaseData->GetGeometryHistoryAt(i);
      if (CommonPrefix.Length() > 7) {
        Name.Replace(0, CommonPrefix.Length(), "... ");
      }
      m_MenuGeometryHistory->AddEntry(Name, i + c_GeometryHistory0);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMain::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
      case c_Open:
        Open();
        break;
        
      case c_LoadGeometry:
        ShowGeometryDialog();
        break;
        
      case c_Exit:
        Exit();
        break;
        
      case c_LoadConfig:
        ShowLoadConfigurationDialog();
        break;
        
      case c_SaveConfig:
        ShowSaveConfigurationDialog();
        break;

      case c_About:
        About();
        break;

      default:
        break;
      }
      if (Parameter1 >= c_FileHistory0 && Parameter1 <= c_FileHistory19) {
        m_BaseData->SetCurrentFileName(m_BaseData->GetFileHistoryAt(Parameter1 - c_FileHistory0));
        UpdateConfiguration();
      }
      if (Parameter1 >= c_GeometryHistory0 && Parameter1 <= c_GeometryHistory19) {
        m_BaseInterface->SetGeometry(m_BaseData->GetGeometryHistoryAt(Parameter1 - c_GeometryHistory0));
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


void MGUIMain::CloseWindow()
{
  // Call exit for controlled good-bye

  Exit();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::Exit()
{
  // Exit the application and store all data:

  m_BaseInterface->SaveConfiguration();
  gApplication->Terminate(0);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::SetConfiguration(MSettings* Cfg) 
{
  m_BaseData = Cfg;

  UpdateConfiguration();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMain::UpdateConfiguration()
{
  // Update the Gui

  m_StatusBarFile->SetContent("Active:", m_BaseData->GetCurrentFileName());
  m_StatusBarGeo->SetContent("Active:", m_BaseData->GetGeometryFileName());

  UpdateFileHistory();
  UpdateGeometryHistory();
}


////////////////////////////////////////////////////////////////////////////////




// void MGUIMain::SaveGuiData(MString FileName)
// {
//   m_BaseData->SaveData(FileName);
// }


// ////////////////////////////////////////////////////////////////////////////////


// void MGUIMain::LoadGuiData(MString FileName)
// {
//   // Load the data
//   m_BaseData->ReadData(FileName);

//   // Update the Gui
//   m_StatusBarFile->SetContent("Active:", m_BaseData->GetCurrentFile());
//   m_StatusBarGeo->SetContent("Active:", m_BaseData->GetGeometryFileName());

//   UpdateFileHistory();
//   UpdateGeometryHistory();
// }


////////////////////////////////////////////////////////////////////////////////


bool MGUIMain::ShowLoadConfigurationDialog()
{
  // Load a configuration file...

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;


  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  //Info.fIniDir = gSystem->GetCurrentDirectory();
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_BaseInterface->LoadConfiguration(MString(Info.fFilename));
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMain::ShowGeometryDialog()
{
  // Show the geometry dialog
  // Returns the geometry file name

  MGUIGeometry* Geo = 
    new MGUIGeometry(gClient->GetRoot(), this, m_BaseData);
  gClient->WaitForUnmap(Geo);
  MString Name = Geo->GetGeometryFileName();
  delete Geo;
  for (unsigned int i = 0; i < 100; ++i) {
    gSystem->ProcessEvents();
  }

  if (m_BaseInterface->SetGeometry(Name) == true) {
    UpdateConfiguration();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMain::ShowSaveConfigurationDialog()
{
  // Save a configuration file...

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;
  

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  //Info.fIniDir = StrDup(gSystem->DirName(m_Data->GetCurrentFile()));
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_BaseInterface->SaveConfiguration(MString(Info.fFilename));
  } 

  return true;
}


// MGUIMain: the end...
////////////////////////////////////////////////////////////////////////////////
