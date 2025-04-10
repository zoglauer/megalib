/*
 * MERCSRBayesian.cxx
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
// MERCSRBayesian
//
// Compton sequence reconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERCSRBayesian.h"

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
#include "MRESE.h"
#include "MRECluster.h"
#include "MRETrack.h"
#include "MComptonEvent.h"
#include "MGeometryRevan.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERCSRBayesian)
#endif


////////////////////////////////////////////////////////////////////////////////


MERCSRBayesian::MERCSRBayesian() : MERCSR()
{
  // Construct an instance of MERCSRBayesian
}


////////////////////////////////////////////////////////////////////////////////


MERCSRBayesian::~MERCSRBayesian()
{
  // Delete this instance of MERCSRBayesian
}


////////////////////////////////////////////////////////////////////////////////


bool MERCSRBayesian::SetParameters(MString FileName, 
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

  m_UseAbsorptionsUpTo = 5;

  // If you change it here, also change it in MResponseMultiComptonBayes
  m_MaxCosineLimit = 2;

  m_FileName = FileName;

  if (m_CreateOnlyPermutations == false) {
    MString Suffix = ".mc.goodbad.rsp";
    
    if (MFile::FileExists(FileName) == false ||
        FileName.EndsWith(Suffix) == false) {
      mgui<<"Files for Bayesian Compton Tracking not found: \""<<FileName<<"\""<<error;
      return false;
    }

    MString Prefix = FileName.Remove(FileName.Length()-Suffix.Length());
    
    if (MFile::FileExists(Prefix + ".mc.goodbad.rsp") == true) {
      m_GoodBad.Read(Prefix + ".mc.goodbad.rsp");
      
      m_GoodDualDeposit.Read(Prefix + ".mc.dual.good.rsp");
      m_SumGoodDualDeposit = m_GoodDualDeposit.GetSum();
      m_BadDualDeposit.Read(Prefix + ".mc.dual.bad.rsp");
      m_SumBadDualDeposit = m_BadDualDeposit.GetSum();
      
      m_GoodStartDeposit.Read(Prefix + ".mc.start.good.rsp");
      m_SumGoodStartDeposit = m_GoodStartDeposit.GetSumMatrixO1(3);
      m_BadStartDeposit.Read(Prefix + ".mc.start.bad.rsp");
      m_SumBadStartDeposit = m_BadStartDeposit.GetSumMatrixO1(3);
      
      m_GoodTrack.Read(Prefix + ".mc.track.good.rsp");
      m_SumGoodTrack = m_GoodTrack.GetSumMatrixO1(5);
      m_BadTrack.Read(Prefix + ".mc.track.bad.rsp");
      m_SumBadTrack = m_BadTrack.GetSumMatrixO1(5);
      
      m_GoodComptonDistance.Read(Prefix + ".mc.comptondistance.good.rsp");
      m_SumGoodComptonDistance = m_GoodComptonDistance.GetSumMatrixO1(3);
      m_BadComptonDistance.Read(Prefix + ".mc.comptondistance.bad.rsp");
      m_SumBadComptonDistance = m_BadComptonDistance.GetSumMatrixO1(3);
      
      m_GoodPhotoDistance.Read(Prefix + ".mc.photodistance.good.rsp");
      m_SumGoodPhotoDistance = m_GoodPhotoDistance.GetSumMatrixO1(3);
      m_BadPhotoDistance.Read(Prefix + ".mc.photodistance.bad.rsp");
      m_SumBadPhotoDistance = m_BadPhotoDistance.GetSumMatrixO1(3);
      
      m_GoodCompton.Read(Prefix + ".mc.compton.good.rsp");
      m_SumGoodCompton = m_GoodCompton.GetSumMatrixO1(5);
      m_BadCompton.Read(Prefix + ".mc.compton.bad.rsp");
      m_SumBadCompton = m_BadCompton.GetSumMatrixO1(5);
    } else {
      mgui<<"Bayesian response files not found!"<<error;
      return false;
    }
  }

  if (Geometry->AreCrossSectionsPresent() == false) {
    mgui<<"Analyzing events via Klein-Nishina and photo effect for the Bayesian"<<endl;
    mgui<<"event reconstruction requires absorption probabilities in Geomega."<<endl;
    mgui<<"However, Geomega indicates that those have not been proberly loaded."<<endl;
    mgui<<"Aborting analysis"<<error;
    return false;
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::ComputeQualityFactor(vector<MRESE*>& Interactions)
{
  massert(Interactions.size() >= 2);

  bool ShowDebug = false;

  // How to handle no entries:

  double BP = 0; // Bayesian probability

  double Etot = 0;
  for (unsigned int i = 0; i < Interactions.size(); ++i) {
    Etot += Interactions[i]->GetEnergy();
  }

  // If the las tinteraction is a track - then this combination is more than bad...
  if (Interactions.back()->GetType() == MRESE::c_Track) {
    if (ShowDebug == true) {
      cout<<"Sequence ends with a track -> rejected!"<<endl;
    }
    return c_CSRFailed;
  }

  unsigned int Size = Interactions.size();
  float SizeF = float(Size);

  float Material;
  float Ratio = m_GoodBad.Get(1.5, SizeF)/m_GoodBad.Get(0.5, SizeF);

  float EntriesGood = 0;
  float EntriesBad = 0;
  float SumGood = 0;
  float SumBad = 0;
  const float MinEntries = 0;

  vector<MRESE*>::iterator Iter;
  if (ShowDebug == true) {
    cout<<endl;
    cout<<"**** Sequenz: ";
    for (Iter = Interactions.begin(); Iter != Interactions.end(); ++Iter) {
      cout<<(*Iter)->GetID()<<"  ";
    }
    cout<<endl;
    
    cout<<"Ratio:        "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<0.0<<"  "
        <<setw(8)<<Ratio<<endl;
  }
  Iter = Interactions.begin();

  if (Size == 2) {
    Material = float(GetMaterial(*Iter));
    double CosPhiE = CalculateCosPhiE((*Iter), Etot);
    if (CosPhiE <= -m_MaxCosineLimit) CosPhiE = -0.99*m_MaxCosineLimit;
    if (CosPhiE >= +m_MaxCosineLimit) CosPhiE = +0.99*m_MaxCosineLimit;
    double Distance = CalculateTotalDistance((*Iter)->GetPosition(), (*(Iter+1))->GetPosition(), (*(Iter+1))->GetEnergy());

    EntriesGood = m_GoodDualDeposit.Get(Etot, CosPhiE, Distance, Material);
    EntriesBad = m_BadDualDeposit.Get(Etot, CosPhiE, Distance, Material);
    SumGood = m_SumGoodDualDeposit;
    SumBad = m_SumBadDualDeposit;
    VerifyEntries(EntriesGood, EntriesBad);
    
    if (ShowDebug == true) {
      cout<<"Dual:        "
          <<setw(8)<<Etot<<"  "
          <<setw(8)<<CosPhiE<<"  "
          <<setw(8)<<Distance<<"  "
          <<setw(8)<<Material<<"  "
          <<setw(8)<<0.0<<"  "
          <<setw(8)<<0.0<<"  ";
    }
    if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
      Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
      if (ShowDebug == true) {
        cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
        cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
      }
      //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
      //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
    } else {
      if (ShowDebug == true) {
        cout<<"no entries"<<endl;
      }
    }

  } else {
    // Check start:
    Material = float(GetMaterial(*Iter));
    double CosPhiE = CalculateCosPhiE((*Iter), Etot);
    if (CosPhiE <= -m_MaxCosineLimit) CosPhiE = -0.99*m_MaxCosineLimit;
    if (CosPhiE >= +m_MaxCosineLimit) CosPhiE = +0.99*m_MaxCosineLimit;

    EntriesGood = m_GoodStartDeposit.Get(Etot, CosPhiE, SizeF, Material);
    EntriesBad = m_BadStartDeposit.Get(Etot, CosPhiE, SizeF, Material);
    SumGood = m_SumGoodStartDeposit.Get(SizeF);
    SumBad = m_SumBadStartDeposit.Get(SizeF);
    VerifyEntries(EntriesGood, EntriesBad);
    
    if (ShowDebug == true) {
      cout<<"Start:        "
          <<setw(8)<<Etot<<"  "
          <<setw(8)<<CosPhiE<<"  "
          <<setw(8)<<Size<<"  "
          <<setw(8)<<Material<<"  "
          <<setw(8)<<0.0<<"  "
          <<setw(8)<<0.0<<"  ";
    }
    if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
      Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
      if (ShowDebug == true) {
        cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
        cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
      }
      //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
      //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
    } else {
      if (ShowDebug == true) {
        cout<<"no entries"<<endl;
      }
    }

    // Check initial track:
    if ((*Iter)->GetType() ==  MRESE::c_Track) {
      double dAlpha = CalculateDCosAlpha((MRETrack*) (*Iter), *(Iter+1), Etot);
      if (dAlpha <= -m_MaxCosineLimit) dAlpha = -0.99*m_MaxCosineLimit;
      if (dAlpha >= +m_MaxCosineLimit) dAlpha = +0.99*m_MaxCosineLimit;
      double Alpha = CalculateCosAlphaG((MRETrack*) (*Iter), *(Iter+1), Etot);
      if (Alpha <= -m_MaxCosineLimit) Alpha = -0.99*m_MaxCosineLimit;
      if (Alpha >= +m_MaxCosineLimit) Alpha = +0.99*m_MaxCosineLimit;

      double ElectronEnergy = (*Iter)->GetEnergy();
      EntriesGood = m_GoodTrack.Get(dAlpha, Alpha, 1, ElectronEnergy, SizeF, Material);
      EntriesBad = m_BadTrack.Get(dAlpha, Alpha, 1, ElectronEnergy, SizeF, Material);
      SumGood = m_SumGoodTrack.Get(SizeF);
      SumBad = m_SumBadTrack.Get(SizeF);
      VerifyEntries(EntriesGood, EntriesBad);

      if (ShowDebug == true) {
        cout<<"Start:        "
            <<setw(8)<<dAlpha<<"  "
            <<setw(8)<<Alpha<<"  "
            <<setw(8)<<1<<"  "
            <<setw(8)<<ElectronEnergy<<"  "
            <<setw(8)<<Size<<"  "
            <<setw(8)<<Material<<"  ";
      }
      if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
        Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
        if (ShowDebug == true) {
          cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
          cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
        }
        //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
        //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
      } else {
        if (ShowDebug == true) {
          cout<<"no entries: S:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
        }
      }
    }
  

    Iter++;
    while ((Iter+1) != Interactions.end()) {
      Etot -= (*(Iter-1))->GetEnergy();

    
      // Compton Distance:
      if (Size <= m_UseAbsorptionsUpTo) {
        double ComptonDistance = CalculateReach((*(Iter-1))->GetPosition(), (*Iter)->GetPosition(), Etot);
        Material = float(GetMaterial(*Iter));
        EntriesGood = m_GoodComptonDistance.Get(ComptonDistance, Etot, SizeF, Material);
        EntriesBad = m_BadComptonDistance.Get(ComptonDistance, Etot, SizeF, Material);
        SumGood = m_SumGoodComptonDistance.Get(SizeF);
        SumBad = m_SumBadComptonDistance.Get(SizeF);
        VerifyEntries(EntriesGood, EntriesBad);
        
        if (ShowDebug == true) {
          cout<<"Compton Dist: "
              <<setw(8)<<ComptonDistance<<"  "
              <<setw(8)<<Etot<<"  "
              <<setw(8)<<Size<<"  "
              <<setw(8)<<Material<<"  "
              <<setw(8)<<0.0<<"  "
              <<setw(8)<<0.0<<"  ";
        }
        if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
          Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
          //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
          if (ShowDebug == true) {
            cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
            cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
            //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
          }
        } else {
          if (ShowDebug == true) {
            cout<<"no entries: S:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
          }
        }
      }

      // Compton:
      double dPhi = CalculateDCosPhi((*(Iter-1)), (*Iter), (*(Iter+1)), Etot);
      if (dPhi <= -m_MaxCosineLimit) dPhi = -0.99*m_MaxCosineLimit;
      if (dPhi >= +m_MaxCosineLimit) dPhi = +0.99*m_MaxCosineLimit;
      double PhiE = CalculateCosPhiE((*Iter), Etot);
      if (PhiE <= -m_MaxCosineLimit) PhiE = -0.99*m_MaxCosineLimit;
      if (PhiE >= +m_MaxCosineLimit) PhiE = +0.99*m_MaxCosineLimit;
      double Lever = CalculateMinLeverArm((*(Iter-1))->GetPosition(), 
                                          (*Iter)->GetPosition(),
                                          (*(Iter+1))->GetPosition());
      Material = float(GetMaterial(*Iter));
      EntriesGood = m_GoodCompton.Get(dPhi, PhiE, Lever, Etot, SizeF, Material);
      EntriesBad = m_BadCompton.Get(dPhi, PhiE, Lever, Etot, SizeF, Material);
      SumGood = m_SumGoodCompton.Get(SizeF);
      SumBad = m_SumBadCompton.Get(SizeF);
      VerifyEntries(EntriesGood, EntriesBad);
      
      if (ShowDebug == true) {
        cout<<"Compton:      "
            <<setw(8)<<dPhi<<"  "
            <<setw(8)<<PhiE<<"  "
            <<setw(8)<<Lever<<"  "
            <<setw(8)<<Etot<<"  "
            <<setw(8)<<Size<<"  "
            <<setw(8)<<Material<<"  ";
      }
      if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
        Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
        //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
        if (ShowDebug == true) {
          cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood<<"  ";
          cout<<"G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
          //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
        }
      } else {
        if (ShowDebug == true) {
          cout<<"no entries: S:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
        }
      }
      
      // Check central track:
      if ((*Iter)->GetType() ==  MRESE::c_Track) {
        double dAlpha = CalculateDCosAlpha((MRETrack*) (*Iter), *(Iter+1), Etot);
        if (dAlpha <= -m_MaxCosineLimit) dAlpha = -0.99*m_MaxCosineLimit;
        if (dAlpha >= +m_MaxCosineLimit) dAlpha = +0.99*m_MaxCosineLimit;
        double Alpha = CalculateCosAlphaG((MRETrack*) (*Iter), *(Iter+1), Etot);
        if (Alpha <= -m_MaxCosineLimit) Alpha = -0.99*m_MaxCosineLimit;
        if (Alpha >= +m_MaxCosineLimit) Alpha = +0.99*m_MaxCosineLimit;
        double ElectronEnergy = (*Iter)->GetEnergy();
        EntriesGood = m_GoodTrack.Get(dAlpha, Alpha, 1, ElectronEnergy, SizeF, Material);
        EntriesBad = m_BadTrack.Get(dAlpha, Alpha, 1, ElectronEnergy, SizeF, Material);
        SumGood = m_SumGoodTrack.Get(SizeF);
        SumBad = m_SumBadTrack.Get(SizeF);
        VerifyEntries(EntriesGood, EntriesBad);
        
        if (ShowDebug == true) {
          cout<<"Start:        "
              <<setw(8)<<dAlpha<<"  "
              <<setw(8)<<Alpha<<"  "
              <<setw(8)<<1<<"  "
              <<setw(8)<<ElectronEnergy<<"  "
              <<setw(8)<<Size<<"  "
              <<setw(8)<<Material<<"  ";
        }
        if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
          Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
          if (ShowDebug == true) {
            cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
            cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
          }
          //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
          //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
        } else {
          if (ShowDebug == true) {
            cout<<"no entries: S:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
          }
        }
      }

      Iter++;
    }
    Etot -= (*(Iter-1))->GetEnergy();

    // Photo distance
    if (Size <= m_UseAbsorptionsUpTo) {
      double Distance = 
        CalculatePhotoDistance((*(Iter-1))->GetPosition(), 
                               (*Iter)->GetPosition(), Etot);
      Material = float(GetMaterial(*Iter));
      EntriesGood = m_GoodPhotoDistance.Get(Distance, Etot, SizeF, Material);
      EntriesBad = m_BadPhotoDistance.Get(Distance, Etot, SizeF, Material);
      SumGood = m_SumGoodPhotoDistance.Get(SizeF);
      SumBad = m_SumBadPhotoDistance.Get(SizeF);
      VerifyEntries(EntriesGood, EntriesBad);

      if (ShowDebug == true) {
        cout<<"Photo Dist:   "
            <<setw(8)<<Distance<<"  "
            <<setw(8)<<Etot<<"  "
            <<setw(8)<<Size<<"  "
            <<setw(8)<<Material<<"  "
            <<setw(8)<<0.0<<"  "
            <<setw(8)<<0.0<<"  ";
      }
      if (EntriesGood > MinEntries && EntriesBad > MinEntries) {
        Ratio *= EntriesGood/EntriesBad * SumBad/SumGood;
        //Ratio *= EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size);
        if (ShowDebug == true) {
          cout<<setw(8)<<EntriesGood/EntriesBad * SumBad/SumGood;
          cout<<"  G:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
          //cout<<setw(8)<<EntriesGood/EntriesBad * m_GoodBad.Get(0.5, Size)/m_GoodBad.Get(1.5, Size)<<endl;
        }
      } else {
        if (ShowDebug == true) {
          cout<<"no entries: S:"<<EntriesGood<<"/"<<SumGood<<"  B:"<<EntriesBad<<"/"<<SumBad<<endl;
        }
      }
    }
  }

 
  // Determine the final probability:
  BP = Ratio/(1+Ratio);
  if (ShowDebug == true) {
    cout<<"Probability: "<<BP<<" 1-P: "<<1-BP<<endl;
  }

  return 1-BP;
}


////////////////////////////////////////////////////////////////////////////////


void MERCSRBayesian::VerifyEntries(float& NEntriesGood, float& NEntriesBad)
{
  if (NEntriesGood == 0 && NEntriesBad > 1) {
    NEntriesGood = 1;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateDCosPhi(MRESE* Start, MRESE* Central, MRESE* Stop, 
                                     double Energy)
{
  // Return the difference of the cosines of the Compton scatter angles 
  // calculated via kinematics and via geometry

  if (Energy-Central->GetEnergy() == 0 || Energy == 0) return 2;

  double PhiG = cos((Central->GetPosition() - Start->GetPosition()).
                    Angle(Stop->GetPosition() - Central->GetPosition()));
 
  const double c_E0 = 511.004;
  double PhiE = 1 - c_E0/(Energy-Central->GetEnergy()) + c_E0/Energy;

  return PhiE-PhiG;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateDPhiInDegree(MRESE* Start, MRESE* Central, 
                                     MRESE* Stop, double Energy)
{
  if (Energy - Central->GetEnergy() == 0 || Energy == 0) return 180.5;

  double PhiG = (Central->GetPosition() - Start->GetPosition()).
    Angle(Stop->GetPosition() - Central->GetPosition())*c_Deg;
 
  double PhiE = 1 - c_E0/(Energy-Central->GetEnergy()) + c_E0/Energy;
  if (PhiE < -1 || PhiE > 1) {
    if (PhiG > 180) return 180.5;
    else return -0.5;
  }
  PhiE = acos(PhiE)*c_Deg;

  double Value = fabs(PhiG-PhiE);
  if (Value > 180) Value = 180.5;

  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateCosPhiE(MRESE* Central, double Etot)
{
  if (Etot-Central->GetEnergy() == 0 || Etot == 0) return 2;
  return 1 - c_E0/(Etot-Central->GetEnergy()) + c_E0/Etot;
}



////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculatePhiEInDegree(MRESE* Central, double Etot)
{
  if (Etot-Central->GetEnergy() == 0 || Etot == 0) return 180.5;
  double Value = 1 - c_E0/(Etot-Central->GetEnergy()) + c_E0/Etot;

  if (Value > 1) Value = 180.5;
  else if (Value < -1) Value = -0.5;
  else Value = acos(Value)*c_Deg;

  return Value;
}

////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculatePhiGInDegree(MRESE* Start, MRESE* Central, 
                                              MRESE* Stop)
{
  double PhiG = (Central->GetPosition() - Start->GetPosition()).
    Angle(Stop->GetPosition() - Central->GetPosition())*c_Deg;

  return PhiG;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateCosPhiG(MRESE* Start, MRESE* Central, 
                                        MRESE* Stop)
{
  double CosPhiG = (Central->GetPosition() - Start->GetPosition()).
  Dot(Stop->GetPosition() - Central->GetPosition());
  
  CosPhiG /= (Central->GetPosition() - Start->GetPosition()).Mag();
  CosPhiG /= (Stop->GetPosition() - Central->GetPosition()).Mag();
  
  return CosPhiG;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateDCosAlpha(MRETrack* Start, MRESE* Central, double Energy)
{
  double AlphaE = (Start->GetEnergy()*((Energy - Start->GetEnergy()) - c_E0)) / 
    ((Energy - Start->GetEnergy())*sqrt(Start->GetEnergy()*(Start->GetEnergy() + 2*c_E0)));
  double AlphaG = cos((Central->GetPosition() - Start->GetPosition()).Angle(Start->GetDirection()));

  return AlphaE - AlphaG;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateDAlphaInDegree(MRETrack* Start, MRESE* Central, double Energy)
{
  double AlphaE = (Start->GetEnergy()*((Energy - Start->GetEnergy()) - c_E0)) / 
    ((Energy - Start->GetEnergy())*sqrt(Start->GetEnergy()*(Start->GetEnergy() + 2*c_E0)));

  if (AlphaE < -1 || AlphaE > 1) return 180.5; 
  AlphaE = acos(AlphaE)*c_Deg;
  double AlphaG = (Central->GetPosition() - Start->GetPosition()).Angle(Start->GetDirection())*c_Deg;

  if (fabs(AlphaE - AlphaG) > 180.0) return 180.5;

  return fabs(AlphaE - AlphaG);
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateAlphaEInDegree(MRETrack* Start, MRESE* Central, double Energy)
{
  double Value = (Start->GetEnergy()*((Energy - Start->GetEnergy()) - c_E0)) / 
    ((Energy - Start->GetEnergy())*sqrt(Start->GetEnergy()*(Start->GetEnergy() + 2*c_E0)));
  if (Value > 1.0 || Value < -1.0) return 180.5;

  return acos(Value)*c_Deg;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateCosAlphaE(MRETrack* Start, MRESE* Central, double Energy)
{
  double Value = (Start->GetEnergy()*((Energy - Start->GetEnergy()) - c_E0)) / 
    ((Energy - Start->GetEnergy())*sqrt(Start->GetEnergy()*(Start->GetEnergy() + 2*c_E0)));
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateAlphaGInDegree(MRETrack* Start, MRESE* Central, double Energy)
{
  return (Central->GetPosition() - Start->GetPosition()).Angle(Start->GetDirection())*c_Deg;
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateCosAlphaG(MRETrack* Start, MRESE* Central, double Energy)
{
  return cos((Central->GetPosition() - Start->GetPosition()).Angle(Start->GetDirection()));
}


////////////////////////////////////////////////////////////////////////////////


double MERCSRBayesian::CalculateMinLeverArm(const MVector& Start, 
                                            const MVector& Central, 
                                            const MVector& Stop)
{
  return min((Start - Central).Mag(), (Stop - Central).Mag());
}


////////////////////////////////////////////////////////////////////////////////


int MERCSRBayesian::GetMaterial(MRESE* Hit)
{
  MDVolume* V = Hit->GetVolumeSequence()->GetDeepestVolume();
  if (V == 0) {
    merr<<"MERCSRBayesian: No deepest volume. Do you use the correct geometry?"<<endl;
    merr<<"Hit postion: "<<Hit->GetPosition()<<show;
    return 0;
  }

  int ZMain = V->GetMaterial()->GetAtomicNumberMainComponent();

  if (ZMain == 14) { // Si
    return 1;
  } else if (ZMain == 32) { // Ge
    return 2;
  } else if (ZMain == 54) { // Xe
    return 3;
  } else if (ZMain == 55) { // CsI
    return 4;
  } else {
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MERCSRBayesian::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# CSR - Bayesian options:"<<endl;
  out<<"# "<<endl;
  out<<"# Filename:                        "<<m_FileName<<endl;
  out<<MERCSR::ToString(true);
  out<<"# "<<endl;
  
  return out.str().c_str();
}

// MERCSRBayesian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
