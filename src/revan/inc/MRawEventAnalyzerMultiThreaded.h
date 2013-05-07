/*
 * MRawEventAnalyzerMultiThreaded.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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


#ifdef ___CINT___
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
