/*
 * MGUIEImage.h
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


#ifndef __MGUIEImage__
#define __MGUIEImage__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TGClient.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MImage.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEImage : public MGUIElement
{
  // public interface:
 public:
  MGUIEImage(const TGWindow* Parent, MImage *Image, unsigned int Width = 100, unsigned int Height = 50, bool DisplayUndock = true);
  virtual ~MGUIEImage();

  bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  MImage* GetImage();

  // protected methods:
 protected:
  void Create();
  void Undock();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MImage* m_Image;

  TGLayoutHints *m_ImageCanvasLayout;
  TRootEmbeddedCanvas *m_ImageCanvas;

  bool m_DisplayUndock;

  TGPictureButton *m_UndockButton;
  TGLayoutHints *m_UndockButtonLayout;
  

#ifdef ___CLING___
 public:
  ClassDef(MGUIEImage, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
