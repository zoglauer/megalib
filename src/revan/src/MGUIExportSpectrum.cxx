/*
 * MGUIExportSpectrum.cxx
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
// MGUIExportSpectrum
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIExportSpectrum.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MRawEventAnalyzer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExportSpectrum)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExportSpectrum::MGUIExportSpectrum(const TGWindow* Parent, 
                                       const TGWindow* Main, 
                                       MSettingsRevan* Data, 
                                       bool& OKPressed)
  : MGUIDialog(Parent, Main), m_OKPressed(OKPressed)
{
  // Construct an instance of MGUIExportSpectrum and bring it to the screen

  m_Data = Data;
  m_OKPressed = false;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIExportSpectrum::~MGUIExportSpectrum()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExportSpectrum::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Export the initial energy spectrum");  
  AddSubTitle("Options for exporting the initial energy spectrum"); 

  TGLayoutHints* StandardLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 20, 20, 10, 0);
  TGLayoutHints* InnerLeftLayout = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 0, 0);
  TGLayoutHints* InnerRightLayout = new TGLayoutHints(kLHintsTop, 29, 20, 0, 0);

  TGHorizontalFrame* Frame = new TGHorizontalFrame(this);
  AddFrame(Frame, StandardLayout);

  m_Bins = new MGUIEEntry(Frame, "Bins: ", false, m_Data->GetExportSpectrumBins(), true, 1);
  Frame->AddFrame(m_Bins, InnerLeftLayout);
  
  m_Log = new TGCheckButton(Frame, "Log");
  m_Log->SetState((m_Data->GetExportSpectrumLog() == true) ?  kButtonDown : kButtonUp);
  Frame->AddFrame(m_Log, InnerRightLayout);

  m_Range = new MGUIEMinMaxEntry(this, "Range [keV]: ", false, m_Data->GetExportSpectrumMin(), m_Data->GetExportSpectrumMax(), true, 0.0);
  AddFrame(m_Range, StandardLayout);
  
  m_File = new MGUIEFileSelector(this, "Name of the export file:", m_Data->GetExportSpectrumFileName());
  AddFrame(m_File, StandardLayout);

  AddButtons();
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  
  Layout();

  fClient->WaitFor(this);
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExportSpectrum::OnApply()
{
  // The Apply button has been pressed

  if (m_Log->GetState() == kButtonDown && m_Range->GetMinValue() == 0) {
    mgui<<"You have to use a minimum range larger than 0 for logarithmic binning"<<error;
    return false;
  }
  m_Data->SetExportSpectrumBins(m_Bins->GetAsInt());
  m_Data->SetExportSpectrumLog((m_Log->GetState() == kButtonDown) ? true : false);
  m_Data->SetExportSpectrumMin(m_Range->GetMinValue());
  m_Data->SetExportSpectrumMax(m_Range->GetMaxValue());
  m_Data->SetExportSpectrumFileName(m_File->GetFileName());

  m_OKPressed = true;

  return true;
}


// MGUIExportSpectrum: the end...
////////////////////////////////////////////////////////////////////////////////
