/*
 * MGUICBSelection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUICBSelection__
#define __MGUICBSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIECBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUICBSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUICBSelection(const TGWindow *p, const TGWindow *main, 
                  MString Title = "", MString SubTitle = "");
  virtual ~MGUICBSelection();

  void Add(MString Title, int Selected = 0);
  void Add(TObjArray Titles, TArrayI Selected = TArrayI());
  TArrayI GetSelected();
  void EmitOkMessage(const TGWindow *MsgWindow, unsigned int MsgID1, unsigned int MsgID2);
  void SetMultiple(bool On);

  void Create();

  bool IsOKPressed();


  // protected methods:
 protected:
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  virtual bool OnOk();
  virtual bool OnCancel();
  virtual bool OnSelectAll();
  virtual bool OnSelectNone();


  // private methods:
 private:



  // protected members:
 protected:

  enum ButtonIDs { e_SelectAll = 10, e_SelectNone };

  // protected members:
 protected:
  MString m_Title;
  MString m_SubTitle;

  unsigned int m_MsgID1;
  unsigned int m_MsgID2;
  const TGWindow *m_MsgWindow;

	bool m_IsMultiple;
  
  MGUIECBList* m_List;
  TGLayoutHints* m_ListLayout;

  TGTextButton *m_SelectAll;
  TGTextButton *m_SelectNone;
  
  int m_LastPressedButton;


#ifdef ___CINT___
 public:
  ClassDef(MGUICBSelection, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
