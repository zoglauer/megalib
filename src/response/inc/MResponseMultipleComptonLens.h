/*
 * MResponseMultipleComptonLens.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMultipleComptonLens__
#define __MResponseMultipleComptonLens__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MRESE.h"
#include "MRETrack.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMultipleComptonLens : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleComptonLens();
  //! Default destructor
  virtual ~MResponseMultipleComptonLens();

  //! The current file under investigation is from the lens
  void SetLensOrigin(bool IsLensOrigin) { m_IsLensOrigin = IsLensOrigin; }

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

  double CalculateConeLensDistance(MRESE& First, MRESE& Second, double Etot);
  double CalculateRadiusFromBeamCenter(MRESE& First);
  double CalculateInteractionDepth(MRESE& First);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The underlying file originates from a lens simulation
  bool m_IsLensOrigin;

  //! Center of the lens in the detector coordinates
  MVector m_LensCenter;
  //! Center of the focal spot on the detector surface!
  MVector m_FocalSpotCenter;

  // The event/pdf matrices:
  //! Store good events originating from lens
  MResponseMatrixO3 m_PdfFromLensGood;
  //! Store bad events originating from lens
  MResponseMatrixO3 m_PdfFromLensBad;


#ifdef ___CINT___
 public:
  ClassDef(MResponseMultipleComptonLens, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
