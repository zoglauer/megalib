/*
 * MGUIMultiProgressBar.cxx
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
// MGUIMultiProgressBar
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIMultiProgressBar.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:
#include <TSystem.h>
#include <TGClient.h>
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MExceptions.h"
#include "MGUIDefaults.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIMultiProgressBar)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The default constructor
MGUIMultiProgressBar::MGUIMultiProgressBar(unsigned int NumberOfProgressBars)
  : TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 320, 240)
{
  // standard constructor

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_Title = "Status";
  m_SubTitle = "Progress";

  m_ShowMasterProgressBar = false;
  SetNumberOfProgressBars(NumberOfProgressBars);
  
  m_MinimumChange = 0.1;
  
  m_HasBeenCancelled = false;
}


////////////////////////////////////////////////////////////////////////////////


//! The default destructor
MGUIMultiProgressBar::~MGUIMultiProgressBar()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


//! Set the number of progress bars
void MGUIMultiProgressBar::SetNumberOfProgressBars(unsigned int ProgressBars) 
{ 
  m_NumberOfProgressBars = ProgressBars;
  
  m_Names.resize(m_NumberOfProgressBars);
  m_Minima.resize(m_NumberOfProgressBars);
  m_Maxima.resize(m_NumberOfProgressBars);
  m_Values.resize(m_NumberOfProgressBars);
  m_LastPositions.resize(m_NumberOfProgressBars);
  
  for (unsigned int i = 0; i < m_NumberOfProgressBars; ++i) {
    m_Names[i] = "";
    m_Minima[i] = 0;
    m_Maxima[i] = 1;
    m_Values[i] = 0;
    m_LastPositions[i] = 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the Name of the given progress bar ID
//! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
void MGUIMultiProgressBar::SetTitle(unsigned int ID, const MString& Name)
{
  if (ID < m_NumberOfProgressBars) {
    m_Names[ID] = Name;
    Update();
  } else {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfProgressBars, ID);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the minimum and maximum value for the given progress bar ID
//! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
void MGUIMultiProgressBar::SetMinMax(unsigned int ID, double Minimum, double Maximum)
{
  if (ID < m_NumberOfProgressBars) {
    m_Minima[ID] = Minimum;
    m_Maxima[ID] = Maximum;
    m_Values[ID] = Minimum;
    Update();
  } else {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfProgressBars, ID);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the current value for the given progress bar ID
//! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
void MGUIMultiProgressBar::SetValue(unsigned int ID, double Value)
{
  if (ID < m_NumberOfProgressBars) {
    m_Values[ID] = Value;
    Update();
  } else {
    throw MExceptionIndexOutOfBounds(0, m_NumberOfProgressBars, ID);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Process all messages for this UI
bool MGUIMultiProgressBar::ProcessMessage(long msg, long param1, long param2)
{
  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch(param1) {
      case 1:  
        CloseWindow();
        break;
      default:
        break;
      } // button
      break;
    default:
      break;
    } // submsg
    break;
  default:
    break;
  } // msg
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIMultiProgressBar::CloseWindow()
{
  // If somebody presses the x-button we reach this function
  // Normally this window should be deleted, but the programmer has to take care of 
  // the deletion of this window, it is not allowed to do this by itself.

  UnmapWindow();
  m_HasBeenCancelled = true;
}


////////////////////////////////////////////////////////////////////////////////


//! Bring the GUI to the screen
void MGUIMultiProgressBar::Create()
{
  double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();
  
  SetWindowName(m_Title);  
  
  // Sub title: Frame and label
  TGHorizontalFrame* SubTitleFrame = new TGHorizontalFrame(this, 100, 0, kRaisedFrame);
  TGLayoutHints* SubTitleFrameLayout = new TGLayoutHints(kLHintsExpandX | kLHintsCenterX | kLHintsTop, 10, 10, 8, 2);
  AddFrame(SubTitleFrame, SubTitleFrameLayout);

  m_SubTitleLabel = new TGLabel(SubTitleFrame, m_SubTitle);
  TGLayoutHints* SubTitleLabelLayout = new TGLayoutHints(kLHintsCenterX, 0, 0, 2, 2);
  SubTitleFrame->AddFrame(m_SubTitleLabel, SubTitleLabelLayout);


  // Total time estimate label
  m_InfoLabel = new TGLabel(this, "Duration: estimating...");
  TGLayoutHints* InfoLabelLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 2, 10, 0);
  AddFrame(m_InfoLabel, InfoLabelLayout);


  // Frame which contains all status bars:
  TGVerticalFrame* ProgressBarFrame = new TGVerticalFrame(this, 200, 100);
  TGLayoutHints* ProgressBarFrameLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 10, 10, 5, 16);
  AddFrame(ProgressBarFrame, ProgressBarFrameLayout);

  TGLayoutHints* ProgressBarLayout = new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 0, 0, 2, 2);

  // Add the progress bar
  for (unsigned int p = 0; p < m_NumberOfProgressBars; ++p) {
    TGHProgressBar* ProgressBar = new TGHProgressBar(ProgressBarFrame, FontScaler*200, FontScaler*24);
    ProgressBar->SetBarColor("lightblue");
    ProgressBar->SetMin(0);
    ProgressBar->SetMax(100);
    ProgressBar->SetPosition(0);
    MString Format(m_Names[p] + ": %.1f %%");
    ProgressBar->ShowPosition(true, false, Format);
    ProgressBarFrame->AddFrame(ProgressBar, ProgressBarLayout);

    m_ProgressBars.push_back(ProgressBar);
  }

  // Cancel button
  m_CancelButton = new TGTextButton(this, "Cancel", 1); 
  m_CancelButton->Associate(this);
  TGLayoutHints* CancelButtonLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 70, 70, 0, 10);
  AddFrame(m_CancelButton, CancelButtonLayout);

  TGDimension D = GetDefaultSize();
  if (D.fWidth < 350) D.fWidth = 350;
  Resize(D);
  
  CenterOnParent();
  
  MapSubwindows();
  MapWindow();  
  Layout();
  
  //! Finally start the timer
  m_Timer.Start();
  
  return;
}
 

////////////////////////////////////////////////////////////////////////////////


void MGUIMultiProgressBar::Update()
{
  // We have no GUI yet...
  if (m_ProgressBars.size() == 0) return;
  
  
  // Start with calculating the new positions
  vector<double> NewPositions(m_ProgressBars.size());
  for (unsigned int p = 0; p < m_ProgressBars.size(); ++p) {
    if (m_Maxima[p] - m_Minima[p] > 0) {
      NewPositions[p] = 100.0*m_Values[p]/(m_Maxima[p] - m_Minima[p]);
    } else {
      NewPositions[p] = 0.0;
    }
    if (NewPositions[p] > 100.0) NewPositions[p] = 100.0; 
    if (NewPositions[p] < 0.0) NewPositions[p] = 0.0; 
  }
  
  
  // Check if we have at least 0.1% change in one of the bars:
  bool PerformUpdate = false;
  for (unsigned int p = 0; p < m_ProgressBars.size(); ++p) {
    if (NewPositions[p] - m_LastPositions[p] > m_MinimumChange) {
      PerformUpdate = true;
      break;
    }
  }
  if (PerformUpdate == false) return;
      
  
  // Start with updating the progress bars
  double SlowestProgress = 100.0;
  for (unsigned int p = 0; p < m_ProgressBars.size(); ++p) {
    m_LastPositions[p] = NewPositions[p];
    if (NewPositions[p] < SlowestProgress) SlowestProgress = NewPositions[p];
    m_ProgressBars[p]->SetPosition(NewPositions[p]);
  }
  
  // Update the info section
  double Elapsed = m_Timer.ElapsedTime();
  if (Elapsed < 3.0 || SlowestProgress == 0) { 
    m_InfoLabel->SetText("Duration: estimating...");
  } else {
    double Time = 100.0 * m_Timer.ElapsedTime() / SlowestProgress;
    ostringstream out;
    
    out<<"Duration: "<<int(Elapsed)/60<<":"<<setfill('0')<<setw(2)<<int(Elapsed)%60
       <<" / "<<int(Time)/60<<":"<<setw(2)<<int(Time)%60;
    m_InfoLabel->SetText(out.str().c_str());
  }
  
  gSystem->ProcessEvents();
}


// MGUIMultiProgressBar: the end...
////////////////////////////////////////////////////////////////////////////////
