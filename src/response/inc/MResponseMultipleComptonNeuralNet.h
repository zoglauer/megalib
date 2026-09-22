/*
 * MResponseMultipleComptonNeuralNet.h
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


#ifndef __MResponseMultipleComptonNeuralNet__
#define __MResponseMultipleComptonNeuralNet__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"
#include "MNeuralNetworkIOStore.h"
#include "MNeuralNetworkBackpropagation.h"
#include "MNeuralNetworkBackpropagationAssembly.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMultipleComptonNeuralNet : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleComptonNeuralNet();
  //! Default destructor
  virtual ~MResponseMultipleComptonNeuralNet();
  
  //! Initialize the response matrices and their generation
  virtual bool Initialize();
  
  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
  
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();
  
  // protected methods:
 protected:
   
   //! Save the response matrices
   virtual bool Save();
  
  //! Store a specific sequence NN  
  bool SaveMatrixSequenceNN(unsigned int e, unsigned int s, MString Flag);
  //! Store a specific quality NN 
  bool SaveMatrixQualityNN(unsigned int e, unsigned int s, MString Flag);

  //! Find the first two interaction in the simulated event
  bool FindFirstInteractions(const vector<MRESE*>& RESEs, MRESE*& First, MRESE*& Second);
  //! Find the correct interaction sequence, or return false if non can be found  
  bool FindCorrectSequence(const vector<MRESE*>& RESEs, vector<MRESE*>& Sorted);
  //! Return true if all RESEs are completely aborbed within the given measurement uncertainties
  bool AreCompletelyAbsorbed(const vector<MRESE*>& RESEs, MRERawEvent* RE);
  //! Return the number of Compton interactions
  unsigned int NumberOfComptonInteractions(vector<int> AllSimIds);

  //! Shuffle the RESEs around...
  void Shuffle(vector<MRESE*>& RESEs);

  //! Teach the neural network the events
  void Teach();

  // private methods:
 private:



  // protected members:
 protected:
  //! The neural network determining the sequence - array over energy intervals and sequence lengths 
  vector<vector<MNeuralNetworkBackpropagation>> m_SequenceNNs;
  //! The neural network determining the quality of the event - array over energy intervals and sequence lengths 
  vector<vector<MNeuralNetworkBackpropagation>> m_QualityNNs;

  //! A stored list of events as NN IO as a function of energy and seuqence --- quality NN
  vector<vector<MNeuralNetworkIOStore>> m_QualityNNIOStore;
  //! A stored list of events as NN IO as a function of energy and seuqence --- sequence NN
  vector<vector<MNeuralNetworkIOStore>> m_SequenceNNIOStore;

  //! All possible Permutations for fast access:
  vector<vector<vector<unsigned int>>> m_Permutator;

  //!  Best quality ratio of all epochs using only the verification data
  vector<vector<double>> m_SequenceBestVerificationDataRatio;
  //!  Best quality ratio of all epochs using only the verification data
  vector<vector<double>> m_QualityBestVerificationDataRatio;


  double m_xMin;
  double m_xInt;
  
  double m_yMin;
  double m_yInt;
  
  double m_zMin;
  double m_zInt;

  double m_MaximumDistance;


  unsigned int m_EventsToStore;

  vector<double> m_EnergyMin;
  vector<double> m_EnergyMax;

  // Flags indicating which dimensions to use:
  bool m_UseRawData;
  bool m_UseDistances;
  bool m_UseInteractionProbabilities;
  bool m_UseComptonScatterProbabilities;
  bool m_UseAbsorptions;
  bool m_UseComptonScatterAngles;
  bool m_UseDPhiCriterion;

  //! Desired output value for good
  const double m_GoodValue = 0.1;
  //! Desired output value for bad
  const double m_BadValue = 0.9;
  
#ifdef ___CLING___
 public:
  ClassDef(MResponseMultipleComptonNeuralNet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
