/*
 * MResponseEventQuality.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEventQuality__
#define __MResponseEventQuality__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixON.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEventQuality : public MResponseBuilder
{
  // public interface:
 public:
  MResponseEventQuality();
  virtual ~MResponseEventQuality();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();

  
  // protected methods:
 protected:

  //! Save the response matrices
  virtual bool Save();
  
  
  // private methods:
 private:



  // protected members:
 protected:
  //! The event quality response --- good events 
  MResponseMatrixON m_GoodQuality;
  //! The event quality response --- bad events 
  MResponseMatrixON m_BadQuality;
  

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseEventQuality, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
