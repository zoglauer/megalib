/*
 * MGUIDummy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDummy__
#define __MGUIDummy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIData.h"
#include "MInterface.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDummy : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIDummy(const TGWindow* Parent, const TGWindow* Main, 
            MGUIData* Data = 0);
  //! Standard destructor
  virtual ~MGUIDummy();


  // protected methods:
 protected:
  //! Create all widgets of theis window
  virtual void Create();
  //! Process all messages for this window
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! Called when the Ok button has been pressed
  virtual bool OnOk();
  //! Called when the Apply or Ok button has been pressed
  virtual bool OnApply();
  //! Called when the Cancel button has been pressed
  virtual bool OnCancel();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The data storage
  MGUIData* m_Data;


#ifdef ___CLING___
 public:
  ClassDef(MGUIDummy, 0) // GUI window for unknown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
