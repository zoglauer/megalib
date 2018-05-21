/*
 * MREAMGuardRingHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREAMGuardRingHit__
#define __MREAMGuardRingHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MREAM.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MREAMGuardRingHit : public MREAM
{
  // public interface:
 public:
  //! Default constructor for a guard ring hit
  MREAMGuardRingHit();
  //! Copy constructor for a guard ring hit
  MREAMGuardRingHit(const MREAMGuardRingHit& REAMGuardRingHit);
  //! Default destructor for a guard ring hit
  virtual ~MREAMGuardRingHit();

  //! Default assignment constructor
  const MREAMGuardRingHit& operator=(const MREAMGuardRingHit&);

  //! In almost all cases when we need a replica of this object we do not know what it is -
  //! Thus we need a general duplication mechanism
  virtual MREAM* Clone();

  //! Set the total energy deposit in the guard ring
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  //! Return the total energy deposit in the guard ring
  double GetEnergy() const { return m_Energy; }

  //! Set the resolution of the energy deposit in the guard ring
  void SetEnergyResolution(const double Resolution) { m_EnergyResolution = Resolution; }
  //! Return the resolution of the total energy deposit in the guard ring
  double GetEnergyResolution() const { return m_EnergyResolution; }

  //! Convert to a string in the evta file
  virtual MString ToEvtaString(const int Precision, const int Version = 1);

  // protected methods:
 protected:
  //MREAMGuardRingHit() {};
  //MREAMGuardRingHit(const MREAMGuardRingHit& REAMGuardRingHit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Total energy deposit in the guard ring
  double m_Energy;
  //! Energy resolution of the according energy deposit
  double m_EnergyResolution;


#ifdef ___CLING___
 public:
  ClassDef(MREAMGuardRingHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
