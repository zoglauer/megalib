/*
 * MGUIRealtaMain.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIRealtaMain__
#define __MGUIRealtaMain__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGTab.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGMenu.h"
#include "TGStatusBar.h"
#include "TTime.h"
#include "TRootEmbeddedCanvas.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsRealta.h"
#include "MRealTimeAnalyzer.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIRealtaMain : public TGMainFrame
{
  // Public Interface:
 public:
  MGUIRealtaMain();
  virtual ~MGUIRealtaMain();

  void Display();
  void DisplayWindow();

  //! Run the main control loop
  void DoControlLoop();
  
  //! Perform a graceful shutdown, of all threads and the GUI
  virtual void CloseWindow();
  
  
  // protected methods:
 protected:
  void Create();

  Bool_t ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);

  //! Load a configuration file 
  void OnLoad();
  //! Save a configuration file
  void OnSave();
  
  //! Quit realta
  void OnExit();
  //! Show the about dialog
  void OnAbout();
  
  //! Open the network GUI
  void OnNetwork();
  //! Open the network GUI
  void OnAccumulation();
  //! Open the geometry GUI
  void OnGeometry();
  
  //! Connect to the other machine
  void OnConnect();
  //! Disconnect from the other machine
  void OnDisconnect();
  //! Reset the analysis
  void OnReset();
  

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:
  //! The realta settings
  MSettingsRealta* m_Settings;

  //! A local copy of the geometry
  MDGeometryQuest* m_Geometry;
  
  //! The connection statsu display
  TGLabel* m_ConnectionStatus;
  
  //! The canvas for showing the count rates
  TRootEmbeddedCanvas* m_CountRateCanvas;
  //! The canvas for showing the count rates
  TRootEmbeddedCanvas* m_SpectrumCanvas;
  //! The canvas for showing the count rates
  TRootEmbeddedCanvas* m_ImageCanvas;

  //! The CPU usage of the transmission thread
  TGLabel* m_TransmissionThreadCpuUsage;
  //! The CPU usage of the coincidence thread
  TGLabel* m_CoincidenceThreadCpuUsage;
  //! The CPU usage of the reconstruction thread
  TGLabel* m_ReconstructionThreadCpuUsage;
  //! The CPU usage of the imaging thread
  TGLabel* m_ImagingThreadCpuUsage;
  //! The CPU usage of the histogramming thread
  TGLabel* m_HistogrammingThreadCpuUsage;
  //! The CPU usage of the identification thread
  TGLabel* m_IdentificationThreadCpuUsage;

  //! The ID of the last handled event of the transmission thread
  TGLabel* m_TransmissionThreadLastEventID;
  //! The ID of the last handled event of the coincidence thread
  TGLabel* m_CoincidenceThreadLastEventID;
  //! The ID of the last handled event of the reconstruction thread
  TGLabel* m_ReconstructionThreadLastEventID;
  //! The ID of the last handled event of the imaging thread
  TGLabel* m_ImagingThreadLastEventID;
  //! The ID of the last handled event of the histogramming thread
  TGLabel* m_HistogrammingThreadLastEventID;
  //! The ID of the last handled event of the identification thread
  TGLabel* m_IdentificationThreadLastEventID;
  
  
  //! The analysis thread
  MRealTimeAnalyzer* m_Analyzer;

  Bool_t m_StopControlLoop;


  enum ButtonIDs { c_Load = 1234, 
                   c_Save, 
                   c_Exit,
                   c_Network, 
                   c_Accumulation, 
                   c_Geometry, 
                   c_EventReconstruction, 
                   c_Coincidence, 
                   c_Clustering, 
                   c_Tracking, 
                   c_Sequencing, 
                   c_EventReconstructionSelection, 
                   c_ImagingSelection, 
                   c_CoordinateSystem, 
                   c_Zoom, 
                   c_ImageOptions, 
                   c_Memory, 
                   c_LikelihoodAlgorithm, 
                   c_Response, 
                   c_FitParameter,
                   c_Spectralyze,
                   c_About,
                   c_Connect,
                   c_Disconnect,
                   c_Reset };

  
  
#ifdef ___CINT___
 public:
  ClassDef(MGUIRealtaMain, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
