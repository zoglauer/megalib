/*
 * MCCrossSections.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class to calculate cross sections!
 *
 */

#ifndef ___MCCrossSections___
#define ___MCCrossSections___

// Geant4:
#include "globals.hh"

// Cosima:

// MEGAlib:
#include "MString.h"

// Standard lib:

// Forward declarations:


/******************************************************************************/

class MCCrossSections
{
  // public interface:
public:
  /// Default constructor
  MCCrossSections();
  /// Default destructor
  virtual ~MCCrossSections();

  /// Create all absorption files in the correct directory
  bool CreateCrossSectionFiles(MString Path);

  // protected methods:
protected:
  

  // protected members:
protected:


  // private members:
private:


};

#endif


/*
 * MCCrossSections.hh: the end...
 ******************************************************************************/
