/*
 * MResponseConeShapes.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseConeShapes__
#define __MResponseConeShapes__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"
#include "MResponse.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseConeShapes : public MResponse
{
  // public interface:
 public:
  MResponseConeShapes();
  virtual ~MResponseConeShapes();

  bool LoadResponseFile(const MString& ResponseFileName);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonMaximum();
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  virtual bool AnalyzeEvent(MPhysicalEvent* Event); 


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  //! The transversal Compton response
  MResponseMatrixO5 m_ARMShape;

  //! Compton scatter angle of current Compton event:
  double m_Phi;
  //! Total energy of current Compton event:
  double m_Ei;
  //! Distance between the first two interactions
  double m_Distance;
  //! Numober of interactions
  int m_NumberOfInteractions;

  //! Data splice of the Compton Transversal Response 
  MResponseMatrixO1 m_Slice;

  //! Integral (Sum) over the Compton response
  double m_ComptonIntegral;


#ifdef ___CINT___
 public:
  ClassDef(MResponseConeShapes, 0) // basic response class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
