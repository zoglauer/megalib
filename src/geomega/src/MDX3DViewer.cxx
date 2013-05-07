/*
 * MDX3DViewer.cxx                                   v1.0  01/05/2003
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MDX3DViewer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDX3DViewer.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDX3DViewer)
#endif


////////////////////////////////////////////////////////////////////////////////


MDX3DViewer::MDX3DViewer(TVirtualPad* pad, Option_t* option, 
                         const char* title, unsigned int width, unsigned int height) :
  TViewerX3D(pad, option, title, width, height)
{
  // Construct an instance of MDX3DViewer
  //CreateScene("");
}


////////////////////////////////////////////////////////////////////////////////


MDX3DViewer::~MDX3DViewer()
{
  // Delete this instance of MDX3DViewer
}


////////////////////////////////////////////////////////////////////////////////


void MDX3DViewer::CloseWindow()
{
  // Only unmap the window, do not close it

  UnmapWindow();
}


// MDX3DViewer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
