/*
 * MResponseMultipleComptonBayes.cxx
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
// MResponseMultipleComptonBayes
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseMultipleComptonBayes.h"

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


#ifdef ___CLING___
ClassImp(MResponseMultipleComptonBayes)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseMultipleComptonBayes::MResponseMultipleComptonBayes()
{  
  m_ResponseNameSuffix = "mc";
  
  m_DoAbsorptions = true;
  m_MaxAbsorptions = 5;
  m_MaxNInteractions = 7;

  m_MaxEnergyDifference = 5; // keV
  m_MaxEnergyDifferencePercent = 0.02;

  m_MaxTrackEnergyDifference = 30; // keV
  m_MaxTrackEnergyDifferencePercent = 0.1;

  m_EnergyMinimum = 100; // keV
  m_EnergyMaximum = 10000; // keV

  // If you change it here, also change it in MResponseMultiComptonBayes
  m_MaxCosineLimit = 2;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseMultipleComptonBayes::~MResponseMultipleComptonBayes()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseMultipleComptonBayes::Description()
{
  return MString("Compton event reconstruction (Bayes)");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseMultipleComptonBayes::Options()
{
  ostringstream out;
  out<<"             maxia:                 the maximum number of interactions (default: 7)"<<endl;
  out<<"             emin:                  minimum energy (default: 100 keV)"<<endl;
  out<<"             emax:                  maximum energy (default: 10000 keV)"<<endl;

  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseMultipleComptonBayes::ParseOptions(const MString& Options)
{
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("="));
  }

  // Basic sanity check and to lower for all options
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i].size() == 0) {
      mout<<"Error: Empty option in string "<<Options<<endl;
      return false;
    }
    if (Split2[i].size() == 1) {
      mout<<"Error: Option has no value: "<<Split2[i][0]<<endl;
      return false;
    }
    if (Split2[i].size() > 2) {
      mout<<"Error: Option has more than one value or you used the wrong separator (not \":\"): "<<Split1[i]<<endl;
      return false;
    }
    Split2[i][0].ToLowerInPlace();
  }

  // Parse
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    string Value = Split2[i][1].Data();

    if (Split2[i][0] == "emin") {
      m_EnergyMinimum = stod(Value);
    } else if (Split2[i][0] == "emax") {
      m_EnergyMaximum = stod(Value);
    } else if (Split2[i][0] == "maxia") {
      m_MaxNInteractions = stoi(Value);
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }

  // Sanity checks:
  if (m_EnergyMinimum <= 0 || m_EnergyMaximum <= 0) {
    mout<<"Error: All energy values must be positive (larger than zero)"<<endl;
    return false;
  }
  if (m_EnergyMinimum >= m_EnergyMaximum) {
    mout<<"Error: The minimum energy must be smaller than the maximum energy"<<endl;
    return false;
  }
  if (m_MaxNInteractions < 2) {
    mout<<"Error: You need at least 2 interactions"<<endl;
    return false;
  }

  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for binned imaging response:"<<endl;
  mout<<"  Minimum energy:                                     "<<m_EnergyMinimum<<endl;
  mout<<"  Maximum energy:                                     "<<m_EnergyMaximum<<endl;
  mout<<"  Maximum number of interactions:                     "<<m_MaxNInteractions<<endl;
  mout<<endl;

  return true;
}


  
////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseMultipleComptonBayes::Initialize()
{ 
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;
  
  m_MaxNInteractions = m_RevanSettings.GetCSRMaxNHits();
  //m_MaxNInteractions = 3;
  m_ReReader->SetCSROnlyCreateSequences(true);

  if (m_ReReader->PreAnalysis() == false) return false;



  // Axis representing the sequence length:
  vector<float> AxisSequenceLength;
  for (unsigned int i = 2; i <= m_MaxNInteractions+1; ++i) {
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
  AxisComptonScatterAngleStart = CreateEquiDist(-1.5, 1.1, 26, -m_MaxCosineLimit, +m_MaxCosineLimit);
  MString NameAxisComptonScatterAngleStart = "cos#varphi";

  // Scatter probability axis
  // vector<float> AxisScatterProbability = CreateEquiDist(-0.001, 1.001, 42);
  vector<float> AxisScatterProbability = { -0.001, 0.001, 0.005, 0.01 };
  vector<float> Middle = CreateEquiDist(0.025, 0.975, 38);
  AxisScatterProbability.insert(AxisScatterProbability.end(), Middle.begin(), Middle.end());
  AxisScatterProbability.insert(AxisScatterProbability.end(),{ 0.99, 0.995, 0.999, 1.001 });
  MString NameAxisScatterProbabilityPhoto = "Scatter probability";
  MString NameAxisScatterProbabilityCompton = "Reach probability";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyDistances = CreateLogDist(15, m_EnergyMaximum, 38, 1, 20000);
  MString NameAxisTotalEnergyDistances = "Energy [keV]";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyStart = CreateLogDist(m_EnergyMinimum, m_EnergyMaximum, 38, 1, 20000);
  MString NameAxisTotalEnergyStart = "Energy [keV]";


  // Make sure it starts, well below 0 and exceeds (slightly) 1!
  vector<float> AxisComptonScatterAngleDual;
  AxisComptonScatterAngleDual = CreateEquiDist(-1.5, 1.1, 26, -m_MaxCosineLimit, +m_MaxCosineLimit);
  MString NameAxisComptonScatterAngleDual = "cos#varphi";

  // Total energy axis for scatter probabilities
  vector<float> AxisTotalEnergyDual = CreateLogDist(m_EnergyMinimum, m_EnergyMaximum, 18, 1, 20000);
  MString NameAxisTotalEnergyDual = "Energy [keV]";

  // Scatter probability axis
  vector<float> AxisScatterProbabilityDual = CreateEquiDist(-0.001, 0.99, 15);
  AxisScatterProbabilityDual.push_back(1.001);
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
                                     CreateEquiDist(-0.5, 1.5, 36, -m_MaxCosineLimit, m_MaxCosineLimit),
                                     CreateEquiDist(-0.5, 1.5, 1, -m_MaxCosineLimit, m_MaxCosineLimit),
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
                                    CreateEquiDist(-0.5, 1.5, 36, -m_MaxCosineLimit, m_MaxCosineLimit),
                                    CreateEquiDist(-0.5, 1.5, 1, -m_MaxCosineLimit, m_MaxCosineLimit),
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
  m_PdfComptonScatterProbabilityGood.SetAxisNames(NameAxisScatterProbabilityCompton,
                                 NameAxisTotalEnergyDistances, 
                                 NameAxisSequenceLength, 
                                 NameAxisMaterial);
  m_PdfComptonScatterProbabilityBad = MResponseMatrixO4("MC: Compton distance (bad)", 
                                       AxisScatterProbability, 
                                       AxisTotalEnergyDistances, 
                                       AxisSequenceLength, 
                                       AxisMaterial);
  m_PdfComptonScatterProbabilityBad.SetAxisNames(NameAxisScatterProbabilityCompton,
                                NameAxisTotalEnergyDistances, 
                                NameAxisSequenceLength, 
                                NameAxisMaterial);


  // Lastdistance:
  m_PdfPhotoAbsorptionProbabilityGood = MResponseMatrixO4("MC: Photo distance (good)", 
                                            AxisScatterProbability, 
                                            AxisTotalEnergyDistances, 
                                            AxisSequenceLength, 
                                            AxisMaterial);
  m_PdfPhotoAbsorptionProbabilityGood.SetAxisNames(NameAxisScatterProbabilityPhoto,
                                     NameAxisTotalEnergyDistances, 
                                     NameAxisSequenceLength, 
                                     NameAxisMaterial);
  m_PdfPhotoAbsorptionProbabilityBad = MResponseMatrixO4("MC: Photo distance (bad)", 
                                           AxisScatterProbability, 
                                           AxisTotalEnergyDistances, 
                                           AxisSequenceLength, 
                                           AxisMaterial);
  m_PdfPhotoAbsorptionProbabilityBad.SetAxisNames(NameAxisScatterProbabilityPhoto,
                                    NameAxisTotalEnergyDistances, 
                                    NameAxisSequenceLength, 
                                    NameAxisMaterial);


  // CentralCompton:
  
  // Assymetries would be best handled if -1 .. 1
  //vector<float> AxisDifferenceComptonScatterAngle = 
  //  CreateLogDist(1E-3, 2, 18, 0.0000001, m_MaxCosineLimit);
  vector<float> AxisDifferenceComptonScatterAngle; 
  vector<float> A = CreateLogDist(0.003, 1.8, 13, c_NoBound, m_MaxCosineLimit);
  for (unsigned int i = A.size()-1; i < A.size(); --i) {
    AxisDifferenceComptonScatterAngle.push_back(-A[i]);
  }
  for (unsigned int i = 0; i < A.size(); ++i) {
    AxisDifferenceComptonScatterAngle.push_back(A[i]);
  }
  MString NameAxisDifferenceComptonScatterAngle = "cos#varphi_{E} - cos#varphi_{G}";

  vector<float> AxisComptonScatterAngle;
  AxisComptonScatterAngle = CreateEquiDist(-1.4, 1.2, 13, -m_MaxCosineLimit, m_MaxCosineLimit);
  MString NameAxisComptonScatterAngle = NameAxisComptonScatterAngleStart;


  vector<float> AxisDistance = CreateLogDist(0.2, 10, 7, 0.01, 100, 0, false);
  MString NameAxisDistance = "Distance [cm]";

  vector<float> AxisTotalEnergy = CreateLogDist(100, m_EnergyMaximum, 4, 1, std::max(2*m_EnergyMaximum, 10000.0), 0, false);
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


//! Save the responses
bool MResponseMultipleComptonBayes::Save()
{
  MResponseBuilder::Save(); 

  m_GoodBadTable.Write(GetFilePrefix() + ".goodbad" + m_Suffix, true);
  
  m_PdfDualGood.Write(GetFilePrefix() + ".dual.good" + m_Suffix, true);
  m_PdfDualBad.Write(GetFilePrefix() + ".dual.bad" + m_Suffix, true);
  
  m_PdfStartGood.Write(GetFilePrefix() + ".start.good" + m_Suffix, true);
  m_PdfStartBad.Write(GetFilePrefix() + ".start.bad" + m_Suffix, true);
  
  m_PdfTrackGood.Write(GetFilePrefix() + ".track.good" + m_Suffix, true);
  m_PdfTrackBad.Write(GetFilePrefix() + ".track.bad" + m_Suffix, true);
  
  m_PdfComptonGood.Write(GetFilePrefix() + ".compton.good" + m_Suffix, true);
  m_PdfComptonBad.Write(GetFilePrefix() + ".compton.bad" + m_Suffix, true);
  
  m_PdfComptonScatterProbabilityGood.Write(GetFilePrefix() + ".comptondistance.good" + m_Suffix, true);
  m_PdfComptonScatterProbabilityBad.Write(GetFilePrefix() + ".comptondistance.bad" + m_Suffix, true);
      
  m_PdfPhotoAbsorptionProbabilityGood.Write(GetFilePrefix() + ".photodistance.good" + m_Suffix, true);
  m_PdfPhotoAbsorptionProbabilityBad.Write(GetFilePrefix() + ".photodistance.bad" + m_Suffix, true);
      
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseMultipleComptonBayes::Analyze()
{ 
  // Initlize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  
  //g_Verbosity = 1;
  double Etot = 0;
  double Eres = 0;
 
  //cout<<endl<<endl<<"Analyze: 'ing "<<m_ReEvents.size()<<" sequences"<<endl;
  for (auto RE: m_ReEvents) {
    if (RE == nullptr) continue;
    
    if (RE->GetNRESEs() <= 1) {
      mdebug<<"GeneratePdf: Not enough hits!"<<endl;
      continue;
    }
    
    if (RE->GetEnergy() < m_EnergyMinimum || RE->GetEnergy() > m_EnergyMaximum) {
      mdebug<<"GeneratePdf: Energy out of bounds!"<<endl;
      continue;
    }

    mdebug<<endl<<endl;
    mdebug<<RE->ToString()<<endl;
    
    if (RE->GetStartPoint() == 0) continue;
    
    // Check if complete sequence is ok:
    bool SequenceOk = true;
    unsigned int SequenceLength = RE->GetNRESEs();
    int PreviousPosition = 0;
    
    if (SequenceLength > m_MaxNInteractions) continue;
    
    if (SequenceLength == 2) {
      // Look at start:
      MRESEIterator Iter;
      Iter.Start(RE->GetStartPoint());
      Iter.GetNextRESE();
      Etot = RE->GetEnergy();
      Eres = RE->GetEnergyResolution();
      
      double CosPhiE = CalculateCosPhiE(*Iter.GetCurrent(), Etot);
      if (CosPhiE <= -m_MaxCosineLimit) CosPhiE = -0.99*m_MaxCosineLimit;
      if (CosPhiE >= +m_MaxCosineLimit) CosPhiE = +0.99*m_MaxCosineLimit;
      double PhotoDistance = CalculateAbsorptionProbabilityTotal(*Iter.GetCurrent(), *Iter.GetNext(), Iter.GetNext()->GetEnergy());
      
      if (IsComptonStart(*Iter.GetCurrent(), Etot, Eres) == true) {
        // IsComptonSequence caluculates absorption for the second component only, thus I have to:
        double Etot2 = Etot - Iter.GetCurrent()->GetEnergy();
        double Eres2 = sqrt(Eres*Eres - Iter.GetCurrent()->GetEnergyResolution()*Iter.GetCurrent()->GetEnergyResolution());
        if (IsComptonSequence(*Iter.GetCurrent(), *Iter.GetNext(), Etot2, Eres2) == true) {
          mdebug<<"--------> Found GOOD dual Compton event!"<<endl;
          m_PdfDualGood.Add(Etot, CosPhiE, PhotoDistance, GetMaterial(*Iter.GetCurrent()));
        } else {
          mdebug<<"--------> Found bad dual Compton event!"<<endl;
          m_PdfDualBad.Add(Etot, CosPhiE, PhotoDistance, GetMaterial(*Iter.GetCurrent()));
          SequenceOk = false;
        }
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

      double CosPhiE = CalculateCosPhiE(*Iter.GetCurrent(), Etot);
      if (CosPhiE <= -m_MaxCosineLimit) CosPhiE = -0.99*m_MaxCosineLimit;
      if (CosPhiE >= +m_MaxCosineLimit) CosPhiE = +0.99*m_MaxCosineLimit;

      if (IsComptonStart(*Iter.GetCurrent(), Etot, Eres) == true) {
        mdebug<<"--------> Found GOOD Compton start!"<<endl;
        m_PdfStartGood.Add(Etot, CosPhiE, SequenceLength, GetMaterial(*Iter.GetCurrent()));
      } else {
        mdebug<<"--------> Found bad Compton start!"<<endl;
        m_PdfStartBad.Add(Etot, CosPhiE, SequenceLength, GetMaterial(*Iter.GetCurrent()));
        SequenceOk = false;
      }
      
      // Track at start?
      if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
        double dAlpha = CalculateDCosAlpha(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
        if (dAlpha <= -m_MaxCosineLimit) dAlpha = -0.99*m_MaxCosineLimit;
        if (dAlpha >= +m_MaxCosineLimit) dAlpha = +0.99*m_MaxCosineLimit;
        double Alpha = CalculateCosAlphaG(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
        if (Alpha <= -m_MaxCosineLimit) Alpha = -0.99*m_MaxCosineLimit;
        if (Alpha >= +m_MaxCosineLimit) Alpha = +0.99*m_MaxCosineLimit;

        if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), Etot, Eres) == true) {
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
          double ComptonDistance = CalculateReach(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot);
          mdebug<<"Dist C: "<<ComptonDistance<<": real:"<<(Iter.GetCurrent()->GetPosition() - Iter.GetPrevious()->GetPosition()).Mag()<<endl;
          if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot, Eres) == true) {
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
        if (dPhi <= -m_MaxCosineLimit) dPhi = -0.99*m_MaxCosineLimit;
        if (dPhi >= +m_MaxCosineLimit) dPhi = +0.99*m_MaxCosineLimit;
        double PhiE = CalculateCosPhiE(*Iter.GetCurrent(), Etot);
        if (PhiE <= -m_MaxCosineLimit) PhiE = -0.99*m_MaxCosineLimit;
        if (PhiE >= +m_MaxCosineLimit) PhiE = +0.99*m_MaxCosineLimit;

        double Lever = CalculateMinLeverArm(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext());
        int Material = GetMaterial(*Iter.GetCurrent());
        
        if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext(), Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD internal Compton sequence!"<<endl;
          // Retrieve the data:
          m_PdfComptonGood.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);
          
        } else {
          mdebug<<"--------> Found bad internal Compton sequence!"<<endl;
          // Retrieve the data:

          m_PdfComptonBad.Add(dPhi, PhiE, Lever, Etot, SequenceLength, Material);
          if (fabs(dPhi) < 0.1) {
            cout<<"Close to good: dPhi="<<dPhi<<" PhiE="<<PhiE<<" PhiG="<<CalculateCosPhiG(*Iter.GetPrevious(), *Iter.GetCurrent(), *Iter.GetNext())<<endl;
          }
          SequenceOk = false;
        } // Add good / bad
        
        // Track at central?
        if (Iter.GetCurrent()->GetType() == MRESE::c_Track) {
          //MRETrack* T = (MRETrack*) Iter.GetCurrent();
          double dAlpha = CalculateDCosAlpha(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          if (dAlpha <= -m_MaxCosineLimit) dAlpha = -0.99*m_MaxCosineLimit;
          if (dAlpha >= +m_MaxCosineLimit) dAlpha = +0.99*m_MaxCosineLimit;
          double Alpha = CalculateCosAlphaG(*((MRETrack*) Iter.GetCurrent()), *Iter.GetNext(), Etot);
          if (Alpha <= -m_MaxCosineLimit) Alpha = -0.99*m_MaxCosineLimit;
          if (Alpha >= +m_MaxCosineLimit) Alpha = +0.99*m_MaxCosineLimit;

          if (IsComptonTrack(*Iter.GetCurrent(), *Iter.GetNext(), Etot, Eres) == true) {
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
        if (IsComptonSequence(*Iter.GetPrevious(), *Iter.GetCurrent(), Etot, Eres) == true) {
          mdebug<<"--------> Found GOOD Lastdistance sequence!"<<endl;
          // Retrieve the data:
          m_PdfPhotoAbsorptionProbabilityGood.Add(LastDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
        } else {
          mdebug<<"--------> Found bad Lastdistance sequence!"<<endl;
          // Retrieve the data:
          m_PdfPhotoAbsorptionProbabilityBad.Add(LastDistance, Etot, SequenceLength, GetMaterial(*Iter.GetCurrent()));
          if (LastDistance > 0.4 && LastDistance < 0.6 && Etot < 200) {
            cout<<"Close to good: dist="<<LastDistance<<" && Etot="<<Etot<<endl;
          }
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
  
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseMultipleComptonBayes::Finalize()
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::IsComptonTrack(MRESE& Start, MRESE& Center, double Etot, double Eres)
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
  if (IsComptonSequence(Start, Center, Etot-Start.GetEnergy(), Eres) == false) {
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


bool MResponseMultipleComptonBayes::IsTrackStart(MRESE& Start, MRESE& Central, double Energy)
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


bool MResponseMultipleComptonBayes::IsTrackStop(MRESE& Central,
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


bool MResponseMultipleComptonBayes::AreReseInSequence(MRESE& Start,
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


bool MResponseMultipleComptonBayes::IsComptonStart(MRESE& Start, double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for tripple Comptons requires:
  // (1) An absorption better than 3 sigma energy resolution + 2 keV for the first hit and the while sequence...
  // (2) The origin is or produced a photon
  // (3) We have exactly one Compton interaction and maybe some other, secondary interaction

  if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Looking at: "<<Start.GetID()<<endl;


  // Get the origin IDs of the RESEs
  vector<int> StartOriginIds = GetOriginIds(&Start);
  if (StartOriginIds.size() == 0) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Start has no Sim IDs!"<<endl;
    return false;
  }
  
  // Find the smallest origin ID in the sequence
  int SmallestOriginID = numeric_limits<int>::max();
  for (unsigned int i = 0; i < StartOriginIds.size(); ++i) {
    if (StartOriginIds[i] < SmallestOriginID) SmallestOriginID = StartOriginIds[i];
  }
  
  // Find the real Origin
  int TrueOrigin = m_SiEvent->GetIAAt(SmallestOriginID-1)->GetOriginID();
  
  // Test (1) - Absorption:
  if (Etot > 0) {
    // First hit
    if (IsAbsorbed(StartOriginIds, Start.GetEnergy(), Start.GetEnergyResolution()) == false) {
      if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: IA not completely absorbed!"<<endl;
      return false;
    }
    // Full sequence
    if (IsTotallyAbsorbed(StartOriginIds, Etot, Eres) == false) {
      if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Remaining sequence not completely absorbed!"<<endl;
      return false;
    }
  }
  
  // Test (2) - the start IA must be a photon (and create nothing) or be anything and create a photon
  MSimIA* OriginIA = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(SmallestOriginID-1)->GetOriginID()-1);
  if (OriginIA->GetSecondaryParticleID() == 1 || // create photon
     (OriginIA->GetSecondaryParticleID() == 0 && OriginIA->GetMotherParticleID() == 1)) { // create nothing and be a photon
    // good...
  } else {
    // Only photons can be good...
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: IA which triggered first RESE did not create a photon or is not photon and create nothing"<<endl;
    return false;
  }
  
  // Check if the IA only contains Compton dependants
  if (ContainsOnlyComptonDependants(StartOriginIds) == false) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Start contains not only Compton dependants"<<endl;
    return false;
  }
  
  // Check if we really have only one Compton interaction
  if (NumberOfComptonInteractions(StartOriginIds, TrueOrigin) != 1) {
    if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Not exactly one Compton in interaction"<<endl;
      return false;       
  }
  
  if (g_Verbosity >= c_Chatty) mout<<"IsComptonStart: Passed!"<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MResponseMultipleComptonBayes::NumberOfComptonInteractions(vector<int> AllSimIds, int Origin)
{
  unsigned int N = 0;
  
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetProcess() == "COMP" && m_SiEvent->GetIAAt(AllSimIds[i]-1)->GetOriginID() == Origin)  {
      ++N;
    }
  }
  
  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::IsComptonSequence(MRESE& Start, MRESE& Central, double Etot, double Eres)
{
  // Return true if the given RESEs are in sequence
  //
  // A good start point for double Comptons requires:
  // (1) An absorption better than 99%
  // (2) the IDs of their hits are in increasing order without holes
  // (3) Start is the StartPosition's Compton IA

  mdebug<<"IsComptonSequence2: Looking at: "<<Start.GetID()<<" - "<<Central.GetID()<<endl;


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
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence2: Not in Compton sequence!"<<endl;
    return false;
  }

  if (Etot > 0) {
    if (IsTotallyAbsorbed(CentralOriginIds, Etot, Eres) == false) {
      mdebug<<"IsComptonSequence2: Not completely absorbed!"<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::IsComptonSequence(MRESE& Start, MRESE& Central, MRESE& Stop, double Etot, double Eres)
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
  if (AreInComptonSequence(StartOriginIds, CentralOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }
  if (AreInComptonSequence(CentralOriginIds, StopOriginIds) == false) {
    mdebug<<"IsComptonSequence3: Not in Compton sequence!"<<endl;
    return false;
  }

  if (IsTotallyAbsorbed(CentralOriginIds, Etot, Eres) == false) {
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


bool MResponseMultipleComptonBayes::IsComptonEnd(MRESE& End)
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
  if (IsTotallyAbsorbed(EndOriginIds, End.GetEnergy(), End.GetEnergyResolution()) == false) {
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


bool MResponseMultipleComptonBayes::IsSingleCompton(MRESE& Start)
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


bool MResponseMultipleComptonBayes::AreInComptonSequence(const vector<int>& StartOriginIds, const vector<int>& CentralOriginIds)
{
  // They are in good sequence if 
  //  (1) Both smallest IDs originate from 1
  //  (2) The IDs are in sequence

  massert(StartOriginIds.size() > 0);
  massert(CentralOriginIds.size() > 0);
  
  mdebug<<"AreInComptonSequence: Start origin ID: "<<StartOriginIds[0]<<" Central origin ID: "<<CentralOriginIds[0]<<endl;

  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetProcess() != "COMP") {
    mdebug<<"AreInComptonSequence: No COMP"<<endl;
    return false;
  }

  if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetOriginID() != 
      m_SiEvent->GetIAAt(CentralOriginIds[0]-1)->GetOriginID()) {
    mdebug<<"AreInComptonSequence: No common origin"<<endl;
    return false;
  }

//   if (m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber() != 1) {
//     mdebug<<"AreInComptonSequence: Mother is no photon: "<<m_SiEvent->GetIAAt(StartOriginIds[0]-1)->GetMotherParticleNumber()<<endl;
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
    mdebug<<"AreInComptonSequence: Not behind each other!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::ContainsOnlyComptonDependants(vector<int> AllSimIds)
{
  // ?

  // We do two checks here, one down the tree and one up the tree
  
  // First check upwards: Is everything in there originating from somewhere else in there or is it an initial process (COMP, INIT, produced a photon)
  //cout<<"ContainsOnlyComptonDependants: Sim IDs: "; for (auto i: AllSimIds) { cout<<i<<" "; } cout<<endl;

  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    //cout<<"ContainsOnlyComptonDependants checking "<<AllSimIds[i]<<endl;
    int ID = AllSimIds[i];
    int HighestOriginID = ID;
    while (true) {
      // Get new origin ID
      ID = m_SiEvent->GetIAAt(ID-1)->GetOriginID();
      // Check if included
      for (unsigned int j = 0; j < AllSimIds.size(); ++j) {
        if (i != j && ID == AllSimIds[j]) {
          HighestOriginID = ID;
          break;
        }
      }
      
      // Check if we are done
      if (m_SiEvent->GetIAAt(ID-1)->GetProcess() == "INIT") break; // Compton dependents, remember?
    }
    
    // We are good if the end point is a Compton scatter or a photo effect which is preceeded by a Compton in the list:
    bool IsGood = false;
    mdebug<<"ContainsOnlyComptonDependants: Highest ID "<<HighestOriginID<<endl;
    if (m_SiEvent->GetIAAt(HighestOriginID-1)->GetProcess() == "COMP") {
      mdebug<<"ContainsOnlyComptonDependants: Highest ID "<<HighestOriginID<<" is COMP"<<endl;
      IsGood = true;
    } else if (m_SiEvent->GetIAAt(HighestOriginID-1)->GetProcess() == "PHOT") {
      int Predeccessor = HighestOriginID-1;
      while (true) {
        if (Predeccessor == 1) {
          mdebug<<"ContainsOnlyComptonDependants: Good. "<<Predeccessor<<" is INIT"<<endl;
          break; // we reached init
        }
        if (m_SiEvent->GetIAAt(Predeccessor-1)->GetOriginID() == m_SiEvent->GetIAAt(HighestOriginID-1)->GetOriginID()) {
          if (m_SiEvent->GetIAAt(Predeccessor-1)->GetProcess() == "COMP") {
            /* 2024-11-24: This doesn't make sense: If its a predecessor it will never be in the list
            if (find(AllSimIds.begin(), AllSimIds.end(), Predeccessor) != AllSimIds.end()) {
              mdebug<<"ContainsOnlyComptonDependants: Good. Preceeding Compton IA "<<Predeccessor<<" is in list"<<endl;
              IsGood = true;
              break;
            } else {
              mdebug<<"ContainsOnlyComptonDependants: Preceeding Compton IA "<<Predeccessor<<" is missing in list"<<endl;
              return false;
            }
            */
            // Replacement:
            IsGood = true;
            break;
          }
        }
        --Predeccessor;
      }
    }
    if (IsGood == false) {
      mdebug<<"ContainsOnlyComptonDependants: Hits other than dependants for ID="<<ID<<endl;
      return false;
    }
  }

  
  // The second check is downward: Are all particles generated from this process contained?
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    vector<int> Descendents;
    Descendents.push_back(AllSimIds[i]);
    unsigned int Size = 1;
    do {
      Size = Descendents.size();
      for (unsigned int j = 0; j < m_SiEvent->GetNIAs(); ++j) {
        for (unsigned int d = 0; d < Descendents.size(); ++d) {
          if (m_SiEvent->GetIAAt(j)->GetOriginID() == Descendents[d]) {
            if (find(Descendents.begin(), Descendents.end(), j+1) != Descendents.end()) {
              Descendents.push_back(j+1);
            }
          }
        }
      }
    } while (Size < Descendents.size());
    
    // Now check if all are contianed:
    for (unsigned int d = 0; d < Descendents.size(); ++d) {
      if (m_SiEvent->GetIAAt(Descendents[d]-1)->GetSecondaryParticleID() != 0 && m_SiEvent->GetIAAt(Descendents[d]-1)->GetSecondaryParticleID() != 1) {
        bool Found = false;
        for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
          if (d == i) {
            Found = true; 
          }
        }
        if (Found == false) {
          mdebug<<"ContainsOnlyComptonDependants: Originates but not contained: ID="<<d<<endl; 
        }
      }
    }
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::IsAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution)
{
  // Rules:
  // If a Compton scattering is included, the energy of the of the recoil electron plus any instant deposits, bremsstrahlug, etc. must be contained
  // If a photo is included, the energy of the INCOMING gamma-ray must be contained
  
  EnergyResolution = 3.0*EnergyResolution + 2.0;
  
  double IdealEnergy = 0.0;
  
  vector<int> SimIDs = AllSimIds;
  
  // (1) First clean the IDs
  vector<int> CleanedSimIDs;
  // (a) If we have secondaries, remove them if the parent is contained
  //     Or inverse, keep all IAs whose parents are not included
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    bool Found = false;
    for (unsigned int j = 0; j < SimIDs.size(); ++j) {
      if (i != j) {
        if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetOriginID() == SimIDs[j]) {
          Found = true;
          break;
        }
      }
    }
    if (Found == false) {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
  
  // (b) Keep only the smallest PHOT IA since we require it to be contained
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "PHOT") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetOriginID() == m_SiEvent->GetIAAt(SimIDs[i]-2)->GetOriginID() && m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() != "PHOT") {
        CleanedSimIDs.push_back(SimIDs[i]);
      }
    } else {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
    
  // (c) Exclude fluoresence COMP
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "COMP") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetSecondaryParticleID() == 3) {
        CleanedSimIDs.push_back(SimIDs[i]);
      }
    } else {
      CleanedSimIDs.push_back(SimIDs[i]);
    }
  }
  SimIDs = CleanedSimIDs;
  CleanedSimIDs.clear();
  
  mdebug<<"SimIDs: "; for (int i: SimIDs) mdebug<<i<<" "; mdebug<<endl;
  
  // (d) Sanity check - we just should have COMP & PHOT, RAYL & PAIR in our list
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() != "COMP" && m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() != "PHOT" && m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() != "RAYL") {
      cout<<"Info: We are ignoreing events that contain any other interaction but COMP, PHOT, RAYL. This one has \""<<m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess()<<"\"."<<endl;
      return false;
    }
  }
  
  // (2) Now add the energies
  for (unsigned int i = 0; i < SimIDs.size(); ++i) {
    if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "COMP") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "COMP" || m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "RAYL") {
        mdebug<<"COMP with COMP/RAYL predeccessor: Adding mother difference energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy();
      } else {
        mdebug<<"COMP WITHOUT COMP/RAYL predeccessor: Adding secondary-mother difference energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy() - m_SiEvent->GetIAAt(SimIDs[i]-1)->GetMotherEnergy();
      }
    } else if (m_SiEvent->GetIAAt(SimIDs[i]-1)->GetProcess() == "PHOT") {
      if (m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "COMP" || m_SiEvent->GetIAAt(SimIDs[i]-2)->GetProcess() == "RAYL") {
        mdebug<<"PHOT with COMP/RAYL predeccessor: Adding mother energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetMotherEnergy();
      } else {
        mdebug<<"PHOT withOUT COMP/RAYL predeccessor: Adding secondary energy: "<<m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy()<<endl;
        IdealEnergy += m_SiEvent->GetIAAt(SimIDs[i]-2)->GetSecondaryEnergy();
      }      
    } else {
      cout<<"Error: We only should have COMP, RAYL, and PHOT IA's at this point. Did you use hadronic processes for the simulations?"<<endl;
    }
  }
  
  // (3) Check for absorption
  if (fabs(Energy - IdealEnergy) > EnergyResolution) {
    mdebug<<"IsAbsorbed(): Not completely absorbed: ideal="<<IdealEnergy<<"  observed="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"IsAbsorbed(): Completely absorbed: ideal="<<IdealEnergy<<" observed="<<Energy<<endl;
  }
  return true;  
}


////////////////////////////////////////////////////////////////////////////////


bool MResponseMultipleComptonBayes::IsTotallyAbsorbed(const vector<int>& AllSimIds, double Energy, double EnergyResolution)
{
  // This checks if the sequence is totally absorbed

  massert(AllSimIds.size() > 0);

  EnergyResolution = 3*EnergyResolution+2;

  // Find the minimum ID, as it will belong to the original Compton
  int MinId = numeric_limits<int>::max();
  for (unsigned int i = 0; i < AllSimIds.size(); ++i) {
    if (MinId > AllSimIds[i] && AllSimIds[i] > 1) MinId = AllSimIds[i];
  }
  mdebug<<"IsTotallyAbsorbed: Min ID: "<<MinId<<endl;

  if (MinId == numeric_limits<int>::max()) return false;

  int CurrentIDInVector = MinId - 1;
  int MotherIDInVector = MinId - 2;

  MSimIA* Top = 0;
  // If Current and mother have same origin ID then we are in a normal sequence, and choose the mother
  if (m_SiEvent->GetIAAt(MotherIDInVector)->GetOriginID() == m_SiEvent->GetIAAt(CurrentIDInVector)->GetOriginID()) {
    Top = m_SiEvent->GetIAAt(MotherIDInVector);
  }
  // Other wise we have something like Bremsstrahlung and have to find the start this way:
  else {
    Top = m_SiEvent->GetIAAt(m_SiEvent->GetIAAt(CurrentIDInVector)->GetOriginID()-1);
  }

  double Ideal = 0.0;
  if (m_SiEvent->GetIAAt(MotherIDInVector)->GetProcess() == "COMP") {
    Ideal = Top->GetMotherEnergy();
    mdebug<<"IsTotallyAbsorbed: GetMother of "<<Top->GetID()<<" with "<<Ideal<<endl;
  } else {
    Ideal = Top->GetSecondaryEnergy();
    mdebug<<"IsTotallyAbsorbed: GetSecondary of "<<Top->GetID()<<" with "<<Ideal<<endl;
  }

  /* Now that was redundant and wrong
  if (MotherIDInVector != 0) {
    Ideal = m_SiEvent->GetIAAt(MotherIDInVector)->GetMotherEnergy();
  } else {
    Ideal = m_SiEvent->GetIAAt(MotherIDInVector)->GetSecondaryEnergy();
  }
  */

  if (fabs(Ideal - Energy) > EnergyResolution) {
      mdebug<<"IsTotallyAbsorbed: Not completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
    return false;
  } else {
    mdebug<<"IsTotallyAbsorbed: Completely absorbed: Tot abs: i="<<Ideal<<"  r="<<Energy<<"  3*Eres+2="<<EnergyResolution<<endl;
  }

//   if (fabs((Ideal - Energy)/Ideal) > m_MaxEnergyDifferencePercent &&
//       fabs(Ideal - Energy) > m_MaxEnergyDifference ) {
//       mdebug<<"IsTotallyAbsorbed: Not completely absorbed: Tot abs: i="<<Ideal<<" r="<<Energy<<endl;
//     return false;
//   }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::GetIdealDepositedEnergy(int MinId)
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


bool MResponseMultipleComptonBayes::IsTrackCompletelyAbsorbed(const vector<int>& Ids, double Energy)
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


double MResponseMultipleComptonBayes::CalculateMinLeverArm(MRESE& Start, MRESE& Central,
                                               MRESE& Stop)
{
  return MERCSRBayesian::CalculateMinLeverArm(Start.GetPosition(),
                                              Central.GetPosition(),
                                              Stop.GetPosition());
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateDPhiInDegree(MRESE& Start, MRESE& Central,
                                        MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDPhiInDegree(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateDCosPhi(MRESE& Start, MRESE& Central,
                                                  MRESE& Stop, double Energy)
{
  return MERCSRBayesian::CalculateDCosPhi(&Start, &Central, &Stop, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateDCosAlpha(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDCosAlpha(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateDAlphaInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateDAlphaInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateCosAlphaE(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaE(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateAlphaEInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaEInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateCosAlphaG(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateCosAlphaG(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateAlphaGInDegree(MRETrack& Track, MRESE& Central, double Energy)
{
  return MERCSRBayesian::CalculateAlphaGInDegree(&Track, &Central, Energy);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateAbsorptionProbabilityPhoto(MRESE& Start,
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


double MResponseMultipleComptonBayes::CalculateAbsorptionProbabilityTotal(MRESE& Start,
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


double MResponseMultipleComptonBayes::CalculateAbsorptionProbabilityCompton(MRESE& Start, MRESE& Stop, double Etot)
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


double MResponseMultipleComptonBayes::CalculateReach(MRESE& Start, MRESE& Stop, double Etot)
{
  double ReachProbability = 1.0;
  ReachProbability *= (1 - m_ReGeometry->GetPairAbsorptionProbability(Start.GetPosition(), Stop.GetPosition(), Etot));
  ReachProbability *= (1 - m_ReGeometry->GetComptonAbsorptionProbability(Start.GetPosition(), Stop.GetPosition(), Etot));
  ReachProbability *= (1 - m_ReGeometry->GetPhotoAbsorptionProbability(Start.GetPosition(), Stop.GetPosition(), Etot));

  return ReachProbability;
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculatePhiEInDegree(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculatePhiEInDegree(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculatePhiGInDegree(MRESE& Start, MRESE& Central,
                                                       MRESE& Stop)
{
  return MERCSRBayesian::CalculatePhiGInDegree(&Start, &Central, &Stop);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateCosPhiG(MRESE& Start, MRESE& Central,
                                                  MRESE& Stop)
{
  return MERCSRBayesian::CalculateCosPhiG(&Start, &Central, &Stop);
}


////////////////////////////////////////////////////////////////////////////////


double MResponseMultipleComptonBayes::CalculateCosPhiE(MRESE& Central, double Etot)
{
  return MERCSRBayesian::CalculateCosPhiE(&Central, Etot);
}


////////////////////////////////////////////////////////////////////////////////


int MResponseMultipleComptonBayes::GetMaterial(MRESE& Hit)
{
  return MERCSRBayesian::GetMaterial(&Hit);
}


// MResponseMultipleComptonBayes.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
