/*
 * MImageContainer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImageContainer__
#define __MImageContainer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MImage;

////////////////////////////////////////////////////////////////////////////////


class MImageContainer : public TObject
{
  // public interface:
 public:
  MImageContainer();
  ~MImageContainer();

  int GetNImages();
  MImage* GetImageAt(int N);
  void AddImage(MImage *Image);
  void RemoveImage(MImage *Image);
  void RemoveImage(int N);
  void DeleteAll();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MImageContainer, 0) // container for MImage classes
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
