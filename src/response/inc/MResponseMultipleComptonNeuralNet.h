/*
 * MResponseMultipleComptonNeuralNet.h
 *
 * Copyright (C) 2004-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
#include "MNeuralNetworkBackpropagation.h"

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
  vector<vector<MNeuralNetworkBackpropagation> > m_SequenceNNs;
  //! The neural network determining the quality of the event - array over energy intervals and sequence lengths 
  vector<vector<MNeuralNetworkBackpropagation> > m_QualityNNs;

  //! A stored list of events as NN IO --- quality NN
  vector<vector<list<MNeuralNetworkIOStore> > > m_QualityNNIOStore;

  //! A stored list of events as NN IO --- sequence NN
  vector<vector<list<MNeuralNetworkIOStore> > > m_SequenceNNIOStore;

  //! All possible Permutations for fast access:
  vector<vector<vector<unsigned int> > > m_Permutator;

  //!  Best quality ratio of all epochs using only the verification data
  vector<vector<double> > m_SequenceBestVerificationDataRatio;


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


#ifdef ___CINT___
 public:
  ClassDef(MResponseMultipleComptonNeuralNet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
