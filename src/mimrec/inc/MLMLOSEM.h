/*
 * MLMLOSEM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MLMLOSEM__
#define __MLMLOSEM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MLMLClassicEM.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MLMLOSEM : public MLMLClassicEM
{
  // public interface:
 public:
  //! Default constructor
  MLMLOSEM();
  //! Default destructor
  virtual ~MLMLOSEM();

  //! Set the number of subsets:
  void SetNSubSets(unsigned int NSubSets) { m_NSubSets = NSubSets; if (m_NSubSets < 1) m_NSubSets = 1; }

  //! performs one iteration of the algorithm
  virtual bool DoOneIteration();

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The number of subsets
  unsigned int m_NSubSets;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MLMLOSEM, 0) // my "classic" OS-EM 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
