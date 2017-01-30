/*
 * MNeuralNetworkIOStore.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNeuralNetworkIOStore__
#define __MNeuralNetworkIOStore__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNeuralNetworkIO.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Storage for a set og neural network IO's
class MNeuralNetworkIOStore
{
  // public interface:
public:
  //! Default constructor
  MNeuralNetworkIOStore();
  //! Default destructor
  virtual ~MNeuralNetworkIOStore();
  
  //! Load the store
  bool Load(MString FileName);
  //! Save the store
  bool Save(MString FileName);
  
  //! Return the number of available  neural network IOs
  unsigned int Size() const { return m_IOs.size(); }
  //! Add a neural network IO
  void Add(const MNeuralNetworkIO& IO) { m_IOs.push_back(IO); }
  //! Add a neural network IO
  void RemoveFirst() { if (m_IOs.size() > 0) m_IOs.erase(m_IOs.begin()); }
  //! Return a neural network IO --- throw an exception MExceptionIndexOutOfBounds otherwise
  MNeuralNetworkIO Get(unsigned int i) const;
  //! Return the last neural network IO --- throw an exception MExceptionObjectDoesNotExist otherwise
  MNeuralNetworkIO GetLast() const;
  
  
  
  // protected methods:
protected:
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  vector<MNeuralNetworkIO> m_IOs;
  
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetworkIOStore, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
