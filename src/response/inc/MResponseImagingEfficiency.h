/*
 * MResponseImagingEfficiency.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingEfficiency__
#define __MResponseImagingEfficiency__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingEfficiency : public MResponseBuilder
{
  // public interface:
 public:
  MResponseImagingEfficiency();
  virtual ~MResponseImagingEfficiency();

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
  //! The basic efficiency matrix
  MResponseMatrixO2 m_Efficiency1;
  //! The rotation associated with the basic efficiency matrix
  MRotation m_Rotation1;
  
  //! The 90 degree rotated efficiency matrix
  MResponseMatrixO2 m_Efficiency2;
  //! The rotation associated with the rotated efficiency matrix
  MRotation m_Rotation2;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseImagingEfficiency, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
