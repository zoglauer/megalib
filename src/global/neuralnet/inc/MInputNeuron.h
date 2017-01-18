/*
 * MInputNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInputNeuron__
#define __MInputNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MInputNeuron : public MNeuron
{
  // public interface:
public:
  //! Standard constructor
  MInputNeuron(int NValues = 1);
  //! Copy constructor
  MInputNeuron(const MInputNeuron& N);
  
  
  // protected methods:
protected:
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  
  
  
  #ifdef ___CINT___
public:
  ClassDef(MInputNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
