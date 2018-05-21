/*
 * MERCSRTMVAMethods.cxx
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


// Include the header:
#include "MERCSRTMVAMethods.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCSRTMVAMethods)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MERCSRTMVAMethods::MERCSRTMVAMethods()
{
  ResetUsedMethods();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MERCSRTMVAMethods::~MERCSRTMVAMethods()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Return the string as method, if not found c_Unknown is returned
MERCSRTMVAMethod MERCSRTMVAMethods::GetMethod(MString String) const
{
  String.ToUpperInPlace();
  
  if (String == "MLP") {
    return MERCSRTMVAMethod::c_MLP;
  } else if (String == "BDTD") {
    return MERCSRTMVAMethod::c_BDTD;
  } else if (String == "PDEFOAMBOOST") {
    return MERCSRTMVAMethod::c_PDEFoamBoost;
  } else if (String == "DNN_CPU") {
    return MERCSRTMVAMethod::c_DNN_CPU;
  } else if (String == "DNN_GPU") {
    return MERCSRTMVAMethod::c_DNN_GPU;
  } else {
    return MERCSRTMVAMethod::c_Unknown;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the method as a string, if not found "Unknown" is returned
MString MERCSRTMVAMethods::GetString(MERCSRTMVAMethod Method) const
{
  if (Method == MERCSRTMVAMethod::c_MLP) {
    return "MLP";
  } else if (Method == MERCSRTMVAMethod::c_BDTD) {
    return "BDTD";
  } else if (Method == MERCSRTMVAMethod::c_PDEFoamBoost) {
    return "PDEFoamBoost";
  } else if (Method == MERCSRTMVAMethod::c_DNN_CPU) {
    return "DNN_CPU";
  } else if (Method == MERCSRTMVAMethod::c_DNN_GPU) {
    return "DNN_GPU";
  } else {
    return "Unknown";
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the method as a string, if not found "Unknown" is returned
MString MERCSRTMVAMethods::GetFullString(MERCSRTMVAMethod Method) const
{
  if (Method == MERCSRTMVAMethod::c_MLP) {
    return "Multi-layer perceptron";
  } else if (Method == MERCSRTMVAMethod::c_BDTD) {
    return "Boosted decision trees";
  } else if (Method == MERCSRTMVAMethod::c_PDEFoamBoost) {
    return "Likelihood estimator using boosted self-adapting phase-space binning (PDE-Foam)";
  } else if (Method == MERCSRTMVAMethod::c_DNN_CPU) {
    return "Deep neural network on CPU";
  } else if (Method == MERCSRTMVAMethod::c_DNN_GPU) {
    return "Deep neural network on GPU";
  } else {
    return "Unknown";
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set all used modes
void MERCSRTMVAMethods::ResetUsedMethods()
{
  for (int i = static_cast<int>(MERCSRTMVAMethod::c_Unknown) + 1; i < static_cast<int>(MERCSRTMVAMethod::c_Overflow); ++i) {
    m_UsedMethods[static_cast<MERCSRTMVAMethod>(i)] = false;
  }
}

////////////////////////////////////////////////////////////////////////////////


//! Return all methods as a vector
vector<MERCSRTMVAMethod> MERCSRTMVAMethods::GetAllMethods() const
{
  vector<MERCSRTMVAMethod> Methods;
  for (int i = static_cast<int>(MERCSRTMVAMethod::c_Unknown) + 1; i < static_cast<int>(MERCSRTMVAMethod::c_Overflow); ++i) {
    Methods.push_back(static_cast<MERCSRTMVAMethod>(i));
  }
  return Methods;
}



////////////////////////////////////////////////////////////////////////////////


//! Set all used methods
bool MERCSRTMVAMethods::SetUsedMethods(MString MethodsString)
{
  vector<MString> MethodsStrings = MethodsString.Tokenize(",");
  for (MString M: MethodsStrings) {
    if (GetMethod(M) != MERCSRTMVAMethod::c_Unknown) {
      m_UsedMethods[GetMethod(M)] = true;
    } else {
      merr<<"Unable to find method: \""<<M<<"\""<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all used modes - no reset is made
void MERCSRTMVAMethods::UnsetUsedMethod(MERCSRTMVAMethod Method)
{
  m_UsedMethods[Method] = false;
}

////////////////////////////////////////////////////////////////////////////////


//! Return true if the method is used
bool MERCSRTMVAMethods::IsUsedMethod(MERCSRTMVAMethod Method) const
{
  // More complicated than needed for const-correctness
  auto I = m_UsedMethods.find(Method);
  if (I != m_UsedMethods.end()) return I->second;
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the used methods as a string
MString MERCSRTMVAMethods::GetUsedMethodsString() const
{
  MString Methods;
  for (auto I = m_UsedMethods.begin(); I != m_UsedMethods.end(); ++I) {
    if (I->second == true) {
      if (Methods != "") Methods += ",";
      Methods += GetString(I->first);
    }
  }
  return Methods;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the used methods as a vector
vector<MERCSRTMVAMethod> MERCSRTMVAMethods::GetUsedMethods() const
{
  vector<MERCSRTMVAMethod> Methods;
  for (auto I = m_UsedMethods.begin(); I != m_UsedMethods.end(); ++I) {
    if (I->second == true) {
      Methods.push_back(I->first);
    }
  }
  return Methods;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of used TMVCA methods
unsigned int MERCSRTMVAMethods::GetNumberOfUsedMethods() const
{
  unsigned int NUsedMethods = 0;
  for (int i = static_cast<int>(MERCSRTMVAMethod::c_Unknown) + 1; i < static_cast<int>(MERCSRTMVAMethod::c_Overflow); ++i) {
    auto I = m_UsedMethods.find(static_cast<MERCSRTMVAMethod>(i));
    if (I != m_UsedMethods.end() && I->second == true) { 
      ++NUsedMethods;
    }
  }
  
  return NUsedMethods;
}


// MERCSRTMVAMethods.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
