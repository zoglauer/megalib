/*
 * MNeuralNetworkBackpropagation.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
