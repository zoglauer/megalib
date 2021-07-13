/*
 * MPointSourceList.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPointSourceList__
#define __MPointSourceList__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MPointSource.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MPointSourceList : public TObject
{
  // public interface:
 public:
  MPointSourceList();
  ~MPointSourceList();

  // Image container interface


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MPointSourceList, 0) // a list of class MPointSource
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
