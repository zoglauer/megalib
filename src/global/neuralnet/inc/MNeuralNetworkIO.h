/*
 * MNeuralNetworkIO.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNeuralNetworkIO__
#define __MNeuralNetworkIO__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNeuralNetworkIO
{
  // public interface:
public:
  //! Default constructor
  MNeuralNetworkIO();
  //! Default destructor
  virtual ~MNeuralNetworkIO();
  
  //! Set the number of input values
  void SetNInputs(const unsigned int N);
  //! Get the number of input values
  unsigned int GetNInputs() const;
  //! Set a specific input value
  //! Throws MExceptionIndexOutOfBounds if the output does not exist
  void SetInput(const unsigned int i, const double Value);
  //! Get a specific input value
  //! Throws MExceptionIndexOutOfBounds if the output does not exist
  double GetInput(const unsigned int i) const;
  
  //! Set the number of output values
  void SetNOutputs(const unsigned int N);
  //! Get the number of output values
  unsigned int GetNOutputs() const;
  //! Set a specific output value
  //! Throws MExceptionIndexOutOfBounds if the output does not exist
  void SetOutput(const unsigned int i, const double Value);
  //! Get a specific output value
  //! Throws MExceptionIndexOutOfBounds if the output does not exist
  double GetOutput(const unsigned int i) const;
  
  //! Add a user value with the given key, if key exists exchange the value
  void AddUserValue(const MString& Key, const double Value);
  //! Return the user value with the given key
  //! Throws MExceptionObjectDoesNotExist if key does not exist
  double GetUserValue(const MString& Key) const;
  
  //! True if this data set shall be used for verification
  bool IsVerificationData() const { return m_IsVerificationData; }
  //! Set if this data set shall be used for verification
  void IsVerificationData(bool IsVerificationData) { m_IsVerificationData = IsVerificationData; }
  
  //! Dump the content into a string
  MString ToString() const;
  
  // protected methods:
protected:
  //! Find a user key
  //! If not found return numeric_limits<unsigned int>::max()
  unsigned int FindKey(const MString& Key) const;

  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! A vector of the input values
  vector<double> m_Inputs;
  //! A vector of the output values
  vector<double> m_Outputs;
  
  //! The keys to the user values
  vector<MString> m_UserValueKeys;
  //! The user values
  vector<double> m_UserValues;
  
  //! True if this data set shall be used for verification
  bool m_IsVerificationData;
  
  #ifdef ___CLING___
public:
  ClassDef(MNeuralNetworkIO, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
