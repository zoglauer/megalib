/*
 * MERCSRDataSet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRDataSet__
#define __MERCSRDataSet__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MERCSRDataSet
{
  // public interface:
 public:
   //! Default constructor
   MERCSRDataSet();
   //! Standard constructor - initialize for the given sequence length
   MERCSRDataSet(unsigned int SequenceLength);
   //! Default destuctor 
  virtual ~MERCSRDataSet();

  // Create for the given sequence length, 2..N
  void Initialize(unsigned int SequenceLength);
  
  // Create trees
  void CreateTrees(vector<TTree*>& GoodTree, vector<TTree*>& BadTree);
  
  // Create readers
  void CreateReaders(vector<TMVA::Reader*>& Readers);
  
  // Fill the data sets from RESEs
  void Fill(Long64_t ID, vector<MRESE*>& SequencedRESEs, MDGeometryQuest* Geometry);
  
  //! Simulation ID
  vector<Long64_t> m_SimulationIDs;
  //! Measured energies
  vector<vector<Float_t>> m_Energies;
  //! Measured positions x
  vector<vector<Float_t>> m_PositionsX;
  //! Measured positions y
  vector<vector<Float_t>> m_PositionsY;
  //! Measured positions z
  vector<vector<Float_t>> m_PositionsZ;
  //! Interaction distances 
  vector<vector<Float_t>> m_InteractionDistances;
  //! The Compton scatter angles 
  vector<vector<Float_t>> m_CosComptonScatterAngles;
  //! The Compton scatter probability (KN)
  vector<vector<Float_t>> m_KleinNishinaProbability;
  //! The Compton scatter scatter angle difference
  vector<vector<Float_t>> m_CosComptonScatterAngleDifference;
  //! The total absorption probabilities
  vector<vector<Float_t>> m_AbsorptionProbabilities;
  //! The average column density to first IA
  vector<Float_t> m_AbsorptionProbabilityToFirstIAAverage;
  //! The peak column density to first IA
  vector<Float_t> m_AbsorptionProbabilityToFirstIAMaximum;
  //! The minimum column density to first IA
  vector<Float_t> m_AbsorptionProbabilityToFirstIAMinimum;
  //! The Zenith angle
  vector<Float_t> m_ZenithAngle;
  //! The Nadir angle
  vector<Float_t> m_NadirAngle;  
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The event ID 
  long m_EventID;
  //! The index map of the first two interactions
  vector<pair<int, int>> m_Indices;
  //! The average column density to first IA
  vector<Float_t> m_StoreAbsorptionProbabilityToFirstIAAverage;
  //! The peak column density to first IA
  vector<Float_t> m_StoreAbsorptionProbabilityToFirstIAMaximum;
  //! The minimum column density to first IA
  vector<Float_t> m_StoreAbsorptionProbabilityToFirstIAMinimum;
  //! The Zenith angle
  vector<Float_t> m_StoreZenithAngle;
  //! The Nadir angle
  vector<Float_t> m_StoreNadirAngle;  
  
  


#ifdef ___CINT___
 public:
  ClassDef(MERCSRDataSet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
