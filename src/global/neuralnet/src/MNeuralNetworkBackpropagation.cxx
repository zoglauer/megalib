/*
 * MNeuralNetworkBackpropagation.cxx
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
// MNeuralNetworkBackpropagation
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuralNetworkBackpropagation.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNeuralNetworkBackpropagation)
#endif


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagation::MNeuralNetworkBackpropagation() : MNeuralNetwork()
{
  // Construct an instance of MNeuralNetworkBackpropagation
  
  m_NInputNodes = 0;
  m_NMiddleNodes = 0;
  m_NOutputNodes = 0;
  m_LearningRate = 0.4;
  m_Momentum = 0.4;
}

////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagation::MNeuralNetworkBackpropagation(const MNeuralNetworkBackpropagation& NN)
{
  m_NInputNodes = NN.m_NInputNodes;
  m_NMiddleNodes = NN.m_NMiddleNodes;
  m_NOutputNodes = NN.m_NOutputNodes;
  m_LearningRate = NN.m_LearningRate;
  m_Momentum = NN.m_Momentum;
  
  m_InputNodes.clear();
  for (unsigned int i = 0; i < m_NInputNodes; ++i) {
    MInputNeuron* N = new MInputNeuron(*(NN.m_InputNodes[i]));
    m_InputNodes.push_back(N);
  }
  
  m_MiddleNodes.clear();
  for (unsigned int i = 0; i < m_NMiddleNodes; ++i) {
    MBackpropagationMiddleNeuron* N = new MBackpropagationMiddleNeuron(*(NN.m_MiddleNodes[i]));
    m_MiddleNodes.push_back(N);
  }
  
  m_OutputNodes.clear();
  for (unsigned int i = 0; i < m_NOutputNodes; ++i) {
    MBackpropagationOutputNeuron* N = new MBackpropagationOutputNeuron(*(NN.m_OutputNodes[i]));
    m_OutputNodes.push_back(N);
  }
  
  m_Synapses.clear();
  for (unsigned int i = 0; i < NN.m_Synapses.size(); ++i) {
    MBackpropagationSynapse* S = new MBackpropagationSynapse(*(NN.m_Synapses[i]));
    m_Synapses.push_back(S);    
  }
  
  RestoreLinks();
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagation::~MNeuralNetworkBackpropagation()
{
  // Delete this instance of MNeuralNetworkBackpropagation
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagation::RestoreLinks()
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


void MNeuralNetworkBackpropagation::SetNInputNodes(unsigned int N)
{
  //! Set the input nodes
  
  m_NInputNodes = N;
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetNMiddleNodes(unsigned int N)
{
  //! Set the nodes of the hidden middle layer
  
  m_NMiddleNodes = N;
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetNOutputNodes(unsigned int N)
{
  //! Set the output nodes
  
  m_NOutputNodes = N;
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetLearningRate(double LearningRate)
{
  //! Set the learning rate
  
  if (LearningRate > 1.0) {
    merr<<"LearningRate needs to be within [0.0, 1.0]!"<<endl;
    merr<<"Setting it to 1.0!"<<show;
    m_LearningRate = 1.0;
  } else if (LearningRate < 0.0) {
    merr<<"LearningRate needs to be within [0.0, 1.0]!"<<endl;
    merr<<"Setting it to 0.0!"<<show;
    m_LearningRate = 0.0;
  } else {
    m_LearningRate = LearningRate;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetMomentum(double Momentum)
{
  //! Set the momentum
  
  if (Momentum > 1.0) {
    merr<<"Momentum needs to be within [0.0, 1.0]!"<<endl;
    merr<<"Setting it to 1.0!"<<show;
    m_Momentum = 1.0;
  } else if (Momentum < 0.0) {
    merr<<"Momentum needs to be within [0.0, 1.0]!"<<endl;
    merr<<"Setting it to 0.0!"<<show;
    m_Momentum = 0.0;
  } else {
    m_Momentum = Momentum;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagation::Create()
{
  //! Create the neural network layout
  
  for (unsigned int i = 0; i < m_NInputNodes; ++i) {
    MInputNeuron* iNeuron = new MInputNeuron();
    m_InputNodes.push_back(iNeuron);
  }
  
  for (unsigned int i = 0; i < m_NMiddleNodes; ++i) {
    MBackpropagationMiddleNeuron* mNeuron = new MBackpropagationMiddleNeuron(m_LearningRate, m_Momentum, 1, 1);
    m_MiddleNodes.push_back(mNeuron);
  }
  
  for (unsigned int i = 0; i < m_NOutputNodes; ++i) {
    MBackpropagationOutputNeuron* aNeuron = new MBackpropagationOutputNeuron(m_LearningRate, m_Momentum, 1, 1);
    m_OutputNodes.push_back(aNeuron);
  }
  
  vector<MBackpropagationSynapse*> Synapses;
  for (unsigned int i = 0; i < m_NInputNodes; ++i) {
    for (unsigned int m = 0; m < m_NMiddleNodes; ++m) {
      MBackpropagationSynapse* bSynapse = new MBackpropagationSynapse();
      bSynapse->Connect(m_InputNodes[i], m_MiddleNodes[m]);
      m_Synapses.push_back(bSynapse);
    }
  }
  
  for (unsigned int m = 0; m < m_NMiddleNodes; ++m) {
    for (unsigned int o = 0; o < m_NOutputNodes; ++o) {
      MBackpropagationSynapse* bSynapse = new MBackpropagationSynapse();
      bSynapse->Connect(m_MiddleNodes[m], m_OutputNodes[o]);
      m_Synapses.push_back(bSynapse);
    }
  }
  
  m_IsCreated = true;
  
  return m_IsCreated;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagation::SetInput(unsigned int i, double Value)
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


bool MNeuralNetworkBackpropagation::SetInput(MNeuralNetworkIOStore& Store)
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


bool MNeuralNetworkBackpropagation::SetOutput(unsigned int i, double Value)
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


bool MNeuralNetworkBackpropagation::Run()
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


double MNeuralNetworkBackpropagation::GetOutput(unsigned int i)
{
  //! Return the output of one specific node (numbering starts with zero)
  
  if (i >= m_NOutputNodes) {
    merr<<"Output node ID out of range: ID="<<i<<", ID-MAX="<<m_NOutputNodes-1<<show;
    return 0.0;
  }
  
  return m_OutputNodes[i]->GetValue();
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetOutputError(unsigned int i, double Value)
{
  //! Return the output of one specific node (numbering starts with zero)
  
  if (i >= m_NOutputNodes) {
    merr<<"Output node ID out of range: ID="<<i<<", ID-MAX="<<m_NOutputNodes-1<<show;
    return;
  }
  
  m_OutputNodes[i]->SetError(Value);
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkBackpropagation::SetOutputError(MNeuralNetworkIOStore& Store)
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


bool MNeuralNetworkBackpropagation::Learn()
{
  //! Do the learning --- we will not decide if the learning was sufficient!
  
  // For bookkeeping call the base class
  MNeuralNetwork::Learn();
  
  for (unsigned int o = m_NOutputNodes-1; o < m_NOutputNodes; --o) {
    m_OutputNodes[o]->Learn();
  }
  for (unsigned int m = m_NMiddleNodes-1; m < m_NMiddleNodes; --m) {
    m_MiddleNodes[m]->Learn();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIOStore MNeuralNetworkBackpropagation::GetIOStore()
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


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagation::Stream(const bool Read)
{
  // Hopefully a faster way to stream data from and to a file than ROOT...
  
  bool Return = true;
  
  if (Read == true) {
    // Ret  0, if the line got correctly parsed
    // Ret  1, if the line got not correctly parsed
    // Ret  2, if the line got not parsed
    // Ret -1, if the end of event has been reached
    
    int Ret;
    MString Line;
    while (IsGood() == true) {
      ReadLine(Line);
      if (Line.Length() < 2) continue;
      
      if (Line[0] == 'N' && Line[1] == 'I') {
        int Number;
        if (sscanf(Line, "NI %d", &Number) == 1) {
          //m_InputNeurons.resize(Number);
          // Loop over input neurons:
          while (IsGood()) {
            ReadLine(Line);
            if (Line.Length() < 2) continue;
            
            //cout<<"Line: \""<<Line<<"\""<<endl;
            if (MString(Line) == "NE") {
              //cout<<"New neuron"<<endl;
              MInputNeuron* N = new MInputNeuron();
              if (N->Stream(*this, m_Version, Read) == false) {
                return false;
              }
              m_InputNodes.push_back(N);
            } else if (MString(Line) == "NI EN") {
              break;
            }
          }
        } else {
          mout<<"Unable to parse NI!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'N' && Line[1] == 'M') {
        int Number;
        if (sscanf(Line, "NM %d", &Number) == 1) {
          //m_MiddleNeurons.resize(Number);
          while (IsGood()) {
            ReadLine(Line);
            if (Line.Length() < 2) continue;
            
            if (MString(Line) == "NE") {
              MBackpropagationMiddleNeuron* N = new MBackpropagationMiddleNeuron();
              if (N->Stream(*this, m_Version, Read) == false) {
                return false;
              }
              m_MiddleNodes.push_back(N);
            } else if (MString(Line) == "NM EN") {
              break;
            }
          }
        } else {
          mout<<"Unable to parse NM!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'N' && Line[1] == 'O') {
        int Number;
        if (Line == "NO NE") {
          // No more output neurons...
        } else if (sscanf(Line, "NO %d", &Number) == 1) {
          //m_OutputNeurons.resize(Number);
          while (IsGood()) {
            ReadLine(Line);
            if (Line.Length() < 2) continue;
            
            if (MString(Line) == "NE") {
              MBackpropagationOutputNeuron* N = new MBackpropagationOutputNeuron();
              if (N->Stream(*this, m_Version, Read) == false) {
                return false;
              }
              m_OutputNodes.push_back(N);
            } else if (MString(Line) == "NO EN") {
              break;
            }
          }
        } else {
          mout<<"Unable to parse NO!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'S' && Line[1] == 'Y') {
        MBackpropagationSynapse* Sy = new MBackpropagationSynapse();
        if (Sy->Stream(*this, m_Version, Read) == false) {
          return false;
        }
        m_Synapses.push_back(Sy);        
      } else if (Line[0] == 'L' && Line[1] == 'R') {
        if (sscanf(Line, "LR %lf", &m_LearningRate) != 1) {
          mout<<"Unable to parse LR!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'M' && Line[1] == 'M') {
        if (sscanf(Line, "MM %lf", &m_Momentum) != 1) {
          mout<<"Unable to parse MM!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'E' && Line[1] == 'N') {
        Ret = -1;
        break;
      }
    }
    
    if (Ret != 1) {
      if (RestoreLinks() == false) {
        Ret = 1;
      }
    }
    
    
  } else {
    
    ostringstream S;
    
    WriteLine("Type RSP");
    WriteLine("Version 1");
    WriteLine();
    
    WriteLine(MString("# Number of times the function \"Learn()\" has been called: ") + m_NLearningRuns);
    WriteLine("\n");
    
    for (unsigned int i = 0; i < m_UserComments.size(); ++i) {
      WriteLine(m_UserComments[i]);
    }
    WriteLine("\n");
    
    WriteLine(MString("NI ") + m_NInputNodes);
    for (unsigned int i = 0; i < m_InputNodes.size(); ++i) {
      m_InputNodes[i]->Stream(*this, m_Version, Read);
      WriteLine("NE EN");
    }
    WriteLine("NI EN");
    
    WriteLine(MString("NM ") + m_NMiddleNodes);
    for (unsigned int i = 0; i < m_MiddleNodes.size(); ++i) {
      m_MiddleNodes[i]->Stream(*this, m_Version, Read);
      WriteLine("NE EN");
    }
    WriteLine("NM EN");
    
    WriteLine(MString("NO ") + m_NOutputNodes);
    for (unsigned int i = 0; i < m_OutputNodes.size(); ++i) {
      m_OutputNodes[i]->Stream(*this, m_Version, Read);
      WriteLine("NE EN");
    }
    WriteLine("NO EN");
    
    for (unsigned int i = 0; i < m_Synapses.size(); ++i) {
      m_Synapses[i]->Stream(*this, m_Version, Read);
      WriteLine("SY EN");
    }
    
    WriteLine(MString("LR ") + m_LearningRate);
    WriteLine(MString("MM ") + m_Momentum);
    WriteLine("EN");
    
    Flush();    
  }
  
  //cout<<"Return code: "<<Return<<endl;
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string 
MString MNeuralNetworkBackpropagation::ToString() const
{
  ostringstream S;
  
  S<<"Type RSP"<<endl;
  S<<"Version 1"<<endl;
  S<<endl;
  
  S<<"# Number of times the function \"Learn()\" has been called: "<<m_NLearningRuns<<endl;
  S<<endl;
  
  for (unsigned int i = 0; i < m_UserComments.size(); ++i) {
    S<<m_UserComments[i]<<endl;
  }
  S<<endl;
  
  S<<"NI "<<m_NInputNodes<<endl;
  for (unsigned int i = 0; i < m_InputNodes.size(); ++i) {
    S<<m_InputNodes[i]->ToString();
    S<<"NE EN"<<endl;
  }
  S<<"NI EN"<<endl;
  
  S<<"NM "<<m_NMiddleNodes<<endl;
  for (unsigned int i = 0; i < m_MiddleNodes.size(); ++i) {
    S<<m_MiddleNodes[i]->ToString();
    S<<"NE EN"<<endl;
  }
  S<<"NM EN"<<endl;
  
  S<<"NO "<<m_NOutputNodes<<endl;
  for (unsigned int i = 0; i < m_OutputNodes.size(); ++i) {
    S<<m_OutputNodes[i]->ToString();
    S<<"NE EN"<<endl;
  }
  S<<"NO EN"<<endl;
  
  for (unsigned int i = 0; i < m_Synapses.size(); ++i) {
    S<<m_Synapses[i]->ToString();
    S<<"SY EN"<<endl;
  }
  
  S<<"LR "<<m_LearningRate<<endl;
  S<<"MM "<<m_Momentum<<endl;
  S<<"EN"<<endl;
  
  return MString(S);
}


// MNeuralNetworkBackpropagation.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
