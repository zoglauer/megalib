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
#include "MResponseMatrixO2.h"

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
  MResponseMatrixO2 m_EnergyBeforeER;
  MResponseMatrixO2 m_EnergyUnselected;
  MResponseMatrixO2 m_EnergySelected;
 
  MResponseMatrixO2 m_EnergyRatioBeforeER;
  MResponseMatrixO2 m_EnergyRatioUnselected;
  MResponseMatrixO2 m_EnergyRatioSelected;
  
                                        
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseSpectral, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
