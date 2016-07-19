/*
 * MFileManager.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileManager__
#define __MFileManager__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileManager : public TObject
{
  // public interface:
 public:
  MFileManager();
  virtual ~MFileManager();

  MString SearchFile();
  bool SelectFileToLoad(MString& DefaultName, const char** FileTypes = 0);
  bool SelectFileToSave(MString& DefaultName, const char** FileTypes = 0);
  int ErrorCode();

  static MString MakeRelativePath(MString FileName, MString Alias)

/* //////////////////////////////////////////////////////////////////////////////// */


/* MString MFileManager::MakeRelativePath(MString FileName, MString Alias) */
{
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


#ifdef ___CINT___
 public:
  ClassDef(MFileManager, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
