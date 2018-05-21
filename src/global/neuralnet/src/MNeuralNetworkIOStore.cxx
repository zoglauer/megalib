/*
 * MNeuralNetworkIOStore.cxx
 *
 *
 * Copyright (C) 2005-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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
