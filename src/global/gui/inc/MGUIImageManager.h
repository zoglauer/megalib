/*
 * MGUIImageManager.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MGUIImageManager__
#define __MGUIImageManager__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGFrame.h>
#include <TObjArray.h>
#include <TRootEmbeddedCanvas.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MImage.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIImageManager : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIImageManager();
  MGUIImageManager(const TGWindow* Parent, const TGWindow* Main);
  virtual ~MGUIImageManager();

  void Display();

  void AddAndDisplay(MImage* Image);

  virtual void CloseWindow(); // Only unmap!

  // protected methods:
 protected:

  // GUI interface:
  void Create();
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  // Image container interface
  void AddImage(MImage* Image);
  MImage* GetImageAt(int i);
  int GetNImages();
  void RemoveImage(MImage *Image);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TObjArray* m_ImageList;

  TGLayoutHints* m_CanvasColumnLayout;
  TGHorizontalFrame* m_CanvasColumn;

  TGLayoutHints* m_ECanvasLayout;
  TRootEmbeddedCanvas* m_ECanvas;



#ifdef ___CLING___
 public:
  ClassDef(MGUIImageManager, 0) // GUI window: manages display and manipulation of images
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
