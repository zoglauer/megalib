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


MResponseTracking::MResponseTracking()
{
  // Construct an instance of MResponseTracking
}


////////////////////////////////////////////////////////////////////////////////


MResponseTracking::~MResponseTracking()
{
  // Delete this instance of MResponseTracking
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseTracking::SetRevanConfigurationFileName(const MString FileName)
{
  // Set and verify the simulation file name

  if (MFile::Exists(FileName) == false) {
    mout<<"*** Error: \""<<FileName<<"\" does not exist"<<endl;
    return false;
  }
  m_RevanCfgFileName = FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseTracking::OpenSimulationFile()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;
  m_ReReader->SetGeometry(m_ReGeometry);

  MSettingsRevan Cfg(false);
  Cfg.Read(m_RevanCfgFileName);
  m_ReReader->SetSettings(&Cfg);

  m_ReReader->SetNTrackSequencesToKeep(numeric_limits<int>::max());

  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseTracking::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  //cout<<"Trying to generate a pdf for track recognition - please stand by..."<<endl;

  vector<Quadruple> QuadruplesGoodCenter;
  vector<Quadruple> QuadruplesBadCenter;
  vector<Quadruple> QuadruplesGoodStop;
  vector<Quadruple> QuadruplesBadStop;
  vector<Quadruple> QuadruplesGoodStart;
  vector<Quadruple> QuadruplesBadStart;
  // vector<Quadruple>::iterator QuadruplesIter;

  MRETrack* Track = 0;
  MRESE* RESE = 0;
  MRERawEvent* RE = 0;
  MRawEventList* REList = 0;

  int r_max, e_max;

  // the data:
  double Etot;
  double Edep;
  double AngleIn;
  double AngleOutTheta;
  double AngleOutPhi;

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
  MResponseMatrixO1 GoodBadTable("GoodBadTable", X1Axis); 
  GoodBadTable.SetAxisNames("GoodBad");


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

  MResponseMatrixO3 PdfDualGood("TracksStartGood", X1Axis, X2Axis, X3Axis);
  PdfDualGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  MResponseMatrixO3 PdfDualBad("TracksStartBad", X1Axis, X2Axis, X3Axis);
  PdfDualBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


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

  MResponseMatrixO3 PdfStartGood("TracksStartGood", X1Axis, X2Axis, X3Axis);
  PdfStartGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");
  MResponseMatrixO3 PdfStartBad("TracksStartBad", X1Axis, X2Axis, X3Axis);
  PdfStartBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}");


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


  MResponseMatrixO5 PdfGood("TracksGood", X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  PdfGood.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");
  MResponseMatrixO5 PdfBad("TracksBad", X1Axis, X2Axis, X3Axis, X4Axis, X5Axis);
  PdfBad.SetAxisNames("E_{tot}", "Angle_{In}", "E_{dep}", "Angle_{Out, #varphi}", "Angle_{Out, #vartheta}");


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

  MResponseMatrixO2 PdfStopGood("PdfStopGood", X1Axis, X2Axis);
  PdfStopGood.SetAxisNames("E_{tot}", "Angle_{In}");
  MResponseMatrixO2 PdfStopBad("PdfStopBad", X1Axis, X2Axis);
  PdfStopBad.SetAxisNames("E_{tot}", "Angle_{In}");


  int Counter = 0;
  while (InitializeNextMatchingEvent() == true) {

    //g_Verbosity = 1;

    //cout<<"New event"<<endl;
    
    RESE = 0;
    RE = 0;
    REList = m_ReReader->GetRawEventList();

    QuadruplesGoodStart.clear();
    QuadruplesBadStart.clear();

    QuadruplesGoodCenter.clear();
    QuadruplesBadCenter.clear();

    QuadruplesGoodStop.clear();
    QuadruplesBadStop.clear();

    // First try to find the tracks in all the RawEvents:
    r_max = REList->GetNRawEvents();
    for (int r = 0; r < r_max; ++r) {
      RE = REList->GetRawEventAt(r);

      
      e_max = RE->GetNRESEs();
      for (int e = 0; e < e_max; ++e) {
        RESE = RE->GetRESEAt(e);

        if (RESE->GetType() != MRESE::c_Track) continue;

        bool AllGood = true;

        Track = (MRETrack*) RESE;
        mdebug<<"Looking at track: "<<Track->ToString()<<endl;

        MRESEIterator Iter;
        Iter.Start(Track->GetStartPoint());

        if (Iter.GetNRESEs() < 2) continue;


        if (Iter.GetNRESEs() == 2) {
          Iter.GetNextRESE();
          Etot = Track->GetEnergy();
          if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Etot) == true &&
              IsTrackStop(*Iter.GetCurrent(), *Iter.GetNext(), Etot - Iter.GetCurrent()->GetEnergy()) == true) {
            mdebug<<"GOOD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfDualGood.Add(Etot, 
                            CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                            Iter.GetCurrent()->GetEnergy());
          } else {
            mdebug<<"BAD dual: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfDualBad.Add(Etot, 
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
            PdfStartGood.Add(Etot, 
                             CalculateAngleIn(*Iter.GetCurrent(), *Iter.GetNext())*c_Deg,
                             Iter.GetCurrent()->GetEnergy());
          } else {
            mdebug<<"BAD start: "<<Iter.GetCurrent()->GetID()<<" - "<<Iter.GetNext()->GetID()<<" - "<<Etot<<endl;
            PdfStartBad.Add(Etot, 
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
              PdfGood.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
            } else {
              // Retrieve the data:
              Edep = Iter.GetCurrent()->GetEnergy();
              AngleIn = CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg;
              AngleOutPhi = CalculateAngleOutPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              AngleOutTheta = CalculateAngleOutTheta(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())*c_Deg;
              PdfBad.Add(Etot, AngleIn, Edep, AngleOutPhi, AngleOutTheta, 1);
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
            PdfStopGood.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
          } else {
            mdebug<<"BAD stop: "<<Iter.GetPrevious()->GetID()
                  <<" - "<<Iter.GetCurrent()->GetID()<<" - "<<Etot<<endl;
            PdfStopBad.Add(Etot, CalculateAngleIn(*Iter.GetPrevious(), *Iter.GetCurrent())*c_Deg);
            AllGood = false;
          }
        }
        if (AllGood == false) {
          GoodBadTable.Add(0.5, 1);
          //mdebug<<"No good sequence exists"<<endl<<endl<<endl<<endl;
        } else {
          GoodBadTable.Add(1.5, 1);
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
    
    if (++Counter % m_SaveAfter == 0) {
      GoodBadTable.Write(m_ResponseName + ".t.goodbad.rsp", true);
  
      PdfStartGood.Write(m_ResponseName + ".t.start.good.rsp", true);
      PdfStartBad.Write(m_ResponseName + ".t.start.bad.rsp", true);
  
      PdfDualGood.Write(m_ResponseName + ".t.dual.good.rsp", true);
      PdfDualBad.Write(m_ResponseName + ".t.dual.bad.rsp", true);
      
      PdfGood.Write(m_ResponseName + ".t.central.good.rsp", true);
      PdfBad.Write(m_ResponseName + ".t.central.bad.rsp", true);
      
      PdfStopGood.Write(m_ResponseName + ".t.stop.good.rsp", true);
      PdfStopBad.Write(m_ResponseName + ".t.stop.bad.rsp", true);
    }
  }
  
  GoodBadTable.Write(m_ResponseName + ".t.goodbad.rsp", true);
  
  PdfStartGood.Write(m_ResponseName + ".t.start.good.rsp", true);
  PdfStartBad.Write(m_ResponseName + ".t.start.bad.rsp", true);
  
  PdfDualGood.Write(m_ResponseName + ".t.dual.good.rsp", true);
  PdfDualBad.Write(m_ResponseName + ".t.dual.bad.rsp", true);

  PdfGood.Write(m_ResponseName + ".t.central.good.rsp", true);
  PdfBad.Write(m_ResponseName + ".t.central.bad.rsp", true);

  PdfStopGood.Write(m_ResponseName + ".t.stop.good.rsp", true);
  PdfStopBad.Write(m_ResponseName + ".t.stop.bad.rsp", true);

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
