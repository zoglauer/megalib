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
  
  //! Set the input nodes
  void SetNInputNodes(unsigned int N);
  //! Return the number of input nodes
  unsigned int GetNInputNodes() const { return m_InputNodes.size(); }
  //! Set the nodes of the hidden middle layer
  void SetNMiddleNodes(unsigned int N);
  //! Return the number of middle nodes
  unsigned int GetNMiddleNodes() const { return m_MiddleNodes.size(); }
  //! Set the output nodes
  void SetNOutputNodes(unsigned int N);
  //! Return the number of output nodes
  unsigned int GetNOutputNodes() const { return m_OutputNodes.size(); }
  //! Set the learning rate
  void SetLearningRate(double LearningRate);
  //! Set the momentum
  void SetMomentum(double Momentum);
  
  //! Create the neural network layout
  bool Create();
  
  //! Set the input via pre-stored values
  bool SetInput(MNeuralNetworkIOStore& Store);
  
  //! Set the input of one specific input node (numbering starts with zero)
  bool SetInput(unsigned int i, double Value);
  
  //! Run, i.e. create the output
  bool Run();
  
  //! Return the output of one specific node (numbering starts with zero)
  double GetOutput(unsigned int i);
  
  //! Set the output of one specific input node (numbering starts with zero)
  bool SetOutput(unsigned int i, double Value);
  
  //! Set the output error via per-stored values
  void SetOutputError(MNeuralNetworkIOStore& Store);
  
  //! Set the output error of one specific node (numbering starts with zero)
  void SetOutputError(unsigned int i, double Value);
  
  //! Do the learning --- we will not decide if the learning was sufficient!
  virtual bool Learn();
  
  //! Read/Write the data to file
  virtual bool Stream(const bool Read);
  
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  //! Return the current stored values of the input and outputg nodes...
  virtual MNeuralNetworkIOStore GetIOStore();
  
  
  // protected methods:
protected:
  //MNeuralNetworkBackpropagation() {};
  //MNeuralNetworkBackpropagation(const MNeuralNetworkBackpropagation& NeuralNetworkBackpropagation) {};
  
  //! Restore the links after copy construction or loading
  bool RestoreLinks();
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! The number of input nodes
  unsigned int m_NInputNodes;
  //! The input neurons
  vector<MInputNeuron*> m_InputNodes;
  //! The number of middle nodes
  unsigned int m_NMiddleNodes;
  //! The hidden middle layer
  vector<MBackpropagationMiddleNeuron*> m_MiddleNodes;
  //! The number of output nodes
  unsigned int m_NOutputNodes;
  //! The output layer
  vector<MBackpropagationOutputNeuron*> m_OutputNodes;
  
  //! List of the stored synapses:
  vector<MBackpropagationSynapse*> m_Synapses;
  
  //! Learning rate
  double m_LearningRate;
  //! The momentum
  double m_Momentum;
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetworkBackpropagation, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
