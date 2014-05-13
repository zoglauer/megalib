/*
 * MGUIMemory.cxx
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
// MGUIMemory
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIMemory.h"

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MSystem.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMemory)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIMemory::MGUIMemory(const TGWindow* Parent, const TGWindow* Main, 
                       MSettingsImaging* Data)
  : MGUIDialog(Parent, Main)
{
  // standard constructor

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIMemory::~MGUIMemory()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMemory::Create()
{
  // Create the main window

  int Width = m_FontScaler*650;

  // We start with a name and an icon...
  SetWindowName("Memory, accuracy, and thread management");  

  AddSubTitle("Memory, accuracy, and thread management"); 

  TGLayoutHints* StandardLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 20, 20, 10, 20);

  m_MaxRAM = new MGUIEEntryList(this, "List-mode image reconstruction is rather memory extensive. You can give here the amount of RAM, which may be used for the image slices of the response (the \"backprojections\"). If the selected amount of memory is consumed, the response calculation is stopped and the iteration process starts.");
  m_MaxRAM->Add("Maximum amout of RAM in MB for the \"backprojections\":", m_GUIData->GetRAM(), true, 64);
  m_MaxRAM->SetWrapLength(Width - m_FontScaler*40);
  m_MaxRAM->Create();
  AddFrame(m_MaxRAM, StandardLayout);


  m_Bytes = new MGUIERBList(this, "The dynamic range of the response slice corresponding to one event can be represented either in 1 byte (256 intensity steps) or in 4 bytes (float accuracy). While a 1-byte-depth results in slightly worse images and is restricted to a maximum of 65536 image bins, it allows to store roughly 4 times more events:");
  m_Bytes->Add("1 byte");
  m_Bytes->Add("4 byte");
  m_Bytes->SetSelected(m_GUIData->GetBytes());
  m_Bytes->SetWrapLength(Width - m_FontScaler*40);
  m_Bytes->Create();
  AddFrame(m_Bytes, StandardLayout);


  m_Maths = new MGUIERBList(this, "The built-in accuracy of several expensive mathematical functions (e.g. acos(), exp(), etc.) exceeds the requirements for imaging by far. The following allows to switch between the built-in functions and approximations:");
  m_Maths->Add("Exact maths");
  m_Maths->Add("Approximated maths (up to 75% faster)");
  m_Maths->SetSelected((m_GUIData->GetApproximatedMaths() == true) ? 1 : 0);
  m_Maths->SetWrapLength(Width - m_FontScaler*40);
  m_Maths->Create();
  AddFrame(m_Maths, StandardLayout);


  m_Parsing = new MGUIERBList(this, "This option enables fast parsing of the tra files with the disadantage of eliminating all error checks. Use this option only on unchanged files generated with MEGAlib:");
  m_Parsing->Add("Secure file parsing");
  m_Parsing->Add("Fast file parsing");
  m_Parsing->SetSelected((m_GUIData->GetFastFileParsing() == true) ? 1 : 0);
  m_Parsing->SetWrapLength(Width - m_FontScaler*40);
  m_Parsing->Create();
  AddFrame(m_Parsing, StandardLayout);


  m_Threads = 
    new MGUIEEntryList(this,
                   "Using multiple threads can speed up the image reconstruction on multi-core and/or multi-processor systems. However due to overhead and limitations (e.g. reading events from file), the performance will not scale with the number of threads. In addition, if you encounter unexpected crashes, reduce the number of threads to one.");
  m_Threads->Add("Number of threads to use: ", m_GUIData->GetNThreads(), true, 1, 64);
  m_Threads->SetWrapLength(Width - m_FontScaler*40);
  m_Threads->Create();
  AddFrame(m_Threads, StandardLayout);

  AddButtons();

  //PositionWindow(GetDefaultWidth(), GetDefaultHeight(), false);
  PositionWindow(Width, GetDefaultHeight(), false);

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIMemory::OnApply()
{
	// The Apply button has been pressed

  if (m_GUIData->GetRAM() != m_MaxRAM->GetAsInt(0)) m_GUIData->SetRAM(m_MaxRAM->GetAsInt(0));
  if (m_GUIData->GetBytes() != m_Bytes->GetSelected()) m_GUIData->SetBytes(m_Bytes->GetSelected());
  if (m_GUIData->GetApproximatedMaths() != (m_Maths->GetSelected() == 1) ? true : false) m_GUIData->SetApproximatedMaths((m_Maths->GetSelected() == 1) ? true : false);
  if (m_GUIData->GetFastFileParsing() != (m_Parsing->GetSelected() == 1) ? true : false) m_GUIData->SetFastFileParsing((m_Parsing->GetSelected() == 1) ? true : false);
  if (m_GUIData->GetNThreads() != m_Threads->GetAsInt(0)) m_GUIData->SetNThreads(m_Threads->GetAsInt(0));

  return true;
}


// MGUIMemory: the end...
////////////////////////////////////////////////////////////////////////////////
