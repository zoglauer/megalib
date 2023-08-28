/*
 * MCSource.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * This class represents a source with all its basic data and in addition
 * it can create generate the start conditions for the event
 *
 */

#ifndef ___MCSource___
#define ___MCSource___

// Geant4:
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"

// ROOT:
#include "TF1.h"

// Cosima:
#include "MCOrientation.hh"

// MEGAlib:
#include "MTime.h"
#include "MTokenizer.h"
#include "MFunction.h"
#include "MFunction2D.h"
#include "MFunction3DSpherical.h"

// Standard libs:
#include <vector>
#include <list>
#include <deque>
using namespace std;

// Forward declarations:
class G4GeneralParticleSource;


/******************************************************************************/

class MCSource
{
  // public interface:
public:
  /// Construction
  MCSource();
  /// Construction by name
  MCSource(MString Name);
  /// Default destructor
  virtual ~MCSource();
  
  /// Common initalization
  void Initialize();

  /// In the case this is the delayed events list, check if the source
  /// is compatible with the NEXT particle and if yes set an event to be skipped
  /// This has to be called *before* GenerateParticles
  bool GenerateSkippedEvents(MCSource* Source);

  /// Generate the particle of this source for the particle gun
  bool GenerateParticles(G4GeneralParticleSource* ParticleGun);

  /// Return the number of generated particles
  long GetNGeneratedParticles() const { return m_NGeneratedParticles; }

  /// Return the name of this source
  MString GetName() const { return m_Name; }

  /// Return the ID of this source
  inline unsigned int GetID() const { return m_ID; }

  /// Return true, if we have a variable flux:
  bool IsFluxVariable() const { return m_IsFluxVariable; }

  /// Calculate the next emission of this source
  bool CalculateNextEmission(MTime Time, double Scale);

  /// Return the time to the next photon emission of this source:
  MTime GetNextEmission() const { return m_NextEmission; }

  /// Set a certain amount of events which should not be started:
  void NEventsToSkip(int NEvents) { m_NEventsToSkip += NEvents; }

  /// Get the number of events which should not be started:
  unsigned int GetNEventsToSkip() const { return m_NEventsToSkip; }

  /// Get the number of events which already have been skipped:
  unsigned int GetNEventsSkipped() const { return m_NEventsSkipped; }

  /// Return true, if this source has still photons for the emission
  bool IsActive() const { return m_IsActive; }

  /// Make this source to an event list
  void SetEventList() { m_IsEventList = true; }
  /// Return true if this is an event list
  bool IsEventList() const { return m_IsEventList; }
  /// Make this source to a build up event list
  void SetBuildUpEventList() { m_IsBuildUpEventList = true; m_IsEventList = true; }
  /// Return true if this is a build up event list
  bool IsBuildUpEventList() const { return m_IsBuildUpEventList; }
  /// Return the current size of the event list -- more events could be added later or via reading from file and 
  unsigned int SizeEventList() const { return m_EventList.size(); }
  /// Add an entry of the event list -- will return false if we read the event list from file
  bool AddToEventList(double Energy, G4ThreeVector Position, G4ThreeVector Direction, 
                      G4ThreeVector Polarization, MTime Time,
                      G4ParticleDefinition* ParticleType, MString VolumeName);
  /// Return the next particle type in the event list
  G4ParticleDefinition* GetEventListNextParticle() { return (m_EventListSize > 0) ? m_EventList[0]->m_ParticleDefinition : nullptr; }
  /// Return the next volume in the event list
  MString GetEventListNextVolume() { return (m_EventListSize > 0) ? m_EventList[0]->m_VolumeName : ""; }

  /// Set entries of the event list from file 
  bool SetEventListFromFile(MString FileName);
  /// Set entries of the event list from file
  bool ContinueReadingEventList();
  
  
  
  /// Return true, if the coordinate system could be set correctly
  bool SetCoordinateSystem(const int& CoordinateSystem);
  /// Return the coordinate system type
  int GetCoordinateSystem() const { return m_CoordinateSystem; }

  /// Return true, if the spectral type could be set correctly
  bool SetSpectralType(const int& SpectralType);
  /// Return the spectral type
  int GetSpectralType() const { return m_SpectralType; }

  /// Return true, if the region type could be set correctly
  bool SetBeamType(const int& CoordinateSystem, const int& BeamType);
  /// Return the region type
  int GetBeamType() const { return m_BeamType; }

  /// Return true, if the particle type could be set correctly
  bool SetParticleType(const int& ParticleType);
  /// Return the particle type
  int GetParticleType() const { return m_ParticleType; }

  /// Return true, if the particle type could be set correctly
  bool SetParticleExcitation(const double& Excitation);
  /// Return the particle type
  double GetParticleExcitation() const { return m_ParticleExcitation; }

  /// Return the particle definition
  G4ParticleDefinition* GetParticleDefinition() const { return m_ParticleDefinition; }

  /// Return true, if the intensity could be set correctly
  bool SetFlux(const double& Flux);
  /// Return the intensity
  double GetFlux() const { return m_Flux; } 
  /// Return the intensity
  double GetInputFlux() const { return m_InputFlux; } 

  /// Return true, if the total energy flux (energy/cm^2) could be set correctly
  bool SetTotalEnergyFlux(const double& TotalEnergyFlux);
  /// Return the total energy flux
  double GetTotalEnergyFlux() const { return m_TotalEnergyFlux; } 

  /// Return true, if the light curve type could be set correctly
  bool SetLightCurveType(const int& LightCurveType);
  /// Return the light curve type
  int GetLightCurveType() const { return m_LightCurveType; }

  /// Return true, if the light curve could be set correctly
  bool SetLightCurve(const MString& FileName, const bool& Repeats);

  
  /// Set the polarization type
  bool SetPolarizationType(const int& Type);
  /// Get the polarization type
  int GetPolarizationType() const { return m_PolarizationType; }
  
  /// Return true, if the position vector could be set correctly
  bool SetPolarization(double PolarizationParam1 = c_Invalid, 
                       double PolarizationParam2 = c_Invalid, 
                       double PolarizationParam3 = c_Invalid);
  /// Return the polarization vector
  G4ThreeVector GetPolarizationVector() const { return m_Polarization; }

  /// Set the degree of polarization 1.0 == 100% polarized
  void SetPolarizationDegree(const double& Degree);
  /// Return the degree of polarization 1.0 == 100% polarized
  double GetPolarizationDegree() const { return m_PolarizationDegree; }

  
  /// Set the orientation
  bool SetOrientation(const MCOrientation& Orientation) { m_Orientation = Orientation; return true; }
  /// Return the orientation
  MCOrientation GetOrientation() const { return m_Orientation; }
  
  
  /// Return true, if the far field transmission probability file could be set and read correctly
  bool SetFarFieldTransmissionProbability(const MString& FileName);

  
  /// Return true, if the successor flag couls be set correctly
  bool SetIsSuccessor(const bool& IsSuccessorFlag) { m_IsSuccessor = IsSuccessorFlag; return true; }
  /// Return the successor flag
  bool IsSuccessor() const { return m_IsSuccessor; }

  /// Set the name of a source which is the generator of the next particle
  bool SetSuccessor(const MString& Successor) { m_Successor = Successor; return true; }
  /// Return the name of a source which is the generator of the next particle
  MString GetSuccessor() const { return m_Successor; }


  /// Return true, if the position vector could be set correctly
  bool SetPosition(double PositionParam1 = c_Invalid, 
                   double PositionParam2 = c_Invalid, 
                   double PositionParam3 = c_Invalid, 
                   double PositionParam4 = c_Invalid, 
                   double PositionParam5 = c_Invalid, 
                   double PositionParam6 = c_Invalid, 
                   double PositionParam7 = c_Invalid, 
                   double PositionParam8 = c_Invalid, 
                   double PositionParam9 = c_Invalid, 
                   double PositionParam10 = c_Invalid, 
                   double PositionParam11 = c_Invalid,
                   double PositionParam12 = c_Invalid,
                   double PositionParam13 = c_Invalid,
                   double PositionParam14 = c_Invalid);
  /// Return true, if the file containing the beam could be set correctly
  bool SetPosition(MString FileName);
  /// Return the specific position parameter 
  double GetPositionParameter(unsigned int i);

  /// Return true if the combined normalized energy-beam-flux-function could be set
  bool SetNormalizedEnergyBeamFluxFunction(MString FileName);

  /// Return true if the volume could be set correctly
  bool SetVolume(MString Volume) { m_Volume = Volume; return true; }
  /// Return the volume
  MString GetVolume() { return m_Volume; }

  /// Return true, if the energy vector could be set correctly
  bool SetEnergy(double EnergyParam1 = c_Invalid, 
                 double EnergyParam2 = c_Invalid, 
                 double EnergyParam3 = c_Invalid, 
                 double EnergyParam4 = c_Invalid, 
                 double EnergyParam5 = c_Invalid, 
                 double EnergyParam6 = c_Invalid,
                 double EnergyParam7 = c_Invalid);
  /// Return true, if the file containing the spectrum could be set correctly
  bool SetEnergy(MString FileName); 
  /// Return the specific energy parameter 
  double GetEnergyParameter(unsigned int i);


  /// Set the isotope count - promotes this source to an isotope count
  bool SetIsotopeCount(double IsotopeCount);


  /// Set the typ eof area, on which particles in the far field start 
  bool SetStartAreaType(const int& StartAreaType);
  /// Get the type of area on which particles in the far field start 
  int GetStartAreaType() const { return m_StartAreaType; }
  /// Get the start sphere radius, on which particles in the far field start 
  bool SetStartAreaParameters(double StartAreaParam1 = c_Invalid, 
                              double StartAreaParam2 = c_Invalid,
                              double StartAreaParam3 = c_Invalid,
                              double StartAreaParam4 = c_Invalid,
                              double StartAreaParam5 = c_Invalid,
                              double StartAreaParam6 = c_Invalid,
                              double StartAreaParam7 = c_Invalid,
                              double StartAreaParam8 = c_Invalid);
  /// Get the average start area for sources in the far field
  double GetStartAreaAverageArea() const { return m_StartAreaAverageArea; }

  /// Return the mean energy produced by the current spectra (mean, not average)
  double GetMeanEnergy() const;

  /// Generate a particle definition (this has to be done AFTER the physics list is initialized
  bool GenerateParticleDefinition();

  /// Generate a particle in the particle gun
  bool GenerateParticle(G4GeneralParticleSource* Gun);
  /// Generate an inititial energy in the particle gun
  bool GenerateEnergy(G4GeneralParticleSource* Gun);
  /// Generate an inititial position in the particle gun
  bool GeneratePosition(G4GeneralParticleSource* Gun);
  /// Generate an inititial polarization in the particle gun
  bool GeneratePolarization(G4GeneralParticleSource* Gun);

  /// Return the beam as string
  string GetBeamAsString() const;
  /// Return the type of the beam (e.g. FarFieldAreaSource)
  string GetBeamTypeAsString() const;
  /// Return the spectrum as string
  string GetSpectralAsString() const;
  /// Return the name of the spectrum (e.g. mono, etc.)
  string GetSpectralTypeAsString() const;
  /// Return the name of the polarization type
  string GetPolarizationTypeAsString() const;

  /// Id of an invalid type
  static const int c_Invalid; 

  /// Id of a spherical coordinate system
  static const int c_FarField;
  /// Id of a cartesian coordinate system
  static const int c_NearField;

  /// Id of a map
  static const int c_Map; 

  /// Id of a monoenergetic energy distribution
  static const int c_Monoenergetic; 
  /// Id of a linear energy distribution
  static const int c_Linear; 
  /// Id of a power law energy distribution
  static const int c_PowerLaw; 
  /// Id of a broken power law energy distribution
  static const int c_BrokenPowerLaw; 
  /// Id of a cutoff power law energy distribution
  static const int c_CutOffPowerLaw;
  /// Id of a Comptonized distribution: E^alpha * exp(-(alpha+2)*E/E_peak)
  static const int c_Comptonized;
  /// Id of a Gaussian energy distribution
  static const int c_Gaussian; 
  /// Id of a thermal bremsstrahlung distribution: 1/E*exp(E/E_T)
  static const int c_ThermalBremsstrahlung; 
  /// Id of a black body distribution: E^2/(exp(E/E_T)-1)
  static const int c_BlackBody; 
  /// Id of a band function distribution
  static const int c_BandFunction; 
  /// Id of a Spectrum from file in format of a differential flux p/cm2/sec/keV
  static const int c_FileDifferentialFlux; 
  /// Id of an activation, i.e. decays
  static const int c_Activation; 
  /// Id of a beam distribution from a combined energy-position-flux-function
  static const int c_NormalizedEnergyBeamFluxFunction;


  /// Id of an unknown start area
  static const int c_StartAreaUnknown;
  /// Id of a spherical start area
  static const int c_StartAreaSphere;
  /// Id of a tubical start area 
  static const int c_StartAreaTube;


  /// Id of a flat light curve, i.e. light curve not used 
  static const int c_LightCurveFlat;
  /// Id of a file based light curve
  static const int c_LightCurveFile;


  /// Id of a point like source in spherical coordinates
  static const int c_FarFieldPoint;
  /// Id of a area like source in spherical coordinates
  static const int c_FarFieldArea;
  /// Id of a symmetric Gaussian-like source in spherical coordinates
  static const int c_FarFieldGaussian;
  /// Id of a assymetric Gaussian-like source in spherical coordinates
  static const int c_FarFieldAssymetricGaussian;
  /// Id of a beam distribution from file in form of a zenith dependent distribution
  static const int c_FarFieldFileZenithDependent;
  /// Id of a beam distribution from a combined energy-position-flux-function
  static const int c_FarFieldNormalizedEnergyBeamFluxFunction;
  /// Id of an isotropic far-field beam 
  static const int c_FarFieldIsotropic;
  /// Id of an far-field disk 
  static const int c_FarFieldDisk;
  
  /// Id of a point like source in Cartesian coordinates
  static const int c_NearFieldPoint;
  /// Id of a point like source in Cartesian coordinates restricted towards the surrounding sphere
  static const int c_NearFieldRestrictedPoint;
  /// Id of a beam in Cartesian coordinates - diffraction of a point
  static const int c_NearFieldDiffractionPoint;
  /// Id of a beam in Cartesian coordinates - diffraction of a point in k-space 
  static const int c_NearFieldDiffractionPointKSpace;
  /// Id of a line like source in Cartesian coordinates
  static const int c_NearFieldLine;
  /// Id of a line like source in Cartesian coordinates restricted towards the surrounding sphere
  static const int c_NearFieldRestrictedLine;
  /// Id of a box like source in Cartesian coordinates
  static const int c_NearFieldBox;
  /// Id of a spherical like source in Cartesian coordinates
  static const int c_NearFieldSphere;
  /// Id of a disk like source in Cartesian coordinates
  static const int c_NearFieldDisk;
  /// Id of a beam in Cartesian coordinates
  static const int c_NearFieldBeam;
  /// Id of a beam in Cartesian coordinates with a 1D (radial) profile
  static const int c_NearFieldBeam1DProfile;
  /// Id of a beam in Cartesian coordinates with a 2D (map) profile
  static const int c_NearFieldBeam2DProfile;
  /// Id of a divergent beam (with homogeneous intensity)
  /// from a point in Cartesian coordinates
  static const int c_NearFieldConeBeam;
  /// Id of a divergent beam (with Gaussian intensity profile) 
  /// from a point in Cartesian coordinates
  static const int c_NearFieldConeBeamGauss;
  /// Id of a fan beam with a beam width
  static const int c_NearFieldFanBeam;
  /// Id of a illuminated disk 
  static const int c_NearFieldIlluminatedDisk;
  /// Id of a illuminated box
  static const int c_NearFieldIlluminatedSquare;
  /// Id of a activation, i.e. decays evenly distributed in volumes
  static const int c_NearFieldActivation;
  /// Id of a radiating volume, i.e. the start positions are randomly distributed within the volume
  static const int c_NearFieldVolume;
  /// Id of a 2D structure emitting particles in isotropically in 3D
  static const int c_NearFieldFlatMap;

  /// The position is identical to the predecessor & the the momentum direction is opposite (e.g. for simulating annihilation) 
  static const int c_NearFieldReverseDirectionToPredecessor;
  
  // --> Polarization modes

  /// Id of the polarization being in absolute coordinates
  static const int c_PolarizationNone;
  /// Id of the polarization being in absolute coordinates
  static const int c_PolarizationRandom;
  /// Id of the polarization being in absolute coordinates
  static const int c_PolarizationAbsolute;
  /// Id of the polarization being calculated relative to particle direction and x-axis
  static const int c_PolarizationRelativeX;
  /// Id of the polarization being calculated relative to particle direction and y-axis
  static const int c_PolarizationRelativeY;
  /// Id of the polarization being calculated relative to particle direction and z-axis
  static const int c_PolarizationRelativeZ;


  // --> Particles

  /// Id of a gamma
  static const int c_Gamma;

  /// Id of a positron
  static const int c_Positron;
  /// Id of an electron
  static const int c_Electron;

  /// Id of a proton
  static const int c_Proton;
  /// Id of an anti proton
  static const int c_AntiProton;
  /// Id of a neutron
  static const int c_Neutron;
  /// Id of an anti neutron
  static const int c_AntiNeutron;

  /// Id of a muon plus
  static const int c_MuonPlus;
  /// Id of a muon minus
  static const int c_MuonMinus;
  /// Id of a tauon plus
  static const int c_TauonPlus;
  /// Id of a tauon minus  
  static const int c_TauonMinus;
  /// Id of an electron neutrino

  static const int c_ElectronNeutrino;
  /// Id of an anti electron neutrino
  static const int c_AntiElectronNeutrino;
  /// Id of an muon neutrino
  static const int c_MuonNeutrino;
  /// Id of an anti muon neutrino
  static const int c_AntiMuonNeutrino;
  /// Id of a tauon neutrino  
  static const int c_TauonNeutrino;
  /// Id of a tauon anti neutrino
  static const int c_AntiTauonNeutrino;

  // --> Ions

  /// Id of a deuteron  
  static const int c_Deuteron;
  /// Id of a triton  
  static const int c_Triton;
  /// Id of an He3 particle 
  static const int c_He3;
  /// Id of an alpha particle  
  static const int c_Alpha;
  /// Id of an genric ion particle  
  static const int c_GenericIon;

  // --> Mesons

  /// Id of a Pi+
  static const int c_PiPlus;
  /// Id of a Pi0  
  static const int c_PiZero;
  /// Id of a Pi-  
  static const int c_PiMinus;
  /// Id of an eta particle 
  static const int c_Eta;
  /// Id of an eta prime particle 
  static const int c_EtaPrime;

  /// Id of an Kaon plus particle 
  static const int c_KaonPlus;
  /// Id of an Kaon null particle 
  static const int c_KaonZero;
  /// Id of an Anti Kaon null particle 
  static const int c_AntiKaonZero;
  /// Id of an Kaon null S particle 
  static const int c_KaonZeroS;
  /// Id of an Kaon null L particle 
  static const int c_KaonZeroL;
  /// Id of an Kaon minus particle 
  static const int c_KaonMinus;

  /// Id of a Rho+
  static const int c_RhoPlus;
  /// Id of a Rho0  
  static const int c_RhoZero;
  /// Id of a Rho-  
  static const int c_RhoMinus;

  // --> Baryons

  /// Id of an lambda particle 
  static const int c_Lambda;
  /// Id of an lambda particle 
  static const int c_AntiLambda;

  /// Id of an Sigma Minus particle 
  static const int c_SigmaMinus;
  /// Id of an Anti Sigma Minus particle 
  static const int c_AntiSigmaMinus;
  /// Id of an Sigma Zero particle 
  static const int c_SigmaZero;
  /// Id of an Anti Sigma Zero particle 
  static const int c_AntiSigmaZero;
  /// Id of an Sigma Plus particle 
  static const int c_SigmaPlus;
  /// Id of an Anti Sigma Plus particle 
  static const int c_AntiSigmaPlus;

  /// Id of an Xi Minus particle 
  static const int c_XiMinus;
  /// Id of an anti Xi Minus particle 
  static const int c_AntiXiMinus;
  /// Id of an Xi Zero particle 
  static const int c_XiZero;
  /// Id of an anti Xi Zero particle 
  static const int c_AntiXiZero;

  /// Id of an Omega Minus particle 
  static const int c_OmegaMinus;
  /// Id of an anti Omega Minus particle 
  static const int c_AntiOmegaMinus;

  /// Id of an Delta Minus particle 
  static const int c_DeltaMinus;
  /// Id of an Delta Zero particle 
  static const int c_DeltaZero;
  /// Id of an Delta Plus particle 
  static const int c_DeltaPlus;
  /// Id of an Delta PlusPlus particle 
  static const int c_DeltaPlusPlus;

  /// Id of an Omega particle 
  static const int c_Omega;


  // protected methods:
protected:
  /// Upgrade the position parameters and do some sanity checks
  bool UpgradePosition();
  /// Upgrade the start area parameters and do some sanity checks
  bool UpgradeStartArea();
  /// Upgrade the energy parameters and do some sanity checks
  bool UpgradeEnergy();
  /// Upgrade the energy parameters and do some sanity checks
  bool UpgradeFlux();
  /// Upgrade the energy parameters and do some sanity checks
  bool UpgradeLightCurve();
  /// Shape of black body emission (Temperature in keV)
  double BlackBody(double Energy, double Temperature) const; 
  /// Shape of a Band function
  double BandFunction(const double Energy, double Alpha, const double Beta, const double E0) const; 
  /// Shape of a Comptonized spectrum
  double Comptonized(const double Energy, double Alpha, double Epeak) const;

  /// Perform an orientation of the vector from local into oriented coordinate system
  bool PerformOrientation(G4ThreeVector& Direction);
  /// Perform an orientation of the vector from local into oriented coordinate system
  bool PerformOrientation(G4ThreeVector& Position, G4ThreeVector& Direction);


  // protected members:
protected:


  // private members:
private:
  /// Name of this source
  MString m_Name;

  /// Unique ID of this source (to speed up comparisons)
  unsigned int m_ID;

  /// A global static ID counter
  static unsigned int s_ID;

  /// Time of the next emission
  MTime m_NextEmission;

  /// Radius of the disk on which particles in spherical coordinates start 
  int m_StartAreaType;
  /// Position of the zero point on which the distance vector for the start area begins 
  G4ThreeVector m_StartAreaPosition;
  /// The average area from which the photons starts
  double m_StartAreaAverageArea;
  /// Parameter 1 of the start area 
  double m_StartAreaParam1;
  /// Parameter 2 of the start area
  double m_StartAreaParam2;

  /// True if the data originates from an event list
  bool m_IsEventList;
  /// True if the data originates from an event list of build up simulations
  bool m_IsBuildUpEventList;
  
  /// Local class representing one event list entry
  class MEventListEntry {
  public:
    long m_ID;
    double m_Energy;
    G4ThreeVector m_Position;
    G4ThreeVector m_Direction;
    G4ThreeVector m_Polarization;
    int m_ParticleType;
    double m_ParticleExcitation;
    G4ParticleDefinition* m_ParticleDefinition;
    bool m_IsSuccessor;
    MString m_VolumeName;
    MTime m_Time;
    
    bool operator <(const MEventListEntry A) const { return this->m_Time < A.m_Time; }
  };
  
  /// The event list (i.e. deque)
  deque<MEventListEntry*> m_EventList;
  /// Size of the event list (list::size() is slow...)
  unsigned int m_EventListSize; 

  /// Number of next events which should not be started
  int m_NEventsToSkip;

  /// Total number of events which have not be started in history of this source (required for speed optimizations)
  int m_NEventsSkipped;

  /// Event list file
  ifstream m_EventListFile;
  /// True if this is an event list read from file
  bool m_IsFileEventList;
  
  
  /// True if this source is an isotope count source
  bool m_IsIsotopeCount;
  /// The current isotope count
  double m_IsotopeCount;


  /// Id of the coordiante system
  int m_CoordinateSystem;
  /// Id of the spectral type
  int m_SpectralType;
  /// Id of the region type
  int m_BeamType;
  /// Id of the particle type
  int m_ParticleType;
  /// Id of the particle type
  int m_LightCurveType;

  /// The last calculated energy
  double m_Energy;
  /// The last calculated position
  G4ThreeVector m_Position;
  /// The last calculated direction
  G4ThreeVector m_Direction;
  /// The last calculated polarization
  G4ThreeVector m_Polarization;
  
  
  /// Excitation of the particle
  double m_ParticleExcitation;

  /// Half life of the particle
  double m_HalfLife;

  /// The Geant4 particle definition 
  G4ParticleDefinition* m_ParticleDefinition;

  /// True if this source is flux variable, e.g. has a light curve
  bool m_IsFluxVariable;

  /// True if this source has still events to emit
  bool m_IsActive;

  /// True if this secondary emitter in a cascade
  bool m_IsSuccessor;
  /// Name of the secondray emitter in a cascade
  MString m_Successor;

  /// Input flux --- not yet correctly normalized
  double m_InputFlux;
  /// Flux of this source
  double m_Flux;

  /// TotalEnergyFlux of this source
  double m_TotalEnergyFlux;

  /// The light curve
  MFunction m_LightCurveFunction;
  /// True if the light curve is repeating (i.e. pulsar vs. GRB)
  bool m_IsRepeatingLightCurve;
  /// The current light curve cycle
  unsigned int m_LightCurveCycle;
  /// The current light curve integration value
  double m_LightCurveIntegration;

  
  /// The polarization type
  int m_PolarizationType;
  
  /// Parameter 1 of the polarization 
  double m_PolarizationParam1;
  /// Parameter 2 of the polarization 
  double m_PolarizationParam2;
  /// Parameter 3 of the polarization 
  double m_PolarizationParam3;

  /// Degree of polarization
  double m_PolarizationDegree;

  
  /// The orientation
  MCOrientation m_Orientation;
  
  
  /// Flag indicating we should apply the far field transmission probabilities
  bool m_UseFarFieldTransmissionProbability;
  /// Far field transmission probabilities
  MFunction2D m_FarFieldTransmissionProbability;
  
  
  /// Number of generated particles
  long m_NGeneratedParticles;

  /// Parameter 1 of the position 
  double m_PositionParam1;
  /// Parameter 2 of the position 
  double m_PositionParam2;
  /// Parameter 3 of the position 
  double m_PositionParam3;
  /// Parameter 4 of the position 
  double m_PositionParam4;
  /// Parameter 5 of the position 
  double m_PositionParam5;
  /// Parameter 6 of the position 
  double m_PositionParam6;
  /// Parameter 7 of the position 
  double m_PositionParam7;
  /// Parameter 8 of the position 
  double m_PositionParam8;
  /// Parameter 9 of the position 
  double m_PositionParam9;
  /// Parameter 10 of the position 
  double m_PositionParam10;
  /// Parameter 11 of the position 
  double m_PositionParam11;
  /// Parameter 11 of the position 
  double m_PositionParam12;
  /// Parameter 11 of the position 
  double m_PositionParam13;
  /// Parameter 11 of the position 
  double m_PositionParam14;

  /// A volume as start position
  MString m_Volume;

  /// A file containing the beam distribution
  MFunction m_PositionFunction;
  /// A file containing the beam distribution as 2D function
  MFunction2D m_PositionFunction2D;
  /// A ROOT TF1 function containing some auxiliary aspects of the position distribution
  /// e.g. for CartesianConeBeamGaus: the probability density function for the beam internal zenith angle
  TF1* m_PositionTF1;
  /// A file containing the spectrum, beam, and flux
  MFunction3DSpherical m_NormalizedEnergyBeamFluxFunction;

  /// Parameter 1 of the energy 
  double m_EnergyParam1;
  /// Parameter 2 of the energy 
  double m_EnergyParam2;
  /// Parameter 3 of the energy 
  double m_EnergyParam3;
  /// Parameter 4 of the energy 
  double m_EnergyParam4;
  /// Parameter 5 of the energy 
  double m_EnergyParam5;
  /// Parameter 6 of the energy 
  double m_EnergyParam6;
  /// Parameter 7 of the energy 
  double m_EnergyParam7;

  /// A file containing the spectrum
  MFunction m_EnergyFunction;
};

#endif


/*
 * MCSource.hh: the end...
 ******************************************************************************/
