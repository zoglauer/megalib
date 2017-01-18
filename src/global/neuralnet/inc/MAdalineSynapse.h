/*
 * MAdalineSynapse.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAdalineSynapse__
#define __MAdalineSynapse__


////////////////////////////////////////////////////////////////////////////////



// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSynapse.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MAdalineSynapse : public MSynapse
{
  // public interface:
public:
  MAdalineSynapse();
  virtual ~MAdalineSynapse() {}
  
  
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
  ClassDef(MAdalineSynapse, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
