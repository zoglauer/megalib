/*
 * MGUISivanMain.cxx
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
#include "MGUISivanMain.h"

// Standard libs:

// ROOT libs:
#include <TGPicture.h>
#include <TStyle.h>

// MEGAlib libs:
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUISivanTracking.h"
#include "MInterfaceSivan.h"
#include "MSystem.h"
#include "MStreams.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUISivanMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUISivanMain::MGUISivanMain(MInterfaceSivan* Interface, MSettingsSivan* Data)
  : MGUIMain(660, 270, Interface, Data)
{
  m_Interface = Interface;
  m_Data = Data;
}


////////////////////////////////////////////////////////////////////////////////


MGUISivanMain::~MGUISivanMain()
{
  // Standard destructor - in all normal exit cases the session have already 
  // been deleted in CloseWindow

}


////////////////////////////////////////////////////////////////////////////////


void MGUISivanMain::Create()
{
  // Create the main window
  
  MGUIMain::Create();

  // We start with a name and an icon...
  SetWindowName("Sivan - SImulated eVent ANalyzer");  

  m_MenuAnalysis = new TGPopupMenu(fClient->GetRoot());
  m_MenuAnalysis->AddEntry("Options", c_Options);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Analyze events and create tra-file", c_Start);
  m_MenuAnalysis->AddEntry("Retrieve the ideal events and create tra-file", c_IdealAnalysis);
  // m_MenuAnalysis->AddSeparator();
  // m_MenuAnalysis->AddEntry("Noise and chance coincidences", c_Chance);
  m_MenuAnalysis->AddSeparator();
  //m_MenuAnalysis->AddEntry("View hits", c_ViewHits);
  //m_MenuAnalysis->AddEntry("Energy loss", c_EnergyLoss);
  //m_MenuAnalysis->AddEntry("Energy per voxel", c_EnergyPerVoxel);
  m_MenuAnalysis->AddEntry("Start locations", c_StartLocations);
  m_MenuAnalysis->AddEntry("Initial interaction statistics", c_InitialInteraction);
  m_MenuAnalysis->AddEntry("Incidence Angle", c_IncidenceAngle);
  m_MenuAnalysis->AddEntry("Incidence Energy", c_IncidenceEnergy);
  m_MenuAnalysis->AddEntry("Incidence Vs. Measured Energy", c_IncidenceVsMeasuredEnergy);
  m_MenuAnalysis->AddEntry("Initial Compton scatter angle", c_InitialComptonScatterAngle);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Secondary generation pattern", c_SecondaryGenerationPattern); 
  m_MenuAnalysis->AddEntry("Interaction detector sequence", c_InteractionDetectorSequence); 
  m_MenuAnalysis->AddEntry("Energy of Secondaries", c_EnergyOfSecondaries);
  m_MenuAnalysis->AddEntry("Energy Loss By Material", c_EnergyLossByMaterial);
  m_MenuAnalysis->AddEntry("Number of Hits per event and detector", c_Hits);
  m_MenuAnalysis->AddEntry("Statistics on ideal interactions", c_NInteractions);
  m_MenuAnalysis->AddEntry("Position of first detected interaction", c_FirstHitPosition);
  m_MenuAnalysis->AddEntry("Positions of all detected interactions", c_AllHitPositions);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Compton energy escape", c_ComptonEnergyEscape);
  m_MenuAnalysis->AddEntry("Statistics on missing interactions", c_MissingInteractions);
  m_MenuAnalysis->AddEntry("Ratio of completly absorbed events", c_CompleteAbsorptionRatio);
  m_MenuAnalysis->AddEntry("Interaction per voxels...", c_InteractionsPerVoxel);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Newly generated isotopes distribution", c_IsotopeGeneration);
  m_MenuAnalysis->AddEntry("Energy distribution by decayed isotope", c_EnergyPerNucleus);
  // m_MenuAnalysis->AddEntry("Charge sharing between voxels...", c_ChargeSharing);
  m_MenuAnalysis->AddSeparator();
  m_MenuAnalysis->AddEntry("Doppler broadening", c_DopplerARM);


//   m_MenuAnalysis->AddEntry("Event selections", M_SV_EVENTSELECTION);
//   m_MenuAnalysis->AddSeparator();
//   m_MenuAnalysis->AddEntry("Options", M_SV_OPTIONS);
//   m_MenuAnalysis->AddSeparator();
//   m_MenuAnalysis->AddEntry("Start creating tra-file", M_SV_START);
//   m_MenuAnalysis->AddEntry("View hits", M_SV_VIEWHITS);
//   m_MenuAnalysis->AddEntry("Energy loss", M_SV_ENERGYLOSS);
//   m_MenuAnalysis->AddEntry("Crystals with certain deposited energies", M_SV_ENERGYCRYSTAL);
//   m_MenuAnalysis->AddEntry("Distance and angular error of multiple comptons", M_SV_MULTIPLECOMPTONS);
//   m_MenuAnalysis->AddEntry("Create Response", M_SV_CREATERESPONSE);
//   m_MenuAnalysis->AddEntry("Doppler Broadening", M_SV_DOPPLER);

  // m_MenuAnalysis->AddSeparator();
  // m_MenuAnalysis->AddEntry("Some testing...", c_Test);
  m_MenuAnalysis->Associate(this);
  m_MenuBar->AddPopup("Analysis", m_MenuAnalysis, m_MenuBarItemLayoutLeft);

  // Only add the info here at the end
  m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayoutRight);

  // Add buttons to the tool bar:
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Load.xpm", c_Open, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Start.xpm", c_Start, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Exit.xpm", c_Exit, 20);

  // Title:
  MString TitleIcon(g_MEGAlibPath + "/resource/icons/sivan/Large.xpm");
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
  m_StatusBarGeo->SetContent("Active:", MFile::GetBaseName(m_Data->GetGeometryFileName()));
  m_StatusBarGeo->Create();
  AddFrame(m_StatusBarGeo, m_StatusBarLayout);

  m_StatusBarFile->SetContent("Active:", MFile::GetBaseName(m_Data->GetCurrentFileName()));
  m_StatusBarFile->Create();
  AddFrame(m_StatusBarFile, m_StatusBarLayout);


  MapSubwindows();
  MapWindow();  
  Layout();

  gSystem->ProcessEvents();

  // Now set the minimum and maximum size of the display:
  int xDisplay, yDisplay;
  unsigned int wDisplay, hDisplay;
  gVirtualX->GetGeometry(-1, xDisplay, yDisplay, wDisplay, hDisplay);
  SetWMSizeHints(660, 330, wDisplay, hDisplay, 0, 0);

  UpdateConfiguration();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUISivanMain::ProcessMessage(long Message, long Parameter1, 
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

      case c_Options:
        new MGUISivanTracking(gClient->GetRoot(), this, m_Data);
        break;
        
      case c_Test:
        m_Interface->TestOnly();
        //m_Interface->InitializeEventView();
        //m_Interface->ViewEvent();
        break;

      case c_Start:
        Launch();
        break;

      case c_IdealAnalysis:
        m_Interface->AnalyzeSimEvents(true);
        break;

      case c_Chance:
        m_Interface->ChanceCoincidences();
        break;

      case c_ViewHits:
        m_Interface->ViewHits();
        break;

      case c_EnergyLoss:
        m_Interface->EnergyLoss();
        break;

      case c_EnergyLossByMaterial:
        m_Interface->EnergyLossByMaterial();
        break;

      case c_EnergyPerVoxel:
        m_Interface->EnergyPerVoxel();
        break;

      case c_ComptonEnergyEscape:
        m_Interface->ComptonEnergyEscape();
        break;

      case c_MissingInteractions:
        m_Interface->MissingInteractionsStatistics();
        break;

      case c_CompleteAbsorptionRatio:
        m_Interface->CompleteAbsorptionRatio();
        break;

      case c_StartLocations:
        m_Interface->StartLocations();
        break;

      case c_IncidenceAngle:
        m_Interface->IncidenceAngle();
        break;

      case c_IncidenceEnergy:
        m_Interface->IncidenceEnergy();
        break;

      case c_InitialInteraction:
        m_Interface->InitialInteraction();
        break;

      case c_IncidenceVsMeasuredEnergy:
        m_Interface->IncidenceVsMeasuredEnergy();
        break;

      case c_InitialComptonScatterAngle:
        m_Interface->InitialComptonScatterAngle();
        break;

      case c_SecondaryGenerationPattern:
        m_Interface->SecondaryGenerationPattern();
        break;
        
      case c_EnergyOfSecondaries: 
        m_Interface->EnergyOfSecondaries();
        break;

      case c_InteractionDetectorSequence:
        m_Interface->InteractionDetectorSequence();
        break;

      case c_Hits:
        m_Interface->AverageNumberOfHits();
        break;

      case c_NInteractions:
        m_Interface->NInteractions();
        break;

      case c_FirstHitPosition:
        m_Interface->LocationOfFirstDetectedInteraction();
        break;

      case c_AllHitPositions:
        m_Interface->LocationsOfAllDetectedInteractions();
        break;

      case c_DopplerARM:
        m_Interface->DopplerArm();
        break;

      case c_ChargeSharing:
        m_Interface->ChargeSharing();
        break;

      case c_InteractionsPerVoxel:
        m_Interface->InteractionsPerVoxel();
        break;

      case c_EnergyPerNucleus:
        m_Interface->EnergyPerNucleus();
        break;
        
      case c_IsotopeGeneration:
        m_Interface->IsotopeGeneration();
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


void MGUISivanMain::Open()
{
  // Open a file

  const char** Types = new const char*[8];
  Types[0] = "Simulated events";
  Types[1] = "*.sim";
  Types[2] = "Gzip'ed simulated events";
  Types[3] = "*.sim.gz";
  Types[4] = "All files";
  Types[5] = "*";
  Types[6] = 0;
  Types[7] = 0;
  
  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  Info.fIniDir = StrDup(gSystem->DirName(m_Data->GetCurrentFileName()));
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  delete [] Types;
  
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


void MGUISivanMain::About()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Sivan");
  About->SetIconPath(g_MEGAlibPath + "/resource/icons/sivan/Small.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  About->Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUISivanMain::Launch()
{
  // This method is called after pressing the "Start" button
  
  MString Name = m_Data->GetCurrentFileName();

  if (Name.EndsWith("sim") == false && Name.EndsWith("sim.gz") == false) {
    mgui<<"The input file of Sivan needs to have the suffix \"sim\" or \"sim.gz\"!"<<error;
    return;
  }

  m_Interface->AnalyzeSimEvents();
}


// MGUISivanMain: the end...
////////////////////////////////////////////////////////////////////////////////
