/*
 * MERTrackBayesian.cxx
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
// MERTrackBayesian
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERTrackBayesian.h"

// Standard libs:
#include <list>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <functional>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"
#include "MRESEIterator.h"
#include "MRETrack.h"
#include "MREHit.h"
#include "MMuonEvent.h"
#include "MTimer.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERTrackBayesian)
#endif


////////////////////////////////////////////////////////////////////////////////


MERTrackBayesian::MERTrackBayesian() : MERTrack()
{
  // Construct an instance of MERTrackBayesian

  m_AllowOnlyMinNumberOfRESEsD1 = true;
}


////////////////////////////////////////////////////////////////////////////////


MERTrackBayesian::~MERTrackBayesian()
{
  // Delete this instance of MERTrackBayesian
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackBayesian::SetSpecialParameters(MString FileName)
{
  m_FileName = FileName;

  MString Suffix = ".t.goodbad.rsp";

  if (MFile::FileExists(FileName) == false) {
    mgui<<"ERTrackBayesian: File \""<<FileName<<"\" not found!"<<error;
    return false;
  }
  if (FileName.EndsWith(Suffix) == false) {
    mgui<<"ERTrackBayesian: File \""<<FileName<<"\" is not of the correct type: \""<<Suffix<<"\""<<error;
    return false;
  }

  MString Prefix = FileName.Remove(FileName.Length()-Suffix.Length());

  if (MFile::FileExists(Prefix + ".t.goodbad.rsp") == true) {
    m_GoodBad.Read(Prefix + ".t.goodbad.rsp");
    m_GoodStart.Read(Prefix + ".t.start.good.rsp");
    m_BadStart.Read(Prefix + ".t.start.bad.rsp");
    m_GoodCentral.Read(Prefix + ".t.central.good.rsp");
    m_BadCentral.Read(Prefix + ".t.central.bad.rsp");
    //m_GoodCentral.Read(g_MEGAlibPath + "/resource/response/EC.good.rsp");
    //m_BadCentral.Read(g_MEGAlibPath + "/resource/response/EC.bad.rsp");
    m_GoodStop.Read(Prefix + ".t.stop.good.rsp");
    m_BadStop.Read(Prefix + ".t.stop.bad.rsp");
    m_GoodDual.Read(Prefix + ".t.dual.good.rsp");
    m_BadDual.Read(Prefix + ".t.dual.bad.rsp");

    m_SumGoodStart = m_GoodStart.GetSum();
    m_SumBadStart = m_BadStart.GetSum();

    m_SumGoodCentral = m_GoodCentral.GetSum();
    m_SumBadCentral = m_BadCentral.GetSum();

    m_SumGoodStop = m_GoodStop.GetSum();
    m_SumBadStop = m_BadStop.GetSum();

    m_SumGoodDual = m_GoodDual.GetSum();
    m_SumBadDual = m_BadDual.GetSum();

    massert(m_SumGoodStart != 0);
    massert(m_SumBadStart != 0);
    massert(m_SumGoodCentral != 0);
    massert(m_SumBadCentral != 0);
    massert(m_SumGoodStop != 0);
    massert(m_SumBadStop != 0);
    massert(m_SumGoodDual != 0);
    massert(m_SumBadDual != 0);
  } else {
    mgui<<"ERTrackBayesian: Not all files for BET found!"<<error;
    return false;
  }

  m_FileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERTrackBayesian::EvaluateTracks(MRERawEvent* RE)
{
  // First take care of pairs:
  if (RE->GetVertex() != 0) {
    return EvaluatePairs(RE);
  }

  // Now take care of the potential Compton tracks:

  const float MinEntriesGood = 1;
  const float MinEntriesBad = 10;

  const double MinNGood = 0;

  double TrackQualityFactor = MRERawEvent::c_NoQualityFactor;
  
  // The bayesian approach:
  double Etot = 0;
  double AngleIn = 0;
  double Edep = 0;
  double AngleOutTheta = 0;
  double AngleOutPhi = 0;

  double NGood = 0;
  double NBad = 0;

  int NAverages = 0;
  double Average = 0.0;
  double P_good = m_SumGoodCentral/(m_SumGoodCentral + m_SumBadCentral);
  double P_bad = m_SumBadCentral/(m_SumGoodCentral + m_SumBadCentral);
  double P_Sgood = m_SumGoodStop/(m_SumGoodStop + m_SumBadStop);
  double P_Sbad = m_SumBadStop/(m_SumGoodStop + m_SumBadStop);

  double Ratio = 0.0;
  double GoodBadRatio = 1.0;
  double TotalRatio = 1.0;

  double TotalGood = 0.0;
  double TotalBad = 0.0;

  int NPointsWithGoodStatistics = 0;

  MRESEIterator Iter;

  MRETrack* Track = 0;
  int NTracks = 0;
  for (int i = 0; i < RE->GetNRESEs(); i++) {
    if (RE->GetRESEAt(i)->GetType() == MRESE::c_Track) {
      NTracks++;
      //if (NTracks > 1) return false;
    }
  }

  bool ShowDebug = false;

  if (ShowDebug == true) {
    mout<<RE->ToString()<<endl;
  }

  for (int i = 0; i < RE->GetNRESEs(); i++) {

    if (RE->GetRESEAt(i)->GetType() == MRESE::c_Track) {
      Track = (MRETrack *) RE->GetRESEAt(i);
      if (Track->GetStartPoint() == 0) {
        mout<<"Track has no start point!"<<endl;
        continue;
      }
      massert(Track->GetNRESEs() >= 2);

      Etot = Track->GetEnergy();

      // Reinitialize:
      NAverages = 0;
      Average = 0;

      TotalGood = 0;
      TotalBad = 0;

      GoodBadRatio = 1;
      TrackQualityFactor = MRERawEvent::c_NoQualityFactor;
      NPointsWithGoodStatistics = 0;

      if (ShowDebug == true) {
        mout<<"Start z: "<<Track->GetStartPoint()->GetPosition()[2]<<endl;
      }

      MRESEIterator Iter;
      Iter.Start(Track->GetStartPoint());

      if (ShowDebug == true) {
        mout<<endl;
        mout<<"**** Sequenz: ";
        do {
          Iter.GetNextRESE();
          mout<<Iter.GetCurrent()->GetID()<<"  ";
        } while (Iter.GetNext() != 0);
        mout<<endl;
      }

      if (Track->GetNRESEs() == 2) {
        Iter.Start(Track->GetStartPoint());
        Iter.GetNextRESE();
        Etot = Track->GetEnergy();
        Edep = Iter.GetCurrent()->GetEnergy();
        AngleIn = CalculateAngleIn(Iter.GetCurrent(), Iter.GetNext())*c_Deg;
        NGood = m_GoodDual.GetInterpolated(Etot, AngleIn, Edep);
        if (NGood < MinEntriesGood) {
          NGood = MinNGood;
          NPointsWithGoodStatistics--;
        }
        NBad = m_BadDual.GetInterpolated(Etot, AngleIn, Edep);
        if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
          Ratio = NGood*m_SumBadDual/NBad/m_SumGoodDual;
          GoodBadRatio *= Ratio;
          NPointsWithGoodStatistics++;
        }

        if (ShowDebug == true) {
          mout<<"dual:      "
              <<setw(8)<<Etot<<"  "
              <<setw(8)<<AngleIn<<"  "
              <<setw(8)<<Edep<<"  "
              <<setw(8)<<0<<"  "
              <<setw(8)<<0<<"  ";
          if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
            mout<<setw(8)<<Ratio<<setw(8)<<NGood<<setw(8)<<NBad<<endl;
          } else {
            mout<<"not enough entries: G:"<<NGood<<"/"<<MinEntriesGood
                  <<" B:"<<NBad<<"/"<<MinEntriesBad<<endl;
          }      
        }
      } else {
        // Define start point:
        Iter.Start(Track->GetStartPoint());
        Iter.GetRESEAt(0);
        Edep = Iter.GetCurrent()->GetEnergy();
        AngleIn = CalculateAngleIn(Iter.GetCurrent(), Iter.GetNext())*c_Deg;
        NGood = m_GoodStart.Get(Etot, AngleIn, Edep);
        if (NGood < MinEntriesGood) {
          NGood = MinNGood;
          NPointsWithGoodStatistics--;
        }
        NBad = m_BadStart.Get(Etot, AngleIn, Edep);
        if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
          Ratio = NGood*m_SumBadStart/NBad/m_SumGoodStart;
          GoodBadRatio *= Ratio;
          NPointsWithGoodStatistics++;
        } 
        if (ShowDebug == true) {
          mout<<"start:      "
                <<setw(8)<<Etot<<"  "
                <<setw(8)<<AngleIn<<"  "
                <<setw(8)<<Edep<<"  "
                <<setw(8)<<0<<"  "
                <<setw(8)<<0<<"  ";
          if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
            mout<<setw(8)<<Ratio<<setw(8)<<NGood<<setw(8)<<NBad<<endl;
          } else {
            mout<<"not enough entries: G:"<<NGood<<"/"<<MinEntriesGood
                  <<" B:"<<NBad<<"/"<<MinEntriesBad<<endl;
          }      
        }        
        // Define central points:
        int i_max = Iter.GetNRESEs();
        for (int i = 1; i < i_max-1; ++i) {
          //         mout<<endl<<"Track element: "<<i<<endl;
          Etot -= Iter.GetCurrent()->GetEnergy();
          Iter.GetNextRESE();
          Edep = Iter.GetCurrent()->GetEnergy();
          AngleIn = CalculateAngleIn(Iter.GetPrevious(), Iter.GetCurrent())*c_Deg;
          AngleOutTheta = CalculateAngleOutTheta(Iter.GetPrevious(), Iter.GetCurrent(), Iter.GetNext())*c_Deg;
          AngleOutPhi = CalculateAngleOutPhi(Iter.GetPrevious(), Iter.GetCurrent(), Iter.GetNext())*c_Deg;
          NGood = m_GoodCentral.Get(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta);
          if (NGood < MinEntriesGood) {
            NGood = MinNGood;
            NPointsWithGoodStatistics--;
          }
          NBad = m_BadCentral.Get(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta);
          if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
            Ratio = NGood*m_SumBadCentral/NBad/m_SumGoodCentral;
            GoodBadRatio *= Ratio;
            NPointsWithGoodStatistics++;
          }
          
          if (ShowDebug == true) {
            mout<<"Central:      "
                  <<setw(8)<<Etot<<"  "
                  <<setw(8)<<AngleIn<<"  "
                  <<setw(8)<<Edep<<"  "
                  <<setw(8)<<AngleOutPhi<<"  "
                  <<setw(8)<<AngleOutTheta<<"  ";
            if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
              mout<<setw(8)<<Ratio<<setw(8)<<NGood<<setw(8)<<NBad<<endl;
            } else {
              mout<<"not enough entries: G:"<<NGood<<"/"<<MinEntriesGood
                    <<" B:"<<NBad<<"/"<<MinEntriesBad<<endl;
            }
          }
          double P_M_good = NGood/m_SumGoodCentral; //(NGood+NBad);
          double P_M_bad = NBad/m_SumBadCentral; //(NGood+NBad);
          double P_corr = P_good*P_M_good/(P_good*P_M_good + P_bad*P_M_bad);
          
          TotalGood += NGood;
          TotalBad += NBad;
          
          NAverages++;
          Average += P_corr;
        }

        // Define stop points:
        Iter.GetRESEAt(i_max-1);
        Edep = Iter.GetCurrent()->GetEnergy();
        AngleIn = CalculateAngleIn(Iter.GetPrevious(), Iter.GetCurrent())*c_Deg;
        NAverages++;
        NGood = m_GoodStop.Get(Edep, AngleIn);
        if (NGood < MinEntriesGood) {
          NGood = MinNGood;
          NPointsWithGoodStatistics--;
        }
        NBad = m_BadStop.Get(Edep, AngleIn);
        double P_M_Sgood = NGood/m_SumGoodStop; //(NGood+NBad);
        double P_M_Sbad = NBad/m_SumBadStop; //(NGood+NBad);
        double P_Scorr = P_Sgood*P_M_Sgood/(P_Sgood*P_M_Sgood + P_Sbad*P_M_Sbad);
        
        NAverages++;
        Average += P_Scorr;

        if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
          Ratio = NGood*m_SumBadStop/NBad/m_SumGoodStop;
          GoodBadRatio *= Ratio;
          NPointsWithGoodStatistics++;
        }
 
        if (ShowDebug == true) {
          mout<<"Final:        "
                <<setw(8)<<Edep<<"  "
                <<setw(8)<<AngleIn<<"  "
                <<setw(8)<<0<<"  "
                <<setw(8)<<0<<"  "
                <<setw(8)<<0<<"  ";
          if (NGood >= MinEntriesGood && NBad >= MinEntriesBad) {
            mout<<setw(8)<<Ratio<<endl;
          } else {
            mout<<"not enough entries: G:"<<NGood<<"/"<<MinEntriesGood
                  <<" B:"<<NBad<<"/"<<MinEntriesBad<<endl;
          }
        }
      }

      if (GoodBadRatio != 1 && NPointsWithGoodStatistics > 0) {
        GoodBadRatio *= m_GoodBad.Get(0.5)/m_GoodBad.Get(1.5);
        TrackQualityFactor = GoodBadRatio/(1+GoodBadRatio);
      } else {
        mout<<"No statistics for BET!"<<endl;
        TrackQualityFactor = 0;
      }


      double tP_M_good = TotalGood/m_SumGoodCentral; //(NGood+NBad);
      double tP_M_bad = TotalBad/m_SumBadCentral; //(NGood+NBad);
      //mout<<"tP_good: "<<P_good<<"  P_bad: "<<P_bad<<"  tP_M_good: "<<tP_M_good<<"  tP_M_bad: "<<tP_M_bad<<endl;
      double tP_corr = P_good*tP_M_good/(P_good*tP_M_good + P_bad*tP_M_bad);

      if (ShowDebug == true) {
        mout<<"Bayes as total value: "<<tP_corr<<endl;
        Average = (NAverages > 0) ? Average/NAverages : 0;
        mout<<"Bayes as average: "<<Average<<endl;
        mout<<"Bayes with independence assumption: "
              <<setprecision(20)<<TrackQualityFactor<<setprecision(6)<<endl;
      }
      Track->SetQualityFactor(1-TrackQualityFactor);

      TotalRatio *= TrackQualityFactor; 

    } // Track
  } // RESE

  RE->SetTrackQualityFactor(1-TotalRatio); // 0: good, 1: bad ;-)
  RE->SetPairQualityFactor(MRERawEvent::c_NoQualityFactor);

  if (ShowDebug == true) {
    mout<<RE->ToString()<<endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERTrackBayesian::SortByTrackQualityFactor(MRawEventList* List)
{
  // Sort decreasing:

  List->SortByTrackQualityFactor(false);
}


////////////////////////////////////////////////////////////////////////////////


double MERTrackBayesian::CalculateAngleIn(MRESE* Start, MRESE* Central)
{
  static int NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega!"<<endl;
    // --> RevanGeometry weis wie...
  }
  NWarnings = 1;
  
  double Angle = (Central->GetPosition()-Start->GetPosition()).Angle(MVector(0, 0, 1));
  if (Angle > TMath::Pi()/2) Angle = TMath::Pi() - Angle;

  return Angle;
}


////////////////////////////////////////////////////////////////////////////////


double MERTrackBayesian::CalculateAngleOutTheta(MRESE* Start, MRESE* Central, MRESE* Stop)
{
  MVector In = Central->GetPosition() - Start->GetPosition();
  MVector Out = Stop->GetPosition() - Central->GetPosition();

  return In.Angle(Out);
}


////////////////////////////////////////////////////////////////////////////////


double MERTrackBayesian::CalculateAngleOutPhi(MRESE* Start, MRESE* Central, MRESE* Stop)
{
  static int NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega!"<<endl;
    // --> RevanGeometry weiß wie...
  }
  NWarnings = 1;

  MVector E1 = (Central->GetPosition() - Start->GetPosition()).Cross(MVector(0, 0, 1));
  MVector E2 = (Central->GetPosition() - Start->GetPosition()).Cross(Stop->GetPosition() - Central->GetPosition());

  return E1.Angle(E2);
}


////////////////////////////////////////////////////////////////////////////////


MString MERTrackBayesian::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  if (CoreOnly == false) {
    out<<"# Tracking - Bayesian options:"<<endl;
    out<<"# "<<endl;
  }
  out<<"# File name:                  "<<m_FileName<<endl;
  out<<MERTrack::ToString(true);
  if (CoreOnly == false) {
    out<<"# "<<endl;
  }
  
  return out.str().c_str();
}


// MERTrackBayesian.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
