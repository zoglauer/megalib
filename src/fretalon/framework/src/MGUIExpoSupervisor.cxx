/*
 * MGUIExpoSupervisor.cxx
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
#include "MGUIExpoSupervisor.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExpoSupervisor)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoSupervisor::MGUIExpoSupervisor(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Supervisor";

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoSupervisor::~MGUIExpoSupervisor()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoSupervisor::Reset()
{
  //! Reset the data in the UI

  m_ModuleNames.clear();
  m_ModuleProcessedEvents.clear();
  m_ModuleProcessingTime.clear();
}

////////////////////////////////////////////////////////////////////////////////



//! Set the energy histogram parameters 
void MGUIExpoSupervisor::SetNModules(int NModules)
{
  m_ModuleNames.resize(NModules);
  m_ModuleProcessedEvents.resize(NModules);
  m_ModuleProcessingTime.resize(NModules);
}


////////////////////////////////////////////////////////////////////////////////


//! Set the energy histogram parameters 
void MGUIExpoSupervisor::SetModuleName(unsigned int ModuleID, MString Name)
{
  if (m_ModuleNames.size() <= ModuleID) m_ModuleNames.resize(ModuleID);
  m_ModuleNames[ModuleID] = Name;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the energy histogram parameters 
void MGUIExpoSupervisor::SetProcessedEvents(unsigned int ModuleID, long ProcessedCounts)
{
  if (m_ModuleProcessedEvents.size() <= ModuleID) m_ModuleProcessedEvents.resize(ModuleID);
  m_ModuleProcessedEvents[ModuleID] = ProcessedCounts; 
}


////////////////////////////////////////////////////////////////////////////////


//! Set the processing time
void MGUIExpoSupervisor::SetProcessingTime(unsigned int ModuleID, double ProcessingTime)
{
  if (m_ModuleProcessingTime.size() <= ModuleID) m_ModuleProcessingTime.resize(ModuleID);
  m_ModuleProcessingTime[ModuleID] = ProcessingTime; 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoSupervisor::Export(const MString& FileName)
{
  // Nothing to save...
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoSupervisor::Create()
{
  // Add the GUI options here
  
  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  TGLayoutHints* ModuleFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX, 20, 20, 20, 20);

  TGHorizontalFrame* ModuleFrame = new TGHorizontalFrame(this);
  AddFrame(ModuleFrame, ModuleFrameLayout);
  
  TGLayoutHints* ColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX, 10, 10, 2, 2);

  TGVerticalFrame* NameFrame = new TGVerticalFrame(ModuleFrame);
  ModuleFrame->AddFrame(NameFrame, ColumnLayout);

  TGLabel* TitleColumnName = new TGLabel(NameFrame, "Module name");
  TitleColumnName->SetTextFont(MGUIDefaults::GetInstance()->GetNormalBoldFont()->GetFontStruct());
  TGLayoutHints* TitleColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 10);
  NameFrame->AddFrame(TitleColumnName, TitleColumnLayout);

  
  TGVerticalFrame* EventsFrame = new TGVerticalFrame(ModuleFrame);
  ModuleFrame->AddFrame(EventsFrame, ColumnLayout);

  TGLabel* TitleColumnEvents = new TGLabel(EventsFrame, "Processed assemblies"); 
  TitleColumnEvents->SetTextFont(MGUIDefaults::GetInstance()->GetNormalBoldFont()->GetFontStruct());
  EventsFrame->AddFrame(TitleColumnEvents, TitleColumnLayout);
  
  TGVerticalFrame* TimeFrame = new TGVerticalFrame(ModuleFrame);
  ModuleFrame->AddFrame(TimeFrame, ColumnLayout);

  TGLabel* TitleColumnTime = new TGLabel(TimeFrame, "Processing time"); 
  TitleColumnTime->SetTextFont(MGUIDefaults::GetInstance()->GetNormalBoldFont()->GetFontStruct());
  TimeFrame->AddFrame(TitleColumnTime, TitleColumnLayout);
  
  
  for (unsigned int i = 0; i < m_ModuleNames.size(); ++i) {
    // Name
    TGLabel* LabelName = new TGLabel(NameFrame, m_ModuleNames[i]); 
    TGLayoutHints* LabelNameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
    NameFrame->AddFrame(LabelName, LabelNameLayout);
    
    // Events
    ostringstream out;
    out<<m_ModuleProcessedEvents[i];
    
    TGLabel* LabelProcessedEvents = new TGLabel(EventsFrame, out.str().c_str()); 
    TGLayoutHints* LabelProcessedEventsLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
    EventsFrame->AddFrame(LabelProcessedEvents, LabelProcessedEventsLayout);
    m_LabelProcessedEvents.push_back(LabelProcessedEvents);
    
    // Time
    ostringstream t;
    t<<m_ModuleProcessingTime[i]<<" s";
    
    TGLabel* LabelProcessingTime = new TGLabel(TimeFrame, t.str().c_str()); 
    TGLayoutHints* LabelProcessingTimeLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
    TimeFrame->AddFrame(LabelProcessingTime, LabelProcessingTimeLayout);
    m_LabelProcessingTime.push_back(LabelProcessingTime);
  }

  
  m_IsCreated = true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoSupervisor::Update()
{
  //! Update the frame

  for (unsigned int i = 0; i < m_LabelProcessedEvents.size(); ++i) {
    ostringstream out;
    out<<m_ModuleProcessedEvents[i];
    m_LabelProcessedEvents[i]->SetText(out.str().c_str());
    
    ostringstream t;
    t<<m_ModuleProcessingTime[i]<<" s";
    m_LabelProcessingTime[i]->SetText(t.str().c_str());
  }
}


// MGUIExpoSupervisor: the end...
////////////////////////////////////////////////////////////////////////////////
