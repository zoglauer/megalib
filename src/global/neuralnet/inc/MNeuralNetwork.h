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


////////////////////////////////////////////////////////////////////////////////


class MNeuralNetwork : public MParser
{
  // public interface:
public:
  MNeuralNetwork();
  MNeuralNetwork(const MNeuralNetwork& NN);
  virtual ~MNeuralNetwork();
  
  //! Open the file name and read the header
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);
  virtual bool Stream(const bool Read) { return false; }
  
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
  
  
  //! Do the learning --- we will not decide if the learning was sufficient!
  virtual bool Learn();
  
  //!
  virtual MNeuralNetworkIOStore GetIOStore() { MNeuralNetworkIOStore IO; return IO; }
  
  //!
  bool IsCreated() { return m_IsCreated; }
  
  // protected methods:
protected:
  //MNeuralNetwork() {};
  //MNeuralNetwork(const MNeuralNetwork& NeuralNetwork) {};
  
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
  
  // private members:
private:
  
  
  
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetwork, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
