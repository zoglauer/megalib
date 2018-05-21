/*
 * MGUIRevanMain.cxx
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
#include "MGUIRevanMain.h"

// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:
#include <TGPicture.h>

// MEGAlib libs:
#include "MGUIAbout.h"
#include "MInterfaceRevan.h"
#include "MGUIGeometry.h"
#include "MSystem.h"
#include "MFile.h"
#include "MStreams.h"
#include "MGUIERAlgorithm.h"
#include "MGUIOptionsCoincidence.h"
#include "MGUIOptionsClustering.h"
#include "MGUIOptionsTracking.h"
#include "MGUIOptionsCSR.h"
#include "MGUIOptionsDecay.h"
#include "MGUIOptionsGeneral.h"
#include "MGUIExportSpectrum.h"
#include "MGUISpectralAnalyzer.h"
#include "MGUIReconstructedSpectrum.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIRevanMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIRevanMain::MGUIRevanMain(MInterfaceRevan* Interface, MSettingsRevan* Data)
  : MGUIMain(660, 270, Interface, Data)
{
  m_Interface = Interface;
  m_Data = Data;
}


////////////////////////////////////////////////////////////////////////////////


MGUIRevanMain::~MGUIRevanMain()
{
  // Standard destructor - in all normal exit cases the session have already 
  // been deleted in CloseWindow
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRevanMain::Create()
{
  // Create the main window
  
  MGUIMain::Create();

  // We start with a name and an icon...
  SetWindowName("Revan - Real event analyzer");  

  m_MenuReconstruction = new TGPopupMenu(fClient->GetRoot());
  m_MenuReconstruction->AddLabel("Analysis");
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("Start Event reconstruction", c_Start);
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddLabel("Options");
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("Selection of general algorithms", c_Options);
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("Coincidence options", c_OptionsCoincidence);
  m_MenuReconstruction->AddEntry("Clustering options", c_OptionsClustering);
  m_MenuReconstruction->AddEntry("Electron tracking options", c_OptionsTracking);
  m_MenuReconstruction->AddEntry("Compton sequencing options", c_OptionsSequencing);
  //m_MenuReconstruction->AddEntry("Decay options", c_OptionsDecay);
  m_MenuReconstruction->AddSeparator();
  m_MenuReconstruction->AddEntry("General event selections", c_OptionsGeneral);
  m_MenuReconstruction->Associate(this);
  m_MenuBar->AddPopup("Reconstruction", m_MenuReconstruction, m_MenuBarItemLayoutLeft);

  m_MenuApplications = new TGPopupMenu(fClient->GetRoot());
  m_MenuApplications->AddLabel("Spectra");
  m_MenuApplications->AddEntry("Generate spectra", c_Spectrum);
  m_MenuApplications->AddEntry("Export initial energy spectrum", c_ExportSpectrum);
  m_MenuApplications->AddEntry("Initial energy spectrum", c_EnergyInitial);
  m_MenuApplications->AddEntry("Energy spectrum Before/After analysis", c_EnergyDistribution);
  m_MenuApplications->AddEntry("Spectral analyzer (experimental)", c_SpectralAnalyzer);
  m_MenuApplications->AddSeparator();
  m_MenuApplications->AddLabel("Statistics");
  m_MenuApplications->AddEntry("Show trigger statistics", c_TriggerStatistics);
  m_MenuApplications->AddEntry("Initial event statistics", c_InitialEventStatistics);
  m_MenuApplications->AddSeparator();
  m_MenuApplications->AddLabel("Timing");
  m_MenuApplications->AddEntry("Time between events", c_Coincidence);
  m_MenuApplications->AddSeparator();
  m_MenuApplications->AddLabel("Miscellaneous");
  m_MenuApplications->AddEntry("Energy per cluster per detector type", c_HitStatistics);
  m_MenuApplications->AddEntry("Energy per central track element", c_EnergyPerCentralTrackElement);
  m_MenuApplications->AddEntry("Spatial hit distribution", c_SpatialHitDistribution);
  m_MenuApplications->AddEntry("Spatial energy distribution", c_SpatialEnergyDistribution);
  m_MenuApplications->AddEntry("Depth profile by detector", c_DepthProfileByDetector);
  // m_MenuApplications->AddEntry("CSR Test statistics", c_CSRTestStatistics);
  m_MenuApplications->AddEntry("Number of clusters", c_NClusters);
  m_MenuApplications->AddEntry("Cluster distribution by detector type before reconstruction", c_DetectorTypeClusterDistributionBefore);
  m_MenuApplications->AddEntry("Cluster distribution by detector type after reconstruction", c_DetectorTypeClusterDistributionAfter);
  m_MenuApplications->AddEntry("Hit distribution by detector type before reconstruction", c_DetectorTypeHitDistributionBefore);
  m_MenuApplications->AddEntry("Hit distribution by detector type after reconstruction", c_DetectorTypeHitDistributionAfter);

  m_MenuApplications->Associate(this);
  m_MenuBar->AddPopup("Analysis", m_MenuApplications, m_MenuBarItemLayoutLeft);

  // Only add the info here at the end
  m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayoutRight);

  // Add buttons to the tool bar:
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Load.xpm", c_Open, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Start.xpm", c_Start, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Exit.xpm", c_Exit, 20);

  // Title:
  MString TitleIcon(g_MEGAlibPath + "/resource/icons/revan/Large.xpm");
  MFile::ExpandFileName(TitleIcon);
  
  m_TitlePicture = fClient->GetPicture(TitleIcon);
  if (m_TitlePicture == 0) {
    mgui<<"Can't find picture "<<TitleIcon<<"! Aborting!"<<error;
    return;
  }
  m_TitleIcon = new TGIcon(this, m_TitlePicture, m_TitlePicture->GetWidth(), m_TitlePicture->GetHeight());
  m_TitleIconLayout = new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 0, 0, 10, 10);
  AddFrame(m_TitleIcon, m_TitleIconLayout);

  // The status bars  
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


bool MGUIRevanMain::ProcessMessage(long Message, long Parameter1, 
                                     long Parameter2)
{
  // Process the messages for this application

  if (MGUIMain::ProcessMessage(Message, Parameter1, Parameter2) == false) {
    return false;
  }

  bool OKPressed = false;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_MENU:
      if (Parameter1 > c_Min && Parameter1 < c_Max) {
        m_Data->Write();
      }
      switch (Parameter1) {
      case c_Options:
        new MGUIERAlgorithm(gClient->GetRoot(), this, m_Data);
        break;

      case c_OptionsGeneral:
        new MGUIOptionsGeneral(gClient->GetRoot(), this, m_Data);
        break;

      case c_OptionsTracking:
        new MGUIOptionsTracking(gClient->GetRoot(), this, m_Data, m_Interface->GetGeometry());
        break;

      case c_OptionsCoincidence:
        new MGUIOptionsCoincidence(gClient->GetRoot(), this, m_Data);
        break;

      case c_OptionsClustering:
        new MGUIOptionsClustering(gClient->GetRoot(), this, m_Data);
        break;

      case c_OptionsSequencing:
        new MGUIOptionsCSR(gClient->GetRoot(), this, m_Data);
        break;

      case c_OptionsDecay:
        new MGUIOptionsDecay(gClient->GetRoot(), this, m_Data);
        break;
        
      case c_Test:
        m_Interface->Test();
        //m_Interface->InitializeEventView();
        //m_Interface->ViewEvent();
        break;

      case c_Start:
        Launch();
        break;

      case c_Spectrum:
        new MGUIReconstructedSpectrum(gClient->GetRoot(), this, m_Data, OKPressed);        
        if (OKPressed == true) {
          m_Interface->GenerateSpectra();
        }
        break;

      case c_ExportSpectrum:
        new MGUIExportSpectrum(gClient->GetRoot(), this, m_Data, OKPressed);        
        if (OKPressed == true) {
          m_Interface->ExportSpectrum();
        }
        break;

      case c_InitialEventStatistics:
        m_Interface->InitialEventStatistics();
        break;

      case c_TriggerStatistics:
        m_Interface->TriggerStatistics();
        break;

      case c_EnergyInitial:
        m_Interface->InitialEnergySpectrum();
        break;

      case c_SpectralAnalyzer:
        new MGUISpectralAnalyzer(gClient->GetRoot(), this, m_Data, &OKPressed);
        if (OKPressed == true) {
          m_Interface->SpectralAnalyzer();
        }
        break;

      case c_Coincidence:
        m_Interface->Coincidence();
        break;

      case c_EnergyDistribution:
        m_Interface->EnergyDistribution();
        break;

      case c_EnergyPerCentralTrackElement:
        m_Interface->EnergyPerCentralTrackElement();
        break;

      case c_HitStatistics:
        m_Interface->HitStatistics();
        break;

      case c_SpatialHitDistribution:
        m_Interface->SpatialDistribution(false);
        break;

      case c_SpatialEnergyDistribution:
        m_Interface->SpatialDistribution(true);
        break;

      case c_DepthProfileByDetector:
        m_Interface->DepthProfileByDetector();
        break;

//       case c_CSRTestStatistics:
//         m_Interface->ShowCSRTestStatistics();
//         break;

      case c_BeamPath:
        m_Interface->FindBeamPath();
        break;

      case c_Polarization:
        m_Interface->FindPolarization();
        break;

      case c_NClusters:
        m_Interface->NumberOfClusters();
        break;

      case c_DetectorTypeClusterDistributionBefore:
        m_Interface->DetectorTypeClusterDistribution(true);
        break;

      case c_DetectorTypeClusterDistributionAfter:
        m_Interface->DetectorTypeClusterDistribution(false);
        break;

      case c_DetectorTypeHitDistributionBefore:
        m_Interface->DetectorTypeHitDistribution(true);
        break;

      case c_DetectorTypeHitDistributionAfter:
        m_Interface->DetectorTypeHitDistribution(false);
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


void MGUIRevanMain::Open()
{
  // Open a file

  const char** Types = new const char*[12];

  MSystem Sys;
  MString Suffix;
  Sys.GetFileSuffix(m_Data->GetCurrentFileName(), &Suffix);

  if (Suffix == "sim") {
    Types[0] = "Simulated events";
    Types[1] = "*.sim";
    Types[2] = "Gzip'ed simulated events";
    Types[3] = "*.sim.gz";
    Types[4] = "Analyzed events";
    Types[5] = "*.evta";
    Types[6] = "Gzip'ed analyzed events";
    Types[7] = "*.evta.gz";
    Types[8] = "All files";
    Types[9] = "*";
    Types[10] = 0;
    Types[1] = 0;
  } else {
    Types[0] = "Analyzed events";
    Types[1] = "*.evta";
    Types[2] = "Gzip'ed analyzed events";
    Types[3] = "*.evta.gz";
    Types[4] = "Simulated events";
    Types[5] = "*.sim";
    Types[6] = "Gzip'ed simulated events";
    Types[7] = "*.sim.gz";
    Types[8] = "All files";
    Types[9] = "*";
    Types[10] = 0;
    Types[11] = 0;
  }

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

  // I have no idea if I have to delete [] Types;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRevanMain::About()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Revan");
  About->SetIconPath(g_MEGAlibPath + "/resource/icons/revan/Small.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  About->SetReference("Chapter 4: Description of classic and Bayesian event reconstruction approach",
                      "A. Zoglauer, \"First Light for the Next Generation of Compton and Pair Telescopes\", Doctoral thesis, TU Munich, 2005");
  About->SetReference("Description of the Bayesian event reconstruction approach",
                      "A. Zoglauer et al., \"Recognition of Compton Scattering Patterns in Advanced Compton Telecopes}\", Proc. SPIE 6700, 2007");
  About->SetReference("Description of the neural network event reconstruction approach",
                      "A. Zoglauer et al., \"Application of Neural Networks to the Identification of the Compton Interaction Sequence in Compton Imagers\", Nuclear Science Symposium Conference Record, 2007. NSS '07. IEEE, 2007");
  About->Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRevanMain::Launch()
{
  // This method is called after pressing the MEGA-button in the 
  // M.I.Works main GUI:
  // The reconstruction is started.
  
  MString Name = m_Data->GetCurrentFileName();

  if (Name.EndsWith("sim") == false && Name.EndsWith("evta") == false && 
      Name.EndsWith("sim.gz") == false && Name.EndsWith("evta.gz") == false) {
    mgui<<"The input file of Revan needs to have the suffix \"sim\", \"sim.gz\", \"evta\", or \"evta.gz\"!"<<error;
    return;
  }

  m_Interface->AnalyzeEvents();
}


// MGUIRevanMain: the end...
////////////////////////////////////////////////////////////////////////////////
