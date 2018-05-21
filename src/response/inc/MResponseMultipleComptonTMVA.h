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
#include "MERCSRTMVAMethods.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMultipleComptonTMVA : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleComptonTMVA();
  //! Default destructor
  virtual ~MResponseMultipleComptonTMVA();
  
  //! Return a brief description of this response class
  static MString Description();
  //! Return information on the parsable options for this response class
  static MString Options();
  //! Parse the options
  virtual bool ParseOptions(const MString& Options);
  
  //! Set TMVA methods striung -- parsing happens in Initialize()
  void SetMethods(MString MethodsString) { m_MethodsString = MethodsString; }
  
  //! Initialize the response matrices and their generation
  virtual bool Initialize();
  
  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
  
  // protected methods:
 protected:
  //! Entry point for the deconvolution thread
  virtual void AnalysisThreadEntry(unsigned int ThreadID);
  
  //! Save the response matrices
  virtual bool Save() { return true; }   

  // private methods:
 private:



  // protected members:
 protected:
  //! The used TMVA methods
  MERCSRTMVAMethods m_Methods;
  //! The methods string
  MString m_MethodsString;
  
  //! The good file names
  vector<MString> m_GoodFileNames;
  //! The bad file names
  vector<MString> m_BadFileNames;
  //! The sequence length
  vector<unsigned int> m_SequenceLengths;
   
  //! Flags indicating the threads are running:
  vector<bool> m_ThreadRunning;
  //! Thread parameter mutex
  mutex m_TheadMutex;
   
#ifdef ___CLING___
 public:
  ClassDef(MResponseMultipleComptonTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
