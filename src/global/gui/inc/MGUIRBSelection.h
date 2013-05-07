/*
 * MGUICBSelection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIRBSelection__
#define __MGUIRBSelection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIERBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIRBSelection : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIRBSelection(const TGWindow *p, const TGWindow *main, 
                  MString Title = "", MString SubTitle = "");
  virtual ~MGUIRBSelection();

  void Add(MString Title, int Selected = 0);
  void Add(TObjArray *Titles);
  void SetSelected(int Selected);
  int GetSelected();
  void EmitOkMessage(const TGWindow *MsgWindow, unsigned int MsgID1, unsigned int MsgID2);

  void Create();

  bool IsOKPressed();


  // protected methods:
 protected:
	bool OnOk();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_Title;
  MString m_SubTitle;

  unsigned int m_MsgID1;
  unsigned int m_MsgID2;
  const TGWindow *m_MsgWindow;
  
  MGUIERBList* m_List;
  TGLayoutHints* m_ListLayout;
  
  int m_LastPressedButton;


#ifdef ___CINT___
 public:
  ClassDef(MGUIRBSelection, 0) // GUI window for unknown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
