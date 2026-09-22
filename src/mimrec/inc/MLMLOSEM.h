/*
 * MLMLOSEM.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
