/*
 * MDX3DViewer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDX3DViewer__
#define __MDX3DViewer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TViewerX3D.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDX3DViewer : public TViewerX3D
{
  // public interface:
 public:
  MDX3DViewer(TVirtualPad* pad, Option_t* option, 
              const char* title = "X3D Viewer", unsigned int width = 800, 
              unsigned int height = 600);
  virtual ~MDX3DViewer();

  virtual void CloseWindow();

  // protected methods:
 protected:
  //MDX3DViewer() {};
  //MDX3DViewer(const MDX3DViewer& Dummy) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MDX3DViewer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
