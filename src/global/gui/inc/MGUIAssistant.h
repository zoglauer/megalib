/*
 * MGUIAssistant.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIAssistant__
#define __MGUIAssistant__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIAssistant : public TGTransientFrame
{
  // public Interface
 public:
  MGUIAssistant();
  MGUIAssistant(const TGWindow *p, const TGWindow *main, unsigned int Type = 2, unsigned int w = 320, unsigned int h = 240, unsigned int options = kMainFrame | kVerticalFrame);
  virtual ~MGUIAssistant();


  enum AssistantButtons { c_Back = 1, c_Next, c_Cancel };
  enum AssistantType { c_First = 1, c_Middle, c_Last };

  // protected methods:
 protected:
  void PositionWindow(int a, int b, bool AllowResize = true);

  virtual void Create() = 0;
  virtual bool ProcessMessage(long msg, long param1, long) = 0;
  virtual void CloseWindow();

  void Header(MString Label);
  void SetFooterText(MString Back, MString Next, MString Cancel);
  void Footer();

  void SetTextBack(MString String = "<< Back <<");
  void SetTextCancel(MString String = "Cancel");
  void SetTextNext(MString String = ">> Next >>");

  
  virtual void OnBack();
  virtual void OnNext();
  virtual void OnCancel();
  

  // protected members:
 protected:
  TGFrame *m_ParentWindow;              // The window which called this class

  TGHorizontalFrame *m_ButtonFrame;     // Frame for the OK- and Cancel-button
  TGLayoutHints *m_ButtonFrameLayout;   // its layout

  MString m_BackText;
  TGTextButton *m_BackButton;             // the OK-Button
  TGLayoutHints *m_BackButtonLayout;      // its layout

  MString m_CancelText;
  TGTextButton *m_CancelButton;         // the cancel button
  TGLayoutHints *m_CancelButtonLayout;  // its layout

  MString m_NextText;
  TGTextButton *m_NextButton;             // the OK-Button
  TGLayoutHints *m_NextButtonLayout;      // its layout

  TGCompositeFrame *m_Frame;            // Frame where all other windows are added
  TGLayoutHints *m_FrameLayout;      // its layout

  // private members:
 private:
  unsigned int m_Type;

  bool m_ButtonsAdded;
  bool m_SubTitleAdded;

  TGVerticalFrame *m_LabelFrame;        // Frame for the subtitle
  TGLayoutHints *m_LabelFrameLayout;    // Layout for this frame
  TGLayoutHints *m_SubTitleFirstLayout; // Layout of the subtitles
  TGLayoutHints *m_SubTitleMiddleLayout;// Layout of the subtitles
  TGLayoutHints *m_SubTitleLastLayout;  // Layout of the subtitles
  TGLayoutHints *m_SubTitleOnlyLayout;  // Layout of the subtitles
  TObjArray *m_SubTitleLabel;           // Array of the subtitle rows



#ifdef ___CLING___
 public:
  ClassDef(MGUIAssistant, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
