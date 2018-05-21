/*
 * MFileInfo.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileInfo__
#define __MFileInfo__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileInfo
{
  // public interface:
 public:
  MFileInfo();
  virtual ~MFileInfo();

  void SetFileName(MString Filename);
  MString GetFileName();

  void SetCreator(MString Creator);
  MString GetCreator();

  void SetIsotopes(MString Isotopes);
  MString GetIsotopes();

  void SetComment(MString Comment);
  MString GetComment();

  void SetPosition(MVector Position);
  MVector GetPosition();

  MString ToString();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_FileName;

  MString m_Creator;
  MString m_Isotopes;
  MString m_Comment;

  MVector m_Position;

#ifdef ___CLING___
 public:
  ClassDef(MFileInfo, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
