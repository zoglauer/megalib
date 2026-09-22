/*
 * MFileManager.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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


#ifdef ___CLING___
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
