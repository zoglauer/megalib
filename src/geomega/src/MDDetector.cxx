/*
 * MDDetector.cxx
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
// MDDetector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDDetector.h"

// Standard libs:
#include <limits>
#include <iostream>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGALib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MDShapeBRIK.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDDetector)
#endif


////////////////////////////////////////////////////////////////////////////////


int MDDetector::m_IDCounter                         = 1;
int MDDetector::m_SensIDCounter                     = 1;

// Never ever change this numbering, unless you want to break the sim files!!!!
const int MDDetector::c_NoDetectorType              = 0;
const int MDDetector::c_Strip2D                     = 1;
const int MDDetector::c_Calorimeter                 = 2;
const int MDDetector::c_Strip3D                     = 3;
const int MDDetector::c_ACS                         = 4;
const int MDDetector::c_Scintillator                = 4;
const int MDDetector::c_DriftChamber                = 5;
const int MDDetector::c_Strip3DDirectional          = 6;
const int MDDetector::c_AngerCamera                 = 7;
const int MDDetector::c_Voxel3D                     = 8;

const int MDDetector::c_MinDetector                 = 1;
const int MDDetector::c_MaxDetector                 = 8;

const MString MDDetector::c_NoDetectorTypeName      = "NoDetectorType";
const MString MDDetector::c_Strip2DName             = "Strip2D";
const MString MDDetector::c_CalorimeterName         = "Calorimeter";
const MString MDDetector::c_Strip3DName             = "Strip3D";
const MString MDDetector::c_ACSName                 = "Scintillator";
const MString MDDetector::c_ScintillatorName        = "Scintillator";
const MString MDDetector::c_DriftChamberName        = "DriftChamber";
const MString MDDetector::c_Strip3DDirectionalName  = "Strip3DDirectional";
const MString MDDetector::c_AngerCameraName         = "AngerCamera";
const MString MDDetector::c_Voxel3DName             = "Voxel3D";

const int MDDetector::c_EnergyResolutionTypeUnknown     = 0;
const int MDDetector::c_EnergyResolutionTypeNone        = 1;
const int MDDetector::c_EnergyResolutionTypeIdeal       = 2;
const int MDDetector::c_EnergyResolutionTypeGauss       = 3;
const int MDDetector::c_EnergyResolutionTypeLorentz     = 4;
const int MDDetector::c_EnergyResolutionTypeGaussLandau = 5;

const int MDDetector::c_EnergyLossTypeUnknown       = 0;
const int MDDetector::c_EnergyLossTypeNone          = 1;
const int MDDetector::c_EnergyLossTypeMap           = 2;

const int MDDetector::c_TimeResolutionTypeUnknown     = 0;
const int MDDetector::c_TimeResolutionTypeNone        = 1;
const int MDDetector::c_TimeResolutionTypeIdeal       = 2;
const int MDDetector::c_TimeResolutionTypeGauss       = 3;

const int MDDetector::c_DepthResolutionTypeUnknown     = 0;
const int MDDetector::c_DepthResolutionTypeNone        = 1;
const int MDDetector::c_DepthResolutionTypeIdeal       = 2;
const int MDDetector::c_DepthResolutionTypeGauss       = 3;

const int MDDetector::c_GuardringEnergyResolutionTypeUnknown     = 0;
const int MDDetector::c_GuardringEnergyResolutionTypeNone        = 1;
const int MDDetector::c_GuardringEnergyResolutionTypeIdeal       = 2;
const int MDDetector::c_GuardringEnergyResolutionTypeGauss       = 3;


////////////////////////////////////////////////////////////////////////////////


MDDetector::MDDetector(MString Name)
{
  // default constructor

  // ID of this detector:
  if (m_IDCounter == numeric_limits<int>::max()) {
    m_IDCounter = 0;
  } else {
    m_ID = m_IDCounter++;
  }  
  
  m_Name = Name;
  m_Description = "Unknown";

  m_Type = c_NoDetectorType;

  m_DetectorVolume = 0;
  m_CommonVolume = 0;
  
  m_StructuralDimension = g_VectorNotDefined;
  m_StructuralPitch = g_VectorNotDefined;
  m_StructuralOffset = g_VectorNotDefined;
  m_StructuralSize = g_VectorNotDefined;
  
  m_IsNamedDetector = false;
  m_NamedAfter = 0;
  
  m_NoiseThresholdEqualsTriggerThresholdSet = false;
  m_NoiseThresholdEqualsTriggerThreshold = false;
  
  m_NoiseThreshold = g_DoubleNotDefined;
  m_NoiseThresholdSigma = g_DoubleNotDefined;
  
  m_TriggerThreshold = g_DoubleNotDefined;
  m_TriggerThresholdSigma = g_DoubleNotDefined;

  m_FailureRate = g_DoubleNotDefined;
  
  m_Overflow = g_DoubleNotDefined;
  m_OverflowSigma = g_DoubleNotDefined;

  m_EnergyLossType = c_EnergyLossTypeUnknown;
  m_EnergyResolutionType = c_EnergyResolutionTypeUnknown;
  m_TimeResolutionType = c_TimeResolutionTypeUnknown;

  m_PulseShapeSet = false;
  m_PulseShape = 0;
  m_PulseShapeMin = 0;
  m_PulseShapeMax = 0;

  m_NoiseActive = true;

  m_UseDivisions = false;
  m_ShortNameDivisionX = g_StringNotDefined;
  m_ShortNameDivisionY = g_StringNotDefined;
  m_ShortNameDivisionZ = g_StringNotDefined;

  m_HasGuardring = false;

  m_AreBlockedTriggerChannelsUsed = false;

  m_EnergyCalibrationSet = false;
  m_UseEnergyCalibration = false;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector::MDDetector(const MDDetector& D)
{
  // Copy constructor

  m_Name = D.m_Name;
  
  m_Type = D.m_Type;
  m_Description = D.m_Description;

  for (unsigned int i = 0; i < m_SVs.size(); ++i) {
    m_SVs.push_back(D.m_SVs[i]);
  }
  m_DetectorVolume = D.m_DetectorVolume;
  m_CommonVolume = D.m_CommonVolume;
  
  m_ID = m_IDCounter++;
  m_SensID = m_SensIDCounter++;            

  m_IsNamedDetector = D.m_IsNamedDetector;
  m_NamedAfter = D.m_NamedAfter;
  
  m_UseDivisions = D.m_UseDivisions;
  m_ShortNameDivisionX = D.m_ShortNameDivisionX;
  m_ShortNameDivisionY = D.m_ShortNameDivisionY;
  m_ShortNameDivisionZ = D.m_ShortNameDivisionZ;

  m_EnergyLossType = D.m_EnergyLossType;
  m_EnergyLossMap = D.m_EnergyLossMap;

  m_EnergyResolutionType = D.m_EnergyResolutionType;
  m_EnergyResolutionPeak1 = D.m_EnergyResolutionPeak1; 
  m_EnergyResolutionWidth1 = D.m_EnergyResolutionWidth1;
  m_EnergyResolutionPeak2 = D.m_EnergyResolutionPeak2; 
  m_EnergyResolutionWidth2 = D.m_EnergyResolutionWidth2;
  m_EnergyResolutionRatio = D.m_EnergyResolutionRatio;

  m_TimeResolutionType = D.m_TimeResolutionType;
  m_TimeResolution = D.m_TimeResolution;

  m_FailureRate = D.m_FailureRate;

  m_NoiseThresholdEqualsTriggerThresholdSet = D.m_NoiseThresholdEqualsTriggerThresholdSet;
  m_NoiseThresholdEqualsTriggerThreshold = D.m_NoiseThresholdEqualsTriggerThreshold;
  
  m_NoiseThreshold = D.m_NoiseThreshold;
  m_NoiseThresholdSigma = D.m_NoiseThresholdSigma;

  m_TriggerThreshold = D.m_TriggerThreshold;
  m_TriggerThresholdSigma = D.m_TriggerThresholdSigma;

  m_Overflow = D.m_Overflow;
  m_OverflowSigma = D.m_OverflowSigma;


  m_StructuralDimension = D.m_StructuralDimension;     
  m_StructuralSize = D.m_StructuralSize;     
  m_StructuralOffset = D.m_StructuralOffset;
  m_StructuralPitch = D.m_StructuralPitch;    

  m_HasGuardring = D.m_HasGuardring;

  if (D.m_PulseShape != 0) {
    m_PulseShape = new TF1(*D.m_PulseShape);
  } else {
    m_PulseShape = 0; 
  }
  m_PulseShapeMin = D.m_PulseShapeMin;
  m_PulseShapeMax = D.m_PulseShapeMax;

  m_NoiseActive = D.m_NoiseActive;

  m_AreBlockedTriggerChannelsUsed = D.m_AreBlockedTriggerChannelsUsed;
  m_BlockedTriggerChannels = D.m_BlockedTriggerChannels;
  
  m_EnergyCalibrationSet = D.m_EnergyCalibrationSet;
  m_UseEnergyCalibration = D.m_UseEnergyCalibration;
  m_EnergyCalibration = D.m_EnergyCalibration;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int d = 0; d < m_NamedDetectors.size(); ++d) {

    m_NamedDetectors[d]->m_Description = m_Description;

    for (unsigned int i = 0; i < m_SVs.size(); ++i) {
      m_NamedDetectors[d]->m_SVs.push_back(m_SVs[i]);
    }
    m_NamedDetectors[d]->m_DetectorVolume = m_DetectorVolume;
    
    m_NamedDetectors[d]->m_UseDivisions = m_UseDivisions;
    m_NamedDetectors[d]->m_ShortNameDivisionX = m_ShortNameDivisionX;
    m_NamedDetectors[d]->m_ShortNameDivisionY = m_ShortNameDivisionY;
    m_NamedDetectors[d]->m_ShortNameDivisionZ = m_ShortNameDivisionZ;

    
    if (m_NamedDetectors[d]->m_EnergyLossType == c_EnergyLossTypeUnknown && 
        m_EnergyLossType != c_EnergyLossTypeUnknown) {
      m_NamedDetectors[d]->m_EnergyLossType = m_EnergyLossType;
      m_NamedDetectors[d]->m_EnergyLossMap = m_EnergyLossMap;
    }
    
    if (m_NamedDetectors[d]->m_EnergyResolutionType == c_EnergyResolutionTypeUnknown && 
        m_EnergyResolutionType != c_EnergyResolutionTypeUnknown) {
      m_NamedDetectors[d]->m_EnergyResolutionType = m_EnergyResolutionType;
      m_NamedDetectors[d]->m_EnergyResolutionPeak1 = m_EnergyResolutionPeak1; 
      m_NamedDetectors[d]->m_EnergyResolutionWidth1 = m_EnergyResolutionWidth1;
      m_NamedDetectors[d]->m_EnergyResolutionPeak2 = m_EnergyResolutionPeak2; 
      m_NamedDetectors[d]->m_EnergyResolutionWidth2 = m_EnergyResolutionWidth2;
      m_NamedDetectors[d]->m_EnergyResolutionRatio = m_EnergyResolutionRatio;
    }
    
    if (m_NamedDetectors[d]->m_TimeResolutionType == c_TimeResolutionTypeUnknown && 
        m_TimeResolutionType != c_TimeResolutionTypeUnknown) {
      m_NamedDetectors[d]->m_TimeResolutionType = m_TimeResolutionType;
      m_NamedDetectors[d]->m_TimeResolution = m_TimeResolution;
    }
    
    if (m_NamedDetectors[d]->m_FailureRate == g_DoubleNotDefined && 
        m_FailureRate != g_DoubleNotDefined) {
      m_NamedDetectors[d]->m_FailureRate = m_FailureRate;
    }

    if (m_NamedDetectors[d]->m_NoiseThresholdEqualsTriggerThresholdSet == false && 
        m_NoiseThresholdEqualsTriggerThresholdSet == true) {
      m_NamedDetectors[d]->m_NoiseThresholdEqualsTriggerThresholdSet = m_NoiseThresholdEqualsTriggerThresholdSet;
      m_NamedDetectors[d]->m_NoiseThresholdEqualsTriggerThreshold = m_NoiseThresholdEqualsTriggerThreshold;
    }

    if (m_NamedDetectors[d]->m_NoiseThreshold == g_DoubleNotDefined && 
        m_NoiseThreshold != g_DoubleNotDefined) {
      m_NamedDetectors[d]->m_NoiseThreshold = m_NoiseThreshold;
      m_NamedDetectors[d]->m_NoiseThresholdSigma = m_NoiseThresholdSigma;
    }
    
    if (m_NamedDetectors[d]->m_TriggerThreshold == g_DoubleNotDefined && 
        m_TriggerThreshold != g_DoubleNotDefined) {
      m_NamedDetectors[d]->m_TriggerThreshold = m_TriggerThreshold;
      m_NamedDetectors[d]->m_TriggerThresholdSigma = m_TriggerThresholdSigma;
    }

    if (m_NamedDetectors[d]->m_Overflow == g_DoubleNotDefined && 
        m_Overflow != g_DoubleNotDefined) {
      m_NamedDetectors[d]->m_Overflow = m_Overflow;
      m_NamedDetectors[d]->m_OverflowSigma = m_OverflowSigma;
    }
    
    m_NamedDetectors[d]->m_StructuralDimension = m_StructuralDimension;     
    m_NamedDetectors[d]->m_StructuralSize = m_StructuralSize;     
    m_NamedDetectors[d]->m_StructuralOffset = m_StructuralOffset;
    m_NamedDetectors[d]->m_StructuralPitch = m_StructuralPitch;    

    m_NamedDetectors[d]->m_HasGuardring = m_HasGuardring;

    if (m_PulseShape != 0) m_NamedDetectors[d]->m_PulseShape = new TF1(*m_PulseShape);
    m_NamedDetectors[d]->m_PulseShapeMin = m_PulseShapeMin;
    m_NamedDetectors[d]->m_PulseShapeMax = m_PulseShapeMax;

    m_NamedDetectors[d]->m_NoiseActive = m_NoiseActive;

    m_NamedDetectors[d]->m_AreBlockedTriggerChannelsUsed = m_AreBlockedTriggerChannelsUsed;
    m_NamedDetectors[d]->m_BlockedTriggerChannels = m_BlockedTriggerChannels;

    if (m_NamedDetectors[d]->m_EnergyCalibrationSet == false && 
        m_EnergyCalibrationSet == true) {
      m_NamedDetectors[d]->m_EnergyCalibrationSet = m_EnergyCalibrationSet;
      m_NamedDetectors[d]->m_UseEnergyCalibration = m_UseEnergyCalibration;
      m_NamedDetectors[d]->m_EnergyCalibration = m_EnergyCalibration;
    }
  }
  
  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


MDDetector::~MDDetector()
{
  // default destructor
  
  delete m_PulseShape;
}


////////////////////////////////////////////////////////////////////////////////


MString MDDetector::GetDetectorTypeName(const int Type)
{
  // Return the string description of this detector type:

  if (Type == c_Strip2D) {
    return c_Strip2DName;
  } else if (Type == c_Calorimeter ) {
    return c_CalorimeterName;
  } else if (Type == c_Strip3D) {
    return c_Strip3DName;
  } else if (Type == c_ACS || Type == c_Scintillator) {
    return c_ScintillatorName;
  } else if (Type == c_DriftChamber) {
    return c_DriftChamberName;
  } else if (Type == c_AngerCamera) {
    return c_AngerCameraName;
  } else if (Type == c_Voxel3D) {
    return c_Voxel3DName;
  } 

  merr<<"Unknown detector type: "<<Type<<endl;
  
  return c_NoDetectorTypeName;
}


////////////////////////////////////////////////////////////////////////////////


int MDDetector::GetDetectorType(const MString& Type)
{
  // Return the string description of this detector type:

  if (Type == c_Strip2DName) {
    return c_Strip2D;
  } else if (Type == c_CalorimeterName) {
    return c_Calorimeter;
  } else if (Type == c_Strip3DName) {
    return c_Strip3D;
  } else if (Type == c_ACSName || Type == c_ScintillatorName) {
    return c_Scintillator;
  } else if (Type == c_DriftChamberName) {
    return c_DriftChamber;
  } else if (Type == c_Strip3DDirectionalName) {
    return c_Strip3DDirectional;
  } else if (Type == c_AngerCameraName) {
    return c_AngerCamera;
  } 

  merr<<"Unknown detector type: "<<Type<<show;
  
  return c_NoDetectorType;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::IsValidDetectorType(const MString& Name)
{
  //

  if (Name == c_Strip2DName ||
      Name == c_CalorimeterName ||
      Name == c_Strip3DName ||
      Name == c_Strip3DDirectionalName ||
      Name == c_ScintillatorName ||
      Name == c_DriftChamberName ||
      Name == c_AngerCameraName) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::IsValidDetectorType(int ID)
{
  //

  if (ID >= c_MinDetector && ID <= c_MaxDetector) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::UseDivisions(const MString& ShortNameX, 
                              const MString& ShortNameY, 
                              const MString& ShortNameZ)
{
  m_UseDivisions = true;

  m_ShortNameDivisionX = ShortNameX;
  m_ShortNameDivisionY = ShortNameY;
  m_ShortNameDivisionZ = ShortNameZ;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetNoiseThreshold(const double Threshold)
{
  //

  m_NoiseThreshold = Threshold;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetNoiseThreshold(const MVector& Position) const
{
  // 

  return m_NoiseThreshold;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetTriggerThreshold(const double Threshold)
{
  //

  m_TriggerThreshold = Threshold;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetTriggerThreshold(const MVector& Position) const
{
  // 

  return m_TriggerThreshold;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetSecureUpperLimitTriggerThreshold() const
{
  // In ANY case the real trigger threshold is below this value:

  return 1.25*m_TriggerThreshold + 7.0*m_TriggerThresholdSigma;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetNoiseThresholdSigma(const double ThresholdSigma)
{
  //

  m_NoiseThresholdSigma = ThresholdSigma;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetNoiseThresholdSigma(const MVector& Position) const
{
  // 

  return m_NoiseThresholdSigma;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetTriggerThresholdSigma(const double ThresholdSigma)
{
  //

  m_TriggerThresholdSigma = ThresholdSigma;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetTriggerThresholdSigma(const MVector& Position) const 
{
  // 

  return m_TriggerThresholdSigma;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetPulseShape(const double Pol0, const double Pol1, const double Pol2, 
                               const double Pol3, const double Pol4, const double Pol5, 
                               const double Pol6, const double Pol7, const double Pol8, 
                               const double Pol9, const double Min, const double Max)
{
  m_PulseShapeMin = Min;
  m_PulseShapeMax = Max;

  delete m_PulseShape;
  m_PulseShape = new TF1("PulseShape", "pol9", Min, Max);
  m_PulseShape->SetParameters(Pol0, Pol1, Pol2, Pol3, Pol4, Pol5, Pol6, Pol7, Pol8, Pol9);
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::HasTimeResolution() const
{
  // Return true if the detector has a time resolution

  if (m_TimeResolutionType != c_TimeResolutionTypeNone && 
      m_TimeResolutionType != c_TimeResolutionTypeUnknown) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::ApplyPulseShape(const double NanoSeconds, const double Energy) const
{
  if (m_PulseShape == 0 || 
      NanoSeconds < m_PulseShapeMin || 
      NanoSeconds > m_PulseShapeMax) {
    return 0;
  }

  return m_PulseShape->Eval(NanoSeconds)*Energy;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::ApplyEnergyResolution(double& Energy, const MVector& Position) const 
{
  // Noise the energy...
  
  if (m_EnergyResolutionType == c_EnergyResolutionTypeNone) {
    Energy = 0;
  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeIdeal) {
    // do nothing
  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
    Energy = GetEnergyResolutionPeak1(Energy, Position) + 
      gRandom->Gaus(0.0, GetEnergyResolutionWidth1(Energy, Position));

  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {

    double Peak = GetEnergyResolutionPeak1(Energy, Position);
    double Width = 2.35/2*GetEnergyResolutionWidth1(Energy, Position);
    static const double Min = 0.0025;
    
    double CutOff = sqrt((Width*Width*(1-Min))/Min);
    double E = 0.0;
    double Height = 0.0;

    // sample the Lorentz distribution
    do {
      E = gRandom->Rndm()*CutOff;
      Height = Width*Width/(Width*Width + E*E);
    } while (gRandom->Rndm() > Height);

    if (gRandom->Rndm() >= 0.5) {
      Energy = Peak+E;
    } else {
      Energy = Peak-E;      
    }

  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
    
    double ScalerGauss = GetEnergyResolutionRatio(Energy, Position);
    double SigmaGauss = GetEnergyResolutionWidth1(Energy, Position);
    double MeanGauss = GetEnergyResolutionPeak1(Energy, Position);
    
    double ScalerLandau = 1-ScalerGauss;
    double SigmaLandau = GetEnergyResolutionWidth2(Energy, Position);
    double MeanLandau = GetEnergyResolutionPeak2(Energy, Position);

    // Determine the maximum of the distribution for the given input energy
    double Max = 0;
    double arg = (Energy - MeanGauss)/SigmaGauss;
    Max += ScalerGauss*TMath::Exp(-0.5*arg*arg); 
    Max += ScalerLandau*TMath::Landau(-Energy + MeanLandau, 0, SigmaLandau);


    double E;
    double EMin = MeanGauss - 10*SigmaLandau-10*SigmaGauss;
    if (EMin < 0) EMin = 0;
    double EMax = MeanGauss + 4*SigmaLandau+4*SigmaGauss;
    double Random = 0.0;
    int Trials = 0;
    do {
      // x-value:
      E = gRandom->Rndm()*(EMax - EMin) + EMin;
      arg = (E - MeanGauss)/SigmaGauss;
      Random = 0.0;
      Random += ScalerGauss*TMath::Exp(-0.5*arg*arg); 
      Random += ScalerLandau*TMath::Landau(-E + MeanLandau, 0, SigmaLandau);
      //cout<<"R: "<<Random<<" - Max: "<<Max<<" - Energy: "<<MeanGauss<<endl;
      if (++Trials >= 100) {
        MeanGauss = 0;
        break;
      }
    } while (Random < gRandom->Rndm()*Max);
    
    Energy = E;
    
  } else {
    merr<<"Unknown energy resolution type!!!!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::ApplyEnergyCalibration(double& Energy) const
{
  // Retrieve a calibrated energy

  if (m_UseEnergyCalibration == true) {
    Energy = m_EnergyCalibration.Evaluate(Energy);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::ApplyTimeResolution(double& Time, const double Energy) const 
{
  // Noise the time...

  if (m_TimeResolutionType == c_TimeResolutionTypeNone) {
    Time = 0;
  } else if (m_TimeResolutionType == c_TimeResolutionTypeIdeal) {
    // do nothing
  } else if (m_TimeResolutionType == c_TimeResolutionTypeGauss) {
    Time = gRandom->Gaus(Time, GetTimeResolution(Energy));
  } else {
    merr<<"Unknown time resolution type: "<<m_TimeResolutionType<<endl;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::ApplyNoiseThreshold(double& Energy, const MVector& Position) const 
{
  // Test if the energy is in the noise threshold regime and apply it     

  double NoiseThreshold = 0.0;
  
  if (m_NoiseThresholdEqualsTriggerThreshold == true) {
    // If the flag is set no own noise threshold is given...
    NoiseThreshold = gRandom->Gaus(GetTriggerThreshold(Position), m_TriggerThresholdSigma);
  } else {
    NoiseThreshold = gRandom->Gaus(GetNoiseThreshold(Position), m_NoiseThresholdSigma);
  }

  if (Energy < NoiseThreshold) {
    Energy = 0;
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::ApplyOverflow(double& Energy) const 
{
  // Test if the energy is in the overflow and apply it     

  double Overflow = gRandom->Gaus(m_Overflow, m_OverflowSigma);

  if (Energy > Overflow) {
    Energy = Overflow;
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::IsAboveTriggerThreshold(const double& Energy, const MDGridPoint& Point) const 
{
  // Is this hit above the trigger threshold ?

  // If the channel is blocked from triggering, we are per definition never above the trigger threshold...
  if (m_AreBlockedTriggerChannelsUsed == true) {
    if (m_BlockedTriggerChannels.GetVoxelValue(Point) > 0.0) {
      return false;
    }
  }

  // Given this flag, in case we passed the noise criteria, we also have a trigger:
  if (m_NoiseThresholdEqualsTriggerThreshold == true) {
    return true;
  }

  // If you change anything here, make sure to change GetSecureUpperLimitTriggerThreshold !!!!
  double NoisedThreshold = 
    gRandom->Gaus(GetTriggerThreshold(MVector(0.0, 0.0, Point.GetPosition().Z())), 
                  m_TriggerThresholdSigma);

  if (Energy > NoisedThreshold) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetOverflow(const double Overflow)
{
  // Set the overflow

  m_Overflow = Overflow;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetOverflow() const
{
  // Return the overflow

  return m_Overflow;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetOverflowSigma(const double Avg)
{
  // Set the sigma value for the overflow bin

  m_OverflowSigma = Avg;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetOverflowSigma() const
{
  // Return the sigma value for the overflow bin

  return m_OverflowSigma;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetFailureRate(const double FailureRate)
{
  // Set the percentage [0..1] of not connected pixels

  m_FailureRate = FailureRate;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetFailureRate() const
{
  // Return the percentage [0..1] of not connected pixels

  return m_FailureRate;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetStructuralOffset(const MVector& Offset)
{
  // Set the Offset to the first sensitive volume from 
  // -x, -y, -z direction:

  //cout<<"Setting struct off for "<<m_Name<<endl;

  m_StructuralOffset = Offset;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDDetector::GetStructuralOffset() const
{
  // Return the Offset to the first sensitive volume from 
  // -x, -y, -z direction:
  
  return m_StructuralOffset;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetStructuralPitch(const MVector& Pitch)
{
  // Set the pitch between the sensitive volumes of this detector

  //cout<<"Setting struct pitch for "<<m_Name<<endl;
  m_StructuralPitch = Pitch;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDDetector::GetStructuralPitch() const 
{
  // Return the pitch between the sensitive volumes of this detector

  return m_StructuralPitch;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::AddSensitiveVolume(MDVolume *Volume)
{
  // Add a sensitive volume to this detector

  m_SVs.push_back(Volume);
  Volume->SetSensitiveVolumeID(m_SensIDCounter++);
  Volume->SetDetector(this);
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDDetector::GetSensitiveVolume(const unsigned int i) 
{
  //

  if (i < GetNSensitiveVolumes()) {
    return m_SVs[i];
  } else {
    mout<<"MDVolume* MDDetector::GetSensitiveVolumeAt(int i)"<<endl;
    mout<<"Index ("<<i<<") out of bounds (0, "<<GetNSensitiveVolumes()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDDetector::GetNSensitiveVolumes() const
{
  //

  return m_SVs.size();
}


////////////////////////////////////////////////////////////////////////////////


int MDDetector::GetGlobalNSensitiveVolumes() const
{
  //

  return m_SensIDCounter-1;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetEnergyLossMap(const MString& EnergyLossMap)
{
  // Use a energy loss based on a 3D energy loss map

  if (m_EnergyLossMap.Set(EnergyLossMap, "DP", MFunction3D::c_InterpolationLinear) == false) {
    m_EnergyLossType = c_EnergyLossTypeNone;
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Cannot read energy loss map!"<<endl;
  } else {
    m_EnergyLossType = c_EnergyLossTypeMap;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::SetEnergyResolutionType(const int EnergyResolutionType) 
{ 
  //! Set the energy resolution type, return false if you try to overwrite an existing type

  if (m_EnergyResolutionType == EnergyResolutionType) return true;

  if (m_EnergyResolutionType != c_EnergyResolutionTypeUnknown) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"You can only set an energy resolution type once!"<<endl;
    return false;
  }

  if (EnergyResolutionType == c_EnergyResolutionTypeIdeal ||
      EnergyResolutionType == c_EnergyResolutionTypeNone ||
      EnergyResolutionType == c_EnergyResolutionTypeGauss ||
      EnergyResolutionType == c_EnergyResolutionTypeLorentz ||
      EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
    m_EnergyResolutionType = EnergyResolutionType; 
  } else {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Unknown energy resolution type: "<<EnergyResolutionType<<endl;
    return false;    
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetEnergyResolution(const double InputEnergy, 
                                     const double Peak1, 
                                     const double Width1, 
                                     const double Peak2,
                                     const double Width2,
                                     const double Ratio)
{
  // Set the energy resolution

  if (InputEnergy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Input energy for energy resolution needs to be non-negative!"<<endl;
    return; 
  }
  if (Peak1 < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak for energy resolution needs to be positive!"<<endl;
    return; 
  }
  if (Width1 < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak width for energy resolution needs to be non-negative!"<<endl;
    return; 
  }
  if (Peak2 < 0 && Peak2 != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak for energy resolution needs to be positive!"<<endl;
    return; 
  }
  if (Width2 < 0 && Width2 != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak width for energy resolution needs to be non-negative!"<<endl;
    return; 
  }
  if ((Ratio < 0 || Ratio > 1) && Ratio != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Ratio for energy resolution needs to be between [0..1]!"<<endl;
    return; 
  }

  m_EnergyResolutionPeak1.Add(InputEnergy, Peak1);
  m_EnergyResolutionWidth1.Add(InputEnergy, Width1);
  if (Peak2 != g_DoubleNotDefined) {
    m_EnergyResolutionPeak2.Add(InputEnergy, Peak2);
  }
  if (Width2 != g_DoubleNotDefined) {
    m_EnergyResolutionWidth2.Add(InputEnergy, Width2);
  }
  if (Ratio != g_DoubleNotDefined) { 
    m_EnergyResolutionRatio.Add(InputEnergy, Ratio);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolutionWidth1(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  double Value = m_EnergyResolutionWidth1.Evaluate(Energy);
  if (Value < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution width 1 value below zero."<<endl;
    return 0.0;
  }
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolutionWidth2(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  double Value = m_EnergyResolutionWidth2.Evaluate(Energy);
  if (Value < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution width 2 value below zero."<<endl;
    return 0.0;
  }
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolutionPeak1(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  double Value = m_EnergyResolutionPeak1.Evaluate(Energy);
  if (Value < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution peak 1 value below zero."<<endl;
    return 0.0;
  }
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolutionPeak2(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  double Value = m_EnergyResolutionPeak2.Evaluate(Energy);
  if (Value < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution peak 2 value below zero."<<endl;
    return 0.0;
  }
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolutionRatio(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  double Value = m_EnergyResolutionRatio.Evaluate(Energy);
  if (Value < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution ratio value below zero."<<endl;
    return 0.0;
  }
  if (Value > 1.0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Give more energy resolution values, because the interpolation fails at E="<<Energy<<"keV"<<endl;
    mout<<"Energy resolution ratio value above one."<<endl;
    return 1.0;
  }
  return Value;
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetEnergyResolution(const double Energy, const MVector& PositionInDetector) const
{
  //! Returns an average energy resolution width

  if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
    return m_EnergyResolutionWidth1.Evaluate(Energy);
  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {
    return m_EnergyResolutionWidth1.Evaluate(Energy);
  } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
    mimp<<"   ***  Info  ***  in detector "<<m_Name<<endl;
    mimp<<"There is no good GetEnergyResolution-function for the Gauss-Landau distribution. Using gauss only..."<<endl;
    return m_EnergyResolutionWidth1.Evaluate(Energy);
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetEnergyCalibration(const MFunction& EnergyCalibration)
{
  // Set a energy calibration function

  m_EnergyCalibrationSet = true;
  m_EnergyCalibration = EnergyCalibration;
  if (m_EnergyCalibration.GetSize() > 0) {
    m_UseEnergyCalibration = true;
  } else {
    m_UseEnergyCalibration = false;   
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetTimeResolution(const double Energy, const double Sigma)
{
  // 

  m_TimeResolutionType = c_TimeResolutionTypeGauss;
  m_TimeResolution.Add(Energy, Sigma);
}


////////////////////////////////////////////////////////////////////////////////


double MDDetector::GetTimeResolution(const double Energy) const
{
  // 
  
  if (m_TimeResolutionType == c_TimeResolutionTypeNone) {
    return numeric_limits<double>::max()/1000;
  } else if (m_TimeResolutionType == c_TimeResolutionTypeIdeal) {
    return 0;
  } else if (m_TimeResolutionType == c_TimeResolutionTypeGauss) {
    return m_TimeResolution.Evaluate(Energy);
  } else {
    merr<<"Unknown time resolution type: "<<m_TimeResolutionType<<endl;
    return numeric_limits<double>::max()/1000;
  }
}


////////////////////////////////////////////////////////////////////////////////


MVector MDDetector::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  return m_StructuralSize;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::SetDetectorVolume(MDVolume* Volume)
{
  // Set the volume which represents this detector

  m_DetectorVolume = Volume;
  // The volume needs to know the original detector not the named one
  if (m_IsNamedDetector == false) {
    Volume->SetIsDetectorVolume(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MDVolume* MDDetector::GetDetectorVolume() 
{
  // Return the volume, which represents this detector

  return m_DetectorVolume;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::Validate()
{
  // Make sure everything is reasonable:

  if (m_DetectorVolume == 0 && m_SVs.size() == 1) {
    SetDetectorVolume(m_SVs[0]);
  }

  if (m_DetectorVolume == 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector has no detector volume!"<<endl;
    return false;
  }
  if (m_DetectorVolume->IsVirtual() == true) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector volume "<<m_DetectorVolume->GetName()<<" is not allowed to be virtual!"<<endl;
    return false;
  }
  if (m_DetectorVolume->IsClone() == true) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"A detector volume cannot be generated via <Template>.Copy <detector volume name>)!"<<endl;
    mout<<"You can position it several times or it can be part of a cloned mother volume, but it cannot be cloned itself."<<endl;
    return false;
  }

  if (m_SVs.size() == 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Detector has no sensitive volume!"<<endl;
    return false;
  }
  for (unsigned int i = 0; i < m_SVs.size(); ++i) {
    if (m_SVs[i]->IsVirtual() == true) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"The sensitive volume is not allowed to be virtual!"<<endl;
      return false;
    }
  }
  if (m_SVs.size() == 1 && m_CommonVolume == 0) {
    m_CommonVolume = m_DetectorVolume;
  }

  // In case we have only one sensitive volume and the sensitive volume is identical with the detector volume
  // we do not necessarily need a structural pitch and a structural offset
  if (m_DetectorVolume == m_SVs[0] || m_SVs.size() > 1) {
    if (m_StructuralPitch == g_VectorNotDefined) {
      m_StructuralPitch = MVector(0, 0, 0);
    }
    if (m_StructuralOffset == g_VectorNotDefined) {
      m_StructuralOffset = MVector(0, 0, 0);      
    }
  } else {
    if (m_StructuralPitch == g_VectorNotDefined) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"No spacing (keyword StructuralPitch) between the sensitive volumes defined"<<endl;
      return false;
    }
    if (m_StructuralOffset == g_VectorNotDefined) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"No offset (keyword StructuralOffset)  from the detector volume to the first sensitive volumes defined"<<endl;
      return false;
    }
  }

  if (m_StructuralPitch.X() < 0 || m_StructuralPitch.Y() < 0 || m_StructuralPitch.Z() < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"No component of the structural pitch is allowed to be negative: "<<m_StructuralPitch<<endl;
    return false;
  }
  if (m_StructuralOffset.X() < 0 || m_StructuralOffset.Y() < 0 || m_StructuralOffset.Z() < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"No component of the structural offset is allowed to be negative: "<<m_StructuralOffset<<endl;
    return false;
  }

  if (m_EnergyResolutionType == c_EnergyResolutionTypeUnknown) {
    mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    mout<<"No energy resolution defined --- assuming ideal"<<endl; 
    m_EnergyResolutionType = c_EnergyResolutionTypeIdeal; 
  }
  
  if (m_EnergyResolutionType != c_EnergyResolutionTypeIdeal && m_EnergyResolutionType != c_EnergyResolutionTypeNone) {
    if (m_EnergyResolutionPeak1.GetSize() < 2) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"Please give at least two data points for the energy resolution or state that you don't want an energy resolution."<<endl;
      return false;        
    }
    if (m_EnergyResolutionPeak1.GetSize() == 0) {
      mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
      mout<<"No energy resolution given. Assuming ideal."<<endl;
      mout<<"You might add a statement like \""<<m_Name<<".EnergyResolution Ideal\" to your file."<<endl;
      m_EnergyResolutionType = c_EnergyResolutionTypeIdeal;
    }
  }

  if (m_EnergyLossType == c_EnergyLossTypeUnknown) {
    m_EnergyLossType = c_EnergyLossTypeNone;
  }

  if (m_EnergyCalibrationSet == false) {
    m_UseEnergyCalibration = false; 
  }
  
  if (m_TimeResolutionType == c_TimeResolutionTypeUnknown) {
    m_TimeResolutionType = c_TimeResolutionTypeNone;
  }
  
  if (m_NoiseThresholdEqualsTriggerThresholdSet == false) {
    m_NoiseThresholdEqualsTriggerThresholdSet = true;
    m_NoiseThresholdEqualsTriggerThreshold = false;
  }
  
  if (m_NoiseThresholdEqualsTriggerThreshold == true) {
    if (m_NoiseThreshold != g_DoubleNotDefined) {
      mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
      mout<<"Ignoring noise threshold, because NoiseThresholdEqualsTriggerThreshold is set"<<endl; 
      m_NoiseThreshold = 0;
      m_NoiseThresholdSigma = 0;
    }
  }
  
  if (m_NoiseThreshold == g_DoubleNotDefined) {
    if (m_NoiseThresholdEqualsTriggerThreshold == false) {
      mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
      mout<<"No noise threshold defined --- setting it to zero"<<endl; 
    }
    m_NoiseThreshold = 0;
    m_NoiseThresholdSigma = 0;
  }
  
  if (m_NoiseThresholdSigma == g_DoubleNotDefined) {
    m_NoiseThresholdSigma = 0;
  }

  if (m_TriggerThreshold == g_DoubleNotDefined) {
    mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    mout<<"No trigger threshold defined --- setting it to zero"<<endl; 
    m_TriggerThreshold = 0;
    m_TriggerThresholdSigma = 0;
  }
  
  if (m_TriggerThresholdSigma == g_DoubleNotDefined) {
    m_TriggerThresholdSigma = 0;
  }
  
  if (m_FailureRate == g_DoubleNotDefined) {
    // mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    // mout<<"No failure rate defined --- setting it to zero"<<endl; 
    m_FailureRate = 0;    
  }
  
  if (m_Overflow == g_DoubleNotDefined) {
    // mout<<"   ***  Info  ***  for detector "<<m_Name<<endl;
    // mout<<"No overflow defined --- setting it to zero"<<endl; 
    m_Overflow = 10E+20;    
    m_OverflowSigma = 1;    
  }
  
  if (m_NoiseThresholdEqualsTriggerThreshold == true && 
      (m_NoiseThreshold != 0 || m_NoiseThresholdSigma != 0)) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Setting NoiseThresholdEqualsTriggerThreshold ignores all values of NoiseThreshold and its uncertainty"<<endl;
    return false;    
  }

  // The blocked trigger channel have 

  
  if (m_IsNamedDetector == true) {
    if (m_VolumeSequence.IsEmpty() == true) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"This named detector has no assigned volume/position (use Assign keyword)"<<endl;
      return false;    
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::CreateBlockedTriggerChannelsGrid()
{
  // Create the grid only if it is really used

  m_BlockedTriggerChannels.Set(MDGrid::c_Voxel, 1, 1);
  m_AreBlockedTriggerChannelsUsed = true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::AreNear(const MVector& Pos1, const MVector& dPos1, 
                         const MVector& Pos2, const MVector& dPos2, 
                         const double Sigma, const int Level) const
{
  //  

  mimp<<"If anybody sees this error message, then let me know... Andreas"<<show;

  if (fabs(Pos1[0] - Pos2[0])/(m_StructuralPitch.X() + 2*m_StructuralSize.X()) < 1.1*Level &&
      fabs(Pos1[1] - Pos2[1])/(m_StructuralPitch.Y() + 2*m_StructuralSize.Y()) < 1.1*Level) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::BlockTriggerChannel(const unsigned int xGrid, const unsigned int yGrid)
{
  // Block a channel from triggering
  
  if (m_AreBlockedTriggerChannelsUsed == false) {
    CreateBlockedTriggerChannelsGrid();
  }

  if (m_BlockedTriggerChannels.SetVoxelValue(1.0, xGrid, yGrid) == false) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Grid position: "<<xGrid<<" "<<yGrid<<" does not exist"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::ResetIDs()
{
  //

  MDDetector::m_IDCounter = 1;
  MDDetector::m_SensIDCounter = 1;
}


////////////////////////////////////////////////////////////////////////////////


void MDDetector::ActivateNoising(const bool ActivateNoising)
{
  // True if data is noised

  m_NoiseActive = ActivateNoising;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::IsVeto(const MVector& Pos, const double Energy) const
{
  // Check if we have a veto

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDDetector::AddNamedDetector(MDDetector* Detector)
{
  //! Add a named detector

  if (m_IsNamedDetector == true) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"This is already a named detector and you can not add a named detector to a named detector!"<<endl;
    return false;
  }
  
  Detector->m_IsNamedDetector = true; 
  Detector->m_NamedAfter = this; 
  m_NamedDetectors.push_back(Detector);
  
  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


bool MDDetector::HasNamedDetector(const MString& Name) const
{
  //! Return true if this detector contains the given named detector

  for (unsigned int d = 0; d < m_NamedDetectors.size(); ++d) {
    if (m_NamedDetectors[d]->m_Name == Name) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MDDetector::GetNamedDetectorName(unsigned int i) const
{
  //! Return the name of the "named detector"

  if (i > m_NamedDetectors.size()) {
    merr<<"Index for named detector name out of range: "<<i<<" ( you have "<<m_NamedDetectors.size()<<" named detectors!"<<show;
    return "";
  }

  return m_NamedDetectors[i]->GetName();
}


////////////////////////////////////////////////////////////////////////////////


MDVolumeSequence MDDetector::GetNamedDetectorVolumeSequence(unsigned int i)
{
  //! Return the volume sequence of the "named detector"

  if (i > m_NamedDetectors.size()) {
    merr<<"Index for named detector volume sequence out of range: "<<i<<" ( you have "<<m_NamedDetectors.size()<<" named detectors!"<<show;
    return MDVolumeSequence();
  }

  return m_NamedDetectors[i]->m_VolumeSequence;
}


////////////////////////////////////////////////////////////////////////////////


MDDetector* MDDetector::FindNamedDetector(const MDVolumeSequence& VS)
{
  // Find the named detector
 
  for (unsigned int d = 0; d < m_NamedDetectors.size(); ++d) {
    //cout<<"A: "<<VS.ToString()<<endl;
    //cout<<"B: "<<m_NamedDetectors[d]->m_VolumeSequence.ToString()<<endl;
    if (VS.HasSameDetector(m_NamedDetectors[d]->m_VolumeSequence) == true) {
      return m_NamedDetectors[d];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDDetector::GetGlobalPosition(const MVector& PositionInDetector, const MString& NamedDetector)
{
  //! Use this function to convert a position within a NAMED detector (i.e. uniquely identifyable) into a position in the global coordinate system

  MVector Position = g_VectorNotDefined;
  
  if (m_IsNamedDetector == true) {
    if (m_VolumeSequence.GetSensitiveVolume() == 0) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"The named detector's ("<<NamedDetector<<") volume sequence has no sensitive volume --- this should not have happened!"<<endl;
      return g_VectorNotDefined;
    }
    Position = m_VolumeSequence.GetPositionInFirstVolume(PositionInDetector, m_VolumeSequence.GetSensitiveVolume());
  } else {
    bool Found = false;
    for (unsigned int d = 0; d < m_NamedDetectors.size(); ++d) {
      if (m_NamedDetectors[d]->m_Name == NamedDetector) {
        if (m_NamedDetectors[d]->m_VolumeSequence.GetSensitiveVolume() == 0) {
          mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
          mout<<"The named detector's ("<<m_NamedDetectors[d]->m_Name<<") volume sequence has no sensitive volume --- this should not have happened!"<<endl;
          return g_VectorNotDefined;
        }
        Position = m_NamedDetectors[d]->m_VolumeSequence.GetPositionInFirstVolume(PositionInDetector, m_NamedDetectors[d]->m_VolumeSequence.GetSensitiveVolume());
        Found = true;
      }
    } 

    if (Found == false) {
      mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
      mout<<"Named detector not found: "<<NamedDetector<<endl;
    }
  }

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


MString MDDetector::ToString() const
{
  ostringstream out;

  if (m_IsNamedDetector == false) {
    out<<"Detector \"";
  } else {
    out<<"Named detector (named after: "<<m_NamedAfter->m_Name<<") \"";
  }
  out<<m_Name<<"\" of type "<<GetDetectorTypeName(m_Type)<<endl;
  out<<"   with detector volume: "<<m_DetectorVolume->GetName()<<endl;  
  out<<"   with sensitive volumes: ";  
  for (unsigned int i = 0; i < m_SVs.size(); i++) {
    out<<m_SVs[i]->GetName()<<" ";
  }
  out<<endl;
  out<<"   energy resolution type: "<<m_EnergyResolutionType<<endl;
  if (HasNamedDetectors() == true) {
    out<<"   with "<<GetNNamedDetectors()<<" named detectors: ";
    for (unsigned int d = 0; d < m_NamedDetectors.size(); ++d) {
      out<<m_NamedDetectors[d]->GetName()<<" ";
    }
    out<<endl;
	}
  
  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDDetector::GetGeomegaCommon(bool PrintVolumes,
                                     bool PrintStructural,
                                     bool PrintEnergyResolution,
                                     bool PrintTimeResolution,
                                     bool PrintTriggerThreshold,
                                     bool PrintNoiseThreshold,
                                     bool PrintOverflow,
                                     bool PrintFailureRate,
                                     bool PrintPulseShape) const
{
  // Return all common detector characteristics in Geomega-Format
  // Function is called by GetGeomega() of derived classes
  
  ostringstream out;

  if (PrintVolumes == true) {
    out<<m_Name<<".DetectorVolume "<<m_DetectorVolume->GetName()<<endl;
    for (unsigned int s = 0; s < m_SVs.size(); ++s) {
      out<<m_Name<<".SensitiveVolume "<<m_SVs[s]->GetName()<<endl;
    }
  }
  if (PrintStructural == true) {
    out<<m_Name<<".StructuralPitch "
       <<m_StructuralPitch.X()<<" "
       <<m_StructuralPitch.Y()<<" "
       <<m_StructuralPitch.Z()<<endl;
    out<<m_Name<<".StructuralOffset "
       <<m_StructuralOffset.X()<<" "
       <<m_StructuralOffset.Y()<<" "
       <<m_StructuralOffset.Z()<<endl;
  }
  if (PrintEnergyResolution == true) {
    if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
      out<<m_Name<<".EnergyResolutionType Gauss"<<endl;
    } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {
      out<<m_Name<<".EnergyResolutionType Lorentz "<<endl;
    }
    for (unsigned int d = 0; d < m_EnergyResolutionPeak1.GetNDataPoints(); ++d) {
      out<<m_Name<<".EnergyResolution ";
      if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
        out<<"Gauss "<<
          m_EnergyResolutionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionWidth1.GetDataPointY(d)<<endl;
      } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {
        out<<"Lorentz "<<
          m_EnergyResolutionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionWidth1.GetDataPointY(d)<<endl;
      } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
        out<<"GaussLandau "<<
          m_EnergyResolutionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionWidth1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionPeak2.GetDataPointY(d)<<" "<<
          m_EnergyResolutionWidth2.GetDataPointY(d)<<" "<<
          m_EnergyResolutionRatio.GetDataPointY(d)<<endl;
      }
    }
  }
  if (PrintTimeResolution == true) {
    for (int d = 0; d < m_TimeResolution.GetSize(); ++d) {
      out<<m_Name<<".TimeResolution "<<
        m_TimeResolution.GetDataPointX(d)<<" "<<
        m_TimeResolution.GetDataPointY(d)<<endl;
    }
  }
  if (PrintTriggerThreshold == true) {    out<<m_Name<<".TriggerThreshold "<<m_TriggerThreshold<<" "<<m_TriggerThresholdSigma<<endl;
  }
  if (PrintNoiseThreshold == true) {
    if (m_NoiseThresholdEqualsTriggerThreshold == true) {
      out<<m_Name<<".NoiseThresholdEqualsTriggerThreshold"<<endl;
    } else {
      out<<m_Name<<".NoiseThreshold "<<m_NoiseThreshold<<" "<<m_NoiseThresholdSigma<<endl;
    }
  }
  if (PrintOverflow == true) {
    out<<m_Name<<".Overflow "<<m_Overflow<<" "<<m_OverflowSigma<<endl;
  }
  if (PrintFailureRate == true) {
    out<<m_Name<<".FailureRate "<<m_FailureRate<<endl;
  }
  if (PrintPulseShape == true) {
    if (m_PulseShape != 0) {
      out<<m_Name<<".PulseShape "<<
        m_PulseShape->GetParameter(0)<<" "<<
        m_PulseShape->GetParameter(1)<<" "<<
        m_PulseShape->GetParameter(2)<<" "<<
        m_PulseShape->GetParameter(3)<<" "<<
        m_PulseShape->GetParameter(4)<<" "<<
        m_PulseShape->GetParameter(5)<<" "<<
        m_PulseShape->GetParameter(6)<<" "<<
        m_PulseShape->GetParameter(7)<<" "<<
        m_PulseShape->GetParameter(8)<<" "<<
        m_PulseShape->GetParameter(9)<<" "<<
        m_PulseShapeMin<<" "<<
        m_PulseShapeMin<<endl;
    }
  }

  return out.str().c_str();  
}

// MDDetector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
