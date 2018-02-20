/*
 * MResponseEventClusterizerTMVAEventFile.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEventClusterizerTMVAEventFile__
#define __MResponseEventClusterizerTMVAEventFile__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MEREventClusterizerDataSet.h"
#include "MResponseBuilder.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEventClusterizerTMVAEventFile : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseEventClusterizerTMVAEventFile();
  //! Default destructor
  virtual ~MResponseEventClusterizerTMVAEventFile();
  
  //! Return a brief description of this response class
  static MString Description();
  //! Return information on the parsable options for this response class
  static MString Options();
  //! Parse the options
  virtual bool ParseOptions(const MString& Options);
  
  //! Initialize the response matrices and their generation
  virtual bool Initialize();
  
  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
  
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();
  
  // protected methods:
 protected:
   
  //! Save the response matrices
  virtual bool Save();

  //! Store one element
  bool Store(unsigned int SequenceLength, bool IsGood);
  
  

  // private methods:
 private:



  // protected members:
 protected:
  // The data sets, one per number of x and y hit strips
  vector<MEREventClusterizerDataSet*> m_DataSets;
  
  //! The data storage for the events, one per number of x and y hit strips
  vector<TTree*> m_Trees;
   
  //! The maximum allowed number of hits
  unsigned int m_MaxNHits;
  //! The maximum allowed number of groups
  unsigned int m_MaxNGroups;
  
  
  
#ifdef ___CINT___
 public:
  ClassDef(MResponseEventClusterizerTMVAEventFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
