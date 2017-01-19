/*
 * MNeuralNetwork.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MNeuralNetwork
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuralNetwork.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MNeuron.h"
#include "MSynapse.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNeuralNetwork)
#endif


////////////////////////////////////////////////////////////////////////////////


MNeuralNetwork::MNeuralNetwork() : MParser(' ', false)
{
  // Construct an instance of MNeuralNetwork
  
  m_IsCreated = false;
  
  m_NLearningRuns = 0;
  
  m_NInputNodes = 0;
  m_NMiddleNodes = 0;
  m_NOutputNodes = 0;  
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetwork::MNeuralNetwork(const MNeuralNetwork& NN)
{
  // Copy constructor:
  
  m_IsCreated = false; // Derived CC has to create the network
  m_UserComments = NN.m_UserComments;
  m_NLearningRuns = 0;
  
  m_NInputNodes = NN.m_NInputNodes;
  m_NMiddleNodes = NN.m_NMiddleNodes;
  m_NOutputNodes = NN.m_NOutputNodes;
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetwork::~MNeuralNetwork()
{
  // Delete this instance of MNeuralNetwork
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Open(MString FileName, unsigned int Way)
{
  // Open the file and do the parsing
  
  bool Return = true;
  
  if (MFile::Open(FileName, Way) == false) {
    mlog<<"MNeuralNetwork::Open: Unable to open file "<<FileName<<"."<<endl;
    return false;
  }
  
  if (Way == c_Read) {
    // Read header information:
    MTokenizer T;
    
    // Check the first 100 lines for Version and type: 
    int Lines = 100;
    while (TokenizeLine(T) == true) {
      Lines--;
      if (T.GetNTokens() < 2) continue;
      if (T.GetTokenAt(0) == "Version") {
        m_Version = T.GetTokenAtAsInt(1);
      } else if (T.GetTokenAt(0) == "Type") {
        m_FileType = T.GetTokenAtAsString(1);
      }
      if (Lines == 0) break;
      if (m_Version != c_VersionUnknown && 
        m_FileType != c_TypeUnknown) break;
    }
    if (m_Version == c_VersionUnknown) {
      mout<<"Warning: No version info found in the file \""<<FileName<<"\"!!"<<endl;
    }
    if (m_FileType == c_TypeUnknown) {
      mout<<"Error: No file type info found in the file \""<<FileName<<"\"!!"<<endl;
      Close();
      return false;
    }
    if (m_FileType != "RSP") {
      mout<<"Error: Unknown file type \""<<m_FileType<<"\"!!"<<endl;
      Close();
      return false;
    }
    
    Rewind();
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Save(MString FileName)
{
  bool Return = true;
  
  if (Open(FileName, MFile::c_Create) == true) {
    Return = Stream(false);
  } else {
    Return = false;
  }
  Close();
  
  return Return;
}   


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Load(MString FileName)
{
  bool Return = true;
  
  if (Open(FileName, MFile::c_Read) == true) {
    Return = Stream(true);
  } else {
    Return = false;
  }
  Close();
  
  if (Return == true) {
    m_IsCreated = true;
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::RestoreLinks()
{
  // Setup all linked lists via their stored IDs:
  for (unsigned int s = 0; s < m_Synapses.size(); ++s) {
    MNeuron* In = 0;
    MNeuron* Out = 0;
    for (unsigned int n = 0; n < m_InputNodes.size(); ++n) {
      if (m_InputNodes[n]->GetID() == m_Synapses[s]->GetInNeuronID()) {
        In = m_InputNodes[n];
      }
      if (m_InputNodes[n]->GetID() == m_Synapses[s]->GetOutNeuronID()) {
        Out = m_InputNodes[n];
      }
    }
    for (unsigned int n = 0; n < m_MiddleNodes.size(); ++n) {
      if (m_MiddleNodes[n]->GetID() == m_Synapses[s]->GetInNeuronID()) {
        In = m_MiddleNodes[n];
      }
      if (m_MiddleNodes[n]->GetID() == m_Synapses[s]->GetOutNeuronID()) {
        Out = m_MiddleNodes[n];
      }
    }
    for (unsigned int n = 0; n < m_OutputNodes.size(); ++n) {
      if (m_OutputNodes[n]->GetID() == m_Synapses[s]->GetInNeuronID()) {
        In = m_OutputNodes[n];
      }
      if (m_OutputNodes[n]->GetID() == m_Synapses[s]->GetOutNeuronID()) {
        Out = m_OutputNodes[n];
      }
    }
    if (In == 0 || Out == 0) {
      mout<<"Unable to find input and output neuron for synapse "<<m_Synapses[s]->GetID()<<endl;
      return false;
    } else {
      m_Synapses[s]->Connect(In, Out);
    }
  }
  
  m_NInputNodes = m_InputNodes.size();
  m_NMiddleNodes = m_MiddleNodes.size();
  m_NOutputNodes = m_OutputNodes.size();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::SetInput(unsigned int i, double Value)
{
  //! Set the input of one specific input node (numbering starts with zero)
  
  if (i >= m_NInputNodes) {
    merr<<"Input node ID out of range: ID="<<i<<", Max+1="<<m_NInputNodes<<show;
    return false;
  }
  
  if (Value <= 0 || Value >= 1.0) {
    merr<<"Input node "<<i<<": value out of range: "<<Value<<"! Needs to be ]0..1["<<show;
    return false;
  }
  
  //cout<<i<<": "<<Value<<endl;
  m_InputNodes[i]->SetValue(Value);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::SetInput(MNeuralNetworkIOStore& Store)
{
  // Set the input via pre-stored values
  
  if (Store.GetNInputs() != m_NInputNodes) {
    merr<<"Input node number not equal: Store="<<Store.GetNInputs()<<", NN="<<m_NInputNodes<<show;
    return false;
  }
  
  for (unsigned int i = 0; i < Store.GetNInputs(); ++i) {
    m_InputNodes[i]->SetValue(Store.GetInput(i));
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::SetOutput(unsigned int i, double Value)
{
  //! Set the input of one specific input node (numbering starts with zero)
  
  if (i >= m_NOutputNodes) {
    merr<<"Output node ID out of range: ID="<<i<<", ID-MAX="<<m_NOutputNodes-1<<show;
    return false;
  }
  
  if (Value <= 0 || Value >= 1.0) {
    merr<<"Output node "<<i<<": value out of range: "<<Value<<"! Needs to be ]0..1["<<show;
    return false;
  }
  
  //cout<<i<<": "<<Value<<endl;
  m_OutputNodes[i]->SetValue(Value);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Run()
{
  //! Run, i.e. create the output
  
  for (unsigned int m = 0; m < m_NMiddleNodes; ++m) {
    m_MiddleNodes[m]->Run();
  }
  for (unsigned int o = 0; o < m_NOutputNodes; ++o) {
    m_OutputNodes[o]->Run();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MNeuralNetwork::GetOutput(unsigned int i)
{
  //! Return the output of one specific node (numbering starts with zero)
  
  if (i >= m_NOutputNodes) {
    merr<<"Output node ID out of range: ID="<<i<<", ID-MAX="<<m_NOutputNodes-1<<show;
    return 0.0;
  }
  
  return m_OutputNodes[i]->GetValue();
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetwork::SetOutputError(unsigned int i, double Value)
{
  //! Return the output of one specific node (numbering starts with zero)
  
  if (i >= m_NOutputNodes) {
    merr<<"Output node ID out of range: ID="<<i<<", ID-MAX="<<m_NOutputNodes-1<<show;
    return;
  }
  
  m_OutputNodes[i]->SetError(Value);
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetwork::SetOutputError(MNeuralNetworkIOStore& Store)
{
  // Set the output via pre-stored values
  
  if (Store.GetNOutputs() != m_NOutputNodes) {
    merr<<"Output node number not equal: Store="<<Store.GetNOutputs()<<", NN="<<m_NOutputNodes<<show;
    return;
  }
  
  for (unsigned int i = 0; i < Store.GetNOutputs(); ++i) {
    m_OutputNodes[i]->SetError(Store.GetOutput(i));
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Learn()
{
  //! Do the learning --- we will not decide if the learning was sufficient!
  
  m_NLearningRuns++;
  
  for (unsigned int o = m_NOutputNodes-1; o < m_NOutputNodes; --o) {
    m_OutputNodes[o]->Learn();
  }
  for (unsigned int m = m_NMiddleNodes-1; m < m_NMiddleNodes; --m) {
    m_MiddleNodes[m]->Learn();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIOStore MNeuralNetwork::GetIOStore()
{
  // Return the current stored values of the input and outputg nodes...
  
  MNeuralNetworkIOStore IOStore;
  IOStore.SetNInputs(m_NInputNodes);
  for (unsigned int i = 0; i < m_NInputNodes; ++i) {
    IOStore.SetInput(i, m_InputNodes[i]->GetValue()); 
  }
  IOStore.SetNOutputs(m_NOutputNodes);
  for (unsigned int i = 0; i < m_NOutputNodes; ++i) {
    IOStore.SetOutput(i, m_OutputNodes[i]->GetValue()); 
  }
  
  return IOStore;
}



// MNeuralNetwork.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
