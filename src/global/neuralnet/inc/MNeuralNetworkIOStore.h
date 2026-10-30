/*
 * MNeuralNetworkIOStore.h
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
  
  
  #ifdef ___CLING___
public:
  ClassDef(MNeuralNetworkIOStore, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
