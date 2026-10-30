/*
 * MBackpropagationOutputNeuron.h
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


#ifndef __MBackpropagationOutputNeuron__
#define __MBackpropagationOutputNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MBackpropagationNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationOutputNeuron : public MBackpropagationNeuron
{
  // public interface:
public:
  //! Standard constructor
  MBackpropagationOutputNeuron();
  //! Copy constructor
  MBackpropagationOutputNeuron(const MBackpropagationOutputNeuron& N);
  //! Standard constructor
  MBackpropagationOutputNeuron(double LearningRate, double Momentum, int NValues = 1, int NErrors = 1);
  
  //! Set all parameters
  virtual void Set(double LearningRate, double Momentum, int NValues = 1, int NErrors = 1);
  
  //! Learn
  virtual void Learn(int Mode = 0);
  //! Compute the error
  virtual double ComputeError(int Mode = 0);
  
  //! Stream from the file
  virtual bool Stream(MFile& S, const int Version, const bool Read);
  //! Parse an individual line
  virtual int ParseLine(MString Line);
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  // protected methods:
protected:
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! The learning rate
  double m_LearningRate;
  
  
  #ifdef ___CLING___
public:
  ClassDef(MBackpropagationOutputNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
