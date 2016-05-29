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


class MGUIProgressBar : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIProgressBar();
  MGUIProgressBar(const TGWindow* Parent, const char* Title, const char* SubTitle);
  virtual ~MGUIProgressBar();

  void SetTitles(const char* Title, const char* SubTitle);
  void SetMinMax(double Minimum, double Maximum);
  void SetValue(double Value, unsigned int Level = 0);
  void Reset();
  //! Just reset the timer
  void ResetTimer();

  bool TestCancel();

  void CloseWindow();


  // protected methods:
 protected:
  bool ProcessMessage(long msg, long param1, long);  
  void Create();

  virtual bool OnOk();
  virtual bool OnCancel();
  void Update(double Percentage, unsigned int Level);
  void AddProgressBar();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;
  MString m_SubTitle;

  TGHorizontalFrame* m_TitleFrame;
  TGLayoutHints * m_TitleFrameLayout;

  TGLabel* m_SubTitleLabel;
  TGLayoutHints* m_SubTitleLabelLayout;

  TGLabel* m_InfoLabel;
  TGLayoutHints* m_InfoLabelLayout;

  TGVerticalFrame* m_ProgressBarFrame;
  TGLayoutHints * m_ProgressBarFrameLayout;

  vector<TGHProgressBar*> m_ProgressBar;
  TGLayoutHints * m_ProgressBarLayout;

  TGVerticalFrame* m_ButtonFrame;
  TGLayoutHints * m_ButtonFrameLayout;

  TGButton* m_CancelButton;
  TGLayoutHints* m_CancelButtonLayout;

  
  vector<double> m_Percentage;
  vector<double> m_ColorLevel;

  double m_Minimum;
  double m_Maximum;

  double m_UpdateFrequency;

  unsigned int m_Width;

  bool m_Cancel;

  bool m_IsFirstUpdate;
  MTimer m_Timer;

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
