/*
 * MRealTimeAnalyzer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRealTimeAnalyzer__
#define __MRealTimeAnalyzer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <list>
#include <vector>
using namespace std;

// ROOT libs:
#include "TH1.h"
#include "TThread.h"
#include "TMutex.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsRealta.h"
#include "MRealTimeEvent.h"
#include "MTransceiverTcpIp.h"
#include "MImageSpheric.h"
#include "MImagerExternallyManaged.h"
#include "MQualifiedIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


void* StartTransmissionThread(void* Analysis);
void* StartCoincidenceThread(void* Analysis);
void* StartReconstructionThread(void* Analysis);
void* StartImagingThread(void* Analysis);
void* StartHistogrammingThread(void* Analysis);
void* StartIdentificationThread(void* Analysis);


////////////////////////////////////////////////////////////////////////////////


class MRealTimeAnalyzer
{
  // public interface:
 public:
  //! Default constructor for the real-time analyzer
  MRealTimeAnalyzer();
  //! Default desstructor for the real-time analyzer
  virtual ~MRealTimeAnalyzer();

  //! Set all the user definable settings
  void SetSettings(MSettingsRealta* Settings);
  //! Set the accumulation time
  void SetAccumulationTime(double AccumulationTime);
  
  //! Start all the analysis
  void StartAnalysis();
  //! Stop all the analysis
  void StopAnalysis();

  //! Connect to the other machine
  bool Connect();
  //! Disconnect from the other machine
  bool Disconnect();
  //! Reset the analysis
  bool Reset();

  
  //! The infinite reading loop
  void OneTransmissionLoop();
  //! The infinite coincidence loop
  void OneCoincidenceLoop();
  //! The infinite reconstruction loop
  void OneReconstructionLoop();
  //! The infinite backprojection loop
  void OneImagingLoop();
  //! The infinite backprojection loop
  void OneHistogrammingLoop();
  //! The infinite identification loop
  void OneIdentificationLoop();
  
  //! Get count rate historgram
  TH1D* GetCountRateHistogram() { return m_CountRate; }
  //! Get spectrum historgram
  TH1D* GetSpectrumHistogram() { return m_Spectrum; }
  //! Get image
  MImage* GetImage() { return m_Image; }
  //! Get a COPY of the isotope list
  vector<MQualifiedIsotope> GetIsotopes();
  //! Get the current list of the minima of the energy windows
  vector<double> GetMinimaOfSpectralWindows() { return m_SpectrumMin; }
  //! Get the current list of the minima of the energy windows
  vector<double> GetMaximaOfSpectralWindows() { return m_SpectrumMax; }
  
  //! Return the connection
  bool IsConnected() { return m_IsConnected; }
  
  //! Get the transmission thread CPU usage...
  double GetTransmissionThreadCpuUsage() { return m_TransmissionThreadCpuUsage; }
  //! Get the coincidence thread CPU usage...
  double GetCoincidenceThreadCpuUsage() { return m_CoincidenceThreadCpuUsage; }
  //! Get the reconstruction thread CPU usage...
  double GetReconstructionThreadCpuUsage() { return m_ReconstructionThreadCpuUsage; }
  //! Get the imaging thread CPU usage...
  double GetImagingThreadCpuUsage() { return m_ImagingThreadCpuUsage; }
  //! Get the imaging thread CPU usage...
  double GetHistogrammingThreadCpuUsage() { return m_HistogrammingThreadCpuUsage; }
  //! Get the identification thread CPU usage...
  double GetIdentificationThreadCpuUsage() { return m_IdentificationThreadCpuUsage; }
  
  //! Get the ID of the last event handled in the transmission thread...
  unsigned int GetTransmissionThreadLastEventID() { return m_TransmissionThreadLastEventID; }
  //! Get the ID of the last event handled in the coincidence thread...
  unsigned int GetCoincidenceThreadLastEventID() { return m_CoincidenceThreadLastEventID; }
  //! Get the ID of the last event handled in the reconstruction thread...
  unsigned int GetReconstructionThreadLastEventID() { return m_ReconstructionThreadLastEventID; }
  //! Get the ID of the last event handled in the imaging thread...
  unsigned int GetImagingThreadLastEventID() { return m_ImagingThreadLastEventID; }
  //! Get the ID of the last event handled in the histogramming thread...
  unsigned int GetHistogrammingThreadLastEventID() { return m_HistogrammingThreadLastEventID; }
  //! Get the ID of the last event handled in the identification thread...
  unsigned int GetIdentificationThreadLastEventID() { return m_IdentificationThreadLastEventID; }
  
  // protected methods:
 protected:
  //! initialize an object of MImagerExternallyManaged 
  MImagerExternallyManaged* InitializeImager();
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The user settings
  MSettingsRealta* m_Settings; 
   
  //! The thread where the transmission happens
  TThread* m_TransmissionThread;     
  //! True if the transmission thread is in its execution loop
  bool m_IsTransmissionThreadRunning;
  //! The CPU usage of the transmission thread
  double m_TransmissionThreadCpuUsage;
  //! The ID of the last transmitted event 
  unsigned int m_TransmissionThreadLastEventID;
  
  //! The thread where the coincidence finding happens
  TThread* m_CoincidenceThread;     
  //! True if the coincidence thread is in its execution loop
  bool m_IsCoincidenceThreadRunning;
  //! The CPU usage of the coincidence thread
  double m_CoincidenceThreadCpuUsage;
  //! The ID of the last coincident event 
  unsigned int m_CoincidenceThreadLastEventID;
  
  //! The thread where the event reconstruction happens
  TThread* m_ReconstructionThread;     
  //! True if the reconstruction thread is in its execution loop
  bool m_IsReconstructionThreadRunning;
  //! The CPU usage of the reconstruction thread
  double m_ReconstructionThreadCpuUsage;
  //! The ID of the last reconstructed event 
  unsigned int m_ReconstructionThreadLastEventID;
  
  //! The thread where the backprojection happens
  TThread* m_ImagingThread;
  //! True if the imaging thread is in its execution loop
  bool m_IsImagingThreadRunning;
  //! The CPU usage of the imaging thread
  double m_ImagingThreadCpuUsage;
  //! The ID of the last imaged event 
  unsigned int m_ImagingThreadLastEventID;

  //! The thread where the histogramming happens
  TThread* m_HistogrammingThread;
  //! True if the histogramming thread is in its execution loop
  bool m_IsHistogrammingThreadRunning;
  //! The CPU usage of the histogramming thread
  double m_HistogrammingThreadCpuUsage;
  //! The ID of the last histogrammed event
  unsigned int m_HistogrammingThreadLastEventID;

  //! The thread where the identification happens
  TThread* m_IdentificationThread;
  //! True if the identification thread is in its execution loop
  bool m_IsIdentificationThreadRunning;
  //! The CPU usage of the identification thread
  double m_IdentificationThreadCpuUsage;
  //! The ID of the last identification event
  unsigned int m_IdentificationThreadLastEventID;

  
  //! Unique Id for all the threads...
  static int m_ThreadId;
  //! True if the threads should be stopped
  bool m_StopThreads;
  //! True is the analysis is running
  bool m_IsAnalysisRunning;
  //! True if we are at the thread initialization stage
  bool m_IsInitializing;

  //! True is the analysis is running
  bool m_IsConnected;
  //! True if we should disconnect
  bool m_DoDisconnect;
    
  //! The geometry file name
  MString m_GeometryFileName;
  
  //! The accumulation time in seconds 
  double m_AccumulationTime;
  //! And a guarding mutex for the accumulation time
  TMutex m_AccumulationTimeMutex;
  
  //! The "list"
  list<MRealTimeEvent*> m_Events;
  //! Current number of events in the list
  unsigned int m_NEvents;
  //! Maximum number of events to keep in the list
  unsigned int m_MaxNEvents;

  //! The current count rate histogram
  TH1D* m_CountRate;
  //! The current spectrum histogram
  TH1D* m_Spectrum;
  //! The current image
  MImage* m_Image;
  
  //! The current list of the minima of the energy windows
  vector<double> m_SpectrumMin;
  //! The current list of the maxima of the energy windows
  vector<double> m_SpectrumMax;
  
  //! A list of isoptopes to identify:
  vector<MQualifiedIsotope> m_Isotopes;
  //! And a guarding mutex for the isotope retrieval
  TMutex m_IsotopeMutex;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MRealTimeAnalyzer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
