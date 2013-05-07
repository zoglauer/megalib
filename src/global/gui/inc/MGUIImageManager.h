/*
 * MGUIImageManager.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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



#ifdef ___CINT___
 public:
  ClassDef(MGUIImageManager, 0) // GUI window: manages display and manipulation of images
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
