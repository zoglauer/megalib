/*
 * MGUIMultiInput.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIMultiInput__
#define __MGUIMultiInput__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include <TGButton.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TObjArray.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIData.h"



////////////////////////////////////////////////////////////////////////////////


class MGUIMultiInput : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIMultiInput(const TGWindow* Parent, const TGWindow* Main, 
                 MString Title, MString SubTitle, int nEntries,
                 MString* Label, MString* Input);
  ~MGUIMultiInput();


  // protected methods:
 protected:
  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;      // Title of the window
  MString m_SubTitle;   // Description of the purpose of this dialog 
  
  int m_NEntries;     // number textboxes

  MString* m_Label;     // labels of the textboxes
  MString* m_Input;     // pointer to the default and returned text

  //
  TGVerticalFrame* m_LabelFrame;
  TGLayoutHints* m_LabelFrameLayout;

  TGLayoutHints* m_SubTitleLabelLayout;
  TObjArray* m_SubTitleLabel;

  //
  TGHorizontalFrame* m_AddFrame;
  TGLayoutHints* m_AddFrameLayout;
  
  TGVerticalFrame* m_AddLabelsFrame;
  TGLayoutHints* m_AddLabelsFrameLayout;

  TObjArray* m_NameLabel;
  TGLayoutHints* m_NameLabelLayout;

  TGVerticalFrame* m_AddEntryFrame;
  TGLayoutHints* m_AddEntryFrameLayout;

  TObjArray* m_NameEntry;
  TGLayoutHints* m_NameEntryLayout;

  //
  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_CancelButton;
  TGLayoutHints* m_CancelButtonLayout;

  TGTextButton* m_OKButton;
  TGLayoutHints* m_OKButtonLayout;



#ifdef ___CINT___
 public:
  ClassDef(MGUIMultiInput, 0) // multiple input dialog box
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
