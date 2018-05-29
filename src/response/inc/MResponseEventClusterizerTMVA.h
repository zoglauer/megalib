/*
 * MResponseEventClusterizerTMVA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEventClusterizerTMVA__
#define __MResponseEventClusterizerTMVA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <mutex>
using namespace std;

// ROOT libs:
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"
#include "MERCSRTMVAMethods.h"
#include "MResponseBuilder.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEventClusterizerTMVA : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseEventClusterizerTMVA();
  //! Default destructor
  virtual ~MResponseEventClusterizerTMVA();
  
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
  //! Entry point for the parallel learning thread
  virtual void AnalysisThreadEntry(unsigned int ThreadID);
  
  //! Save the response matrices
  virtual bool Save() { return true; }   

  // private methods:
 private:



  // protected members:
 protected:
   //! The maximum number of hits to handle
   unsigned int m_MaxNHits;
   
   //! The maximum number of groups to handle
   unsigned int m_MaxNGroups;
   
   //! The used TMVA methods
  MERCSRTMVAMethods m_Methods;
  //! The methods string
  MString m_MethodsString;
  
  //! The data file names
  vector<MString> m_FileNames;
  //! The number of hits in the file name
  vector<unsigned int> m_FileHits;
   
  //! Flags indicating the threads are running:
  vector<bool> m_ThreadRunning;
  //! Thread parameter mutex
  mutex m_TheadMutex;
   
#ifdef ___CLING___
 public:
  ClassDef(MResponseEventClusterizerTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
