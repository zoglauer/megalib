/*
 * MERCSRTMVAMethods.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MERCSRTMVAModes__
#define __MERCSRTMVAModes__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Definition of the coordinate system IDs
enum class MERCSRTMVAMethod : int { 
  c_Unknown = 0, c_MLP = 1, c_BDTD = 2, c_PDEFoam = 3, c_PDEFoamBoost = 4, c_DNN_CPU = 5, c_DNN_GPU = 6, c_Overflow = 7
};


////////////////////////////////////////////////////////////////////////////////


//! The TMVA modes
class MERCSRTMVAMethods
{
  // public interface:
 public:
  //! Default constructor
  MERCSRTMVAMethods();
  //! Default destuctor 
  virtual ~MERCSRTMVAMethods();

  //! Return the string as method, if not found c_Unknown is returned
  MERCSRTMVAMethod GetMethod(MString String) const;
  
  //! Return the method as a string, if not found "Unknown" is returned
  MString GetString(MERCSRTMVAMethod Method) const;
  
  //! Return the method as a string, if not found "Unknown" is returned
  MString GetFullString(MERCSRTMVAMethod Method) const;
  
  //! Return all methods
  vector<MERCSRTMVAMethod> GetAllMethods() const;
  
  //! Set all used modes
  void ResetUsedMethods();
  
  //! Set all used modes - no reset is made
  bool SetUsedMethods(MString MethodsString);
  
  //! Add one used method
  void AddUsedMethod(MERCSRTMVAMethod Method) { m_UsedMethods[Method] = true; }
  
  //! Set all used modes - no reset is made
  void UnsetUsedMethod(MERCSRTMVAMethod Method);
  
  //! Return true if the method is used
  bool IsUsedMethod(MERCSRTMVAMethod Method) const;
  
  //! Return the number of used TMVCA methods
  unsigned int GetNumberOfUsedMethods() const;
  
  //! Return the used methods as a vector
  vector<MERCSRTMVAMethod> GetUsedMethods() const;
  
  //! Return the used methods as a string
  MString GetUsedMethodsString() const;
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The used methods
  map<MERCSRTMVAMethod, bool> m_UsedMethods;  


#ifdef ___CLING___
 public:
   ClassDef(MERCSRTMVAMethods, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
