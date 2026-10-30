/*
 * MRawEventAnalyzerMultiThreaded.h
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


#ifndef __MRawEventAnalyzerMultiThreaded__
#define __MRawEventAnalyzerMultiThreaded__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <MString.h>
#include <TThread.h>

// Standard libs::
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEventsEvta.h"
#include "MFileEventsTra.h"
#include "MGeometryRevan.h"
#include "MRawEventAnalyzer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


void SplitCalculation(void *adr);


////////////////////////////////////////////////////////////////////////////////


class MRawEventAnalyzerMultiThreaded
{
  // public interface:
 public:
  MRawEventAnalyzerMultiThreaded(MString Filename, MString FilenameOut, MGeometryRevan* Geometry, unsigned int NJobs);
  virtual ~MRawEventAnalyzerMultiThreaded();
  
  MRawEventAnalyzer* GetAnalyzer(unsigned int i);
  void AnalyzeAllEvents();

  bool PreAnalysis();
  bool PostAnalysis();

  void* ThreadedCalculation(unsigned int ID);

  // protected methods:
 protected:
  //MRawEventAnalyzerMultiThreaded() {};
  //MRawEventAnalyzerMultiThreaded(const MRawEventAnalyzerMultiThreaded& RawEventAnalyzerMultiThreaded) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  vector<MRawEventAnalyzer*> m_Analyzers;

  MFileEventsEvta* m_FileIn;
  MString m_FileNameIn;
  MFileEventsTra* m_FileOut;
  MString m_FileNameOut;
  MGeometryRevan* m_Geometry;

  unsigned int m_NJobs;
  vector<TThread*> m_Threads;
  vector<int> m_ThreadStatus;


#ifdef ___CLING___
 public:
  ClassDef(MRawEventAnalyzerMultiThreaded, 0) // no description
#endif

};


////////////////////////////////////////////////////////////////////////////////


class MRawEventAnalyzerMultiThreadedContainer
{
 public:
  MRawEventAnalyzerMultiThreadedContainer(unsigned int ID, MRawEventAnalyzerMultiThreaded* Class) {
    m_ID = ID; m_Class = Class;
  }

  unsigned int GetID() { return m_ID; }
  MRawEventAnalyzerMultiThreaded* GetClass() { return m_Class; }

  unsigned int m_ID;
  MRawEventAnalyzerMultiThreaded* m_Class;
};

#endif


////////////////////////////////////////////////////////////////////////////////
