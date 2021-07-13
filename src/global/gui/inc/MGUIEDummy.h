/*
 * MGUIEDummy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEDummy__
#define __MGUIEDummy__


////////////////////////////////////////////////////////////////////////////////


// Standardlibs:

// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEDummy : public MGUIElement
{
  // public interface:
 public:
  //! Standard constructor
  MGUIEDummy(const TGWindow* Parent, MString Label, bool Emphasize); 
  //! Default destructor
  virtual ~MGUIEDummy();

  //! Create the GUI
  void Create();
  //! Process or redirect all messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // private methods:
 private:

  // private members:
 private:
  MString m_Label;

  //! The Message-IDs of all manipulatable elements 
  enum Type { e_Nothing };


#ifdef ___CLING___
 public:
  ClassDef(MGUIEDummy, 0) // GUI window for unkown purpose ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
