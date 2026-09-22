/*
 * MBackpropagationSynapse.h
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


#ifndef __MBackpropagationSynapse__
#define __MBackpropagationSynapse__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSynapse.h"
#include "MFile.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationSynapse : public MSynapse
{
  // public interface:
public:
  //! Default and standard construyctor
  MBackpropagationSynapse(int Size = 2);
  //! Copy constructor
  MBackpropagationSynapse(const MBackpropagationSynapse& S);
  
  //! Set the weight
  virtual void SetWeight(double Delta);
  
  //! Stream from the file
  virtual bool Stream(MFile& S, const int Version, const bool Read);
  //! Parse an individual line
  virtual int ParseLine(MString Line);
  
  //! Dump the content into a string 
  virtual MString ToString() const;
  
  
  // protected methods:
protected:
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  //! The delta
  double m_Delta;
  
  
  #ifdef ___CLING___
public:
  ClassDef(MDummy, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////

