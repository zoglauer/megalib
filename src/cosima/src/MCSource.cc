/*
 * MCSource.cxx
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


// Cosima:
#include "MCCommon.hh"
#include "MCSource.hh"
#include "MCSteppingAction.hh"
#include "MCRunManager.hh"
#include "MCDetectorConstruction.hh"
#include "MCActivator.hh"

// Stdlib:
#include <limits>
using namespace std;

// Geant4:
#include "globals.hh"
#include "Randomize.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"

// ROOT:
#include "TRandom.h"
#include "TSystem.h"

// MEGAlib:
#include "MStreams.h"
#include "MAssert.h"


/******************************************************************************/


const int MCSource::c_Map                                          = 0; 

const int MCSource::c_Monoenergetic                                = 1; 
const int MCSource::c_Linear                                       = 2; 
const int MCSource::c_PowerLaw                                     = 3; 
const int MCSource::c_BrokenPowerLaw                               = 4; 
const int MCSource::c_Gaussian                                     = 5; 
const int MCSource::c_ThermalBremsstrahlung                        = 6; 
const int MCSource::c_BlackBody                                    = 7; 
const int MCSource::c_BandFunction                                 = 8; 
const int MCSource::c_FileDifferentialFlux                         = 9;
const int MCSource::c_Activation                                   = 10; 
const int MCSource::c_NormalizedEnergyBeamFluxFunction             = 11;

const int MCSource::c_StartAreaUnknown                             = 0;
const int MCSource::c_StartAreaSphere                              = 1;
const int MCSource::c_StartAreaTube                                = 2;

const int MCSource::c_FarField                                     = 1;
const int MCSource::c_NearField                                    = 2;

const int MCSource::c_FarFieldPoint                                = 1;
const int MCSource::c_FarFieldArea                                 = 2;
const int MCSource::c_FarFieldGaussian                             = 3;
const int MCSource::c_FarFieldFileZenithDependent                  = 4;
const int MCSource::c_FarFieldNormalizedEnergyBeamFluxFunction     = 5;

const int MCSource::c_NearFieldPoint                               = 10;
const int MCSource::c_NearFieldRestrictedPoint                     = 11;
const int MCSource::c_NearFieldDiffractionPoint                    = 12;
const int MCSource::c_NearFieldDiffractionPointKSpace              = 13;
const int MCSource::c_NearFieldLine                                = 14;
const int MCSource::c_NearFieldBox                                 = 15;
const int MCSource::c_NearFieldDisk                                = 16;
const int MCSource::c_NearFieldSphere                              = 17;
const int MCSource::c_NearFieldBeam                                = 18;
const int MCSource::c_NearFieldActivation                          = 19;
const int MCSource::c_NearFieldBeam1DProfile                       = 20;
const int MCSource::c_NearFieldBeam2DProfile                       = 21;
const int MCSource::c_NearFieldConeBeam                            = 22;
const int MCSource::c_NearFieldConeBeamGauss                       = 23;
const int MCSource::c_NearFieldIlluminatedDisk                     = 24;
const int MCSource::c_NearFieldIlluminatedSquare                   = 25;
const int MCSource::c_NearFieldVolume                              = 26;
const int MCSource::c_NearFieldFlatMap                             = 27;


// Don't change this list because the ID is written to the Sim file
const int MCSource::c_Gamma                                        = 1;

const int MCSource::c_Positron                                     = 2;
const int MCSource::c_Electron                                     = 3;

const int MCSource::c_Proton                                       = 4;
const int MCSource::c_AntiProton                                   = 5;
const int MCSource::c_Neutron                                      = 6;
const int MCSource::c_AntiNeutron                                  = 7;

const int MCSource::c_MuonPlus                                     = 8;
const int MCSource::c_MuonMinus                                    = 9;
const int MCSource::c_TauonPlus                                    = 10;
const int MCSource::c_TauonMinus                                   = 11;

const int MCSource::c_ElectronNeutrino                             = 12;
const int MCSource::c_AntiElectronNeutrino                         = 13;
const int MCSource::c_MuonNeutrino                                 = 14;
const int MCSource::c_AntiMuonNeutrino                             = 15;
const int MCSource::c_TauonNeutrino                                = 16;
const int MCSource::c_AntiTauonNeutrino                            = 17;

const int MCSource::c_Deuteron                                     = 18;
const int MCSource::c_Triton                                       = 19;
const int MCSource::c_He3                                          = 20;
const int MCSource::c_Alpha                                        = 21;

const int MCSource::c_GenericIon                                   = 22;

const int MCSource::c_PiPlus                                       = 23;
const int MCSource::c_PiZero                                       = 24;
const int MCSource::c_PiMinus                                      = 25;
const int MCSource::c_Eta                                          = 26;
const int MCSource::c_EtaPrime                                     = 27;

const int MCSource::c_KaonPlus                                     = 28;
const int MCSource::c_KaonZero                                     = 29;
const int MCSource::c_AntiKaonZero                                 = 30;
const int MCSource::c_KaonZeroS                                    = 31;
const int MCSource::c_KaonZeroL                                    = 32;
const int MCSource::c_KaonMinus                                    = 33;

const int MCSource::c_Lambda                                       = 34;
const int MCSource::c_AntiLambda                                   = 35;

const int MCSource::c_SigmaPlus                                    = 36;
const int MCSource::c_AntiSigmaPlus                                = 37;
const int MCSource::c_SigmaZero                                    = 38;
const int MCSource::c_AntiSigmaZero                                = 39;
const int MCSource::c_SigmaMinus                                   = 40;
const int MCSource::c_AntiSigmaMinus                               = 41;

const int MCSource::c_XiZero                                       = 42;
const int MCSource::c_AntiXiZero                                   = 43;
const int MCSource::c_XiMinus                                      = 44;
const int MCSource::c_AntiXiMinus                                  = 45;

const int MCSource::c_OmegaMinus                                   = 46;
const int MCSource::c_AntiOmegaMinus                               = 47;

const int MCSource::c_RhoPlus                                      = 48;
const int MCSource::c_RhoZero                                      = 49;
const int MCSource::c_RhoMinus                                     = 50;

const int MCSource::c_DeltaMinus                                   = 51;
const int MCSource::c_DeltaZero                                    = 52;
const int MCSource::c_DeltaPlus                                    = 53;
const int MCSource::c_DeltaPlusPlus                                = 54;

const int MCSource::c_Omega                                        = 55;

const int MCSource::c_Invalid                                      = -99999987;

unsigned int MCSource::s_ID = 0;



/******************************************************************************
 * Construction
 */
MCSource::MCSource()
{
  m_ID = s_ID++;

  Initialize();
}


/******************************************************************************
 * Construction by name
 */
MCSource::MCSource(MString Name) : m_Name(Name)
{
  m_ID = s_ID++;

  Initialize();
}


/******************************************************************************
 * Construction by name
 */
void MCSource::Initialize()
{
  m_Successor = "";

  m_NextEmission = 0;

  m_NGeneratedParticles = 0;
  m_NEventsPerLightCurveContent = 0;

  m_StartAreaType = c_StartAreaUnknown;
  m_StartAreaAverageArea = 0.0;
  m_StartAreaParam1 = c_Invalid;
  m_StartAreaParam2 = c_Invalid;
  
  m_CoordinateSystem = c_Invalid;
  m_SpectralType = c_Invalid;
  m_BeamType = c_Invalid;
  m_ParticleType = c_Invalid;
  m_ParticleExcitation = 0.0;
  m_ParticleDefinition = 0;

  m_NEventsToSkip = 0;
  m_NEventsSkipped = 0;

  m_IsFluxVariable = false;
  m_IsActive = true;
  m_IsSuccessor = false;
  m_IsEventList = false;
  m_IsBuildUpEventList = false;
  m_IsIsotopeCount = false;

  m_EventListSize = 0;
  m_IsotopeCount = 0;

  // Intensity of this source
  m_InputFlux = c_Invalid;
  m_Flux = c_Invalid;

  m_TotalEnergyFlux = c_Invalid;
  
  m_BinWidthLightCurve = c_Invalid;
  m_LightCurveOffset = c_Invalid;
  m_NEventsPerLightCurveContent = c_Invalid;

  m_PolarizationIsAbsolute = true;
  m_PolarizationDegree = 0.0;
  
  m_NGeneratedParticles = 0;
  
  m_PositionParam1 = c_Invalid;
  m_PositionParam2 = c_Invalid;
  m_PositionParam3 = c_Invalid;
  m_PositionParam4 = c_Invalid;
  m_PositionParam5 = c_Invalid;
  m_PositionParam6 = c_Invalid;
  m_PositionParam7 = c_Invalid;
  m_PositionParam8 = c_Invalid;
  m_PositionParam9 = c_Invalid;
  m_PositionParam10 = c_Invalid;
  m_PositionParam11 = c_Invalid;
  
  m_EnergyParam1 = c_Invalid;
  m_EnergyParam2 = c_Invalid;
  m_EnergyParam3 = c_Invalid;
  m_EnergyParam4 = c_Invalid;
  m_EnergyParam5 = c_Invalid;
  m_EnergyParam6 = c_Invalid;

  m_PositionTF1 = 0;
}


/******************************************************************************
 * Standard destructor
 */
MCSource::~MCSource()
{
  delete m_PositionTF1;
}


/******************************************************************************
 * In the case this is the delayed events list, check if the source
 * is compatible with the NEXT particle and if yes set an event to be skipped
 */
bool MCSource::GenerateSkippedEvents(MCSource* Source)
{
  if (m_IsBuildUpEventList == true && 
      Source->m_IsEventList == false && 
      m_EventListSize > 0) {
    
    if (Source->m_ParticleDefinition == m_EventListParticleType.front() && 
        Source->m_Volume == m_EventListVolumeName.front()) {
      Source->NEventsToSkip(1);
      //cout<<m_Name<<": Setting one event to skip: "<<Source->m_NEventsToSkip<<" in "<<Source->m_Name<<endl;
      return true;
    }
  }

  return false;
}


/******************************************************************************
 * Generate the particle of this source for the particle gun
 */
bool MCSource::GenerateParticles(G4GeneralParticleSource* ParticleGun)
{
  if (m_NEventsToSkip > 0) {
    mout<<m_Name<<": Skipped one event: "<<m_NEventsToSkip<<endl;
    m_NEventsToSkip--;
    m_NEventsSkipped++;
    return false;
  }

  if (m_IsEventList == true && m_EventListSize == 0) {
    mout<<m_Name<<": No events in event list"<<endl;
    return false;
  }

  if (GenerateParticle(ParticleGun) == false) {
    mout<<m_Name<<": GenerateParticle failed"<<endl;
    return false;
  }
  if (GenerateEnergy(ParticleGun) == false) {
    mout<<m_Name<<": GenerateEnergy failed"<<endl;
    return false;
  }
  if (GeneratePosition(ParticleGun) == false) {
    mout<<m_Name<<": GeneratePosition failed"<<endl;
    return false;
  }
  if (GeneratePolarization(ParticleGun) == false) {
    mout<<m_Name<<": GeneratePolarization failed"<<endl;
    return false;
  }

  if (m_IsEventList == true) {
    m_EventListEnergy.pop_front();
    m_EventListPosition.pop_front();
    m_EventListDirection.pop_front();
    m_EventListPolarization.pop_front();
    m_EventListTime.pop_front();
    m_EventListParticleType.pop_front();
    m_EventListVolumeName.pop_front();
    m_EventListSize--;
  }

  ++m_NGeneratedParticles;

  return true;
}


/******************************************************************************
 * Set the type of the start area from ehich the photons start in the far field
 */
bool MCSource::SetStartAreaType(const int& StartAreaType)
{
  switch (StartAreaType) {
  case c_StartAreaUnknown:
  case c_StartAreaSphere:
  case c_StartAreaTube:
    m_StartAreaType = StartAreaType;
    return true;
  default:
    return false;
  }

  if (StartAreaType == c_StartAreaTube) {
    if (m_BeamType == c_NearFieldIlluminatedDisk || 
        m_BeamType == c_NearFieldIlluminatedSquare || 
        m_BeamType == c_NearFieldRestrictedPoint) {

      mout<<m_Name<<": The given beam type requires a spherical start area and not a tube-like!"<<endl;
      return false;    
    }
  }


  return true;
}


/******************************************************************************
 * Return true, if the start area vector could be set correctly. For a 
 * description of the meaning of the parameters see the documentation. 
 */
bool MCSource::SetStartAreaParameters(double StartAreaParam1, 
                                      double StartAreaParam2, 
                                      double StartAreaParam3, 
                                      double StartAreaParam4, 
                                      double StartAreaParam5, 
                                      double StartAreaParam6, 
                                      double StartAreaParam7, 
                                      double StartAreaParam8)
{
  if (StartAreaParam1 == c_Invalid ||
      StartAreaParam2 == c_Invalid ||
      StartAreaParam3 == c_Invalid) {
    mout<<m_Name<<": The first 3 parameters of the start area must be a position"<<endl;
    return false;
  }
  m_StartAreaPosition = G4ThreeVector(StartAreaParam1, StartAreaParam2, StartAreaParam3);

  if (StartAreaParam4 == c_Invalid ||
      StartAreaParam5 == c_Invalid ||
      StartAreaParam6 == c_Invalid) {
    mout<<m_Name<<": The second 3 parameters of the start area must be a rotation"<<endl;
    return false;
  }

  if (StartAreaParam4 != 0.0 ||
      StartAreaParam5 != 0.0 ||
      StartAreaParam6 != 0.0) {
    mout<<m_Name<<": The parameters 4, 5 & 6 of the start area, the rotation, must be zero since the rotation is not yet implemented."<<endl;
    return false;
  }


  // Some sanity checks:
  if (m_StartAreaType == c_StartAreaUnknown) {
    mout<<m_Name<<": The start area typ is not yet set"<<endl;
    return false;
  } else if (m_StartAreaType == c_StartAreaSphere) {
    if (StartAreaParam7 == c_Invalid) {
      mout<<m_Name<<": A valid radius for the start area sphere is required"<<endl;
      return false;
    }
    if (StartAreaParam7 <= 0) {
      mout<<m_Name<<": A radius larger 0 for the start area sphere is required"<<endl;
      return false;
    }
    m_StartAreaParam1 = StartAreaParam7;
  } else if (m_StartAreaType == c_StartAreaTube) {
    if (StartAreaParam7 == c_Invalid) {
      mout<<m_Name<<": A valid radius for the start area tube is required"<<endl;
      return false;
    }
    if (StartAreaParam7 <= 0) {
      mout<<m_Name<<": A radius larger 0 for the start area tube is required"<<endl;
      return false;
    }
    m_StartAreaParam1 = StartAreaParam7;
    if (StartAreaParam8 == c_Invalid) {
      mout<<m_Name<<": A valid height for the start area tube is required"<<endl;
      return false;
    }
    if (StartAreaParam8 <= 0) {
      mout<<m_Name<<": A height larger 0 for the start area tube is required"<<endl;
      return false;
    }
    m_StartAreaParam2 = StartAreaParam8;
  }

  // We have to update the start area parameters
  UpgradeStartArea();
  // We have to update the position vectors
  UpgradePosition();
  // and the flux normalization
  UpgradeFlux();

  return true;
}


/******************************************************************************
 * Upgrade the start area parameters and do some sanity checks 
 */
bool MCSource::UpgradeStartArea() 
{
  if (m_StartAreaType == c_StartAreaSphere) {
     m_StartAreaAverageArea = c_Pi*m_StartAreaParam1*m_StartAreaParam1;
  } else if (m_StartAreaType == c_StartAreaTube) {
    // Calculate the average start area
    if (m_BeamType == c_FarFieldPoint) {
      // Area is: 4*R*H*sin(theta) + 4*R*R*cos(theta)
      m_StartAreaAverageArea = 4*m_StartAreaParam1 * (m_StartAreaParam2*sin(m_PositionParam1) + m_StartAreaParam1*fabs(cos(m_PositionParam1)));
    } 
    else if (m_BeamType == c_FarFieldArea) {
//       m_StartAreaAverageArea = 
//         2*m_StartAreaParam1*m_StartAreaParam2*(cos(m_PositionParam1)*cos(m_PositionParam1) - cos(m_PositionParam2)*cos(m_PositionParam2)) +
//         2*m_StartAreaParam1*m_StartAreaParam1*(cos(m_PositionParam2)*sin(m_PositionParam2) - cos(m_PositionParam1)*sin(m_PositionParam1)) +
//         2*m_StartAreaParam1*m_StartAreaParam1*(m_PositionParam2 - m_PositionParam1); 
//       cout<<"SA: "<<m_StartAreaAverageArea<<endl;
      
//       m_StartAreaAverageArea /= 2*c_Pi*(cos(m_PositionParam1) - cos(m_PositionParam2)) * (m_PositionParam4 - m_PositionParam3)/(2*c_Pi);
      m_StartAreaAverageArea = 0.0;
      int i_max = 5000;
      double t1, t2, ta;
      double t_diff = (m_PositionParam2-m_PositionParam1)/i_max;
      for (int i = 0; i < i_max-1; ++i) {
        t1 = m_PositionParam1 + i*t_diff;
        t2 = t1 + t_diff;
        ta = 0.5*(t1+t2);
        m_StartAreaAverageArea += 2*c_Pi*(cos(t1)-cos(t2)) * 4*m_StartAreaParam1*(m_StartAreaParam2*sin(ta) + m_StartAreaParam1*fabs(cos(ta)));
      }
      m_StartAreaAverageArea /= 2*c_Pi*(cos(m_PositionParam1) - cos(m_PositionParam2)) * (m_PositionParam4 - m_PositionParam3)/(2*c_Pi);
    } 
    else if (m_BeamType == c_FarFieldFileZenithDependent) {
      double AverageArea = 0.0;
      m_StartAreaAverageArea = 0.0;
      int i_max = 5000;
      double t1, t2, ta;
      double t_diff = (m_PositionFunction.GetXMax() - m_PositionFunction.GetXMin())/i_max;
      for (int i = 0; i < i_max-1; ++i) {
        t1 = m_PositionFunction.GetXMin() + i*t_diff;
        t2 = t1 + t_diff;
        ta = 0.5*(t1+t2);
        m_StartAreaAverageArea += m_PositionFunction.Eval(ta) * 2*c_Pi*(cos(t1)-cos(t2)) * 4*m_StartAreaParam1*(m_StartAreaParam2*sin(ta) + m_StartAreaParam1*fabs(cos(ta)));
        AverageArea += m_PositionFunction.Eval(ta) * 2*c_Pi*(cos(t1)-cos(t2));
      }
      m_StartAreaAverageArea /= AverageArea;
    }
    else if (m_BeamType == c_FarFieldNormalizedEnergyBeamFluxFunction) {
      cout<<"Calculating tube start area for FarFieldNormalizedEnergyBeamFluxFunction... "<<flush;
      double AreaNormalizer = 0.0;
      m_StartAreaAverageArea = 0.0;
      
      vector<double> PhiAxis = m_NormalizedEnergyBeamFluxFunction.GetXAxis();
      vector<double> ThetaAxis = m_NormalizedEnergyBeamFluxFunction.GetYAxis();
      vector<double> EnergyAxis = m_NormalizedEnergyBeamFluxFunction.GetZAxis();


      double PhiStepSize = 10;
      double ThetaStepSize = 2;
      double EnergyStepSize = 10;
      unsigned int PhiInterval, ThetaInterval, EnergyInterval;
      double PhiMin, PhiMax, ThetaMin, ThetaMax, EnergyMin, EnergyMax;
      double PhiDiff, ThetaDiff, EnergyDiff;
      double PhiAvg, ThetaAvg, EnergyAvg;

      double Eval, Area, StartArea;
      for (unsigned int p = 0; p < PhiAxis.size()-1; ++p) {
        PhiInterval = (unsigned int) ((PhiAxis[p+1] - PhiAxis[p])/PhiStepSize) + 1;
        PhiDiff = (PhiAxis[p+1] - PhiAxis[p])/PhiInterval;
        for (unsigned int dp = 0; dp < PhiInterval; ++dp) {
          PhiMin = PhiAxis[p] + dp * PhiDiff;
          PhiMax = PhiAxis[p] + (dp+1) * PhiDiff;
          PhiAvg = 0.5*(PhiMin + PhiMax);
          for (unsigned int t = 0; t < ThetaAxis.size()-1; ++t) {
            ThetaInterval = (unsigned int) ((ThetaAxis[t+1] - ThetaAxis[t])/ThetaStepSize) + 1;
            ThetaDiff = (ThetaAxis[t+1] - ThetaAxis[t])/ThetaInterval;
            for (unsigned int dt = 0; dt < ThetaInterval; ++dt) {
              ThetaMin = ThetaAxis[t] + dt * ThetaDiff;
              ThetaMax = ThetaAxis[t] + (dt+1) * ThetaDiff;
              ThetaAvg = 0.5*(ThetaMin + ThetaMax);
              for (unsigned int e = 0; e < EnergyAxis.size()-1; ++e) {
                EnergyInterval = (unsigned int) ((EnergyAxis[e+1] - EnergyAxis[e])/EnergyStepSize) + 1;
                EnergyDiff = (EnergyAxis[e+1] - EnergyAxis[e])/EnergyInterval;               
                for (unsigned int de = 0; de < EnergyInterval; ++de) {
                  EnergyMin = EnergyAxis[e] + de * EnergyDiff;
                  EnergyMax = EnergyAxis[e] + (de+1) * EnergyDiff;
                  EnergyAvg = 0.5*(EnergyMin + EnergyMax);
                  
                  Eval = m_NormalizedEnergyBeamFluxFunction.Eval(PhiAvg, ThetaAvg, EnergyAvg);
                  Area = (cos(ThetaMin*c_Rad)-cos(ThetaMax*c_Rad))*(PhiMax*c_Rad-PhiMin*c_Rad)*(EnergyMax-EnergyMin);
                  StartArea = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(ThetaAvg*c_Rad) + m_StartAreaParam1*fabs(cos(ThetaAvg*c_Rad)));
                  m_StartAreaAverageArea += Eval*Area*StartArea;
                  AreaNormalizer += Eval*Area;
                }
              }
            }
          }
        }
      } 

      m_StartAreaAverageArea /= AreaNormalizer;
      if (fabs(AreaNormalizer - m_NormalizedEnergyBeamFluxFunction.Integrate())/AreaNormalizer > 0.02) {
        cout<<m_Name<<": Please use a spherical start area --- uncertainties for *fast* integral calculation are too large: "<<100*fabs(AreaNormalizer - m_NormalizedEnergyBeamFluxFunction.Integrate())/AreaNormalizer<<" % "<<endl;
        return false;
      }
      cout<<"Done!"<<endl;
    }
  }

  cout<<m_Name<<": average start area: "<<m_StartAreaAverageArea/cm/cm<<" cm2"<<endl;

  return true;
}


/******************************************************************************
 *  Return true, if the particle type could be set correctly
 */
bool MCSource::SetSpectralType(const int& SpectralType)
{
  switch (SpectralType) {
  case c_Map:
  case c_Monoenergetic:
  case c_Linear:
  case c_PowerLaw:
  case c_BrokenPowerLaw:
  case c_Gaussian:
  case c_ThermalBremsstrahlung:
  case c_BlackBody:
  case c_BandFunction:
  case c_FileDifferentialFlux:
  case c_Activation:
  case c_NormalizedEnergyBeamFluxFunction:
    m_SpectralType = SpectralType;
    return true;
  default:
    return false;
  }

  return true;
}


/******************************************************************************
 * Return the name of the spectrum (e.g. mono, etc.)
 */
string MCSource::GetSpectralTypeAsString() const
{
  string Name = "";

  switch (m_SpectralType) {
  case c_Map:
    Name = "Map";
    break;
  case c_Monoenergetic:
    Name = "Mono";
    break;
  case c_Linear:
    Name = "Linear";
    break;
  case c_PowerLaw:
    Name = "PowerLaw";
    break;
  case c_BrokenPowerLaw:
    Name = "BrokenPowerLaw";
    break;
  case c_Gaussian:
    Name = "Gaussian";
    break;
  case c_ThermalBremsstrahlung:
    Name = "ThermalBremsstrahlung";
    break;
  case c_BlackBody:
    Name = "BlackBody";
    break;
  case c_BandFunction:
    Name = "BandFunction";
    break;
  case c_FileDifferentialFlux:
    Name = "FileDifferentialFlux";
    break;
  case c_Activation:
    Name = "Activation";
    break;
  case c_NormalizedEnergyBeamFluxFunction:
    Name = "NormalizedEnergyBeamFluxFunction";
    break;
  default:
    break;
  }
  
  return Name;
}


/******************************************************************************
 * Return true, if the coordinate system could be set correctly
 */
bool MCSource::SetCoordinateSystem(const int& CoordinateSystem)
{
  switch (CoordinateSystem) {
  case c_NearField:
  case c_FarField:
    m_CoordinateSystem = CoordinateSystem;
    return true;
  default:
    mout<<m_Name<<": Unknown coordinate system: "<<CoordinateSystem<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true, if the particle type could be set correctly
 */
bool MCSource::SetParticleType(const int& ParticleType)
{

  // First check for a  nucleus:
  if (ParticleType > 1000) {
    mimp<<"Missing test if correct"<<show;
    m_ParticleType = ParticleType;
    return true;
  }

  switch (ParticleType) {
  case c_Gamma:

  case c_Positron:
  case c_Electron:

  case c_Proton:
  case c_AntiProton:
  case c_Neutron:
  case c_AntiNeutron:

  case c_MuonPlus:
  case c_MuonMinus:
  case c_TauonPlus:
  case c_TauonMinus:

  case c_ElectronNeutrino:
  case c_AntiElectronNeutrino:
  case c_MuonNeutrino:
  case c_AntiMuonNeutrino:
  case c_TauonNeutrino:
  case c_AntiTauonNeutrino:

  case c_Deuteron:
  case c_Triton:
  case c_He3:
  case c_Alpha:

  case c_PiPlus:
  case c_PiZero:
  case c_PiMinus:
  case c_Eta:
  case c_EtaPrime:

  case c_KaonPlus:
  case c_KaonZero:
  case c_AntiKaonZero:
  case c_KaonZeroS:
  case c_KaonZeroL:
  case c_KaonMinus:

  case c_Lambda:
  case c_AntiLambda:

  case c_SigmaPlus:
  case c_AntiSigmaPlus:
  case c_SigmaZero:
  case c_AntiSigmaZero:
  case c_SigmaMinus:
  case c_AntiSigmaMinus:

  case c_XiZero:
  case c_AntiXiZero:
  case c_XiMinus:
  case c_AntiXiMinus:

  case c_OmegaMinus:
  case c_AntiOmegaMinus:

    m_ParticleType = ParticleType;
    break;
  default:
    mout<<m_Name<<": Unknown particle type!"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true, if the particle type could be set correctly
 */
bool MCSource::SetParticleExcitation(const double& ParticleExcitation)
{
  m_ParticleExcitation = ParticleExcitation;

  return true;
}


/******************************************************************************
 * Return true, if the region type could be set correctly
 */
bool MCSource::SetBeamType(const int& CoordinateSystem, const int& BeamType)
{
  switch (CoordinateSystem) {
  case c_NearField:
  case c_FarField:
    m_CoordinateSystem = CoordinateSystem;
    break;
  default:
    mout<<m_Name<<": Unknown Coordinate system: "<<CoordinateSystem<<endl;
    return false;
  }

  switch (BeamType) {
  case c_NearFieldPoint:
  case c_NearFieldLine:
  case c_NearFieldBox:
  case c_NearFieldDisk:
  case c_NearFieldSphere:
  case c_NearFieldBeam:
  case c_NearFieldActivation:
  case c_NearFieldRestrictedPoint:
  case c_NearFieldDiffractionPoint:
  case c_NearFieldDiffractionPointKSpace:
  case c_NearFieldBeam1DProfile:
  case c_NearFieldBeam2DProfile:
  case c_NearFieldConeBeam:
  case c_NearFieldConeBeamGauss:
  case c_NearFieldIlluminatedDisk:
  case c_NearFieldIlluminatedSquare:
  case c_NearFieldVolume:
  case c_NearFieldFlatMap:
  case c_FarFieldPoint:
  case c_FarFieldArea:
  case c_FarFieldGaussian:
  case c_FarFieldFileZenithDependent:
  case c_FarFieldNormalizedEnergyBeamFluxFunction:
    m_BeamType = BeamType;
    break;
  default:
    mout<<m_Name<<": Unknown beam type: "<<BeamType<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return the name of the beam (e.g. point, line, etc.)
 */
string MCSource::GetBeamTypeAsString() const
{
  string Name = "";

  switch (m_BeamType) {
  case c_NearFieldPoint:
    Name = "Point";
    break;
  case c_NearFieldLine:
    Name = "Line";
    break;
  case c_NearFieldBox:
    Name = "Box";
    break;
  case c_NearFieldDisk:
    Name = "Disk";
    break;
  case c_NearFieldSphere:
    Name = "Sphere";
    break;
  case c_NearFieldBeam:
    Name = "HomogeneousBeam";
    break;
  case c_NearFieldActivation:
    Name = "Activation";
    break;
  case c_NearFieldRestrictedPoint:
    Name = "RestrictedPoint";
    break;
  case c_NearFieldDiffractionPoint:
    Name = "DiffractionPointSource";
    break;
  case c_NearFieldDiffractionPointKSpace:
    Name = "DiffractionPointKSpace";
    break;
  case c_NearFieldBeam1DProfile:
    Name = "RadialProfileBeam";
    break;
  case c_NearFieldBeam2DProfile:
    Name = "MapProfileBeam";
    break;
  case c_NearFieldConeBeam:
    Name = "ConeBeam";
    break;
  case c_NearFieldConeBeamGauss:
    Name = "GaussianConeBeam";
    break;
  case c_NearFieldIlluminatedDisk:
    Name = "IlluminatedDisk";
    break;
  case c_NearFieldIlluminatedSquare:
    Name = "IlluminatedSquare";
    break;
  case c_NearFieldVolume:
    Name = "Volume";
    break;
  case c_NearFieldFlatMap:
    Name = "Map";
    break;
  case c_FarFieldPoint:
    Name = "FarFieldPointSource";
    break;
  case c_FarFieldArea:
    Name = "FarFieldPointSource";
    break;
  case c_FarFieldGaussian:
    Name = "FarFieldGaussian";
    break;
  case c_FarFieldFileZenithDependent:
    Name = "FarFieldFileZenithDependent";
    break;
  case c_FarFieldNormalizedEnergyBeamFluxFunction:
    Name = "FarFieldNormalizedEnergyBeamFluxFunction";
    break;
  default:
    break;
  }

  return Name;
}


/******************************************************************************
 * Return true, if the position vector could be set correctly. For a 
 * description of the meaning of the parameters see the documentation. 
 */
bool MCSource::SetPosition(double PositionParam1, 
                           double PositionParam2, 
                           double PositionParam3, 
                           double PositionParam4, 
                           double PositionParam5, 
                           double PositionParam6, 
                           double PositionParam7, 
                           double PositionParam8, 
                           double PositionParam9, 
                           double PositionParam10, 
                           double PositionParam11)
{
  m_PositionParam1 = PositionParam1;
  m_PositionParam2 = PositionParam2;
  m_PositionParam3 = PositionParam3;
  m_PositionParam4 = PositionParam4;
  m_PositionParam5 = PositionParam5;
  m_PositionParam6 = PositionParam6;
  m_PositionParam7 = PositionParam7;
  m_PositionParam8 = PositionParam8;
  m_PositionParam9 = PositionParam9;
  m_PositionParam10 = PositionParam10;
  m_PositionParam11 = PositionParam11;

  // Some sanity checks:
  if (m_BeamType == c_FarFieldPoint) {
    if (m_PositionParam1 < 0 || m_PositionParam1 > c_Pi) {
      mout<<m_Name<<": Theta must be within [0..pi]"<<endl;
      return false;
    }
  } else if (m_BeamType == c_FarFieldArea) {
    if (m_PositionParam1 < 0 || m_PositionParam1 > c_Pi) {
      mout<<m_Name<<": Minimum theta must be within [0..pi]"<<endl;
      return false;
    }
    if (m_PositionParam2 < 0 || m_PositionParam2 > c_Pi) {
      mout<<m_Name<<": Maximum theta must be within [0..pi]"<<endl;
      return false;
    }
    if (m_PositionParam2 <= m_PositionParam1) {
      mout<<m_Name<<": Maximum theta must be larger than minimum theta"<<endl;
      return false;
    }
    if (m_PositionParam4 <= m_PositionParam3) {
      mout<<m_Name<<": Maximum phi must be larger than minimum theta"<<endl;
      return false;
    }
  } else if (m_BeamType == c_FarFieldGaussian) {
    // not implemented
  } else if (m_BeamType == c_FarFieldFileZenithDependent) {
    // nothing
  } else if (m_BeamType == c_FarFieldNormalizedEnergyBeamFluxFunction) {
    // nothing
  } else if (m_BeamType == c_NearFieldPoint) {
    // nothing
  } else if (m_BeamType == c_NearFieldRestrictedPoint) {
    // nothing
  } else if (m_BeamType == c_NearFieldDiffractionPoint ||
             m_BeamType == c_NearFieldDiffractionPointKSpace) {
    if (m_PositionParam4 == 0 && m_PositionParam5 == 0 && m_PositionParam6 == 0) {
      mout<<m_Name<<": The direction of the normal vector must not be (0, 0, 0)"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldLine) {
    if (m_PositionParam1 == m_PositionParam4 &&
        m_PositionParam2 == m_PositionParam5 &&
        m_PositionParam3 == m_PositionParam6) {
      mout<<m_Name<<": Position 1 must be different from position 2"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldBox) {
    if (m_PositionParam1 == m_PositionParam4 &&
        m_PositionParam2 == m_PositionParam5 &&
        m_PositionParam3 == m_PositionParam6) {
      mout<<m_Name<<": Position 1 must be different from position 2"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldSphere) {
    if (m_PositionParam4 <= 0 || m_PositionParam5 <= 0 || m_PositionParam6 <= 0) {
      mout<<m_Name<<": The radii must be larger than zero"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldDisk) {
    if (m_PositionParam6 <= 0 || m_PositionParam7 <= 0 || m_PositionParam8 <= 0) {
      mout<<m_Name<<": Inner radius, outer radius, and height must be larger than zero"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldBeam) {
    if (m_PositionParam4 == 0 && m_PositionParam5 == 0 && m_PositionParam6 == 0) {
      mout<<m_Name<<": The direction must not be (0, 0, 0)"<<endl;
      return false;
    }
    if (m_PositionParam7 <= 0) {
      mout<<m_Name<<": The radius must be larger than zero"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldBeam1DProfile || m_BeamType == c_NearFieldBeam2DProfile || m_BeamType == c_NearFieldFlatMap) {
    if (m_PositionParam4 == 0 && m_PositionParam5 == 0 && m_PositionParam6 == 0) {
      mout<<m_Name<<": The direction must not be (0, 0, 0)"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldConeBeam) {
    if (m_PositionParam4 == 0 && m_PositionParam5 == 0 && m_PositionParam6 == 0) {
      mout<<m_Name<<": The direction must not be (0, 0, 0)"<<endl;
      return false;
    }
    if (m_PositionParam7 <= 0) {
      mout<<m_Name<<": The opening angle must be larger than zero"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldConeBeamGauss) {
    if (m_PositionParam4 == 0 && m_PositionParam5 == 0 && m_PositionParam6 == 0) {
      mout<<m_Name<<": The direction must not be (0, 0, 0)"<<endl;
      return false;
    }
    if (m_PositionParam7 <= 0) {
      mout<<m_Name<<": The opening angle must be larger than zero"<<endl;
      return false;
    }
    if (m_PositionParam8 <= 0) {
      mout<<m_Name<<": The gaussian 1-sigma value must be larger than zero"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldIlluminatedDisk) {
    if (m_PositionParam4 <= 0) {
      mout<<m_Name<<": The radius of the disk must be larger than zero"<<endl;
      return false;
    }
    if (m_PositionParam5 < 0 || m_PositionParam5 > c_Pi) {
      mout<<m_Name<<": The phi orientation of the disk must be within [0..pi]"<<endl;
      return false;
    }
    if (m_PositionParam7 < 0 || m_PositionParam7 > c_Pi) {
      mout<<m_Name<<": The phi orientation of the beam must be within [0..pi]"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldIlluminatedSquare) {
    if (m_PositionParam4 <= 0) {
      mout<<m_Name<<": The half length of the box must be larger than zero"<<endl;
      return false;
    }
    if (m_PositionParam5 < 0 || m_PositionParam5 > c_Pi) {
      mout<<m_Name<<": The phi orientation of the box must be within [0..pi]"<<endl;
      return false;
    }
    if (m_PositionParam7 < 0 || m_PositionParam7 > c_Pi) {
      mout<<m_Name<<": The phi orientation of the beam must be within [0..pi]"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldActivation || 
             m_BeamType == c_NearFieldVolume) {
    if (MCRunManager::GetMCRunManager()->GetDetectorConstruction()->IsValidVolume(m_Volume) == false) {
      mout<<m_Name<<": The volume "<<m_Volume<<" does not exist in the loaded geometry!"<<endl;
      return false;      
    }
  }
  
  return UpgradePosition();
}


/******************************************************************************
 * Return true if the combined normalized energy-beam-flux-function could be set
 */
bool MCSource::SetNormalizedEnergyBeamFluxFunction(MString FileName)
{
  if (m_BeamType != c_FarFieldNormalizedEnergyBeamFluxFunction) return false;
  
  if (MFile::Exists(FileName) == false) return false;

  if (m_NormalizedEnergyBeamFluxFunction.Set(FileName, "AP") == false) return false;

  // Integrate over 3D to determine the flux
  m_InputFlux = m_NormalizedEnergyBeamFluxFunction.Integrate()/cm/cm/s;
  
  UpgradeFlux();

  return true;
}


/******************************************************************************
 * Return true, if all tests and upgrades for the position parameters
 * are sucessfull
 */
bool MCSource::UpgradePosition()
{
  // Attention: This function might be called twice:

  if (m_BeamType == c_NearFieldRestrictedPoint) {
    if (m_StartAreaType != c_StartAreaSphere && m_StartAreaType != c_StartAreaUnknown) {
      mout<<m_Name<<": The beam type NearFieldRestrictedPoint requires a sphere as start area!"<<endl;
      return false;
    }

    // CartesianRestrictedPoint is a special case of CartesianConeBeam
    // Thus upgrade the former to the later:

    // Set the beam direction: m_PositionParam4, m_PositionParam5, m_PositionParam6
    // Direction from the origin to the center of the surrounding sphere
    m_PositionParam4 = m_StartAreaPosition.getX() - m_PositionParam1;
    m_PositionParam5 = m_StartAreaPosition.getY() - m_PositionParam2;
    m_PositionParam6 = m_StartAreaPosition.getZ() - m_PositionParam3;

    // Set the cone angle (i.e. half opening angle): m_PositionParam7
    // Pay attention to the case, when the start position is inside the sphere
    if (G4ThreeVector(m_PositionParam4, m_PositionParam5, m_PositionParam6).mag() > m_StartAreaParam1) {
      m_PositionParam7 = asin(m_StartAreaParam1/(m_StartAreaPosition-G4ThreeVector(m_PositionParam1, m_PositionParam2, m_PositionParam3)).mag());
    } else {
      m_PositionParam7 = c_Pi;
    }
  }

  else if (m_BeamType == c_NearFieldConeBeamGauss) {
    if (m_PositionTF1 != 0) delete m_PositionTF1;
    m_PositionTF1 = new TF1("CartesianConeBeamGauss", "exp(-(x*x)/(2.*[0]*[0])) * sin(x)", 0.0, m_PositionParam7);
    m_PositionTF1->SetParameter(0, m_PositionParam8);
  }

  return true;
}


/******************************************************************************
 * Return true, if the file containing the spectrum could be set correctly
 */  
bool MCSource::SetPosition(MString FileName)
{
  if (m_BeamType == c_FarFieldFileZenithDependent) {
    if (m_PositionFunction.Set(FileName, "DP") == false) {
      mout<<m_Name<<": SphericalFileZenithDependent: Unable to load beam!"<<endl;
      return false;
    }
    //m_PositionFunction.Plot();
    m_PositionFunction.ScaleX(deg);
    m_PositionParam1 = m_PositionFunction.GetXMin();
    m_PositionParam2 = m_PositionFunction.GetXMax();
    m_PositionParam3 = m_PositionFunction.GetYMax();
    if (m_PositionParam3 == 0.0) {
      mout<<m_Name<<": SphericalFileZenithDependent: Maximum is zero: "<<m_PositionFunction.GetYMax()<<endl;
      return false;
    }
    if (m_PositionFunction.GetSize() < 2) {
      mout<<m_Name<<": At least two entries in the file are required!"<<endl;
      return false;
    }
  } else if (m_BeamType == c_NearFieldBeam1DProfile) {
    if (m_PositionFunction.Set(FileName, "DP") == false) {
      mout<<m_Name<<": CartesianBeam1DProfile: Unable to load beam profile!"<<endl;
      return false;
    }
    // Scale x to cm
    m_PositionFunction.ScaleX(cm);
    // We have to multiply the content with [2*pi*] x to simplify the simulation
    //m_PositionFunction.ScaleYTimesX(2*c_Pi);
    //m_PositionFunction.Plot();
  } else if (m_BeamType == c_NearFieldBeam2DProfile || m_BeamType == c_NearFieldFlatMap) {
    if (m_PositionFunction2D.Set(FileName, "AV") == false) {
      mout<<m_Name<<": Unable to load 2D distribution!"<<endl;
      return false;
    }
    m_PositionFunction2D.ScaleX(cm);
    m_PositionFunction2D.ScaleY(cm);
//     m_PositionFunction2D.Plot();
//     while (true) {
//       gSystem->ProcessEvents();
//       gSystem->Sleep(10);
//     }
  } else if (m_BeamType == c_NearFieldDiffractionPoint) {
    if (m_PositionFunction2D.Set(FileName, "DP") == false) {
      mout<<m_Name<<": Unable to load 2D distribution!"<<endl;
      return false;
    }
    m_PositionFunction2D.ScaleX(c_Pi/180);
    m_PositionFunction2D.ScaleY(c_Pi/180);

    // Retrieve the minimum and maximum theta and phi
    m_PositionParam8 = m_PositionFunction2D.GetXMin();
    m_PositionParam9 = m_PositionFunction2D.GetXMax();
    m_PositionParam10 = m_PositionFunction2D.GetYMin();
    m_PositionParam11 = m_PositionFunction2D.GetYMax();

//     m_PositionFunction2D.Plot();
//     while (true) {
//       gSystem->ProcessEvents();
//       gSystem->Sleep(10);
//     }
  } else if (m_BeamType == c_NearFieldDiffractionPointKSpace) {
    if (m_PositionFunction2D.Set(FileName, "DP") == false) {
      mout<<m_Name<<": Unable to load 2D distribution!"<<endl;
      return false;
    }

//     m_PositionFunction2D.Plot();
//     while (true) {
//       gSystem->ProcessEvents();
//       gSystem->Sleep(10);
//     }
  } else {
    mout<<m_Name<<": Wrong region type for beam type "<<m_BeamType<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true, if the energy vector could be set correctly. For a 
 * description of the meaning of the parameters see the documentation.
 */
bool MCSource::SetEnergy(double EnergyParam1, 
                         double EnergyParam2, 
                         double EnergyParam3, 
                         double EnergyParam4, 
                         double EnergyParam5, 
                         double EnergyParam6,
                         double EnergyParam7)
{
  m_EnergyParam1 = EnergyParam1;
  m_EnergyParam2 = EnergyParam2;
  m_EnergyParam3 = EnergyParam3;
  m_EnergyParam4 = EnergyParam4;
  m_EnergyParam5 = EnergyParam5;
  m_EnergyParam6 = EnergyParam6;
  m_EnergyParam7 = EnergyParam7;

  // Perform some sanity checks:
  if (m_SpectralType == c_Monoenergetic) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The energy must be larger than 0!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_Linear) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The minimum energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= m_EnergyParam1) {
      mout<<m_Name<<": The maximum energy must be larger than the minimum energy!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_PowerLaw) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The minimum energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= m_EnergyParam1) {
      mout<<m_Name<<": The maximum energy must be larger than the minimum energy!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_BrokenPowerLaw) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The minimum energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= m_EnergyParam1) {
      mout<<m_Name<<": The maximum energy must be larger than the minimum energy!"<<endl;
      return false;
    }
    if (m_EnergyParam3 <= m_EnergyParam1 || m_EnergyParam3 > m_EnergyParam2) {
      mout<<m_Name<<": The break energy must be within the minimum and maximum energy!"<<endl;
      return false;
    }
    if (m_EnergyParam4 <= 0) {
      mout<<m_Name<<": The first index (alpha) must be positive!"<<endl;
      return false;      
    }
    if (m_EnergyParam5 <= 0) {
      mout<<m_Name<<": The second index (alpha) must be positive!"<<endl;
      return false;      
    }
  } else if (m_SpectralType == c_Gaussian) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= 0) {
      mout<<m_Name<<": The sigma must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam3 <= 0) {
      mout<<m_Name<<": The cut-off must be larger than 0!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_ThermalBremsstrahlung) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The minimum energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= m_EnergyParam1) {
      mout<<m_Name<<": The maximum energy must be larger than the minimum energy!"<<endl;
      return false;
    }
    if (m_EnergyParam3 <= 0) {
      mout<<m_Name<<": The temperature must be positive!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_BlackBody) {
    if (m_EnergyParam1 <= 0) {
      mout<<m_Name<<": The minimum energy must be larger than 0!"<<endl;
      return false;
    }
    if (m_EnergyParam2 <= m_EnergyParam1) {
      mout<<m_Name<<": The maximum energy must be larger than the minimum energy!"<<endl;
      return false;
    }
    if (m_EnergyParam3 <= 0) {
      mout<<m_Name<<": The temperature must be positive!"<<endl;
      return false;
    }
  } else if (m_SpectralType == c_BandFunction) {
    // requires reimplementation
  } else if (m_SpectralType == c_FileDifferentialFlux) {
  } else if (m_SpectralType == c_Activation) {
  } else if (m_SpectralType == c_NormalizedEnergyBeamFluxFunction) {
  }

  return UpgradeEnergy();
}


/******************************************************************************
 * Return true, if all tests and upgrade to the energy parameters could be 
 * performed
 */
bool MCSource::UpgradeEnergy() 
{
  if (m_SpectralType == c_BrokenPowerLaw) {
    m_EnergyParam6 = pow(m_EnergyParam3, -m_EnergyParam4+m_EnergyParam5);
  } else if (m_SpectralType == c_BlackBody) {
    massert(m_EnergyParam1 > 0);
    massert(m_EnergyParam2 > m_EnergyParam1);
    massert(m_EnergyParam3 > 0);

    const double MaximumShift = 1.593624260;
    m_EnergyParam4 = BlackBody(MaximumShift*m_EnergyParam3, m_EnergyParam3);
    if (MaximumShift*m_EnergyParam3 < m_EnergyParam1) m_EnergyParam4 = BlackBody(m_EnergyParam1, m_EnergyParam3);
    if (MaximumShift*m_EnergyParam3 > m_EnergyParam2) m_EnergyParam4 = BlackBody(m_EnergyParam2, m_EnergyParam3);
  } else if (m_SpectralType == c_BandFunction) {
    massert(m_EnergyParam1 > 0);
    massert(m_EnergyParam2 > 0);
    massert(m_EnergyParam1 < m_EnergyParam2);
    massert(m_EnergyParam3 > m_EnergyParam4);
    massert(m_EnergyParam5 > 0);
    
    // Calculate Maximum:
    m_EnergyParam6 = BandFunction(m_EnergyParam1, m_EnergyParam3, m_EnergyParam4, m_EnergyParam5);
    cout<<"Band-Max: "<<m_EnergyParam6 <<endl;
    //m_EnergyParam6 = BandFunction(m_EnergyParam3*m_EnergyParam5, m_EnergyParam3, m_EnergyParam4, m_EnergyParam5);
  }
  
  return true;
}


/******************************************************************************
 * Return true, if the file containing the spectrum could be set correctly
 */
bool MCSource::SetEnergy(MString FileName)
{
  if (m_SpectralType == c_FileDifferentialFlux) {
    if (m_EnergyFunction.Set(FileName, "DP") == false) {
      mout<<m_Name<<": Unable to load spectrum correctly!"<<endl;
      return false;
    }
    // m_EnergyFunction.Plot();
    m_EnergyFunction.ScaleX(keV);
    m_EnergyFunction.ScaleY(1.0/cm/cm/s/keV);

    if (m_EnergyFunction.GetXMin() < 0.0 ||
        m_EnergyFunction.GetXMax() < 0.0 ||
        m_EnergyFunction.GetYMin() < 0.0 ||
        m_EnergyFunction.GetYMax() < 0.0) {
      mout<<m_Name<<": Neither x nor y values are allowed to be negative!"<<endl;
      return false;
    }
    if (m_EnergyFunction.GetSize() < 2) {
      mout<<m_Name<<": At least two entries in the file are required!"<<endl;
      return false;
    }
    return true;
  } else {
    return false;
  }
}


/******************************************************************************
 * Return true, if the flux could be set correctly
 */
bool MCSource::SetFlux(const double& Flux) 
{
  if (m_SpectralType == c_NormalizedEnergyBeamFluxFunction || m_BeamType == c_FarFieldNormalizedEnergyBeamFluxFunction) {
    mout<<m_Name<<": The beam NormalizedEnergyBeamFluxFunction doesn't need a flux since it is already normalized!"<<endl;
    return false;
  }
  
  if (Flux > 0) {
    m_InputFlux = Flux;

    return UpgradeFlux();
  }

  return false;
}


/******************************************************************************
 * Return true, if the intensity could be set correctly
 */
bool MCSource::UpgradeFlux() 
{ 
  if (m_IsIsotopeCount == true) return true;

  m_Flux = m_InputFlux;

  if (m_CoordinateSystem == c_FarField) {
    m_Flux *= m_StartAreaAverageArea;
  } else {
    if (m_BeamType == c_NearFieldRestrictedPoint) {
      if (m_StartAreaType != c_StartAreaSphere && m_StartAreaType != c_StartAreaUnknown) {
        mout<<m_Name<<": The beam type NearFieldRestrictedPoint requires a sphere as start area!"<<endl;
        return false;
      }

      // If the position is outside the sphere:
      if ((m_StartAreaPosition-G4ThreeVector(m_PositionParam1, m_PositionParam2, m_PositionParam3)).mag() > m_StartAreaParam1) {
        // NearFieldRestrictedPoint is a special case of NearFieldPoint
        // We emit only into a special angle thus we have to downscale the flux
        double ConeAngle = asin(m_StartAreaParam1/(m_StartAreaPosition-G4ThreeVector(m_PositionParam1, m_PositionParam2, m_PositionParam3)).mag());
        double Area = 2*c_Pi*(1-cos(ConeAngle));
        m_Flux *= Area/(4*c_Pi);
      }
    }
  }

  if (m_Flux > 0 && !isnan(m_Flux)) {
    cout<<m_Name<<": Final flux: "<<m_Flux/m_StartAreaAverageArea*second<<" ph/sec"<<endl;
  }

  if (m_Flux <= 0 && m_StartAreaType != c_StartAreaUnknown && m_IsSuccessor == false) {
    mout<<m_Name<<": We do not have a positive flux ("<<m_Flux<<")... Reason unknown... aborting..."<<endl;
    abort();
    return false;
  }

  return true;
}


/******************************************************************************
 * Return true, if the intensity could be set correctly
 */
bool MCSource::SetTotalEnergyFlux(const double& TotalEnergyFlux) 
{ 
  if (TotalEnergyFlux > 0) {
    m_TotalEnergyFlux = TotalEnergyFlux;
    return true;
  }

  return false;
}

   
/******************************************************************************
 *  Return true, if the total energy flux (energy/cm^2) could be set correctly
 */
bool MCSource::SetLightCurve(const double& BinWidth, const double& Offset, 
                             const vector<double> Curve) 
{
  double Content = 0.0;
  for (unsigned int i = 0; i < Curve.size(); ++i) {
    Content += Curve[i];
  }

  if (Content > 0 && BinWidth > 0 && Curve.size() > 0 && Offset >= 0) {
    m_BinWidthLightCurve = BinWidth;
    m_LightCurveOffset = Offset;
    m_LightCurve = Curve; 
    m_IsFluxVariable = true;
    m_NEventsPerLightCurveContent = 0;
    return true;
  } else {
    return false;
  } 
}


/******************************************************************************
 * Set the polarization, the first parameter indicates, if the vector is
 * in absolute coordinates (true) or relative to the flight direction (false)
 */
bool MCSource::SetPolarization(const bool Absolute, const double x, 
                               const double y, const double z, 
                               const double Degree)
{
  m_PolarizationIsAbsolute = Absolute;
  m_Polarization = G4ThreeVector(x, y, z);
  m_PolarizationDegree = Degree;

  return true;
}


/******************************************************************************
 * Set an entry of the event list
 */  
bool MCSource::AddToEventList(double Energy, 
                              G4ThreeVector Position, 
                              G4ThreeVector Direction, 
                              G4ThreeVector Polarization, 
                              double Time,  
                              G4ParticleDefinition* ParticleType, 
                              MString VolumeName)
{
  // If this gets really slow we might use a set here at one point in time...

  //cout<<"Size: "<<m_EventListTime.size()<<endl;

  unsigned int Max = 10000000;
  if (m_EventListSize >= Max) {
    mout<<"Event list too large (exeeds "<<Max<<"). Last event eliminated (t="<<m_EventListTime.back()/s<<" sec)"<<endl;
    m_EventListEnergy.pop_back();
    m_EventListPosition.pop_back();
    m_EventListDirection.pop_back();
    m_EventListPolarization.pop_back();
    m_EventListTime.pop_back();
    m_EventListParticleType.pop_back();
    m_EventListVolumeName.pop_back();
    m_EventListSize--;
  }

  // Find the position where to add the event
  // Since we have bidirectional iterators, we cannot just add an offset to them
  list<double>::iterator EnergyIter;
  list<G4ThreeVector>::iterator PositionIter;
  list<G4ThreeVector>::iterator DirectionIter;
  list<G4ThreeVector>::iterator PolarizationIter;
  list<double>::iterator TimeIter;
  list<G4ParticleDefinition*>::iterator ParticleTypeIter;
  list<MString>::iterator VolumeNameIter;
  for (EnergyIter = m_EventListEnergy.begin(), 
         PositionIter = m_EventListPosition.begin(), 
         DirectionIter = m_EventListDirection.begin(), 
         PolarizationIter = m_EventListPolarization.begin(), 
         TimeIter = m_EventListTime.begin(), 
         ParticleTypeIter = m_EventListParticleType.begin(),
         VolumeNameIter = m_EventListVolumeName.begin(); 
       TimeIter != m_EventListTime.end(); 
       ++EnergyIter, ++PositionIter, ++DirectionIter, ++PolarizationIter, ++TimeIter, ++ParticleTypeIter, ++VolumeNameIter) {
    if (Time < *TimeIter) {
      m_EventListEnergy.insert(EnergyIter, Energy);
      m_EventListPosition.insert(PositionIter, Position);
      m_EventListDirection.insert(DirectionIter, Direction);
      m_EventListPolarization.insert(PolarizationIter, Polarization);
      m_EventListTime.insert(TimeIter, Time);
      m_EventListParticleType.insert(ParticleTypeIter, ParticleType);
      m_EventListVolumeName.insert(VolumeNameIter, VolumeName);
      m_EventListSize++;
      return true;
    }
  }

  m_EventListEnergy.push_back(Energy);
  m_EventListPosition.push_back(Position);
  m_EventListDirection.push_back(Direction);
  m_EventListPolarization.push_back(Polarization);
  m_EventListTime.push_back(Time);
  m_EventListParticleType.push_back(ParticleType);
  m_EventListVolumeName.push_back(VolumeName);
  m_EventListSize++;

  return true;
}


/******************************************************************************
 * Set the isotope count - promotes this source to an isotope count
 */
bool MCSource::SetIsotopeCount(double IsotopeCount) 
{ 
  if (IsotopeCount <= 0) {
    mout<<m_Name<<": Set isotope count requires a positive isotope count!"<<endl;    
    return false;
  }
  if (IsotopeCount > pow(2.0, numeric_limits<double>::digits)) {
    mout<<m_Name<<": The isotope count is too high ("<<IsotopeCount<<", max: "<<pow(2.0, numeric_limits<double>::digits)<<") to allow an event-by-event simulation!"<<endl;    
    return false;
  }

  m_IsIsotopeCount = true; 
  m_IsotopeCount = IsotopeCount; 
  
  return true; 
}


/******************************************************************************
 * Return the time to the next photon emission of this source:
 * The error in this routine is <= Scale
 */
bool MCSource::CalculateNextEmission(double Time, double Scale)
{
  double NextEmission = 0;
  double MinimumTime = numeric_limits<double>::max();

  if (m_IsEventList == true) {
    if (m_EventListSize > 0) {
      NextEmission = m_EventListTime.front() - Time;
    } else {
      NextEmission = numeric_limits<double>::max();
    }
  } else if (m_IsIsotopeCount == true) {
    if (m_ParticleDefinition == 0) GenerateParticleDefinition();
    double Random = 0.0;
    while (Random == 0.0) Random = CLHEP::RandFlat::shoot(1);
  
    NextEmission = -log(Random)/log(2)*m_HalfLife/m_IsotopeCount;
    m_IsotopeCount -= 1.0;
  
  } else if (m_IsFluxVariable == false) {
    // This is the main time loop:
    massert(m_Flux > 0);

    // A random exponential can be used to simulate
    // random arrival times of the photons (Poisson Arrival Model)
    //NextEmission = gRandom->Exp(1.0/GetFlux());
    NextEmission = CLHEP::RandExponential::shoot(1.0/GetFlux());
    
    // old version:
    //MinimumTime = Scale*1.0/GetFlux();
    //do {
    //  NextEmission += MinimumTime;
    //} while (CLHEP::RandFlat::shoot(1) > Scale);
  } else {
    massert(m_LightCurve.size() > 0);
    massert(m_BinWidthLightCurve > 0);
    massert(m_TotalEnergyFlux > 0);

    if (m_NEventsPerLightCurveContent <= 0) {
      // Initialize:
      double NLightCurveContentBins = 0;
      for (unsigned int i = 0; i < m_LightCurve.size(); ++i) {
        NLightCurveContentBins += m_LightCurve[i];
      }
      //cout<<"Mean= "<<GetMeanEnergy()<<" Cont="<<NLightCurveContentBins<<endl;
      m_NEventsPerLightCurveContent = 
        m_TotalEnergyFlux / (GetMeanEnergy() * NLightCurveContentBins);
    }
    //cout<<"m_NEventsPerLightCurveContent: "<<m_NEventsPerLightCurveContent<<endl;
    
    if (Time < m_LightCurveOffset) {
      NextEmission += m_LightCurveOffset - Time;
    }
    bool Found = false;
    for (unsigned int t = (unsigned int) ((Time-m_LightCurveOffset+NextEmission)/m_BinWidthLightCurve); 
         t < m_LightCurve.size(); ++t) {
      //cout<<"Time: "<<Time<<"   t/tmax "<<t<<"/"<<m_LightCurve.size()<<endl;
      Found = false;
      // Remaining time in this light curve bin:
      double Progress = 1 - ((Time-m_LightCurveOffset+NextEmission)/m_BinWidthLightCurve - 
                             (int) ((Time-m_LightCurveOffset+NextEmission)/m_BinWidthLightCurve));
      //cout<<"Progress:"<<Progress<<endl;
      // Remaining events in this light curve bin:
      double Events = m_BinWidthLightCurve*m_LightCurve[t]*m_NEventsPerLightCurveContent*Progress*
        m_StartAreaParam1*m_StartAreaParam1*c_Pi;
      //cout<<"Start sphere: "<<m_StartAreaParam1*m_StartAreaParam1*c_Pi<<endl;
      //cout<<"Remaining events for light curve bin "<<t<<":"<<Events<<endl;
      // Number of required steps:
      int Steps = (int) (Events/Scale + 1);
      //cout<<"Steps:"<<Steps<<endl;
      // The time of one step:
      if (Steps > 0) {
        MinimumTime = Progress*m_BinWidthLightCurve/Steps;
        for (int s = 0; s < Steps; ++s) {
          //cout<<"Step : "<<s<<endl;
          NextEmission += MinimumTime;
          if (CLHEP::RandFlat::shoot(1) < Scale) {
            Found = true;
            //cout<<"Found!"<<endl;
            break;
          }
        }
        if (Found == true) {
          break;
        }
      } else {
        NextEmission += Progress*m_BinWidthLightCurve;
      }
    }
    if (Found == false) {
      m_IsActive = false;
      NextEmission = numeric_limits<double>::max();
    }
  }

  m_NextEmission = NextEmission + Time;
  
  return true;
}


/******************************************************************************
 * Generate a particle in the particle gun
 */
bool MCSource::GenerateParticle(G4GeneralParticleSource* ParticleGun)
{

  if (m_IsEventList == true) {
    if (m_EventListSize > 0) {
      ParticleGun->SetParticleDefinition(m_EventListParticleType.front());
      m_ParticleType = MCSteppingAction::GetParticleId(m_EventListParticleType.front()->GetParticleName());
      return true;
    } else {
      return false;
    }
  }

  if (m_ParticleDefinition != 0) {
    ParticleGun->SetParticleDefinition(m_ParticleDefinition);
  } else {
    merr<<"No particle definition found: "<<m_Name<<endl;    
    return false;
  }
  
  return true;
}


/******************************************************************************
 * Generate the particle definition
 */
bool MCSource::GenerateParticleDefinition()
{
  bool Ret = true;

  if (m_ParticleDefinition != 0) return true;

  if (m_IsEventList == true) {
    // We already have a particle definiton
    m_ParticleDefinition = 0;
  }

  // Check for 
  int Type = m_ParticleType;
  if (Type > 1000) {
    G4ParticleDefinition* Ion = 0;
    G4ParticleTable* ParticleTable = G4ParticleTable::GetParticleTable();
    int AtomicNumber = int(Type/1000);
    int AtomicMass = Type - int(Type/1000)*1000;
    Ion = ParticleTable->GetIon(AtomicNumber, AtomicMass, m_ParticleExcitation);
    if (Ion == 0) {
      merr<<"Particle type not yet implemented!"<<endl;
      m_ParticleDefinition = 0;
      Ret = false;
    } else {
      m_ParticleDefinition = Ion;
    }
  } else {
    switch (Type) {
    case c_Gamma:
      m_ParticleDefinition = G4Gamma::Gamma();
      break;

    case c_Positron:
      m_ParticleDefinition = G4Positron::Positron();
      break;
    case c_Electron:
      m_ParticleDefinition = G4Electron::Electron();
      break;
      
    case c_Proton:
      m_ParticleDefinition = G4Proton::Proton();
      break;
    case c_AntiProton:
      m_ParticleDefinition = G4AntiProton::AntiProton();
      break;
    case c_Neutron:
      m_ParticleDefinition = G4Neutron::Neutron();
    break;
    case c_AntiNeutron:
      m_ParticleDefinition = G4AntiNeutron::AntiNeutron();
      break;
      
    case c_MuonPlus:
      m_ParticleDefinition = G4MuonPlus::MuonPlus();
      break;
    case c_MuonMinus:
      m_ParticleDefinition = G4MuonMinus::MuonMinus();
      break;
    case c_TauonPlus:
      m_ParticleDefinition = G4TauPlus::TauPlus();
      break;
    case c_TauonMinus:
      m_ParticleDefinition = G4TauMinus::TauMinus();
      break;
      
    case c_ElectronNeutrino:
      m_ParticleDefinition = G4NeutrinoE::NeutrinoE();
      break;
    case c_AntiElectronNeutrino:
      m_ParticleDefinition = G4AntiNeutrinoE::AntiNeutrinoE();
      break;
    case c_MuonNeutrino:
      m_ParticleDefinition = G4NeutrinoMu::NeutrinoMu();
      break;
    case c_AntiMuonNeutrino:
      m_ParticleDefinition = G4AntiNeutrinoMu::AntiNeutrinoMu();
      break;
    case c_TauonNeutrino:
      m_ParticleDefinition = G4NeutrinoTau::NeutrinoTau();
      break;
    case c_AntiTauonNeutrino:
      m_ParticleDefinition = G4AntiNeutrinoTau::AntiNeutrinoTau();
      break;

    case c_Deuteron:
      m_ParticleDefinition = G4Deuteron::Deuteron();
      break;
    case c_Triton:
      m_ParticleDefinition = G4Triton::Triton();
      break;
    case c_He3:
      m_ParticleDefinition = G4He3::He3();
      break;
    case c_Alpha:
      m_ParticleDefinition = G4Alpha::Alpha();
      break;

    case c_GenericIon:
      m_ParticleDefinition = G4GenericIon::GenericIon();
      break;
      
    case c_PiPlus:
      m_ParticleDefinition = G4PionPlus::PionPlus();
      break;
    case c_PiZero:
      m_ParticleDefinition = G4PionZero::PionZero();
      break;
    case c_PiMinus:
      m_ParticleDefinition = G4PionMinus::PionMinus();
      break;
    case c_Eta:
      m_ParticleDefinition = G4Eta::Eta();
      break;
    case c_EtaPrime:
      m_ParticleDefinition = G4EtaPrime::EtaPrime();
      break;

    case c_KaonPlus:
      m_ParticleDefinition = G4KaonPlus::KaonPlus();
      break;
    case c_KaonZero:
      m_ParticleDefinition = G4KaonZero::KaonZero();
      break;
    case c_AntiKaonZero:
      m_ParticleDefinition = G4AntiKaonZero::AntiKaonZero();
      break;
    case c_KaonZeroS:
      m_ParticleDefinition = G4KaonZeroShort::KaonZeroShort();
      break;
    case c_KaonZeroL:
      m_ParticleDefinition = G4KaonZeroLong::KaonZeroLong();
      break;
    case c_KaonMinus:
      m_ParticleDefinition = G4KaonMinus::KaonMinus();
      break;

    case c_Lambda:
      m_ParticleDefinition = G4Lambda::Lambda();
      break;
    case c_AntiLambda:
      m_ParticleDefinition = G4AntiLambda::AntiLambda();
      break;

    case c_SigmaPlus:
      m_ParticleDefinition = G4SigmaPlus::SigmaPlus();
      break;
    case c_AntiSigmaPlus:
      m_ParticleDefinition = G4AntiSigmaPlus::AntiSigmaPlus();
      break;
    case c_SigmaZero:
      m_ParticleDefinition = G4SigmaZero::SigmaZero();
      break;
    case c_AntiSigmaZero:
      m_ParticleDefinition = G4AntiSigmaZero::AntiSigmaZero();
      break;
    case c_SigmaMinus:
      m_ParticleDefinition = G4SigmaMinus::SigmaMinus();
      break;
    case c_AntiSigmaMinus:
      m_ParticleDefinition = G4AntiSigmaMinus::AntiSigmaMinus();
      break;

    case c_XiZero:
      m_ParticleDefinition = G4XiZero::XiZero();
      break;
    case c_AntiXiZero:
      m_ParticleDefinition = G4AntiXiZero::AntiXiZero();
      break;
    case c_XiMinus:
      m_ParticleDefinition = G4XiMinus::XiMinus();
      break;
    case c_AntiXiMinus:  
      m_ParticleDefinition = G4AntiXiMinus::AntiXiMinus();
      break;

    case c_OmegaMinus:
      m_ParticleDefinition = G4OmegaMinus::OmegaMinus();
      break;
    case c_AntiOmegaMinus:
      m_ParticleDefinition = G4AntiOmegaMinus::AntiOmegaMinus();
      break;
      
    default:
      m_ParticleDefinition = 0;
      merr<<"Particle type not yet implemented: "<<Type<<endl;
      Ret = false;
      break;
    }
  }
  
  if (Ret == false) return false;
  
  m_HalfLife = 0;
  double Exitation = 0;
  MCActivator A;
  Ret = A.DetermineHalfLife(m_ParticleDefinition, m_HalfLife, Exitation, true);

  return Ret;
}


/******************************************************************************
 * Return the mean produced energy:
 */
double MCSource::GetMeanEnergy() const
{
  double Energy = 0.0;
  double p1, p2, p3, p4;

  switch (m_SpectralType) {
  case c_Monoenergetic:
    Energy = m_EnergyParam1;
    break;
  case c_Linear:
    Energy = 0.5*(m_EnergyParam1+m_EnergyParam2);
    break;
  case c_PowerLaw: 
    if (m_EnergyParam3 == 2) {
      Energy = 
        (log(m_EnergyParam2)-log(m_EnergyParam1))/
        (pow(m_EnergyParam2,-m_EnergyParam3+1)/(-m_EnergyParam3+1)-
         pow(m_EnergyParam1,-m_EnergyParam3+1)/(-m_EnergyParam3+1));
    } else if (m_EnergyParam3 == 1) {
      Energy = 
        (pow(m_EnergyParam2,-m_EnergyParam3+2)/(-m_EnergyParam3+2)-
         pow(m_EnergyParam1,-m_EnergyParam3+2)/(-m_EnergyParam3+2))/
        (log(m_EnergyParam2)-log(m_EnergyParam1));
    } else {
      Energy = 
        (pow(m_EnergyParam2,-m_EnergyParam3+2)/(-m_EnergyParam3+2)-
         pow(m_EnergyParam1,-m_EnergyParam3+2)/(-m_EnergyParam3+2))/
        (pow(m_EnergyParam2,-m_EnergyParam3+1)/(-m_EnergyParam3+1)-
         pow(m_EnergyParam1,-m_EnergyParam3+1)/(-m_EnergyParam3+1));
    }
    break;
  case c_BrokenPowerLaw:

    if (m_EnergyParam4 == 2) {
      p1 = log(m_EnergyParam3)-log(m_EnergyParam1);
    } else {
      p1 = pow(m_EnergyParam3,-m_EnergyParam4+2)/(-m_EnergyParam4+2)-
        pow(m_EnergyParam1,-m_EnergyParam4+2)/(-m_EnergyParam4+2);
    }
    if (m_EnergyParam5 == 2) {
      p2 = log(m_EnergyParam2)-log(m_EnergyParam3);
    } else {
      p2 = pow(m_EnergyParam2,-m_EnergyParam5+2)/(-m_EnergyParam5+2)-
        pow(m_EnergyParam3,-m_EnergyParam5+2)/(-m_EnergyParam5+2);
    }
    if (m_EnergyParam4 == 1) {
      p3 = log(m_EnergyParam3)-log(m_EnergyParam1);
    } else {
      p3 = pow(m_EnergyParam3,-m_EnergyParam4+1)/(-m_EnergyParam4+1)-
        pow(m_EnergyParam1,-m_EnergyParam4+1)/(-m_EnergyParam4+1);
    }
    if (m_EnergyParam5 == 1) {
      p4 = log(m_EnergyParam2)-log(m_EnergyParam3);
    } else {
      p4 = pow(m_EnergyParam2,-m_EnergyParam5+1)/(-m_EnergyParam5+1)-
        pow(m_EnergyParam3,-m_EnergyParam5+1)/(-m_EnergyParam5+1);
    }

    Energy = (p1+m_EnergyParam6*p2)/(p3+m_EnergyParam6*p4);
    
    break;
  case c_Gaussian:
    Energy = m_EnergyParam1;
    break;
  case c_ThermalBremsstrahlung:
    mimp<<"GetMeanNotYetImplemented!"<<show;
    break;
  case c_BlackBody:
    mimp<<"GetMeanNotYetImplemented!"<<show;
    break;
  case c_BandFunction:
    mimp<<"GetMeanNotYetImplemented!"<<show;
    break;
  case c_FileDifferentialFlux:
    mimp<<"GetMeanNotYetImplemented!"<<show;
    break;
  case c_NormalizedEnergyBeamFluxFunction:
    mimp<<"GetMeanNotYetImplemented!"<<show;
    break;    
  default:
    merr<<"Energy type not yet implemented!"<<show;
    Energy = 2.0*MeV;
    return false;
    break;
  }

  return Energy;
}



/******************************************************************************
 * Generate an inititial energy in the particle gun
 */
bool MCSource::GenerateEnergy(G4GeneralParticleSource* ParticleGun)
{
  double Probability = 0.0;
  double Energy = 0.0;

	massert(ParticleGun->GetNumberofSource() == 1);

  if (m_IsEventList == true) {
    if (m_EventListSize > 0) {
      ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
      ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
      ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
      ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(m_EventListEnergy.front());
      return true;
    } else {
      return false;
    }
  }
  
  switch (m_SpectralType) {
  case c_Monoenergetic:
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(m_EnergyParam1);
    break;
  case c_Linear:
    // This function does definitely not behave as expected....
    // ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Lin");
    // ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(m_EnergyParam1);
    // ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(m_EnergyParam2);

    Energy = m_EnergyParam1 + 
      CLHEP::RandFlat::shoot(1)*(m_EnergyParam2-m_EnergyParam1);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_PowerLaw:
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Pow");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(m_EnergyParam1);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(m_EnergyParam2);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetAlpha(-m_EnergyParam3);
    break;
  case c_BrokenPowerLaw:
    // This algorithm can be very slow...
    while (true) {
      Energy = m_EnergyParam1 + CLHEP::RandFlat::shoot(1)*(m_EnergyParam2-m_EnergyParam1);
      if (Energy > m_EnergyParam3) {
        Probability = 
          m_EnergyParam6*pow(m_EnergyParam1, m_EnergyParam4)/pow(Energy, m_EnergyParam5);
      } else {
        Probability = pow(m_EnergyParam1/Energy, m_EnergyParam4);
      }
      if (CLHEP::RandFlat::shoot(1) < Probability) break;
    }
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_Gaussian:
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(gRandom->Gaus(m_EnergyParam1, m_EnergyParam2));
    break;
  case c_ThermalBremsstrahlung:
    while (true) {
      Energy = m_EnergyParam1 + CLHEP::RandFlat::shoot(1)*(m_EnergyParam2-m_EnergyParam1);
      if (CLHEP::RandFlat::shoot(1) <= 
          (1.0/Energy * exp(-Energy/m_EnergyParam3))/
          (1.0/m_EnergyParam1 * exp(-m_EnergyParam1/m_EnergyParam3))) break;
    }    
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_BlackBody:
    while (true) {
      Energy = m_EnergyParam1 + CLHEP::RandFlat::shoot(1)*(m_EnergyParam2-m_EnergyParam1);
      if (CLHEP::RandFlat::shoot(1) <= BlackBody(Energy, m_EnergyParam3)/m_EnergyParam4) break;
    }    
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_BandFunction:
    while (true) {
      Energy = m_EnergyParam1 + CLHEP::RandFlat::shoot(1)*(m_EnergyParam2-m_EnergyParam1);
      double BandValue = BandFunction(Energy, m_EnergyParam3, m_EnergyParam4, m_EnergyParam5);
      if (BandValue > m_EnergyParam6) {
        mout<<"Precalculated maximum of band function is wrong!!"<<endl;
      }
      if (CLHEP::RandFlat::shoot(1) <= BandValue/m_EnergyParam6) break;
    }    
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_FileDifferentialFlux:
    // All functionality is in MFunction:
    Energy = m_EnergyFunction.GetRandom();
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_NormalizedEnergyBeamFluxFunction:
    // Most of the functionality is in MFunction3DSpherical:
    // We temporarily store the position here in m_PositionParam1, m_PositionParam2
    if (m_StartAreaType == c_StartAreaSphere) {
      m_NormalizedEnergyBeamFluxFunction.GetRandom(m_PositionParam2, m_PositionParam1, Energy);
    } else if (m_StartAreaType == c_StartAreaTube) {
      double Area = 0.0;
      double AngleMaxArea = atan(m_StartAreaParam2/m_StartAreaParam1);
      double MaxArea = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(AngleMaxArea) + m_StartAreaParam1*fabs(cos(AngleMaxArea)));
      
      double xMax = m_NormalizedEnergyBeamFluxFunction.GetXMax();
      double yMax = m_NormalizedEnergyBeamFluxFunction.GetYMax();
      double zMax = m_NormalizedEnergyBeamFluxFunction.GetZMax();
      double vMax = m_NormalizedEnergyBeamFluxFunction.GetVMax();
      double xMin = m_NormalizedEnergyBeamFluxFunction.GetXMin();
      double yMin = m_NormalizedEnergyBeamFluxFunction.GetYMin();
      double zMin = m_NormalizedEnergyBeamFluxFunction.GetZMin();
      
      double v = 0;
      do {
        do {
          m_PositionParam1 = acos(cos(yMin*c_Rad) - gRandom->Rndm()*(cos(yMin*c_Rad) - cos(yMax*c_Rad)))*c_Deg;
          Area = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(m_PositionParam1*c_Rad) + m_StartAreaParam1*fabs(cos(m_PositionParam1*c_Rad)));
          //cout<<Area<<":"<<MaxArea<<endl;
        } while (gRandom->Rndm() > Area/MaxArea);
        
        m_PositionParam2 = gRandom->Rndm()*(xMax - xMin) + xMin;
        Energy = gRandom->Rndm()*(zMax - zMin) + zMin;

        v = m_NormalizedEnergyBeamFluxFunction.Eval(m_PositionParam2, m_PositionParam1, Energy);
      } while (vMax*gRandom->Rndm() > v);
    } else {
      mout<<m_Name<<": Unknown start area type for position generation"<<endl;
    }
    
    Energy *= keV;
    m_PositionParam1 *= c_Rad;
    m_PositionParam2 *= c_Rad;
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    break;
  case c_Activation:
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(0.0);
    break;
  default:
    merr<<"Energy type not yet implemented: "<<m_SpectralType<<endl;
    Energy = 2.0*MeV;
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEnergyDisType("Mono");
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmin(0*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetEmax(1E30*keV);
    ParticleGun->GetCurrentSource()->GetEneDist()->SetMonoEnergy(Energy);
    return false;
    break;
  }
  
  return true;
}


/******************************************************************************
 * Generate an inititial position in the particle gun
 */
bool MCSource::GeneratePosition(G4GeneralParticleSource* Gun)
{
  G4ThreeVector Position;
  G4ThreeVector Direction;

  if (m_IsEventList == true) {
    if (m_EventListSize > 0) {
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(m_EventListPosition.front());      
      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(m_EventListDirection.front());
      return true;
    } else {
      return false;
    }
  }

	if (m_CoordinateSystem == c_FarField) {
    double Theta = 0.0;
    double Phi = 0.0;
    if (m_BeamType == c_FarFieldPoint || 
        m_BeamType == c_FarFieldArea ||
        m_BeamType == c_FarFieldFileZenithDependent ||
        m_BeamType == c_FarFieldNormalizedEnergyBeamFluxFunction) {
      if (m_BeamType == c_FarFieldPoint || m_BeamType == c_FarFieldNormalizedEnergyBeamFluxFunction) {
        // Fixed start direction or temporarily stored in these parameteres
        Theta = m_PositionParam1;
        Phi = m_PositionParam2;
      } else if (m_BeamType == c_FarFieldArea) {

        // Determine start direction randomly between the theta and phi limits
        if (m_StartAreaType == c_StartAreaSphere) {
          Theta = acos(cos(m_PositionParam1) - CLHEP::RandFlat::shoot(1)*(cos(m_PositionParam1) - cos(m_PositionParam2)));
          Phi = m_PositionParam3 + CLHEP::RandFlat::shoot(1)*(m_PositionParam4 - m_PositionParam3);
        } else if (m_StartAreaType == c_StartAreaTube) {
          double Area = 0.0;
          double AngleMaxArea = atan(m_StartAreaParam2/m_StartAreaParam1);
          double MaxArea = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(AngleMaxArea) + m_StartAreaParam1*fabs(cos(AngleMaxArea)));
          while (true) {
            Theta = acos(cos(m_PositionParam1) - CLHEP::RandFlat::shoot(1)*(cos(m_PositionParam1) - cos(m_PositionParam2)));
            Area = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(Theta) + m_StartAreaParam1*fabs(cos(Theta)));
            if (CLHEP::RandFlat::shoot(1) <= Area/MaxArea) {
              Phi = m_PositionParam3 + CLHEP::RandFlat::shoot(1)*(m_PositionParam4 - m_PositionParam3);
              break;
            }
          }    

        } else {
          mout<<m_Name<<": Unknown start area type for position generation"<<endl;
        }

      } else if (m_BeamType == c_FarFieldFileZenithDependent) {
        // Determine a random position on the sphere between 
        // theta min and theta max in the file:

        if (m_StartAreaType == c_StartAreaSphere) {
          while (true) {
            Theta = acos(cos(m_PositionParam1) - CLHEP::RandFlat::shoot(1)*(cos(m_PositionParam1) - cos(m_PositionParam2)));
            Phi = CLHEP::RandFlat::shoot(1)*360*deg;
            if (CLHEP::RandFlat::shoot(1) <= m_PositionFunction.Eval(Theta)/m_PositionParam3) {
              break;
            }
          }    
        } else if (m_StartAreaType == c_StartAreaTube) {
          double Area = 0.0;
          double AngleMaxArea = atan(m_StartAreaParam2/m_StartAreaParam1);
          double MaxArea = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(AngleMaxArea) + m_StartAreaParam1*fabs(cos(AngleMaxArea)));
          while (true) {
            Theta = acos(cos(m_PositionParam1) - CLHEP::RandFlat::shoot(1)*(cos(m_PositionParam1) - cos(m_PositionParam2)));
            if (CLHEP::RandFlat::shoot(1) <= m_PositionFunction.Eval(Theta)/m_PositionParam3) {
              Area = 4*m_StartAreaParam1*(m_StartAreaParam2*sin(Theta) + m_StartAreaParam1*fabs(cos(Theta)));
              if (CLHEP::RandFlat::shoot(1) <= Area/MaxArea) {
                Phi = CLHEP::RandFlat::shoot(1)*360*deg;
                break;
              }
            }
          }
        } else {
          mout<<m_Name<<": Unknown start area type for position generation"<<endl;
        }
      }
      
      // Generate start direction:
      Direction.setRThetaPhi(1, Theta, Phi);
      Direction = -Direction;

      double x = 0.0, y = 0.0, z = 0.0;
      // Determine start position on disk on sphere
      if (m_StartAreaType == c_StartAreaSphere) {
        // Disk is currently fixed on (0, 0, Radius)
        while (true) {
          x = m_StartAreaParam1*(2*(CLHEP::RandFlat::shoot(1)-0.5));
          y = m_StartAreaParam1*(2*(CLHEP::RandFlat::shoot(1)-0.5));
          if (sqrt(x*x+y*y) <= m_StartAreaParam1) break;
        }
        z = m_StartAreaParam1;
      } 
      // Determine start position on square on the tube
      else if (m_StartAreaType == c_StartAreaTube) {
        x = (m_StartAreaParam2 * sin(Theta) + m_StartAreaParam1*fabs(cos(Theta))) * (2*(CLHEP::RandFlat::shoot(1)-0.5));
        y = m_StartAreaParam1 * (2*(CLHEP::RandFlat::shoot(1)-0.5));
        z = sqrt(m_StartAreaParam1*m_StartAreaParam1 + m_StartAreaParam2*m_StartAreaParam2);
        //cout<<"Start pos:"<<x/cm<<":"<<y/cm<<":"<<z/cm<<endl;
      } else {
        mout<<m_Name<<": Unknown start area type for position generation"<<endl;
      }
     
      // Rotate according to theta and phi
      // left-handed rotation-matrix: first theta (rotation around y-axis) then phi (rotation around z-axis):
      // | +cosp -sinp 0 |   | +cost 0 +sint |   | x |
      // | +sinp +cosp 0 | * |   0   1   0   | * | y | 
      // |   0     0   1 |   | -sint 0 +cost |   | z |        
      Position[0] = (x*cos(Theta)+z*sin(Theta))*cos(Phi) - y*sin(Phi);
      Position[1] = (x*cos(Theta)+z*sin(Theta))*sin(Phi) + y*cos(Phi);
      Position[2] = -x*sin(Theta)+z*cos(Theta);
      
      // Translate sphere center
      Position += m_StartAreaPosition;
      
      //cout<<"Start pos: "<<Position/cm<<":"<<x/cm<<":"<<y/cm<<":"<<z/cm<<endl;

      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);      
      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);
    } 

    else if  (m_BeamType == c_FarFieldGaussian) {
      mout<<"Mode SphericalGaussian not yet implemented!"<<endl;
      return false;
    } 
  } 

  else if (m_CoordinateSystem == c_NearField) {

    // Start position:

    if (m_BeamType == c_NearFieldPoint || 
        m_BeamType == c_NearFieldRestrictedPoint ||
        m_BeamType == c_NearFieldDiffractionPoint ||
        m_BeamType == c_NearFieldDiffractionPointKSpace ||
        m_BeamType == c_NearFieldConeBeam ||
        m_BeamType == c_NearFieldConeBeamGauss) {
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(G4ThreeVector(m_PositionParam1, 
                                                                           m_PositionParam2, 
                                                                           m_PositionParam3));
    }

    else if  (m_BeamType == c_NearFieldLine) {
      double Random = CLHEP::RandFlat::shoot(1);
      Position[0] = m_PositionParam1 + 
        Random*(m_PositionParam4 - m_PositionParam1);
      Position[1] = m_PositionParam2 + 
        Random*(m_PositionParam5 - m_PositionParam2);
      Position[2] = m_PositionParam3 + 
        Random*(m_PositionParam6 - m_PositionParam3);
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);      
    } 

    else if  (m_BeamType == c_NearFieldBox) {
      Position[0] = m_PositionParam1 + 
        CLHEP::RandFlat::shoot(1)*(m_PositionParam4 - m_PositionParam1);
      Position[1] = m_PositionParam2 + 
        CLHEP::RandFlat::shoot(1)*(m_PositionParam5 - m_PositionParam2);
      Position[2] = m_PositionParam3 + 
        CLHEP::RandFlat::shoot(1)*(m_PositionParam6 - m_PositionParam3);
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);      
    } 

    else if  (m_BeamType == c_NearFieldSphere) {
      do {
        Position[0] = (CLHEP::RandFlat::shoot(1)-0.5)*2*m_PositionParam4;
        Position[1] = (CLHEP::RandFlat::shoot(1)-0.5)*2*m_PositionParam5;
        Position[2] = (CLHEP::RandFlat::shoot(1)-0.5)*2*m_PositionParam6;
      } while (Position[0]*Position[0]/(m_PositionParam4*m_PositionParam4) + 
               Position[1]*Position[1]/(m_PositionParam5*m_PositionParam5) + 
               Position[2]*Position[2]/(m_PositionParam6*m_PositionParam6) > 1);
      Position = G4ThreeVector(m_PositionParam1, 
                               m_PositionParam2, 
                               m_PositionParam3) + Position;
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);      
    } 

    else if (m_BeamType == c_NearFieldBeam ||
             m_BeamType == c_NearFieldBeam1DProfile ||
             m_BeamType == c_NearFieldBeam2DProfile ||
             m_BeamType == c_NearFieldDisk ||
             m_BeamType == c_NearFieldFlatMap) {
      // Create the circular beam:
      G4ThreeVector Temp;
      G4double Radius, Theta, Phi;

      if (m_BeamType == c_NearFieldBeam || 
          m_BeamType == c_NearFieldBeam1DProfile) {
        Phi = 2*c_Pi*CLHEP::RandFlat::shoot();
        if (m_BeamType == c_NearFieldBeam) {
          Radius = m_PositionParam7*sqrt(CLHEP::RandFlat::shoot());
        } else {
          // Using GetRandomTimesX multiplies the spectrum with x.
          // The means that the profile is correctly scaled to allow distributing the linear profile over the disc
          Radius = m_PositionFunction.GetRandomTimesX();
        }
        Temp[0] = Radius*cos(Phi);
        Temp[1] = Radius*sin(Phi);
        Temp[2] = 0.0;
      } else if (m_BeamType == c_NearFieldBeam2DProfile ||
                 m_BeamType == c_NearFieldFlatMap) {
        double x = 0, y = 0;
        m_PositionFunction2D.GetRandom(x, y);

        // Rotate around z-axis:
        // (a) convert into spherical coordinates
        Radius = sqrt(x*x + y*y);

        if (x == 0) {
          if (y > 0) {
            Phi = c_Pi/2;
          } else if (y < 0) {
            Phi = -c_Pi/2;
          } else {
            Phi = 0.0;
          }
        } else if (x > 0) {
          Phi = atan(y/x);
        } else if (x < 0) {
          if (y >= 0) {
            Phi = atan(y/x) + c_Pi;
          } else {
            Phi = atan(y/x) - c_Pi;
          }
        }

        // (b) add rotation
        Phi += m_PositionParam7; // Already in rad...

        // (c) convert back
        Temp[0] = Radius*cos(Phi);
        Temp[1] = Radius*sin(Phi);
        Temp[2] = 0.0;

      } else if (m_BeamType == c_NearFieldDisk) {
        Phi = m_PositionParam10 + CLHEP::RandFlat::shoot()*(m_PositionParam11-m_PositionParam10);
        Radius = sqrt(m_PositionParam7*m_PositionParam7 + (m_PositionParam8*m_PositionParam8-m_PositionParam7*m_PositionParam7)*CLHEP::RandFlat::shoot());

        Temp[0] = Radius*cos(Phi);
        Temp[1] = Radius*sin(Phi);
        Temp[2] = (gRandom->Rndm()-0.5)*m_PositionParam9;
      }
      
      // Rotate into normal direction (first around theta, then phi):
      Direction.setX(m_PositionParam4);
      Direction.setY(m_PositionParam5);
      Direction.setZ(m_PositionParam6);
      Theta = Direction.theta();
      Phi = Direction.phi();

      Position[0] = 
        (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*cos(Phi) - Temp[1]*sin(Phi);
      Position[1] = 
        (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*sin(Phi) + Temp[1]*cos(Phi);
      Position[2] = 
        -Temp[0]*sin(Theta)+Temp[2]*cos(Theta);
      
      // Translate:
      Position[0] += m_PositionParam1;
      Position[1] += m_PositionParam2;
      Position[2] += m_PositionParam3;    
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);
    } 

    else if (m_BeamType == c_NearFieldActivation ||
             m_BeamType == c_NearFieldVolume) {
      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Position = MCRunManager::GetMCRunManager()->GetDetectorConstruction()->GetRandomPosition(m_Volume);
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);      
    }


    // Start direction:

    if (m_BeamType == c_NearFieldPoint || 
        m_BeamType == c_NearFieldLine || 
        m_BeamType == c_NearFieldBox || 
        m_BeamType == c_NearFieldDisk || 
        m_BeamType == c_NearFieldSphere ||
        m_BeamType == c_NearFieldActivation ||
        m_BeamType == c_NearFieldVolume ||
        m_BeamType == c_NearFieldFlatMap) {
      Gun->GetCurrentSource()->GetAngDist()->SetAngDistType("iso");
    } 

    else if (m_BeamType == c_NearFieldDiffractionPoint) {
      double Theta, Phi;
      // We cannot use m_PositionFunction2D.GetRandom() here since GetRandom assumes Cartesian coordinate system...
      do {
        // Theta = acos(cos(m_PositionParam8) - (cos(m_PositionParam8)-cos(m_PositionParam9)) * CLHEP::RandFlat::shoot());
        // Phi = m_PositionParam10 + (m_PositionParam11-m_PositionParam10)*CLHEP::RandFlat::shoot();
        Theta = acos(1-2*CLHEP::RandFlat::shoot());
        Phi = 2*c_Pi*CLHEP::RandFlat::shoot();
      } while (m_PositionFunction2D.Eval(Theta, Phi) < m_PositionFunction2D.GetZMax()*CLHEP::RandFlat::shoot());
      
      // Add rotation
      Phi += m_PositionParam7; // Already in rad...

      G4ThreeVector Temp;
      Temp.setRThetaPhi(1.0, Theta, Phi);

      // Rotate into normal direction (first around theta, then phi):
      Direction.setX(m_PositionParam4);
      Direction.setY(m_PositionParam5);
      Direction.setZ(m_PositionParam6);
      Theta = Direction.theta();
      Phi = Direction.phi();

      Direction[0] = 
        (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*cos(Phi) - Temp[1]*sin(Phi);
      Direction[1] = 
        (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*sin(Phi) + Temp[1]*cos(Phi);
      Direction[2] = 
        -Temp[0]*sin(Theta)+Temp[2]*cos(Theta);

      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);

    }

    else if (m_BeamType == c_NearFieldDiffractionPointKSpace) {

      // To Be implemented

      Direction[0] = 0;
      Direction[1] = 0;
      Direction[2] = 1;

      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);
    }
      
    else if  (m_BeamType == c_NearFieldBeam ||
              m_BeamType == c_NearFieldBeam1DProfile ||
              m_BeamType == c_NearFieldBeam2DProfile) {
      Direction.setX(m_PositionParam4);
      Direction.setY(m_PositionParam5);
      Direction.setZ(m_PositionParam6);

      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);
    } 

    else if  (m_BeamType == c_NearFieldRestrictedPoint ||
              m_BeamType == c_NearFieldConeBeam ||
              m_BeamType == c_NearFieldConeBeamGauss) {

      // Initial remark: During SetPosition all parameters have been set in a way that 
      // CartesianRestrictedPoint is HERE identical with CartesianConeBeam

      // Determine theta, and phi of the beam direction: 
      G4ThreeVector BeamDirection;
      BeamDirection.set(m_PositionParam4, m_PositionParam5, m_PositionParam6);
      double BeamTheta = BeamDirection.theta();
      double BeamPhi = BeamDirection.phi();
      
      // Generate a random zenith angle Theta (in [0, m_PositionParam7]) and 
      // an azimuth angle Phi (in [0, 2pi]) relative to an on-axis cone-beam direction
      double Theta, Phi;
      if (m_BeamType == c_NearFieldRestrictedPoint ||
          m_BeamType == c_NearFieldConeBeam) {
        // We have a flat distribution in angle space
        Phi = 2*c_Pi * CLHEP::RandFlat::shoot(1);
        Theta = acos(1.0 - CLHEP::RandFlat::shoot(1) * (1.0 - cos(m_PositionParam7)));
      } else if (m_BeamType == c_NearFieldConeBeamGauss) {
        // We have a gaussian profile in angle space
        Phi = 2*c_Pi * CLHEP::RandFlat::shoot(1);
        Theta = m_PositionTF1->GetRandom();
      }

      // Now we have the random direction relative to an on-axis cone-beam
      G4ThreeVector DirectionOnAxis;
      DirectionOnAxis.setRThetaPhi(1.0, Theta, Phi);

      // Rotate this direction in beam direction:
      // Right-handed rotation-matrix: first BeamTheta (rotation around y-axis),
      // then BeamPhi (rotation around z-axis):
      // | +cosp -sinp 0 |   | +cost 0 +sint |   | x |
      // | +sinp +cosp 0 | * |   0   1   0   | * | y | 
      // |   0     0   1 |   | -sint 0 +cost |   | z |
      Direction[0] = (DirectionOnAxis[0]*cos(BeamTheta)+DirectionOnAxis[2]*sin(BeamTheta))*cos(BeamPhi) - DirectionOnAxis[1]*sin(BeamPhi);
      Direction[1] = (DirectionOnAxis[0]*cos(BeamTheta)+DirectionOnAxis[2]*sin(BeamTheta))*sin(BeamPhi) + DirectionOnAxis[1]*cos(BeamPhi);
      Direction[2] = -DirectionOnAxis[0]*sin(BeamTheta)+DirectionOnAxis[2]*cos(BeamTheta);

      // Give the information to the particle gun:
      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);
    }

    else if (m_BeamType == c_NearFieldIlluminatedDisk || 
             m_BeamType == c_NearFieldIlluminatedSquare) {

      if (m_StartAreaType != c_StartAreaSphere) {
        mout<<m_Name<<": NearFieldIlluminatedXXX requires a sphere as start area, not a tube!"<<endl;
        return false;
      }

      // Step 1: Generate a random position on the disk
      G4ThreeVector Temp;
      while (true) {
        Temp[0] = 2*CLHEP::RandFlat::shoot(m_PositionParam4) - m_PositionParam4;
        Temp[1] = 2*CLHEP::RandFlat::shoot(m_PositionParam4) - m_PositionParam4;
        Temp[2] = 0.0;

        // If we have a box we can exit here
        if (m_BeamType == c_NearFieldIlluminatedSquare) break;

        // ... otherwise check if we are on the disk
        if (sqrt(Temp[0]*Temp[0]+Temp[1]*Temp[1]) < m_PositionParam4) {
          break;
        }
      }
      

      // Step 2: Tilt the disk --- internally the stuff is already in rad
      double Theta = m_PositionParam5;
      double Phi = m_PositionParam6;
      
      // Rotation details:
      // left-handed rotation-matrix: first theta (rotation around y-axis) then phi (rotation around z-axis):
      // | +cosp -sinp 0 |   | +cost 0 +sint |   | x |
      // | +sinp +cosp 0 | * |   0   1   0   | * | y | 
      // |   0     0   1 |   | -sint 0 +cost |   | z |        

      Position[0] = (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*cos(Phi) - Temp[1]*sin(Phi);
      Position[1] = (Temp[0]*cos(Theta)+Temp[2]*sin(Theta))*sin(Phi) + Temp[1]*cos(Phi);
      Position[2] = -Temp[0]*sin(Theta)+Temp[2]*cos(Theta);

      // There shouldn't be any problem in this case with rotation ambiguities etc., 
      // since the normal vector of the disk points towards (0, 0, 1) in the beginning


      // Step 3: Translate disk center into expected disk position
      Position[0] += m_PositionParam1;
      Position[1] += m_PositionParam2;
      Position[2] += m_PositionParam3;    

      // Now we have a point which the beam has to intercept


      // Step 4: Project onto the surrounding sphere - not the disk:
      // Determine the intersection between a line and a sphere
      // Equation for line:   X = P + l*D 
      // Equation for sphere: (X-M)^2 = r^2

      // Solving for lambda results in two solutions:
      // l^2 * [D^2] + l * [2*D*(P-M)] + [(P-M)^2 - r^2] = 0
      //       := a        := b          := c

      // M := m_StartAreaPosition
      // D := Direction
      // P := Position

      Direction.setRThetaPhi(1, m_PositionParam7, m_PositionParam8);
      Direction = -Direction;

      double a = Direction.dot(Direction);
      double b = 2*Direction.dot(Position-m_StartAreaPosition);
      double c = (Position-m_StartAreaPosition).dot(Position-m_StartAreaPosition) - m_StartAreaParam1*m_StartAreaParam1;

      // Solutions:
      // l1,2 = (-b+-sqrt(b*b-4*a*c))/(2*a);

      // By definition of our line and direction D, the solution has to be the smaller lambda
      // Since a is by definition positive the "-" is the correct solution
      massert((-b+sqrt(b*b-4*a*c))/(2*a) > (-b-sqrt(b*b-4*a*c))/(2*a));

      double l = (-b-sqrt(b*b-4*a*c))/(2*a);

      // The final solution:
      Position += l*Direction;

      Gun->GetCurrentSource()->GetPosDist()->SetPosDisType("Point");
      Gun->GetCurrentSource()->GetPosDist()->SetCentreCoords(Position);
      Gun->GetCurrentSource()->GetAngDist()->SetParticleMomentumDirection(Direction);
    } 
  }

  return true;
}


/******************************************************************************
 * Generate an inititial polarization in the particle gun
 */
bool MCSource::GeneratePolarization(G4GeneralParticleSource* Gun)
{
  if (m_IsEventList == true) {
    if (m_EventListSize > 0) {
      Gun->SetParticlePolarization(m_EventListPolarization.front());
      return true;
    } else {
      return false;
    }
  }

  if (m_PolarizationIsAbsolute) {
    if (CLHEP::RandFlat::shoot(1) < m_PolarizationDegree) {
      Gun->SetParticlePolarization(m_Polarization);
    } else {
      Gun->SetParticlePolarization(G4ThreeVector(0.0, 0.0, 0.0));
    }
  } else {
    merr<<"Relative polarization not yet implemented!"<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Shape of black body emission (Temperature in keV)
 */  
double MCSource::BlackBody(double Energy, double Temperature) const
{
  if (Energy == 0 || Temperature == 0) return 0.0;
  return (Energy*Energy)/(exp(Energy/Temperature)-1);
}


/******************************************************************************
 * Shape of a Band function
 */  
double MCSource::BandFunction(const double Energy, double Alpha, 
                              const double Beta, const double E0) const
{
  if (Energy == 0 || Alpha - Beta < 0) return 0.0;

  double Ebreak = (Alpha - Beta)*E0;
  if (Energy <= Ebreak) {
    return pow(Energy, Alpha)*exp(-Energy/E0);
  } else {
    return pow(Ebreak, Alpha-Beta)*exp(-Alpha+Beta)*pow(Energy, Beta);
  }
}


/*
 * MCSource.cc: the end...
 ******************************************************************************/
