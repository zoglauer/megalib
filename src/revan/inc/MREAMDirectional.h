/*
 * MREAMDirectional.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MREAMDirectional__
#define __MREAMDirectional__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MREAM.h"
#include "MRESE.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MREAMDirectional : public MREAM
{
  // public interface:
 public:
  //! Default constructor for a charged particle direction as measured by a Strip3DDirectional detector
  MREAMDirectional();
  //! Copy constructor for a charged particle direction
  MREAMDirectional(const MREAMDirectional& REAMDirectional);
  //! Default destructor for a charged particle direction
  virtual ~MREAMDirectional();

  //! Default assignment constructor
  const MREAMDirectional& operator=(const MREAMDirectional&);

  //! In almost all cases when we need a replica of this object we do not know what it is -
  //! Thus we need a general duplication mechanism
  virtual MREAM* Clone();

  //! Dump the essential content of this REAM
  virtual MString ToString(int Level = 0);


  //! Set the direction of the charged particle
  void SetDirection(const MVector& Direction) { m_Direction = Direction; }
  //! Return the direction of the charged particle
  MVector GetDirection() const { return m_Direction; }

  //! Set the total energy deposit of the charged particle
  void SetEnergy(const double Energy) { m_Energy = Energy; }
  //! Return the total energy deposit of the charged particle
  double GetEnergy() const { return m_Energy; }

  //! Set the total energy deposit of the charged particle
  void SetRESE(MRESE* RESE) { m_RESE = RESE; }
  //! Return the total energy deposit of the charged particle
  MRESE* GetRESE() { return m_RESE; }


  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  //! Direction of charged particle
  MVector m_Direction;
  //! Total energy of charged particle
  double m_Energy;
  //! The RESE this directional belongs to...
  MRESE* m_RESE;

#ifdef ___CLING___
 public:
  ClassDef(MREAMDirectional, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
