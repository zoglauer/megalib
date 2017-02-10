/*
 * MResponseSpectral.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseSpectral__
#define __MResponseSpectral__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixON.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseSpectral : public MResponseBuilder
{
  // public interface:
 public:
  MResponseSpectral();
  virtual ~MResponseSpectral();

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
  MResponseMatrixON m_EnergyBeforeER;
  MResponseMatrixON m_EnergyUnselected;
  MResponseMatrixON m_EnergySelected;
 
  MResponseMatrixON m_EnergyRatioBeforeER;
  MResponseMatrixON m_EnergyRatioUnselected;
  MResponseMatrixON m_EnergyRatioSelected;
  
                                        
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseSpectral, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
