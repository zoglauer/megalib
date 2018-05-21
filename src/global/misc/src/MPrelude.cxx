/*
 * MPrelude.cxx
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
// MPrelude
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPrelude.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"
#include "MGUIPrelude.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPrelude)
#endif


////////////////////////////////////////////////////////////////////////////////


MPrelude::MPrelude()
{
  // Construct an instance of MPrelude
  
}


////////////////////////////////////////////////////////////////////////////////


MPrelude::~MPrelude()
{
  // Delete this instance of MPrelude
}


////////////////////////////////////////////////////////////////////////////////


bool MPrelude::Play()
{
  // First load the .megalib.cfg file
  m_Settings.Read();

  // Step 1: Take care of the license file
  MString LicenseFile = "$(MEGALIB)/doc/License.txt";
  MFile::ExpandFileName(LicenseFile);
  if (MFile::Exists(LicenseFile) == true) {
    ifstream in;
    in.open(LicenseFile);
    MString License;
    License.Read(in);
    if (License.GetHash() != m_Settings.GetLicenseHash()) {
      MGUIPrelude* P = new MGUIPrelude("License agreement", 
                                       "Please read the following license agreement very carefully\n"
                                       "You have to accept the license agreement to use the software", 
                                       License, "Accept", "Decline");
      P->Create();
      if (P->IsOKed() == false) return false;
      delete P;     
      
      m_Settings.SetLicenseHash(License.GetHash());
    }
  }

  // Step 2: Take care of the change log file
  MString ChangeLogFile = "$(MEGALIB)/doc/ChangeLog.txt";
  MFile::ExpandFileName(ChangeLogFile);
  if (MFile::Exists(ChangeLogFile) == true) {
    ifstream in;
    in.open(ChangeLogFile);
    MString ChangeLog;
    ChangeLog.Read(in);
    if (ChangeLog.GetHash() != m_Settings.GetChangeLogHash()) {
      MGUIPrelude* P = new MGUIPrelude("Change Log", 
                                       "Please read the following change log file very carefully\n"
                                       "It contains information about enhancements, bugs, or changes required to run MEGAlib.", 
                                       ChangeLog, "OK");
      P->Create();
      delete P;     

      m_Settings.SetChangeLogHash(ChangeLog.GetHash());
    }
  }

  // Finally save the global configuration file again
  m_Settings.Write();
  
  return true;
}



// MPrelude.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
