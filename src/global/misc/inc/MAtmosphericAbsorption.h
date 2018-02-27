/*
 * MAtmosphericAbsorption.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAtmosphericAbsorption__
#define __MAtmosphericAbsorption__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFunction3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MAtmosphericAbsorption
{
  // public interface:
 public:
  //! Default constructor
  MAtmosphericAbsorption();
  //! Default destuctor 
  virtual ~MAtmosphericAbsorption();

  //! Read the data from file
  bool Read(MString FileName);
  
  //! Get transmission probability a value
  double GetTransmissionProbability(double Energy, double Azimuth, double Altitude) const;
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The data
  MFunction3D m_TransmissionProbabilities;


#ifdef ___CINT___
 public:
  ClassDef(MAtmosphericAbsorption, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
