/*
 * MSimEventLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimEventLoader__
#define __MSimEventLoader__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSimEvent.h"
#include "MDGeometryQuest.h"

// Standard libs:
#include <iostream>
#include <fstream>
#include <stdlib.h>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSimEventLoader : public TObject
{
  // public interface:
 public:
  MSimEventLoader();
  MSimEventLoader(MString FileName);
  ~MSimEventLoader();

  void Init();

  void SetFileName(MString FileName);
  void SetGeometry(MDGeometryQuest *Geo);

  MSimEvent* GetNextEvent();
  bool IsFileOpen();

  double GetProgress();


  // protected methods:
 protected:
  bool OpenFile();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MString m_FileName;            // Name of the file

  bool m_IsFileOpen;           // true if the file is open
  fstream *m_FileStream;         // pointer to the filestream
  int m_FileLength;

  int m_LineLength;            // length of the line-buffer
  char *m_LineBuffer;          // buffer for one event as ascii-text

  MSimEvent* m_Event;

  MDGeometryQuest *m_Geometry;


#ifdef ___CINT___
 public:
  ClassDef(MSimEventLoader, 0) // loads simulated events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
