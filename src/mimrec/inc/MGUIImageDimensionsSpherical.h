/*
 * MGUIImageDimensionsSpherical.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIImageDimensionsSpherical__
#define __MGUIImageDimensionsSpherical__


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


class MGUIImageDimensionsSpherical : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIImageDimensionsSpherical(const TGWindow* Parent, const TGWindow* Main, 
                               MGUIDataMimrec* Data);
  virtual ~MGUIImageDimensionsSpherical();

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

  TGLayoutHints* m_AxisLayoutFirst;
  TGLayoutHints* m_AxisLayoutLast;
  TGLayoutHints* m_DimensionLayout;
  TGLayoutHints* m_BinLayout;

  MGUIEEntry* m_ThetaImageAxis;
  MGUIEEntry* m_PhiImageAxis;

  MGUIEMinMaxEntry* m_ThetaDimension;
  MGUIEEntry* m_ThetaBins;
  MGUIEMinMaxEntry* m_PhiDimension;
  MGUIEEntry* m_PhiBins;


#ifdef ___CINT___
 public:
  ClassDef(MGUIImageDimensionsSpherical, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
