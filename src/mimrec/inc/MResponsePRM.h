/*
 * MResponsePRM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponsePRM__
#define __MResponsePRM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"
#include "MResponse.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO3.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponsePRM : public MResponse
{
  // public interface:
 public:
  MResponsePRM();
  virtual ~MResponsePRM();

  bool LoadResponseFiles(const MString& TransFileName, 
                         const MString& LongFileName,
                         const MString& PairFileName);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonResponse(const double t, const double l);
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
  MResponseMatrixO3 m_ComptonTrans;
  //! The longitudinal Compton response
  MResponseMatrixO3 m_ComptonLong;

  //! Compton scatter angle of current Compton event:
  double m_Phi;
  //! Total energy of current Compton event:
  double m_Ei;
  //! Electron scatter angle  of current Compton event:
  double m_Epsilon;
  //! Energy of recoil electron of current Compton event:
  double m_Ee;

  //! Data splice of the Compton Transversal Response 
  MResponseMatrixO1 m_SliceTrans;
  //! Data splice of the Compton Longitudinal Response 
  MResponseMatrixO1 m_SliceLong;

  //! Integral (Sum) over the Compton response
  double m_ComptonIntegral;


#ifdef ___CINT___
 public:
  ClassDef(MResponsePRM, 0) // basic response class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
