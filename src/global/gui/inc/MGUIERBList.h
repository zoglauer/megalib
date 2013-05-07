/*
 * MGUIERBList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIERBList__
#define __MGUIERBList__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIERBList : public MGUIElement
{
  // public interface:
 public:
  MGUIERBList(const TGWindow* Parent, MString Label = "", 
              bool Emphasize = false, int Mode = c_MultipleLine);
  virtual ~MGUIERBList();

	void SetEnabled(bool flag = true);
  bool IsEnabled() { return m_IsEnabled; }

  //! Set the wrap length of all text
	virtual void SetWrapLength(int WrapLength);

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  void Associate(TGWindow* Associate);

  int GetSelected();
  int GetNEntries() { return m_RBList->GetSize(); }
  void SetSelected(int Selected);
  void Add(MString RBLabel, int Selected = 0);
  void Create();

  enum Mode { c_MultipleLine, c_SingleLine };

  // protected methods:
 protected:


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  int m_Mode;

  MString m_Label;

  int m_Selected;
  TObjArray* m_RBList;
  TGLayoutHints* m_RBLayout;

  TGLabel* m_TextLabel;                   
  TGLayoutHints* m_TextLabelLayout;

  TGWindow* m_Associate;




#ifdef ___CINT___
 public:
  ClassDef(MGUIERBList, 0) // Basic GUI element: a radio button list
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
