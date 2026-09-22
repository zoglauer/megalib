/*
 * MResponseMultipleComptonTMVA.h
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
  
  //! Set the default options
  static void SetDefaultOptions();
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
  
  //! Selected options for MLP
  MString m_MLPOptions;
  //! Default options for MLP
  static MString m_MLPOptionsDefault;
  //! Selected options for BDTD
  MString m_BDTDOptions;
  //! Default options for BDTD
  static MString m_BDTDOptionsDefault;
  //! Selected options for PDEFoamBoost
  MString m_PDEFoamBoostOptions;
  //! Default options for PDEFoamBoost
  static MString m_PDEFoamBoostOptionsDefault;
  //! Selected options for DNN_CPU
  MString m_DNNCPUOptions;
  //! Default options for DNN_CPU
  static MString m_DNNCPUOptionsDefault;
  //! Selected option for DNN_GPU
  MString m_DNNGPUOptions;
  //! Default option for DNN_GPU
  static MString m_DNNGPUOptionsDefault;
  
  
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
