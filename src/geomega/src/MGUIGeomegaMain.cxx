/*
 * MGUIGeomegaMain.cxx
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
#include "MGUIGeomegaMain.h"

// Standard libs:

// ROOT libs:
#include <TGPicture.h>

// MEBAlib libs:
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUIMGeant.h"
#include "MGUIPosition.h"
#include "MGUIPathLength.h"
#include "MInterfaceGeomega.h"
#include "MSystem.h"
#include "MFile.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIGeomegaMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIGeomegaMain::MGUIGeomegaMain(MInterfaceGeomega* Interface, MSettingsGeomega* Data)
  : MGUIMain(660, 270, Interface, Data)
{
  m_Interface = Interface;
  m_Data = Data;
}


////////////////////////////////////////////////////////////////////////////////


MGUIGeomegaMain::~MGUIGeomegaMain()
{
  // Standard destructor - in all normal exit cases the session have already 
  // been deleted in CloseWindow
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::Create()
{
  // Create the main window
  
  MGUIMain::Create();

  // We start with a name and an icon...
  SetWindowName("Geomega - GEOmetry for MEGA");  

  m_MenuAnalysis = new TGPopupMenu(fClient->GetRoot());
  m_MenuAnalysis->AddEntry("Scan setup file", c_Scan);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("View geometry", c_View);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Calculate masses", c_Masses);
  m_MenuAnalysis->AddEntry("Determine detector resolutions", c_Resolutions);
  m_MenuAnalysis->AddEntry("Dump geometry information", c_Dump);
  m_MenuAnalysis->AddEntry("Position/volume information", c_Position);
  m_MenuAnalysis->AddEntry("Determine path lengths", c_PathLengths);
  m_MenuAnalysis->AddEntry("Check for overlaps", c_Intersect);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Create Geant3 files", c_WriteG3);
  m_MenuAnalysis->AddEntry("Create MGeant/MGGPOD files", c_WriteMG);

  // If cosima does not exist disable the overlap check options
  if (MFile::Exists(g_MEGAlibPath + "/bin/cosima") == false) {
    m_MenuAnalysis->GetEntry(c_Intersect)->GetLabel()->SetString("Check for overlaps - no cosima");
    m_MenuAnalysis->DisableEntry(c_Intersect);
  }
  m_MenuAnalysis->Associate(this);
  m_MenuBar->AddPopup("Analysis", m_MenuAnalysis, m_MenuBarItemLayoutLeft);


  // Only add the info here at the end
  m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayoutRight);

  // Add buttons to the tool bar:
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Load.xpm", c_Open, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Start.xpm", c_View, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Exit.xpm", c_Exit, 20);

  // Title:
  MString TitleIcon(g_MEGAlibPath + "/resource/icons/geomega/Large.xpm");
  MFile::ExpandFileName(TitleIcon);
  
  m_TitlePicture = fClient->GetPicture(TitleIcon);
  if (m_TitlePicture == 0) {
    mgui<<"Can't find picture "<<TitleIcon<<"! Aborting!"<<error;
    return;
  }
  m_TitleIcon = new TGIcon(this, m_TitlePicture, m_TitlePicture->GetWidth(), m_TitlePicture->GetHeight());
  m_TitleIconLayout = new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 0, 0, 10, 10);
  AddFrame(m_TitleIcon, m_TitleIconLayout);

  // The status bar
  m_StatusBarFile->SetContent("Active:", MFile::GetBaseName(m_Data->GetCurrentFileName()));
  m_StatusBarFile->Create();
  AddFrame(m_StatusBarFile, m_StatusBarLayout);


  MapSubwindows();
  MapWindow();  
  Layout();

  // We do not have a file status-bar, so unload it...
  m_StatusBarGeo->UnmapWindow();

  gSystem->ProcessEvents();

  // Now set the minimum and maximum size of the display:
  int xDisplay, yDisplay;
  unsigned int wDisplay, hDisplay;
  gVirtualX->GetGeometry(-1, xDisplay, yDisplay, wDisplay, hDisplay);
  SetWMSizeHints(660, 300, wDisplay, hDisplay, 0, 0);

  UpdateConfiguration();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIGeomegaMain::ProcessMessage(long Message, long Parameter1, 
                                     long Parameter2)
{
  // Process the messages for this application

  if (MGUIMain::ProcessMessage(Message, Parameter1, Parameter2) == false) {
    return false;
  }

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {

      case c_Scan:
        m_Interface->ReadGeometry();
        break;

      case c_View:
        m_Interface->ViewGeometry();
        break;

      case c_Intersect:
        m_Interface->TestIntersections();
        break;

      case c_Masses:
        m_Interface->CalculateMasses();
        break;

      case c_Resolutions:
        m_Interface->GetResolutions();
        break;

      case c_Dump:
        m_Interface->DumpInformation();
        break;

      case c_WriteG3:
        m_Interface->WriteGeant3Files();
        break;

      case c_WriteMG:
        WriteMGeant();
        break;

      case c_Position:
        Position();
        break;

      case c_PathLengths:
        PathLengths();
        break;

      case c_Test:
        m_Interface->TestOnly();
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


void MGUIGeomegaMain::Open()
{
  // Open a file

  const char** Types = new const char*[8];
  Types[0] = "Geometry setup files";
  Types[1] = "*.setup";
  Types[2] = "Geometry files";
  Types[3] = "*.geo";
  Types[4] = "All files";
  Types[5] = "*";
  Types[6] = 0;
  Types[7] = 0;
  
  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  Info.fIniDir = StrDup(gSystem->DirName(m_Data->GetCurrentFileName()));
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Data->SetCurrentFileName(MString(Info.fFilename));
    UpdateConfiguration();
  } 
  // ... or return when cancel has been pressed
  else {
    return;
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::WriteMGeant()
{
  // Write the geometry in MGeant format
  
  bool OkPressed = false;
  new MGUIMGeant(gClient->GetRoot(), this, m_Data, OkPressed);
  
  if (OkPressed == true) {
    m_Interface->WriteMGeantFiles();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::Position()
{
  // At the given position determine the volume hierarachy
  
  bool OkPressed = false;
  new MGUIPosition(gClient->GetRoot(), this, m_Data, OkPressed);
  
  if (OkPressed == true) {
    m_Interface->FindVolume(m_Data->GetPosition());
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::PathLengths()
{
  // Determine the absorption along a path
  
  bool OkPressed = false;
  new MGUIPathLength(gClient->GetRoot(), this, m_Data, OkPressed);
  
  if (OkPressed == true) {
    m_Interface->GetPathLengths(m_Data->GetPathLengthStart(), m_Data->GetPathLengthStop());
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::About()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Geomega");
  About->SetIconPath(g_MEGAlibPath + "/resource/icons/geomega/Small.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  About->SetReference("Includes a chapter about the basics of Geomega", 
                      "A. Zoglauer et al., \"Cosima - The cosmic simulator of MEGAlib\", IEEE Nuclear Science Symposium Conference Record 2009, pp. 2053-2059, 2009");  
  About->Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIGeomegaMain::Launch()
{
  // This method is called after pressing the MEGA-button in the 
  // M.I.Works main GUI:
  // The reconstruction is started.

  MSystem Sys;
  MString Suffix;
  Sys.GetFileSuffix(m_Data->GetCurrentFileName(), &Suffix);

  if (Suffix != "setup") {
    mgui<<"Setup-file needs to have the suffix \"setup\"!"<<error;
    return;
  }

  m_Interface->ReadGeometry();
  m_Interface->ViewGeometry();
}


// MGUIGeomegaMain: the end...
////////////////////////////////////////////////////////////////////////////////
