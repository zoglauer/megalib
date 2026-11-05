/*
 * UTSourceShared.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef ___UTSourceShared___
#define ___UTSourceShared___


// Cosima:
#include "MCDetectorConstruction.hh"
#include "MCParameterFile.hh"
#include "MCRunManager.hh"
#include "MCSource.hh"
#include "MCSteppingAction.hh"

// MEGAlib:
#include "MStreams.h"
#include "MString.h"
#include "MUnitTest.h"

// Geant4:
#include "G4BaryonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4GeneralParticleSource.hh"
#include "G4GenericIon.hh"
#include "G4IonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4NuclideTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"

// ROOT:
#include "TError.h"
#include "TRandom.h"
#include "TRandom3.h"

// Standard lib:
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Basic statistics of a sample
struct UTSourceStatistics
{
  //! Number of entries
  unsigned int m_N = 0;
  //! Mean value
  double m_Mean = 0.0;
  //! Standard deviation of the sample
  double m_Sigma = 0.0;
  //! Smallest value
  double m_Min = 0.0;
  //! Largest value
  double m_Max = 0.0;
};


//! Base class for the unit tests of MCSource: it owns the minimal cosima environment (run manager,
//! parameter file, geometry, particles, and a real Geant4 general particle source), silences output,
//! and provides sampling and integration helpers
class UTSourceTestBase : public MUnitTest
{
public:
  //! Default constructor
  UTSourceTestBase(const MString& Name) : MUnitTest(Name), m_CoutBuffer(nullptr), m_CerrBuffer(nullptr), m_ROOTErrorLevel(kUnset),
    m_Parameters(nullptr), m_RunManager(nullptr), m_Detector(nullptr), m_Gun(nullptr),
    m_EnvironmentInitialized(false), m_EnvironmentOK(false) {}
  //! Destructor: delete what the environment created. The detector construction is owned by the run manager (Geant4 deletes its user
  //! initialization), the gun and the parameter file are owned by this class. The run manager must go before the parameter file it refers to
  virtual ~UTSourceTestBase()
  {
    delete m_Gun;
    m_Gun = nullptr;
    delete m_RunManager;
    m_RunManager = nullptr;
    m_Detector = nullptr;
    delete m_Parameters;
    m_Parameters = nullptr;
  }

protected:
  //! Redirect cout, cerr and the MEGAlib streams - the source prints a lot
  void SilenceOutput()
  {
    DisableDefaultStreams();
    mimp.Enable(false);
    m_ROOTErrorLevel = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kError + 1;
    m_Sink.str("");
    m_CoutBuffer = cout.rdbuf(m_Sink.rdbuf());
    m_CerrBuffer = cerr.rdbuf(m_Sink.rdbuf());
  }

  //! Restore cout, cerr and the MEGAlib streams
  void RestoreOutput()
  {
    if (m_CoutBuffer != nullptr) cout.rdbuf(m_CoutBuffer);
    if (m_CerrBuffer != nullptr) cerr.rdbuf(m_CerrBuffer);
    m_CoutBuffer = nullptr;
    m_CerrBuffer = nullptr;
    mimp.Enable(true);
    gErrorIgnoreLevel = m_ROOTErrorLevel;
    EnableDefaultStreams();
  }

  //! Return everything which has been printed since the last call to SilenceOutput
  MString GetSilencedText() const { return m_Sink.str().c_str(); }

  //! Create the environment: the parameter file with one run, the run manager, the particles, a tiny geometry, and the particle gun
  //! The run manager must exist before any particle is created, as in cosima itself
  bool InitializeEnvironment(const MString& ExtraRunLines = "")
  {
    if (m_EnvironmentInitialized == true) return m_EnvironmentOK;
    m_EnvironmentInitialized = true;

    if (PrepareTemporaryDirectory("Environment") == false) return false;
    const MString Directory = GetTemporaryDirectoryName("Environment");
    const MString GeometryFile = Directory + "/Tiny.geo.setup";
    const MString SourceFile = Directory + "/Tiny.source";

    MString Geometry;
    Geometry += "Name TinyWorld\n";
    Geometry += "Version 1.0\n";
    Geometry += "SurroundingSphere 60.0  0.0  0.0  0.0  60.0\n";
    Geometry += "Material Vacuum\n";
    Geometry += "Vacuum.Density 1.0E-12\n";
    Geometry += "Vacuum.ComponentByAtoms H 1\n";
    Geometry += "Volume WorldVolume\n";
    Geometry += "WorldVolume.Material Vacuum\n";
    Geometry += "WorldVolume.Visibility 0\n";
    Geometry += "WorldVolume.Shape BRIK 100. 100. 100.\n";
    Geometry += "WorldVolume.Mother 0\n";
    Geometry += "Volume Cube\n";
    Geometry += "Cube.Material Vacuum\n";
    Geometry += "Cube.Visibility 1\n";
    Geometry += "Cube.Shape BRIK 10. 10. 10.\n";
    Geometry += "Cube.Position 0.0 0.0 0.0\n";
    Geometry += "Cube.Mother WorldVolume\n";
    if (WriteTextFile(GeometryFile, Geometry) == false) return false;

    MString Source;
    Source += "Version 1\n";
    Source += MString("Geometry ") + GeometryFile + "\n";
    Source += "PhysicsListEM Standard\n";
    Source += "StoreSimulationInfo init-only\n";
    Source += "Run Dummy\n";
    Source += "Dummy.FileName Dummy\n";
    Source += "Dummy.Triggers 10\n";
    Source += "Dummy.Source DummySource\n";
    Source += ExtraRunLines;
    Source += "DummySource.ParticleType 1\n";
    Source += "DummySource.Beam PointSource 0 0 0\n";
    Source += "DummySource.Spectrum Mono 100\n";
    Source += "DummySource.Flux 1\n";
    if (WriteTextFile(SourceFile, Source) == false) return false;

    SilenceOutput();
    m_Parameters = new MCParameterFile();
    bool OK = m_Parameters->Open(SourceFile);
    m_RunManager = new MCRunManager(*m_Parameters);

    G4BosonConstructor().ConstructParticle();
    G4LeptonConstructor().ConstructParticle();
    G4BaryonConstructor().ConstructParticle();
    G4MesonConstructor().ConstructParticle();
    G4IonConstructor().ConstructParticle();
    // The generic ion needs a process manager before ions can be created:
    G4ParticleDefinition* GenericIon = G4GenericIon::Definition();
    GenericIon->SetProcessManager(new G4ProcessManager(GenericIon));
    G4ParticleTable::GetParticleTable()->SetReadiness();
    G4NuclideTable::GetInstance()->SetLevelTolerance(100*eV);
    G4NuclideTable::GetInstance()->SetThresholdOfHalfLife(0.0001*ns);

    m_Detector = new MCDetectorConstruction(*m_Parameters);
    OK = m_Detector->Initialize() && OK;
    m_RunManager->SetUserInitialization(m_Detector);
    m_Gun = new G4GeneralParticleSource();
    RestoreOutput();

    m_EnvironmentOK = OK;
    return m_EnvironmentOK;
  }

  //! Return the particle gun of the environment
  G4GeneralParticleSource* GetGun() { return m_Gun; }

  //! Return the energy which was last generated into the gun
  double GetGunEnergy() const { return m_Gun->GetCurrentSource()->GetEneDist()->GetMonoEnergy(); }
  //! Return the position which was last generated into the gun
  G4ThreeVector GetGunPosition() const { return m_Gun->GetCurrentSource()->GetPosDist()->GetCentreCoords(); }
  //! Return the direction which was last generated into the gun
  G4ThreeVector GetGunDirection() const { return m_Gun->GetCurrentSource()->GetAngDist()->GetDirection(); }
  //! Return the polarization which was last generated into the gun
  G4ThreeVector GetGunPolarization() const { return m_Gun->GetParticlePolarization(); }

  //! Set a source which is complete enough for the generation: particle, spectrum, beam, and flux - near field point source at the given position
  //! The particle definition is generated. Everything is silent. Return true if everything worked
  bool PrepareNearFieldPointSource(MCSource& Source, int Particle, int Spectrum, const vector<double>& Energy, double X = 0.0, double Y = 0.0, double Z = 0.0)
  {
    SilenceOutput();
    bool OK = Source.SetParticleType(Particle);
    OK = Source.SetSpectralType(Spectrum) && OK;
    OK = Source.SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldPoint) && OK;
    OK = Source.SetEnergy(Energy.size() > 0 ? Energy[0] : MCSource::c_Invalid,
                          Energy.size() > 1 ? Energy[1] : MCSource::c_Invalid,
                          Energy.size() > 2 ? Energy[2] : MCSource::c_Invalid,
                          Energy.size() > 3 ? Energy[3] : MCSource::c_Invalid,
                          Energy.size() > 4 ? Energy[4] : MCSource::c_Invalid,
                          Energy.size() > 5 ? Energy[5] : MCSource::c_Invalid) && OK;
    OK = Source.SetPosition(X, Y, Z) && OK;
    OK = Source.SetFlux(10.0) && OK;
    OK = Source.GenerateParticleDefinition() && OK;
    RestoreOutput();
    return OK;
  }

  //! Generate N energies with the given source silently and return them (keV), stop at the first failure
  vector<double> SampleEnergies(MCSource& Source, unsigned int N, bool* AllOK = nullptr)
  {
    vector<double> Energies;
    Energies.reserve(N);
    bool OK = true;
    SilenceOutput();
    for (unsigned int i = 0; i < N; ++i) {
      if (Source.GenerateEnergy(m_Gun) == false) {
        OK = false;
        break;
      }
      Energies.push_back(GetGunEnergy()/keV);
    }
    RestoreOutput();
    if (AllOK != nullptr) *AllOK = OK;
    return Energies;
  }

  //! Compute the statistics of a sample
  static UTSourceStatistics Statistics(const vector<double>& Values)
  {
    UTSourceStatistics S;
    S.m_N = Values.size();
    if (Values.size() == 0) return S;
    S.m_Min = Values[0];
    S.m_Max = Values[0];
    double Sum = 0.0;
    for (double V: Values) {
      Sum += V;
      if (V < S.m_Min) S.m_Min = V;
      if (V > S.m_Max) S.m_Max = V;
    }
    S.m_Mean = Sum/Values.size();
    double Squares = 0.0;
    for (double V: Values) Squares += (V - S.m_Mean)*(V - S.m_Mean);
    S.m_Sigma = (Values.size() > 1) ? sqrt(Squares/(Values.size() - 1)) : 0.0;
    return S;
  }

  //! Simpson integration of a function - independent reference for distributions
  static double Integrate(const function<double(double)>& F, double A, double B, unsigned int N = 4000)
  {
    if (N % 2 == 1) ++N;
    const double h = (B - A)/N;
    double Sum = F(A) + F(B);
    for (unsigned int i = 1; i < N; ++i) {
      Sum += F(A + i*h)*((i % 2 == 1) ? 4.0 : 2.0);
    }
    return Sum*h/3.0;
  }

  //! Mean of a density on [A, B] computed by numerical integration - independent reference for the mean energy of a spectrum
  static double MeanOfDensity(const function<double(double)>& Density, double A, double B)
  {
    return Integrate([&Density](double E) { return E*Density(E); }, A, B)/Integrate(Density, A, B);
  }

  //! Return the Kolmogorov-Smirnov distance between the samples and the density on [A, B]
  static double KSDistance(vector<double> Samples, const function<double(double)>& Density, double A, double B)
  {
    sort(Samples.begin(), Samples.end());
    const unsigned int Grid = 40000;
    vector<double> Cumulative(Grid + 1, 0.0);
    const double h = (B - A)/Grid;
    for (unsigned int i = 1; i <= Grid; ++i) {
      Cumulative[i] = Cumulative[i-1] + 0.5*h*(Density(A + (i-1)*h) + Density(A + i*h));
    }
    auto CDF = [&](double X) {
      if (X <= A) return 0.0;
      if (X >= B) return 1.0;
      const double Position = (X - A)/h;
      const unsigned int i = min((unsigned int) Position, Grid - 1);
      return (Cumulative[i] + (Position - i)*(Cumulative[i+1] - Cumulative[i]))/Cumulative[Grid];
    };
    double D = 0.0;
    for (unsigned int i = 0; i < Samples.size(); ++i) {
      const double F = CDF(Samples[i]);
      D = max(D, max(fabs(F - double(i)/Samples.size()), fabs(F - double(i + 1)/Samples.size())));
    }
    return D;
  }

  //! Critical Kolmogorov-Smirnov distance at about the 0.1% level
  static double KSCritical(unsigned int N) { return 1.95/sqrt(double(N)); }

  //! Fixes the random seeds for ROOT and CLHEP and restores the previous ones on destruction
  class RandomGuard
  {
  public:
    //! Save the complete states of both random engines and set the new seed
    RandomGuard(unsigned int Seed) : m_ROOTState(nullptr), m_CLHEPState(CLHEP::HepRandom::getTheEngine()->put())
    {
      // The ROOT generator of the sources is a TRandom3: clone it to keep its full state (the seed alone is not the state)
      if (dynamic_cast<TRandom3*>(gRandom) != nullptr) m_ROOTState = static_cast<TRandom3*>(gRandom->Clone());
      gRandom->SetSeed(Seed);
      CLHEP::HepRandom::setTheSeed(Seed);
    }
    //! Restore the complete states
    ~RandomGuard()
    {
      if (m_ROOTState != nullptr) {
        *static_cast<TRandom3*>(gRandom) = *m_ROOTState;
        delete m_ROOTState;
      }
      CLHEP::HepRandom::getTheEngine()->get(m_CLHEPState);
    }
    RandomGuard(const RandomGuard&) = delete;
    RandomGuard& operator=(const RandomGuard&) = delete;
  private:
    //! The ROOT generator state before the guard (a copy of the generator)
    TRandom3* m_ROOTState;
    //! The CLHEP engine state before the guard
    vector<unsigned long> m_CLHEPState;
  };

  //! The original cout buffer while silenced
  streambuf* m_CoutBuffer;
  //! The original cerr buffer while silenced
  streambuf* m_CerrBuffer;
  //! The ROOT message level which is restored by RestoreOutput()
  int m_ROOTErrorLevel;
  //! The sink for the silenced output
  ostringstream m_Sink;

  //! The parameter file of the environment
  MCParameterFile* m_Parameters;
  //! The run manager of the environment
  MCRunManager* m_RunManager;
  //! The detector construction of the environment
  MCDetectorConstruction* m_Detector;
  //! The particle gun of the environment
  G4GeneralParticleSource* m_Gun;
  //! Flag indicating if the environment has been created
  bool m_EnvironmentInitialized;
  //! Flag indicating if the environment has been created successfully
  bool m_EnvironmentOK;
};


#endif


/*
 * UTSourceShared.h: the end...
 ******************************************************************************/
