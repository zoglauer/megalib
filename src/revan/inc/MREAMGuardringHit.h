/*
 * MREAMGuardringHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREAMGuardringHit__
#define __MREAMGuardringHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MREAM.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MREAMGuardringHit : public MREAM
{
  // public interface:
 public:
  //! Default constructor for a guard ring hit
  MREAMGuardringHit();
  //! Copy constructor for a guard ring hit
  MREAMGuardringHit(const MREAMGuardringHit& REAMGuardringHit);
  //! Default destructor for a guard ring hit
  virtual ~MREAMGuardringHit();

  //! Default assignment constructor
  const MREAMGuardringHit& operator=(const MREAMGuardringHit&);

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
  //MREAMGuardringHit() {};
  //MREAMGuardringHit(const MREAMGuardringHit& REAMGuardringHit) {};

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


#ifdef ___CINT___
 public:
  ClassDef(MREAMGuardringHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
