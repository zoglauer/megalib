/*
 * MGUIEviewOptions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEviewOptions__
#define __MGUIEviewOptions__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEviewOptions : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIEviewOptions(const TGWindow *p, const TGWindow *main, MGUIData *Data = 0);
  virtual ~MGUIEviewOptions();


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
  MGUIData* m_GUIData;




#ifdef ___CINT___
 public:
  ClassDef(MGUIEviewOptions, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
