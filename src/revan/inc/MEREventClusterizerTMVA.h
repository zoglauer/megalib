/*
 * MEREventClusterizerTMVA.h
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
