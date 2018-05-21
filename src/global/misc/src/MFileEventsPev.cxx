/*
 * MFileEventsPev.cxx                                   v0.1  14/09/2001
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
// MFileEventsPev
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsPev.h"

// Standard libs:

// ROOT libs:
#include <TKey.h>

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileEventsPev)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileEventsPev::MFileEventsPev(MString FileName, unsigned int Way) 
  : MFileEvents(FileName, Way)
{
  // Construct an instance of MFileEventsPev

  m_Mode = c_Root;

  Open(FileName, Way);
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsPev::~MFileEventsPev()
{
  // Delete this instance of MFileEventsPev
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MFileEventsPev::GetNextEvent()
{
  // Return the next event... or 0 if it is the last one
  // So remember to test for more events!
  if (UpdateProgress() == false) return 0;

  TKey *Key = (TKey *) m_FileRootIter->Next();
  if (Key == 0) {
    ShowProgress(false);
    return 0;
  }
  return (MPhysicalEvent *) Key->ReadObj(); 
}


////////////////////////////////////////////////////////////////////////////////


void MFileEventsPev::AddEvent(MPhysicalEvent* Pev)
{
  char Text[10];

  sprintf(Text, "PEV%i", m_NEvents++);
  Pev->Write(Text, TObject::kOverwrite);
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsPev::OpenRoot(unsigned int Way)
{
  // Open the file as a Root file

  if (CheckFileExtension("pev") == false) {
    Error("MFileEventsPev::OpenRoot", "The file \"%s\" is no \"physical events\"-file!", 
              m_FileName.Data());
    return false;
  }
  
  return MFileEvents::OpenRoot(Way);
}


// MFileEventsPev.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
