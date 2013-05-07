/*
 * MResponseGaussian.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseGaussian__
#define __MResponseGaussian__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponse.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseGaussian : public MResponse
{
  // public interface:
 public:
  MResponseGaussian(const double ComptonTransversal, 
                    const double ComptonLongitudinal, 
                    const double PairDistance);
  virtual ~MResponseGaussian();

  //! Set the two Gaussian fit parameters 
  void SetGaussians(const double ComptonTransversal, 
                    const double ComptonLongitudinal, 
                    const double PairDistance);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonResponse(const double t, const double l);
  virtual double GetComptonMaximum();
  virtual double GetComptonIntegral(double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  //! Get pair response
  virtual double GetPairResponse(const double t);
  virtual double GetPairIntegral() const;

  bool AnalyzeEvent(MPhysicalEvent* Event);

  // protected methods:
 protected:
  //! Special version of exp for Gauss-calculations: exp(-x) --> this input x has to be positive!!
  //! We have a cut-off at x = 10.4: y < 0.00003 -> y = 0
  //! The cut-off is at the absolute accuracy limit of the algorithm
  float GaussExp(float x);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  double m_LongitudinalFit;         // longitudinal sigma of the cone-arc
  double m_TransversalFit;          // transversal sigma of the cone
  double m_PairFit;               

  double m_GaussSquareSigmaLong;
  double m_GaussSquareSigmaTrans;
  double m_GaussSquareSigmaPair;

  double m_GaussFactorsLong;
  double m_GaussFactorsTrans;
  // m_GaussFactorsLong*m_GaussFactorsTrans for speeding up track calculations
  double m_GaussFactorsTransLong;
  double m_GaussFactorsPair;

  double m_TransversalMax;
  double m_LongitudinalMax;

  double m_Phi;                     // Compton scatter angle

  bool m_HasTrack;




#ifdef ___CINT___
 public:
  ClassDef(MResponseGaussian, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
