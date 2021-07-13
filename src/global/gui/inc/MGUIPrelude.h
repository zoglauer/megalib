/*
 * MGUIPrelude.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIPrelude__
#define __MGUIPrelude__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGTextView.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIPrelude : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPrelude(MString Title, MString SubTitle, MString Text, MString OKButton, MString CancelButton = "");
  virtual ~MGUIPrelude();

  //! Close the window
  virtual void CloseWindow();

  bool IsOKed();
  void Create();


  // protected methods:
 protected:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  bool OnOK();
  bool OnCancel();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;
  MString m_SubTitle;
  MString m_Text;
  MString m_OKButtonText;
  MString m_CancelButtonText;
   
  bool m_IsOKed;

  TGTextView* m_TextView;
  TGLayoutHints* m_TextLayout;

  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_OKButton;
  TGTextButton* m_CancelButton;
  TGLayoutHints* m_ButtonLayout;



#ifdef ___CLING___
 public:
  ClassDef(MGUIPrelude, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
