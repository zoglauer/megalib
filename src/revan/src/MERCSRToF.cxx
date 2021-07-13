/*
 * MERCSRToF.cxx
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
// MERCSRToF
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRToF.h"

// Standard libs:
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

// ROOT libs:
#include "TRandom.h"
#include "TMath.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"
#include "MAssert.h"
#include "MFile.h"
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"
#include "MDDriftChamber.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCSRToF)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCSRToF::MERCSRToF() : MERCSR()
{
  // Construct an instance of MERCSRToF
}


////////////////////////////////////////////////////////////////////////////////


MERCSRToF::~MERCSRToF()
{
  // Delete this instance of MERCSRToF
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRToF::ModifyEventList()
{
  // Correct the energy measurement with the additionally measured light 
  // and recalculate all energy resolutions: 

  // Make sure not to add/remove any RESEs after this

  int e_max = m_List->GetNRawEvents();
  for (int e = 0; e < e_max; ++e) {
    MRERawEvent* Event = m_List->GetRawEventAt(e);
    
    for (int r = 0; r < Event->GetNRESEs(); ++r) {
      // From now on, if the event structure is changed energy and resolution are
      // no longer recalculated...
      Event->GetRESEAt(r)->AllowOverwrite(false);
    }

    for (unsigned int m = 0; m < Event->GetNREAMs(); ++m) {
      if (Event->GetREAMAt(m)->GetType() == MREAM::c_DriftChamberEnergy) {
        // Create a list of all hits in this drift chamber module and update energy
        // and energy resolution:

        vector<MRESE*> List;
        for (int r = 0; r < Event->GetNRESEs(); ++r) {
          if (Event->GetREAMAt(m)->GetVolumeSequence()->HasSameDetector(Event->GetRESEAt(r)->GetVolumeSequence()) == true) {
            List.push_back(Event->GetRESEAt(r));
          }
        }

        CorrectEnergiesAndTime(dynamic_cast<MREAMDriftChamberEnergy*>(Event->GetREAMAt(m)), 
                               List);
      }
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRToF::CorrectEnergiesAndTime(MREAMDriftChamberEnergy* Ream, 
                                       vector<MRESE*>& Interactions)
{
  // Upgrade energies & energy resolutions of individual hits (non iterative):
  // Formula according to Knoll: Radiation detection and measurement
  // (Combination of independent measurements with unequal errors)
  // and some adaptions by Uwe Oberlack

  bool Error = false;

  vector<double> NewEnergy(Interactions.size());
  vector<double> NewEnergyResolution(Interactions.size());
  
  mdebug<<"Light: "<<Ream->GetEnergy()<<"+-"<<Ream->GetEnergyResolution()<<endl;

  for (unsigned int i = 0; i < Interactions.size(); ++i) {
    mdebug<<"IA "<<i<<":"<<Interactions[i]->GetEnergy()<<"+-"<<Interactions[i]->GetEnergyResolution()<<endl;
  }

  for (unsigned int i = 0; i < Interactions.size(); ++i) {
    double TotalError = Ream->GetEnergyResolution()*Ream->GetEnergyResolution();
    double TotalErrorNoti = Ream->GetEnergyResolution()*Ream->GetEnergyResolution();
    for (unsigned int j = 0; j < Interactions.size(); ++j) {
      TotalError += Interactions[j]->GetEnergyResolution()*
        Interactions[j]->GetEnergyResolution();
      if (j != i) {
        TotalErrorNoti += Interactions[j]->GetEnergyResolution()*
          Interactions[j]->GetEnergyResolution();
      }      
    }
    
    if (TotalError == 0) {
      merr<<"Total error is zero. Are your energy resolutions correct??"<<endl;
      Error = true;
      break;
    }

    double iEnergy = Ream->GetEnergy();
    double iEnergyError2 = Ream->GetEnergyResolution()*Ream->GetEnergyResolution();
    for (unsigned int j = 0; j < Interactions.size(); ++j) {
      if (j != i) {
        iEnergy -= Interactions[j]->GetEnergy();
        iEnergyError2 += Interactions[j]->GetEnergyResolution()*Interactions[j]->GetEnergyResolution();
      }
    }

    double EnergyCorr = 
      (TotalErrorNoti*Interactions[i]->GetEnergy() + 
       Interactions[i]->GetEnergyResolution()*Interactions[i]->GetEnergyResolution()*iEnergy)/
      TotalError;

    mdebug<<TotalErrorNoti<<"*"<<Interactions[i]->GetEnergy()<<"+"<<pow(Interactions[i]->GetEnergyResolution(), 2)<<"*"<<iEnergy<<"/"<<TotalError<<endl;


    double EnergyResolutionCorr =
      TotalErrorNoti*TotalErrorNoti*Interactions[i]->GetEnergyResolution()*Interactions[i]->GetEnergyResolution() +
      Interactions[i]->GetEnergyResolution()*Interactions[i]->GetEnergyResolution()*iEnergyError2*iEnergyError2;
    EnergyResolutionCorr = sqrt(EnergyResolutionCorr)/TotalError; 


    NewEnergy[i] = EnergyCorr;
    NewEnergyResolution[i] = EnergyResolutionCorr;

    mdebug<<i<<": Original: "<<Interactions[i]->GetEnergy()<<"+-"<<Interactions[i]->GetEnergyResolution()
          <<"  Corrected: "<<EnergyCorr<<"+-"<<EnergyResolutionCorr<<endl;
  }

  // Set the data:
  if (Error == false) {
    for (unsigned int i = 0; i < Interactions.size(); ++i) {
      Interactions[i]->SetEnergy(NewEnergy[i]);
      Interactions[i]->SetEnergyResolution(NewEnergyResolution[i]);

      //ugo,2005-06-22: travel time to PMT's cancels out.
      //ugo,2005-06-23: but DriftTime=GetLightTravelTime has been added in MDDriftChamber.cxx
         Interactions[i]->SetTime(Interactions[i]->GetTime() - 
                               dynamic_cast<MDDriftChamber*>(Ream->GetVolumeSequence()->GetDetector())
                               ->GetLightTravelTime(Interactions[i]->GetVolumeSequence()->GetPositionInDetector()));
      // Interactions[i]->SetTimeResolution(); ????
    }
  }

  for (unsigned int i = 0; i < Interactions.size(); ++i) {
    mdebug<<"IA "<<i<<":"<<Interactions[i]->GetEnergy()<<"+-"<<Interactions[i]->GetEnergyResolution()<<endl;
  }

}


////////////////////////////////////////////////////////////////////////////////


double MERCSRToF::ComputeQualityFactor(vector<MRESE*>& Interactions)
{
  // Calculate a Compton quality factor for Compton kinematics & ToF 
  // via a pseudo chi-square method

  vector<MRESE*>::iterator Iter;

  mdebug<<"**** Sequenz: ";
  for (Iter = Interactions.begin(); Iter != Interactions.end(); ++Iter) {
    mdebug<<(*Iter)->GetID()<<"  ";
  }
  mdebug<<endl;

  // Calculate Quality factor:
  int DegreesOfFreedom = 0;
  double ChiSquare = 0; // gRandom->Rndm();

  // Time dependent part:
  for (unsigned int i = 1; i < Interactions.size(); ++i) {

    // This should be used only for interactions in separate modules ...
    if (m_Geometry->AreInSameDetectorVolume(Interactions[i-1], Interactions[i]) == false) {
      //if (Interactions.size() == 2) {
      //  mout<<"Different module: "<<Interactions[i-1]->GetPosition()<<":"<<Interactions[i]->GetPosition()<<endl;
      //}

      // Measured time difference between steps i and i-1 in currently tested sequence:  
      double Dist3D = (Interactions[i]->GetPosition() - Interactions[i-1]->GetPosition()).Mag();
      if (Dist3D == 0) {
        merr<<"CSR ToF: Two interaction occupy the same position!"<<endl;
        continue;
      }
      double TDiffMeas = Interactions[i]->GetTime() - Interactions[i-1]->GetTime();
      double TDiffCalc = Dist3D/c_SpeedOfLight;
      
      // Errors resulting from position uncertainties are included, but should be negligible
      // for position errors of ~1mm and time errors of ~100 ps.
      double TDiffErr1 = Interactions[i-1]->GetTimeResolution();
      if (TDiffErr1 == 0) {
        merr<<"CSR ToF: Interactions are required to have a time resolution!"<<endl;
        continue;
      }

      double TDiffErr2 = Interactions[i]->GetTimeResolution();
      if (TDiffErr2 == 0) {
        merr<<"CSR ToF: Interactions are required to have a time resolution!"<<endl;
        continue;
      }

      double DistX = Interactions[i]->GetPositionX() - Interactions[i-1]->GetPositionX();
      double DistY = Interactions[i]->GetPositionY() - Interactions[i-1]->GetPositionY();
      double DistZ = Interactions[i]->GetPositionZ() - Interactions[i-1]->GetPositionZ();
      double SigSqrX = (Interactions[i]->GetPositionResolutionX()*Interactions[i]->GetPositionResolutionX())
        + (Interactions[i-1]->GetPositionResolutionX()*Interactions[i-1]->GetPositionResolutionX());
      double SigSqrY = (Interactions[i]->GetPositionResolutionY()*Interactions[i]->GetPositionResolutionY())
        + (Interactions[i-1]->GetPositionResolutionY()*Interactions[i-1]->GetPositionResolutionY());
      double SigSqrZ = (Interactions[i]->GetPositionResolutionZ()*Interactions[i]->GetPositionResolutionZ())
        + (Interactions[i-1]->GetPositionResolutionZ()*Interactions[i-1]->GetPositionResolutionZ());
      double TPosErrSqr = (DistX*DistX*SigSqrX + DistY*DistY*SigSqrY + DistZ*DistZ*SigSqrZ)
        / (Dist3D*Dist3D*c_SpeedOfLight*c_SpeedOfLight);

      //ugo debug:
      // if (Interactions.size()==2 && i==1) {
      //     mout<<"==i-1,i Z/dTm/dTc/err: "
      //         <<Interactions[i-1]->GetPositionZ()<<" "<<Interactions[i]->GetPositionZ()
      //         <<" "<<TDiffMeas<<" "<<TDiffCalc<<" "
      //         <<sqrt(TDiffErr1*TDiffErr1 +TDiffErr2*TDiffErr2 + TPosErrSqr)<<endl;
      //       //<<" TDiffErr1: "<<TDiffErr1<<" TDiffErr2: "<<TDiffErr2
      //       //<<" TPosErr: "<<sqrt(TPosErrSqr)<<endl;
      // }
    

      mdebug<<"Calculated Time Diff: "<<TDiffCalc<<"Measured Time Diff: "<<TDiffMeas<<endl;
      
      ChiSquare += (TDiffMeas - TDiffCalc)*(TDiffMeas - TDiffCalc)/ 
        (TDiffErr1*TDiffErr1 + TDiffErr2*TDiffErr2 + TPosErrSqr);

      DegreesOfFreedom++;
    } else {
      if (Interactions.size() == 2) {
        mout<<"Same module: "<<Interactions[i-1]->GetPosition()<<":"<<Interactions[i]->GetPosition()<<endl;
      }
    }
  }

  // Compton part:
  for (unsigned int i = 1; i < Interactions.size()-1; ++i) {

    double Ei = 0.0;         // energy incoming gamma
    double Eg = 0.0;         // energy scattered gamma
    double Ee = 0.0;         // energy recoil electron
    double dEg = 0.0;         // energy scattered gamma
    double dEe = 0.0;         // energy recoil electron

    double CosPhiE = 0.0;  // cos(phi) computed by energies
    double dCosPhiE2 = 0.0;  // cos(phi) computed by energies
    
    double CosPhiA = 0.0;  // cos(phi) computed by angles
    double dCosPhiA2 = 0.0;  // cos(phi) computed by angles

    // Calculate energies:
    Ee = Interactions[i]->GetEnergy(); // Das muss i heissen - definitiv!!!
    dEe = Interactions[i]->GetEnergyResolution();

    Eg = 0.0;
    dEg = 0.0;

    for (unsigned int j = i+1; j < Interactions.size(); ++j) {
      Eg += Interactions[j]->GetEnergy();
      dEg += Interactions[j]->GetEnergyResolution()*Interactions[j]->GetEnergyResolution();
    }
    Ei = Ee + Eg;

    if (Ee == 0 || Eg == 0) {
      merr<<"Interactions are not allowed to have zero energy at this level of reconstruction!"<<endl;
      continue;
    }

    CosPhiE = 1 - c_E0/Eg + c_E0/(Ee+Eg);
    dCosPhiE2 = c_E0*c_E0/(Ei*Ei*Ei*Ei)*dEe*dEe+pow(c_E0/(Eg*Eg)-c_E0/(Ee+Eg)/(Ee+Eg),2)*dEg*dEg;
    
    CosPhiA = 
      cos((Interactions[i]->GetPosition() - Interactions[i-1]->GetPosition()).
          Angle(Interactions[i+1]->GetPosition() - Interactions[i]->GetPosition()));
    dCosPhiA2 = pow(ComputePositionError(Interactions[i-1], Interactions[i], Interactions[i+1]), 2);

    if (dCosPhiE2 == 0 || dCosPhiA2 == 0) {
      merr<<"Interactions are not allowed to have perfect energy/angular resolution!"<<endl;
      continue;
    }

    ChiSquare += (CosPhiE - CosPhiA)*(CosPhiE - CosPhiA)/(dCosPhiE2 + dCosPhiA2);
    
    DegreesOfFreedom++;
  } 

  mdebug<<"ChiSquare: "<<ChiSquare<<" DF: "<<DegreesOfFreedom<<endl;

  double QF;
  if (Interactions.size() == 2 && DegreesOfFreedom == 0) {
    QF = 1;
  } else {
    QF = 1 - TMath::Prob(ChiSquare, DegreesOfFreedom);
    //QF = ChiSquare/DegreesOfFreedom;
  }

  //ugo: Debug output:
  //mout<<"Modified MERCSRToF"<<endl;  
  //if (Interactions.size()==2) {
  //    mout<<"2-site event: ChiSquare (time only): "<<ChiSquare<<" DF: "<<DegreesOfFreedom
  //        <<" QF: "<<QF<<endl;
  //}

  
  return QF;
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSRToF::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  return MERCSR::ToString();
}


// MERCSRToF.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
