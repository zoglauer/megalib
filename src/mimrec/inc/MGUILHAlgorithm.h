/*
 * MGUILHAlgorithm.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUILHAlgorithm__
#define __MGUILHAlgorithm__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGNumberEntry.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIDataMimrec.h"
#include "MGUIERBList.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUILHAlgorithm : public MGUIDialog
{
  // Public Interface:
 public:
  MGUILHAlgorithm(const TGWindow* Parent, const TGWindow* Main, 
                  MGUIDataMimrec* Data );
  virtual ~MGUILHAlgorithm();

  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
	virtual bool OnApply();

  void ToggleRadioButtons(int ID);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MGUIDataMimrec* m_Data;

  TGRadioButton* m_EM;
  TGRadioButton* m_OSEM;
  MGUIEEntry* m_SubSets;

  TGLabel* m_StopLabel;
  TGLayoutHints* m_StopLabelLayout;

  TGLayoutHints* m_RBLayout;
  TGLayoutHints* m_RBEntryLayout;
  TGLayoutHints* m_RBFrameLayout;

	TGCompositeFrame* m_RBIterationsFrame;
  TGRadioButton* m_RBIterations;
  MGUIEEntry* m_Iterations;

  TGCompositeFrame* m_RBIncreaseFrame;
  TGRadioButton* m_RBIncrease;
  MGUIEEntry* m_Increase;

	int m_Selected;


	enum ButtonIds { c_ClassicEM, c_OSEM, c_Iterations, c_Increase };


#ifdef ___CINT___
 public:
  ClassDef(MGUILHAlgorithm, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
