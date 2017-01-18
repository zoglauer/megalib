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
}


////////////////////////////////////////////////////////////////////////////////


MNeuralNetwork::MNeuralNetwork(const MNeuralNetwork& NN)
{
  // Copy constructor:
  
  m_IsCreated = NN.m_IsCreated;
  
  m_UserComments = NN.m_UserComments;
  
  m_NLearningRuns = 0;
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
    
    //cout<<ToString()<<endl;
    //return false;
  }
  
  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNeuralNetwork::Learn()
{
  // Learn --- thus function should be overwritten by a base class
  // which should call this function in the beginning
  
  m_NLearningRuns++;
  
  return true;
}


// MNeuralNetwork.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
