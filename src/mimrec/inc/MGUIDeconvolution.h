/*
 * MGUIDeconvolution.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDeconvolution__
#define __MGUIDeconvolution__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGNumberEntry.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIERBList.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDeconvolution : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIDeconvolution(const TGWindow* Parent, const TGWindow* Main, 
                    MSettingsImaging* Data );
  virtual ~MGUIDeconvolution();

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
  MSettingsImaging* m_Data;

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


  enum ButtonIds { c_ClassicEM, c_OSEM, c_Iterations, c_Increase };


#ifdef ___CLING___
 public:
  ClassDef(MGUIDeconvolution, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
