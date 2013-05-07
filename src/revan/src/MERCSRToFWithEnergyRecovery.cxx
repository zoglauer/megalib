/*
 * MERCSRToFWithEnergyRecovery.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Elena Novikova, Eric Wulf, Clarisse Hamadache, Vincent Tatischeff
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
// MERCSRToFWithEnergyRecovery
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRToFWithEnergyRecovery.h"

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


const int MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareWithoutErrors = 1;
const int MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareWithErrors = 2;
const int MERCSRToFWithEnergyRecovery::c_TestStatisticsChiSquareProbability = 3;

const int MERCSRToFWithEnergyRecovery::c_TestStatisticsFirst = 1;
const int MERCSRToFWithEnergyRecovery::c_TestStatisticsLast = 3;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERCSRToFWithEnergyRecovery)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCSRToFWithEnergyRecovery::MERCSRToFWithEnergyRecovery() : MERCSR()
{
  // Construct an instance of MERCSRToFWithEnergyRecovery

  //m_TestStatisticsType = c_TestStatisticsChiSquareWithErrors; // commented out by VT
  m_TestStatisticsType = c_TestStatisticsChiSquareProbability;
  m_MaxSearchSequence = numeric_limits<int>::max();
  //m_MaxSearchSequence = 5; // commented out by VT
}


////////////////////////////////////////////////////////////////////////////////


MERCSRToFWithEnergyRecovery::~MERCSRToFWithEnergyRecovery()
{
  // Delete this instance of MERCSRToFWithEnergyRecovery
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRToFWithEnergyRecovery::SetParameters(MGeometryRevan* Geometry, 
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

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRToFWithEnergyRecovery::ModifyEventList()
{
/*
  // Split all tracks, which are below a certain quality level
  // and add a new event for them:

  // cout<<"void MERCSRToFWithEnergyRecovery::ModifyEventList()"<<endl;


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


double MERCSRToFWithEnergyRecovery::ComputeQualityFactor(vector<MRESE*>& RESEs)
{
  
  //mout<<"Entering incomplete CSR with ToF and energy recovery"<<endl;
  
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


void MERCSRToFWithEnergyRecovery::CalculateQF(vector<MRESE*>& RESEs, double& CQF, double& Eavg)
{
 
  // 2010/07/16 modification (Clarisse)  
  // merge EnergyRecovery and ToF routines

  CQF = c_CSRFailed;

  //Calculate Quality factor Energy Recovery CQF_ER 
  //-------------------------------------------------------------
  //-------------------------------------------------------------
  
  // Here the test statistics always contains an energy and a 
  // Klein-Nishina criteria...
  // Initial version does not take care of tracks...
  // ToDo: Add polarization effects...

  // 
  double CQF_ER = c_CSRFailed;
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
      CQF_ER = 1-MComptonEvent::GetKleinNishinaNormalized(Etot, Phi);
      Eavg = Etot; // Assume complete absorption.
    } else {
      CQF_ER = c_CSRFailed;
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
      CQF_ER = c_CSRFailed;
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
      CQF_ER = c_CSRFailed;
      return;
    }

    // Test if we have complete absorption:
    if (fabs(EDiff) < MaxEDiffSigma*dEest) {
      mdebug<<"CSR-Rec (3): Completely absorbed 3-Compton !"<<endl;
      // Test if the completely absorbed event is kinematically correct:
      if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), RESEs[1]->GetEnergy()+RESEs[2]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[1]->GetEnergy(), RESEs[2]->GetEnergy()) == false) {
        mdebug<<"CSR-Rec (3): Kinematics failed!"<<endl;
        CQF_ER = c_CSRFailed;
      } else {
        mdebug<<"CSR-Rec (3): Kinematics ok!"<<endl;

        // The Chi-Square test statistics results in a probability....
        // Ignoring: CQF_ER = (1-TMath::Prob(EDiff*EDiff/dEest/dEest, 1));

        // The CQF_ER consists of three times the normalized cross-section
        double Ei = Eest;
        double Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[0]->GetEnergy(), Ei - RESEs[0]->GetEnergy());
        double KN = MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
      
        Ei = Eest - RESEs[0]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[1]->GetEnergy(), Ei - RESEs[1]->GetEnergy());
        KN *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        // It probably would be good to have a third probability here...

        // Renormalized CQF_ER - in the moment there is not good solution 
        // for a combined incomplete and completely absorbed quality factor...
        CQF_ER = 1 - KN; // Ignores original CQF_ER;
        
        // Store the total energy as average estimated one:
        Eavg = Etot;
      }
    } else {
      // We do have incomplete absorption...
      
      // Check Compton kinematics:
      // Modified by VT (26/08/2010): only the first two interactions are used for the CQF
/*      if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), Eest - RESEs[0]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[1]->GetEnergy(), Eest - RESEs[0]->GetEnergy() - RESEs[1]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[2]->GetEnergy(), EDiff) == false) { */
      if (MComptonEvent::IsKinematicsOK(RESEs[0]->GetEnergy(), Eest - RESEs[0]->GetEnergy()) == false ||
          MComptonEvent::IsKinematicsOK(RESEs[1]->GetEnergy(), Eest - RESEs[0]->GetEnergy() - RESEs[1]->GetEnergy()) == false) { 
        mdebug<<"CSR-Rec (3): Kinematics failed!"<<endl;
        CQF_ER = c_CSRFailed;
      } 
      // All checks passed, lets calculate 
      else {
        mdebug<<"CSR-Rec (3): Kinematics ok!"<<endl;
        CQF_ER = 1; // "passed" energy test (per definition) with Chi-square = 0
	
        // The CQF_ER consists of three times the normalized cross-section
	// VT: only two times the normalized cross sections
        double Ei = Eest;
        double Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[0]->GetEnergy(), Ei - RESEs[0]->GetEnergy());
        CQF_ER *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        Ei = Eest - RESEs[0]->GetEnergy();
        Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[1]->GetEnergy(), Ei - RESEs[1]->GetEnergy());
        CQF_ER *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
        
        //Ei = Eest - RESEs[1]->GetEnergy();
        //Phi = MComptonEvent::ComputePhiViaEeEg(RESEs[2]->GetEnergy(), Ei - RESEs[2]->GetEnergy());
        //CQF_ER *= MComptonEvent::GetKleinNishinaNormalized(Ei, Phi);
 
        // Renormalized CQF_ER:
        CQF_ER = 1 - CQF_ER;

        // Return the estimated energy:
        Eavg = Eest;
      }
      
      //CQF_ER = c_CSRFailed;
      //Eavg = 0.;	
    }
    mdebug<<"CSR-Rec (3): Final quality factor: "<<CQF_ER<<endl;
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
      CQF_ER = c_CSRFailed;
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

      CQF_ER = Chi;
      mdebug<<"CSR-Rec (4+): CQF_ER: "<<CQF_ER<<" prob: "<<TMath::Prob(Chi, Eests.size())<<endl;
      if (m_TestStatisticsType == c_TestStatisticsChiSquareProbability) {
        CQF_ER = 1-TMath::Prob(Chi, Eests.size());
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
      CQF_ER = c_CSRFailed;
    }
  }

  //mout << "Quality Factor for energy recovery CQF_ER = " << CQF_ER << endl;

  // Calculate a Compton quality factor for Compton kinematics & ToF 
  // via a pseudo chi-square method

  vector<MRESE*>::iterator Iter;

  mdebug<<"**** Sequenz: ";
  for (Iter = RESEs.begin(); Iter != RESEs.end(); ++Iter) {
    mdebug<<(*Iter)->GetID()<<"  ";
  }
  mdebug<<endl;

  // Calculate Quality factor:
  int DegreesOfFreedom = 0;
  double ChiSquare = 0; // gRandom->Rndm();

  // Time dependent part:
  for (unsigned int i = 1; i < RESEs.size(); ++i) {

    // This should be used only for interactions in separate modules ...
    if (m_Geometry->AreInSameDetectorVolume(RESEs[i-1], RESEs[i]) == false) {
      //if (RESEs.size() == 2) {
      //  mout<<"Different module: "<<RESEs[i-1]->GetPosition()<<":"<<RESEs[i]->GetPosition()<<endl;
      //}

      // Measured time difference between steps i and i-1 in currently tested sequence:  
      double Dist3D = (RESEs[i]->GetPosition() - RESEs[i-1]->GetPosition()).Mag();
      if (Dist3D == 0) {
        merr<<"CSR ToF: Two interaction occupy the same position!"<<endl;
        continue;
      }
      double TDiffMeas = RESEs[i]->GetTime() - RESEs[i-1]->GetTime();
      double TDiffCalc = Dist3D/c_SpeedOfLight;
      
      // Errors resulting from position uncertainties are included, but should be negligible
      // for position errors of ~1mm and time errors of ~100 ps.
      double TDiffErr1 = RESEs[i-1]->GetTimeResolution();
      if (TDiffErr1 == 0) {
        merr<<"CSR ToF: Interactions are required to have a time resolution!"<<endl;
        continue;
      }

      double TDiffErr2 = RESEs[i]->GetTimeResolution();
      if (TDiffErr2 == 0) {
        merr<<"CSR ToF: Interactions are required to have a time resolution!"<<endl;
        continue;
      }

      double DistX = RESEs[i]->GetPositionX() - RESEs[i-1]->GetPositionX();
      double DistY = RESEs[i]->GetPositionY() - RESEs[i-1]->GetPositionY();
      double DistZ = RESEs[i]->GetPositionZ() - RESEs[i-1]->GetPositionZ();
      double SigSqrX = (RESEs[i]->GetPositionResolutionX()*RESEs[i]->GetPositionResolutionX())
        + (RESEs[i-1]->GetPositionResolutionX()*RESEs[i-1]->GetPositionResolutionX());
      double SigSqrY = (RESEs[i]->GetPositionResolutionY()*RESEs[i]->GetPositionResolutionY())
        + (RESEs[i-1]->GetPositionResolutionY()*RESEs[i-1]->GetPositionResolutionY());
      double SigSqrZ = (RESEs[i]->GetPositionResolutionZ()*RESEs[i]->GetPositionResolutionZ())
        + (RESEs[i-1]->GetPositionResolutionZ()*RESEs[i-1]->GetPositionResolutionZ());
      double TPosErrSqr = (DistX*DistX*SigSqrX + DistY*DistY*SigSqrY + DistZ*DistZ*SigSqrZ)
        / (Dist3D*Dist3D*c_SpeedOfLight*c_SpeedOfLight);

      //ugo debug:
      // if (RESEs.size()==2 && i==1) {
      //     mout<<"==i-1,i Z/dTm/dTc/err: "
      //         <<RESEs[i-1]->GetPositionZ()<<" "<<RESEs[i]->GetPositionZ()
      //         <<" "<<TDiffMeas<<" "<<TDiffCalc<<" "
      //         <<sqrt(TDiffErr1*TDiffErr1 +TDiffErr2*TDiffErr2 + TPosErrSqr)<<endl;
      //       //<<" TDiffErr1: "<<TDiffErr1<<" TDiffErr2: "<<TDiffErr2
      //       //<<" TPosErr: "<<sqrt(TPosErrSqr)<<endl;
      // }
    

      mdebug<<"Calculated Time Diff: "<<TDiffCalc<<"Measured Time Diff: "<<TDiffMeas<<endl;
      
      ChiSquare += (TDiffMeas - TDiffCalc)*(TDiffMeas - TDiffCalc)/ 
        (TDiffErr1*TDiffErr1 + TDiffErr2*TDiffErr2 + TPosErrSqr);

      DegreesOfFreedom++;
      //mout<<"N!=2 : ChiSquare: "<<ChiSquare<<" DF: "<<DegreesOfFreedom<<endl;
      //if (RESEs.size() == 3) { 
      //  mout<<"   TDiffMeas: "<<TDiffMeas<<"   TDiffCalc: "<<TDiffCalc
      //      <<"   TDiffErr1: "<<TDiffErr1<<"   TDiffErr2: "<<TDiffErr2<<"   TPosErrSqr: "<<TPosErrSqr<<endl;
      //}
    } else {
      if (RESEs.size() == 2) {
        //mout<<"Same module: "<<RESEs[i-1]->GetPosition()<<":"<<RESEs[i]->GetPosition()<<endl;

      }
    }
  }

  //mdebug<<"ChiSquare: "<<ChiSquare<<" DF: "<<DegreesOfFreedom<<endl;

  double CQF_ToF;
  if (RESEs.size() == 2 && DegreesOfFreedom == 0) {
    CQF_ToF = 1;
  } else {
    CQF_ToF = 1 - TMath::Prob(ChiSquare, DegreesOfFreedom);
    //CQF_ToF = ChiSquare/DegreesOfFreedom;
    //mout << "Quality Factor for Time of Flight CQF_ToF = " << CQF_ToF << endl;
  }

  // Calculated final CQF= product of two pseudo-probabilities 
  if (CQF_ER == c_CSRFailed) { 
    CQF = c_CSRFailed;
  } else {
    CQF = 1 - (1-CQF_ER)*(1-CQF_ToF);
    //CQF = CQF_ER;
  }

  //mout << "Final CQF = 1 - (1-CQF_ER)*(1-CQF_ToF) = " << CQF <<endl;
  //if (RESEs.size() == 3) { sleep(10);}
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRToFWithEnergyRecovery::GetEscapedEnergy(vector<MRESE*>& RESEs)
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


MString MERCSRToFWithEnergyRecovery::ToString(bool CoreOnly) const
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


// MERCSRToFWithEnergyRecovery.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
