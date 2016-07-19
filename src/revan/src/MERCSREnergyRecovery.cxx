/*
 * MERCSREnergyRecovery.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Elena Novikova, Eric Wulf.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * the copyright holder(s).
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MERCSREnergyRecovery
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSREnergyRecovery.h"

// Standard libs:
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TObjArray.h"
#include "TMath.h"
#include <TSystem.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MAssert.h"
#include "MFile.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"
#include "MRawEventList.h"


////////////////////////////////////////////////////////////////////////////////


const int MERCSREnergyRecovery::c_TestStatisticsChiSquareWithoutErrors = 1;
const int MERCSREnergyRecovery::c_TestStatisticsChiSquareWithErrors = 2;
const int MERCSREnergyRecovery::c_TestStatisticsChiSquareProbability = 3;

const int MERCSREnergyRecovery::c_TestStatisticsFirst = 1;
const int MERCSREnergyRecovery::c_TestStatisticsLast = 3;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERCSREnergyRecovery)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCSREnergyRecovery::MERCSREnergyRecovery() : MERCSR()
{
  // Construct an instance of MERCSREnergyRecovery

  m_TestStatisticsType = c_TestStatisticsChiSquareWithErrors;
  m_MaxSearchSequence = numeric_limits<int>::max();
  m_MaxSearchSequence = 5;
}


////////////////////////////////////////////////////////////////////////////////


MERCSREnergyRecovery::~MERCSREnergyRecovery()
{
  // Delete this instance of MERCSREnergyRecovery
}


////////////////////////////////////////////////////////////////////////////////


void MERCSREnergyRecovery::ModifyEventList()
{
/*
  // Split all tracks, which are below a certain quality level
  // and add a new event for them:

  // cout<<"void MERCSREnergyRecovery::ModifyEventList()"<<endl;


  // Stays here until we decide we need a GUI element
  double SplitLevel = 0; //numeric_limits<double>::max();

  int e_max = m_List->GetNRawEvents();
  for (int e = 0; e < e_max; ++e) {
    MRERawEvent* First = m_List->GetRawEventAt(e);

    // To make sure everything is fine, create a new list:
    MRawEventList* NewList = new MRawEventList();
    NewList->AddRawEvent(First);

    for (int n = 0; n < First->GetNRESEs(); ++n) {
      int ID = First->GetRESEAt(n)->GetID();
      int r_max = NewList->GetNRawEvents();
      for (int r = 0; r < r_max; ++r) {
        MRERawEvent* RE = NewList->GetRawEventAt(r);
        if (RE->ContainsRESE(ID) == true && RE->GetRESE(ID)->GetType() == MRESE::c_Track) {
          if (dynamic_cast<MRETrack*>(RE->GetRESE(ID))->GetScore() >= SplitLevel) {
            MRERawEvent* NewRE = RE->Duplicate();
            MRESE* Track = NewRE->GetRESE(ID);
            NewRE->RemoveRESE(Track);
            NewRE->CompressRESEs();
            for (int t = 0; t < Track->GetNRESEs(); ++t) {
              MRESE* Sub = Track->RemoveRESEAt(t);
              Sub->RemoveAllLinks();
              NewRE->AddRESE(Sub);
            }
            delete Track;
            NewList->AddRawEvent(NewRE);
          }
        }
      }
    }
    
    // Copy all but the first raw event from the new to the old list:
    int r_max = NewList->GetNRawEvents();
    for (int r = 1; r < r_max; ++r) {
      m_List->AddRawEvent(NewList->GetRawEventAt(r));
    }
    delete NewList;
  }

//   // Check what we have:
//   cout<<"Number of events: "<<m_List->GetNRawEvents()<<endl;
//   for (int e = 0; e < m_List->GetNRawEvents(); ++e) {
//     MRERawEvent* First = m_List->GetRawEventAt(e);
//     cout<<First->ToString()<<endl;
//   }
*/

  return;
}




////////////////////////////////////////////////////////////////////////////////


double MERCSREnergyRecovery::ComputeQualityFactor(vector<MRESE*>& RESEs)
{
  mdebug<<"CSR-E seq: "<<endl;

  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    mdebug<<RESEs[r]->GetID();
    if ( r < RESEs.size()-1) {
      mdebug<<" - ";
    } else {
      mdebug<<endl;
    }
  }

  double TS = 0;
  double Eavg = 0;

  CalculateQF(RESEs, TS, Eavg);

  return TS;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSREnergyRecovery::CalculateQF(vector<MRESE*>& RESEs, double& CQF, double& Eavg)
{
  // Here the test statistics always contains an energy and a 
  // Klein-Nishina criteria...
  // Initial version does not take care of tracks...
  // ToDo: Add polarization effects...

  // 
  CQF = c_CSRFailed;
  Eavg = 0;

  // Magic numbers:
  // How much are the calculated and the determined full energy are allowed 
  // to differ (in sigma of energy resolution) so that we call the event completely absorbed:
  static const double MaxEDiffSigma = 2.5;

  double Etot = 0;
  for (unsigned int r = 0; r < RESEs.size(); ++r) {
    Etot += RESEs[r]->GetEnergy();
  }

  if (RESEs.size() == 2) {
    // Take the event if its kinematics is ok. Then the quality factor is 1- normalized Klein-Nishina
    if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), RESEs[1]->GetEnergy()) == true) {
      double Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[0]->GetEnergy(), RESEs[1]->GetEnergy());
      CQF = 1-MComptonEvent::GetKleinNishinaNormalized(Etot, Phi);
      Eavg = Etot; // Assume complete absorption.
    } else {
      CQF = c_CSRFailed;
    } 
  } 
  // Three site events:
  else if (RESEs.size() == 3) {

    // Calculate second Compton scatter angle + error
    double CosPhiA = 
      cos((RESEs[1]->GetPosition() - RESEs[0]->GetPosition()).
          Angle(RESEs[2]->GetPosition() - RESEs[1]->GetPosition()));
    double dCosPhiA = ComputePositionError(RESEs[0], RESEs[1], RESEs[2]);

    if (CosPhiA <= -1 && CosPhiA >= 1) {
      mdebug<<"CSR-Rec (2): CosPhiA out of bounds: "<<CosPhiA<<endl;
      CQF = c_CSRFailed;
      return;
    }
    
    // Estimate the energy + error of the photon:
    double E1 = RESEs[0]->GetEnergy();
    double E2 = RESEs[1]->GetEnergy();

    if (E1 == 0 || E2 == 0) {
      merr<<"CSR-Rec (3): Energies are zero - this error should have been catched elsewhere!"<<endl;
      return;
    }

    double dE1 = RESEs[0]->GetEnergyResolution();
    double dE2 = RESEs[1]->GetEnergyResolution();
    double Eest = E1 + 0.5*(E2 + sqrt(E2*E2 + 4*c_E0*E2/(1-CosPhiA)));
    double dEest = sqrt(dE1*dE1 + 
                 pow(.5+.25/sqrt(E2*E2+4*c_E0*E2/(1-CosPhiA))*(2*E2+4*c_E0/(1-CosPhiA)), 2)*dE2*dE2 +
                 pow(1.0/sqrt(E2*E2+4*c_E0*E2/(1-CosPhiA))*c_E0*E2/(1-CosPhiA)/(1-CosPhiA), 2)*dCosPhiA*dCosPhiA);
    double EDiff = Etot - Eest;

    mdebug<<"CSR-Rec (3): Eest: "<<Eest<<" +- "<<dEest<<" - Etot: "<<Etot<<dCosPhiA<<endl;

    if (Eest+MaxEDiffSigma*dEest < Etot) {
      mdebug<<"CSR-Rec (3): Estimated energy (+ Delta) lower than measured deposits!"<<endl;
      CQF = c_CSRFailed;
      return;
    }

    // Test if we have complete absorption:
    if (fabs(EDiff) < MaxEDiffSigma*dEest) {
      mdebug<<"CSR-Rec (3): Completely absorbed 3-Compton !"<<endl;
      // Test if the completely absorbed event is kinematically correct:
      if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), RESEs[1]->GetEnergy()+RESEs[2]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[1]->GetEnergy(), RESEs[2]->GetEnergy()) == false) {
        mdebug<<"CSR-Rec (3): Kinematics failed!"<<endl;
        CQF = c_CSRFailed;
      } else {
        mdebug<<"CSR-Rec (3): Kinematics ok!"<<endl;

        // The Chi-Square test statistics results in a probability....
        // Ignoring: CQF = (1-TMath::Prob(EDiff*EDiff/dEest/dEest, 1));

        // The CQF consists of three times the normalized cross-section
        double Ei = Eest;
        double Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[0]->GetEnergy(), Ei - RESEs[0]->GetEnergy());
        double KN = MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
      
        Ei = Eest - RESEs[0]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[1]->GetEnergy(), Ei - RESEs[1]->GetEnergy());
        KN *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        // It probably would be good to have a third probability here...

        // Renormalized CQF - in the moment there is not good solution 
        // for a combined incomplete and completely absorbed quality factor...
        CQF = 1 - KN; // Ignores original CQF;
        
        // Store the total energy as average estimated one:
        Eavg = Etot;
      }
    } else {
      // We do have incomplete absorption...
      /*
      // Check Compton kinematics:
      if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), Eest - RESEs[0]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[1]->GetEnergy(), Eest - RESEs[0]->GetEnergy() - RESEs[1]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[2]->GetEnergy(), EDiff) == false) {
        mdebug<<"CSR-Rec (3): Kinematics failed!"<<endl;
        CQF = c_CSRFailed;
      } 
      // All checks passed, lets calculate 
      else {
        mdebug<<"CSR-Rec (3): Kinematics ok!"<<endl;
        CQF = 1; // "passed" energy test (per definition) with Chi-square = 0

        // The CQF consists of three times the normalized cross-section
        double Ei = Eest;
        double Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[0]->GetEnergy(), Ei - RESEs[0]->GetEnergy());
        CQF *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        Ei = Eest - RESEs[0]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[1]->GetEnergy(), Ei - RESEs[1]->GetEnergy());
        CQF *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        Ei = Eest - RESEs[1]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[2]->GetEnergy(), Ei - RESEs[2]->GetEnergy());
        CQF *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
 
        // Renormalized CQF:
        CQF = 1 - CQF;

        // Return the estimated energy:
        Eavg = Eest;
      }
      */
      CQF = c_CSRFailed;
      Eavg = 0.;  
    }
    mdebug<<"CSR-Rec (3): Final quality factor: "<<CQF<<endl;
  } 
  // 4+ events
  else {
    vector<double> Eests;
    vector<double> dEests;

    // Loop over all triplets and calculate the estimated energy
    unsigned int i_max = min(int(RESEs.size()), m_MaxSearchSequence) - 1;
    for (unsigned int i = 1; i < i_max; ++i) {

      double CosPhiA = 
        cos((RESEs[i]->GetPosition() - RESEs[i-1]->GetPosition()).
            Angle(RESEs[i+1]->GetPosition() - RESEs[i]->GetPosition()));
      double dCosPhiA = ComputePositionError(RESEs[i-1], RESEs[i], RESEs[i+1]);

      if (CosPhiA <= -1 && CosPhiA >= 1) {
        mdebug<<"CSR-Rec (4+): CosPhiA out of bounds: "<<CosPhiA<<endl;
        continue;
      }

      double E1 = 0;
      double dE1 = 0;
      for (unsigned int j = 0; j < i; ++j) {
        E1 += RESEs[j]->GetEnergy();
        dE1 += RESEs[j]->GetEnergyResolution()*RESEs[j]->GetEnergyResolution();
      }
      dE1 = sqrt(dE1);

      double E2 = RESEs[i]->GetEnergy();
      double dE2 = RESEs[i]->GetEnergyResolution();
      
      double eMassCosPhi = 4*c_E0/(1-CosPhiA);
      double E2_sq = E2*E2;
      double sqrtBracket = sqrt(E2_sq + eMassCosPhi*E2);
      double Eest = E1 + 0.5*(E2 + sqrtBracket);
      double dEdE2 = .5 + .25 / sqrtBracket * (2*E2 + eMassCosPhi); 
      double dEdCosPhiA = eMassCosPhi*E2/((1-CosPhiA)*4.*sqrtBracket);
      double dEest = sqrt(dE1*dE1 + dEdE2*dEdE2*dE2*dE2 + dEdCosPhiA*dEdCosPhiA*dCosPhiA*dCosPhiA);

      if (dEest == 0) {
        merr<<"CSR-Rec (4+): dEest == 0 --> Do not tell me you do not have any errors!"<<endl;
        continue;
      }
      mdebug<<"CSR-Rec (4+): Estimated energy: "<<Eest<<"+-"<<dEest<<endl;

      Eests.push_back(Eest); 
      dEests.push_back(dEest); 
    }

    // Calculate weighted average:
    Eavg = 0;
    double dEavg = 0;
    for (unsigned int i = 0; i < Eests.size(); ++i) {
      Eavg += Eests[i]/dEests[i]/dEests[i];
      dEavg += 1.0/dEests[i]/dEests[i];
    }
    Eavg = Eavg/dEavg;
    dEavg = sqrt(dEavg)/dEavg;
    mdebug<<"CSR-Rec (4+): Avg. estimated energy: "<<Eavg<<"+-"<<dEavg<<endl;
    
    if (Eavg+MaxEDiffSigma*dEavg < Etot) {
      mdebug<<"CSR-Rec (3): Estimated energy (+ Delta) lower than measured deposits!"<<endl;
      CQF = c_CSRFailed;
      return;
    }

    // Verify that each element of the sequence is Compton-compatible:
    bool Failed = false;
    double dEtot = 0;
    Etot = Eavg;
    double Eg = 0;
    for (unsigned int i = 0; i < i_max; ++i) { // We do not check the last hit because it might, be completely absorbed!
      Eg = Etot - RESEs[i]->GetEnergy();
      if (MComptonEvent::IsKinematicsOK(Etot-Eg, Eg) == false) {
        mdebug<<"CSR-Rec (4+): IA "<<i<<" is not Compton compatible!"<<endl;
        Failed = true;
        break;
      }
      Etot -= RESEs[i]->GetEnergy();
      dEtot += RESEs[i]->GetEnergyResolution()*RESEs[i]->GetEnergyResolution();
    }
    Etot -= RESEs[RESEs.size()-1]->GetEnergy();

    // Calculate Chi**2 test statistics
    double Chi = 0;
    if (Failed == false) {
      for (unsigned int i = 0; i < Eests.size(); ++i) {
        if (m_TestStatisticsType == c_TestStatisticsChiSquareWithoutErrors) {
          Chi += pow((Eests[i] - Eavg), 2);          
        } else {
          Chi += pow((Eests[i] - Eavg)/dEests[i], 2);
        }
      }

      CQF = Chi;
      mdebug<<"CSR-Rec (4+): CQF: "<<CQF<<" prob: "<<TMath::Prob(Chi, Eests.size())<<endl;
      if (m_TestStatisticsType == c_TestStatisticsChiSquareProbability) {
        CQF = 1-TMath::Prob(Chi, Eests.size());
      }

      // Do not upgrade events close to the total energy:
      if (fabs(Etot) < MaxEDiffSigma*sqrt(dEtot)) {
        Eavg = 0;
        for (unsigned int i = 0; i < RESEs.size(); ++i) {
          Eavg += RESEs[i]->GetEnergy();
        }
        mdebug<<"CSR-Rec (4+): Im using original energy: "<<Eavg<<endl;
      } else {
        mdebug<<"CSR-Rec (4+): Not using original energy: "<<fabs(Etot)<<" > "<<MaxEDiffSigma*sqrt(dEtot)<<endl;
      }
    } else {
      CQF = c_CSRFailed;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


double MERCSREnergyRecovery::GetEscapedEnergy(vector<MRESE*>& RESEs)
{
  // The class is not searching for escaped energy!

  double CQF = 0;
  double Eavg = 0;

  CalculateQF(RESEs, CQF, Eavg);
 
  double Eescaped = Eavg;
  for (unsigned int i = 0; i < RESEs.size(); ++i) {
    Eescaped -= RESEs[i]->GetEnergy();
  }

  return Eescaped;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSREnergyRecovery::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# CSR - energy recovery options:"<<endl;
  out<<"# "<<endl;
  out<<"# Test statistics:                 Energy recovery"<<endl;
  out<<"# QualityFactorMin:               "<<m_QualityFactorMin<<endl;
  out<<"# QualityFactorMax:               "<<m_QualityFactorMax<<endl;
  out<<"# MaxNInteractions:               "<<m_MaxNInteractions<<endl;
  out<<"# GuaranteeStartD1:               "<<m_GuaranteeStartD1<<endl;
  out<<"# "<<endl;
  
  return out.str().c_str();
}


// MERCSREnergyRecovery.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
