/*
 * MFileManager.h
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


#ifndef __MFileManager__
#define __MFileManager__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileManager 
{
  // public interface:
 public:
  MFileManager();
  virtual ~MFileManager();

  MString SearchFile();
  bool SelectFileToLoad(MString& DefaultName, const char** FileTypes = 0);
  bool SelectFileToSave(MString& DefaultName, const char** FileTypes = 0);
  int ErrorCode();

  static MString MakeRelativePath(MString FileName, MString Alias) {
  // 

  MFile::ExpandFileName(FileName);

  // First get the real path of the alias
  MString AliasPath = Alias;
  MFile::ExpandFileName(AliasPath);

  // Then compare if FileName starts with AliasPath
  if (FileName.BeginsWith(AliasPath) == true) {
    // If yes replace AliasPath with Alias and return
    FileName.ReplaceAll(AliasPath, Alias);
  }
    
  // Otherwise return the original path
  return FileName;
};

  static MString MakeAbsolutePath(MString FileName);
  static bool FileExists(MString Filename);
  static bool DirectoryExists(MString Filename);

  enum Errors { NoError, Error };

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  int m_ErrorCode;


#ifdef ___CLING___
 public:
  ClassDef(MFileManager, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
