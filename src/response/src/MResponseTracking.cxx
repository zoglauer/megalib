/*
 * MResponseTracking.cxx
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
// MResponseTracking
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseTracking.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MSettingsRevan.h"
#include "MRESEIterator.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseTracking)
#endif

/******************************************************************************/

class Quadruple
{
public:
  Quadruple(int Start, int Central, int Stop, int Etot = 0) {
    massert(Start > 0); massert(Central > 0); massert(Stop > 0);
    massert(Start != Central); massert(Stop != Central); massert(Start != Stop);

    m_Start = Start;
    m_Stop = Stop;
    m_Central = Central;
    m_Etot = Etot;
  }
  // Copy constuctor, destructor and Assignment operator 
  // should be correctly defined by the compiler

  bool operator==(const Quadruple& T) {
    if (m_Start != T.m_Start) return false;
    if (m_Central != T.m_Central) return false;
    if (m_Stop != T.m_Stop) return false;
    if (m_Etot != T.m_Etot) return false;
    return true;
  }

  int GetStart() { return m_Start; }
  int GetCentral() { return m_Central; }
  int GetStop() { return m_Stop; }
  int GetEtot() { return m_Etot; }

  friend ostream& operator<<(ostream& os, Quadruple& T);

private:
  int m_Start;
  int m_Central;
  int m_Stop;
  int m_Etot;
};


/******************************************************************************/

ostream& operator<<(ostream& os, Quadruple& T)
{
  os<<"("<<T.m_Start<<", "<<T.m_Central<<", "<<T.m_Stop<<", "<<T.m_Etot<<")";
  return os;
}


////////////////////////////////////////////////////////////////////////////////


//! Default contructor
MResponseTracking::MResponseTracking()
{
  m_ResponseNameSuffix = "t";
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseTracking::~MResponseTracking()
{
  // Nothing to delete
}

  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseTracking::Initialize() 
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  m_ReReader->SetNTrackSequencesToKeep(numeric_limits<int>::max());
  if (m_ReReader->PreAnalysis() == false) return false;

  
  // create axis:
  vector<float> Axis;
  vector<float> X1Axis;
  vector<float> X2Axis;
  vector<float> X3Axis;
  vector<float> X4Axis;
  vector<float> X5Axis;
  

  vector<float> EmptyAxis;
  EmptyAxis.push_back(1E-10);
  EmptyAxis.push_back(1E+10);


  // Global good/bad:
  X1Axis.clear();
  X1Axis = CreateEquiDist(0, 2, 2);
  m_GoodBadTable.SetName("GoodBadTable");
  m_GoodBadTable.SetAxis(X1Axis); 
  m_GoodBadTable.SetAxisNames("GoodBad");


  // Matrix Dual:

  // Etot
  X1Axis.clear();
  X1Axis = CreateEquiDist(1, 500, 15);
  Axis = CreateEquiDist(600, 1000, 4, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
 
  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 6);
  //X2Axis = CreateEquiDist(0, 90, 1);

  // Edep
  X3Axis.clear();
  X3Axis = CreateEquiDist(1, 500, 15); // Mega
  // X3Axis = CreateEquiDist(1, 500, 50); // Superior
  Axis = CreateEquiDist(600, 1000, 4, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  m_PdfDualGood.SetName("TracksStartGood");
  m_PdfDualGood.SetAxis(X1Axis, X2Axis, X3Axis);
  m_PdfDualGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  m_PdfDualBad.SetName("TracksStartBad");
  m_PdfDualBad.SetAxis(X1Axis, X2Axis, X3Axis);
  m_PdfDualBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


  // Matrix Start:

  // Etot
  X1Axis.clear();
  X1Axis = CreateEquiDist(1, 1500, 10);
  Axis = CreateEquiDist(1800, 3000, 4);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
  Axis = CreateEquiDist(4000, 10000, 6, c_NoBound, 100000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));

  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 6);
  //X2Axis = CreateEquiDist(0, 90, 1);

  // Edep
  X3Axis.clear();
  // X3Axis = CreateEquiDist(1, 500, 20); // Mega
  X3Axis = CreateEquiDist(1, 500, 50); // Superior
  Axis = CreateEquiDist(600, 1000, 5, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  m_PdfStartGood.SetName("TracksStartGood");
  m_PdfStartGood.SetAxis(X1Axis, X2Axis, X3Axis);
  m_PdfStartGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  m_PdfStartBad.SetName("TracksStartBad");
  m_PdfStartBad.SetAxis(X1Axis, X2Axis, X3Axis);
  m_PdfStartBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


  // Matrix Central:

  // Edep:
  X3Axis.clear();
  // X3Axis = CreateEquiDist(1, 100, 2);  // Mega
  // Axis = CreateEquiDist(115, 400, 19); // Mega
  // copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));
  X3Axis = CreateEquiDist(1, 400, 40); // Superior
  Axis = CreateEquiDist(500, 1000, 5, c_NoBound, 10000);
  copy(Axis.begin(), Axis.end(), back_inserter(X3Axis));

  // out phi
  X4Axis.clear();
  //X4Axis = CreateEquiDist(0, 180, 1);
  X4Axis = CreateEquiDist(0, 180, 6);

  // out theta
  X5Axis.clear();
  //X5Axis = CreateEquiDist(0, 180, 1);
  X5Axis = CreateEquiDist(0, 90, 6);
  Axis = CreateEquiDist(120, 180, 2);
  copy(Axis.begin(), Axis.end(), back_inserter(X5Axis));


  m_PdfGood.SetName("TracksGood");
  m_PdfGood.SetAxis(X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  m_PdfGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");
  m_PdfBad.SetName("TracksBad");
  m_PdfBad.SetAxis(X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  m_PdfBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");


  // Matrix Stop:

  // Etot
  X1Axis.clear();
  // X1Axis = CreateEquiDist(1, 1500, 45); // Mega
  X1Axis = CreateEquiDist(1, 1500, 150); // Superior
  Axis = CreateEquiDist(1800, 3000, 12);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));
  Axis = CreateEquiDist(4000, 10000, 6, c_NoBound, 100000);
  copy(Axis.begin(), Axis.end(), back_inserter(X1Axis));

  // Angle in
  X2Axis.clear();
  X2Axis = CreateEquiDist(0, 90, 45);

  m_PdfStopGood.SetName("PdfStopGood");
  m_PdfStopGood.SetAxis(X1Axis, X2Axis);
  m_PdfStopGood.SetAxisNames("E_{tot}", "Angle_{In}");
  m_PdfStopBad.SetName("PdfStopBad");
  m_PdfStopBad.SetAxis(X1Axis, X2Axis);
  m_PdfStopBad.SetAxisNames("E_{tot}", "Angle_{In}");  
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseTracking::Analyze()
{
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  vector<Quadruple> QuadruplesGoodCenter;
  vector<Quadruple> QuadruplesBadCenter;
  vector<Quadruple> QuadruplesGoodStop;
  vector<Quadruple> QuadruplesBadStop;
  vector<Quadruple> QuadruplesGoodStart;
  vector<Quadruple> QuadruplesBadStart;
  // vector<Quadruple>::iterator QuadruplesIter;
  
  
  // the data:
  double Etot;
  double Edep;
  double AngleIn;
  double AngleOutTheta;
  double AngleOutPhi;
  
  
  
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  MRawEventList* REList = m_ReReader->GetRawEventList();
  
  QuadruplesGoodStart.clear();
  QuadruplesBadStart.clear();
  
  QuadruplesGoodCenter.clear();
  QuadruplesBadCenter.clear();
  
  QuadruplesGoodStop.clear();
  QuadruplesBadStop.clear();
  
  // First try to find the tracks in all the RawEvents:
  int r_max = REList->GetNRawEvents();
  for (int r = 0; r < r_max; ++r) {
    RE = REList->GetRawEventAt(r);
    
    
    int e_max = RE->GetNRESEs();
    for (int e = 0; e < e_max; ++e) {
      RESE = RE->GetRESEAt(e);
      
      if (RESE->GetType() != MRESE::c_Track) continue;
      
      bool AllGood = true;
      
      MRETrack* Track = (MRETrack*) RESE;
      mdebug<<"Looking at track: "<<Track->ToString()<<endl;
      
      MRESEIterator Iter;
      Iter.Start(Track->GetStartPoint());
      
      if (Iter.GetNRESEs() < 2) continue;
      
      
      if (Iter.GetNRESEs() == 2) {
        Iter.GetNextRESE();
        double Etot = Track->GetEnergy();
        if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Etot) == true &&
          IsTrackStop(*Iter.GetCurrent(), *Iter.GetNext(), Etot - Iter.GetCurrent()->GetEnergy()) == true) {
          mdebug<<"GOOD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
        m_PdfDualGood.Add(Etot, 
                          CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                          Iter.GetCurrent()->GetEnergy());
          } else {
            mdebug<<"BAD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            m_PdfDualBad.Add(Etot, 
                             CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                             Iter.GetCurrent()->GetEnergy());
            AllGood = false;
          }
      } else {
        // Start of the track:
        Iter.GetNextRESE();
        Etot = Track->GetEnergy();
        if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Etot) == true) {
          mdebug<<"GOOD start: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
          m_PdfStartGood.Add(Etot, 
                             CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                             Iter.GetCurrent()->GetEnergy());
        } else {
          mdebug<<"BAD start: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
          m_PdfStartBad.Add(Etot, 
                            CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                            Iter.GetCurrent()->GetEnergy());
          AllGood = false;
        }
        
        
        // Central part of the track
        Iter.GetNextRESE();
        while (Iter.GetNext() != 0) {
          Etot -= Iter.GetPrevious()->GetEnergy();
          
          // Decide if it is good or bad...
          // In the current implementation/simulation the hits have to be in increasing order...
          if (AreReseInSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), Etot) == true) {
            // Retrieve the data:
            Edep = Iter.GetCurrent()->GetEnergy();
            AngleIn = CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg;
            AngleOutPhi = CalculateAngleOutPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
            AngleOutTheta = CalculateAngleOutTheta(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
            mdebug<<"GOOD central: "<<Iter.GetPrevious()->GetID()<<" - "<<Iter.GetCurrent()->GetID()
            <<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            m_PdfGood.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
          } else {
            // Retrieve the data:
            Edep = Iter.GetCurrent()->GetEnergy();
            AngleIn = CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg;
            AngleOutPhi = CalculateAngleOutPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
            AngleOutTheta = CalculateAngleOutTheta(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
            m_PdfBad.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
            mdebug<<"BAD central: "<<Iter.GetPrevious()->GetID()<<" - "<<Iter.GetCurrent()->GetID()
            <<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            
            AllGood = false;
          } // Add good / bad
          Iter.GetNextRESE();
        } // If we have a next element
        
        
        // The stop section:
        Etot -= Iter.GetPrevious()->GetEnergy();
        
        if (IsTrackStop(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot) == true) {
          mdebug<<"GOOD stop: "<<Iter.GetPrevious()->GetID()
          <<" - "<<Iter.GetCurrent()->GetID()<<" - "<<Etot<<endl;
          m_PdfStopGood.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
        } else {
          mdebug<<"BAD stop: "<<Iter.GetPrevious()->GetID()
          <<" - "<<Iter.GetCurrent()->GetID()<<" - "<<Etot<<endl;
          m_PdfStopBad.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
          AllGood = false;
        }
      }
      if (AllGood == false) {
        m_GoodBadTable.Add(0.5, 1);
        //mdebug<<"No good sequence exists"<<endl<<endl<<endl<<endl;
      } else {
        m_GoodBadTable.Add(1.5, 1);
        mdebug<<"One good sequence exists"<<endl<<endl<<endl<<endl;
      }
    } // all reses
  } // all raw events
  
  // mdebug<<"Good triples:"<<endl;
  for (unsigned int t = 0; t < QuadruplesGoodCenter.size(); ++t) {
    //cout<<"t: "<<QuadruplesGood[t];
    //mdebug<<t<<": "<<QuadruplesGood[t]<<endl;
  }
  
  //mdebug<<"Bad triples:"<<endl;
  for (unsigned int t = 0; t < QuadruplesBadCenter.size(); ++t) {
    //mdebug<<t<<": "<<QuadruplesBad[t]<<endl;
  }
  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseTracking::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseTracking::Save()
{
  MResponseBuilder::Save();
  
  m_GoodBadTable.Write(GetFilePrefix() + ".goodbad" + m_Suffix, true);
  
  m_PdfStartGood.Write(GetFilePrefix() + ".start.good" + m_Suffix, true);
  m_PdfStartBad.Write(GetFilePrefix() + ".start.bad" + m_Suffix, true);
  
  m_PdfDualGood.Write(GetFilePrefix() + ".dual.good" + m_Suffix, true);
  m_PdfDualBad.Write(GetFilePrefix() + ".dual.bad" + m_Suffix, true);

  m_PdfGood.Write(GetFilePrefix() + ".central.good" + m_Suffix, true);
  m_PdfBad.Write(GetFilePrefix() + ".central.bad" + m_Suffix, true);

  m_PdfStopGood.Write(GetFilePrefix() + ".stop.good" + m_Suffix, true);
  m_PdfStopBad.Write(GetFilePrefix() + ".stop.bad" + m_Suffix, true);

  return true;
}



/******************************************************************************
 * 
 */
double MResponseTracking::CalculateAngleIn(MRESE& Start, MRESE& Central)
{
  static long NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega-type!"<<endl;
    // --> RevanGeometry weiß wie...
  }
  NWarnings = 1;
  
  double Angle = (Central.GetPosition()-Start.GetPosition()).Angle(MVector(0, 0, 1));
  if (Angle > TMath::Pi()/2) Angle = TMath::Pi() - Angle;

  return Angle;
}


/******************************************************************************
 *
 */
double MResponseTracking::CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop)
{
  MVector In = Central.GetPosition() - Start.GetPosition();
  MVector Out = Stop.GetPosition() - Central.GetPosition();

  return In.Angle(Out);
}


/******************************************************************************
 * 
 */
double MResponseTracking::CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop)
{
  static long NWarnings = 0;
  if (NWarnings == 0) {
    merr<<"Does not work for all geometries --> Only Mega-type!"<<endl;
    // --> RevanGeometry weiß wie...
  }
  NWarnings = 1;

  MVector E1 = (Central.GetPosition() - Start.GetPosition()).Cross(MVector(0, 0, 1));
  MVector E2 = (Central.GetPosition() - Start.GetPosition()).Cross(Stop.GetPosition() - Central.GetPosition());

  return E1.Angle(E2);
}


// MResponseTracking.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
