/*
 * MResponseMultipleComptonTMVA.h
 *
 * Copyright (C) 2004-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMultipleComptonTMVA__
#define __MResponseMultipleComptonTMVA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <mutex>
using namespace std;

// ROOT libs:
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMultipleComptonTMVA : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleComptonTMVA();
  //! Default destructor
  virtual ~MResponseMultipleComptonTMVA();
  
  //! Initialize the response matrices and their generation
  virtual bool Initialize();
  
  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
  
  // protected methods:
 protected:
   //! Entry point for the deconvolution thread
   virtual void AnalysisThreadEntry(unsigned int ThreadID);
   

  // private methods:
 private:



  // protected members:
 protected:
   //! The used methods
   map<MString, int> m_Methods;  

   //! The good file names
   vector<MString> m_GoodFileNames;
   //! The bad file names
   vector<MString> m_BadFileNames;
   
   //! Flags indicating the threads are running:
   vector<bool> m_ThreadRunning;
   //! Thread parameter mutex
   mutex m_TheadMutex;
   
#ifdef ___CINT___
 public:
  ClassDef(MResponseMultipleComptonTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
