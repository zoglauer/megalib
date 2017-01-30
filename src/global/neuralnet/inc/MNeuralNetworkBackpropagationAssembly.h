/*
 * MNeuralNetworkBackpropagationAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNeuralNetworkBackpropagationAssembly__
#define __MNeuralNetworkBackpropagationAssembly__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"
#include "MInputNeuron.h"
#include "MNeuralNetworkBackpropagation.h"
#include "MBackpropagationMiddleNeuron.h"
#include "MBackpropagationOutputNeuron.h"
#include "MBackpropagationSynapse.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Internally this class has several MNeuralNetworkBackpropagation networks
class MNeuralNetworkBackpropagationAssembly : public MNeuralNetworkBackpropagation
{
  // public interface:
public:
  //! Default constructor 
  MNeuralNetworkBackpropagationAssembly();
  //! Copy constructor 
  MNeuralNetworkBackpropagationAssembly(const MNeuralNetworkBackpropagationAssembly& NN);
  //! Destructor
  virtual ~MNeuralNetworkBackpropagationAssembly();
  
  //! Set the number of networks
  void SetNNeuralNetworks(unsigned int N) { m_NNeuralNetworks = N; }
  
  //! Create the neural network layout -- needs to be overwritten
  virtual bool Create();
  
  
  //! Make sure the base class "bool SetInput(MNeuralNetworkIO& Store);"
  using MNeuralNetwork::SetInput;
  //! Set the input of one specific input node for all networks
  virtual bool SetInput(unsigned int i, double Value);
  
  //! Run, i.e. create the output
  virtual bool Run();
  
  //! Return the output of one specific node (numbering starts with zero)
  double GetOutput(unsigned int i);
  //! Return the output node with the smallest output value
  virtual unsigned int GetOutputNodeWithSmallestValue();
  
  //! Set the output of one specific input node for learning for all networks
  bool SetOutput(unsigned int i, double Value);
  
  //! Make sure the base class "bool SetOutputError(MNeuralNetworkIO& Store);"
  using MNeuralNetwork::SetOutputError;
  //! Set the output error of one specific node for all networks
  bool SetOutputError(unsigned int i, double Value);
  
  //! Do the learning --- we will not decide if the learning was sufficient!
  virtual bool Learn();

  
  //! Read/Write the data to file
  virtual bool Stream(const bool Read);
  
  //! Return an IO store
  virtual MNeuralNetworkIO GetIOStore();
  
  
  // protected methods:
protected:
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! The number of neural networks
  unsigned int m_NNeuralNetworks;
  
  //! The neural networks
  vector<MNeuralNetworkBackpropagation*> m_NeuralNetworks;

  
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetworkBackpropagationAssembly, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
