/*
 * MDStrip3D.cxx
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
// MDStrip3D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDStrip3D.h"

// Standard libs:
#include <limits>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDStrip3D)
#endif


////////////////////////////////////////////////////////////////////////////////


MDStrip3D::MDStrip3D(MString Name) : MDStrip2D(Name)
{
  // Construct an instance of MDStrip3D

  m_Type = c_Strip3D;
  m_Description = c_Strip3DName;

  m_DepthResolutionType = c_DepthResolutionTypeUnknown;
  m_DepthResolutionThreshold = g_DoubleNotDefined;
  
  m_EnergyResolutionDepthCorrectionSet = false;
  m_TriggerThresholdDepthCorrectionSet = false;
  m_NoiseThresholdDepthCorrectionSet = false;
  
  m_DriftConstant = g_DoubleNotDefined; // No opening angle for the drift
  m_EnergyPerElectron = g_DoubleNotDefined; //kev: Not used if no opening angle is defined
}


////////////////////////////////////////////////////////////////////////////////


MDStrip3D::MDStrip3D(const MDStrip3D& S) : MDStrip2D(S)
{
  m_DepthResolutionType = S.m_DepthResolutionType;
  m_DepthResolution = S.m_DepthResolution; 
  m_DepthResolutionThreshold = S.m_DepthResolutionThreshold; 
  m_DepthResolutionSigma = S.m_DepthResolutionSigma; 
  
  m_EnergyResolutionDepthCorrectionSet = S.m_EnergyResolutionDepthCorrectionSet; 
  m_EnergyResolutionDepthCorrectionPeak1 = S.m_EnergyResolutionDepthCorrectionPeak1; 
  m_EnergyResolutionDepthCorrectionWidth1 = S.m_EnergyResolutionDepthCorrectionWidth1; 
  m_EnergyResolutionDepthCorrectionPeak2 = S.m_EnergyResolutionDepthCorrectionPeak2; 
  m_EnergyResolutionDepthCorrectionWidth2 = S.m_EnergyResolutionDepthCorrectionWidth2; 
  m_EnergyResolutionDepthCorrectionRatio = S.m_EnergyResolutionDepthCorrectionRatio;

  m_TriggerThresholdDepthCorrectionSet = S.m_TriggerThresholdDepthCorrectionSet;
  m_TriggerThresholdDepthCorrection = S.m_TriggerThresholdDepthCorrection;
  m_NoiseThresholdDepthCorrectionSet = S.m_NoiseThresholdDepthCorrectionSet;
  m_NoiseThresholdDepthCorrection = S.m_NoiseThresholdDepthCorrection;
  
  m_DriftConstant = S.m_DriftConstant;
  m_EnergyPerElectron = S.m_EnergyPerElectron;
}

 
////////////////////////////////////////////////////////////////////////////////


MDDetector* MDStrip3D::Clone()
{
  // Duplicate this detector

  massert(this != 0);
  return new MDStrip3D(*this);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::CopyDataToNamedDetectors()
{
  //! Copy data to named detectors

  MDStrip2D::CopyDataToNamedDetectors();
  
  if (m_IsNamedDetector == true) return true;
  
  for (unsigned int n = 0; n < m_NamedDetectors.size(); ++n) {
    if (dynamic_cast<MDStrip3D*>(m_NamedDetectors[n]) == 0) {
      mout<<"   ***  Internal error  ***  in detector "<<m_Name<<endl;
      mout<<"We have a named detector ("<<m_NamedDetectors[n]->GetName()<<") which is not of the same type as the base detector!"<<endl;
      return false;
    }
    MDStrip3D* D = dynamic_cast<MDStrip3D*>(m_NamedDetectors[n]);
  
    if (D->m_DepthResolutionType == c_DepthResolutionTypeUnknown && 
        m_DepthResolutionType != c_DepthResolutionTypeUnknown) {
      D->m_DepthResolutionType = m_DepthResolutionType;
      D->m_DepthResolution = m_DepthResolution;
      D->m_DepthResolutionSigma = m_DepthResolutionSigma;
    }
  
  
    if (D->m_DepthResolutionThreshold == g_DoubleNotDefined && 
        m_DepthResolutionThreshold != g_DoubleNotDefined) {
      D->m_DepthResolutionThreshold = m_DepthResolutionThreshold; 
    }
  
    if (D->m_EnergyResolutionDepthCorrectionSet == false && 
        m_EnergyResolutionDepthCorrectionSet == true) {
      D->m_EnergyResolutionDepthCorrectionSet = true;
      D->m_EnergyResolutionDepthCorrectionPeak1 = m_EnergyResolutionDepthCorrectionPeak1; 
      D->m_EnergyResolutionDepthCorrectionWidth1 = m_EnergyResolutionDepthCorrectionWidth1; 
      D->m_EnergyResolutionDepthCorrectionPeak2 = m_EnergyResolutionDepthCorrectionPeak2; 
      D->m_EnergyResolutionDepthCorrectionWidth2 = m_EnergyResolutionDepthCorrectionWidth2; 
      D->m_EnergyResolutionDepthCorrectionRatio = m_EnergyResolutionDepthCorrectionRatio;
    }

    if (D->m_TriggerThresholdDepthCorrectionSet == false && 
        m_TriggerThresholdDepthCorrectionSet == true) {
      D->m_TriggerThresholdDepthCorrectionSet = true;
      D->m_TriggerThresholdDepthCorrection = m_TriggerThresholdDepthCorrection;
    }
    if (D->m_TriggerThresholdDepthCorrectionSet == false && 
        m_TriggerThresholdDepthCorrectionSet == true) {
      D->m_NoiseThresholdDepthCorrectionSet = true;
      D->m_NoiseThresholdDepthCorrection = m_NoiseThresholdDepthCorrection; 
    }
    
    if (D->m_DriftConstant == g_DoubleNotDefined && m_DriftConstant != g_DoubleNotDefined) {
      D->m_DriftConstant = m_DriftConstant;
    }
      
    if (D->m_EnergyPerElectron == g_DoubleNotDefined && m_EnergyPerElectron != g_DoubleNotDefined) {
      D->m_EnergyPerElectron = m_EnergyPerElectron;
    }
  }

  return true;
}
    

////////////////////////////////////////////////////////////////////////////////


MDStrip3D::~MDStrip3D()
{
  // Delete this instance of MDStrip3D
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::SetEnergyResolutionDepthCorrection(const double InputDepth, 
                                                   const double Peak1, 
                                                   const double Width1, 
                                                   const double Peak2, 
                                                   const double Width2, 
                                                   const double Ratio)
{
  // Add an depth correction factor to the energy resolution
  
  if (Peak1 <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak correction for energy resolution needs to be positive!"<<endl;
    return false; 
  }
  if (Width1 < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak correction width for energy resolution needs to be non-negative!"<<endl;
    return false; 
  }
  if (Peak2 <= 0 && Peak2 != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak 2 correction for energy resolution needs to be positive!"<<endl;
    return false; 
  }
  if (Width2 < 0 && Width2 != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy peak 2 correction width for energy resolution needs to be non-negative!"<<endl;
    return false; 
  }
  if ((Ratio < 0 || Ratio > 1) && Ratio != g_DoubleNotDefined) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Ratio correction for energy resolution needs to be between [0..1]!"<<endl;
    return false; 
  }
  
  m_EnergyResolutionDepthCorrectionPeak1.Add(InputDepth, Peak1);
  m_EnergyResolutionDepthCorrectionWidth1.Add(InputDepth, Width1);
  if (Peak2 != g_DoubleNotDefined) {
    m_EnergyResolutionDepthCorrectionPeak2.Add(InputDepth, Peak2);
  }
  if (Width2 != g_DoubleNotDefined) {
    m_EnergyResolutionDepthCorrectionWidth2.Add(InputDepth, Width2);
  }
  if (Ratio != g_DoubleNotDefined) { 
    m_EnergyResolutionDepthCorrectionRatio.Add(InputDepth, Ratio);
  }

  m_EnergyResolutionDepthCorrectionSet = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolutionWidth1(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    return m_EnergyResolutionWidth1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth1.Evaluate(Position[2]);
  }

  return MDDetector::GetEnergyResolutionWidth1(Energy, Position);
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolutionWidth2(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    return m_EnergyResolutionWidth2.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth2.Evaluate(Position[2]);
  }
  
  return MDDetector::GetEnergyResolutionWidth2(Energy, Position);
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolutionPeak1(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    return m_EnergyResolutionPeak1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionPeak1.Evaluate(Position[2]);
  }

  return MDDetector::GetEnergyResolutionPeak1(Energy, Position);
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolutionPeak2(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    return m_EnergyResolutionPeak2.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionPeak2.Evaluate(Position[2]);
  }

  return MDDetector::GetEnergyResolutionPeak2(Energy, Position);
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolutionRatio(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    return m_EnergyResolutionRatio.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionRatio.Evaluate(Position[2]);
  }

  return MDDetector::GetEnergyResolutionRatio(Energy, Position);
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetEnergyResolution(const double Energy, const MVector& Position) const
{
  // Some detectors have a depth dependend energy resolution (e.g. Strip 3D ), 
  // but not the default energy resolution handler here:

  if (HasEnergyResolutionDepthCorrection() == true) {
    if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
      return m_EnergyResolutionWidth1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth1.Evaluate(Position[2]);
    } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {
      return m_EnergyResolutionWidth1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth1.Evaluate(Position[2]);
    } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
      return sqrt(m_EnergyResolutionRatio.Evaluate(Energy)*m_EnergyResolutionWidth1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth1.Evaluate(Position[2])*
                  m_EnergyResolutionRatio.Evaluate(Energy)*m_EnergyResolutionWidth1.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth1.Evaluate(Position[2]) +
                  (1-m_EnergyResolutionRatio.Evaluate(Energy))*m_EnergyResolutionWidth2.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth2.Evaluate(Position[2])*
                  (1-m_EnergyResolutionRatio.Evaluate(Energy))*m_EnergyResolutionWidth2.Evaluate(Energy)*m_EnergyResolutionDepthCorrectionWidth2.Evaluate(Position[2]));
    }
  } else {
    return MDDetector::GetEnergyResolution(Energy, Position);
  }

  return 0.0;
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::HasEnergyResolutionDepthCorrection() const
{
  //! Return true if this detector has a depth dependent energy resolution:

  if (m_EnergyResolutionDepthCorrectionPeak1.GetNDataPoints() > 0) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::SetDepthResolutionThreshold(const double DepthResolutionThreshold)
{
  // Set a threshold in the depth resolution: Below this value, no depth resolution exists
  
  m_DepthResolutionThreshold = DepthResolutionThreshold;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::SetDepthResolutionAt(const double Energy, 
                                     const double Resolution, 
                                     const double Sigma)
{
  // Set a variable depth resolution

  if (Energy < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Energy for depth resolution needs to be non-negative!"<<endl;
    return false; 
  }
  if (Resolution <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Depth resolution needs to be positive!"<<endl;
    return false; 
  }
  if (Sigma < 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Sigma of depth resolution needs to be non-negative!"<<endl;
    return false; 
  }

  m_DepthResolutionType = c_DepthResolutionTypeGauss;
  m_DepthResolution.Add(Energy, Resolution);
  m_DepthResolutionSigma.Add(Energy, Sigma);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::SetTriggerThresholdDepthCorrection(const double Depth, const double Correction)
{
  // Add an depth correction factor to the trigger threshold

  if (Correction <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Depth correction factor for depth dependent trigger threshold needs to be larger than zero!"<<endl;
    return false; 
  }

  m_TriggerThresholdDepthCorrection.Add(Depth, Correction);

  m_TriggerThresholdDepthCorrectionSet = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetTriggerThresholdDepthCorrection(const double Depth) const
{
  //! Return the depth correction factor to the trigger threshold

  return m_TriggerThresholdDepthCorrection.Evaluate(Depth);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::HasTriggerThresholdDepthCorrection() const
{
  //! Return true if this detector has a depth dependent trigger threshold:

  if (m_TriggerThresholdDepthCorrection.GetSize() > 0) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetTriggerThreshold(const MVector& Position) const
{
  //! Return the trigger threshold at an certain energy, including potential depth corrections (depth: 1.0 (top) ... 0.0 (bottom))

  if (m_TriggerThresholdDepthCorrection.GetSize() > 0) {
    return MDDetector::GetTriggerThreshold()*m_TriggerThresholdDepthCorrection.Evaluate(Position.Z());
  } else {
    return MDDetector::GetTriggerThreshold();
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetSecureUpperLimitTriggerThreshold() const
{
  // In ANY case the real trigger threshold is below this value:
  // We cannot guarantee any limit here, so:
  // (and make sure fortran can read it...)

  if (m_TriggerThresholdDepthCorrection.GetSize() > 0) {
    return c_LargestEnergy;
  } else {
    return MDDetector::GetSecureUpperLimitTriggerThreshold();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::SetNoiseThresholdDepthCorrection(const double Depth, const double Correction)
{
  // Add an depth correction factor to the noise threshold

  if (Correction <= 0) {
    mout<<"   ***  Error  ***  in detector "<<m_Name<<endl;
    mout<<"Depth correction factor for depth dependent noise threshold needs to be larger than zero!"<<endl;
    return false; 
  }

  m_NoiseThresholdDepthCorrection.Add(Depth, Correction);
  m_NoiseThresholdDepthCorrectionSet = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetNoiseThresholdDepthCorrection(const double Depth) const
{
  //! Return the depth correction factor to the noise threshold

  return m_NoiseThresholdDepthCorrection.Evaluate(Depth);
}


////////////////////////////////////////////////////////////////////////////////


bool MDStrip3D::HasNoiseThresholdDepthCorrection() const
{
  //! Return true if this detector has a depth dependent noise threshold:

  if (m_NoiseThresholdDepthCorrection.GetSize() > 0) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


double MDStrip3D::GetNoiseThreshold(const MVector& Position) const
{
  //! Return the noise threshold at an certain energy, including potential depth corrections (depth: 1.0 (top) ... 0.0 (bottom))

  if (m_NoiseThresholdDepthCorrection.GetSize() > 0) {
    return MDDetector::GetNoiseThreshold()*m_NoiseThresholdDepthCorrection.Evaluate(Position.Z());
  } else {
    return MDDetector::GetNoiseThreshold();
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MDGridPoint> MDStrip3D::Grid(const MVector& PosInDetectorVolume, const double& Energy, const double& Time, const MDVolume* Volume) const
{
  // Apply charge sharing and return a set of new grid points

  vector<MDGridPoint> Collection;
  
  if (m_DriftConstant == g_DoubleNotDefined || m_DriftConstant == 0) {
    MDGridPoint P = GetGridPoint(PosInDetectorVolume);
    P.SetEnergy(Energy);
    P.SetTime(Time);
    Collection.push_back(P);
    return Collection; 
  }
  
  
  MVector Pos = PosInDetectorVolume;
  
  // Translate into sensitive volume
  Pos += m_StructuralDimension;
  Pos -= m_StructuralOffset;
  int xWafer = int(Pos.X()/(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  int yWafer = int(Pos.Y()/(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  
  Pos.SetX(Pos.X() - xWafer*(2*m_StructuralSize.X()+m_StructuralPitch.X()));
  Pos.SetY(Pos.Y() - yWafer*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));
  Pos -= m_StructuralSize;
  
  // Ignore the hit if it is outside the sensitive part (guard ring?):
  if (fabs(Pos.X()) > m_WidthX/2.0 - m_OffsetX || fabs(Pos.Y()) > m_WidthY/2.0 - m_OffsetY) {
    if (fabs(Pos.X()) > m_WidthX/2.0 || fabs(Pos.Y()) > m_WidthY/2.0) {
      merr<<"Hit outside detector! ("<<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    } else {
      mdebug<<"Hit in guard ring: ("<<Pos[0]<<", "<<Pos[1]<<", "<<Pos[2]<<")"<<endl;
    }
    return Collection;
  }
  
  
  // The drift:
  
  // Split the step into "electrons"
  int NElectrons = int(Energy/m_EnergyPerElectron);
  if (NElectrons == 0 || m_DriftConstant == 0) NElectrons = 1;
  double EnergyPerElectron = Energy/NElectrons;
  
  // Calculate the drift parameters
  double DriftLength = Pos.Z() + m_StructuralSize.Z();
  if (DriftLength < 0) {
    merr<<"ERROR: DriftLength smaller than 0: "<<DriftLength<<" Setting it to 0"<<endl;
    DriftLength = 0;
  }
  double DriftRadiusSigma = m_DriftConstant * sqrt(DriftLength);
  
  cout<<"Drift radius sigma: "<<DriftRadiusSigma<<endl;
  
  // double DriftRadius = 0;
  // double DriftAngle = 0;
  double DriftX = 0;
  double DriftY = 0;
  
  //cout<<"Electron with: "<<Energy<<"!"<<NElectrons<<"!"<<m_EnergyPerElectron<<endl;
  for (int e = 0; e < NElectrons; ++e) {
    
    // Randomize x, y position:
    
    // Original but much too narrow distribution:
    // DriftRadius = gRandom->Gaus(0, DriftRadiusSigma);
    // DriftAngle = gRandom->Rndm() * c_Pi;
    // DriftX = DriftRadius*cos(DriftAngle);
    // DriftY = DriftRadius*sin(DriftAngle);
    
    // Real 2D Gaussian ("Rannor" gives 1 sigma distributions):
    gRandom->Rannor(DriftX, DriftY);
    DriftX *= DriftRadiusSigma;
    DriftY *= DriftRadiusSigma;
    
    MVector DriftPosition = Pos + MVector(DriftX, DriftY, 0);
    if (fabs(DriftPosition.X()) > m_WidthX/2 - m_OffsetX || 
      fabs(DriftPosition.Y()) > m_WidthY/2 - m_OffsetY) {
      //mout<<"Energy lost at "<<DriftPosition<<": "<<m_WidthX/2<<", "<<m_WidthY/2<<endl;
      continue;
      }
      
      int xStrip;
    if (m_NStripsX == 1 || m_PitchX == 0) {
      xStrip = 0;
    } else {
      xStrip = (int) ((DriftPosition.X() + m_WidthX/2 - m_OffsetX)/m_PitchX);
    }
    
    int yStrip;
    if (m_NStripsY == 1 || m_PitchY == 0) {  
      yStrip = 0;
    } else {
      yStrip = (int) ((DriftPosition.Y() + m_WidthY/2 - m_OffsetY)/m_PitchY);
    }
    
    // Check if we have a reasonable strip:
    if (xStrip < 0 || xStrip >= m_NStripsX) {
      merr<<"Invalid x-strip number: "<<xStrip<<endl
      <<"   Position was "<<DriftPosition<<endl;
      continue;
    }
    if (yStrip < 0 || yStrip >= m_NStripsY) {
      merr<<"Invalid y-strip number: "<<yStrip<<endl
      <<"   Position was "<<DriftPosition<<endl;
      continue;
    }
    
    MDGridPoint P(xStrip+xWafer*m_NStripsX, 
                  yStrip+yWafer*m_NStripsY, 
                  0,
                  MDGridPoint::c_Voxel,
                  MVector(0.0, 0.0, Pos.Z()), 
                  EnergyPerElectron, 
                  Time);
    Collection.push_back(P);
  }
  
  return Collection;
}


////////////////////////////////////////////////////////////////////////////////
  
  
void MDStrip3D::Noise(MVector& Pos, double& Energy, double& Time, MDVolume* Volume) const
  {
    // Noise Position and energy of this hit:
  
  if (m_NoiseActive == false) return;

  bool IsOverflow = false;

  // Test for failure:
  if (gRandom->Rndm() < m_FailureRate) {
    Energy = 0;
    return;
  }

  // Noise:
  ApplyEnergyResolution(Energy, Pos);

  // Overflow:
  IsOverflow = ApplyOverflow(Energy);

  // Noise threshold:
  //if (ApplyNoiseThreshold(Energy, Pos) == true) {
  //  Pos[2] = 0.0; 
  // return;
  //} 
  
  // Noise depth:
  if (IsOverflow == true || m_DepthResolution.GetNDataPoints() == 0 || Energy < m_DepthResolutionThreshold) {
    Pos[2] = 0.0;     
  } else {
    // Step 1: Determine the resolution:
    int Trials = 5;
    double Sigma;
    do {
      Sigma = gRandom->Gaus(m_DepthResolution.Evaluate(Energy), 
                            m_DepthResolutionSigma.Evaluate(Energy));
      Trials--;
    } while (Sigma < 0 && Trials >= 0);
    if (Trials <= 0) {
      Sigma = m_DepthResolution.Evaluate(Energy);
    }

    // Step 2: Determine the position
    double z = numeric_limits<double>::max();
    while (z < -m_StructuralSize[2] || z > m_StructuralSize[2]) {
      z = gRandom->Gaus(Pos[2], Sigma);
    }
    Pos[2] = z;
  }

  // Noise the time:
  ApplyTimeResolution(Time, Energy);

  // Now calibrate
  ApplyEnergyCalibration(Energy);
}


////////////////////////////////////////////////////////////////////////////////


MDGridPoint MDStrip3D::GetGridPoint(const MVector& PosInDetector) const
{
  // Discretize Pos to a voxel of this volume

  MDGridPoint P = MDStrip2D::GetGridPoint(PosInDetector);
  MVector PositionWithCorrectDepth = P.GetPosition();
  PositionWithCorrectDepth.SetZ(PosInDetector.Z());
  P.SetPosition(PositionWithCorrectDepth);

  return P;
}


////////////////////////////////////////////////////////////////////////////////


MVector MDStrip3D::GetPositionInDetectorVolume(const unsigned int xGrid, 
                                               const unsigned int yGrid,
                                               const unsigned int zGrid,
                                               const MVector PositionInGrid,
                                               const unsigned int Type,
                                               const MDVolume* Volume) const
{
  // Return the position in the detector volume

  int xWafer = int(xGrid/m_NStripsX);
  int yWafer = int(yGrid/m_NStripsY);

  MVector Position;

  // Position in Wafer relative to its center:
  Position.SetX(-m_WidthX/2 + m_OffsetX + (xGrid - xWafer*m_NStripsX + 0.5)*m_PitchX);
  Position.SetY(-m_WidthY/2 + m_OffsetY + (yGrid - yWafer*m_NStripsY + 0.5)*m_PitchY);
  Position.SetZ(PositionInGrid.Z());

  // Position in Wafer realitive to its negative edge:
  Position += m_StructuralSize;

  // Position in Detector relative to lower edge of detector at (-x, -y, -z)
  Position.SetX(Position.X() + xWafer*(2*m_StructuralSize.X()+m_StructuralPitch.X()));  
  Position.SetY(Position.Y() + yWafer*(2*m_StructuralSize.Y()+m_StructuralPitch.Y()));  

  // Position in Detector realitive to its negative edge: 
  Position += m_StructuralOffset;

  // Position in detector relaitive to its center:
  Position -= m_StructuralDimension;

  // The depth always was in detector coordinates!
  Position.SetZ(PositionInGrid.Z());

  return Position;
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip3D::GetGeomega() const
{
  // Return all detector characteristics in Geomega-Format

  ostringstream out;

  MString Strip2D = MDStrip2D::GetGeomega();
  Strip2D.ReplaceAll("Strip2D", "Strip3D");

  out<<Strip2D;
  for (unsigned int d = 0; d < m_DepthResolution.GetNDataPoints(); ++d) {
    out<<m_Name<<".DepthResolution "<<
      m_DepthResolution.GetDataPointX(d)<<" "<<
      m_DepthResolution.GetDataPointY(d)<<" "<<
      m_DepthResolutionSigma.GetDataPointY(d)<<endl;
  }
  out<<m_Name<<".DepthResolutionThreshold "<<m_DepthResolutionThreshold<<endl;
  if (HasEnergyResolutionDepthCorrection() == true) {
    for (unsigned int d = 0; d < m_EnergyResolutionDepthCorrectionPeak1.GetNDataPoints(); ++d) {
      out<<m_Name<<".EnergyResolutionDepthCorrection ";
      if (m_EnergyResolutionType == c_EnergyResolutionTypeGauss) {
        out<<"Gauss "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionWidth1.GetDataPointY(d)<<endl;
      } else if (m_EnergyResolutionType == c_EnergyResolutionTypeLorentz) {
        out<<"Lorentz "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionWidth1.GetDataPointY(d)<<endl;
      } else if (m_EnergyResolutionType == c_EnergyResolutionTypeGaussLandau) {
        out<<"GaussLandau "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointX(d)<<" "<<
          m_EnergyResolutionDepthCorrectionPeak1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionWidth1.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionPeak2.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionWidth2.GetDataPointY(d)<<" "<<
          m_EnergyResolutionDepthCorrectionRatio.GetDataPointY(d)<<endl;
      }
    }
  }
  if (HasTriggerThresholdDepthCorrection() == true) {
    for (unsigned int d = 0; d < m_TriggerThresholdDepthCorrection.GetNDataPoints(); ++d) {
      out<<m_Name<<".TriggerThresholdDepthCorrection "<<
        m_TriggerThresholdDepthCorrection.GetDataPointX(d)<<" "<<
        m_TriggerThresholdDepthCorrection.GetDataPointY(d)<<endl;
    }    
  }
  if (HasNoiseThresholdDepthCorrection() == true) {
    for (unsigned int d = 0; d < m_NoiseThresholdDepthCorrection.GetNDataPoints(); ++d) {
      out<<m_Name<<".NoiseThresholdDepthCorrection "<<
        m_NoiseThresholdDepthCorrection.GetDataPointX(d)<<" "<<
        m_NoiseThresholdDepthCorrection.GetDataPointY(d)<<endl;
    }    
  }

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MString MDStrip3D::ToString() const
{
  //

  ostringstream out;

  out<<MDStrip2D::ToString()<<endl;

  return out.str().c_str();  
}


////////////////////////////////////////////////////////////////////////////////


MVector MDStrip3D::GetPositionResolution(const MVector& Pos, const double Energy) const
{
  // Return the position resolution at position Pos

  if (m_DepthResolution.GetSize() != 0) {
    return MVector(m_PitchX/sqrt(12.0), m_PitchY/sqrt(12.0), m_DepthResolution.Evaluate(Energy));
  } else {
    return MDStrip2D::GetPositionResolution(Pos, Energy);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Check if all input is reasonable
bool MDStrip3D::Validate()
{
  if (MDStrip2D::Validate() == false) {
    return false;
  }

  if (m_DepthResolution.GetSize() == 0) {
    mout<<"   ***  Info  ***  in detector "<<m_Name<<endl;
    mout<<"You have not defined a depth resolution, so I assume we don't have one..."<<endl;
    m_DepthResolutionType = c_DepthResolutionTypeNone;
    m_DepthResolutionThreshold = 0;
  }

  if (m_DepthResolutionType == c_DepthResolutionTypeUnknown) {
    m_DepthResolutionType = c_DepthResolutionTypeNone;
    m_DepthResolutionThreshold = 0;
  }
       
  if (m_DepthResolutionThreshold == g_DoubleNotDefined) {
    m_DepthResolutionThreshold = 0;
  }
  
  if (m_DriftConstant == g_DoubleNotDefined) {
    m_DriftConstant = 0;
  }
  
  if (m_EnergyPerElectron == g_DoubleNotDefined) {
    m_EnergyPerElectron = 0.022; //keV
  }
  
  return true;
}


// MDStrip3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
