/*
 * MERCSRNeuralNetwork.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MERCSRNeuralNetwork
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRNeuralNetwork.h"

// Standard libs:
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MFile.h"
#include "MTokenizer.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"
#include "MERCSRBayesian.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCSRNeuralNetwork)
#endif


////////////////////////////////////////////////////////////////////////////////


  MERCSRNeuralNetwork::MERCSRNeuralNetwork() : MERCSR()
{
  // Construct an instance of MERCSRNeuralNetwork
}


////////////////////////////////////////////////////////////////////////////////


MERCSRNeuralNetwork::~MERCSRNeuralNetwork()
{
  // Delete this instance of MERCSRNeuralNetwork
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRNeuralNetwork::SetParameters(MString FileName, 
                                        MGeometryRevan* Geometry, 
                                        double QualityFactorMin, 
                                        double QualityFactorMax, 
                                        int MaxNInteractions,
                                        bool GuaranteeStartD1,
                                        bool CreateOnlyPermutations)
{
  if (MERCSR::SetParameters(Geometry, 
                            QualityFactorMin, 
                            QualityFactorMax, 
                            MaxNInteractions,
                            GuaranteeStartD1,
                            CreateOnlyPermutations) == false) return false;

  m_FileName = FileName;

  if (m_CreateOnlyPermutations == false) {

    MString Suffix = ".nn.erm";
    
    if (MFile::FileExists(FileName) == false ||
        FileName.EndsWith(Suffix) == false) {
      mgui<<"Files for neural network Compton patter recognition not found: \""<<FileName<<"\""<<error;
      return false;
    }

    m_MaxNInteractions = 0;

    MParser Parser;
    Parser.Open(FileName);
    for (unsigned int l = 0; l < Parser.GetNLines(); ++l) {
      MTokenizer* T = Parser.GetTokenizerAt(l);
      if (T->GetNTokens() <= 1) continue;

      cout<<l<<":"<<T->ToString()<<endl;
      if (T->GetTokenAt(0) == "MI") {
        m_EnergyMin.clear();
        m_EnergyMin = T->GetTokenAtAsDoubleVector(1);
      } else if (T->GetTokenAt(0) == "MA") {
        m_EnergyMax.clear();
        m_EnergyMax = T->GetTokenAtAsDoubleVector(1);
      } else if (T->GetTokenAt(0) == "DX") {
        m_xMin = T->GetTokenAtAsDouble(1);
        m_xInt = T->GetTokenAtAsDouble(2);
      } else if (T->GetTokenAt(0) == "DY") {
        m_yMin = T->GetTokenAtAsDouble(1);
        m_yInt = T->GetTokenAtAsDouble(2);
      } else if (T->GetTokenAt(0) == "DZ") {
        m_zMin = T->GetTokenAtAsDouble(1);
        m_zInt = T->GetTokenAtAsDouble(2);
      } else if (T->GetTokenAt(0) == "DI") {
        if (T->GetTokenAt(1) == "UseRawData") {
          m_UseRawData = T->GetTokenAtAsBoolean(2);
        } else if (T->GetTokenAt(1) == "UseDistances") {
          m_UseDistances = T->GetTokenAtAsBoolean(2);
        } else if (T->GetTokenAt(1) == "UseInteractionProbabilities") {
          m_UseInteractionProbabilities = T->GetTokenAtAsBoolean(2);
        } else if (T->GetTokenAt(1) == "UseComptonScatterProbabilities") {
          m_UseComptonScatterProbabilities = T->GetTokenAtAsBoolean(2);
        } else if (T->GetTokenAt(1) == "UseComptonScatterAngles") {
          m_UseComptonScatterAngles = T->GetTokenAtAsBoolean(2);
        } else if (T->GetTokenAt(1) == "UseDPhiCriterion") {
          m_UseDPhiCriterion = T->GetTokenAtAsBoolean(2);
        }
      } else if (T->GetTokenAt(0) == "NS") {
        if (T->GetTokenAtAsInt(2) > m_MaxNInteractions) {
          m_MaxNInteractions = T->GetTokenAtAsInt(2);
        }
      }
    }

    m_SequenceNNs.resize(m_EnergyMin.size());
    m_QualityNNs.resize(m_EnergyMin.size());
    for (unsigned int e = 0; e < m_EnergyMin.size(); ++e) {
      m_SequenceNNs[e].resize(MaxNInteractions+1);
      m_QualityNNs[e].resize(MaxNInteractions+1);
    }

    for (unsigned int l = 0; l < Parser.GetNLines(); ++l) {
      MTokenizer* T = Parser.GetTokenizerAt(l);
      if (T->GetNTokens() <= 1) continue;

      if (T->GetTokenAt(0) == "NS") {
        unsigned int e = T->GetTokenAtAsUnsignedInt(1);
        unsigned int s = T->GetTokenAtAsUnsignedInt(2);
        MString NNFileName = T->GetTokenAt(3);
        NNFileName = MFile::GetDirectoryName(FileName) + "/" + NNFileName;
        if (MFile::FileExists(NNFileName) == true) {
          if (m_SequenceNNs[e][s].Load(NNFileName) == false) {
            cout<<"Unable to load neural network file: "<<NNFileName<<endl;
            return false;
          } else {
            cout<<"Loaded "<<NNFileName<<endl;
          }
        } else {
          cout<<"Neural network file does not exist: "<<NNFileName<<endl;
          return false;
        }
      } else if (T->GetTokenAt(0) == "NQ") {
        unsigned int e = T->GetTokenAtAsUnsignedInt(1);
        unsigned int s = T->GetTokenAtAsUnsignedInt(2);
        MString NNFileName = T->GetTokenAt(3);
        NNFileName = MFile::GetDirectoryName(FileName) + "/" + NNFileName;
        if (MFile::FileExists(NNFileName) == true) {
          if (m_QualityNNs[e][s].Load(NNFileName) == false) {
            cout<<"Unable to load neural network file: "<<NNFileName<<endl;
            return false;
          } else {
            cout<<"Loaded "<<NNFileName<<endl;
          }
        } else {
          cout<<"Neural network file does not exist: "<<NNFileName<<endl;
          return false;
        }
      }        
    }

    // Build permutation matrix:
    m_Permutator.resize(MaxNInteractions+1);
    for (unsigned int i = 2; i <= (unsigned int) MaxNInteractions; ++i) {
      vector<vector<unsigned int> > Permutations;
      vector<unsigned int> Indices;
      for (unsigned int j = 0; j < i; ++j) {
        Indices.push_back(j);
      }
      vector<unsigned int>::iterator Begin = Indices.begin();
      vector<unsigned int>::iterator End = Indices.end();
      do {
        Permutations.push_back(Indices);
      } while (next_permutation(Begin, End));
      m_Permutator[i] = Permutations;
    }

    m_MaximumDistance = sqrt(m_xInt*m_xInt + m_yInt*m_yInt + m_zInt*m_zInt);
  }

  cout<<"Maximum number of interactions allowed: "<<m_MaxNInteractions<<endl;

  //   if (Geometry->AreAbsorptionsPresent() == false) {
  //     mgui<<"Analyzing events via Klein-Nishina and photo effect for the Bayesian"<<endl;
  //     mgui<<"event reconstruction requires absorption probabilities in Geomega."<<endl;
  //     mgui<<"However, Geomega indicates that those have not been proberly loaded."<<endl;
  //     mgui<<"Aborting analysis"<<error;
  //     return false;
  //   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MERCSRNeuralNetwork::ComputeAllQualityFactors(MRERawEvent* RE)
{
  // In contrast to the classic & Bayesian approaches, we are not looping
  // over all different permutations
  // Therefore we have only one or no permutation as result ans all analysis
  // is performed in this function instead of "ComputeQualityFactor"

  double QualityFactor = c_CSRFailed;
  m_QualityFactors.clear();

  RE->Shuffle();

  vector<MRESE*> RESEs(RE->GetNRESEs());
  for (int i = 0; i < RE->GetNRESEs(); ++i) {
    RESEs[i] = RE->GetRESEAt(i);
  }

  if (RESEs.size() <= 1) {
    mdebug<<"MERCSRNeuralNetwork: Not enough hits: "<<RESEs.size()<<endl;
    return 0;
  }

  mdebug<<RE->ToString()<<endl;

  unsigned int EnergyBin = numeric_limits<unsigned int>::max();
  for (unsigned int i = 0; i < m_EnergyMax.size(); ++i) {
    if (RE->GetEnergy() >= m_EnergyMin[i] && RE->GetEnergy() <= m_EnergyMax[i]) {
      EnergyBin = i;
      break;
    }
  }
  if (EnergyBin == numeric_limits<unsigned int>::max()) {
    mout<<"Energy not within NN limits: "<<RE->GetEnergy()<<endl;
    EnergyBin = m_EnergyMax.size() -1;
  }
      
  // Check if complete sequence is ok:
  unsigned int SequenceBin = (unsigned int) RE->GetNRESEs();

  if (SequenceBin >= 2 && SequenceBin <= (unsigned int) m_MaxNInteractions) {
      
    bool InputValid = true;
    unsigned int PosSequenceNN = 0;
    unsigned int PosQualityNN = 0;

    // (a) Raw data:
    if (m_UseRawData == true) {
      for (int r = 0; r < RE->GetNRESEs(); ++r) {
        double x = (RESEs[r]->GetPosition().X() - m_xMin)/m_xInt;
        double y = (RESEs[r]->GetPosition().Y() - m_yMin)/m_yInt;
        double z = (RESEs[r]->GetPosition().Z() - m_zMin)/m_zInt;
        double E = 0.1+0.8*(RESEs[r]->GetEnergy() - m_EnergyMin.front())/(m_EnergyMax[EnergyBin] - m_EnergyMin.front());
        
        if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, x) == false) InputValid = false;
        if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, y) == false) InputValid = false;
        if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, z) == false) InputValid = false;
        if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, E) == false) InputValid = false;
        if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, x) == false) InputValid = false;
        if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, y) == false) InputValid = false;
        if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, z) == false) InputValid = false;
        if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, E) == false) InputValid = false;
      }
    }


    // (b) distances:
    if (m_UseDistances == true) {
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        for (unsigned int r = 0; r < SequenceBin-1; ++r) {
          double Distance = (RESEs[m_Permutator[SequenceBin][i][r]]->GetPosition() - RESEs[m_Permutator[SequenceBin][i][r+1]]->GetPosition()).Mag();
          
          cout<<"Distance: S"<<i<<": ";
          for  (unsigned int re = 0; re < RESEs.size(); ++re) {
            cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
            if (re < RESEs.size()-1) {
            cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" Distance="<<Distance;
          // Normalize between 0.1 .. 0.9;
          Distance = 0.8*Distance/m_MaximumDistance+0.1;
          cout<<" --> "<<Distance<<endl;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Distance) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Distance) == false) InputValid = false;	  
        }
      }
    }


    // (c) absorption/scatter probabilities:
    if (m_UseInteractionProbabilities == true) {
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        double Energy = RE->GetEnergy();
        for (unsigned int r = 0; r < SequenceBin-1; ++r) {
          double Prob = 0.5;
          Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
          // For larger events, this is too time consuming
          //if (RESEs.size() <= 2) {
            if (r == SequenceBin-2) {
              Prob = CalculateTotalDistance(RESEs[m_Permutator[SequenceBin][i][r]]->GetPosition(), RESEs[m_Permutator[SequenceBin][i][r+1]]->GetPosition(), Energy);
            } else {
              Prob = CalculateComptonDistance(RESEs[m_Permutator[SequenceBin][i][r]]->GetPosition(), RESEs[m_Permutator[SequenceBin][i][r+1]]->GetPosition(), Energy);
            }
            //}
          
          cout<<"Prob's: S"<<i<<": ";
          for  (unsigned int re = 0; re < RESEs.size(); ++re) {
            cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
            if (re < RESEs.size()-1) {
              cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" Prob="<<Prob;
          // Normalize between 0.1 .. 0.9;
          Prob = 0.8*Prob+0.1;
          cout<<" --> "<<Prob<<endl;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Prob) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Prob) == false) InputValid = false;	  
        }
      }
    }

    // (d) Compton scatter angles
    //     (+) phi's via energy:
    if (m_UseComptonScatterAngles == true) {
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        double Energy = RE->GetEnergy();
        for (unsigned int r = 0; r < SequenceBin-1; ++r) {
          double CosPhi = MERCSRBayesian::CalculateCosPhiE(RESEs[m_Permutator[SequenceBin][i][r]], Energy);
          Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
          
          cout<<"Phi's: S"<<i<<": ";
          for  (unsigned int r = 0; r < RESEs.size(); ++r) {
            cout<<RESEs[m_Permutator[SequenceBin][i][r]]->GetID();
            if (r < RESEs.size()-1) {
              cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" cosPhi="<<CosPhi;
          // Normalize between 0.1 .. 0.9;
          CosPhi = 0.2+0.6/2.0*(CosPhi+1);
          if (CosPhi > 0.9) CosPhi = 0.9;
          if (CosPhi < 0.1) CosPhi = 0.1;
          cout<<" --> "<<CosPhi<<endl;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, CosPhi) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, CosPhi) == false) InputValid = false;	  
        }
      }

      //   (+) phi's via geometry:
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        for (unsigned int r = 1; r < SequenceBin-1; ++r) {
          double CosPhi = MERCSRBayesian::CalculateCosPhiG(RESEs[m_Permutator[SequenceBin][i][r-1]], 
                                                           RESEs[m_Permutator[SequenceBin][i][r]], 
                                                           RESEs[m_Permutator[SequenceBin][i][r+1]]);
          
          cout<<"Phi's (geo): S"<<i<<": ";
          for  (unsigned int re = 0; re < RESEs.size(); ++re) {
            cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
            if (re < RESEs.size()-1) {
              cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" cosPhi="<<CosPhi;
          // Normalize between 0.1 .. 0.9;
          CosPhi = 0.2+0.6/2.0*(CosPhi+1);
          if (CosPhi > 0.9) CosPhi = 0.9;
          if (CosPhi < 0.1) CosPhi = 0.1;
          cout<<" --> "<<CosPhi<<endl;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, CosPhi) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, CosPhi) == false) InputValid = false;	  
        }
      }
    }



    // (e) Compton scatter propabilities:
    if (m_UseComptonScatterProbabilities == true) {
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        double Energy = RE->GetEnergy();
        for (unsigned int r = 0; r < SequenceBin-1; ++r) {
          double Phi = MERCSRBayesian::CalculatePhiEInDegree(RESEs[m_Permutator[SequenceBin][i][r]], Energy);
          if (Phi < 0) Phi = 0;
          if (Phi > 180) Phi = 180;
          double Prob = MComptonEvent::GetKleinNishinaNormalizedByArea(Energy, Phi*c_Rad);
          
          Energy -= RESEs[m_Permutator[SequenceBin][i][r]]->GetEnergy();
          
          cout<<"Compton prob's: S"<<i<<": ";
          for  (unsigned int re = 0; re < RESEs.size(); ++re) {
            cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
            if (re < RESEs.size()-1) {
              cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" Prob="<<Prob;
          // Normalize between 0.1 .. 0.9;
          Prob = 0.1 + 0.8*Prob;
          if (Prob > 0.9) Prob = 0.9;
          if (Prob < 0.1) Prob = 0.1;
          cout<<" --> "<<Prob<<endl;
          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, Prob) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, Prob) == false) InputValid = false;	  
        }
      }
    }

    // (g) dphi's:
    if (m_UseDPhiCriterion == true) {
      if (RESEs.size() >= 3) {
        for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
          double Energy = RE->GetEnergy();
          double AvgDPhi = 0.0;
          for (unsigned int r = 1; r < RESEs.size()-1; ++r) {
            Energy -=RESEs[m_Permutator[SequenceBin][i][r-1]]->GetEnergy();
            double DPhi = MERCSRBayesian::CalculateDCosPhi(RESEs[m_Permutator[SequenceBin][i][r-1]], 
                                                           RESEs[m_Permutator[SequenceBin][i][r]], 
                                                           RESEs[m_Permutator[SequenceBin][i][r+1]], 
                                                           Energy);
            //if (VerboseLevel > 1) {
              cout<<DPhi<<" - ";
              //}
            AvgDPhi += DPhi*DPhi; 
              
            DPhi = fabs(DPhi);
            //DPhi *= 0.75;
            //DPhi += 0.2;
            //if (DPhi > 0.8) DPhi = 0.8; 
            DPhi = 2*atan(DPhi)/c_Pi;
            //if (VerboseLevel > 1) {
              cout<<" - training value "<<r<<": "<<DPhi<<endl;
              //}
            if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, DPhi) == false) InputValid = false;
            if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, DPhi) == false) InputValid = false;
          }
          AvgDPhi = fabs(sqrt(AvgDPhi));

          cout<<"dPhi-criterion: S"<<i<<": ";
          for  (unsigned int re = 0; re < RESEs.size(); ++re) {
            cout<<RESEs[m_Permutator[SequenceBin][i][re]]->GetID();
            if (re < RESEs.size()-1) {
              cout<<" -> ";
            } else {
              cout<<" : ";
            }
          }
          cout<<" dPhi="<<AvgDPhi;

          // Normalize for input into NN
          //AvgDPhi *= 0.75;
          //AvgDPhi += 0.2;
          //if (AvgDPhi > 0.8) AvgDPhi = 0.8; 
          AvgDPhi = 2*atan(AvgDPhi)/c_Pi;
          cout<<" - input value: "<<AvgDPhi<<endl;

          if (m_SequenceNNs[EnergyBin][SequenceBin].SetInput(PosSequenceNN++, AvgDPhi) == false) InputValid = false;
          if (m_QualityNNs[EnergyBin][SequenceBin].SetInput(PosQualityNN++, AvgDPhi) == false) InputValid = false;
          //cout<<p<<": "<<AvgDPhi<<endl;
        } 
      }
    }      

    //cout<<m_SequenceNNs[EnergyBin][SequenceBin].GetIOStore().ToString()<<endl;

    // We only continue if the input was valid!
    if (InputValid == true) {
      m_QualityNNs[EnergyBin][SequenceBin].Run();
      m_SequenceNNs[EnergyBin][SequenceBin].Run();

      // Output neuron 1 contains the quality factor --- the larger the better
      QualityFactor = m_QualityNNs[EnergyBin][SequenceBin].GetOutput(0);

      // The output neuron with the smallest value is the correct one
      int CorrectSequence = 0;
      double HighestValue = numeric_limits<double>::max();
      for (int i = 0; i < TMath::Factorial(SequenceBin); ++i) {
        if (m_SequenceNNs[EnergyBin][SequenceBin].GetOutput(i) < HighestValue) {
          CorrectSequence = i;
          HighestValue = m_SequenceNNs[EnergyBin][SequenceBin].GetOutput(i);
        }
      }

      vector<MRESE*> CorrectPermutation;
      for (unsigned int i = 0; i < m_Permutator[SequenceBin].size(); ++i) {
        if (CorrectSequence == 0) {
          for (unsigned int r = 0; r < m_Permutator[SequenceBin][i].size(); ++r) {
            CorrectPermutation.push_back(RESEs[m_Permutator[SequenceBin][i][r]]);
          }
          break;
        }
        CorrectSequence--;
      }

      m_QualityFactors.insert(map<double, vector<MRESE*>, less_equal<double> >::value_type(QualityFactor, CorrectPermutation));
      return 1;
    } else {
      cout<<"Input not valid --- are your normalizations correct?"<<endl;
      //massert(false);
      return 0;
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSRNeuralNetwork::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# CSR - Neural net options:"<<endl;
  out<<"# "<<endl;
  out<<"# Filename:                        "<<m_FileName<<endl;
  out<<MERCSR::ToString(true);
  out<<"# "<<endl;
  
  return out.str().c_str();
}

// MERCSRNeuralNetwork.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
