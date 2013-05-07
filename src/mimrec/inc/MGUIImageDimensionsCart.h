/*
 * MGUIImageDimensionsCart.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIImageDimensionsCart__
#define __MGUIImageDimensionsCart__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIDataMimrec.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIImageDimensionsCart : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIImageDimensionsCart(const TGWindow* Parent, 
                          const TGWindow* Main, 
                          MGUIDataMimrec* Data);
  virtual ~MGUIImageDimensionsCart();

  // protected methods:
 protected:
  virtual void Create();
	virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MGUIDataMimrec* m_GUIData;

  TGLayoutHints* m_DimensionLayout;
  TGLayoutHints* m_BinLayout;

  MGUIEMinMaxEntry* m_XDimension;
  MGUIEEntry* m_XBins;
  MGUIEMinMaxEntry* m_YDimension;
  MGUIEEntry* m_YBins;
  MGUIEMinMaxEntry* m_ZDimension;
  MGUIEEntry* m_ZBins;


#ifdef ___CINT___
 public:
  ClassDef(MGUIImageDimensionsCart, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
