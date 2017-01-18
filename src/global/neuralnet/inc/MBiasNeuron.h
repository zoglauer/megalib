/*
 * MBiasNeuron.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBiasNeuron__
#define __MBiasNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MInputNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBiasNeuron : public MInputNeuron
{
  // public interface:
public:
  //! Default and standard constructor
  MBiasNeuron(double Bias = 1.0);
  
  //! No SetValue allowed:
  virtual void SetValue(double Value, int Number);
  //! Retrieve only the one and only value:
  virtual double GetValue(int Number);
  
  
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
  ClassDef(MBiasNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
