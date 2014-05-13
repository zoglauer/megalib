/*
 * MGUIMultiProgressBar.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIMultiProgressBar__
#define __MGUIMultiProgressBar__


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
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////

//! A progress bar with multiple cancelable progress and one master progress
class MGUIMultiProgressBar : public TGTransientFrame
{
  // Public Interface:
 public:
  //! Default constructor 
  MGUIMultiProgressBar(unsigned int NumberOfProgressBars);
  //! Default destructor
  virtual ~MGUIMultiProgressBar();

  //! Set the titles
  void SetTitles(const MString& Title, const MString& SubTitle) { m_Title = Title; m_SubTitle = SubTitle; }
  
  //! True if a master progress bar should be shown
  void ShowMasterProgressBar(bool Show = true) { m_ShowMasterProgressBar = Show; } 
  
  //! Set the minimum change in % required before updating the GUI
  void SetMinimumChange(double MinimumChange) { m_MinimumChange = MinimumChange; } 
  
  //! Set the Name of the given progress bar ID
  //! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
  void SetName(unsigned int ID, const MString& Name);
  //! Set the minimum and maximum value for the given progress bar ID
  //! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
  void SetMinMax(unsigned int ID, double Minimum, double Maximum);
  //! Set the current value for the given progress bar ID
  //! If it doesn't exist, throw an exception MExceptionIndexOutOfBounds
  void SetValue(unsigned int ID, double Value);
  
  //! Create the window - after that, the number of progress bars can no longer be changed!
  void Create();

  //! True if the progress bar cancel has been presses
  bool HasBeenCancelled() { return m_HasBeenCancelled; }

  //! Close the window - it will NOT be deleted by its own if the x of cancel button are pressed
  //! It will only not be shown!
  virtual void CloseWindow();


  // protected methods:
 protected:
  //! Set the number of progress bars
  void SetNumberOfProgressBars(unsigned int ProgressBars);

  //! Process all messagess
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);  
  
  //! Update the progress bars
  void Update();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The window title
  MString m_Title;
  //! The descriptive sub title
  MString m_SubTitle;
  //! Tne number of progress bars
  unsigned int m_NumberOfProgressBars;
  //! Show a master progress bars
  bool m_ShowMasterProgressBar;
  
  //! the names of the progress bars
  vector<MString> m_Names;
  //! The minimum values of the progress bars
  vector<double> m_Minima;
  //! The maximum values of the progress bars
  vector<double> m_Maxima;
  //! The current values of the progress bars
  vector<double> m_Values;
  
  //! The last shown values
  vector<double> m_LastPositions;
  //! The minimum chnage in % required to update the GUI
  double m_MinimumChange;
  
  //! The label with the subtitle
  TGLabel* m_SubTitleLabel;

  //! The label with the time estimaye
  TGLabel* m_InfoLabel;

  //! The progress bars
  vector<TGHProgressBar*> m_ProgressBars;

  //! The cancel button
  TGButton* m_CancelButton;

  //! True if the close or cancel button has been pressed
  bool m_HasBeenCancelled;

  //! The progress timer
  MTimer m_Timer;

#ifdef ___CINT___
 public:
  ClassDef(MGUIMultiProgressBar, 0)
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
