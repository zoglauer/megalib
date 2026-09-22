/*
 * MNeuralNetworkIOStore.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MNeuralNetworkIOStore
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuralNetworkIOStore.h"

// Standard libs:
#include <iomanip>
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNeuralNetworkIOStore)
#endif


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIOStore::MNeuralNetworkIOStore()
{
  // Construct an instance of MNeuralNetworkIOStore
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIOStore::~MNeuralNetworkIOStore()
{
  // Delete this instance of MNeuralNetworkIOStore
}


////////////////////////////////////////////////////////////////////////////////


//! Save the store
bool MNeuralNetworkIOStore::Save(MString FileName)
{
  if (m_IOs.size() == 0) {
    merr<<"IO store is empty - nothing to save."<<endl;
    return false;    
  }
  
  unsigned int NInputs = m_IOs[0].GetNInputs();
  unsigned int NOutputs = m_IOs[0].GetNOutputs();
  
  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    merr<<"Unable to open file "<<FileName<<endl;
    return false;
  }
  
  out<<"# Neural network IO store"<<endl;
  out<<endl;
  out<<"NI "<<NInputs<<endl;
  out<<"NO "<<NOutputs<<endl;
  
  for (unsigned int i = 0; i < m_IOs.size(); ++i) {
    out<<"IO ";
    for (unsigned int n = 0; n < NInputs; ++n) {
      out<<setprecision(12)<<m_IOs[i].GetInput(n)<<" ";
    }
    for (unsigned int n = 0; n < NOutputs; ++n) {
      out<<setprecision(12)<<m_IOs[i].GetOutput(n)<<" ";
    }
    out<<endl;
  }
  
  out.close();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Load the store
bool MNeuralNetworkIOStore::Load(MString FileName)
{
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a neural network IO --- throw an exception otherwise
MNeuralNetworkIO MNeuralNetworkIOStore::Get(unsigned int i) const
{
  if (i >= m_IOs.size()) {
    throw MExceptionIndexOutOfBounds(0, m_IOs.size(), i);
  }
  
  return m_IOs[i];
}


////////////////////////////////////////////////////////////////////////////////


//! Return a neural network IO --- throw an exception otherwise
MNeuralNetworkIO MNeuralNetworkIOStore::GetLast() const
{
  if (m_IOs.size() == 0) {
    throw MExceptionObjectDoesNotExist("The neural network IO store is empty!");
  }
  
  return m_IOs.back();
}


// MNeuralNetworkIOStore.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
