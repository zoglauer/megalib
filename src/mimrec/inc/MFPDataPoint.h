/*
 * MFPDataPoint.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFPDataPoint__
#define __MFPDataPoint__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


class MFPDataPoint : public TObject
{
  // Public Interface:
 public:
  MFPDataPoint();
  MFPDataPoint(double Psi, double Eta, double Phi, double Theta);
  ~MFPDataPoint();


  double GetPsi() { return m_Psi; }
  void SetPsi(double Psi) { m_Psi = Psi; }

  double GetEta() { return m_Eta; }
  void SetEta(double Eta) { m_Eta = Eta; }

  double GetPhi() { return m_Phi; }
  void SetPhi(double Phi) { m_Phi = Phi; }

  double GetTheta() { return m_Theta; }
  void SetTheta(double Theta) { m_Theta = Theta; }


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  double m_Psi;
  double m_Eta;
  double m_Phi;
  double m_Theta;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MFPDataPoint, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
