/*
 * MGUIControlCenter.cxx                                            v0.9  05/04/2001
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
// MGUIControlCenter
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIControlCenter.h"

// Standard libs:
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// ROOT libs:
#include <TGFileDialog.h>
#include <TGStatusBar.h>
#include <TFile.h>
#include <TKey.h>

// MEGAlib libs:
#include "MObjectReceiver.h"
#include "MEventFacets.h"
#include "MEventFacetsList.h"
#include "MHitData.h"
#include "MHitEvent.h"
#include "MHitEventAnalyzer.h"
#include "MPhysicalEventAnalyzer.h"
#include "MDGeometryQuest.h"
#include "MSettingsMimrec.h"
#include "MImage2D.h"
#include "MImage2DUpdate.h"
#include "MImageUpdate.h"
#include "MImageAstro.h"
#include "MBPData.h"
#include "MEventData.h"
#include "MHouseData.h"
#include "MStreams.h"
#include "MChip.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIControlCenter)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIControlCenter::MGUIControlCenter(const TGWindow *Parent, const TGWindow *Main, MSettingsRealta *Data)
  : TGMainFrame(Parent, 400, 500)
{
  // Construct an instance of MGUIControlCenter and bring it to the screen

  //Fatal("", "");

  m_Parent = (TGWindow *) Parent;
  m_GUIData = Data;

  SetIconPixmap("resource/icons/RealtaIconLarge2.xpm");

  m_StoredEvents = 1000;
  m_UpdateFrequency = 100;
  m_StopControlLoop = kFALSE;

  m_Receiver = new TObjectReceiver(9090, kTRUE);

  m_Events = new MEventFacetsList(m_StoredEvents);

  MDGeometryQuest *Geo = new MDGeometryQuest();
  Geo->ScanSetupFile("examples/CalibPart.v1.12.geo.setup");

  m_HEAnalyzer = new MHitEventAnalyzer(Geo);

  m_PEAnalyzer = 0;
  m_GUIDataImaging = new MSettingsMimrec();

  m_FullSpectrum = 0;
  m_BPImage = 0;

  m_ImagesChipPedestal = new TObjArray();
  m_ImagesChipNoise = new TObjArray();
  m_ImagesChipCommonMode = new TObjArray();
  m_ImagesChipTriggerRate = new TObjArray();
  m_NChips = 0;

  m_LastTime = 0;
  m_LastObjects = 0;

  // Storage information:
  m_EventStreamer = 0;
  m_ObjectStreamer = 0;
  m_NReceivedObjects = 0;
  m_NReceivedEvents = 0;
  m_NReceivedHouseKeeping = 0;

  //
  m_SafeMode = kTRUE;
  m_ReadModeTcpIp = kTRUE;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIControlCenter::~MGUIControlCenter()
{
  // Delete an instance of MGUIControlCenter

  m_LogStream<<"Closing the files"<<endl;
  if (m_ObjectStreamer != 0) {
    m_ObjectStreamer->Close();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Realta - Real time analysis control center");  


  // Add here ...

  // The upper part of the window, containing a picture and the network statitics 
  m_UpperFrame = new TGCompositeFrame(this, 60, 60, kHorizontalFrame);
  m_UpperFrameLayout = new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 0);
  AddFrame(m_UpperFrame, m_UpperFrameLayout);


  // Add a nice picture ("REALTA"):
  m_PictureButton = new TGPictureButton(m_UpperFrame, fClient->GetPicture("resource/icons/Realta2.xpm"), 1);
  m_PictureButton->SetState(kButtonDisabled);
  m_PictureButton->ChangeOptions(0);
  m_PictureButton->Associate(this);
  m_PictureButtonLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0);
  m_UpperFrame->AddFrame(m_PictureButton, m_PictureButtonLayout);

  // Add the network statistic:
  m_NetworkHistory =  new MImageUpdate("Received objects", 
                                       0, 
                                       100, 
                                       "History", 
                                       0, 
                                       100, 
                                       100,
                                       MImageUpdate::c_History, 
                                       0, 
                                       0);
  m_NetworkStatistics = new MGUIEImage(this, m_NetworkHistory, 100, 60);
  m_NetworkStatisticsLayout = new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 0, 0, 0);     
  m_UpperFrame->AddFrame(m_NetworkStatistics, m_NetworkStatisticsLayout);


  // The tabs:
  m_ICLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);

  m_Tab = new TGTab(this, 300, 300);

  // Tab with the views
  CreateTabView(m_Tab->AddTab("Views"));

  CreateTabEvents(m_Tab->AddTab("Events - all"));

  CreateTabChipPedestal(m_Tab->AddTab("Chips - Pedestal"));
  CreateTabChipNoise(m_Tab->AddTab("Chips - Noise"));
  CreateTabChipCommonMode(m_Tab->AddTab("Chips - Common-Mode"));
  CreateTabChipTriggerRate(m_Tab->AddTab("Chips - Trigger-Rate"));

  m_TabLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);
  AddFrame(m_Tab, m_TabLayout);



  // The lower frame with the buttons:
  m_LowerFrame = new TGCompositeFrame(this, 400, 30, kHorizontalFrame | kSunkenFrame);
  m_LowerFrameLayout = new TGLayoutHints(kLHintsCenterX| kLHintsExpandX, 0, 0, 0, 0);
  AddFrame(m_LowerFrame, m_LowerFrameLayout);

  m_LowerButtonLayout = 
    new TGLayoutHints(kLHintsCenterY | kLHintsTop | kLHintsLeft | kLHintsExpandX, 10, 10, 2, 2);

  m_StoreButton = new TGTextButton(m_LowerFrame, "Store as ...", 5);
  m_StoreButton->Resize(100, 20);
  m_StoreButton->SetWidth(100);
  m_StoreButton->ChangeOptions(kRaisedFrame | kDoubleBorder | kFixedSize); 
  m_StoreButton->Associate(this);
  m_LowerFrame->AddFrame(m_StoreButton, m_LowerButtonLayout);

  m_ConnectButton = new TGTextButton(m_LowerFrame, "Connect", 3); 
  m_ConnectButton->Associate(this);
  m_LowerFrame->AddFrame(m_ConnectButton, m_LowerButtonLayout);

  m_DisconnectButton = new TGTextButton(m_LowerFrame, "Disconnect", 4); 
  m_DisconnectButton->Associate(this);
  m_LowerFrame->AddFrame(m_DisconnectButton, m_LowerButtonLayout);

  m_HouseKeepingButton = new TGTextButton(m_LowerFrame, "Request HKD", 7); 
  m_HouseKeepingButton->Associate(this);
  m_LowerFrame->AddFrame(m_HouseKeepingButton, m_LowerButtonLayout);

  m_ResetButton = new TGTextButton(m_LowerFrame, "Reset", 6); 
  m_ResetButton->Associate(this);
  m_LowerFrame->AddFrame(m_ResetButton, m_LowerButtonLayout);

  m_QuitButton = new TGTextButton(m_LowerFrame, "Quit", 2); 
  m_QuitButton->Associate(this);
  m_LowerFrame->AddFrame(m_QuitButton, m_LowerButtonLayout);



  // The status bar
  m_StatusBar = new TGStatusBar(this, 100, 10);
  //m_LogStream.SetConnection(m_StatusBar);
  //m_LogStream<<"--- Not connected"<<endl;
  m_StatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 0, 0, 0, 0);

  AddFrame(m_StatusBar, m_StatusBarLayout);

  // PositionWindow(GetDefaultWidth(), GetDefaultHeight());
  // PositionWindow(640, 480, kFALSE);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabView(TGCompositeFrame *Tab)
{
  //

  // Create the images:

  if (m_GUIDataImaging->GetCoordinateSystem() == 0) {
    m_BPImage = new MImage2DUpdate("Summed PSFs", 
                                   0, 
                                   m_GUIDataImaging->GetBinsPhi()*m_GUIDataImaging->GetBinsTheta(), 
                                   "Longitude [deg]", 
                                   m_GUIDataImaging->GetPhiMin(), 
                                   m_GUIDataImaging->GetPhiMax(), 
                                   m_GUIDataImaging->GetBinsPhi(),
                                   "Latitude [deg]", 
                                   m_GUIDataImaging->GetThetaMin(), 
                                   m_GUIDataImaging->GetThetaMax(), 
                                   m_GUIDataImaging->GetBinsTheta());
  } else {
    m_BPImage = new MImage2DUpdate("Summed PSFs", 
                                   0, 
                                   m_GUIDataImaging->GetBinsX()*m_GUIDataImaging->GetBinsY(), 
                                   "X [cm]", 
                                   m_GUIDataImaging->GetXMin(), 
                                   m_GUIDataImaging->GetXMax(), 
                                   m_GUIDataImaging->GetBinsX(),
                                   "Y [cm]", 
                                   m_GUIDataImaging->GetYMin(), 
                                   m_GUIDataImaging->GetYMax(), 
                                   m_GUIDataImaging->GetBinsY());
  }

  // Add them to a container:
  m_ViewIC = new MGUIEImageContainer(Tab, "View", kTRUE);
  m_ViewIC->AddImage(m_BPImage);
  m_ViewIC->Create();

  //m_EImage = new MGUIEImage(Tab, m_FullSpectrum);

  Tab->AddFrame(m_ViewIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabEvents(TGCompositeFrame *Tab)
{
  //

  // Create the images:
  m_FullSpectrum = new MImageUpdate("Energy spectra", 
                                    0, 
                                    20, 
                                    "Energy [keV]", 
                                    m_GUIDataImaging->GetEnergyRangeMin(), 
                                    m_GUIDataImaging->GetEnergyRangeMax(), 
                                    20,
                                    MImageUpdate::c_Normalize, 1000, 10000);

  m_ARM = new MImageUpdate("ARM", 
                           0, 
                           51, 
                           "phi - phi_{m} [deg]", 
                           -m_GUIDataImaging->GetARMGammaRadiusCartesean(), 
                           m_GUIDataImaging->GetARMGammaRadiusCartesean(), 
                           21,
                           MImageUpdate::c_Normalize, 1000, 10000);

  // Add them to a container:
  m_EventIC = new MGUIEImageContainer(Tab, "Events", kTRUE);
  m_EventIC->AddImage(m_FullSpectrum);
  m_EventIC->AddImage(m_ARM);
  m_EventIC->Create();

  //m_EImage = new MGUIEImage(Tab, m_FullSpectrum);

  Tab->AddFrame(m_EventIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabChipPedestal(TGCompositeFrame *Tab)
{
  //

  // Add them to a container:
  m_ChipPedestalIC = new MGUIEImageContainer(Tab, "Pedestal", kTRUE);
  //m_ChipPedestalIC->SetRowsAndColumns(-1, 6);
  m_ChipPedestalIC->SetRowsAndColumns(6, -1, kFALSE);

  Tab->AddFrame(m_ChipPedestalIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabChipNoise(TGCompositeFrame *Tab)
{
  //

  // Add them to a container:
  m_ChipNoiseIC = new MGUIEImageContainer(Tab, "Noise", kTRUE);
  m_ChipNoiseIC->SetRowsAndColumns(6, -1, kFALSE);

  Tab->AddFrame(m_ChipNoiseIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabChipCommonMode(TGCompositeFrame *Tab)
{
  //

  // Add them to a container:
  m_ChipCommonModeIC = new MGUIEImageContainer(Tab, "Common Mode", kTRUE);
  m_ChipCommonModeIC->SetRowsAndColumns(6, -1, kFALSE);

  Tab->AddFrame(m_ChipCommonModeIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::CreateTabChipTriggerRate(TGCompositeFrame *Tab)
{
  //

  // Add them to a container:
  m_ChipTriggerRateIC = new MGUIEImageContainer(Tab, "Trigger Rates", kTRUE);
  m_ChipTriggerRateIC->SetRowsAndColumns(6, -1, kFALSE);

  Tab->AddFrame(m_ChipTriggerRateIC, m_ICLayout);
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MGUIControlCenter::ProcessMessage(Long_t Message, Long_t Parameter1, 
                                 Long_t Parameter2)
{
  // Process the messages for this application

  //cout<<"Message: "<<Message<<", "<<Parameter1<<", "<<Parameter2<<endl;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case 1: // OK
         break;
        
      case 2: // Quit
        CloseWindow();
        break;
        
      case 3: // Connect
        StartControlLoop();
        break;
        
      case 4: // Disconnect
        Disconnect();
        break;
        
      case 5: // Store
        OpenStorage();
        break;
        
      case 6: // Reset
        m_LogStream<<"Reseting all histograms"<<endl;
        m_ViewIC->Reset();                
        m_EventIC->Reset();               
        m_ChipPedestalIC->Reset();         
        m_ChipNoiseIC->Reset();           
        m_ChipCommonModeIC->Reset();      
        m_ChipTriggerRateIC->Reset();  
        m_NetworkHistory->Reset();   
        m_LogStream<<"Reseting all histograms - finished"<<endl;
        break;

      case 7: // Request Housekeeping data
        cout<<"Requesting house keeping data not implemented!"<<endl;
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


void MGUIControlCenter::CloseWindow()
{
  // Don't close but unmap the window
 
  if (m_SafeMode == kTRUE) {
    Int_t a;
    new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Safe mode message", 
                 "This closes the REALTA window, \n"
                 "but does not delete any data and does not stop the analysis process.\n"
                 "To restore the window click in MIWorks-Realta on MEGA.",
                 kMBIconAsterisk, kMBCancel | kMBOk, &a);
   
    // If Ok has been pressed...
    if (a == 64) { 
      return;
    }
  } 

  UnmapWindow();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::DisplayWindow()
{
  MapWindow();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::Display()
{
  //


}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::SetPort(UInt_t Port)
{
  //

  m_Receiver->SetPort(Port);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::Connect()
{
  //

  m_Receiver->Connect();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::Disconnect()
{
  //

  if (m_SafeMode == kTRUE) {
    Int_t a;
    new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Safe mode message", 
                 "Do you really want to disconnect form remote host?\n"
                 "This stops the data-aqusition!",
                 kMBIconAsterisk, kMBCancel | kMBOk, &a);
   
    // If Cancel has been pressed...
    if (a == 64) { 
      return;
    }
  }

  m_Receiver->Disconnect();
  StopControlLoop();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::StartControlLoop()
{
  //

  Int_t i, j;
  Double_t Sum;
  MImage2D* Image;
  MImageAstro* IA;
  MHit *Hit;
  MChip *Chip;
  MHitEvent* HitEvent;
  TObject *Object;
  MPhysicalEvent* PEvent;
  MBPData* BEvent;
  MEventFacets* Event;
  MStr Title;

  m_StopControlLoop = kFALSE;
  UInt_t Update = 0;

  UpdatePhysicalEventAnalyzer();
  Double_t *ImageArray = new Double_t[m_NBins];

  TKey *Key;
  TIter *NextKey;

  if (m_ReadModeTcpIp == kTRUE) {
    // Now connect to the remote host:
    m_Receiver->Connect();

    // Open the storage-files
    if (m_EventStreamer == 0 || m_ObjectStreamer == 0) {
      if (OpenStorage() == kFALSE) {
        return;
      }
    }
  } 
  // Read from file:
  else {
    if (OpenFile() == kFALSE) return;
    NextKey = new TIter(m_ObjectStreamer->GetListOfKeys());
  }


  m_LastTime = gSystem->Now();
  m_LastObjects = m_NReceivedObjects;

  while (m_StopControlLoop == kFALSE) {
    gSystem->ProcessEvents();

    if (m_ReadModeTcpIp == kTRUE) {
      Object = m_Receiver->ReceiveOneObject();
    } else {
      if ((Key = (TKey *) NextKey->Next()) != 0) {
        Object = Key->ReadObj();
      }
    }

    // Update the overall object rate...
    // cout<<(long) (gSystem->Now() - m_LastTime)<<endl;
    if ((long) (gSystem->Now() - m_LastTime) > 30000) {
      m_NetworkHistory->Add(0, (Double_t) (m_NReceivedObjects - m_LastObjects) / 
                            long (gSystem->Now() - m_LastTime)*1000.0);
      //cout<<"Rate: "<<(Double_t) (m_NReceivedObjects - m_LastObjects) / 
      //  long (gSystem->Now() - m_LastTime)*1000.0<<endl;
      m_LastTime = gSystem->Now();
      m_LastObjects = m_NReceivedObjects;
    }

    if (Object == 0) {
      gSystem->Sleep(10);
      continue;
    } else {
      m_NReceivedObjects++;
    }

    // Identify the object:
    if (MString("MEventData").CompareTo(Object->ClassName()) == 0) {

      //delete (MEventData *) Object;
      //continue;

      m_NReceivedEvents++;
      //m_LogStream<<"Analyzing event "<<m_NReceivedEvents<<endl;
      cout<<"Analyzing event "<<m_NReceivedEvents<<endl;

      // Create a HitEvent-Object:
      HitEvent = new MHitEvent();
      for (i = 0; i < ((MEventData *) Object)->GetNHits(); i++) {
        Hit = ((MEventData *) Object)->GetHitAt(i);
        HitEvent->AddHit(new MHitData(Hit->GetDetector(), Hit->GetPosition(), Hit->GetEnergy()));
      }
      
      if (m_ReadModeTcpIp == kTRUE) {
        // Store in *.evta format
        ((MEventData *) Object)->StoreEvent(m_EventStreamer);

        // Store in *.root format
        Title<<"EV "<<m_NReceivedEvents;
        ((MEventData *) Object)->Write(Title.str(), TObject::kOverwrite);
      }        

      delete (MEventData *) Object;

      // Store the Event...
      Event = new MEventFacets();
      Event->SetHitEvent(HitEvent);
      //m_Events->Add(Event);
      


      // Do the event reconstruction...
      if (m_HEAnalyzer->Analyze(HitEvent) == kTRUE) {

        Event->SetPhysicalEvent(m_HEAnalyzer->GetPhysicalEvent());
        
        if (Event->GetPhysicalEvent() != 0) { 
          m_FullSpectrum->Add(Event->GetPhysicalEvent()->GetEnergy(), 1);
          if (Event->GetPhysicalEvent()->GetEventType() == MPhysicalEvent::c_Compton) {
            m_ARM->Add(((MComptonEvent *) (Event->GetPhysicalEvent()))->
                       GetARMGamma(MVector(m_GUIDataImaging->GetARMGammaX(),
                                            m_GUIDataImaging->GetARMGammaY(),
                                            m_GUIDataImaging->GetARMGammaZ()))*grad, 1);
          }
        }


        // Now and only now we can do the imaging...
        UpdatePhysicalEventAnalyzer();
        if (m_PEAnalyzer->Analyze(Event->GetPhysicalEvent()) == kTRUE) {
          Event->SetBackprojectionEvent(m_PEAnalyzer->GetBackprojectionEvent());
          
          if (Event->GetBackprojectionEvent() != 0) { 
            m_BPImage->Add(Event->GetBackprojectionEvent()->GetBPData());
          }

        }
      }     

      delete Event;
      //continue;
     
    } else if (MString("MHouseData").CompareTo(Object->ClassName()) == 0) {

      //delete (MHouseData *) Object;
      //continue;

      m_NReceivedHouseKeeping++;
      m_LogStream<<"Analyzing housekeeping data packet #"<<m_NReceivedHouseKeeping<<endl;

      // Store the objects:
      if (m_ReadModeTcpIp == kTRUE) {
        Title<<"HK "<<m_NReceivedHouseKeeping;
        ((MHouseData *) Object)->Write(Title.str(), TObject::kOverwrite);
      }

      // First test if we have enough images:
      if (m_NChips < ((MHouseData *) Object)->GetNChips()) {
        // Create new images:
        for (i = m_NChips; i < ((MHouseData *) Object)->GetNChips(); i++) {
        
          Chip = ((MHouseData *) Object)->GetChipAt(i);
          m_ImagesChipPedestal->AddLast(new MImageUpdate(Chip->GetName(), 
                                                         0, 
                                                         127, 
                                                         "Channel", 
                                                         0, 
                                                         127, 
                                                         127,
                                                         MImageUpdate::c_Average, 
                                                         1, 
                                                         10,
                                                         MImageUpdate::c_RMS));
          m_ImagesChipNoise->AddLast(new MImageUpdate(Chip->GetName(), 
                                                      0, 
                                                      127, 
                                                      "Channel", 
                                                      0, 
                                                      127, 
                                                      127,
                                                      MImageUpdate::c_Average, 
                                                      1, 
                                                      10,
                                                      MImageUpdate::c_Mean));
          m_ImagesChipCommonMode->AddLast(new MImageUpdate(Chip->GetName(), 
                                                           0, 
                                                           127, 
                                                           "Channel", 
                                                           0, 
                                                           127, 
                                                           127,
                                                           MImageUpdate::c_Average, 
                                                           1, 
                                                           10,
                                                           MImageUpdate::c_RMS));
          m_ImagesChipTriggerRate->AddLast(new MImageUpdate(Chip->GetName(), 
                                                            0, 
                                                            100, 
                                                            "Time", 
                                                            0, 
                                                            100, 
                                                            100,
                                                            MImageUpdate::c_History, 
                                                            0, 
                                                            0,
                                                            MImageUpdate::c_Last));
        }
        m_NChips = ((MHouseData *) Object)->GetNChips();

        // ... and re-display the tab:
        for (i = 0; i < m_ImagesChipPedestal->GetLast()+1; i++) {
          m_ChipPedestalIC->AddImage(((MImage *) (m_ImagesChipPedestal->At(i))));
          m_ChipNoiseIC->AddImage(((MImage *) (m_ImagesChipNoise->At(i))));
          m_ChipCommonModeIC->AddImage(((MImage *) (m_ImagesChipCommonMode->At(i))));
          m_ChipTriggerRateIC->AddImage(((MImage *) (m_ImagesChipTriggerRate->At(i))));
        }
        m_ChipPedestalIC->Create();
        m_ChipNoiseIC->Create();
        m_ChipCommonModeIC->Create();
        m_ChipTriggerRateIC->Create();
        MapSubwindows();
        MapWindow();  
        
        Layout();
      }

      // Now update the chip images:
      for (i = 0; i < ((MHouseData *) Object)->GetNChips(); i++) {
				Int_t NChannels;
        TArrayF *TAF;
				Float_t *FArray;
        Double_t *DArray;

				NChannels = (((MHouseData *) Object)->GetChipAt(i))->GetNChannels();
        // Pedestal:
        FArray = (((MHouseData *) Object)->GetChipAt(i))->GetPedestal();
        DArray = new Double_t[NChannels];
        for (j = 0; j < NChannels; j++) {
          DArray[j] = FArray[j];
        }
        ((MImageUpdate *) (m_ImagesChipPedestal->At(i)))->Add(DArray);
        delete [] DArray;

        // Noise:
        FArray = (((MHouseData *) Object)->GetChipAt(i))->GetNoise();
        DArray = new Double_t[NChannels];
        for (j = 0; j < NChannels; j++) {
          DArray[j] = FArray[j];
        }
        ((MImageUpdate *) (m_ImagesChipNoise->At(i)))->Add(DArray);
        delete [] DArray;
        
        TH1F *H1F;
        // CommonMode:
        H1F = (((MHouseData *) Object)->GetChipAt(i))->GetCommonModeHist();
//         DArray = new Double_t[H1F->GetSize()];
//         for (j = 0; j < TAF->GetSize(); j++) {
//           DArray[j] = H1F->At(j);
//         }
        DArray = new Double_t[128];
        for (j = 0; j < 128; j++) {
          DArray[j] = 1;
        }
        ((MImageUpdate *) (m_ImagesChipCommonMode->At(i)))->Add(DArray);
        delete [] DArray;
        
        // Trigger Rates:
        ((MImageUpdate *) (m_ImagesChipTriggerRate->At(i)))->
          Add(0, (((MHouseData *) Object)->GetChipAt(i))->GetTriggerRate());
      }

      delete (MHouseData *) Object;
      continue;
    }


    Update++;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::UpdatePhysicalEventAnalyzer()
{
  //

  if (m_PEAnalyzer == 0 && m_GUIDataImaging->IsCoordinateSystemModified(kTRUE) == kTRUE) {
    m_PEAnalyzer = new MPhysicalEventAnalyzer(m_GUIDataImaging->GetCoordinateSystem());
  }

  // In case the event selection is modified:
  if (m_GUIDataImaging->IsEventSelectionModified(kTRUE) == kTRUE) {
    m_PEAnalyzer->SetSelectionTotalEnergy(m_GUIDataImaging->GetEnergyRangeMin(), 
                                          m_GUIDataImaging->GetEnergyRangeMax());
    m_PEAnalyzer->SetSelectionEnergyGamma(m_GUIDataImaging->GetEnergyRangeGammaMin(), 
                                      m_GUIDataImaging->GetEnergyRangeGammaMax());
    m_PEAnalyzer->SetSelectionEnergyElectron(m_GUIDataImaging->GetEnergyRangeElectronMin(), 
                                         m_GUIDataImaging->GetEnergyRangeElectronMax());
    
    m_PEAnalyzer->SetSelectionTime(m_GUIDataImaging->GetTimeRangeMin(), 
                                         m_GUIDataImaging->GetTimeRangeMax());

    m_PEAnalyzer->SetSelectionEventType(m_GUIDataImaging->GetEventTypeCompton(), 
                                         m_GUIDataImaging->GetEventTypeDoubleCompton(), 
                                         m_GUIDataImaging->GetEventTypePair(),
                                         m_GUIDataImaging->GetEventTypeComptonNotTracked(), 
                                         m_GUIDataImaging->GetEventTypeComptonTracked());

    m_PEAnalyzer->SetSelectionComptonAngle(m_GUIDataImaging->GetComptonAngleRangeMin(), 
                                       m_GUIDataImaging->GetComptonAngleRangeMax()); 
  }

  // In case the backprojection has been modified:
  if (m_GUIDataImaging->IsBackProjectionModified(kTRUE) == kTRUE) {
    if (m_GUIDataImaging->GetCoordinateSystem() == 0) {
      m_PEAnalyzer->SetViewport(m_GUIDataImaging->GetPhiMin(), 
                             m_GUIDataImaging->GetPhiMax(), 
                             m_GUIDataImaging->GetBinsPhi(),
                             -m_GUIDataImaging->GetThetaMin()+90,        // Some rotation...
                             -m_GUIDataImaging->GetThetaMax()+90, 
                             m_GUIDataImaging->GetBinsTheta(),
                             0, 0, 1, m_GUIDataImaging->GetAxisTheta(), 
                             m_GUIDataImaging->GetAxisPhi(), 0);
      m_NBins = m_GUIDataImaging->GetBinsPhi()*m_GUIDataImaging->GetBinsTheta();
    } else {
      m_PEAnalyzer->SetViewport(m_GUIDataImaging->GetXMin(), 
                             m_GUIDataImaging->GetXMax(), 
                             m_GUIDataImaging->GetBinsX(),
                             m_GUIDataImaging->GetYMin(), 
                             m_GUIDataImaging->GetYMax(), 
                             m_GUIDataImaging->GetBinsY(),
                             m_GUIDataImaging->GetZMin(), 
                             m_GUIDataImaging->GetZMax(), 
                             m_GUIDataImaging->GetBinsZ());
      m_NBins = m_GUIDataImaging->GetBinsX()*m_GUIDataImaging->GetBinsY()*m_GUIDataImaging->GetBinsZ();
    }

    m_PEAnalyzer->SetGaussianFits(m_GUIDataImaging->GetFitParameterComptonLongSphere(), 
                               m_GUIDataImaging->GetFitParameterComptonTransSphere(),
                               m_GUIDataImaging->GetFitParameterPair());
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIControlCenter::StopControlLoop()
{
  //

  m_StopControlLoop = kTRUE;

}


////////////////////////////////////////////////////////////////////////////////


Bool_t MGUIControlCenter::OpenFile()
{
  // 

  MString FileName;

  Char_t **Types = new Char_t*[6];
  Types[0] = "ROOT files";
  Types[1] = "*.root";
  Types[2] = "All files";
  Types[3] = "*";
  Types[4] = 0;
  Types[5] = 0;
  
  TGFileInfo Info;
  Info.fFileTypes = (const Char_t **) Types;
  //Info.fFileTypes = (Char_t **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);

  // Get the filename ...
  if ((Char_t *) Info.fFilename != 0) {
    FileName = MString((char *) Info.fFilename);
    if (FileName.CompareTo("") == 0) {
      return kFALSE;
    }
  } 
  // ... or return when cancel has been pressed
  else {
    return kFALSE;
  }

  // Check if the file really exists:
  if (gSystem->AccessPathName(FileName) == kTRUE) {
    return kFALSE;
  }

  m_ObjectStreamer = new TFile(FileName, "READ", "Realta-Objects", 3);

  return kTRUE;
}


////////////////////////////////////////////////////////////////////////////////


Bool_t MGUIControlCenter::OpenStorage()
{
  // Open the files where the data is stored...

  MString FileName;

  Char_t **Types = new Char_t*[6];
  Types[0] = "All files";
  Types[1] = "*";
  Types[2] = 0;
  Types[3] = 0;
  
  TGFileInfo Info;
  Info.fFileTypes = (const Char_t **) Types;
  //Info.fFileTypes = (Char_t **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);

  // Get the filename ...
  if ((Char_t *) Info.fFilename != 0) {
    FileName = MString((char *) Info.fFilename);
    if (FileName.CompareTo("") == 0) {
      return kFALSE;
    }
  } 
  // ... or return when cancel has been pressed
  else {
    return kFALSE;
  }

  // Check if the files already exist:
  if (gSystem->AccessPathName(FileName + ".root") == kFALSE || 
      gSystem->AccessPathName(FileName + ".evta") == kFALSE) {
    // Ask the user if he wants to append data to the existing files:

    if (m_SafeMode == kTRUE) {
      Int_t a;
      new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), "Safe mode message", 
                   "The file already exists and you are not allowed to overwrite it.\n"
                   "Do you want to append data to the file?",
                   kMBIconAsterisk, kMBYes | kMBNo, &a);
      
      // If No has been pressed return
      if (a == 2) { 
        return kFALSE;
      }
    }
  }

  
  // Take care of the standard *.evta-file:
  if (m_EventStreamer != 0) {
    fclose(m_EventStreamer);
  }

  if ((m_EventStreamer = fopen(FileName + ".evta", "a")) == 0) {
    Error("void MGUIControlCenter::OpenStorage()",
          "Critical error: \a\n"
          "Unable to open file");
    return kFALSE;
  }


  // Secondly store all objects in ROOT format:
  if (m_ObjectStreamer != 0) {
    m_ObjectStreamer->Close();
  }

  // If the file exists ... AccessPathName == kFALSE if exists !!
  if (gSystem->AccessPathName(FileName + ".root") == kFALSE) {
    // Check the last entries for the event and house keeping data number

    m_LogStream<<"File "<<FileName<<" already exists - APPENDING objects!"<<endl;
    m_ObjectStreamer = new TFile(FileName + ".root", "UPDATE", "Realta-Objects", 3);
    
    TIter NextKey(m_ObjectStreamer->GetListOfKeys(), kIterBackward);
    TKey *Key;
    MString Name;
    Bool_t EventFound = kFALSE;
    Bool_t HouseKeepingFound = kFALSE;
    while ((Key = (TKey*) NextKey()) != 0) {
      Name = Key->GetName();
      if (EventFound == kFALSE) {
        if (Name.Contains("EV") != 0) {
          sscanf(Key->GetName(), "EV %d", &m_NReceivedEvents);
          EventFound = kTRUE;
        }
      }
      if (HouseKeepingFound == kFALSE) {
        if (Name.Contains("HK") != 0) {
          sscanf(Key->GetName(), "HK %d", &m_NReceivedHouseKeeping);
          HouseKeepingFound = kTRUE;
        }
      }
      if (HouseKeepingFound == kTRUE && EventFound == kTRUE) {
        continue;
      }
    }
  } else {
    m_LogStream<<"Creating new file "<<FileName<<endl;
    m_ObjectStreamer = new TFile(FileName + ".root", "RECREATE", "Realta-Objects", 3);

    m_NReceivedEvents = 0;
    m_NReceivedHouseKeeping = 0;
  }
 
  return kTRUE;
}


// MGUIControlCenter: the end...
////////////////////////////////////////////////////////////////////////////////
