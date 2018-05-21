/*
 * MERCSRNeuralNetwork.h
 *
 * Copyright (C) 1998-2007 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERCSRNeuralNetwork__
#define __MERCSRNeuralNetwork__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <vector>
#include <map>
using namespace std;

// Root libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MERCSR.h"
#include "MRawEventList.h"
#include "MComptonEvent.h"
#include "MNeuralNetworkBackpropagation.h"
#include "MVector.h"

// Forward declarations:
class MRESE;
class MRETrack;
class MGeometryRevan;

////////////////////////////////////////////////////////////////////////////////


class MERCSRNeuralNetwork : public MERCSR
{
  // public interface:
 public:
  MERCSRNeuralNetwork();
  virtual ~MERCSRNeuralNetwork();

  virtual bool SetParameters(MString FileName,
                             MGeometryRevan* Geometry,
                             double ThresholdMin = 0.0,
                             double ThresholdMax = 0.5,
                             int MaxNHits = 5,
                             bool GuaranteeStartD1 = true,
                             bool CreateOnlyPermutations = false);


  virtual MString ToString(bool CoreOnly = false) const;

  virtual int ComputeAllQualityFactors(MRERawEvent* RE);

  // protected methods:
 protected:
  //MERCSRNeuralNetwork() {};
  //MERCSRNeuralNetwork(const MERCSRNeuralNetwork& ERCSR) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //unsigned int m_UseAbsorptionsUpTo;

  MString m_FileName;

  //! The neural network determining the sequence - array over energy intervals and sequence lengths
  vector<vector<MNeuralNetworkBackpropagation> > m_SequenceNNs;
  //! The neural network determining the quality of the event - array over energy intervals and sequence lengths
  vector<vector<MNeuralNetworkBackpropagation> > m_QualityNNs;

  //! All possible Permutations for fast access:
  vector<vector<vector<unsigned int> > > m_Permutator;

  double m_xMin;
  double m_xInt;

  double m_yMin;
  double m_yInt;

  double m_zMin;
  double m_zInt;

  double m_MaximumDistance;

  vector<double> m_EnergyMin;
  vector<double> m_EnergyMax;


  // Flags indicating which dimensions to use:
  bool m_UseRawData;
  bool m_UseDistances;
  bool m_UseInteractionProbabilities;
  bool m_UseComptonScatterProbabilities;
  ///bool m_UseAbsorptions;
  bool m_UseComptonScatterAngles;
  bool m_UseDPhiCriterion;

#ifdef ___CLING___
 public:
  ClassDef(MERCSRNeuralNetwork, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
