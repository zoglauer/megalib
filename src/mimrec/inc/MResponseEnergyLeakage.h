/*
 * MResponseEnergyLeakage.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseEnergyLeakage__
#define __MResponseEnergyLeakage__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponse.h"

// Forward declarations:
class MComptonEvent;

////////////////////////////////////////////////////////////////////////////////


class MResponseEnergyLeakage : public MResponse
{
  // public interface:
 public:
  MResponseEnergyLeakage(const double Electron, const double Gamma);
  virtual ~MResponseEnergyLeakage();

  void SetGaussians(const double Electron, const double Gamma);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonResponse(const double t, const double l);
  virtual double GetComptonMaximum();
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  bool AnalyzeEvent(MPhysicalEvent* Event);

  double GetKleinNishina(const double Phi) const;

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  double m_FitElectron;         // longitudinal sigma of the cone-arc
  double m_FitGamma;          // transversal sigma of the cone

  double m_tmin;
  double m_tmax;

  double m_Theta;

  MComptonEvent *m_Compton;


#ifdef ___CINT___
 public:
  ClassDef(MResponseEnergyLeakage, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
