/*
* MGUIMainFretalon.cxx
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
#include "MGUIMainFretalon.h"

// Standard libs:

// ROOT libs:
#include <KeySymbols.h>
#include <TApplication.h>
#include <TGPicture.h>
#include <TStyle.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TGClient.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIDefaults.h"
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUIModuleSelector.h"
#include "MGUIEFileSelector.h"

// Nuclearizer libs:
#include "MModule.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMainFretalon)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMainFretalon::MGUIMainFretalon(MSupervisor* Supervisor)
  : TGMainFrame(gClient->GetRoot(), 350, 300, kVerticalFrame),
    m_Supervisor(Supervisor)
{
  gStyle->SetPalette(1, 0);

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  BindKey(this, gVirtualX->KeysymToKeycode(kKey_L), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_S), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_E), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Return), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Enter), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Escape), kAnyModifier);

  m_ProgramName = "Unnamed program";
  m_PicturePath = "";
  m_SubTitle = "No sub title set!";
  m_LeadAuthor = "No lead author";
}


////////////////////////////////////////////////////////////////////////////////


MGUIMainFretalon::~MGUIMainFretalon()
{
  // Deep Cleanup automatically deletes all used GUI elements
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMainFretalon::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Nuclearizer");  

  double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();

  // In the beginning we build the menus and define their layout, ... 
  TGLayoutHints* MenuBarItemLayoutLeft = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  //TGLayoutHints* MenuBarItemLayoutRight = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  TGLayoutHints* MenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  TGMenuBar* MenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);
  AddFrame(MenuBar, MenuBarLayout);

  TGPopupMenu* MenuOptions = new TGPopupMenu(gClient->GetRoot());
  MenuOptions->AddLabel("Configuration file");
  MenuOptions->AddEntry("Open", c_LoadConfig);
  MenuOptions->AddEntry("Save As", c_SaveConfig);
  MenuOptions->AddSeparator();
  MString Geo = MString("Geometry file");
  if (m_Supervisor->GetGeometry() != 0) {
    Geo += " (current: ";
    Geo += m_Supervisor->GetGeometry()->GetName(); 
    Geo += ")";
  }
  MenuOptions->AddLabel(Geo);
  MenuOptions->AddEntry("Open", c_Geometry);
  MenuOptions->AddSeparator();
  MenuOptions->AddEntry("Exit", c_Exit);
  MenuOptions->Associate(this);
  MenuBar->AddPopup("Options", MenuOptions, MenuBarItemLayoutLeft);

  TGPopupMenu* MenuInfo = new TGPopupMenu(fClient->GetRoot());
  MenuInfo->AddEntry("About", c_About);
  MenuInfo->Associate(this);
  MenuBar->AddPopup("Info", MenuInfo, MenuBarItemLayoutLeft);


  // Main label
  //MString TitleIconName("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  MFile::ExpandFileName(m_PicturePath);
  
  TGLayoutHints* TitleIconLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 2, 2, 10, 0);
  bool PictureFound = false;
  if (MFile::Exists(m_PicturePath) == true) {
    const TGPicture* TitlePicture = fClient->GetPicture(m_PicturePath, FontScaler*300, FontScaler*300/5);
    if (TitlePicture == 0) {
      mout<<"Can't find picture \""<<m_PicturePath<<"\"! Aborting!"<<endl;
    } else {
      TGIcon* TitleIcon = new TGIcon(this, TitlePicture, TitlePicture->GetWidth()+2, TitlePicture->GetHeight()+2);
      AddFrame(TitleIcon, TitleIconLayout);
      PictureFound = true;
    }
  }
  
  if (PictureFound == false) {
    mout<<"Can't find picture "<<m_PicturePath<<"! Using text!"<<endl;
    const TGFont* lFont = gClient->GetFont("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-1");
    if (!lFont) lFont = gClient->GetResourcePool()->GetDefaultFont();
    FontStruct_t LargeFont = lFont->GetFontStruct();

    TGLabel* MainLabel = new TGLabel(this, m_ProgramName);
    MainLabel->SetTextFont(LargeFont);
    TGLayoutHints* MainLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 30, 0);
    AddFrame(MainLabel, MainLabelLayout);
  }


  // Sub-title
  FontStruct_t ItalicFont = MGUIDefaults::GetInstance()->GetItalicMediumFont()->GetFontStruct();

  TGLabel* SubTitle = new TGLabel(this, m_SubTitle);
  SubTitle->SetTextFont(ItalicFont);
  TGLayoutHints* SubTitleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 0, FontScaler*12);
  AddFrame(SubTitle, SubTitleLayout);

  
  
  
  
  TGLayoutHints* SectionLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 0, 10);
  

  // Modules
  m_ModuleFrame = new TGGroupFrame(this, "Choose the module sequence for your detector setup");
  AddFrame(m_ModuleFrame, SectionLayout);

  m_ModuleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 3, 3);
  UpdateModules();


  // Start & Exit buttons
  // Frame around the buttons:
  TGHorizontalFrame* ButtonFrame = new TGHorizontalFrame(this, 150, 25);
  TGLayoutHints* ButtonFrameLayout =	new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10, 10, 10, 10);
  AddFrame(ButtonFrame, ButtonFrameLayout);
  
  // The buttons itself
  TGTextButton*	StartButton = new TGTextButton(ButtonFrame, "Start", c_Start); 
  StartButton->Associate(this);
  TGLayoutHints* StartButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 40, 0, 0, 0);
  ButtonFrame->AddFrame(StartButton, StartButtonLayout);
  
  TGTextButton* ViewButton = new TGTextButton(ButtonFrame, "     View     ", c_View); 
  ViewButton->Associate(this);
  TGLayoutHints* ViewButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  ButtonFrame->AddFrame(ViewButton, ViewButtonLayout);
  
  TGTextButton* StopButton = new TGTextButton(ButtonFrame, "     Stop     ", c_Stop); 
  StopButton->Associate(this);
  TGLayoutHints* StopButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 20, 0, 0, 0);
  ButtonFrame->AddFrame(StopButton, StopButtonLayout);
  
  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMainFretalon::UpdateModules()
{
  // Remove all existing modules:
  for (unsigned int m = m_Modules.size()-1; m < m_Modules.size(); --m) {
    m_ModuleFrame->RemoveFrame(m_Modules[m]);
    m_Modules[m]->UnmapWindow();
    delete m_Modules[m];
  }
  m_Modules.clear();

  for (unsigned int m = 0; m < m_Supervisor->GetNModules(); ++m) {
    MGUIEModule* GuiModule = new MGUIEModule(m_ModuleFrame, m, m_Supervisor->GetModule(m));
    GuiModule->Associate(this);
    m_ModuleFrame->AddFrame(GuiModule, m_ModuleLayout);
    m_Modules.push_back(GuiModule);
  }  

  if (m_Supervisor->GetNModules() == 0 || m_Supervisor->GetModule(m_Supervisor->GetNModules()-1)->GetNSucceedingModuleTypes() > 0) {
    MGUIEModule* GuiModule = new MGUIEModule(m_ModuleFrame, m_Supervisor->GetNModules());
    GuiModule->Associate(this);
    m_ModuleFrame->AddFrame(GuiModule, m_ModuleLayout);
    m_Modules.push_back(GuiModule);
  }

  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::HandleKey(Event_t* Event)
{
  // Here we handle all keys...

  char tmp[10];
  unsigned int keysym;

  // Test if we have a key release:
  if (Event->fType != kKeyRelease) return false;

  // First we get the key...
  gVirtualX->LookupString(Event, tmp, sizeof(tmp), keysym);
  
  // ... and than we do what we need to do...
  
  // The following keys need an initialized hardware
  switch ((EKeySym) keysym) {
  case kKey_Escape:
    OnExit();
    break;
  case kKey_Return:
  case kKey_Enter:
    OnStart();
    break;
  case kKey_l:
  case kKey_L:
    OnLoadConfiguration();
    break;
  case kKey_s:
  case kKey_S:
    OnSaveConfiguration();
    break;
  default:
    break;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      if (Parameter1 >= c_Change && Parameter1 <= c_Change+99) {
        OnChange(Parameter1-c_Change);
      } else if (Parameter1 >= c_Remove && Parameter1 <= c_Remove+99) {
        OnRemove(Parameter1-c_Remove);
      } else if (Parameter1 >= c_Options && Parameter1 <= c_Options+99) {
        OnOptions(Parameter1-c_Options);
      }
      switch (Parameter1) {
      case c_Exit:
        Status = OnExit();
        break;

      case c_Start:
        Status = OnStart();
        break;

      case c_Stop:
        Status = OnStop();
        break;

      case c_View:
        Status = OnView();
        break;

      default:
        break;
      }
    case kCM_MENU:
      switch (Parameter1) {

      case c_LoadConfig:
        Status = OnLoadConfiguration();
        break;

      case c_SaveConfig:
        Status = OnSaveConfiguration();
        break;

      case c_Geometry:
        Status = OnGeometry();
        break;

      case c_Exit:
        Status = OnExit();
        break;

      case c_About:
        Status = OnAbout();
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
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMainFretalon::CloseWindow()
{
  // Call exit for controlled good-bye

  OnExit();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnChange(unsigned int ModuleID)
{
  MGUIModuleSelector* S = new MGUIModuleSelector(m_Supervisor, ModuleID);
  gClient->WaitForUnmap(S);
  
  UpdateModules();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnRemove(unsigned int ModuleID)
{
  m_Supervisor->RemoveModule(ModuleID);
  
  UpdateModules();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnOptions(unsigned int ModuleID)
{
  if (m_Supervisor->GetModule(ModuleID) != 0) {
    m_Supervisor->GetModule(ModuleID)->ShowOptionsGUI();
    return true;
  }
  cout<<"Warning: No module with ID: "<<ModuleID<<endl;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnExit()
{
  OnApply();

  m_Supervisor->Exit();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnStart()
{
  if (OnApply() == false) return false;

  m_Supervisor->Analyze();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnStop()
{
  if (OnApply() == false) return false;

  m_Supervisor->SetInterrupt();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnView()
{
  m_Supervisor->View();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnApply()
{
  /*
  if (MFile::Exists(m_FileSelectorLoad->GetFileName()) == false) {
    mgui<<"The data file \""<<m_FileSelectorLoad->GetFileName()<<"\" does not exist."<<show;
    return false;
  }
  */
  /*
  if (MFile::Exists(m_FileSelectorGeometry->GetFileName()) == false) {
    mgui<<"The geometry file \""<<m_FileSelectorGeometry->GetFileName()<<"\" does not exist."<<show;
    return false;
  }
  */
  
  /*
  m_Supervisor->SetLoadFileName(m_FileSelectorLoad->GetFileName());
  m_Supervisor->SetSaveFileName(m_FileSelectorSave->GetFileName());
  m_Supervisor->SetGeometryFileName(m_FileSelectorGeometry->GetFileName());
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnLoadConfiguration()
{
  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;


  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Supervisor->Load(MString(Info.fFilename));
    m_FileSelectorGeometry->SetFileName(m_Supervisor->GetGeometryFileName());

    UpdateModules();
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnSaveConfiguration()
{
  // Save a configuration file...

  if (OnApply() == false) return false;

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;
  

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  //Info.fIniDir = StrDup(gSystem->DirName(m_Supervisor->GetCurrentFile()));
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Supervisor->Save(MString(Info.fFilename));
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnGeometry()
{
  // Show the geometry dialog
  // Returns the geometry file name

  MString Name = m_Supervisor->GetGeometryFileName();
  MGUIGeometry* Geo = new MGUIGeometry(gClient->GetRoot(), gClient->GetRoot(), Name);
  gClient->WaitForUnmap(Geo);
  if (Geo->OkPressed() == true) {
    Name = Geo->GetGeometryFileName();
    delete Geo;
    for (unsigned int i = 0; i < 100; ++i) {
      gSystem->ProcessEvents();
    }

    m_Supervisor->SetGeometryFileName(Name);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMainFretalon::OnAbout()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName(m_ProgramName);
  About->SetIconPath(m_PicturePath);
  About->SetLeadProgrammer(m_LeadAuthor);
  About->SetUpdates("");
  if (m_CoAuthors != "") {
    About->SetProgrammers(m_CoAuthors);
  }
  About->SetCopyright("All rights reserved");
  About->ShowReferencesTab(false);
  About->ShowPeopleTab(false);
  About->ShowBugsTab(false);
  About->ShowDisclaimerTab(false);
  
  About->Create();
  
  return true;
}


// MGUIMainFretalon: the end...
////////////////////////////////////////////////////////////////////////////////
