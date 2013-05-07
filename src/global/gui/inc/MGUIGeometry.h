/*
 * MGUIGeometry.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIGeometry__
#define __MGUIGeometry__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsBasicFiles.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIGeometry : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIGeometry(const TGWindow* Parent, const TGWindow* Main, MSettingsBasicFiles* Data);
  virtual ~MGUIGeometry();

  virtual void Create();
  bool OkPressed() { return m_OkPressed; }
  MString GetGeometryFileName() { return m_FileSelector->GetFileName(); }

  // protected methods:
 protected:
	virtual bool OnOk();
	virtual bool OnApply();
	virtual bool OnCancel();


  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;
  MSettingsBasicFiles* m_GUIData;

  TGLayoutHints* m_FileSelectorLayout;
  MGUIEFileSelector* m_FileSelector;

  //! True if, the OK Button was pressed
  bool m_OkPressed;

  //! The geometry file name
  MString m_GeometryFileName;

#ifdef ___CINT___
 public:
  ClassDef(MGUIGeometry, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
