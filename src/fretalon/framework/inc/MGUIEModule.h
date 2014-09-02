/*
 * MGUIEModule.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEModule__
#define __MGUIEModule__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGLabel.h"
#include "TObjArray.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEModule : public MGUIElement
{
  // Public Interface:
 public:
  //! Default constructor
  MGUIEModule(const TGWindow* Parent, unsigned int ID, MModule* Module = 0);
  //! default destructor
  virtual ~MGUIEModule();

  //! Transfer button clicks to this window
  void Associate(const TGWindow* Window);

  // protected methods:
 protected:
  //! Create this GUI element
  void Create();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! ID
  unsigned int m_ID;
  //! The module
  MModule* m_Module;

  //! The options button
  TGTextButton* m_OptionsButton;
  //! The remove button
  TGTextButton* m_RemoveButton;
  //! The change button
  TGTextButton* m_ChangeButton;


#ifdef ___CINT___
 public:
  ClassDef(MGUIEModule, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
