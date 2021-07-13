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
  void SetNSubSets(unsigned int NSubSets);

  //! performs one iteration of the algorithm
  virtual bool DoOneIteration();

  // protected methods:
 protected:
  //! Shuffle the events around - does notthing here, but in ordered subsets algorithm
  virtual void Shuffle();
  //! Determine the apportionment of the events for the threads
  virtual void CalculateEventApportionment();
  //! Entry point for the convolution thread
  virtual void ConvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop);
  //! Entry point for the deconvolution thread
  virtual void DeconvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop);


  // private methods:
 private:



  // protected members:
 protected:
  //! The number of Set subsets
  unsigned int m_NSetSubSets;
  //! The number of Set subsets
  unsigned int m_NUsedSubSets;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MLMLOSEM, 0) // my "classic" OS-EM
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
