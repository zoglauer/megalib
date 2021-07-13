/*
 * MResponseComptonEnergies.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseComptonEnergies__
#define __MResponseComptonEnergies__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseComptonEnergies : public TObject
{
  // public interface:
 public:
  MResponseComptonEnergies();
  ~MResponseComptonEnergies();


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseComptonEnergies, 0) // Response class for a E1-E2-Phibar-Response
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
