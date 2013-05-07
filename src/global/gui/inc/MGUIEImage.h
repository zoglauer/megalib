/*
 * MGUIEImage.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
  

#ifdef ___CINT___
 public:
  ClassDef(MGUIEImage, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
