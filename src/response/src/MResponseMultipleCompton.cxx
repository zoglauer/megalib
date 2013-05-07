/*
 * MResponseMultipleCompton.cxx
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
// MResponseMultipleCompton
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleCompton.h"

// Standard libs:
#include <limits>
#include <algorithm>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MRESEIterator.h"
#include "MERCSRBayesian.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseMultipleCompton)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleCompton::MResponseMultipleCompton()
{
  // Construct an instance of MResponseMultipleCompton

  m_RevanCfgFileName = g_StringNotDefined;
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 5;
  m_CSRMaxLength = 7;

  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;

  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;
}


////////////////////////////////////////////////////////////////////////////////


MResponseMultipleCompton::~MResponseMultipleCompton()
{
  // Delete this instance of MResponseMultipleCompton
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::SetRevanConfigurationFileName(const MString FileName)
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


bool MResponseMultipleCompton::OpenSimulationFile()
{
  // Load the simulation file --- has to be called after the geometry is loaded

  m_ReReader = new MRawEventAnalyzer();
  m_ReReader->SetGeometry(m_ReGeometry);
  if (m_ReReader->SetInputModeFile(m_SimulationFileName) == false) return false;

  MSettingsRevan Cfg(false);
  Cfg.Read(m_RevanCfgFileName);
  m_ReReader->SetSettings(&Cfg);

  m_CSRMaxLength = Cfg.GetCSRMaxNHits();

  m_ReReader->SetCSROnlyCreateSequences(true);

  if (m_ReReader->PreAnalysis() == false) return false;

  m_SiReader = new MFileEventsSim(m_SiGeometry);
  if (m_SiReader->Open(m_SimulationFileName) == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::CreateMatrices()
{
  double MaxCosineLimit = 10;

  // Axis representing the sequence length:
  vector<float> AxisSequenceLength;
  for (int i = 2; i <= m_CSRMaxLength+1; ++i) {
    AxisSequenceLength.push_back(i);
  }
  MString NameAxisSequenceLength = "Sequence length";

  // Material: 0: unknown, 1: Si, 2: Ge, 3: Xe, 4: CsI
  // Make sure this is identical with: MERCSRBayesian::GetMaterial()
  vector<float> AxisMaterial;
  AxisMaterial = CreateEquiDist(-0.5, 4.5, 5);
  MString NameAxisMaterial = "Material (0: ?, 1: Si, 2: Ge, 3: Xe, 4: CsI)";

  // Compton scatter angle axis:
  // Make sure it starts, well below 0 and exceeds (slightly) 1!
  vector<float> AxisComptonScatterAngleStart;
  AxisComptonScatterAngleStart = CreateEquiDist(-1.5, 1.1, 26, -MaxCosineLimit, +MaxCosineLimit);
  MString NameAxisComptonScatterAngleStart = "cos#varphi";

  // Scatter probability axis
  vector<float> AxisScatterProbability = CreateEquiDist(-0.025, 1.025, 42);
  MString NameAxisScatterProbability = "Scatter probability";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyDistances = CreateLogDist(15, 5000, 38, 1, 20000);
  MString NameAxisTotalEnergyDistances = "Energy [keV]";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyStart = CreateLogDist(100, 5000, 38, 1, 20000);
  MString NameAxisTotalEnergyStart = "Energy [keV]";


  // Make sure it starts, well below 0 and exceeds (slightly) 1!
  vector<float> AxisComptonScatterAngleDual;
  AxisComptonScatterAngleDual = CreateEquiDist(-1.5, 1.1, 26, -MaxCosineLimit, +MaxCosineLimit);
  MString NameAxisComptonScatterAngleDual = "cos#varphi";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyDual = CreateLogDist(150, 2000, 18, 1, 20000);
  MString NameAxisTotalEnergyDual = "Energy [keV]";

  // Scatter probability axis
  vector<float> AxisScatterProbabilityDual = CreateEquiDist(0, 1, 15, -0.025, 1.025);
  MString NameAxisScatterProbabilityDual = "Scatter probability";
  


  // Global good/bad:
  m_GoodBadTable = MResponseMatrixO2("MC: Good/Bad ratio (=prior)", 
                                     CreateEquiDist(0, 2, 2), 
                                     AxisSequenceLength); 
  m_GoodBadTable.SetAxisNames("GoodBad", 
                              NameAxisSequenceLength);

  // Dual:
  m_PdfDualGood = MResponseMatrixO4("MC: Dual (good)", 
                                    AxisTotalEnergyDual,
                                    AxisComptonScatterAngleDual,  
                                    AxisScatterProbabilityDual,
                                    AxisMaterial);
  m_PdfDualGood.SetAxisNames(NameAxisTotalEnergyStart, 
                             NameAxisComptonScatterAngleStart, 
                             NameAxisScatterProbabilityDual, 
                             NameAxisMaterial);
  m_PdfDualBad = MResponseMatrixO4("MC: Dual (bad)", 
                                   AxisTotalEnergyDual,
                                   AxisComptonScatterAngleDual,  
                                   AxisScatterProbabilityDual,
                                   AxisMaterial);
  m_PdfDualBad.SetAxisNames(NameAxisTotalEnergyStart, 
                            NameAxisComptonScatterAngleStart, 
                            NameAxisScatterProbabilityDual, 
                            NameAxisMaterial);
  

  // Start point:
  m_PdfStartGood = MResponseMatrixO4("MC: Start (good)", 
                                     AxisTotalEnergyStart,
                                     AxisComptonScatterAngleStart, 
                                     AxisSequenceLength, 
                                     AxisMaterial);
  m_PdfStartGood.SetAxisNames(NameAxisTotalEnergyStart, 
                              NameAxisComptonScatterAngleStart, 
                              NameAxisSequenceLength, 
                              NameAxisMaterial);
  m_PdfStartBad = MResponseMatrixO4("MC: Start (bad)", 
                                    AxisTotalEnergyStart, 
                                    AxisComptonScatterAngleStart, 
                                    AxisSequenceLength, 
                                    AxisMaterial);
  m_PdfStartBad.SetAxisNames(NameAxisTotalEnergyStart, 
                             NameAxisComptonScatterAngleStart, 
                             NameAxisSequenceLength, 
                             NameAxisMaterial);


  // Track:
  m_PdfTrackGood = MResponseMatrixO6("MC: Track (good)", 
                                     CreateEquiDist(-0.5, 1.5, 36, c_NoBound, MaxCosineLimit),
                                     CreateEquiDist(-0.5, 1.5, 1, c_NoBound, MaxCosineLimit),
                                     CreateEquiDist(0, 1000000, 1),
                                     CreateLogDist(500, 10000, 10, 0, 100000, 0, false),
                                     AxisSequenceLength, 
                                     AxisMaterial);
  m_PdfTrackGood.SetAxisNames("#Delta #alpha [deg]", 
                              "#alpha_{G} [deg]", 
                              "d [cm]", 
                              "E_{e}", 
                              NameAxisSequenceLength, 
                              NameAxisMaterial);
  m_PdfTrackBad = MResponseMatrixO6("MC: Track (bad)", 
                                    CreateEquiDist(-0.5, 1.5, 36, c_NoBound, MaxCosineLimit),
                                    CreateEquiDist(-0.5, 1.5, 1, c_NoBound, MaxCosineLimit),
                                    CreateEquiDist(0, 1000000, 1),
                                    CreateLogDist(500, 10000, 10, 0, 100000, 0, false),
                                    AxisSequenceLength, 
                                    AxisMaterial);
  m_PdfTrackBad.SetAxisNames("#Delta cos#alpha [deg]", 
                             "cos#alpha_{G} [deg]", 
                             "d [cm]", 
                             "E_{e}", 
                             NameAxisSequenceLength, 
                             NameAxisMaterial);

  
  // Compton scatter distance:
  m_PdfComptonScatterProbabilityGood = MResponseMatrixO4("MC: Compton distance (good)", 
                                        AxisScatterProbability,
                                        AxisTotalEnergyDistances, 
                                        AxisSequenceLength, 
                                        AxisMaterial);
  m_PdfComptonScatterProbabilityGood.SetAxisNames(NameAxisScatterProbability, 
                                 NameAxisTotalEnergyDistances, 
                                 NameAxisSequenceLength, 
                                 NameAxisMaterial);
  m_PdfComptonScatterProbabilityBad = MResponseMatrixO4("MC: Compton distance (bad)", 
                                       AxisScatterProbability, 
                                       AxisTotalEnergyDistances, 
                                       AxisSequenceLength, 
                                       AxisMaterial);
  m_PdfComptonScatterProbabilityBad.SetAxisNames(NameAxisScatterProbability, 
                                NameAxisTotalEnergyDistances, 
                                NameAxisSequenceLength, 
                                NameAxisMaterial);


  // Lastdistance:
  m_PdfPhotoAbsorptionProbabilityGood = MResponseMatrixO4("MC: Photo distance (good)", 
                                            AxisScatterProbability, 
                                            AxisTotalEnergyDistances, 
                                            AxisSequenceLength, 
                                            AxisMaterial);
  m_PdfPhotoAbsorptionProbabilityGood.SetAxisNames(NameAxisScatterProbability, 
                                     NameAxisTotalEnergyDistances, 
                                     NameAxisSequenceLength, 
                                     NameAxisMaterial);
  m_PdfPhotoAbsorptionProbabilityBad = MResponseMatrixO4("MC: Photo distance (bad)", 
                                           AxisScatterProbability, 
                                           AxisTotalEnergyDistances, 
                                           AxisSequenceLength, 
                                           AxisMaterial);
  m_PdfPhotoAbsorptionProbabilityBad.SetAxisNames(NameAxisScatterProbability, 
                                    NameAxisTotalEnergyDistances, 
                                    NameAxisSequenceLength, 
                                    NameAxisMaterial);


  // CentralCompton:
  
  // Assymetries would be best handled if -1 .. 1
  //vector<float> AxisDifferenceComptonScatterAngle = 
  //  CreateLogDist(1E-3, 2, 18, 0.0000001, MaxCosineLimit);
  vector<float> AxisDifferenceComptonScatterAngle; 
  vector<float> A = CreateLogDist(0.003, 2, 14, c_NoBound, MaxCosineLimit);
  for (unsigned int i = A.size()-1; i < A.size(); --i) {
    AxisDifferenceComptonScatterAngle.push_back(-A[i]);
  }
  for (unsigned int i = 0; i < A.size(); ++i) {
    AxisDifferenceComptonScatterAngle.push_back(A[i]);
  }
  MString NameAxisDifferenceComptonScatterAngle = 
    "cos#varphi_{E} - cos#varphi_{G}";

  vector<float> AxisComptonScatterAngle;
  AxisComptonScatterAngle = CreateEquiDist(-1.4, 1.2, 13, -MaxCosineLimit, c_NoBound);
  MString NameAxisComptonScatterAngle = NameAxisComptonScatterAngleStart;


  vector<float> AxisDistance = 
    CreateLogDist(0.2, 10, 7, 0.01, 100, 0, false); 
  MString NameAxisDistance = "Distance [cm]";

  vector<float> AxisTotalEnergy = 
    CreateLogDist(100, 1500, 4, 1, 10000, 0, false);
  //CreateLogDist(1, 10000, 1); //, 1, 10000, 0, false);
  //mimp<<"No total energy bins!"<<show;
  MString NameAxisTotalEnergy = "E_{tot} [keV]";

  m_PdfComptonGood = MResponseMatrixO6("MC: Central (good)", 
                                       AxisDifferenceComptonScatterAngle,
                                       AxisComptonScatterAngle, 
                                       AxisDistance,
                                       AxisTotalEnergy,
                                       AxisSequenceLength, 
                                       AxisMaterial);
  m_PdfComptonGood.SetAxisNames(NameAxisDifferenceComptonScatterAngle, 
                                NameAxisComptonScatterAngle, 
                                NameAxisDistance, 
                                NameAxisTotalEnergy, 
                                NameAxisSequenceLength, 
                                NameAxisMaterial);
  m_PdfComptonBad = MResponseMatrixO6("MC: Central (bad)", 
                                      AxisDifferenceComptonScatterAngle, 
                                      AxisComptonScatterAngle,
                                      AxisDistance,
                                      AxisTotalEnergy,
                                      AxisSequenceLength, 
                                      AxisMaterial);
  m_PdfComptonBad.SetAxisNames(NameAxisDifferenceComptonScatterAngle, 
                               NameAxisComptonScatterAngle, 
                               NameAxisDistance, 
                               NameAxisTotalEnergy, 
                               NameAxisSequenceLength, 
                               NameAxisMaterial);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::SaveMatrices()
{
  mout<<"Stared saving matrices... Please remain calm ... and don't throw your computer out of the window!"<<endl;
  m_GoodBadTable.Write(m_ResponseName + ".mc.goodbad.rsp", true);
  
  m_PdfDualGood.Write(m_ResponseName + ".mc.dual.good.rsp", true);
  m_PdfDualBad.Write(m_ResponseName + ".mc.dual.bad.rsp", true);
  
  m_PdfStartGood.Write(m_ResponseName + ".mc.start.good.rsp", true);
  m_PdfStartBad.Write(m_ResponseName + ".mc.start.bad.rsp", true);
  
  m_PdfTrackGood.Write(m_ResponseName + ".mc.track.good.rsp", true);
  m_PdfTrackBad.Write(m_ResponseName + ".mc.track.bad.rsp", true);
  
  m_PdfComptonGood.Write(m_ResponseName + ".mc.compton.good.rsp", true);
  m_PdfComptonBad.Write(m_ResponseName + ".mc.compton.bad.rsp", true);
  
  m_PdfComptonScatterProbabilityGood.Write(m_ResponseName + ".mc.comptondistance.good.rsp", true);
  m_PdfComptonScatterProbabilityBad.Write(m_ResponseName + ".mc.comptondistance.bad.rsp", true);
      
  m_PdfPhotoAbsorptionProbabilityGood.Write(m_ResponseName + ".mc.photodistance.good.rsp", true);
  m_PdfPhotoAbsorptionProbabilityBad.Write(m_ResponseName + ".mc.photodistance.bad.rsp", true);
  mout<<"Matrices saved!"<<endl;
      
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::CreateResponse()
{
  // Create the multiple Compton response

  if ((m_SiGeometry = LoadGeometry(false, 0.0)) == 0) return false;
  if ((m_ReGeometry = LoadGeometry(true, 0.0)) == 0) return false;

  if (OpenSimulationFile() == false) return false;

  if (CreateMatrices() == false) return false;

  // Go ahead event by event and compare the results: 
  MRERawEvent* RE = 0;
  MRawEventList* REList = 0;
  double Etot = 0;
  double Eres = 0;

  int Counter = 0;
  do {  
    if (InitializeNextMatchingEvent() == false) break;

    RE = 0;
    REList = m_ReReader->GetRawEventList();

    //g_DebugLevel = 1;


    int r_max = REList->GetNRawEvents();
    for (int r = 0; r < r_max; ++r) {
      RE = REList->GetRawEventAt(r);
      
      if (RE->GetNRESEs() <= 1) {
        mdebug<<"GeneratePdf: Not enough hits!"<<endl;
        continue;
      }

      mdebug<<RE->ToString()<<endl;

      if (RE->GetStartPoint() == 0) continue;

      // Check if complete sequence is ok:
      bool SequenceOk = true;
      int SequenceLength = RE->GetNRESEs();
      int PreviousPosition = 0;

      if (SequenceLength > m_CSRMaxLength) continue;

      if (SequenceLength == 2) {
        // Look at start:
        MRESEIterator Iter;
        Iter.Start(RE->GetStartPoint());
        Iter.GetNextRESE();
        Etot = RE->GetEnergy();
        Eres = RE->GetEnergyResolution();

        double CosPhiE = CalculateCosPhiE(*Iter.GetCurrent(), Etot);
        double PhotoDistance = CalculateAbsorptionProbabilityTotal(*Iter.GetCurrent(), *Iter.GetNext(), Iter.GetNext()->GetEnergy());

        if (IsComptonStart(*Iter.GetCurrent(), Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD dual Compton event!"<<endl;
          m_PdfDualGood.Add(Etot, CosPhiE, PhotoDistance, GetMaterial(*Iter.GetCurrent()));
        } else {
          mdebug<<"--------> Found bad dual Compton event!"<<endl;
          m_PdfDualBad.Add(Etot, CosPhiE, PhotoDistance, GetMaterial(*Iter.GetCurrent()));
          SequenceOk = false;
        }
      } else {
        // Look at start:
        MRESEIterator Iter;
        Iter.Start(RE->GetStartPoint());
        Iter.GetNextRESE();
        Etot = RE->GetEnergy();
        Eres = RE->GetEnergyResolution();
        if (IsComptonStart(*Iter.GetCurrent(), Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD Compton start!"<<endl;
          m_PdfStartGood.Add(Etot, CalculateCosPhiE(*Iter.GetCurrent(), Etot), SequenceLength, GetMaterial(*Iter.GetCurrent()));
        } else {
          mdebug<<"--------> Found bad Compton start!"<<endl;
          m_PdfStartBad.Add(Etot, CalculateCosPhiE(*Iter.GetCurrent(), Etot), SequenceLength, GetMaterial(*Iter.GetCurrent()));
          SequenceOk = false;
        }

        // Track at start?
        if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
          double dAlpha = CalculateDCosAlpha(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          double Alpha = CalculateCosAlphaG(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
            mdebug<<"--------> Found GOOD Track start!"<<endl;
            m_PdfTrackGood.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
          } else {
            mdebug<<"--------> Found bad Track start!"<<endl;
            m_PdfTrackBad.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
            SequenceOk = false;
          }
        }


        // Central part of the Compton track
        Iter.GetNextRESE();
        while (Iter.GetNext() != 0) {
          // Add here
          Etot -= Iter.GetPrevious()->GetEnergy();
          Eres = sqrt(Eres*Eres - Iter.GetPrevious()->GetEnergyResolution()*Iter.GetPrevious()->GetEnergyResolution());
          PreviousPosition++;
          
          // In the current implementation/simulation the hits have to be in increasing order...
          if (m_DoAbsorptions == true && SequenceLength <= m_MaxAbsorptions) {
            double ComptonDistance = 
              CalculateAbsorptionProbabilityCompton(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot);
            mdebug<<"Dist C: "<<ComptonDistance<<": real:"<<(Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Mag()<<endl;
            if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), PreviousPosition, Etot, Eres) == true) {
              mdebug<<"--------> Found GOOD Distance sequence!"<<endl;
              // Retrieve the data:
              m_PdfComptonScatterProbabilityGood.Add(ComptonDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
            } else {
              mdebug<<"--------> Found bad Distance sequence!"<<endl;
              // Retrieve the data:
              m_PdfComptonScatterProbabilityBad.Add(ComptonDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
              SequenceOk = false;
            } // Add good / bad
          }
        
          // Decide if it is good or bad...
          // In the current implementation/simulation the hits have to be in increasing order...
          double dPhi = CalculateDCosPhi(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), Etot);
          double PhiE = CalculateCosPhiE(*Iter.GetCurrent(), Etot);
          double Lever = CalculateMinLeverArm(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext());
          int Material = GetMaterial(*Iter.GetCurrent());
          
          if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
            mdebug<<"--------> Found GOOD Compton sequence!"<<endl;
            // Retrieve the data:
            m_PdfComptonGood.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);
            
          } else {
            mdebug<<"--------> Found bad Compton sequence!"<<endl;
            // Retrieve the data:
            m_PdfComptonBad.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);
            SequenceOk = false;

          } // Add good / bad

          // Track at central?
          if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
            //MRETrack* T = (MRETrack*) Iter.GetCurrent();
            double dAlpha = CalculateDCosAlpha(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
            double Alpha = CalculateCosAlphaG(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
            if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), PreviousPosition, Etot, Eres) == true) {
              mdebug<<"--------> Found GOOD Track start (central)!"<<endl;
              m_PdfTrackGood.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
            } else {
              mdebug<<"--------> Found bad Track start (central)!"<<endl;
              m_PdfTrackBad.Add(dAlpha, Alpha, 1, Iter.GetCurrent()->GetEnergy(), SequenceLength, GetMaterial(*Iter.GetCurrent()));
              SequenceOk = false;
            }
          }
          Iter.GetNextRESE();
        }
        
        Etot -= Iter.GetPrevious()->GetEnergy();
        Eres = sqrt(Eres*Eres - Iter.GetPrevious()->GetEnergyResolution()*Iter.GetPrevious()->GetEnergyResolution());
        PreviousPosition++;
        
        // Decide if it is good or bad...
        // In the current implementation/simulation the hits have to be in increasing order...
        if (m_DoAbsorptions == true && SequenceLength <= m_MaxAbsorptions) {
          double LastDistance = CalculateAbsorptionProbabilityPhoto(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot);
          mdebug<<"Dist P: "<<LastDistance<<": real:"<<(Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Mag()<<endl;
          if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), PreviousPosition, Etot, Eres) == true) {
            mdebug<<"--------> Found GOOD Lastdistance sequence!"<<endl;
            // Retrieve the data:
            m_PdfPhotoAbsorptionProbabilityGood.Add(CalculateAbsorptionProbabilityPhoto(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot), 
                                                    Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
          } else {
            mdebug<<"--------> Found bad Lastdistance sequence!"<<endl;
            // Retrieve the data:
            m_PdfPhotoAbsorptionProbabilityBad.Add(CalculateAbsorptionProbabilityPhoto(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot), 
                                                   Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
            SequenceOk = false;
          } // Add good / bad
        }
      }

      if (SequenceOk == false) {
        m_GoodBadTable.Add(0.5, SequenceLength, 1);
        //mdebug<<"No good sequence exists"<<endl<<endl<<endl<<endl;
      } else {
        m_GoodBadTable.Add(1.5, SequenceLength, 1);
        mdebug<<"One good sequence exists"<<endl<<endl<<endl<<endl;
      }
    } // For each raw event...

    if (++Counter % m_SaveAfter == 0) {
      SaveMatrices();
    }

    //g_DebugLevel = 0;
  } while (true);

  SaveMatrices();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonTrack(MRESE& Start, MRESE& Center, 
                                       int PreviousPosition, double Etot, double Eres)
{
  // A good start point of the track consists of the following:
  // (1) Start is a track
  // (2) Start is only one Compton interaction
  // (3) One of Centers Compton interactions directly follows the IA of start
  // (4) The first hit of start is the first hit of the track
  //     The second hit of start is the second hit of the track

  mdebug<<"IsComptonTrack: Looking at: "<<Start.GetID()<<"("<<Etot<<")"<<endl;

  // (1) 
  if (Start.GetType() != MRESE::c_Track) {
    mdebug<<"IsComptonTrack: No track!"<<endl;
    return false;    
  }

  // (2)
  if (IsSingleCompton(Start) == false) {
    mdebug<<"IsComptonTrack: Start has multiple Comptons!"<<endl;
    return false;        
  }

  // (3)
  if (IsComptonSequence(Start, Center, PreviousPosition, Etot-Start.GetEnergy(), Eres) == false) {
    // Attention: Eres is a little to large...
    mdebug<<"IsComptonTrack: No Compton sequence!"<<endl;
    return false;        
  }

  // (4)
  MRESEIterator Iter;
  MRETrack* Track = (MRETrack*) &Start;
  Iter.Start(Track->GetStartPoint());
  Iter.GetNextRESE();
  if (IsTrackStart(*Iter.GetCurrent(), *Iter.GetNext(), Track->GetEnergy()) == false) {
    mdebug<<"IsComptonTrack: Track is wrong!"<<endl;
    return false;    
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackStart(MRESE& Start, MRESE& Central, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA
  // (3) The start condition is fullfilled, when the start hit has the lowest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  if (StartIds.size() == 0) return false;
  if (CentralIds.size() == 0) return false;


  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   Is track start: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"Is track Start: Central IDs not in sequence"<<endl;
    return false;
  }

  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   Is track start: Hit between first and central according to time: "
            <<h<<endl;
      mdebug<<"LF: "<<TimeLastFirst<<" FL: "<<TimeFirstLast<<endl;
      return false;      
    }
  } 


//   if (abs(CentralIds[0] - StartIds[StartIds.size()-1]) != 1 && 
//       abs(CentralIds[CentralIds.size()-1] - StartIds[0]) != 1) {
//     mdebug<<"   Is track start: Distance not equal 1: "
//           <<abs(CentralIds[0] - StartIds[StartIds.size()-1])<<", "
//           <<abs(CentralIds[CentralIds.size()-1] - StartIds[0])<<endl;
//     return false;
//   }
  
  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   Is track start: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = numeric_limits<double>::max();
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime() < Timing) {
      Timing = m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() < Timing) {
        mdebug<<"   Is track start: Not first hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (Energy > 0) {
    if (IsTrackCompletelyAbsorbed(StartIds, Energy) == false) {
      mdebug<<"   Is track start: Not completely absorbed"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackStop(MRESE& Central, 
                                           MRESE& Stop, double Energy)
{
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)
  // (3) The start condition is fullfilled, when the start hit has the highest 
  //     time-tag of all hits with this origin

  const int IdOffset = 2;

  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsStop: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsStop: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsStop: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
//   if (abs(StopIds[0] - CentralIds[CentralIds.size()-1]) != 1 && 
//       abs(StopIds[StopIds.size()-1] - CentralIds[0]) != 1) {
//     mdebug<<"   IsStop: Distance not equal 1"<<endl;
//     return false;
//   }


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < CentralIds.size(); ++i) {
    for (unsigned int j = 0; j < StopIds.size(); ++j) {
      for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetNOrigins(); ++oi) {
        for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetNOrigins(); ++oj) {
          if (m_SiEvent->GetHTAt(CentralIds[i]-IdOffset)->GetOriginAt(oi) == 
              m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj)) {
            CommonOrigin = m_SiEvent->GetHTAt(StopIds[j]-IdOffset)->GetOriginAt(oj);
            break;
          }
        }
      }
    }
  }

  if (CommonOrigin == -1) {
    mdebug<<"   IsStop: No common origin!"<<endl;
    return false;
  }

  // Test (3)
  double Timing = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < StopIds.size(); ++i) {
    if (m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime() > Timing) {
      Timing = m_SiEvent->GetHTAt(StopIds[i]-IdOffset)->GetTime();
    }
  }
  unsigned int i_max = m_SiEvent->GetNHTs();
  for (unsigned int i = 0; i < i_max; ++i) {
    if (m_SiEvent->GetHTAt(i)->IsOrigin(CommonOrigin)) {
      if (m_SiEvent->GetHTAt(i)->GetTime() > Timing) {
        mdebug<<"   IsStop: Not last hit (timing)"<<endl;
        return false;
      }
    }
  }

  if (IsTrackCompletelyAbsorbed(StopIds, Energy) == false) {
    mdebug<<"   Is track stop: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::AreReseInSequence(MRESE& Start, 
                                                 MRESE& Central, 
                                                 MRESE& Stop, 
                                                 double Energy)
{
  // Return true if the given RESEs are in sequence
  //
  // According to the current simulation (GMega), the RESEs are in sequence if
  // (1) the IDs of their hits are in increasing order without holes
  // (2) all originate from the same IA (espec. the first and last hit in the 
  //     sequence)

  const int IdOffset = 2;

  vector<int> StartIds = GetReseIds(&Start);
  vector<int> CentralIds = GetReseIds(&Central);
  vector<int> StopIds = GetReseIds(&Stop);
  
  // Test (1)
  if (AreIdsInSequence(StartIds) == false) {
    mdebug<<"   IsCentral: Start IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(CentralIds) == false) {
    mdebug<<"   IsCentral: Central IDs not in sequence"<<endl;
    return false;
  }
  if (AreIdsInSequence(StopIds) == false) {
    mdebug<<"   IsCentral: Stop IDs not in sequence"<<endl;
    return false;
  }
  // Test for holes:
  // The hit after the last one from start is the first from central:
  double TimeLastFirst = m_SiEvent->GetHTAt(StartIds.back()-IdOffset)->GetTime();
  double TimeFirstLast = m_SiEvent->GetHTAt(CentralIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 
  TimeLastFirst = m_SiEvent->GetHTAt(CentralIds.back()-IdOffset)->GetTime();
  TimeFirstLast = m_SiEvent->GetHTAt(StopIds.front()-IdOffset)->GetTime();
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->GetTime() > TimeLastFirst && 
        m_SiEvent->GetHTAt(h)->GetTime() < TimeFirstLast) {
      mdebug<<"   IsCentral: Hit between first and central according to time!"<<endl;
      return false;      
    }
  } 


  // Test (2)
  int CommonOrigin = -1;
  for (unsigned int i = 0; i < StartIds.size(); ++i) {
    for (unsigned int j = 0; j < CentralIds.size(); ++j) {
      for (unsigned int k = 0; k < StopIds.size(); ++k) {
        for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetNOrigins(); ++oi) {
          for (unsigned int oj = 0; oj < m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetNOrigins(); ++oj) {
            for (unsigned int ok = 0; ok < m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetNOrigins(); ++ok) {
              if (m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj) && 
                  m_SiEvent->GetHTAt(StartIds[i]-IdOffset)->GetOriginAt(oi) == 
                  m_SiEvent->GetHTAt(StopIds[k]-IdOffset)->GetOriginAt(ok)) {
                CommonOrigin = m_SiEvent->GetHTAt(CentralIds[j]-IdOffset)->GetOriginAt(oj);
                break;
              }
            }
          }
        }
      }
    }
  }
  if (CommonOrigin == -1) {
    mdebug<<"   IsCentral: No common origin!"<<endl;
    return false;
  }

  // (3) Timing:
  if (m_SiEvent->GetHTAt(StartIds[0]-IdOffset)->GetTime() > 
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() ||
      m_SiEvent->GetHTAt(CentralIds[0]-IdOffset)->GetTime() >
      m_SiEvent->GetHTAt(StopIds[0]-IdOffset)->GetTime()) {
    mdebug<<"   IsCentral: Timing wrong"<<endl;
    return false;
  }


  if (IsTrackCompletelyAbsorbed(CentralIds, Energy) == false) {
    mdebug<<"   IsCentral: Not completely absorbed"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonStart(MRESE& Start, double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) We have one Compton interaction and some other interaction
  // (4) The origin is only the first Compton interaction

  mdebug<<"IsComptonStart: Looking at: "<<Start.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonStart: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (Etot > 0) {
    if (IsAbsorbed(StartOriginIds, Start.GetEnergy(), Start.GetEnergyResolution()) == false) {
      mdebug<<"IsComptonStart: Central not completely absorbed!"<<endl;
      return false;
    }
  }
  
  //cout<<"Cont only Start!"<<endl;
  if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
    mdebug<<"IsComptonStart: Start contains not only Compton dependants"<<endl;
    return false;
  }

  // Test (3)
  if (m_SiEvent->GetNIAs() < 3) { 
    mdebug<<"IsComptonStart: Not enough interactions!"<<endl;
    return false;    
  }
  if (m_SiEvent->GetIAAt(1)->GetProcess() != "COMP") {
    mdebug<<"IsComptonStart: Second interaction is no Compton!"<<endl;
    return false;    
  }
  
  // Test (4)
  int SmallestSimId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StartOriginIds[i] != 1) {
      if (SmallestSimId > StartOriginIds[i]) {
        SmallestSimId = StartOriginIds[i];
      }
    }
  }
  
  if (SmallestSimId != 2) {
    mdebug<<"IsComptonStart: Not correct start point: "<<SmallestSimId<<endl;
    return false;            
  }

  if (Etot > 0) {
    if (IsTotalAbsorbed(StartOriginIds, Etot, Eres) == false) {
      mdebug<<"Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          int StartPosition, double Etot, 
                                          double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for double Comptons requires:
  // (1) An absorption better than 99%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) Start is the StartPosition's Compton IA

  mdebug<<"IsComptonSequence2: Looking at: "
        <<Start.GetID()<<" - "<<Central.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence2: Central has no Sim IDs!"<<endl;
    return false;
  }
  

  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence2: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence2: Not in Compton sequence!"<<endl;
    return false;
  }

  if (Etot > 0) {
    if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
      mdebug<<"IsComptonSequence2: Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonSequence(MRESE& Start, MRESE& Central, 
                                          MRESE& Stop, int StartPosition, 
                                          double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence

  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes

  mdebug<<"IsComptonSequence3: Looking at: "
        <<Start.GetID()<<" - "<<Central.GetID()<<" - "<<Stop.GetID()<<endl;


  // Test (2)
  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Start has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StartOriginIds, Start.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Start not completely absorbed!"<<endl;
//     return false;
//   }
  
  //vector<int> CentralIds = GetReseIds(Central);
  vector<int> CentralOriginIds = GetOriginIds(&Central);
  if (CentralOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Central has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsAbsorbed(CentralOriginIds, Central.GetEnergy(), Central.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonSequence3: Central not completely absorbed!"<<endl;
    return false;
  }
  
  //vector<int> StopIds = GetReseIds(Stop);
  vector<int> StopOriginIds = GetOriginIds(&Stop);
  if (StopOriginIds.size() == 0) {
    mdebug<<"IsComptonSequence3: Stop has no Sim IDs!"<<endl;
    return false;
  }
  
//   // Absorption:
//   if (IsAbsorbed(StopOriginIds, Stop.GetEnergy()) == false) {
//     mdebug<<"IsComptonSequence3: Stop not completely absorbed!"<<endl;
//     return false;
//   }
  
  //cout<<"Cont only Start!"<<endl;
//   if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
//     mdebug<<"IsComptonSequence3: Start contains not only Compton dependants"<<endl;
//     return false;
//   }
  //cout<<"Cont only Central!"<<endl;
  if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Central contains not only Compton dependants"<<endl;
    return false;
  }
  if (ContainsOnlyComptonDependants(StopOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Stop contains not only Compton dependants"<<endl;
    return false;
  }
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds, StartPosition) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }
  if (AreInComptonSequence(CentralOriginIds, StopOriginIds, 0) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }

  if (IsTotalAbsorbed(CentralOriginIds, Etot, Eres) == false) {
    mdebug<<"IsComptonSequence3: Not completely absorbed!"<<endl;
    return false;
  }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StopOriginIds[0] - StartOriginIds[i] == 2) {
      FoundBehind = true;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"IsComptonSequence3: More than one Compton between start and stop!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsComptonEnd(MRESE& End)
{
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 98%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) We have one Compton interaction and some other interaction
  // (4) The origin is only the first Compton interaction

  mdebug<<"IsComptonEnd: Looking at: "<<End.GetID()<<endl;



  // Test (2)
//   vector<int> CentralIds = GetReseIds(Central);
//   vector<int> CentralOriginIds = GetOriginIds(CentralIds);
//   if (CentralOriginIds.size() == 0) {
//     mdebug<<"IsComptonEnd: Central has no Sim IDs!"<<endl;
//     return false;
//   }
  
  //vector<int> EndIds = GetReseIds(End);
  vector<int> EndOriginIds = GetOriginIds(&End);
  if (EndOriginIds.size() == 0) {
    mdebug<<"IsComptonEnd: End has no Sim IDs!"<<endl;
    return false;
  }
  
  // Absorption:
  if (IsTotalAbsorbed(EndOriginIds, End.GetEnergy(), End.GetEnergyResolution()) == false) {
    mdebug<<"IsComptonEnd: Not completely absorbed!"<<endl;
    return false;
  }

//   if (ContainsOnlyComptonDependants(CentralOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Central contains not only Compton dependants"<<endl;
//     return false;
//   }
  if (ContainsOnlyComptonDependants(EndOriginIds) == false) {
    mdebug<<"IsComptonEnd: End contains not only Compton dependants"<<endl;
    return false;
  }
//   if (AreInComptonSequence(CentralOriginIds, EndOriginIds) == false) {
//     mdebug<<"IsComptonEnd: Not in Compton sequence!"<<endl;
//     return false;
//   }
  
  // Test (3)
  if (m_SiEvent->GetNIAs() < 3) { 
    mdebug<<"IsComptonEnd: Not enough interactions!"<<endl;
    return false;    
  }
  
//   // Test (4)
//   // End needs to contain the last interaction of track 1 
//   int LastIA = 0;
//   for (unsigned int i = 1; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOriginID() == 1) {
//       LastIA = i+1;
//     } else {     
//       break;
//     }
//   }
//   bool FoundLastIA = false;
//   for (unsigned int i = 0; i < EndOriginIds.size(); ++i) {
//     if (EndOriginIds[i] == LastIA) {
//       FoundLastIA = true;
//       break;
//     }
//   }
//   if (FoundLastIA == false) {
//     mdebug<<"IsComptonEnd: Last interaction of track 1 not part of last hit"<<endl;
//     return false;    
//   }

  // Motherparticle of smallest ID needs to be a photon:
  int Smallest = EndOriginIds[0];
  for (unsigned int i = 1; i < EndOriginIds.size(); ++i) {
    if (EndOriginIds[i] < Smallest) Smallest = EndOriginIds[i];
  }

  if (m_SiEvent->GetIAAt(Smallest-1)->GetOriginID() == 0) {
    if (m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "ANNI" &&
        m_SiEvent->GetIAAt(Smallest-1)->GetProcess() != "INIT") {
      return false;
    }
  } else {
    if (m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "ANNI" &&
        m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(Smallest-1)->GetOriginID()-1)->GetProcess() != "INIT") {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsSingleCompton(MRESE& Start)
{
  // Check if all interaction in start are from one single Compton interaction:

  //cout<<Start.ToString()<<endl;

  //vector<int> StartIds = GetReseIds(Start);
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    mdebug<<"IsSingleCompton: Start has no Sim IDs!"<<endl;
    return false;
  }

  int NPhotos = 0;
  int NComptons = 0;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"Id:"<<StartOriginIds[i]<<endl;
    if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetProcess() == "COMP") {
      NComptons++;
    } else if (m_SiEvent->GetIAAt(StartOriginIds[i]-1)->GetProcess() == "PHOT") {
      NPhotos++;
    }
  }

  if (NComptons == 1 && NPhotos >= 0) {
    mdebug<<"IsSingleCompton: Single Compton: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else if (NComptons == 0 && NPhotos >= 1) {
    mdebug<<"IsSingleCompton: No Compton but photo: C="<<NComptons<<" P="<<NPhotos<<endl;
    return true;
  } else {
    mdebug<<"IsSingleCompton: No single Compton or photo: C="<<NComptons<<" P="<<NPhotos<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::AreInComptonSequence(const vector<int>& StartOriginIds, 
                                                    const vector<int>& CentralOriginIds,
                                                    int StartPosition)
{
  // They are in good sequence if 
  //  (1) Both smallest IDs originate from 1
  //  (2) The IDs are in sequence

  massert(StartOriginIds.size() > 0);
  massert(CentralOriginIds.size() > 0);
  
  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetProcess() != "COMP") {
    mdebug<<"CS: No COMP"<<endl;
    return false;
  }

  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetOriginID() != 
      m_SiEvent->GetIAAt(CentralOriginIds[0]-1)->GetOriginID()) {
    mdebug<<"CS: No common origin"<<endl;
    return false;
  }

//   if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber() != 1) {
//     mdebug<<"CS: Mother is no photon: "<<m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber()<<endl;
//     return false;
//   }

  bool FoundBehind = false;
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    //cout<<"testing: "<<CentralOriginIds[0]<<"-"<<StartOriginIds[i]<<endl;
    if (CentralOriginIds[0] - StartOriginIds[i] == 1) {
      FoundBehind = true;
      break;
    }
  }
  if (FoundBehind == false) {
    mdebug<<"CS: Not behind each other!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::ContainsOnlyComptonDependants(vector<int> AllSimIds)
{
  massert(AllSimIds.size() > 0);

  int SmallestSimId = numeric_limits<int>::max();
  vector<int> GoodSimIds;

  // Generate sim IDs:
  //cout<<"All: ";
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    //cout<<AllSimIds[i]<<" ";
    if (SmallestSimId > AllSimIds[i]) {
      SmallestSimId = AllSimIds[i];
    }
  }
  //cout<<endl;

  vector<int>::iterator Iter;
  GoodSimIds.push_back(SmallestSimId);
  //cout<<"Adding smallest: "<<SmallestSimId<<endl;
  Iter = find(AllSimIds.begin(), AllSimIds.end(), SmallestSimId);
  AllSimIds.erase(Iter);

  bool MoreSmallest = false;
  do {
    MoreSmallest = false;
    
    for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
      if (AllSimIds[i] == SmallestSimId+1 && 
          m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetOriginID() == 
          m_SiEvent->GetIAAt(SmallestSimId-1)->GetOriginID()) {
        SmallestSimId = AllSimIds[i];
        MoreSmallest = true;
        GoodSimIds.push_back(SmallestSimId);
        //cout<<"Adding smallest: "<<SmallestSimId<<endl;
        Iter = find(AllSimIds.begin(), AllSimIds.end(), SmallestSimId);
        AllSimIds.erase(Iter);
        break;
      }
    }
    
  } while (MoreSmallest == true);


  // Check for dependents:
  bool DependantsFound = false;
  do {
    DependantsFound = false;
    for (unsigned int g = 0; g < GoodSimIds.size(); ++g) {
      for (unsigned int a = 0; a < AllSimIds.size(); ++a) {
        //cout<<"Testing all: "<<AllSimIds[a]<<endl;
        if (m_SiEvent->GetIAAt(AllSimIds[a]-1)->GetOriginID() == GoodSimIds[g]) {
          //cout<<"Found good: "<<AllSimIds[a]<<endl;
          GoodSimIds.push_back(AllSimIds[a]);
          Iter = find(AllSimIds.begin(), AllSimIds.end(), AllSimIds[a]);
          AllSimIds.erase(Iter);
          DependantsFound = true;
          break;
        }
      }
    }
  } while (DependantsFound == true);

  // If we have origins other than dependants:
  if (AllSimIds.size() > 0) {
    mdebug<<"ContainsOnlyComptonDependants: Hits other than dependants: ";
    for (unsigned int a = 0; a < AllSimIds.size(); ++a) {
      mdebug<<AllSimIds[a]<<" ";
    }
    mdebug<<endl;
    return false;
  }

  sort(GoodSimIds.begin(), GoodSimIds.end());

  // Check for dependants which are not contained: 
  for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
    int Origin = m_SiEvent->GetIAAt(i)->GetOriginID();
    //cout<<"Checkin ID="<<i+1<<" with o= "<<Origin<<endl;
    bool OriginatesFromGood = false;
    bool ContainedInGood = false;   
    for (unsigned int g = 0; g < GoodSimIds.size(); ++g) {
      //cout<<"Verifying: "<<GoodSimIds[g]<<endl;
      if (int(i+1) == GoodSimIds[g]) {
        //cout<<"Contained in good!"<<endl;
        ContainedInGood = true;
      } 
      if (Origin == GoodSimIds[g]) {
        //cout<<"Originates from good!"<<endl;
        OriginatesFromGood = true;
      }
    }
    if (OriginatesFromGood == true && ContainedInGood == false) {
      mdebug<<"ContainsOnlyComptonDependants: Originates but not contained: "<<i+1<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsAbsorbed(const vector<int>& AllSimIds, 
                                          double Energy, double EnergyResolution)
{
  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  int MinId = numeric_limits<int>::max();
  //cout<<"Ids: ";
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    //cout<<AllSimIds[i]<<" ";
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }
  //cout<<endl;

  if (MinId == numeric_limits<int>::max()) {
    return false;
  }

  double Ideal;

  MSimIA* Top = 0;
  MSimIA* Bottom = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOriginID() == m_SiEvent->GetIAAt(MinId-1)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOriginID()-1);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
  } else {
    Ideal = Top->GetSecondaryEnergy()-Bottom->GetMotherEnergy();
  }

//   if (fabs(Energy - Ideal)/Ideal > m_MaxEnergyDifferencePercent &&
//       fabs(Energy - Ideal) > m_MaxEnergyDifference) {
//     mdebug<<"IsAbsorbed: Not completely absorbed: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   } else {
//     mdebug<<"Abs!: i="<<Ideal<<" r="<<Energy<<endl;
//   }
  if (fabs(Energy - Ideal) > EnergyResolution) {
    mdebug<<"IsAbsorbed: Not completely absorbed: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"Abs!: i="<<Ideal<<" r="<<Energy<<endl;
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTotalAbsorbed(const vector<int>& AllSimIds, 
                                               double Energy, double EnergyResolution)
{
  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  int MinId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }

  if (MinId == numeric_limits<int>::max()) return false;

  double Ideal;
  MSimIA* Top = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOriginID() == m_SiEvent->GetIAAt(MinId-1)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOriginID()-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy();
  } else {
    Ideal = Top->GetSecondaryEnergy();
  }

  if (MinId-2 != 0) {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetMotherEnergy();
  } else {
    Ideal = m_SiEvent->GetIAAt(MinId-2)->GetSecondaryEnergy();
  }

  if (fabs(Ideal - Energy) > EnergyResolution) {
      mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"Is totally absorbed: Completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
  }
//   if (fabs((Ideal - Energy)/Ideal) > m_MaxEnergyDifferencePercent &&
//       fabs(Ideal - Energy) > m_MaxEnergyDifference ) {
//       mdebug<<"Is totally absorbed: Not completely absorbed: Tot abs: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::GetIdealDepositedEnergy(int MinId)
{
  //  

  double Ideal = 0;

  MSimIA* Top = 0;
  MSimIA* Bottom = 0;
  if (m_SiEvent->GetIAAt(MinId-2)->GetOriginID() == m_SiEvent->GetIAAt(MinId-1)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MinId-2);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  } else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(MinId-1)->GetOriginID()-1);
    Bottom = m_SiEvent->GetIAAt(MinId-1);
  }

  if (m_SiEvent->GetIAAt(MinId-2)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy()-Bottom->GetMotherEnergy();
  } else {
    Ideal = Top->GetSecondaryEnergy()-Bottom->GetMotherEnergy();
  }

  return Ideal;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleCompton::IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy)
{
  // Return true if the track is completely absorbed
  //
  // Prerequisites: Ids have only one common origin!
  // Realization: Nothing originates from this ID AFTER this hit:

  const int IdOffset = 2;

  // Get origin Id
  int Origin = -1;
  for (unsigned int i = 0; i < Ids.size(); ++i) {
    for (unsigned int oi = 0; oi < m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetNOrigins(); ++oi) {
      if (m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi) != 1) { 
        Origin = m_SiEvent->GetHTAt(Ids[i]-IdOffset)->GetOriginAt(oi);
        break;
      }
    }
  } 
  if (Origin <= 1) {
    mdebug<<"   IsTrackCompletelyAbsorbed: No origin"<<endl;
    return false;
  }

//   // Check if we do haven an IA originating from this - which is not part of this track:
//   for (unsigned int i = 0; i < m_SiEvent->GetNIAs(); ++i) {
//     if (m_SiEvent->GetIAAt(i)->GetOriginID() == Origin) {
//       if (m_SiEvent->GetIAAt(i)->GetProcess() == "BREM") {
//         // Search the closest hit:
//         int MinHT = -1;
//         double MinDist = numeric_limits<double>::max();
//         for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
//           if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
//             if ((m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag() < MinDist) {
//               MinDist = (m_SiEvent->GetIAAt(i)->GetPosition() - m_SiEvent->GetHTAt(h)->GetPosition()).Mag();
//               MinHT = h;
//             }
//           }
//         }
//         if (MinHT > 0) {
//           if (m_SiEvent->GetHTAt(MinHT)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
//             mdebug<<"   IsTrackCompletelyAbsorbed: Bremsstrahlung emitted after this hit!"<<endl;
//             return false;
//           }
//         }
//       } else {
//         mdebug<<"   IsTrackCompletelyAbsorbed: Originating interaction: "<<m_SiEvent->GetIAAt(i)->GetProcess()<<endl;
//         return false;
//       }
//     }
//   }

  // Check for complete absorption:
  const double EnergyLimitPercent = 0.15; // %
  const double EnergyLimit = 20; //keV

  double RealEnergy = 0;
  for (unsigned int h = 0; h < m_SiEvent->GetNHTs(); ++h) {
    if (m_SiEvent->GetHTAt(h)->IsOrigin(Origin) == true) {
      if (m_SiEvent->GetHTAt(h)->GetTime() >= m_SiEvent->GetHTAt(Ids[0]-IdOffset)->GetTime()) {
        RealEnergy += m_SiEvent->GetHTAt(h)->GetEnergy();
      }
    }
  }
  
  if (fabs(Energy - RealEnergy) > EnergyLimit &&
      fabs(Energy - RealEnergy)/RealEnergy > EnergyLimitPercent) {
    mdebug<<"   IsTrackCompletelyAbsorbed: Missing/Not much energy: i="
          <<RealEnergy<<" r="<<Energy<<endl;
    return false;   
  }


  return  true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateMinLeverArm(MRESE& Start, MRESE& Central, 
                                               MRESE& Stop)
{
  return MERCSRBayesian::CalculateMinLeverArm(Start.GetPosition(),
                                              Central.GetPosition(),
                                              Stop.GetPosition());
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDPhiInDegree(MRESE& Start, MRESE& Central, 
                                        MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDPhiInDegree(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDCosPhi(MRESE& Start, MRESE& Central, 
                                                  MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDCosPhi(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDCosAlpha(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDCosAlpha(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateDAlphaInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDAlphaInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosAlphaE(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaE(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAlphaEInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaEInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosAlphaG(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaG(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAlphaGInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaGInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityPhoto(MRESE& Start, 
                                                                     MRESE& Stop, 
                                                                     double Etot)
{
  double Probability = 
    m_ReGeometry->GetPhotoAbsorptionProbability(Start.GetPosition(), 
                                                Stop.GetPosition(), Etot); 

  //cout<<"Probability: "<<Probability<<" E: "<<Etot<<endl;
  //cout<<Start.GetPosition()<<":"<<Stop.GetPosition()<<":"<<(Stop.GetPosition()-Start.GetPosition()).Mag()<<endl;
  return Probability;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityTotal(MRESE& Start, 
                                                                     MRESE& Stop, 
                                                                     double Etot)
{
  double Probability = 
    m_ReGeometry->GetAbsorptionProbability(Start.GetPosition(), 
                                           Stop.GetPosition(), Etot); 

  //cout<<"Probability: "<<Probability<<" E: "<<Etot<<endl;
  //cout<<Start.GetPosition()<<":"<<Stop.GetPosition()<<":"<<(Stop.GetPosition()-Start.GetPosition()).Mag()<<endl;
  return Probability;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateAbsorptionProbabilityCompton(MRESE& Start, 
                                                                       MRESE& Stop, 
                                                                       double Etot)
{
  double Distance = 
    m_ReGeometry->GetComptonAbsorptionProbability(Start.GetPosition(), 
                                                  Stop.GetPosition(), Etot); 

  //cout<<"Distance: "<<Distance<<" E: "<<Etot<<endl;
  //cout<<Start.ToString()<<endl;
  //cout<<Stop.ToString()<<endl;
  return Distance;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculatePhiEInDegree(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculatePhiEInDegree(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculatePhiGInDegree(MRESE& Start, MRESE& Central, 
                                                       MRESE& Stop)
{
  return MERCSRBayesian::CalculatePhiGInDegree(&Start, &Central, &Stop);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleCompton::CalculateCosPhiE(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculateCosPhiE(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMultipleCompton::GetMaterial(MRESE& Hit)
{
  return MERCSRBayesian::GetMaterial(&Hit);
}


// MResponseMultipleCompton.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
