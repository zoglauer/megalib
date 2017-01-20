/*
 * MNeuralNetwork.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNeuralNetwork__
#define __MNeuralNetwork__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MTokenizer.h"
#include "MNeuralNetworkIOStore.h"

// Forward declarations:
class MNeuron;
class MSynapse;

////////////////////////////////////////////////////////////////////////////////


class MNeuralNetwork : public MParser
{
  // public interface:
public:
  //! Default constructor
  MNeuralNetwork();
  //! Copy constructor - it does not create the actual network -> that's the job of the derived classes copy constructor
  //! But it needs to be called by the derived class
  MNeuralNetwork(const MNeuralNetwork& NN);
  //! Default destructor
  virtual ~MNeuralNetwork();
  
  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);
  
  //! Stream the content -- needs to be overwritten
  virtual bool Stream(const bool Read) { return false; }
  
  
  // Lay out the neural network
  
  //! Set the input nodes
  void SetNInputNodes(unsigned int N) { m_NInputNodes = N; }
  //! Return the number of input nodes 
  unsigned int GetNInputNodes() const { return m_NInputNodes; }
  
  //! Set the nodes of the hidden middle layer
  void SetNMiddleNodes(unsigned int N) { m_NMiddleNodes = N; }
  //! Return the number of middle nodes 
  unsigned int GetNMiddleNodes() const { return m_NMiddleNodes; }
  
  //! Set the output nodes
  void SetNOutputNodes(unsigned int N) { m_NOutputNodes = N; }
  //! Return the number of output nodes 
  unsigned int GetNOutputNodes() const { return m_NOutputNodes; }
  
  
  //! Create the neural network layout -- needs to be overwritten
  virtual bool Create() { return false; }
  //! Return if the neural network has been created / setup
  bool IsCreated() { return m_IsCreated; }  
  
  
  //! Set the input via pre-stored values
  bool SetInput(MNeuralNetworkIOStore& Store);
  //! Set the input of one specific input node (numbering starts with zero)
  virtual bool SetInput(unsigned int i, double Value);
  
  
  //! Run, i.e. create the output
  virtual bool Run();
  
  //! Return the output of one specific node (numbering starts with zero)
  virtual double GetOutput(unsigned int i);
  //! Return the output node with the smallest output value
  virtual unsigned int GetOutputNodeWithSmallestValue();
  
  //! Set the output of one specific input node for learning (numbering starts with zero)
  virtual bool SetOutput(unsigned int i, double Value);
  // No by store ???
  
  //! Set the output error via pre-stored values
  bool SetOutputError(MNeuralNetworkIOStore& Store);
  //! Set the output error of one specific node (numbering starts with zero)
  virtual bool SetOutputError(unsigned int i, double Value);
  
  //! Do the learning --- we will not decide if the learning was sufficient!
  virtual bool Learn();
  
  
  
  // The user can set several comment
  
  //! Set the number of comments
  void SetNUserComments(unsigned int i) { m_UserComments.resize(i); }
  //! Get the number of comments
  unsigned int SetNUserComments() const { return m_UserComments.size(); }
  //! Set the user comment at position i
  void SetUserComment(unsigned int i, MString UserComment) { m_UserComments[i] = UserComment; }
  //! Get the user comment at position i
  MString GetUserComment(unsigned int i) const { return m_UserComments[i]; }
  
  
  //! Save the content to the file
  bool Save(MString FileName);
  //! Load the content from the file
  bool Load(MString FileName); 
  
  //! Create a string with all the data
  virtual MString ToString() const { return MString(); }
  
  //! Return an IO store
  MNeuralNetworkIOStore GetIOStore();
  
  
  // protected methods:
protected:
  
  //! Restore the links after copy construction or loading
  bool RestoreLinks();
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  //! True if the neural network has been created sucessfully
  bool m_IsCreated;
  
  //! A user comment written to the beginning of the file
  vector<MString> m_UserComments;
  
  //! Number of times the function Learn has been called
  unsigned int m_NLearningRuns;
  
  // Layout
  
  //! The number of input nodes
  unsigned int m_NInputNodes;
  //! The input neurons
  vector<MNeuron*> m_InputNodes;
  //! The number of middle nodes
  unsigned int m_NMiddleNodes;
  //! The hidden middle layer
  vector<MNeuron*> m_MiddleNodes;
  //! The number of output nodes
  unsigned int m_NOutputNodes;
  //! The output layer
  vector<MNeuron*> m_OutputNodes;
  
  //! List of the stored synapses:
  vector<MSynapse*> m_Synapses;  
  
  
  // private members:
private:
  
  
  
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetwork, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
