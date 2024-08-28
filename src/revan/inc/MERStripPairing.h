/*
 * MERStripPairing.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERStripPairing__
#define __MERStripPairing__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MERConstruction.h"
#include "MRawEventIncarnationList.h"
#include "MREStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MERStripPairing : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERStripPairing();
  //! Default destructor
  virtual ~MERStripPairing();

  //! Search for coincidences --- return the event or zero if non found
  bool Analyze(MRawEventIncarnationList* List);

  //! Dump some elemenatry information about the algorithm settings
  MString ToString(bool CoreOnly) const;

  //! ID representing no coincidence search
  static const int c_None = 0;
  //! ID representing a coincidence window
  static const int c_ChiSquare = 1;

  // protected methods:
 protected:
  // Fins a new set of combinations giving the existin gone 
   vector<vector<vector<unsigned int>>> FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MREStripHit*> StripHits);
   
   
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The used algorithm
  int m_Algorithm;

  //! The coincidence window
  MTime m_Window;

  //! The number of found coincidences
  int m_NFoundStripPairings;
  

#ifdef ___CLING___
 public:
  ClassDef(MERStripPairing, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
