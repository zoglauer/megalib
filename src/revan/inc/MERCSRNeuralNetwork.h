/*
 * MERCSRNeuralNetwork.h
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
#include "MRawEventIncarnations.h"
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
