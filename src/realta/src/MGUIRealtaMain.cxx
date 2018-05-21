/*
 * MGUIRealtaMain.cxx
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
// MGUIRealtaMain
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIRealtaMain.h"

// Standard libs:
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <memory>
using namespace std;

// ROOT libs:
#include <TGFileDialog.h>
#include <TGMsgBox.h>
#include <TGStatusBar.h>
#include <TFile.h>
#include <TApplication.h>
#include <TKey.h>
#include <TArrow.h>
#include <TText.h>
#include <TBox.h>
#include <TObject.h>
#include <TList.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"
#include "MDGeometryQuest.h"
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUINetwork.h"
#include "MGUIAccumulation.h"

#include "MGUIERAlgorithm.h"
#include "MGUIOptionsCoincidence.h"
#include "MGUIOptionsClustering.h"
#include "MGUIOptionsTracking.h"
#include "MGUIOptionsCSR.h"
#include "MGUIOptionsGeneral.h"

#include "MGUIBPAlgorithm.h"
#include "MGUICoordinateSystem.h"
#include "MGUIDeconvolution.h"
#include "MGUIEventSelection.h"
#include "MGUIResponseSelection.h"
#include "MGUIResponseParameterGauss1D.h"
#include "MGUIResponseParameterGaussianByUncertainties.h"
#include "MGUIResponseParameterConeShapes.h"
#include "MGUIResponseParameterPRM.h"
#include "MGUIMemory.h"
#include "MGUIImageDimensions.h"
#include "MGUIImageOptions.h"

#include "MGUISpectralAnalyzer.h"

#include "MSettingsMimrec.h"
#include "MImage.h"
#include "MImage2D.h"
#include "MImageSpheric.h"
#include "MBPData.h"
#include "MQualifiedIsotope.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIRealtaMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIRealtaMain::MGUIRealtaMain() : TGMainFrame(gClient->GetRoot(), 800, 800)
{
  // Construct an instance of MGUIRealtaMain and bring it to the screen
  
  m_Settings = new MSettingsRealta();
  
  m_Geometry = new MDGeometryQuest();
  if (m_Geometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_Settings->GetGeometryFileName()<<" failed! Trying the dummy geometry..."<<endl;
    if (m_Geometry->ScanSetupFile("$(MEGALIB)/resource/examples/geomega/special/Dummy.geo.setup", false) == false) {
      cout<<"Loading of dummy geometry "<<m_Geometry->GetName()<<" failed, too!"<<endl;
      cout<<"Delete your ~/.realta.cfg file and make sure MEGAlib is correctly installed! Aborting!"<<endl;
      gApplication->Terminate(0);
    }
  }

  Create();
  
  m_Analyzer = new MRealTimeAnalyzer();
  m_Analyzer->SetSettings(m_Settings);
  if (m_Settings->GetConnectOnStart() == true) {
    m_Analyzer->Connect();  
  }
}


////////////////////////////////////////////////////////////////////////////////


MGUIRealtaMain::~MGUIRealtaMain()
{
  // Delete an instance of MGUIRealtaMain
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Realta - Real-time analysis for MEGAlib");  


  // In the beginning we build the menus and define their layout, ... 
  TGLayoutHints* MenuBarItemLeftLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  TGLayoutHints* MenuBarItemRightLayout = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  TGLayoutHints* MenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  TGMenuBar* MenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);
  
  TGPopupMenu* MenuFile = new TGPopupMenu(gClient->GetRoot());
  MenuFile->AddEntry("Load ...", c_Load);
  MenuFile->AddEntry("Save as ...", c_Save);
  MenuFile->AddSeparator();
  MenuFile->AddEntry("Exit", c_Exit);
  MenuFile->Associate(this);
  MenuBar->AddPopup("File", MenuFile, MenuBarItemLeftLayout);
    
  TGPopupMenu* MenuSettings = new TGPopupMenu(gClient->GetRoot());
  MenuSettings->AddLabel("Important: Reset the connection to apply the settings!");
  MenuSettings->AddSeparator();
  MenuSettings->AddLabel("General settings");
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Threads, Accumulation, Histogramming, and Saving", c_Accumulation);
  MenuSettings->AddEntry("Network options", c_Network);
  MenuSettings->AddEntry("Geometry", c_Geometry);
  MenuSettings->AddSeparator();
  MenuSettings->AddLabel("Event reconstruction settings");
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Selection of algorithm", c_EventReconstruction);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Coincidence options", c_Coincidence);
  MenuSettings->AddEntry("Clustering options", c_Clustering);
  MenuSettings->AddEntry("Tracking options", c_Tracking);
  MenuSettings->AddEntry("Sequencing options", c_Sequencing);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("General event selections", c_EventReconstructionSelection);
  MenuSettings->AddSeparator();
  MenuSettings->AddLabel("Image reconstruction settings");
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Event selection", c_ImagingSelection);
  MenuSettings->AddEntry("Coordinate system", c_CoordinateSystem);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Image dimensions", c_Zoom);
  MenuSettings->AddEntry("Image drawing options", c_ImageOptions);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Memory, accuracy, and thread management", c_Memory);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Reconstruction algorithm", c_LikelihoodAlgorithm);
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Response type selection", c_Response);
  MenuSettings->AddEntry("Response parameter (for the above selected type)", c_FitParameter);
  MenuSettings->AddSeparator();
  MenuSettings->AddLabel("Spectral indentification settings");
  MenuSettings->AddSeparator();
  MenuSettings->AddEntry("Spectralyzer options", c_Spectralyze);
  MenuSettings->Associate(this);
  MenuBar->AddPopup("Settings", MenuSettings, MenuBarItemLeftLayout);

  TGPopupMenu* MenuTasks = new TGPopupMenu(fClient->GetRoot());
  MenuTasks->AddEntry("Connect", c_Connect);
  MenuTasks->AddEntry("Disconnect", c_Disconnect);
  MenuTasks->AddEntry("Reset", c_Reset);
  MenuTasks->Associate(this);
  MenuBar->AddPopup("Tasks", MenuTasks, MenuBarItemLeftLayout);

  TGPopupMenu* MenuInfo = new TGPopupMenu(fClient->GetRoot());
  MenuInfo->AddEntry("About", c_About);
  MenuInfo->Associate(this);
  MenuBar->AddPopup("Info", MenuInfo, MenuBarItemRightLayout);
  
  AddFrame(MenuBar, MenuBarLayout);

  
  // The columns
  TGLayoutHints* ColumnsLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 10);
  TGHorizontalFrame* Columns = new TGHorizontalFrame(this, 150, 1500); //, kRaisedFrame);
  AddFrame(Columns, ColumnsLayout);

  TGLayoutHints* ColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY | kLHintsExpandX, 10, 10, 10, 10);

  
  // The status column
  
  int StatusColumnWidth = 250;
  
  TGLayoutHints* StatusColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 10, 10, 10, 10);
  TGVerticalFrame* StatusColumn = new TGVerticalFrame(Columns, StatusColumnWidth, StatusColumnWidth); //, kRaisedFrame);
  Columns->AddFrame(StatusColumn, StatusColumnLayout);
  

  
  TGLayoutHints* SubTitleLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 2, 2, 30, 2);
  
 
  // Title:
  MString TitleIconName(g_MEGAlibPath + "/resource/icons/realta/Realta.xpm");
  MFile::ExpandFileName(TitleIconName);
  
  TGLayoutHints* TitleIconLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 2, 2, 10, 2);
  if (MFile::Exists(TitleIconName) == true) {
    const TGPicture* TitlePicture = fClient->GetPicture(TitleIconName);
    if (TitlePicture == 0) {
      mgui<<"Can't find picture "<<TitleIconName<<"! Aborting!"<<error;
      return;
    }
    TGIcon* TitleIcon = new TGIcon(StatusColumn, TitlePicture, TitlePicture->GetWidth()+2, TitlePicture->GetHeight()+2);
    StatusColumn->AddFrame(TitleIcon, TitleIconLayout);
  } else {
    FontStruct_t LabelFont;
    LabelFont = gClient->GetFontByName("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-1"); 
    TGGC* Graphics = new TGGC(TGLabel::GetDefaultGC());
    Graphics->SetFont(gVirtualX->GetFontHandle(LabelFont));
    TGLabel* Title = new TGLabel(StatusColumn, "Realta", Graphics->GetGC(), LabelFont);
    StatusColumn->AddFrame(Title, TitleIconLayout);
  }

  TGLabel* RealtaTitle = new TGLabel(StatusColumn, "Real-time Analysis for MEGAlib");
  StatusColumn->AddFrame(RealtaTitle, TitleIconLayout);

  
  // Connection status
  TGLabel* ConnectionTitle = new TGLabel(StatusColumn, "Connection status:");
  StatusColumn->AddFrame(ConnectionTitle, SubTitleLayout);
  
  TGLayoutHints* ConnectionStatusLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  m_ConnectionStatus = new TGLabel(StatusColumn, "NOT CONNECTED!");
  StatusColumn->AddFrame(m_ConnectionStatus, ConnectionStatusLayout);

  
  // Rates

  TGLabel* RatesTitle = new TGLabel(StatusColumn, "Triggered event rate");
  StatusColumn->AddFrame(RatesTitle, SubTitleLayout);
  
  TGLayoutHints* CountRateCanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  m_CountRateCanvas = new TRootEmbeddedCanvas("CountRateCanvas", StatusColumn, StatusColumnWidth, 3*StatusColumnWidth/4);
  StatusColumn->AddFrame(m_CountRateCanvas, CountRateCanvasLayout);
  
  
  // Threads
  TGLabel* ThreadsTitle = new TGLabel(StatusColumn, "Thread utilization");
  StatusColumn->AddFrame(ThreadsTitle, SubTitleLayout);

  TGHorizontalFrame* ThreadsFrame = new TGHorizontalFrame(StatusColumn, 100, 30);
  TGLayoutHints* ThreadFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1);
  StatusColumn->AddFrame(ThreadsFrame, ThreadFrameLayout);

  TGLayoutHints* UsageLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 1, 1);
  TGLayoutHints* UsageLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 3, 1, 1, 1);

  
  // Label
  TGVerticalFrame* ThreadLabelsFrame = new TGVerticalFrame(ThreadsFrame, 100, 300); //, kRaisedFrame);
  TGLayoutHints* ThreadLabelsFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 1, 1);
  ThreadsFrame->AddFrame(ThreadLabelsFrame, ThreadLabelsFrameLayout);

  TGLabel* TransmissionThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Receiving:");
  ThreadLabelsFrame->AddFrame(TransmissionThreadCpuUsageLabel, UsageLabelLayout);
  
  TGLabel* CoincidenceThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Coincidence:");
  ThreadLabelsFrame->AddFrame(CoincidenceThreadCpuUsageLabel, UsageLabelLayout);
  
  TGLabel* ReconstructionThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Recon:");
  ThreadLabelsFrame->AddFrame(ReconstructionThreadCpuUsageLabel, UsageLabelLayout);
  
  TGLabel* ImagingThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Imaging:");
  ThreadLabelsFrame->AddFrame(ImagingThreadCpuUsageLabel, UsageLabelLayout);

  TGLabel* IdentificationThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Identification:");
  ThreadLabelsFrame->AddFrame(IdentificationThreadCpuUsageLabel, UsageLabelLayout);

  TGLabel* HistogrammingThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Histograms:");
  ThreadLabelsFrame->AddFrame(HistogrammingThreadCpuUsageLabel, UsageLabelLayout);

  TGLabel* CleanUpThreadCpuUsageLabel = new TGLabel(ThreadLabelsFrame, "Cleanup:");
  ThreadLabelsFrame->AddFrame(CleanUpThreadCpuUsageLabel, UsageLabelLayout);

  
  // Usage
  TGVerticalFrame* ThreadUsageFrame = new TGVerticalFrame(ThreadsFrame, 100, 30); //, kRaisedFrame);
  TGLayoutHints* ThreadUsageFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1);
  ThreadsFrame->AddFrame(ThreadUsageFrame, ThreadUsageFrameLayout);

  m_TransmissionThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_TransmissionThreadCpuUsage, UsageLayout);

  m_CoincidenceThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_CoincidenceThreadCpuUsage, UsageLayout);

  m_ReconstructionThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_ReconstructionThreadCpuUsage, UsageLayout);

  m_ImagingThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_ImagingThreadCpuUsage, UsageLayout);

  m_IdentificationThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_IdentificationThreadCpuUsage, UsageLayout);

  m_HistogrammingThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_HistogrammingThreadCpuUsage, UsageLayout);

  m_CleanUpThreadCpuUsage = new TGLabel(ThreadUsageFrame, "CPU: 0.0 %    ");
  ThreadUsageFrame->AddFrame(m_CleanUpThreadCpuUsage, UsageLayout);

  
  // IDs
  TGVerticalFrame* ThreadEventIDFrame = new TGVerticalFrame(ThreadsFrame, 100, 30); //, kRaisedFrame);
  TGLayoutHints* ThreadEventIDFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 1, 1, 1, 1);
  ThreadsFrame->AddFrame(ThreadEventIDFrame, ThreadEventIDFrameLayout);

  m_TransmissionThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_TransmissionThreadLastEventID, UsageLayout);

  m_CoincidenceThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_CoincidenceThreadLastEventID, UsageLayout);

  m_ReconstructionThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_ReconstructionThreadLastEventID, UsageLayout);

  m_ImagingThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_ImagingThreadLastEventID, UsageLayout);

  m_IdentificationThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_IdentificationThreadLastEventID, UsageLayout);

  m_HistogrammingThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_HistogrammingThreadLastEventID, UsageLayout);

  m_CleanUpThreadLastEventID = new TGLabel(ThreadEventIDFrame, "ID: 0          ");
  ThreadEventIDFrame->AddFrame(m_CleanUpThreadLastEventID, UsageLayout);

  
 
  
  // The general view column
  
    
  TGVerticalFrame* GeneralColumn = new TGVerticalFrame(Columns, 100, 100); //, kRaisedFrame);
  Columns->AddFrame(GeneralColumn, ColumnLayout);

  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);

  m_SpectrumCanvas = new TRootEmbeddedCanvas("SpectrumCanvas", GeneralColumn, 100, 100);
  GeneralColumn->AddFrame(m_SpectrumCanvas, CanvasLayout);

  m_ImageCanvas = new TRootEmbeddedCanvas("ImageCanvas", GeneralColumn, 100, 100);
  GeneralColumn->AddFrame(m_ImageCanvas, CanvasLayout);

  
  
  // The status bar
  TGStatusBar* StatusBar = new TGStatusBar(this, 100, 10);
  TGLayoutHints* StatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 0, 0, 0, 0);

  AddFrame(StatusBar, StatusBarLayout);

  
  
  // PositionWindow(GetDefaultWidth(), GetDefaultHeight());
  // PositionWindow(640, 480, kFALSE);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MGUIRealtaMain::ProcessMessage(Long_t Message, Long_t Parameter1, 
                                 Long_t Parameter2)
{
  // Process the messages for this application

  //cout<<"Message: "<<Message<<", "<<Parameter1<<", "<<Parameter2<<", "<<c_About<<endl;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_MENU:
      switch (Parameter1) {
       case c_Load: 
        OnLoad();
        break;
        
       case c_Save: 
        OnSave();
        break;
        
       case c_Exit: 
        OnExit();
        break;
        
      case c_About:
        OnAbout();
        break;
        
      case c_Network: 
        OnNetwork();
        break;
        
      case c_Accumulation: 
        OnAccumulation();
        break;
        
      case c_Geometry: 
        OnGeometry();
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
        new MGUIOptionsClustering(gClient->GetRoot(), this, m_Settings);
        break;

      case c_Sequencing:
        new MGUIOptionsCSR(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_ImagingSelection:
        new MGUIEventSelection(gClient->GetRoot(), this, m_Settings, NULL);
        break;
        
      case c_CoordinateSystem:
        new MGUICoordinateSystem(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_Zoom:
        new MGUIImageDimensions(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_ImageOptions:
        new MGUIImageOptions(gClient->GetRoot(), this, m_Settings);
        break;

      case c_Memory:
        new MGUIMemory(gClient->GetRoot(), this, m_Settings);
        break;

      case c_LikelihoodAlgorithm:
        new MGUIDeconvolution(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_Response:
        new MGUIResponseSelection(gClient->GetRoot(), this, m_Settings);
        break;
        
      case c_Spectralyze:
        new MGUISpectralAnalyzer(gClient->GetRoot(), this, m_Settings);
        break;

      case c_FitParameter:
        if (m_Settings->GetResponseType() == MResponseType::Gauss1D) {
          new MGUIResponseParameterGauss1D(gClient->GetRoot(), this, m_Settings);
        } else if (m_Settings->GetResponseType() == MResponseType::GaussByUncertainties) {
          new MGUIResponseParameterGaussianByUncertainties(gClient->GetRoot(), this, m_Settings);
        } else if (m_Settings->GetResponseType() == MResponseType::ConeShapes) {
          new MGUIResponseParameterConeShapes(gClient->GetRoot(), this, m_Settings);
        } else if (m_Settings->GetResponseType() == MResponseType::PRM) {
          new MGUIResponseParameterPRM(gClient->GetRoot(), this, m_Settings);
        }
        break;
      
      case c_Connect: 
        OnConnect();
        break;
        
      case c_Disconnect: 
        OnDisconnect();
        break;
        
      case c_Reset: 
        OnReset();
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
  
  return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::CloseWindow()
{
  // Call exit for controlled good-bye

  OnExit();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnLoad()
{
  //! Load a configuration file 

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  delete [] Types;
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Settings->Read(Info.fFilename);
    delete m_Geometry;
    m_Geometry = new MDGeometryQuest();
    if (m_Geometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
      cout<<"Loading of geometry "<<m_Geometry->GetName()<<" failed!!"<<endl;
      return;
    }
    m_Analyzer->SetSettings(m_Settings);
  } 
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnSave()
{
  //! Save a configuration file

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;
  
  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  delete [] Types;
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Settings->Write(Info.fFilename);
  } 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnExit()
{
  // Save and good bye
  
  m_Analyzer->StopAnalysis();
  m_Settings->Write();
  
  gApplication->Terminate(0);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnAbout()
{
  // Bring up the about GUI
  
  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Realta");
  // About->SetIconPath(g_MEGAlibPath + "/resource/icons/mimrec/Small.xpm");
  About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer\nAll rights reserved");
  //About->SetReference("Implementation details of the imaging approach", 
  //                   "A. Zoglauer et al., \"Design, implementation, and optimization of MEGAlib's image reconstruction tool Mimrec \", NIM A, 2010, in press");
  About->Create();
}

 
////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnNetwork()
{
  new MGUINetwork(gClient->GetRoot(), this, m_Settings);
}

 
////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnAccumulation()
{
  // Show the geometry dialog

  MGUIAccumulation* A = new MGUIAccumulation(gClient->GetRoot(), this, m_Settings);
  gClient->WaitForUnmap(A);
  //delete A;
  for (unsigned int i = 0; i < 100; ++i) gSystem->ProcessEvents();
  m_Analyzer->SetAccumulationTime(m_Settings->GetAccumulationTime());
  cout<<"Done with Accumulation..."<<endl;
}

 
////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnGeometry()
{
  // Show the geometry dialog

  MGUIGeometry* Geo = new MGUIGeometry(gClient->GetRoot(), this, m_Settings);
  gClient->WaitForUnmap(Geo);
  MString Name = Geo->GetGeometryFileName();
  delete Geo;
  for (unsigned int i = 0; i < 100; ++i) {
    gSystem->ProcessEvents();
  }

  m_Settings->SetGeometryFileName(Name);
  
  // Load a geometry for gui-handling:
  delete m_Geometry;
  m_Geometry = new MDGeometryQuest();
  if (m_Geometry->ScanSetupFile(m_Settings->GetGeometryFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_Geometry->GetName()<<" failed!!"<<endl;
    return;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnConnect()
{
  //! Connect to the other machine
  
  m_Analyzer->Connect();  
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnDisconnect()
{
  //! Disconnect from the other machine

  m_Analyzer->Disconnect();      
}


////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::OnReset()
{
  //! Reset the analysis
  
  m_Analyzer->Reset();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIRealtaMain::DoControlLoop()
{
  // This is the main GUI loop
  
  bool AnImageShown = false;
  MTimer CanvasTimer;
  MTimer ThreadTimer;
  
  while (true) {
    gSystem->ProcessEvents();
    gSystem->Sleep(5);
  
    //continue;
    if (ThreadTimer.GetElapsed() > 1) {
      //cout<<"updating GUI..."<<endl;
      ThreadTimer.Reset();
      
      ostringstream usage;
      usage<<setprecision(1)<<fixed;
      
      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetTransmissionThreadCpuUsage()<<" %";
      if (MString(m_TransmissionThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_TransmissionThreadCpuUsage->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetTransmissionThreadLastEventID();
      if (MString(m_TransmissionThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_TransmissionThreadLastEventID->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetCoincidenceThreadCpuUsage()<<" %";
      if (MString(m_CoincidenceThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_CoincidenceThreadCpuUsage->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetCoincidenceThreadLastEventID();
      if (MString(m_CoincidenceThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_CoincidenceThreadLastEventID->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetReconstructionThreadCpuUsage()<<" %";
      if (MString(m_ReconstructionThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_ReconstructionThreadCpuUsage->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetReconstructionThreadLastEventID();
      if (MString(m_ReconstructionThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_ReconstructionThreadLastEventID->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetImagingThreadCpuUsage()<<" %";
      if (MString(m_ImagingThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_ImagingThreadCpuUsage->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetImagingThreadLastEventID();
      if (MString(m_ImagingThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_ImagingThreadLastEventID->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetCleanUpThreadCpuUsage()<<" %";
      if (MString(m_CleanUpThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_CleanUpThreadCpuUsage->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetCleanUpThreadLastEventID();
      if (MString(m_CleanUpThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_CleanUpThreadLastEventID->SetText(usage.str().c_str());
      }
      
      MString ConnectionString;
      if (m_Analyzer->IsConnected() == false) {
        ConnectionString = "NOT CONNECTED!";
      } else {
        ConnectionString = "Connected";        
      }
      if (m_ConnectionStatus->GetText()->Data() != ConnectionString) {
        m_ConnectionStatus->SetText(ConnectionString.Data()); 
      }
   }
      
   if (CanvasTimer.GetElapsed() > 2 || AnImageShown == false) {
     CanvasTimer.Reset();

      if (m_Analyzer->GetCountRateHistogram() != 0) {
        // The whole thing here is to prevent a crash when we are interacting with the canvas during an draw
        TIter Next(m_CountRateCanvas->GetCanvas()->GetListOfPrimitives());
        bool FoundInteraction = false;
        TObject* O = 0;
        while ((O = Next())) {
          if (TString("TPad") == O->ClassName()) {
            FoundInteraction = true; 
          }
        }
        if (FoundInteraction == false) {
          m_CountRateCanvas->GetCanvas()->cd();
          m_Analyzer->GetCountRateHistogram()->Draw();
          m_CountRateCanvas->GetCanvas()->Update();
          AnImageShown = true;
        }
      }
      
      gSystem->ProcessEvents();
      if (m_Analyzer->GetSpectrumHistogram() != 0) {
        // The whole thing here is to prevent a crash when we are interacting with the canvas during an draw
        TIter Next(m_SpectrumCanvas->GetCanvas()->GetListOfPrimitives());
        bool FoundInteraction = false;
        TObject* O = 0;
        while ((O = Next())) {
          if (TString("TPad") == O->ClassName()) {
            FoundInteraction = true; 
          }
        }
        if (FoundInteraction == false) {
          m_SpectrumCanvas->GetCanvas()->cd();
          m_Analyzer->GetSpectrumHistogram()->SetMaximum(-1111);
          m_Analyzer->GetSpectrumHistogram()->SetMaximum(1.35*m_Analyzer->GetSpectrumHistogram()->GetMaximum());
          m_Analyzer->GetSpectrumHistogram()->Draw();
          double Width = m_Analyzer->GetSpectrumHistogram()->GetXaxis()->GetXmax() - m_Analyzer->GetSpectrumHistogram()->GetXaxis()->GetXmin();
          
          // Highlight the windows
          vector<double> Min = m_Analyzer->GetMinimaOfSpectralWindows();
          vector<double> Max = m_Analyzer->GetMaximaOfSpectralWindows();
          for (unsigned int i = 0; i < Min.size(); ++i) {
            if (Max[i] == 0) continue;
            TBox* Box = new TBox(Min[i], 0.0, Max[i], m_Analyzer->GetSpectrumHistogram()->GetMaximum());
            Box->SetFillStyle(0);
            Box->SetLineColor(kBlue+3);
            Box->Draw("SAME");
          }
          //m_Analyzer->GetSpectrumHistogram()->Draw("SAME");
          
          // Print the isotopes
          vector<MQualifiedIsotope> I = m_Analyzer->GetIsotopes();
          for (unsigned int i = 0; i < I.size(); ++i) {
            for (unsigned int l = 0; l < I[i].GetNLines(); ++l) {
              if (I[i].GetLineFound(l) == false) continue;
              double YMax = m_Analyzer->GetSpectrumHistogram()->GetMaximum();
              double Max = m_Analyzer->GetSpectrumHistogram()->GetXaxis()->GetXmax();
              double Min = m_Analyzer->GetSpectrumHistogram()->GetXaxis()->GetXmin();
              double X = (I[i].GetLineEnergy(l) - Min) / (Max - Min);
              X = I[i].GetLineEnergy(l);
              //TArrow* A = new TArrow(X, 0.98*YMax, X, 0.7*YMax, 0.03);
              //A->SetFillColor(1);
              //A->Draw();
              MString Name = I[i].GetElement();
              if (I[i].GetNucleons() > 0) {
                Name += "-";
                Name += I[i].GetNucleons();
              }
              TText* T = new TText(X+0.01*Width, 0.75*YMax, Name);
              T->SetTextAngle(90);
              T->Draw();
              //break; // Hack: Only show the highest energy isotope
            }
          }
          
          m_SpectrumCanvas->GetCanvas()->Update();
          AnImageShown = true;
        }
      }
      
      gSystem->ProcessEvents();
      shared_ptr<MImage> Image = m_Analyzer->GetImage();
      if (Image != nullptr) {
        // The whole thing here is to prevent a crash when we are interacting with the canvas during an draw
        TIter Next(m_ImageCanvas->GetCanvas()->GetListOfPrimitives());
        bool FoundInteraction = false;
        TObject* O = 0;
        while ((O = Next())) {
          if (TString("TPad") == O->ClassName()) {
            FoundInteraction = true; 
          }
        }
        if (FoundInteraction == false) {
          m_ImageCanvas->GetCanvas()->cd();
          Image->Display(m_ImageCanvas->GetCanvas());
          m_ImageCanvas->GetCanvas()->Update();
          AnImageShown = true;
        }
      }
      gSystem->ProcessEvents();

      ostringstream usage;
      usage<<setprecision(1)<<fixed;

      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetHistogrammingThreadCpuUsage()<<" %";
      if (MString(m_HistogrammingThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_HistogrammingThreadCpuUsage->SetText(usage.str().c_str());
      }
      
      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetHistogrammingThreadLastEventID();
      if (MString(m_HistogrammingThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_HistogrammingThreadLastEventID->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"CPU: "<<100*m_Analyzer->GetIdentificationThreadCpuUsage()<<" %";
      if (MString(m_IdentificationThreadCpuUsage->GetText()->Data()) != usage.str().c_str()) {
        m_IdentificationThreadCpuUsage->SetText(usage.str().c_str());
      }

      usage.str("");
      usage<<"ID: "<<m_Analyzer->GetIdentificationThreadLastEventID();
      if (MString(m_IdentificationThreadLastEventID->GetText()->Data()) != usage.str().c_str()) {
        m_IdentificationThreadLastEventID->SetText(usage.str().c_str());
      }

      gSystem->ProcessEvents();
    }
  }
}


// MGUIRealtaMain: the end...
////////////////////////////////////////////////////////////////////////////////
