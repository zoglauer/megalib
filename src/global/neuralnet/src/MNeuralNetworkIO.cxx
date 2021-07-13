/*
 * MNeuralNetworkIO.cxx
 *
 *
 * Copyright (C) 2005-2007 by Andreas Zoglauer.
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
// MNeuralNetworkIO
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNeuralNetworkIO.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNeuralNetworkIO)
#endif


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIO::MNeuralNetworkIO()
{
  // Construct an instance of MNeuralNetworkIO
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetworkIO::~MNeuralNetworkIO()
{
  // Delete this instance of MNeuralNetworkIO
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkIO::SetNInputs(const unsigned int N)
{
  m_Inputs.resize(N);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuralNetworkIO::GetNInputs() const
{
  return m_Inputs.size();
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkIO::SetInput(const unsigned int i, const double Value)
{
  if (i >= m_Inputs.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Inputs.size(), i);
  }
  
  m_Inputs[i] = Value;
}


////////////////////////////////////////////////////////////////////////////////


double MNeuralNetworkIO::GetInput(const unsigned int i) const
{
  if (i >= m_Inputs.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Inputs.size(), i);
  }

  return m_Inputs[i];
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkIO::SetNOutputs(const unsigned int N)
{
  m_Outputs.resize(N);
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuralNetworkIO::GetNOutputs() const
{
  return m_Outputs.size();
}


////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkIO::SetOutput(const unsigned int i, const double Value)
{
   if (i >= m_Outputs.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Outputs.size(), i);
  }

  m_Outputs[i] = Value;
}


////////////////////////////////////////////////////////////////////////////////


double MNeuralNetworkIO::GetOutput(const unsigned int i) const
{
   if (i >= m_Outputs.size()) {
    throw MExceptionIndexOutOfBounds(0, m_Outputs.size(), i);
  }

  return m_Outputs[i];
}




////////////////////////////////////////////////////////////////////////////////


void MNeuralNetworkIO::AddUserValue(const MString& Key, const double Value)
{
  unsigned int Index = FindKey(Key);
  if (Index != numeric_limits<unsigned int>::max()) {
    m_UserValues[Index] = Value;
  } else {
    m_UserValueKeys.push_back(Key);
    m_UserValues.push_back(Value);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MNeuralNetworkIO::GetUserValue(const MString& Key) const
{
  unsigned int Index = FindKey(Key);
  if (Index != numeric_limits<unsigned int>::max()) {
    return m_UserValues[Index];
  }
  
  throw MExceptionObjectDoesNotExist(Key);
  
  return 0;
}

////////////////////////////////////////////////////////////////////////////////


unsigned int MNeuralNetworkIO::FindKey(const MString& Key) const
{
  for (unsigned int i = 0; i < m_UserValueKeys.size(); ++i) {
    if (Key == m_UserValueKeys[i]) {
      return i; 
    }
  }
  
  return numeric_limits<unsigned int>::max();
}


////////////////////////////////////////////////////////////////////////////////


MString MNeuralNetworkIO::ToString() const
{
  MString S = "";

  for (unsigned int i = 0; i < m_Inputs.size(); ++i) {
    S += i;
    S += ": ";
    S += m_Inputs[i];
    S += "\n";
  }

  return S;
}


// MNeuralNetworkIO.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
