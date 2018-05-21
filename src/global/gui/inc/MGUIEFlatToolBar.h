/*
 * MGUIEFlatToolBar.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEFlatToolBar__
#define __MGUIEFlatToolBar__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEFlatToolBar : public TGCompositeFrame
{
  // public interface:
 public:
  MGUIEFlatToolBar(const TGWindow *Parent, unsigned int w, unsigned int h, unsigned int Options);
  virtual ~MGUIEFlatToolBar();

  bool Add(const TGWindow* Associate, MString IconFile, int Id = -1, unsigned int Distance = 0, MString DistToolTip = "");

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TObjArray* m_Pictures;
  TObjArray* m_Buttons;
  TObjArray* m_Layouts;


#ifdef ___CLING___
 public:
  ClassDef(MGUIEFlatToolBar, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
