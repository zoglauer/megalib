/*
 * MGUIExpoSupervisor.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoSupervisor__
#define __MGUIExpoSupervisor__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TString.h>
#include <TGClient.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1.h>
#include <TH2.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MGUIERBList.h"

// NuSTAR libs
#include "MGUIExpo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoSupervisor : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoSupervisor(MModule* Module = nullptr);
  //! Default destructor
  virtual ~MGUIExpoSupervisor();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set numbe rof modules
  void SetNModules(int NModules);

  //! Set the name of a module
  void SetModuleName(unsigned int ModuleID, MString Name);

  //! Set the number of processed events
  void SetProcessedEvents(unsigned int ModuleID, long ProcessedEvents);

  //! Set the processing time
  void SetProcessingTime(unsigned int ModuleID, double ProcessingTime);

  //! Set the running instances of the given module
  void SetInstances(unsigned int ModuleID, unsigned int Instances);

  //! Set the time the youngest event started processing
  void SetLastProcessingTime(MTime& Time);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! The time the youngest event started processing
  MTime m_LastProcessingTime;
  //! The name of the displayed module
  vector<MString> m_ModuleNames;
  //! The processed counts in the modules
  vector<long> m_ModuleProcessedEvents;
  //! The processed counts in the modules
  vector<double> m_ModuleProcessingTime;
  //! The running instances of the module 
  vector<unsigned int> m_ModuleInstances;

  //! The labels containing the processed event info
  vector<TGLabel*> m_LabelProcessedEvents;
  //! The labels containing the processed event info
  vector<TGLabel*> m_LabelProcessingTime;
  //! The labels containing the processed event info
  vector<TGLabel*> m_LabelInstances;
  //! Label containing the last processing time
  TGLabel* m_LastProcessingTimeLabel;
  
#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoSupervisor, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
