/*
 * MGUIProgressBar.cxx
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
// MGUIProgressBar
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIProgressBar.h"

// Standard libs:
#include <sstream>
using namespace std;

// ROOT libs:
#include <TSystem.h>
#include <TGClient.h>
#include <TGMsgBox.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIProgressBar)
#endif


MGUIProgressBar::MGUIProgressBar(): MGUIDialog(gClient->GetRoot(), gClient->GetRoot(), 350, 350)
{
  // standard constructor

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_Title = MString("Status");
  m_SubTitle = MString("Progress");

  m_Cancel = false;
  m_Percentage.push_back(0);
  m_ColorLevel.push_back(0);
  m_Minimum = 0;
  m_Maximum = 1;

  m_UpdateFrequency = 0.001;

  m_IsFirstUpdate = true;

  m_ConfirmCancel = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIProgressBar::MGUIProgressBar(const TGWindow* ParentWindow, const char* Title, 
                                 const char* SubTitle): 
  MGUIDialog(gClient->GetRoot(), gClient->GetRoot(), 350, 350)
{
  //

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  m_Title = MString(Title);
  m_SubTitle = MString(SubTitle);

  m_Cancel = false;
  m_Percentage.push_back(0);
  m_ColorLevel.push_back(0);

  m_Minimum = 0;
  m_Maximum = 1;

  m_UpdateFrequency = 0.001;

  m_IsFirstUpdate = true;

  m_ConfirmCancel = false;

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIProgressBar::~MGUIProgressBar()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::SetTitles(const char* Title, const char* SubTitle)
{
  // Set the titles

  m_Title = MString(Title);
  m_SubTitle = MString(SubTitle);

  SetWindowName(m_Title);  
  m_SubTitleLabel->SetText(m_SubTitle);

	MapSubwindows();
  MapWindow();
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::SetMinMax(double Minimum, double Maximum)
{
  // Set the minimum input value and the maximum input value

  if (Maximum == 0) {
    Warning("void MGUIProgressBar::SetMinMax(double Minimum, double Maximum)",
            "Maximum is not allowed to be 0!");
  }

  m_Minimum = Minimum;
  m_Maximum = Maximum;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::SetValue(double Value, unsigned int Level)
{
  // Set the current value
  // It must be between m_Minimum and m_Maximum set via SetMinMax or as default
  // between 

  Update((Value - m_Minimum)/m_Maximum, Level);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIProgressBar::ProcessMessage(long msg, long param1, long param2)
{
  //

  switch (GET_MSG(msg)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(msg)) {
    case kCM_BUTTON:
      switch(param1) {
      case 1:  
        OnCancel();
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


void MGUIProgressBar::CloseWindow()
{
  // If somebody presses the x-button we reach this function
  // Normally this window should be deleted, but the programmer has to take care of 
  // the deletion of this window, it is not allowed to do this by itself.

  UnmapWindow();
  m_Cancel = true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::Create()
{
  SetWindowName((char *) m_Title.Data());  

  m_TitleFrame = new TGHorizontalFrame(this, this->GetWidth(), 0, kRaisedFrame);
  m_TitleFrameLayout =  new TGLayoutHints(kLHintsExpandX | kLHintsCenterX | kLHintsTop, 10, 10, 8, 2);
  AddFrame(m_TitleFrame, m_TitleFrameLayout);


  // Label:
  m_SubTitleLabel = new TGLabel(m_TitleFrame, (char *) m_SubTitle.Data());
  m_SubTitleLabelLayout = new TGLayoutHints(kLHintsCenterX, 0, 0, 2, 2);
  m_TitleFrame->AddFrame(m_SubTitleLabel, m_SubTitleLabelLayout);


  // Info:
  m_InfoLabel = new TGLabel(this, "Timing: estimating...");
  m_InfoLabelLayout = new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, 2, 2, 10, 0);
  AddFrame(m_InfoLabel, m_InfoLabelLayout);


  // Frame which contains all status bars:
  m_ProgressBarFrame = new TGVerticalFrame(this, this->GetWidth() - m_FontScaler*20, m_FontScaler*100);
  m_ProgressBarFrameLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsCenterX | kLHintsExpandY | kLHintsCenterY, 10, 10, 5, 16);
  AddFrame(m_ProgressBarFrame, m_ProgressBarFrameLayout);

  m_Width = m_FontScaler*this->GetWidth() - m_FontScaler*22;

  m_ProgressBarLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 0, 0, 0, 0);

  // Add the progress bar
  AddProgressBar();

  // Cancel and OK:
  m_ButtonFrame = new TGVerticalFrame(this, this->GetWidth()-m_FontScaler*40, m_FontScaler*25, kFixedSize);
	  
  m_CancelButton = new TGTextButton(m_ButtonFrame, "Cancel", 1); 
  m_CancelButton->Associate(this);
  m_CancelButtonLayout = 
    new TGLayoutHints(kLHintsCenterX | kLHintsExpandX, m_FontScaler*70, m_FontScaler*70, 0, 0);

  m_ButtonFrame->AddFrame(m_CancelButton, m_CancelButtonLayout);
  m_ButtonFrameLayout = 
    new TGLayoutHints(kLHintsExpandX | kLHintsCenterX | kLHintsBottom, 2, 2, 2, 2);
  AddFrame(m_ButtonFrame, m_ButtonFrameLayout);

  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), true); // true required for Mac
  
  MapSubwindows();
  MapWindow();  
  Layout();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::AddProgressBar()
{
  m_ProgressBarFrame->UnmapWindow();

  TGHProgressBar* ProgressBar = new TGHProgressBar(m_ProgressBarFrame, m_Width, m_FontScaler*24);
  m_ProgressBar.push_back(ProgressBar);
  ProgressBar->SetBarColor("darkgreen");
  m_Percentage.push_back(0);
  m_ColorLevel.push_back(1);

  ostringstream out;
  out<<"Level "<<m_ProgressBar.size()<<": %.1f %%";
  ProgressBar->ShowPosition(kTRUE, kFALSE, out.str().c_str());

  m_ProgressBarFrame->AddFrame(ProgressBar, m_ProgressBarLayout);

  m_ProgressBarFrame->MapWindow();


  // Let's resize and position the window, 
  PositionWindow(GetDefaultWidth(), GetDefaultHeight(), true); // true required for Mac

  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::Reset()
{
  // Reset to zero percent:

  m_Cancel = false;
  m_Percentage.clear();
  m_Percentage.push_back(0);
  m_ColorLevel.clear();
  m_ColorLevel.push_back(0);
  m_Minimum = 0;
  m_Maximum = 1;

  m_IsFirstUpdate = true;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::ResetTimer()
{
  // Reset the timer, in this case only tell the function that the next is the first update

  m_IsFirstUpdate = true;
}
 

////////////////////////////////////////////////////////////////////////////////


void MGUIProgressBar::Update(double Percentage, unsigned int Level)
{

  if (Percentage < 0) Percentage = 0;
  if (Percentage > 1) Percentage = 1;

  if (Level >= m_ProgressBar.size()) {
    AddProgressBar();
  }

  if (m_IsFirstUpdate == true) {
    m_Timer.Start();
    m_IsFirstUpdate = false;
  }

  if (Percentage >= 0 && Percentage <= 1) {
    if (Percentage > m_Percentage[Level]+m_UpdateFrequency || Percentage < m_Percentage[Level]) {

      MTimer Intermediate;

      m_ProgressBar[Level]->Increment(100.0*(Percentage-m_Percentage[Level])); 
      m_Percentage[Level] = Percentage;
      
      char c[100];
      if (m_Timer.ElapsedTime() < 3.0 || m_Percentage[0] == 0) { 
        sprintf(c, "Time: estimating...");
      } else {
        double Time = m_Timer.ElapsedTime() / m_Percentage[0];
        sprintf(c, "Time: %i:%02i/%i:%02i", 
                int(m_Timer.ElapsedTime())/60, int(m_Timer.ElapsedTime())%60,
                int(Time)/60, int(Time)%60);
      }
      m_InfoLabel->SetText(new TGString(c));
      
      if (m_Percentage[Level] >= 0 && m_Percentage[Level] < 0.50) {
        if (m_ColorLevel[Level] != 1) {
          m_ProgressBar[Level]->SetBarColor("darkgreen");
          m_ColorLevel[Level] = 1;
        }
      }
      if (m_Percentage[Level] >= 0.50 && m_Percentage[Level] < 0.70) {
        if (m_ColorLevel[Level] != 2) {
          m_ProgressBar[Level]->SetBarColor("green");
          m_ColorLevel[Level] = 2;
        }
      }
      if (m_Percentage[Level] >= 0.70 && m_Percentage[Level] < 0.85) {
        if (m_ColorLevel[Level] != 3) {
          m_ProgressBar[Level]->SetBarColor("yellow");
          m_ColorLevel[Level] = 3;
        }
      }
      if (m_Percentage[Level] >= 0.85 && m_Percentage[Level] < 0.95) {
        if (m_ColorLevel[Level] != 4) {
          m_ProgressBar[Level]->SetBarColor("orange");
          m_ColorLevel[Level] = 4;
        }
      }
      if (m_Percentage[Level] >= 0.95 && m_Percentage[Level] <= 1.00) {
        if (m_ColorLevel[Level] != 5) {
          m_ProgressBar[Level]->SetBarColor("red");
          m_ColorLevel[Level] = 5;
        }
      }
      gSystem->ProcessEvents();

      // Allow some adjustabel updating 
      if (Intermediate.GetElapsed() > 0.1) {
        m_UpdateFrequency = 0.1*Intermediate.GetElapsed();
        if (m_UpdateFrequency > 0.02) m_UpdateFrequency = 0.02;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIProgressBar::OnOk()
{
  // The Ok button/Return key has been pressed
  // We do nothing!!!

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIProgressBar::OnCancel()
{
  if (m_ConfirmCancel == true) {
    int Return; 
    new TGMsgBox(gClient->GetRoot(), this, "Warning", 
                 "Do you really want to cancel?", 
                 kMBIconStop, kMBYes|kMBNo, &Return);
    if (Return == kMBYes) {
      m_Cancel = true;
    }
  } else {
    m_Cancel = true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIProgressBar::TestCancel()
{
  // returns true if the Cancel-button has been pressed previously.

  return m_Cancel;
}


// MGUIProgressBar: the end...
////////////////////////////////////////////////////////////////////////////////
