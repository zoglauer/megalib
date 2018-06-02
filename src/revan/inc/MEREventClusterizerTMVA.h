/*
 * MEREventClusterizerTMVA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventClusterizerTMVA__
#define __MEREventClusterizerTMVA__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnationList.h"
#include "MERCSRTMVAMethods.h"
#include "MEREventClusterizer.h"
#include "MEREventClusterizerDataSet.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Find clusters for hits originating from the same gamma ray 
class MEREventClusterizerTMVA : public MEREventClusterizer
{
  // public interface:
 public:
  //! Default constructor
  MEREventClusterizerTMVA();
  //! Default destructor
  virtual ~MEREventClusterizerTMVA();

  //! Set the file name for TMVA analysis
  virtual bool SetTMVAFileNameAndMethod(MString TMVAFileName, MERCSRTMVAMethods Methods);  
                             
  //! Do the analysis - this will add events to the whole incarnation list
  virtual bool Analyze(MRawEventIncarnationList* List);
  
  //! Dump the reconstruction options into a string
  virtual MString ToString(bool CoreOnly = false) const;
  
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The training data file name
  MString m_FileName;

  //! The maximum number of hits we handle
  unsigned int m_MaxNHits;
   
  //! The maximum number of hits we handle
  unsigned int m_MaxNGroups;
   
  //! The energy bin edges
   vector<int> m_EnergyBinEdges;
   
   //! The used TMVA methods
  MERCSRTMVAMethods m_Methods;
  
  //! A string of the used methods for the EvaluateMVA call
  vector<MString> m_MethodNames;
  
  //! The data set - one per sequence length and energy bin
  vector<vector<MEREventClusterizerDataSet*>> m_DS;
  
  //! The TMVA readers - one per sequence length and energy bin
  vector<vector<TMVA::Reader*>> m_Readers;
  
#ifdef ___CLING___
 public:
  ClassDef(MEREventClusterizerTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
