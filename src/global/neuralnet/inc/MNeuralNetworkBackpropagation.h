/*
 * MNeuralNetworkBackpropagation.h
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


#ifndef __MNeuralNetworkBackpropagation__
#define __MNeuralNetworkBackpropagation__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"
#include "MInputNeuron.h"
#include "MNeuralNetwork.h"
#include "MBackpropagationMiddleNeuron.h"
#include "MBackpropagationOutputNeuron.h"
#include "MBackpropagationSynapse.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNeuralNetworkBackpropagation : public MNeuralNetwork
{
  // public interface:
public:
  //! Default constructor 
  MNeuralNetworkBackpropagation();
  //! Copy constructor 
  MNeuralNetworkBackpropagation(const MNeuralNetworkBackpropagation& NN);
  //! Destructor
  virtual ~MNeuralNetworkBackpropagation();
  
  //! Set the learning rate
  void SetLearningRate(double LearningRate);
  //! Set the momentum
  void SetMomentum(double Momentum);
  
  //! Create the neural network layout
  virtual bool Create();
  
  //! Read/Write the data to file
  virtual bool Stream(const bool Read);
  
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  
  // protected methods:
protected:
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  //! Learning rate
  double m_LearningRate;
  //! The momentum
  double m_Momentum;
  
  
  // private members:
private:
  
  
  #ifdef ___CLING___
public:
  ClassDef(MNeuralNetworkBackpropagation, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
