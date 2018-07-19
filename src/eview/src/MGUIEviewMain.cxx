/*
 * MGUIEviewMain.cxx
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
// MGUIEviewMain
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEviewMain.h"

// Standard libs:
#include <fstream>
#include <sstream>
using namespace std;

// ROOT libs:
#include <TGFileDialog.h>
#include <TGMsgBox.h>
#include <TGApplication.h>
#include <TView.h>
#include <TSystem.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDefaults.h"
#include "MFile.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MRawEventAnalyzer.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MRETrack.h"
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MTimer.h"
#include "MMuonEvent.h"
#include "MFileManager.h"
#include "MGUIERAlgorithm.h"
#include "MGUIOptionsCoincidence.h"
#include "MGUIOptionsHitClustering.h"
#include "MGUIOptionsTracking.h"
#include "MGUIOptionsCSR.h"
#include "MGUIOptionsGeneral.h"
#include "MPrelude.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIEviewMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEviewMain::MGUIEviewMain(const TGWindow *Parent, const TGWindow *Main)
  : TGMainFrame(Parent, 920, 720, kVerticalFrame)
{
  // Construct an instance of MGUIEviewMain and bring it to the screen

  m_Parent = (TGWindow *) Parent;

  m_Analyzer = 0;
  m_ExitLoop = false;
  m_SizeHistory = 10;
  m_PositionHistory = 0;
  m_IsRotating = false;

  m_Display = 0;
  m_Angles = 0;
  m_Rotations = 0;
  m_Energy = 0;
  m_MinHits = 0;

  m_OptionsLayout = 0;

  m_OptionsFrame = 0;
  m_OptionsFrameLayout = 0;

  m_RightFrame = 0;
  m_RightFrameLayout = 0;

  m_ImageCanvas = 0;
  m_ImageCanvasLayout = 0;
  
  m_Settings = new MSettingsEview();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEviewMain::~MGUIEviewMain()
{
  // Delete an instance of MGUIEviewMain

  delete m_Display;
  delete m_Angles;
  delete m_Rotations;
  delete m_Energy;
  delete m_MinHits;

  delete m_OptionsLayout;

  delete m_OptionsFrame;
  delete m_OptionsFrameLayout;

  delete m_RightFrame;
  delete m_RightFrameLayout;

  delete m_ImageCanvas;
  delete m_ImageCanvasLayout;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Eview <options>"<<endl;
  Usage<<"      -f --filename:"<<endl;
  Usage<<"             Load this simulation/data file"<<endl;
  Usage<<"      -g --geometry:"<<endl;
  Usage<<"             Load this geometry"<<endl;
  Usage<<"      -v --volume:"<<endl;
  Usage<<"             Use this volume as master volume for geomega"<<endl;
  Usage<<"      -c --revan-cfg:"<<endl;
  Usage<<"             Load this revan configuration file"<<endl;
  Usage<<"      -d --debug:"<<endl;
  Usage<<"             Use debug mode"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  Usage<<"  Attention: You have to give the data and geometry file name, due to a chicken and egg problem..."<<endl;
  Usage<<endl;

  // Store some options temporarily:
  MString Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // Single argument
    if (Option == "-f" || Option == "--filename" ||
        Option == "-g" || Option == "--geometry" ||
        Option == "-v" || Option == "--volume" ||
        Option == "-c" || Option == "--revan-cfg") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }   
    // Double argument
//     if (Option == "-c" || Option == "--calibrate") {
//      if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
//        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
//        cout<<Usage<<endl;
//        return false;
//      }
//    }

  }
  

  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--debug" || Option == "-d") {
      if (g_Verbosity < 2) g_Verbosity = 2;
      cout<<"Command-line parser: Use debug mode"<<endl;
    } else if (Option == "--revan-cfg" || Option == "-c") {
      cout<<"Command-line parser: Reading revan configuration file"<<endl;
      m_Settings->Read(argv[++i]);
    } 
  }


//   MString FileName = m_Settings->GetCurrentFileName();
//   MString GeoName = m_Settings->GetGeometryFileName();
  MString FileName = g_StringNotDefined;
  MString GeoName = g_StringNotDefined;
  MString VolName = "";

  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--filename" || Option == "-f") {
      FileName = argv[++i];
      m_Settings->SetCurrentFileName(FileName);
      cout<<"Command-line parser: Using file: "<<FileName<<endl;
    } else if (Option == "--geometry" || Option == "-g") {
      GeoName = argv[++i];
      m_Settings->SetGeometryFileName(GeoName);
      cout<<"Command-line parser: Using geometry: "<<GeoName<<endl;
    } else if (Option == "--volume" || Option == "-v") {
      VolName = argv[++i];
      cout<<"Command-line parser: Using this start volume: "<<VolName<<endl;
    }
  }

  if (FileName == g_StringNotDefined || GeoName == g_StringNotDefined) {
    mgui<<"Please give a simulation/data file name and a geometry file name at the command line."<<show;
    cout<<Usage.str()<<endl;
    return false;
  }

  Create();

  m_Geometry = new MGeometryRevan();
  LoadGeometry(GeoName, VolName);
  if (Open(FileName) == false) {
    mgui<<"Unable to load file: "<<FileName<<endl;
    return false;
  }
    
  // Show change log / license if changed:
  MPrelude P;
  if (P.Play() == false) return false; // license was not accepted

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Create()
{
  // Create the main window

  double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  
  // We start with a name and an icon...
  SetWindowName("Eview - MEGA Event viewer");  

  // In the beginning we build the menus and define their layout, ... 
  m_MenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  m_MenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  m_MenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);
  
  m_MenuFile = new TGPopupMenu(gClient->GetRoot());
  m_MenuFile->AddEntry("Open...", c_Open);
  m_MenuFile->AddSeparator();
  m_MenuFile->AddEntry("Load Geometry", c_LoadGeometry);
  m_MenuFile->AddSeparator();
  m_MenuFile->AddEntry("Exit", c_Exit);

  m_MenuFile->Associate(this);
  m_MenuBar->AddPopup("File", m_MenuFile, m_MenuBarItemLayout);

  m_MenuReconstruction = new TGPopupMenu(fClient->GetRoot());
  m_MenuReconstruction->AddEntry("Selection of algorithm", c_EventReconstruction);
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("Coincidence options", c_Coincidence);
  m_MenuReconstruction->AddEntry("Clustering options", c_Clustering);
  m_MenuReconstruction->AddEntry("Tracking options", c_Tracking);
  m_MenuReconstruction->AddEntry("Sequencing options", c_Sequencing);
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("General event selections", c_EventReconstructionSelection);

  m_MenuReconstruction->Associate(this);
  m_MenuBar->AddPopup("Reconstruction", m_MenuReconstruction, m_MenuBarItemLayout);

  m_MenuInfo = new TGPopupMenu(fClient->GetRoot());
  m_MenuInfo->AddEntry("About", c_About);

  m_MenuInfo->Associate(this);
  m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayout);
  
  AddFrame(m_MenuBar, m_MenuBarLayout);

  // The tool bar:
  m_ToolBar = new MGUIEFlatToolBar(this, FontScaler*600, FontScaler*50, kHorizontalFrame | kRaisedFrame);
  m_ToolBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  AddFrame(m_ToolBar, m_ToolBarLayout);

  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Load.xpm", c_Open, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Start.xpm", c_Loop, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Pause.xpm", c_Stop, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Forward.xpm", c_Next, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Reset.xpm", c_Back, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Rotate.xpm", c_Rotate, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/3D.xpm", c_View3D, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Save.xpm", c_Print, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Exit.xpm", c_Exit, 40);

  // The center fold .... ehm frame...
  m_CenterFrame = new TGCompositeFrame(this, FontScaler*80, FontScaler*60, kHorizontalFrame | kRaisedFrame);
  m_CenterFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandY | kLHintsExpandX | kLHintsTop, 0, 0, 0, 0);
  AddFrame(m_CenterFrame, m_CenterFrameLayout);
  

  // The options:
  m_OptionsFrame = new TGCompositeFrame(m_CenterFrame, FontScaler*230, FontScaler*60, kVerticalFrame | kFixedWidth);
  m_OptionsFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 4, 4, 4, 4);
  m_CenterFrame->AddFrame(m_OptionsFrame, m_OptionsFrameLayout);

  m_OptionsLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, 6, 18, 20, 0);

  // Title:
  MString TitleIcon(g_MEGAlibPath + "/resource/icons/eview/Smaller.xpm");
  MFile::ExpandFileName(TitleIcon);
  
  m_TitlePicture = fClient->GetPicture(TitleIcon);
  if (m_TitlePicture == 0) {
    mgui<<"Can't find picture "<<TitleIcon<<"! Aborting!"<<error;
    return;
  }
  m_TitleIcon = new TGIcon(m_OptionsFrame , m_TitlePicture, m_TitlePicture->GetWidth(), m_TitlePicture->GetHeight());
  m_TitleIconLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 0, 0, 10, 10);
  m_OptionsFrame->AddFrame(m_TitleIcon, m_TitleIconLayout);


  // - Display
  m_Display = new MGUIECBList(m_OptionsFrame, "Display options:");
  m_Display->Add("Reconstruct events", (int) m_Settings->GetReconstruct());
  m_Display->Add("Show only good events", (int) m_Settings->GetDisplayOnlyGoodEvents());
  m_Display->Add("Show sequence", (int) m_Settings->GetDisplaySequence());
  m_Display->Add("Show origin", (int) m_Settings->GetDisplayOrigin());
  m_Display->Create();
  m_OptionsFrame->AddFrame(m_Display, m_OptionsLayout);

  // - Angle
  m_Angles = new MGUIEEntryList(m_OptionsFrame, "Angle:");
  m_Angles->Add("Phi [deg]:", (int) m_Settings->GetAnglePhi(), true, 0, 180);
  m_Angles->Add("Theta [deg]:", (int) m_Settings->GetAngleTheta(), true, 0, 360);
  m_Angles->Create();
  m_OptionsFrame->AddFrame(m_Angles, m_OptionsLayout);

  m_AngleButtonFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, 3, 18, 5, 0);
  m_AngleButtonFrame = new TGCompositeFrame(m_OptionsFrame, 10, 10, kHorizontalFrame);
  m_OptionsFrame->AddFrame(m_AngleButtonFrame, m_AngleButtonFrameLayout);

  m_AngleButtonsLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, 2, 2, 0, 0);

  m_AngleSide = new TGTextButton(m_AngleButtonFrame, "Side", c_Side);
  m_AngleSide->Associate(this);
  m_AngleButtonFrame->AddFrame(m_AngleSide, m_AngleButtonsLayout);

  m_AngleTop = new TGTextButton(m_AngleButtonFrame, "Top", c_Top);
  m_AngleTop->Associate(this);
  m_AngleButtonFrame->AddFrame(m_AngleTop, m_AngleButtonsLayout);

  m_AngleFront = new TGTextButton(m_AngleButtonFrame, "Front", c_Front);
  m_AngleFront->Associate(this);
  m_AngleButtonFrame->AddFrame(m_AngleFront, m_AngleButtonsLayout);

  m_AngleApply = new TGTextButton(m_AngleButtonFrame, "Apply", c_ApplyRotation);
  m_AngleApply->Associate(this);
  m_AngleButtonFrame->AddFrame(m_AngleApply, m_AngleButtonsLayout);


  // - Zoom
  m_Zoom = new TGLabel(m_OptionsFrame, "Move/Zoom:");
  m_ZoomLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 6, 0, 20, 0);
  m_OptionsFrame->AddFrame(m_Zoom, m_ZoomLayout);

  m_ZoomButtonFrameLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, 3, 18, 5, 0);
  m_ZoomButtonFrame = new TGCompositeFrame(m_OptionsFrame, 10, 10, kHorizontalFrame);
  m_OptionsFrame->AddFrame(m_ZoomButtonFrame, m_ZoomButtonFrameLayout);
  
  m_ZoomButtonsLayout = new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsTop, 2, 2, 0, 0);

  m_ZoomXNeg = new TGTextButton(m_ZoomButtonFrame, "X-", c_XNeg);
  m_ZoomXNeg->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomXNeg, m_ZoomButtonsLayout);
  
  m_ZoomXPos = new TGTextButton(m_ZoomButtonFrame, "X+", c_XPos);
  m_ZoomXPos->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomXPos, m_ZoomButtonsLayout);

  m_ZoomYNeg = new TGTextButton(m_ZoomButtonFrame, "Y-", c_YNeg);
  m_ZoomYNeg->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomYNeg, m_ZoomButtonsLayout);
  
  m_ZoomYPos = new TGTextButton(m_ZoomButtonFrame, "Y+", c_YPos);
  m_ZoomYPos->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomYPos, m_ZoomButtonsLayout);

  m_ZoomZNeg = new TGTextButton(m_ZoomButtonFrame, "Z-", c_ZNeg);
  m_ZoomZNeg->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomZNeg, m_ZoomButtonsLayout);
  
  m_ZoomZPos = new TGTextButton(m_ZoomButtonFrame, "Z+", c_ZPos);
  m_ZoomZPos->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomZPos, m_ZoomButtonsLayout);
  
  m_ZoomIn = new TGTextButton(m_ZoomButtonFrame, "+", c_In);
  m_ZoomIn->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomIn, m_ZoomButtonsLayout);
  
  m_ZoomOut = new TGTextButton(m_ZoomButtonFrame, "-", c_Out);
  m_ZoomOut->Associate(this);
  m_ZoomButtonFrame->AddFrame(m_ZoomOut, m_ZoomButtonsLayout);


  // - Rotation:
  m_Rotations = new MGUIEEntryList(m_OptionsFrame, "Angular rotation velocity:");
  m_Rotations->Add("Phi [deg/s]:", (int) m_Settings->GetRotationPhi(), true, 0, 180);
  m_Rotations->Add("Theta [deg/s]:", (int) m_Settings->GetRotationTheta(), true, 0, 360);
  m_Rotations->Create();
  m_OptionsFrame->AddFrame(m_Rotations, m_OptionsLayout);

  // - Energy selection:
  m_Energy = new MGUIEEntryList(m_OptionsFrame, "Energy selection:");
  m_Energy->Add("Min. energy [keV]:", (int) m_Settings->GetEnergyMin(), true, 0);
  m_Energy->Add("Max. energy [keV]:", (int) m_Settings->GetEnergyMax(), true, 0);
  m_Energy->Create();
  m_OptionsFrame->AddFrame(m_Energy, m_OptionsLayout);

  // - Hit selection:
  m_MinHits = new MGUIEEntryList(m_OptionsFrame, "Minimum number of hits:");
  m_MinHits->Add("Total:", m_Settings->GetMinHitsTotal(), true, 0);
  m_MinHits->Add("D1:", m_Settings->GetMinHitsD1(), true, 0);
  m_MinHits->Add("D2:", m_Settings->GetMinHitsD2(), true, 0);
  m_MinHits->Create();
  m_OptionsFrame->AddFrame(m_MinHits, m_OptionsLayout);
 

  // The image:
  m_RightFrame = new TGCompositeFrame(m_CenterFrame, FontScaler*60, FontScaler*60, kVerticalFrame);
  m_RightFrameLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY | kLHintsRight, 0, 4, 4, 4);
  m_CenterFrame->AddFrame(m_RightFrame, m_RightFrameLayout);

  m_ImageCanvas = new TRootEmbeddedCanvas("Event View", m_RightFrame, GetWidth()-FontScaler*100, GetHeight());
  m_ImageCanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX |
                                          kLHintsExpandY, 0, 0, 0, 0);
  m_ImageCanvas->GetCanvas()->Clear();
  m_ImageCanvas->GetCanvas()->SetFillColor(0);
  m_ImageCanvas->GetCanvas()->SetFrameBorderSize(0);
  m_ImageCanvas->GetCanvas()->SetFrameBorderMode(0);
  m_ImageCanvas->GetCanvas()->SetBorderSize(0);
  m_ImageCanvas->GetCanvas()->SetBorderMode(0);

  m_RightFrame->AddFrame(m_ImageCanvas, m_ImageCanvasLayout);

  // The status bar  
  m_StatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);

  m_StatusBarEvent = new MGUIEStatusBar(this, "Event", true, FontScaler*50);
  m_StatusBarEvent->Add("Id:", FontScaler*60);
  m_StatusBarEvent->Add("Type:", FontScaler*110);
  m_StatusBarEvent->SetContent("Type:", "No event!");
  m_StatusBarEvent->Add("Energy [keV]:", FontScaler*60);
  m_StatusBarEvent->Add("Framerate [fps]:", FontScaler*30);
  m_StatusBarEvent->Create();

  AddFrame(m_StatusBarEvent, m_StatusBarLayout);

  MapSubwindows();
  MapWindow();  
  Layout();

  gSystem->ProcessEvents();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::ProcessMessage(long Message, long Parameter1, 
                                     long Parameter2)
{
  // Process the messages for this application

  SetOptions();

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
      case c_Open:
        Open();
        break;
        
      case c_EventReconstruction: 
        new MGUIERAlgorithm(gClient->GetRoot(), this, m_Settings);
        break;

      case c_EventReconstructionSelection:
        new MGUIOptionsGeneral(gClient->GetRoot(), this, m_Settings);
        break;

      case c_Tracking:
        new MGUIOptionsTracking(gClient->GetRoot(), this, m_Settings, m_Geometry);
        break;

      case c_Coincidence:
        new MGUIOptionsCoincidence(gClient->GetRoot(), this, m_Settings);
        break;

      case c_Clustering:
        new MGUIOptionsHitClustering(gClient->GetRoot(), this, m_Settings);
        break;

      case c_Sequencing:
        new MGUIOptionsCSR(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_LoadGeometry:
        LoadGeometry();
        break;
        
      case c_StoreImage:
        Save();
        break;
        
      case c_Print:
        Print();
        break;
        
      case c_Loop:
        Loop();
        break;
        
      case c_Stop:
        ExitLoop();
        break;
        
      case c_Next:
        Next();
        break;
        
      case c_Back:
        Back();
        break;
        
      case c_Exit:
        Exit();
        break;
        
      case c_Side:
        ApplyRotation(90, 0);
        break;
        
      case c_Top:
        ApplyRotation(0, 180);
        break;
        
      case c_Front:
        ApplyRotation(90, 90);
        break;
        
      case c_XNeg:
        ApplyZoomMove(-1, 0, 0, 1);
        break;
        
      case c_XPos:
        ApplyZoomMove(1, 0, 0, 1);
        break;
        
      case c_YNeg:
        ApplyZoomMove(0, -1, 0, 1);
        break;
        
      case c_YPos:
        ApplyZoomMove(0, 1, 0, 1);
        break;
        
      case c_ZNeg:
        ApplyZoomMove(0, 0, -1, 1);
        break;
        
      case c_ZPos:
        ApplyZoomMove(0, 0, 1, 1);
        break;
        
      case c_In:
        ApplyZoomMove(0, 0, 0, 0.95);
        break;
        
      case c_Out:
        ApplyZoomMove(0, 0, 0, 1.05);
        break;

      case c_ApplyRotation:
        ApplyRotation();
        break;
        
      case c_View3D:
        View3D();
        break;

      case c_Rotate:
        Rotate();
        break;
        
      case c_About:
        About();
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


void MGUIEviewMain::CloseWindow()
{
  Exit();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::Open(MString FileName)
{
  // Open a file

  if (m_Geometry->IsScanned() == false) {
    mgui<<"Please load a geometry first!"<<info;
    if (LoadGeometry() == false) {
      mgui<<"Unable to load geometry file!"<<error;
      ExitLoop();
      return false;
    }
  }

  if (FileName == "") {
    const char** Types = new const char*[8];
    Types[0] = "Simulated events";
    Types[1] = "*.sim";
    Types[2] = "Calibrated events";
    Types[3] = "*.evta";
    Types[4] = "All files";
    Types[5] = "*";
    Types[6] = 0;
    Types[7] = 0;
    
    TGFileInfo Info;
    Info.fFileTypes = (const char **) Types;
    new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
    
    delete [] Types;
    
    // Get the filename ...
    if ((char *) Info.fFilename != 0) {
      FileName = MString((char *) Info.fFilename);
      if (FileName == "") {
        ExitLoop();
        return false;
      }
    } 
    // ... or return when cancel has been pressed
    else {
      ExitLoop();
      return false;
    }
  }
  
  if (MFile::Exists(FileName) == false) {
    mgui<<"File does not exist: "<<FileName<<endl;
    ExitLoop();
    return false;
  }

  if (m_Analyzer != 0) delete m_Analyzer;
  m_Analyzer = new MRawEventAnalyzer();
  m_Analyzer->SetGeometry(m_Geometry);
  if (m_Analyzer->SetInputModeFile(FileName) == false) return false;
  
  m_Analyzer->SetSettings(m_Settings);
  m_Analyzer->SetBatch(true);

  if (m_Analyzer->PreAnalysis() == false) {
    mgui<<"Pre-analysis of your input failed. Please check the input file as well as the revan configuration file!"<<show;
    delete m_Analyzer;
    m_Analyzer = 0;
    ExitLoop();
    return false;
  }

  m_Settings->SetCurrentFileName(FileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Loop()
{
  // Start an event loop

  m_IsLooping = true;
  m_ExitLoop = false;

  while (m_ExitLoop == false) {
    Next();
    gSystem->ProcessEvents();
  }

  m_IsLooping = false;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::ExitLoop()
{
  // Set all flags to enable stop looping over all events...

  m_ExitLoop = true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Save()
{
  // Save the current event

}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::SetOptions(bool Update)
{
  // Exit the application and store all data:

  m_Settings->SetReconstruct(m_Display->IsSelected(0));
  m_Settings->SetDisplayOnlyGoodEvents(m_Display->IsSelected(1));
  m_Settings->SetDisplaySequence(m_Display->IsSelected(2));
  m_Settings->SetDisplayOrigin(m_Display->IsSelected(3));


  // The current angle is theoretically stored 3 times:
  // a. GUI  b. Data  c. Canvas

  // if the Gui data has been modified then this is most important:
  if (Update == true || m_Angles->GetAsInt(0) != m_Settings->GetAnglePhi() ||
      m_Angles->GetAsInt(1) != m_Settings->GetAngleTheta()) {
    m_Settings->SetAnglePhi(m_Angles->GetAsInt(0));
    m_Settings->SetAngleTheta(m_Angles->GetAsInt(1));

    ShowGeometry();
  }
  // Otherwise store the
    

  m_Settings->SetRotationPhi(m_Rotations->GetAsInt(0));
  m_Settings->SetRotationTheta(m_Rotations->GetAsInt(1));

  m_Settings->SetEnergyMin(m_Energy->GetAsInt(0));
  m_Settings->SetEnergyMax(m_Energy->GetAsInt(1));

  m_Settings->SetMinHitsTotal(m_MinHits->GetAsInt(0));
  m_Settings->SetMinHitsD1(m_MinHits->GetAsInt(1));
  m_Settings->SetMinHitsD2(m_MinHits->GetAsInt(2));
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::About()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Eview");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  About->Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Exit()
{
  // Exit the application and store all data:

  SetOptions();

  m_Settings->Write();

  gApplication->Terminate(0);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::LoadGeometry(MString FileName, MString StartVolumeName)
{
  // Load the geometry

  if (FileName.IsEmpty()) {
    MGUIGeometry* Geo = new MGUIGeometry(gClient->GetRoot(), m_Parent, m_Settings);
    gClient->WaitForUnmap(Geo);
    if (MFile::Exists(Geo->GetGeometryFileName()) == true) {
      m_Settings->SetGeometryFileName(Geo->GetGeometryFileName());
      FileName = Geo->GetGeometryFileName();
    }
    delete Geo;
  }

  m_Geometry->SetStartVolumeName(StartVolumeName);
  bool Success = m_Geometry->ScanSetupFile(FileName);
  
  if (Success == true) {
    SetOptions();
    ShowGeometry(true);
  }

  delete m_Analyzer;
  m_Analyzer = 0;

  return Success;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::ShowGeometry(bool Redraw)
{
  if (m_Geometry == 0) return false;

  m_Canvas = m_ImageCanvas->GetCanvas();
  if (m_Canvas->GetView() == 0) {
    m_Geometry->DrawGeometry(m_Canvas);
  }
  m_View = m_ImageCanvas->GetCanvas()->GetView();
  if (Redraw == true) {
    m_Geometry->DrawGeometry(m_Canvas); 
    m_Canvas->Update();
    m_View = m_ImageCanvas->GetCanvas()->GetView();
  }

  m_View->SetRange(m_Settings->GetPositionXNeg(), m_Settings->GetPositionYNeg(), 
                   m_Settings->GetPositionZNeg(), m_Settings->GetPositionXPos(), 
                   m_Settings->GetPositionYPos(), m_Settings->GetPositionZPos());
  m_View->RotateView(m_Settings->GetAngleTheta(), m_Settings->GetAnglePhi());
  m_View->AdjustScales();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::ApplyRotation(double Phi, double Theta)
{
  // Main RotationLoop

  if (Phi != c_UseGuiValues) {
    m_Angles->SetValue(0, Phi);
  }
  if (Theta != c_UseGuiValues) {
    m_Angles->SetValue(1, Theta);
  }
  SetOptions();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::ApplyZoomMove(double X, double Y, double Z, double In)
{
  // Main RotationLoop

//   m_Settings->SetPositionXNeg((m_Settings->GetPositionXNeg() + X)*In);
//   m_Settings->SetPositionXPos((m_Settings->GetPositionXPos() + X)*In);
//   m_Settings->SetPositionYNeg((m_Settings->GetPositionYNeg() + Y)*In);
//   m_Settings->SetPositionYPos((m_Settings->GetPositionYPos() + Y)*In);
//   m_Settings->SetPositionZNeg((m_Settings->GetPositionZNeg() + Z)*In);
//   m_Settings->SetPositionZPos((m_Settings->GetPositionZPos() + Z)*In);

  double Move = 0.1;

  double xMove = (m_Settings->GetPositionXPos() - m_Settings->GetPositionXNeg())*Move;
  m_Settings->SetPositionXNeg((m_Settings->GetPositionXNeg() + X*xMove)*In);
  m_Settings->SetPositionXPos((m_Settings->GetPositionXPos() + X*xMove)*In);
  double yMove = (m_Settings->GetPositionYPos() - m_Settings->GetPositionYNeg())*Move;
  m_Settings->SetPositionYNeg((m_Settings->GetPositionYNeg() + Y*yMove)*In);
  m_Settings->SetPositionYPos((m_Settings->GetPositionYPos() + Y*yMove)*In);
  double zMove = (m_Settings->GetPositionZPos() - m_Settings->GetPositionZNeg())*Move;
  m_Settings->SetPositionZNeg((m_Settings->GetPositionZNeg() + Z*zMove)*In);
  m_Settings->SetPositionZPos((m_Settings->GetPositionZPos() + Z*zMove)*In);

  SetOptions(true);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Rotate()
{
  // Main RotationLoop

  m_IsRotating = !m_IsRotating;

  if (m_IsRotating == true) {
    RotationLoop();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::RotationLoop()
{
  massert(m_View != 0);

  // Calculate frames per second
  bool Init = true;
  const int TimerLength = 30; // Frames
  int NFrames = 0;
  MTimer Timer;

  // Variables for rotation calculation:
  MTimer GlobalTimer;
  double Theta = 0;
  double Phi = 0;

  MTimer FrameTimer;
  while (m_IsRotating == true) {
    if (NFrames % TimerLength == 0) {
      if (Init == false) {
        m_StatusBarEvent->
          SetContent("Framerate [fps]:", (int) (TimerLength/Timer.ElapsedTime()));
        Timer.Start();
      } else {
        Init = false;
        Timer.Start();
      }
    }

    SetOptions();

    // mimp<<"At some point the view vector changed!"<<show;
    // m_View = m_ImageCanvas->GetCanvas()->GetView();

    // Now rotate:
    // The rotation value from the GUI is an angular velocity
    // Translate this velocity into an angle:
    Theta = m_View->GetLongitude() + m_Settings->GetRotationTheta() * GlobalTimer.ElapsedTime();
    Phi = m_View->GetLatitude() + m_Settings->GetRotationPhi() * GlobalTimer.ElapsedTime();
    GlobalTimer.Start();

    // Stimmt nicht!
     while (Phi > 360) {
       Phi -= 360;
     }
     while (Phi < 0) {
      Phi += 360;
     }
     while (Theta > 360) {
       Theta -= 360;
     }
     while (Theta < 0) {
       Theta += 180;
     }

    m_View->RotateView(Theta, Phi);
    m_Canvas->Update();

    gSystem->ProcessEvents();
    gClient->HandleInput();

    NFrames++;
    
    // Check for any other key-press or input
    while (FrameTimer.GetElapsed() < 1.0/50.0) {
      gSystem->Sleep(10);
      gSystem->ProcessEvents();
      gClient->HandleInput();
    }
    FrameTimer.Reset();
  }
  m_Canvas->Update();

  // Store the current rotation:
  m_Settings->SetAnglePhi(m_View->GetLatitude());
  m_Settings->SetAngleTheta(m_View->GetLongitude());
  m_Angles->SetValue(0, m_View->GetLatitude());
  m_Angles->SetValue(1, m_View->GetLongitude());
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::View3D()
{
  // Display the current image in 3D

  //gROOT->ProcessLine("R__x3d->CloseWindow()");

  m_Canvas->ResetView3D();
  m_Canvas->x3d();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Print()
{
  cout<<"Trying to print the current image...."<<endl;

  const char** Types = new const char*[10];
  Types[0] = "Encapsulated postscript";
  Types[1] = "*.eps";
  Types[2] = "Postscript";
  Types[3] = "*.ps";
  Types[4] = "Graphics Interchange Format";
  Types[5] = "*.gif";
  Types[6] = "All files";
  Types[7] = "*";
  Types[8] = 0;
  Types[9] = 0;

  MString FileName = "Image.eps";
  MFileManager FM;
  if (FM.SelectFileToSave(FileName, Types) == false) {
    return;
  }

  delete [] Types;

  m_Canvas->SaveAs(FileName);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Next()
{
  // Switch to next event

  // Store the current rotation:
  m_Settings->SetAnglePhi(m_View->GetLatitude());
  m_Settings->SetAngleTheta(m_View->GetLongitude());
  m_Angles->SetValue(0, m_View->GetLatitude());
  m_Angles->SetValue(1, m_View->GetLongitude());

  // If we moved back in history, then move forward in the event history
  // until we have to calculate a new event...
  if (m_PositionHistory != 0) {
    int i;
    list<MRERawEvent*>::iterator iter;
    for (iter=m_EventHistory.begin(), i=0; 
         iter != m_EventHistory.end(); 
         iter++, i++) {
      if (i == m_PositionHistory-1) {
        SelectAndDraw(*iter);
        m_PositionHistory--;
        return;
      }
    }
  }

  if (m_Analyzer == 0) {
    mgui<<"Please load an event file first!"<<info;
    if (Open() == false) return;
    if (m_Analyzer == 0) return;
  }

  MRERawEvent* RE = 0;
  while (true) {
    gSystem->ProcessEvents();
    SetOptions();
    RE = 0;
    if (m_Settings->GetReconstruct() == true) {
      if (m_Analyzer->AnalyzeEvent() != MRawEventAnalyzer::c_AnalysisSucess) {
        mgui<<"Analysis error or no more events available. Sorry."<<info;
        delete m_Analyzer;
        m_Analyzer = 0;
        ExitLoop();
        return;
      } else {
        if (m_Analyzer->GetSingleOptimumEvent() != 0) {
          RE = m_Analyzer->GetSingleOptimumEvent()->Duplicate();
        } else if (m_Settings->GetDisplayOnlyGoodEvents() == false) {
          RE = m_Analyzer->GetInitialRawEvent()->Duplicate();
        }
      }
    } else {
      if ((RE = m_Analyzer->GetNextInitialRawEventFromFile()) == 0) {
        mgui<<"No more events available. Sorry."<<info;
        delete m_Analyzer;
        m_Analyzer = 0;
        ExitLoop();
        return;
      } 
    }

    // Temporarily store the event:
    if ((int) m_EventHistory.size() > m_SizeHistory) {
      delete m_EventHistory.back();
      m_EventHistory.pop_back();
    }

    if (RE != 0) {
      // Store the event:
      m_EventHistory.push_front(RE);
   
      // View the event
      //if (SelectAndDraw(m_Analyzer->GetInitialRawEvent()) == true) {
      if (SelectAndDraw(m_EventHistory.front()) == true) {
        break;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewMain::Back()
{
  // Switch to next event

  if (m_EventHistory.size() == 0) {
    mgui<<"You can not move one event back!"<<endl;
    mgui<<"The event history is empty!"<<info;
    return;
  }

  int i;
  list<MRERawEvent*>::iterator iter;
  bool IsDrawn = false;
  for (iter=m_EventHistory.begin(), i=0; 
       iter != m_EventHistory.end(); 
       iter++, i++) {
    if (i == m_PositionHistory+1) {
      SelectAndDraw(*iter);
      IsDrawn = true;
      m_PositionHistory++;
      break;
    }
  }

  if (IsDrawn == false) {
    mgui<<"No more events left in storage!"<<info;
    return;
  }


  return;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewMain::SelectAndDraw(MRERawEvent* RE)
{
  // Check if this event has the parameters of a selected event and then draw it

  if (RE == 0) return false; 

  if (m_Settings->GetDisplayOnlyGoodEvents() == true) {
    if (RE->IsGoodEvent() == false) {
      mout<<"Event deselected: Not good"<<endl;
      return false; 
    }
  }


  const bool ShowLinks = m_Settings->GetDisplaySequence(); 
  const bool ShowOrigin = m_Settings->GetDisplayOrigin(); 

  const int MinNHitsTotal = m_Settings->GetMinHitsTotal();
  const int MinNHitsD1 = m_Settings->GetMinHitsD1();
  const int MinNHitsD2 = m_Settings->GetMinHitsD2();
  const double MinEnergy = m_Settings->GetEnergyMin();
  const double MaxEnergy = m_Settings->GetEnergyMax();

  const double SizeHit = 0.3;
  const double SizeLink = 0.05;
  const double SizeSequence = 0.1;

  // Inverted
//   const int ColorHit = 1;
//   const int ColorLink = 7;
//   const int ColorSequence = 6;
//   const int ColorOrigin = 9;

  // Not inverted
  const int ColorHit = 4;
  const int ColorTrackStart = 1;
  int ColorLink = 2;
  const int ColorSequence = 8;
  const int ColorOrigin = 6;

  
  int NHitsTotal = 0;
  int NHitsD1 = 0;
  int NHitsD2 = 0;
  MRESE* RESE;
  MVector Pos;

  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    ++NHitsTotal;
    if (RE->GetRESEAt(i)->GetDetector() == 1) {
      Pos = RE->GetRESEAt(i)->GetPosition();
      ++NHitsD1;
    } else if (RE->GetRESEAt(i)->GetDetector() == 2) {
      ++NHitsD2;
    }
  }
    

  // Hit selection:
  if (NHitsTotal < MinNHitsTotal) { 
    mout<<"Event deselected: Not enough hits: "<<NHitsTotal<<endl;
    return false; 
  }
  if (NHitsD1 < MinNHitsD1) { 
    mout<<"Event deselected: Not enough hits in D1: "<<NHitsD1<<endl;
    return false; 
  }
  if (NHitsD2 < MinNHitsD2) { 
    mout<<"Event deselected: Not enough hits in D2: "<<NHitsD2<<endl;
    return false; 
  }
    
  // Energy selection
  if (RE->GetEnergy() < MinEnergy || RE->GetEnergy() > MaxEnergy) {
    mout<<"Event deselcted: Energy not within bounds: "<<RE->GetEnergy()<<endl;
    return false; 
  }
 
  // Now clear the geometry ... 
  m_Geometry->RemoveAllHits();
  m_Geometry->RemoveAllLinks();
  
  // ... draw the hits...

  MRETrack* Track = 0; 
  MRESE* LastTrackRESE = 0;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    RESE = RE->GetRESEAt(i);
    bool IsStart = false;
    if (RESE == RE->GetStartPoint()) {
      IsStart = true;
    }
    if (RESE->GetType() == MRESE::c_Track) {
      Track = (MRETrack*) RESE;
      Track->ResetIterator();
      LastTrackRESE = 0;
      for (int h = 0; h < Track->GetNRESEs(); ++h) {
        RESE = Track->GetRESEAt(h);
        ostringstream Name;
        Name<<"Hit "<<RESE->GetID()<<": "<<RESE->GetEnergy()<<" keV";
        if (RESE == Track->GetStartPoint()) {
          m_Geometry->AddHit(Name.str().c_str(), RESE->GetPosition(), SizeHit, ColorTrackStart);
        } else {
          m_Geometry->AddHit(Name.str().c_str(), RESE->GetPosition(), SizeHit, ColorHit);
        }
      }
      while ((RESE = Track->GetNextRESE()) != 0) {
        //m_Geometry->AddHit(RESE->GetPosition(), SizeHit, ColorHit);
        if (LastTrackRESE != 0 && ShowLinks == true) {
          m_Geometry->AddLink(LastTrackRESE->GetPosition(), RESE->GetPosition(), SizeLink, ColorLink);
        }
        LastTrackRESE = RESE;
      }
      ColorLink += 2;
    } else if (RESE->GetType() == MRESE::c_Cluster) {
      ostringstream Name;
      Name<<"Hit "<<RESE->GetID()<<": "<<RESE->GetEnergy()<<" keV";
      for (int r = 0; r < RESE->GetNRESEs(); ++r) {
        if (IsStart == true) {
          m_Geometry->AddHit(Name.str().c_str(), RESE->GetRESEAt(r)->GetPosition(), SizeHit, ColorOrigin);
        } else {
          m_Geometry->AddHit(Name.str().c_str(), RESE->GetRESEAt(r)->GetPosition(), SizeHit, ColorHit);
        }
      }
    } else if (RESE->GetType() == MRESE::c_Hit) {
      ostringstream Name;
      Name<<"Hit "<<RESE->GetID()<<": "<<RESE->GetEnergy()<<" keV";
      if (IsStart == true) {
        m_Geometry->AddHit(Name.str().c_str(), RESE->GetPosition(), SizeHit, ColorOrigin);
      } else {
        m_Geometry->AddHit(Name.str().c_str(), RESE->GetPosition(), SizeHit, ColorHit);
      }
    } else {
      mout<<"Unknown RESE!"<<endl;
    }
  }
  
  // Add the compton sequence
  if (ShowLinks == true) {
    if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {
      MRESE* Before = RE->GetStartPoint();
      MRESE* Next = Before->GetLinkAt(0);
      MRESE* After = 0;
      m_Geometry->AddLink(Before->GetPosition(), Next->GetPosition(), SizeSequence, ColorSequence);
      while ((After = Next->GetOtherLink(Before)) != 0) {
        m_Geometry->AddLink(Next->GetPosition(), After->GetPosition(), SizeSequence, ColorSequence);
        Before = Next;
        Next = After;
        After = 0;
      }
    }
  }
  

  // Add the origins:
  if (ShowOrigin == true) {
    cout<<"Type: "<<RE->GetEventType()<<endl;
    if (RE->GetEventType() == MRERawEvent::c_MipEvent) {
      MMuonEvent* Muon = (MMuonEvent*) RE->GetPhysicalEvent();

      MVector Direction = Muon->GetDirection();

      MVector Start = 
        Muon->GetCenterOfGravity() + 
        Direction*
        (30-Muon->GetCenterOfGravity().Z())*(1.0/Direction.Z());
      MVector Stop = 
        Muon->GetCenterOfGravity() + 
        Direction*
        (-5-Muon->GetCenterOfGravity().Z())*(1.0/Direction.Z());
      m_Geometry->AddLink(Start, Stop, 0.03, ColorOrigin);
      cout<<"Adding: "<<Start.X()<<"!"<<Start.Y()<<"!"<<Start.Z()<<" --  "<<Stop.X()<<"!"<<Stop.Y()<<"!"<<Stop.Z()<<endl;
    } else if (RE->GetEventType() == MRERawEvent::c_ComptonEvent) {
      cout<<"Compton origin missing"<<endl;
    }
  }


  // Set the event type:
  m_StatusBarEvent->SetContent("Id:", RE->GetEventID());
  m_StatusBarEvent->SetContent("Type:", RE->GetEventTypeAsString());
  m_StatusBarEvent->SetContent("Energy [keV]:", (int) RE->GetEnergy());

  // ... and bring it to screen...
  ShowGeometry(true);
  
  return true;
}

// MGUIEviewMain: the end...
////////////////////////////////////////////////////////////////////////////////
