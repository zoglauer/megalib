/*
 * MResponseTracking.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseTracking__
#define __MResponseTracking__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseTracking : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseTracking();
  //! Default destructor
  virtual ~MResponseTracking();

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

   
   
  double CalculateAngleIn(MRESE& Start, MRESE& Central);
  double CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop);

  // private methods:
 private:



  // protected members:
 protected:
   
  MResponseMatrixO1 m_GoodBadTable;

  MResponseMatrixO3 m_PdfDualGood;
  MResponseMatrixO3 m_PdfDualBad;
   
  MResponseMatrixO3 m_PdfStartGood;
  MResponseMatrixO3 m_PdfStartBad;
  
  MResponseMatrixO5 m_PdfGood;
  MResponseMatrixO5 m_PdfBad;
   
  MResponseMatrixO2 m_PdfStopGood;
  MResponseMatrixO2 m_PdfStopBad;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MResponseTracking, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
