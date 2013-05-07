/*
 * MGUILicense.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUILicense__
#define __MGUILicense__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGTextView.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUILicense : public MGUIDialog
{
  // Public Interface:
 public:
  MGUILicense(const TGWindow* Parent, const TGWindow* Main, 
              MString FileName, bool ShowButtons = true);
  virtual ~MGUILicense();

  bool IsAccepted();
  void Create();


  // protected methods:
 protected:
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

	bool OnAccept();
	bool OnDecline();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  bool m_IsAccepted;
  bool m_ShowButtons;
  MString m_FileName;


  TGTextView* m_Text;
  TGLayoutHints* m_TextLayout;

  TGHorizontalFrame* m_ButtonFrame;
  TGLayoutHints* m_ButtonFrameLayout;

  TGTextButton* m_DeclineButton;
  TGTextButton* m_AcceptButton;
  TGLayoutHints* m_ButtonLayout;

  enum ButtonIds { c_Decline = 100, c_Accept };


#ifdef ___CINT___
 public:
  ClassDef(MGUILicense, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
