/*
 * MGUIProgressBar.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIProgressBar__
#define __MGUIProgressBar__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGWindow.h>
#include <TGLabel.h>
#include <TGClient.h>
#include <TSystem.h>
#include <TGProgressBar.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


//! MEGAlib's default progress bar
class MGUIProgressBar : public MGUIDialog
{
  // Public Interface:
 public:
  //! Default constructor
  MGUIProgressBar();
  //! Constructor with title and subtitle
  MGUIProgressBar(MString Title, MString SubTitle);
  //! Default destructor
  virtual ~MGUIProgressBar();

  //! Set the titles
  void SetTitles(MString Title, MString SubTitle);
  //! Set the minimum and maximum value of the progress bar
  void SetMinMax(double Minimum, double Maximum);
  //! Set the value of a given level
  void SetValue(double Value, unsigned int Level = 0);
  //! Reset the progress bar 
  void Reset();
  //! Just reset the timer
  void ResetTimer();

  //! Test if cancel has been pressed
  bool TestCancel() { return m_Cancel; }

  //! Close the window
  void CloseWindow();


  // protected methods:
 protected:
  //! Create the UI
  void Create();
  //! Process all UI messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);  

  //! Overwritten from the base class - don't do anything on OK
  virtual bool OnOk() { return false; }
  //! When the OK button has been pressed
  virtual bool OnCancel();
  
  //! Update the displayed progress
  void Update(double Percentage, unsigned int Level);
  //! Add another progress bar
  void AddProgressBar();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The title
  MString m_Title;
  //! The subtitle
  MString m_SubTitle;

  //! The title frame
  TGHorizontalFrame* m_TitleFrame;
  //! Th layout of the title frame
  TGLayoutHints * m_TitleFrameLayout;

  //! The sub title
  TGLabel* m_SubTitleLabel;
  //! The layout of the sub title
  TGLayoutHints* m_SubTitleLabelLayout;

  //! The info label with the time
  TGLabel* m_InfoLabel;
  //! The layout of the info label
  TGLayoutHints* m_InfoLabelLayout;

  //! The frame contianing the progress bars
  TGVerticalFrame* m_ProgressBarFrame; 
  //! The layout of the frame containing the progress bars
  TGLayoutHints * m_ProgressBarFrameLayout;

  //! The progress bars
  vector<TGHProgressBar*> m_ProgressBar; 
  //! The layout of the progress bars
  TGLayoutHints * m_ProgressBarLayout;

  //! The cancel button frame
  TGVerticalFrame* m_ButtonFrame;
  //! The layout of the cancel button
  TGLayoutHints * m_ButtonFrameLayout;

  //! The cancel button
  TGButton* m_CancelButton;
  //! The cancel button layout
  TGLayoutHints* m_CancelButtonLayout;
  
  // The progress bar width
  unsigned int m_Width;
  
  //! The currently displayed percentages per progress bar
  vector<double> m_Percentage;
  //! The current color of each progress bar
  vector<double> m_ColorLevel;

  //! The last displayed info text
  MString m_LastInfoText;
  
  //! The minimum value of the progress bar
  double m_Minimum;
  //! The maximum value of the progress bar
  double m_Maximum;

  //! The updatye frequency [0..1]
  double m_UpdateFrequency;

  //! The cancel button has been pressed
  bool m_Cancel;

  //! True if this is the first update of the progress bar
  bool m_IsFirstUpdate;
  
  //! The timer since the first update
  MTimer m_Timer;
  //! The last update in seconds of the deepest level
  double m_LastUpdate;

  //! True, if after p[ressing cancel a confirm window is shown
  bool m_ConfirmCancel;
  
  //! The last estimates Duration in seconds;
  double m_Duration;

  
#ifdef ___CINT___
 public:
  ClassDef(MGUIProgressBar, 0)
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
