/*
 * MBackpropagationSynapse.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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

