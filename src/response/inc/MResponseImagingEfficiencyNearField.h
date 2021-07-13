/*
 * MResponseImagingEfficiencyNearField.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingEfficiencyNearField__
#define __MResponseImagingEfficiencyNearField__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixO4.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingEfficiencyNearField : public MResponseBuilder
{
  // public interface:
 public:
  MResponseImagingEfficiencyNearField();
  virtual ~MResponseImagingEfficiencyNearField();

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
  MResponseMatrixO4 m_Efficiency;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseImagingEfficiencyNearField, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
