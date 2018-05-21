/*
 * MNeuralNetworkBackpropagationAssembly.cxx
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
// MNeuralNetworkBackpropagationAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuralNetworkBackpropagationAssembly.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNeuralNetworkBackpropagationAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagationAssembly::MNeuralNetworkBackpropagationAssembly() : MNeuralNetworkBackpropagation()
{
  // Construct an instance of MNeuralNetworkBackpropagationAssembly
  
  m_NNeuralNetworks = 1;
}

////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagationAssembly::MNeuralNetworkBackpropagationAssembly(const MNeuralNetworkBackpropagationAssembly& NN) : MNeuralNetworkBackpropagation(NN)
{
  // TODO: Completely missing
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkBackpropagationAssembly::~MNeuralNetworkBackpropagationAssembly()
{
  // Delete this instance of MNeuralNetworkBackpropagationAssembly
  
  // TODO: Completely missing
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagationAssembly::Create()
{
  //! Create the neural network layout
  
  m_NeuralNetworks.clear();
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    MNeuralNetworkBackpropagation* N = new MNeuralNetworkBackpropagation();
    N->SetLearningRate(m_LearningRate);
    N->SetMomentum(m_Momentum);
    N->SetNInputNodes(m_NInputNodes);
    N->SetNMiddleNodes(m_NMiddleNodes);
    N->SetNOutputNodes(m_NOutputNodes);
    N->MNeuralNetworkBackpropagation::Create();  
    m_NeuralNetworks.push_back(N);
  }
  
  m_IsCreated = true;
  
  return m_IsCreated;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the input of one specific input node for all networks
bool MNeuralNetworkBackpropagationAssembly::SetInput(unsigned int i, double Value)
{
  bool Return = true;
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    if (m_NeuralNetworks[n]->SetInput(i, Value) == false) {
      Return = false;
    }
  }
  
  return Return;
}
  

////////////////////////////////////////////////////////////////////////////////



//! Run, i.e. create the output
bool MNeuralNetworkBackpropagationAssembly::Run()
{
  bool Return = true;
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    if (m_NeuralNetworks[n]->Run() == false) {
      Return = false;
    }
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////



//! Return the output of one specific node (numbering starts with zero)
double MNeuralNetworkBackpropagationAssembly::GetOutput(unsigned int i)
{
  mout<<"Calling the base class function GetOutput(unsigned int i) does not make sense"<<endl;
  
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuralNetworkBackpropagationAssembly::GetOutputNodeWithSmallestValue()
{
  //! Return the output node with the smallest output value
  
  
  // First create a list of all the nodes with the smallest value in all the networks
  vector<unsigned int> NodeList(m_NNeuralNetworks);
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    NodeList[n] = m_NeuralNetworks[n]->GetOutputNodeWithSmallestValue();
  }
  
  // Then create a list how frequent each output node is named as the smallest one
  vector<unsigned int> Frequency(m_NOutputNodes, 0);
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    Frequency[NodeList[n]]++;
  }
  
  // Then determine the maximum and how many nodes have that maximum
  vector<unsigned int> PeakFrequencyNodes;
  unsigned int PeakFrequency = 0;
  for (unsigned int n = 0; n < m_NOutputNodes; ++n) {
    if (Frequency[n] == PeakFrequency) {
      PeakFrequencyNodes.push_back(n);
    } else if (Frequency[n] > PeakFrequency) {
      PeakFrequencyNodes.clear();
      PeakFrequencyNodes.push_back(n);
      PeakFrequency = Frequency[n];
    }
  }
  
  // If we exactly one output node appear most often, we use it
  if (PeakFrequencyNodes.size() == 1) {
    return PeakFrequencyNodes[0];
  }
  
  // Otherwise we look at the values and choose the node with the smallest sum over all networks
  vector<double> Sums(PeakFrequencyNodes.size(), 0);
  for (unsigned int p = 0; p < Sums.size(); ++p) {
    for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
      Sums[p] += m_NeuralNetworks[n]->GetOutput(PeakFrequencyNodes[p]);
    }
  }
  
  // Determine the smallest
  double MinSum = numeric_limits<double>::max();
  unsigned int MinNode = 0;
  for (unsigned int p = 0; p < Sums.size(); ++p) {
    if (Sums[p] < MinSum) {
      MinSum = Sums[p];
      MinNode = PeakFrequencyNodes[p];
    }
  }
  
  return MinNode;
}



////////////////////////////////////////////////////////////////////////////////


//! Set the output of one specific input node for learning for all networks
bool MNeuralNetworkBackpropagationAssembly::SetOutput(unsigned int i, double Value)
{
  bool Return = true;
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    if (m_NeuralNetworks[n]->SetOutput(i, Value) == false) {
      Return = false;
    }
  }
  
  return Return; 
}
  

////////////////////////////////////////////////////////////////////////////////



//! Set the output error of one specific node for all networks
bool MNeuralNetworkBackpropagationAssembly::SetOutputError(unsigned int i, double Value)
{
  bool Return = true;
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    if (m_NeuralNetworks[n]->SetOutputError(i, Value) == false) {
      Return = false;
    }
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////



//! Do the learning --- we will not decide if the learning was sufficient!
bool MNeuralNetworkBackpropagationAssembly::Learn()
{
  bool Return = true;
  
  m_NLearningRuns++;
  
  for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
    if (m_NeuralNetworks[n]->Learn() == false) {
      Return = false;
    }
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetworkBackpropagationAssembly::Stream(const bool Read)
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
    
    m_NeuralNetworks.clear();
    bool IsActive = false;    
    MNeuralNetworkBackpropagation* ActiveNN = nullptr;
    
    while (IsGood() == true) {
      ReadLine(Line);
      if (Line.Length() < 2) continue;
      
      if (Line[0] == 'N' && Line[1] == 'N') {
        unsigned int Number;
        if (sscanf(Line, "NN %u", &Number) == 1) {
          m_NNeuralNetworks = Number;
        } else {
          mout<<"Unable to parse NN!"<<endl;
          Ret = 1;
          break;
        } 
      } else if (Line[0] == 'N' && Line[1] == 'I') {
        ActiveNN = new MNeuralNetworkBackpropagation();
        
        unsigned int Number;
        if (sscanf(Line, "NI %u", &Number) == 1) {
          ActiveNN->SetNInputNodes(Number);
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
              ActiveNN->AddInputNode(N);
            } else if (MString(Line) == "NI EN") {
              break;
            }
          }
        } else {
          mout<<"Unable to parse NI!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'N' && Line[1] == 'M') {
        unsigned int Number;
        if (sscanf(Line, "NM %u", &Number) == 1) {
          ActiveNN->SetNMiddleNodes(Number);
          //m_MiddleNeurons.resize(Number);
          while (IsGood()) {
            ReadLine(Line);
            if (Line.Length() < 2) continue;
            
            if (MString(Line) == "NE") {
              MBackpropagationMiddleNeuron* N = new MBackpropagationMiddleNeuron();
              if (N->Stream(*this, m_Version, Read) == false) {
                return false;
              }
              ActiveNN->AddMiddleNode(N);
            } else if (MString(Line) == "NM EN") {
              break;
            }
          }
        } else {
          mout<<"Unable to parse NM!"<<endl;
          Ret = 1;
        }
      } else if (Line[0] == 'N' && Line[1] == 'O') {
        unsigned int Number;
        if (sscanf(Line, "NO %d", &Number) == 1) {
          ActiveNN->SetNOutputNodes(Number);
          //m_OutputNeurons.resize(Number);
          while (IsGood()) {
            ReadLine(Line);
            if (Line.Length() < 2) continue;
            
            if (MString(Line) == "NE") {
              MBackpropagationOutputNeuron* N = new MBackpropagationOutputNeuron();
              if (N->Stream(*this, m_Version, Read) == false) {
                return false;
              }
              ActiveNN->AddOutputNode(N);
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
        ActiveNN->AddSynapse(Sy);        
      } else if (Line[0] == 'L' && Line[1] == 'R') {
        double LearningRate;
        if (sscanf(Line, "LR %lf", &LearningRate) != 1) {
          mout<<"Unable to parse LR!"<<endl;
          Ret = 1;
        } else {
          ActiveNN->SetLearningRate(LearningRate); 
        }
      } else if (Line[0] == 'M' && Line[1] == 'M') {
        double Momentum;
        if (sscanf(Line, "MM %lf", &Momentum) != 1) {
          mout<<"Unable to parse MM!"<<endl;
          Ret = 1;
        } else {
          ActiveNN->SetMomentum(Momentum); 
        }
      } else if (Line[0] == 'E' && Line[1] == 'N') {
        if (IsActive == true && Ret == 0) {
          IsActive = false;
          ActiveNN->RestoreLinks();
          m_NeuralNetworks.push_back(ActiveNN);
          ActiveNN = nullptr;
        } else {
          Ret = -1;
          break;
        }
      }
    }
    
    
  } else {
    
    ostringstream S;
    
    WriteLine("Type RSP");
    WriteLine("Version 1");
    WriteLine();
    
    WriteLine(MString("# Number of times the function \"Learn()\" has been called: ") + m_NLearningRuns);
    WriteLine();
    
    for (unsigned int i = 0; i < m_UserComments.size(); ++i) {
      WriteLine(m_UserComments[i]);
    }
    WriteLine();
    
    WriteLine(MString("NN ") + m_NNeuralNetworks);
    for (unsigned int n = 0; n < m_NNeuralNetworks; ++n) {
      Write(m_NeuralNetworks[n]->ToString());
    }
    WriteLine("EN");
    
    Flush();    
  }
  
  //cout<<"Return code: "<<Return<<endl;
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIO MNeuralNetworkBackpropagationAssembly::GetIOStore()
{
  // Return the current stored values of the input and outputg nodes...
  
  MNeuralNetworkIO IOStore;
  IOStore.SetNInputs(m_NInputNodes);
  for (unsigned int i = 0; i < m_NInputNodes; ++i) {
    IOStore.SetInput(i, m_NeuralNetworks[0]->GetInput(i)); 
  }
  IOStore.SetNOutputs(m_NOutputNodes);
  for (unsigned int i = 0; i < m_NOutputNodes; ++i) {
    IOStore.SetOutput(i, m_NeuralNetworks[0]->GetOutput(i)); 
  }
  
  return IOStore;
}



// MNeuralNetworkBackpropagationAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
