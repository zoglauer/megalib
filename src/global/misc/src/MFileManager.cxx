/*
 * MFileManager.cxx
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
// MFileManager
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileManager.h"

// Standard libs:

// ROOT libs:
#include <TGFileDialog.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileManager)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileManager::MFileManager()
{
  // Construct an instance of MFileManager
}


////////////////////////////////////////////////////////////////////////////////


MFileManager::~MFileManager()
{
  // Delete this instance of MFileManager
}


////////////////////////////////////////////////////////////////////////////////


bool MFileManager::SelectFileToLoad(MString& FileName, const char** FileTypes)
{
  //

  TGFileInfo Info;
  Info.fFilename = StrDup(gSystem->BaseName(FileName));
  Info.fIniDir = StrDup(gSystem->DirName(FileName));
  Info.fFileTypes = FileTypes;
  
  new TGFileDialog(gClient->GetRoot(), gClient->GetRoot(), kFDOpen, &Info);

  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    FileName = MString((char *) Info.fFilename);
    //mlog<<"New Name: "<<FileName<<endl;
    if (FileName.IsEmpty()) {
      return false;
    }
  } 
  // ... or return when cancel has been pressed
  else {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileManager::SelectFileToSave(MString& FileName, const char** FileTypes)
{
  //

  TGFileInfo Info;
  Info.fFilename = StrDup(gSystem->BaseName(FileName));
  Info.fIniDir = StrDup(gSystem->DirName(FileName));
  Info.fFileTypes = FileTypes;
  new TGFileDialog(gClient->GetRoot(), gClient->GetRoot(), kFDSave, &Info);

  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    FileName = MString((char *) Info.fFilename);
    if (FileName.IsEmpty()) {
      return false;
    }
  } 
  // ... or return when cancel has been pressed
  else {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileManager::SearchFile()
{
  // 

  Fatal("Not implemented", "Not implemented");

  return "";
}


////////////////////////////////////////////////////////////////////////////////


int MFileManager::ErrorCode()
{
  //

  return m_ErrorCode;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileManager::FileExists(MString FileName)
{
  // Return true if the file exists in the current directory (selected in the dialog)

  MFile::ExpandFileName(FileName);

  if (FileName == gSystem->DirName((char *) FileName.Data())) {
    return false;
  }

  FILE *File;
  if ((File = fopen((char *) FileName.Data(), "r")) == NULL) {
    return false;
  }

  fclose(File);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MFileManager::DirectoryExists(MString Filename)
{
  // Return true if the directory exists

  MFile::ExpandFileName(Filename);

  if (Filename == gSystem->DirName((char *) Filename.Data())) {
    return false;
  }

  if (gSystem->OpenDirectory(Filename) == NULL) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileManager::MakeAbsolutePath(MString FileName)
{
  MFile::ExpandFileName(FileName);

  return FileName;
}


// MFileManager.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
