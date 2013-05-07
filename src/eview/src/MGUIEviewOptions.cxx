/*
 * MGUIEviewOptions.cxx
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
// MGUIEviewOptions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIEviewOptions.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIEviewOptions)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIEviewOptions::MGUIEviewOptions(const TGWindow* Parent, const TGWindow* Main, 
                                   MGUIData* Data)
  : MGUIDialog(Parent, Main)
{
  // Construct an instance of MGUIEviewOptions and bring it to the screen

  m_GUIData = Data;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUIEviewOptions::~MGUIEviewOptions()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIEviewOptions::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Title");  

  AddSubTitle("Subtitle"); 

  // Add here ...


  AddOKCancelButtons();

  PositionWindow(GetDefaultWidth(), GetDefaultHeight());

  // and bring it to the screen.
  MapSubwindows();
  MapWindow();  

  Layout();
 
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIEviewOptions::OnApply()
{
  return true;
}


// MGUIEviewOptions: the end...
////////////////////////////////////////////////////////////////////////////////
