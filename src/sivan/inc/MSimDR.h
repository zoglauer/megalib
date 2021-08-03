/*
 * MSimDR.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimDR__
#define __MSimDR__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs

// MEGAlib libs
#include "MGlobal.h"
#include "MVector.h"
#include "MBinaryStore.h"


////////////////////////////////////////////////////////////////////////////////


class MSimDR
{
  // Public Interface:
 public:
  //! Default constructor of the DR keyword
  MSimDR();
  //! Default constructor of the DR keyword
  MSimDR(const MVector& Position, const MVector& Direction, const double Energy);
  //! Default destructor
  virtual ~MSimDR();

  //! Parse one line of the sim file 
  bool AddRawInput(MString LineBuffer, const int Version);
  
  //! Parse the data from a binary stream
  bool ParseBinary(MBinaryStore& Store, const int BinaryPrecision = 32, const int Version = 25);
  
  //! Retrieve the position
  MVector GetPosition() const { return m_Position; }
  //! Set the position
  void SetPosition(const MVector& Position) { m_Position = Position; }

  //! Retrieve the direction
  MVector GetDirection() const { return m_Direction; }
  //! Set the direction
  void SetDirection(const MVector& Direction) { m_Direction = Direction; }

  //! Retrieve the energy
  double GetEnergy() const { return m_Energy; }
  //! Set the energy
  void SetEnergy(const double Energy) { m_Energy = Energy; }

  //! Return the content as a string for the sim file
  MString ToSimString(const int WhatToStore = 1, const int Precision = 0, const int Version = 25) const;
  //! Convert the *key* content to binary
  bool ToBinary(MBinaryStore& Out, const int WhatToStore = 1, const int BinaryPrecision = 32, const int Version = 25);
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The position
  MVector m_Position;
  //! The direction
  MVector m_Direction;
  //! The energy
  double m_Energy;


#ifdef ___CLING___
 public:
  ClassDef(MSimDR, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
