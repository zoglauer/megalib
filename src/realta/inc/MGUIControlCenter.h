/*
 * MGUIControlCenter.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIControlCenter__
#define __MGUIControlCenter__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGTab.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TTime.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsRealta.h"
#include "MGUIEImage.h"
#include "MGUIEImageContainer.h"
#include "MImage.h"
#include "MStreams.h"

// Forward declarations:
class TGStatusBar;
class TFile;
class TObjectReceiver;
class MEventFacetsList;
class MHitEventAnalyzer;
class MPhysicalEventAnalyzer;
class MSettingsMimrec;
class MImageUpdate;
class MImage2DUpdate;


////////////////////////////////////////////////////////////////////////////////


class MGUIControlCenter : public TGMainFrame
{
  // Public Interface:
 public:
  MGUIControlCenter(const TGWindow *p, const TGWindow *main, MSettingsRealta *Data = 0);
  ~MGUIControlCenter();

  void Display();
  void DisplayWindow();

  // protected methods:
 protected:
  void Create();
  void CreateTabView(TGCompositeFrame *Tab);
  void CreateTabEvents(TGCompositeFrame *Tab);
  void CreateTabChipPedestal(TGCompositeFrame *Tab);
  void CreateTabChipNoise(TGCompositeFrame *Tab);
  void CreateTabChipCommonMode(TGCompositeFrame *Tab);
  void CreateTabChipTriggerRate(TGCompositeFrame *Tab);


  Bool_t ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);
  void CloseWindow();

  void SetPort(UInt_t Port);
  void Connect();
  void Disconnect();
  void StartControlLoop();
  void StopControlLoop();
  void UpdatePhysicalEventAnalyzer();

  Bool_t OpenStorage();
  Bool_t OpenFile();

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:
  // GUI-elements
  TGWindow *m_Parent;                           // Frame this own is the daughter
  MSettingsRealta *m_GUIData;                    // GUI-data of this session
  MSettingsMimrec *m_GUIDataImaging;            // GUI-data of the imaging interface


  // Upper part of the REALTA-window: 
  TGCompositeFrame *m_UpperFrame;               // upper most frame, containing the title and the network activity
  TGLayoutHints *m_UpperFrameLayout;            // its layout

  TGPictureButton *m_PictureButton;             // the "Realta"-title
  TGLayoutHints *m_PictureButtonLayout;         // its layout

  MImageUpdate *m_NetworkHistory;               // Image containing the network history
  MGUIEImage *m_NetworkStatistics;              // its GUI element
  TGLayoutHints *m_NetworkStatisticsLayout;     // its layout


  // Center part of the REALTA-window:
  TGTab *m_Tab;                                 // the main tabulator frame
  TGLayoutHints *m_TabLayout;                   // its layout

  MGUIEImageContainer *m_ViewIC;                // the images of the view tab
  MGUIEImageContainer *m_EventViewIC;           // the images of the event view tab
  MGUIEImageContainer *m_EventIC;               // the images of the event tab
  MGUIEImageContainer *m_ChipPedestalIC;        // the images of the chips pedestal  
  MGUIEImageContainer *m_ChipNoiseIC;           // the images of the chips noise
  MGUIEImageContainer *m_ChipCommonModeIC;      // the images of the chips common mode
  MGUIEImageContainer *m_ChipTriggerRateIC;     // the images of the chips trigger rate
  TGLayoutHints *m_ICLayout;                    // their layout


  // Lower part of the REALTA-window:
  TGCompositeFrame *m_LowerFrame;               // the lower most frame, containing the buttons
  TGLayoutHints *m_LowerFrameLayout;            // its layout

  TGTextButton *m_QuitButton;                   // Quit-REALTA button
  TGTextButton *m_ConnectButton;                // Connect to the remote host (PC, Ballon, Satellite)
  TGTextButton *m_DisconnectButton;             // Disconnect from remote host (PC, Ballon, Satellite)
  TGTextButton *m_ResetButton;                  // Reset all images to default values
  TGTextButton *m_StoreButton;                  // Store the data to this file
  TGTextButton *m_HouseKeepingButton;           // Request house keeping data
  TGLayoutHints *m_LowerButtonLayout;           // The layout of all buttons

  TGStatusBar *m_StatusBar;                     // the status-bar ...
  TGLayoutHints *m_StatusBarLayout;             // their layout and ...
  MLog m_LogStream;                       // the streamer containing the statusbars current text


  // The connection and analysis elements:
  TObjectReceiver* m_Receiver;                  // The connection class (Tcp/Ip via a socket)

  MHitEventAnalyzer* m_HEAnalyzer;              // Analyze the hits (Connection to "Revan")
  MPhysicalEventAnalyzer* m_PEAnalyzer;         // Image the events (Connection to "Mimrec")

  Bool_t m_StopControlLoop;

  MEventFacetsList* m_Events;



  UInt_t m_StoredEvents;
  UInt_t m_UpdateFrequency;

  UInt_t m_NBins;

  MImage2DUpdate *m_BPImage;
  MImageUpdate *m_FullSpectrum;
  MImageUpdate *m_ARM;

  TObjArray *m_ImagesChipPedestal;
  TObjArray *m_ImagesChipNoise;
  TObjArray *m_ImagesChipCommonMode;
  TObjArray *m_ImagesChipTriggerRate;
  Int_t m_NChips;

  // Storage:
  TFile *m_ObjectStreamer;                      // Store all object in this file (events and house keeping)
  FILE *m_EventStreamer;                        // Store all events in *.evta-format

  Int_t m_NReceivedObjects;                     // Number of all received objects
  Int_t m_NReceivedEvents;                      // Number of all received events
  Int_t m_NReceivedHouseKeeping;                // Number of all recieved house keeping packages 

  // A Timer...
  TTime m_LastTime;
  Int_t m_LastObjects;

  Bool_t m_SafeMode;
  Bool_t m_ReadModeTcpIp;


#ifdef ___CLING___
 public:
  ClassDef(MGUIControlCenter, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
