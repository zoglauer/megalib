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
  MNeuralNetworkIO();
  virtual ~MNeuralNetworkIO();
  
  void SetNInputs(const unsigned int N);
  unsigned int GetNInputs() const;
  void SetInput(const unsigned int i, const double Value);
  double GetInput(const unsigned int i) const;
  
  void SetNOutputs(const unsigned int N);
  unsigned int GetNOutputs() const;
  void SetOutput(const unsigned int i, const double Value);
  double GetOutput(const unsigned int i) const;
  
  void SetNUserValues(const unsigned int N);
  unsigned int GetNUserValues() const;
  void SetUserValue(const unsigned int i, const double Value);
  double GetUserValue(const unsigned int i) const;
  
  //! True if this data set shall be used for verification
  bool IsVerificationData() const { return m_IsVerificationData; }
  //! Set if this data set shall be used for verification
  void IsVerificationData(bool IsVerificationData) { m_IsVerificationData = IsVerificationData; }
  
  MString ToString() const;
  
  // protected methods:
protected:
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  vector<double> m_Inputs;
  vector<double> m_Outputs;
  
  vector<double> m_UserValues;
  
  //! True if this data set shall be used for verification
  bool m_IsVerificationData;
  
  #ifdef ___CINT___
public:
  ClassDef(MNeuralNetworkIO, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
