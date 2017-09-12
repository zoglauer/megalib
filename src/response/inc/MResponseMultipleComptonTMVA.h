/*
 * MResponseMultipleComptonTMVA.h
 *
 * Copyright (C) 2004-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseMultipleComptonTMVA__
#define __MResponseMultipleComptonTMVA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MRESE.h"
#include "MRERawEvent.h"
#include "MRETrack.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseMultipleComptonTMVA : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseMultipleComptonTMVA();
  //! Default destructor
  virtual ~MResponseMultipleComptonTMVA();
  
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

  //! Store one element
  bool Store(unsigned int SequenceLength, bool IsGood);
  
  
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
   
  //! The data storage for good events, one per sequence length
  vector<TTree*> m_TreeGood;
   
  //! The data storage for bad events
  vector<TTree*> m_TreeBad;
   
  // The TTree input data for each possibility, one per sequence length:
  
  //! Simulation ID
  vector<Long64_t> m_SimulationIDs;
  //! Measured energies
  vector<vector<double>> m_Energies;
  //! Measured positions x
  vector<vector<double>> m_PositionsX;
  //! Measured positions y
  vector<vector<double>> m_PositionsY;
  //! Measured positions z
  vector<vector<double>> m_PositionsZ;
  //! Interaction distances 
  vector<vector<double>> m_InteractionDistances;
  //! The Compton scatter angles 
  vector<vector<double>> m_CosComptonScatterAngles;
  //! The Compton scatter probability (KN)
  vector<vector<double>> m_KleinNishinaProbability;
  //! The Compton scatter scatter angle difference
  vector<vector<double>> m_CosComptonScatterAngleDifference;
  //! The total absorption probabilities
  vector<vector<double>> m_AbsorptionProbabilities;
  //! The average column desnity to first IA
  vector<double> m_AbsorptionProbabilityToFirstIAAverage;
  //! The peak column desnity to first IA
  vector<double> m_AbsorptionProbabilityToFirstIAMaximum;
  //! The minimum column desnity to first IA
  vector<double> m_AbsorptionProbabilityToFirstIAMinimum;
  //! The Zenith angle
  vector<double> m_ZenithAngle;
  //! The Nadir angle
  vector<double> m_NadirAngle;  
  
  //! All possible Permutations for fast access:
  //! Build the following way: [Sequence length] [Permuation ID] [Index]
  vector<vector<vector<unsigned int>>> m_Permutator;
  

  
#ifdef ___CINT___
 public:
  ClassDef(MResponseMultipleComptonTMVA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
