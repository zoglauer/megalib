/*
 * MREAMDriftChamberEnergy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREAMDriftChamberEnergy__
#define __MREAMDriftChamberEnergy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MREAM.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MREAMDriftChamberEnergy : public MREAM
{
  // public interface:
 public:
  //! Default constructor of the additional measurement drift chamber energy
  MREAMDriftChamberEnergy();
  //! Copy constructor of the additional measurement drift chamber energy
  MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy& REAMDriftChamberEnergy);
  //! Default destructor of the additional measurement drift chamber energy
  virtual ~MREAMDriftChamberEnergy();

  //! Default assignment constructor
  const MREAMDriftChamberEnergy& operator=(const MREAMDriftChamberEnergy&);

  //! In almost all cases when we need a replica of this object we do not know what it is -
  //! Thus we need a general duplication mechanism
  virtual MREAM* Clone();

  //! Set the total energy deposit in the drift chamber
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  //! Return the total energy deposit in the drift chamber
  double GetEnergy() const { return m_Energy; }

  //! Set the resolution of the energy deposit in the drift chamber
  void SetEnergyResolution(const double Resolution) { m_EnergyResolution = Resolution; }
  //! Return the resolution of the total energy deposit in the drift chamber
  double GetEnergyResolution() const { return m_EnergyResolution; }

  // protected methods:
 protected:
  //MREAMDriftChamberEnergy() {};
  //MREAMDriftChamberEnergy(const MREAMDriftChamberEnergy& REAMDriftChamberEnergy) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Total energy in this drift chamber
  double m_Energy;
  //! Energy resolution of the according energy deposit
  double m_EnergyResolution;


#ifdef ___CINT___
 public:
  ClassDef(MREAMDriftChamberEnergy, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
