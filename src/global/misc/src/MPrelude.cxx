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


#ifdef ___CINT___
ClassImp(MPrelude)
#endif


////////////////////////////////////////////////////////////////////////////////


MPrelude::MPrelude()
{
  // Construct an instance of MPrelude
  
  m_Data = 0;
  m_ChangeLogHash = 0;
  m_LicenseHash = 0;
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
  
  Load();

  // Step 1: Take care of the license file
  MString LicenseFile = "$(MEGALIB)/doc/License.txt";
  MFile::ExpandFileName(LicenseFile);
  if (MFile::Exists(LicenseFile) == true) {
    ifstream in;
    in.open(LicenseFile);
    MString License;
    License.Read(in);
    if (License.GetHash() != m_LicenseHash) {
      MGUIPrelude* P = new MGUIPrelude("License agreement", 
                                       "Please read the following license agreement very carefully\n"
                                       "You have to accept the license agreement to use the software", 
                                       License, "Accept", "Decline");
      P->Create();
      if (P->IsOKed() == false) return false;
      delete P;     
      
      m_LicenseHash = License.GetHash();
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
    if (ChangeLog.GetHash() != m_ChangeLogHash) {
      MGUIPrelude* P = new MGUIPrelude("Change Log", 
                                       "Please read the following change log file very carefully\n"
                                       "It contains information about enhancements, bugs, or changes required to run MEGAlib.", 
                                       ChangeLog, "OK");
      P->Create();
      delete P;     

      m_ChangeLogHash = ChangeLog.GetHash();
    }
  }

  Save();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MPrelude::Load()
{
  // First load the .megalib.cfg file
  
  m_LicenseHash = 0;  
  m_ChangeLogHash = 0;

  MString FileName(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".megalib.cfg"));
  
  if (MFile::Exists(FileName) == true) {
    MXmlDocument*Data = new MXmlDocument();  
    if (Data->Load(FileName) == true) {;
      if (Data->GetNode("LicenseHash") != 0) {
        m_LicenseHash = Data->GetNode("LicenseHash")->GetValueAsLong();
      }
      if (Data->GetNode("ChangeLogHash") != 0) {
        m_ChangeLogHash = Data->GetNode("ChangeLogHash")->GetValueAsLong();
      }
    }
    delete Data;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MPrelude::Save()
{
  // First load the .megalib.cfg file
  
  MXmlDocument* m_Data = new MXmlDocument("MEGAlib");
  new MXmlNode(m_Data, "LicenseHash", m_LicenseHash);
  new MXmlNode(m_Data, "ChangeLogHash", m_ChangeLogHash);    

  MString FileName(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".megalib.cfg"));
  m_Data->Save(FileName);
  
  delete m_Data;
}


// MPrelude.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
