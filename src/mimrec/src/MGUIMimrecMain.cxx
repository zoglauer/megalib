/*
 * MGUIMimrecMain.cxx
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
#include "MGUIMimrecMain.h"

// Standard libs:

// ROOT libs:
#include <TGPicture.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"
#include "MInterfaceMimrec.h"
#include "MGUIAbout.h"
#include "MGUIBPAlgorithm.h"
#include "MGUICoordinateSystem.h"
#include "MGUIDeconvolution.h"
#include "MGUIPenalty.h"
#include "MGUIEventSelection.h"
#include "MGUIResponseSelection.h"
#include "MGUIResponseParameterGauss1D.h"
#include "MGUIResponseParameterPRM.h"
#include "MGUIARM.h"
#include "MGUISignificance.h"
#include "MGUISensitivity.h"
#include "MGUIAnimation.h"
#include "MGUIMemory.h"
#include "MGUIImageDimensions.h"
#include "MGUIImageOptions.h"
#include "MGUIGeometry.h"
#include "MGUIHistogramData.h"
#include "MGUIPolarization.h"
#include "MGUISpectralAnalyzer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMimrecMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMimrecMain::MGUIMimrecMain(MInterfaceMimrec* Interface, MSettingsMimrec* Data)
  : MGUIMain(660, 270, Interface, Data)
{
  m_Interface = Interface;
  m_Data = Data;
}


////////////////////////////////////////////////////////////////////////////////


MGUIMimrecMain::~MGUIMimrecMain()
{
  // Standard destructor - in all normal exit cases the session have already 
  // been deleted in CloseWindow
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMimrecMain::Create()
{
  // Create the main window
  
  MGUIMain::Create();

  // Check if "convert" exists
  
  
  // We start with a name and an icon...
  SetWindowName("Mimrec - Megalib IMage REConstruction");  

  TGPopupMenu* MenuSelections = new TGPopupMenu(fClient->GetRoot());
  MenuSelections->AddEntry("Event selection", c_EventSelection);
  MenuSelections->AddEntry("Coordinate system", c_CoordinateSystem);
  MenuSelections->AddSeparator();
  MenuSelections->AddEntry("Show event selections", c_ShowEventSelections);
  if (m_Data->GetSpecialMode() == true) {
    MenuSelections->AddEntry("Show event selections stepwise", c_ShowEventSelectionsStepwise);
  }
  MenuSelections->AddEntry("Extract events", c_ExtractEvents);
  MenuSelections->Associate(this);
  m_MenuBar->AddPopup("Selections", MenuSelections, m_MenuBarItemLayoutLeft);

  TGPopupMenu* MenuImaging = new TGPopupMenu(fClient->GetRoot());
  MenuImaging->AddLabel("General imaging options");
  MenuImaging->AddSeparator();
  MenuImaging->AddEntry("Image dimensions", c_Zoom);
  MenuImaging->AddEntry("Image drawing options", c_ImageOptions);
  MenuImaging->AddEntry("Memory, accuracy, and thread management", c_Memory);
  if (m_Data->GetSpecialMode() == true) {
    MenuImaging->AddSeparator();
    MenuImaging->AddEntry("Point-source de-selection", c_PointSourceDeselection);
  }
  MenuImaging->AddSeparator();
  MenuImaging->AddLabel("LM-ML-EM imaging");
  MenuImaging->AddSeparator();
  MenuImaging->AddEntry("Start image reconstuction", c_StartReconstruction);
  if (MFile::ProgramExists("convert") == false) {
    MenuImaging->AddEntry("Create animated gif - \"convert\" not found", c_Animation);
    MenuImaging->DisableEntry(c_Animation);
  } else {
    MenuImaging->AddEntry("Create animated gif (experimental)", c_Animation);    
  }
  //MenuImaging->AddEntry("Stop image reconstuction", c_StopReconstruction);
  MenuImaging->AddSeparator();
  MenuImaging->AddEntry("Reconstruction algorithm", c_LikelihoodAlgorithm);
  //m_Menu->AddEntry("Penalty", c_LikelihoodPenalty);
  MenuImaging->AddSeparator();
  // MenuImaging->AddEntry("Backprojection algorithm", c_Algorithm);
  MenuImaging->AddEntry("Response type selection", c_Response);
  MenuImaging->AddEntry("Response parameter (for the above selected type)", c_FitParameter);
  // MenuImaging->AddEntry("Sensitivity", c_Sensitivity);
  //MenuImaging->DisableEntry(M_BP_SENSITIVITY);
  if (m_Data->GetSpecialMode() == true) {
    MenuImaging->AddSeparator();
    MenuImaging->AddLabel("Brand new, special, or not fully test");
    MenuImaging->AddSeparator();
    MenuImaging->AddEntry("Theta origin distribution", c_ThetaOriginDistribution);
    MenuImaging->AddEntry("Significance Map of Compton events", c_ResponseSignificanceMap);
  }
  MenuImaging->Associate(this);
  m_MenuBar->AddPopup("Imaging", MenuImaging, m_MenuBarItemLayoutLeft);

  
  // The sub menu General
  TGPopupMenu* MenuGeneral = new TGPopupMenu(fClient->GetRoot());
  MenuGeneral->AddEntry("Energy spectra", c_ResponseSpectrum);
  MenuGeneral->AddEntry("Time distribution", c_ResponseTime);
  MenuGeneral->AddEntry("Location of initial interaction", c_ResponseLocationOfInitialInteraction);  
  MenuGeneral->AddEntry("Pointing in galactic coordinates", c_ResponsePointingInGalacticCoordinates);  
  MenuGeneral->AddEntry("Horizon zenith in spherical coordinates", c_ResponseHorizonInSphericalDetectorCoordinates);  
  if (m_Data->GetSpecialMode() == true) {
    MenuGeneral->AddEntry("Time walk", c_ResponseTimeWalk);
    MenuGeneral->AddEntry("Select Ids", c_ResponseSelectIds);
  }

  // The sub menu ARM
  TGPopupMenu* MenuARM = new TGPopupMenu(fClient->GetRoot());
  MenuARM->AddEntry("ARM of scattered gamma-ray", c_ResponseArmGamma);
  MenuARM->AddEntry("SPD of recoil electron", c_ResponseSpdElectron);
  MenuARM->AddEntry("ARM of recoil electron", c_ResponseArmElectron);
  MenuARM->AddEntry("ARM of scattered gamma-ray vs. Compton Scatter Angel (phi)", c_ResponseArmGammaVsCompton);
  MenuARM->AddEntry("ARM of scattered gamma-ray vs. First interaction distance", c_ResponseArmGammaVsDistance);
  MenuARM->AddEntry("SPD of recoil electron vs Compton Scatter Angel (phi)", c_ResponseSpdElectronVsCompton);
  MenuARM->AddEntry("Dual ARM", c_ResponseDualArm);
  MenuARM->AddEntry("ARM-imaging-response comparison", c_ResponseArmComparison);
  MenuARM->Associate(this);

  // The sub menu quality factors:
  TGPopupMenu* MenuQuality = new TGPopupMenu(fClient->GetRoot());
  MenuQuality->AddEntry("Compton Sequence Quality Factor", c_ResponseComptonQualityFactor);
  MenuQuality->AddEntry("Compton Sequence Quality Factor with ARM selection", c_ResponseComptonProbabilityWithARMSelection);
  MenuQuality->AddEntry("ARM of scattered gamma-ray vs. Compton Quality Factor", c_ResponseArmGammaVsComptonProbability);
  MenuQuality->AddEntry("Energy vs. Compton Quality Factor", c_ResponseEnergyVsComptonProbability);
  MenuQuality->AddEntry("Compton Sequence Length vs Compton Quality Factor", c_ResponseComptonSequenceLengthVsComptonProbability);
  MenuQuality->AddEntry("Earth center distance", c_ResponseEarthCenterDistance);
  MenuQuality->AddSeparator();
  MenuQuality->AddEntry("Electron Track Quality Factor", c_ResponseTrackQualityFactor);
  MenuQuality->AddEntry("SPD vs. Track Quality Factor", c_ResponseSPDVsTrackQualityFactor); 
  MenuQuality->AddEntry("SPD vs. Total Scatter Angle (theta) Deviation", c_ResponseSPDVsTotalScatterAngleDeviation); 
  MenuQuality->AddSeparator();
  MenuQuality->AddEntry("Clustering Quality Factor", c_ResponseClusteringQualityFactor);
  MenuQuality->AddEntry("ARM of scattered gamma-ray vs. Clustering Quality Factor", c_ResponseArmGammaVsClusteringProbability);

  // The sub menu quality factors:
  TGPopupMenu* MenuDistributions = new TGPopupMenu(fClient->GetRoot());
  MenuDistributions->AddEntry("Scatter angle distributions", c_ResponsePhi);
  MenuDistributions->AddEntry("Distance distribution", c_ResponseDistance);
  MenuDistributions->AddEntry("Compton sequence and track length", c_ResponseSequenceLengths);
  MenuDistributions->AddEntry("Coincidence window", c_ResponseCoincidenceWindow);
  MenuDistributions->AddEntry("Energy distribution Electron Photon", c_ResponseEnergyDistributionD1D2);
  MenuDistributions->AddEntry("Azimuthal Compton scatter angle distribution", c_ResponseAzimuthalComptonScatterAngle);
  MenuDistributions->AddEntry("Azimuthal Electron scatter angle distribution", c_ResponseAzimuthalElectronScatterAngle);

  // The sub menu pair:
  TGPopupMenu* MenuPair = new TGPopupMenu(fClient->GetRoot());
  MenuPair->AddEntry("Angular resolution pair", c_ResponseAngularResolutionPair);
  MenuPair->AddEntry("Opening angle pair", c_ResponseOpeningAnglePair);
  MenuPair->AddEntry("Initial energy deposit", c_ResponseInitialEnergyDeposit);
  MenuPair->AddEntry("Angular resolution vs quality factor", c_ResponseAngularResolutionVsQualityFactorPair);


  TGPopupMenu* MenuResponse = new TGPopupMenu(fClient->GetRoot());
  MenuResponse->AddLabel("General");
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Energy spectra", c_ResponseSpectrum);
  MenuResponse->AddEntry("Time distribution", c_ResponseTime);
  MenuResponse->AddPopup("All general options", MenuGeneral);
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Spectral analyzer", c_SpectralAnalyzer);
  MenuResponse->AddSeparator();
  MenuResponse->AddLabel("Compton specific");
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("ARM of scattered gamma-ray", c_ResponseArmGamma);
  MenuResponse->AddEntry("SPD of recoil electron", c_ResponseSpdElectron);
  MenuResponse->AddPopup("All ARM/SPD options", MenuARM);
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Scatter angle distributions", c_ResponsePhi);
  MenuResponse->AddEntry("Distance distribution", c_ResponseDistance);
  MenuResponse->AddEntry("Compton sequence and track length", c_ResponseSequenceLengths);
  MenuResponse->AddPopup("All Distribution options", MenuDistributions);
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Compton Sequence Quality Factor", c_ResponseComptonQualityFactor);
  MenuResponse->AddEntry("Electron Track Quality Factor", c_ResponseTrackQualityFactor);
  MenuResponse->AddPopup("All Quality Factor options", MenuQuality);
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Polarization analysis", c_ResponsePolarization);
  //MenuResponse->AddEntry("Standard analysis (experimental)", c_ResponseStandardAnalysis);
  MenuResponse->AddSeparator();
  MenuResponse->AddLabel("Pair specific");
  MenuResponse->AddSeparator();
  MenuResponse->AddEntry("Angular resolution pair", c_ResponseAngularResolutionPair);
  MenuResponse->AddEntry("Opening angle pair", c_ResponseOpeningAnglePair);
  MenuResponse->AddPopup("All Pair options", MenuPair);
  //MenuResponse->AddSeparator();
  //MenuResponse->AddEntry("Experimental", c_ResponseTest);
  MenuResponse->Associate(this);
  m_MenuBar->AddPopup("Analysis/Response", MenuResponse, m_MenuBarItemLayoutLeft);

  // Only add the info here at the end
  m_MenuBar->AddPopup("Info", m_MenuInfo, m_MenuBarItemLayoutRight);

  // Add buttons to the tool bar:
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Load.xpm", c_Open, 0);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Start.xpm", c_StartReconstruction, 20);
  m_ToolBar->Add(this, g_MEGAlibPath + "/resource/icons/global/Exit.xpm", c_Exit, 20);

  // Title:
  MString TitleIcon(g_MEGAlibPath + "/resource/icons/mimrec/Large.xpm");
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


bool MGUIMimrecMain::ProcessMessage(long Message, long Parameter1, 
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
      switch (Parameter1) {

//       case c_Algorithm:
//         new MGUIBPAlgorithm(gClient->GetRoot(), this, m_Data);
//         break;

      case c_Response:
        new MGUIResponseSelection(gClient->GetRoot(), this, m_Data);
        break;

      case c_FitParameter:
        if (m_Data->GetResponseType() == 0) {
          new MGUIResponseParameterGauss1D(gClient->GetRoot(), this, m_Data);
        } else if (m_Data->GetResponseType() == 3) {
          new MGUIResponseParameterPRM(gClient->GetRoot(), this, m_Data);
        }
        break;

//       case c_Sensitivity:
//         new MGUISensitivity(gClient->GetRoot(), this, m_Data);
//         break;
        
      case c_EventSelection:
        new MGUIEventSelection(gClient->GetRoot(), this, m_Data, m_Interface->GetGeometry());
        break;
        
      case c_CoordinateSystem:
        new MGUICoordinateSystem(gClient->GetRoot(), this, m_Data);
        break;
        
      case c_Zoom:
        new MGUIImageDimensions(gClient->GetRoot(), this, m_Data);
        break;
        
      case c_ImageOptions:
        new MGUIImageOptions(gClient->GetRoot(), this, m_Data);
        break;

      case c_Memory:
        new MGUIMemory(gClient->GetRoot(), this, m_Data);
        break;

      case c_LikelihoodAlgorithm:
        new MGUIDeconvolution(gClient->GetRoot(), this, m_Data);
        break;
        
//       case c_LikelihoodPenalty:
//         new MGUIPenalty(gClient->GetRoot(), this, m_Data);
//         break;
        
      case c_StartReconstruction:
        Launch(false);
        break;
        
      case c_Animation:
        new MGUIAnimation(gClient->GetRoot(), this, m_Data, OKPressed);
        if (OKPressed == true) {
          Launch(true);
        }
        break;
        
      case c_StopReconstruction:
        m_Interface->InterruptReconstruction();
        break;

      case c_ShowEventSelections:
        m_Interface->ShowEventSelections();
				break;

      case c_ShowEventSelectionsStepwise:
        m_Interface->ShowEventSelectionsStepwise();
				break;

      case c_ExtractEvents:
        m_Interface->ExtractEvents();
				break;

      case c_ThetaOriginDistribution:
        m_Interface->ThetaOriginDistribution();
				break;
        
      case c_SpectralAnalyzer:
        new MGUISpectralAnalyzer(gClient->GetRoot(), this, m_Data, &OKPressed);
        if (OKPressed == true) {
          m_Interface->SpectralAnalyzer();
        }        
        break;
        
      case c_ResponseSignificanceMap:
        new MGUISignificance(gClient->GetRoot(), this, m_Data, OKPressed);
        if (OKPressed == true) {
          m_Interface->SignificanceMap();
        }
        break;
        
      case c_ResponseArmGamma:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMGamma();
        }
        break;
        
      case c_ResponseArmComparison:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMResponseComparison();
        }
        break;
        
      case c_ResponseArmGammaVsCompton:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMGammaVsCompton();
        }
        break;
        
      case c_ResponseArmGammaVsDistance:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMGammaVsDistance();
        }
        break;
        
      case c_ResponseArmGammaVsComptonProbability:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMGammaVsComptonProbability();
        }
        break;
        
      case c_ResponseArmGammaVsClusteringProbability:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMGammaVsClusteringProbability();
        }
        break;
        
      case c_ResponseSPDVsTrackQualityFactor:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->SPDVsTrackQualityFactor();
        }
        break;
        
      case c_ResponseSPDVsTotalScatterAngleDeviation:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->SPDVsTotalScatterAngleDeviation();
        }
        break;
        
      case c_ResponseEnergyVsComptonProbability:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_Spectrum, OKPressed);
        if (OKPressed == true) {
          m_Interface->EnergyVsComptonProbability();
        }
        break;

      case c_ResponseComptonSequenceLengthVsComptonProbability:
        m_Interface->ComptonSequenceLengthVsComptonProbability();
        break;

      case c_ResponseComptonProbabilityWithARMSelection:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->ComptonProbabilityWithARMSelection();
        }
        break;

      case c_ResponseSpdElectron:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->SPDElectron();
        }
        break;

      case c_ResponseSpdElectronVsCompton:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->SPDElectronVsCompton();
        }
        break;

      case c_ResponseArmElectron:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->ARMElectron();
        }
        break;

      case c_ResponseEarthCenterDistance:
        m_Interface->EarthCenterDistance();
        break;

      case c_ResponseSpectrum:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_Spectrum, OKPressed);
        if (OKPressed == true) {
          m_Interface->EnergySpectra();
        }
        break;

      case c_ResponseInitialEnergyDeposit:
        m_Interface->InitialEnergyDeposit();
        break;

      case c_ResponseEnergyDistributionD1D2:
        m_Interface->EnergyDistributionElectronPhoton();
        break;

      case c_ResponseTime:
        m_Interface->TimeDistribution();
        break;

      case c_ResponseCoincidenceWindow:
        m_Interface->CoincidenceWindowDistribution();
        break;

      case c_ResponseTimeWalk:
        m_Interface->TimeWalkDistribution();
        break;

      case c_ResponsePhi:
        m_Interface->ScatterAnglesDistribution();
        break;

      case c_ResponseClusteringQualityFactor:
        m_Interface->ClusteringQualityFactor();
        break;

      case c_ResponseComptonQualityFactor:
        m_Interface->ComptonQualityFactor();
        break;

      case c_ResponseTrackQualityFactor:
        m_Interface->TrackQualityFactor();
        break;

      case c_ResponseDistance:
        m_Interface->DistanceDistribution();
        break;

      case c_ResponseSequenceLengths:
        m_Interface->SequenceLengths();
        break;

      case c_ResponseAzimuthalComptonScatterAngle:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->AzimuthalComptonScatterAngle();
        }
        break;

      case c_ResponseAzimuthalElectronScatterAngle:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMElectron, OKPressed);
        if (OKPressed == true) {
          m_Interface->AzimuthalElectronScatterAngle();
        }
        break;

      case c_ResponsePolarization:
        new MGUIPolarization(gClient->GetRoot(), this, m_Data, OKPressed);
        if (OKPressed == true) {
          m_Interface->Polarization();
        }
        break;

      case c_ResponseStandardAnalysis:
        //new MGUIPolarization(gClient->GetRoot(), this, m_Data, OKPressed);
        //if (OKPressed == true) {
        //m_Interface->StandardAnalysis();
        //}
        break;

      case c_ResponseTest:
        m_Interface->TestOnly();
        break;

      case c_ResponseOpeningAnglePair:
        m_Interface->OpeningAnglePair();
        break;

      case c_ResponseAngularResolutionPair:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->AngularResolutionPairs();
        }
        break;

      case c_ResponseAngularResolutionVsQualityFactorPair:
        new MGUIARM(gClient->GetRoot(), this, m_Data, MGUIARMModes::m_ARMGamma, OKPressed);
        if (OKPressed == true) {
          m_Interface->AngularResolutionVsQualityFactorPair();
        }
        break;

      case c_ResponseSelectIds:
        m_Interface->SelectIds();
        break;

      case c_ResponseLocationOfInitialInteraction:
        m_Interface->LocationOfInitialInteraction();
        break;

      case c_ResponsePointingInGalacticCoordinates:
        m_Interface->PointingInGalacticCoordinates();
        break;

      case c_ResponseHorizonInSphericalDetectorCoordinates:
        m_Interface->HorizonInSphericalDetectorCoordinates();
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


void MGUIMimrecMain::Open()
{
  // Open a file

  const char** Types = new const char*[8];
  Types[0] = "Tracked events";
  Types[1] = "*.tra";
  Types[2] = "Gzip'ed tracked events";
  Types[3] = "*.tra.gz";
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

  delete [] Types;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMimrecMain::About()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Mimrec");
  About->SetIconPath(g_MEGAlibPath + "/resource/icons/mimrec/Small.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  About->SetReference("Implementation details of the imaging approach", 
                      "A. Zoglauer et al., \"Design, implementation, and optimization of MEGAlib's image reconstruction tool Mimrec \", NIM A 652, 2011");
  About->SetReference("A detailed description of list-mode likelihood image reconstruction - in German", 
                      "A. Zoglauer, \"Methods of image reconstruction for the MEGA Compton telescope\", Diploma thesis, TU Munich, 2000");
  //About->SetReference("Chapter 5: List-mode image reconstruction applied to the MEGA telecope", 
  //                    "A. Zoglauer, \"First Light for the Next Generation of Compton and Pair Telescopes\", Doctoral thesis, TU Munich, 2005");
  About->Create();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMimrecMain::Launch(bool Animate)
{
  // This method is called after pressing the start button
  
  MString Name = m_Data->GetCurrentFileName();

  if (Name.EndsWith("tra") == false && Name.EndsWith("tra.gz") == false) {
    mgui<<"The input file of Mimrec needs to have the suffix \".tra\" or \".tra.gz\"!"<<error;
    return;
  }

  m_Interface->Reconstruct(Animate);
}


// MGUIMimrecMain: the end...
////////////////////////////////////////////////////////////////////////////////
