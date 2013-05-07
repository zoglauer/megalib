/*
 * MSettingsInterface.cxx
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
// MSettingsInterface.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsInterface.h"

// Standard libs:

// ROOT libs:

// MEGAlib:
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsInterface)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsInterface::MSettingsInterface() : m_ModificationLevel(0)
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MSettingsInterface::~MSettingsInterface()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MSettingsInterface::Modify(int m)
{
  // Modify the modification level

  if (m_ModificationLevel < m) {
    m_ModificationLevel = m;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MSettingsInterface::GetModificationLevel(bool Reset) 
{
  // Return the modification level of the file

  int Level = m_ModificationLevel;
  if (Reset == true) {
    m_ModificationLevel = 0;
  }
    
  return Level;
}


////////////////////////////////////////////////////////////////////////////////


MString MSettingsInterface::CleanPath(MString Path)
{
  //! Clean the path, i.e. exchange absolute with relative path $(MEGALIB)
  
  MString ToBeReplaced = "$(MEGALIB)";
  
  if (ToBeReplaced == "" || ToBeReplaced == "/") return Path;

  MFile::ExpandFileName(ToBeReplaced);
  
  MString ToBeReplacedTrunk = ToBeReplaced;
  ToBeReplacedTrunk += "_trunk/";
  ToBeReplaced += "/";

  // First replace the extended version
  Path = Path.ReplaceAll(ToBeReplacedTrunk, "$(MEGALIB)/");
  Path = Path.ReplaceAll(ToBeReplaced, "$(MEGALIB)/");

  return Path;  
}


// MSettingsInterface.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
