/*
 * MERQualityDataSet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERQualityDataSet__
#define __MERQualityDataSet__


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
class MERQualityDataSet
{
  // public interface:
 public:
   //! Default constructor
   MERQualityDataSet();
   //! Standard constructor - initialize for the given sequence length
   MERQualityDataSet(unsigned int MaxSequenceLength, bool UsePathToFirstIA = false);
   //! Default destuctor 
  virtual ~MERQualityDataSet();

  //! Create data set for the given sequence length, 2..N
  void Initialize(unsigned int MaxSequenceLength, bool UsePathToFirstIA = false);
  
  //! Create trees
  //! Tree must be deleted afterwards
  TTree* CreateTree(MString Name);
  
  //! Create readers
  //! Reader must be deleted afterwards
  TMVA::Reader* CreateReader();
  
  //! Fill the data sets from RESEs
  void FillEventData(Long64_t ID, vector<MRESE*>& SequencedRESEs, MDGeometryQuest* Geometry);
  
  //! Fill the evaluation section, whether the event is competely absorbed
  void FillEvaluationIsCompletelyAborbed(bool IsCompletelyAborbed);
  //! Fill the evaluation section, whether the event is competely absorbed
  void FillEvaluationIsReconstructable(bool IsReconstructable);
  //! Fill the evaluation section, about the zenith angle of the incoming gamma ray
  void FillEvaluationZenithAngle(Float_t ZenithAngle);
  
  
  //! Simulation ID -- might overflow but this is just for diagnostics
  Int_t m_SimulationID;
  //! Sequence length
  Int_t m_SequenceLength;
  //! Measured energies
  vector<Float_t> m_Energies;
  //! Measured positions x
  vector<Float_t> m_PositionsX;
  //! Measured positions y
  vector<Float_t> m_PositionsY;
  //! Measured positions z
  vector<Float_t> m_PositionsZ;
  //! Interaction distances 
  vector<Float_t> m_InteractionDistances;
  //! The Compton scatter angles 
  vector<Float_t> m_CosComptonScatterAngles;
  //! The Compton scatter probability (KN)
  vector<Float_t> m_KleinNishinaProbability;
  //! The Compton scatter scatter angle difference
  vector<Float_t> m_CosComptonScatterAngleDifference;
  //! The total absorption probabilities
  vector<Float_t> m_AbsorptionProbabilities;
  //! Closest Nadir approach of the Compton cone
  Float_t m_MinimumNadirAngle;
  //! The absorption probability samples along the path
  vector<Float_t> m_PathSamples;
  
  //! Has the event been completey absorbed
  Int_t m_EvaluationIsCompletelyAborbed;
  //! Is the event reconstructabe
  Int_t m_EvaluationIsReconstructable;
  //! The Zenith angle
  Float_t m_EvaluationZenithAngle;
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:  
  //! The maximum sequence length
  unsigned int m_MaximumSequenceLength;
   
  //! Use path to first IA
  bool m_UsePathToFirstIA;
  //! The number of samples along the path
  unsigned int m_NumberOfPathSamples;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MERQualityDataSet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
