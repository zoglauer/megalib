/*
 * UTComptonEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// ROOT:
#include <TRandom.h>

// MEGAlib:
#include "MComptonEvent.h"
#include "MFile.h"
#include "MExceptions.h"
#include "MPhysicalEventHit.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MComptonEvent
class UTComptonEvent : public MUnitTest
{
public:
  //! Default constructor
  UTComptonEvent() : MUnitTest("UTComptonEvent") {}
  //! Default destructor
  virtual ~UTComptonEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MComptonEvent& Event, const MString& Tra, bool Fast = false);
  //! Test state, validation, and helper calculations
  bool TestBasics();
  //! Test assimilation from hits and text lines
  bool TestAssimilation();
  //! Test the serialization round-trip
  bool TestTraRoundTrip();
  //! Test copy-style assimilation and file-stream round-trip
  bool TestCopyAndStream();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTComptonEvent::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestAssimilation() && Passed;
  Passed = TestTraRoundTrip() && Passed;
  Passed = TestCopyAndStream() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a tra-string line-by-line into an event object
bool UTComptonEvent::ParseTraString(MComptonEvent& Event, const MString& Tra, bool Fast)
{
  istringstream In(Tra.ToString());
  string Line;
  while (getline(In, Line)) {
    if (Line.empty() == true) {
      continue;
    }
    int Ret = Event.ParseLine(Line.c_str(), Fast);
    if (Ret == 1) {
      return false;
    }
  }
  return Event.Validate();
}


////////////////////////////////////////////////////////////////////////////////


//! Test state, validation, and helper calculations
bool UTComptonEvent::TestBasics()
{
  bool Passed = true;

  MComptonEvent Event;
  Passed = Evaluate("GetType()", "default", "Compton events report the correct type", Event.GetType(), MPhysicalEvent::c_Compton) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Compton events report the correct type string", Event.GetTypeString(), MString("Compton")) && Passed;
  Passed = Evaluate("SequenceLength()", "default", "The default sequence length is two", Event.SequenceLength(), 2U) && Passed;
  Passed = Evaluate("TrackLength()", "default", "The default track length is one", Event.TrackLength(), 1U) && Passed;
  Passed = EvaluateFalse("HasTrack()", "default", "A default Compton event has no track", Event.HasTrack()) && Passed;
  Passed = Evaluate("GetOrigin()", "default", "A default Compton event has no origin direction", Event.GetOrigin(), g_VectorNotDefined) && Passed;

  Passed = EvaluateTrue("IsKinematicsOK()", "100 keV + 500 keV", "Representative Compton energies are kinematically valid", MComptonEvent::IsKinematicsOK(100.0, 500.0)) && Passed;
  Passed = EvaluateFalse("IsKinematicsOK()", "1 keV + 1 keV", "Implausible energies are rejected", MComptonEvent::IsKinematicsOK(1.0, 1.0)) && Passed;

  Passed = EvaluateNear("ComputeCosPhiViaEeEg()", "100/500", "The cosine helper returns the expected value", MComptonEvent::ComputeCosPhiViaEeEg(100.0, 500.0), 0.829667, 1e-12) && Passed;
  Passed = EvaluateNear("ComputePhiViaEeEg()", "100/500", "The phi helper returns the expected angle", MComptonEvent::ComputePhiViaEeEg(100.0, 500.0), 0.5922854048410028, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeEeViaPhiEg()", "phi=1.0/Eg=500", "The Ee helper returns the expected energy", MComptonEvent::ComputeEeViaPhiEg(1.0, 500.0), 408.7652890586013, 1e-12) && Passed;
  Passed = EvaluateNear("ComputePhiViaEeEg()", "phi=1.0/Eg=500", "The phi/Eg helper round-trips through the inverse relation", MComptonEvent::ComputePhiViaEeEg(MComptonEvent::ComputeEeViaPhiEg(1.0, 500.0), 500.0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeEeViaPhiEi()", "phi=1.0/Ei=600", "The Ee-via-Ei helper returns the expected energy", MComptonEvent::ComputeEeViaPhiEi(1.0, 600.0), 210.32977200995623, 1e-12) && Passed;
  Passed = EvaluateNear("ComputePhiViaEeEg()", "phi=1.0/Ei=600", "The phi/Ei helper round-trips through the inverse relation", MComptonEvent::ComputePhiViaEeEg(MComptonEvent::ComputeEeViaPhiEi(1.0, 600.0), 600.0 - MComptonEvent::ComputeEeViaPhiEi(1.0, 600.0)), 1.0, 1e-12) && Passed;
  __merr.Enable(false);
  Passed = EvaluateNear("ComputeEgViaThetaEe()", "theta=1.0/Ee=1000", "The Eg helper returns the expected energy", MComptonEvent::ComputeEgViaThetaEe(1.0, 1000.0), 2205.372267724561, 1e-9) && Passed;
  __merr.Enable(true);
  Passed = EvaluateNear("ComputeEeViaThetaEg()", "theta=1.2/Ee=200", "The Ee helper returns the expected energy", MComptonEvent::ComputeEeViaThetaEg(1.2, 4898.857707326749), 200.0, 1e-12) && Passed;
  Passed = EvaluateNear("ComputeEgViaThetaEe()", "theta=1.2/Ee=200", "The inverse helper returns the expected gamma energy", MComptonEvent::ComputeEgViaThetaEe(1.2, 200.0), 4898.857707326749, 1e-9) && Passed;
  Passed = Evaluate("ComputeEeViaThetaEg()", "no solution", "The helper returns the sentinel when no solution exists", MComptonEvent::ComputeEeViaThetaEg(1.2, 500.0), 9.99E+99) && Passed;
  gRandom->SetSeed(123456);
  const double RandomPhi = MComptonEvent::GetRandomPhi(600.0);
  Passed = EvaluateNear("GetRandomPhi()", "600 keV", "Random Compton scatter angles are reproducible with a fixed seed", RandomPhi, 0.454195, 1e-6) && Passed;
  Passed = EvaluateTrue("GetRandomPhi()", "600 keV", "Random Compton scatter angles are in range", RandomPhi >= 0.0 && RandomPhi <= c_Pi) && Passed;

  Passed = EvaluateNear("GetKleinNishina()", "600/1.0", "The Klein-Nishina value matches the reference calculation", MComptonEvent::GetKleinNishina(600.0, 1.0), 2.0606955358931753e-30, 1e-34) && Passed;
  Passed = EvaluateNear("GetDifferentalKleinNishina()", "600/1.0", "The differential Klein-Nishina value matches the reference calculation", MComptonEvent::GetDifferentalKleinNishina(600.0, 1.0), 2.448920489354272e-30, 1e-34) && Passed;
  Passed = EvaluateNear("GetKleinNishinaNormalized()", "600/1.0", "The normalized Klein-Nishina value matches the reference calculation", MComptonEvent::GetKleinNishinaNormalized(600.0, 1.0), 0.7875232996978867, 1e-4) && Passed;
  Passed = EvaluateNear("GetKleinNishinaNormalizedByArea()", "600/1.0", "The area-normalized Klein-Nishina value matches the reference calculation", MComptonEvent::GetKleinNishinaNormalizedByArea(600.0, 1.0), 0.49025440107901136, 1e-4) && Passed;

  Passed = EvaluateNear("GetKleinNishina()", "1000/0.5", "The Klein-Nishina value matches a second reference point", MComptonEvent::GetKleinNishina(1000.0, 0.5), 2.2217366992031657e-30, 1e-34) && Passed;
  Passed = EvaluateNear("GetDifferentalKleinNishina()", "1000/0.5", "The differential Klein-Nishina value matches a second reference point", MComptonEvent::GetDifferentalKleinNishina(1000.0, 0.5), 4.6341642659911654e-30, 1e-34) && Passed;
  Passed = EvaluateNear("GetKleinNishinaNormalized()", "1000/0.5", "The normalized Klein-Nishina value matches a second reference point", MComptonEvent::GetKleinNishinaNormalized(1000.0, 0.5), 0.9999008060035717, 1e-4) && Passed;
  Passed = EvaluateNear("GetKleinNishinaNormalizedByArea()", "1000/0.5", "The area-normalized Klein-Nishina value matches a second reference point", MComptonEvent::GetKleinNishinaNormalizedByArea(1000.0, 0.5), 0.6694365458574993, 1e-4) && Passed;

  MComptonEvent ARMEvent;
  ARMEvent.SetEg(256.0);
  ARMEvent.SetEe(MComptonEvent::ComputeEeViaPhiEg(c_Pi/2.0, 256.0));
  ARMEvent.SetC1(MVector(0.0, 0.0, 0.0));
  ARMEvent.SetC2(MVector(1.0, 0.0, 0.0));
  ARMEvent.SetDe(MVector(0.0, 0.0, 1.0));
  Passed = EvaluateTrue("Validate()", "arm geometry", "A representative Compton geometry validates successfully", ARMEvent.Validate()) && Passed;
  Passed = EvaluateNear("Phi()", "arm geometry", "The geometry produces a right-angle Compton scatter", ARMEvent.Phi(), c_Pi/2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetARMGamma()", "arm geometry", "ARM gamma is zero on the cone surface", ARMEvent.GetARMGamma(MVector(0.0, 1.0, 0.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetARMElectron()", "arm geometry", "ARM electron is zero on the electron cone surface", ARMEvent.GetARMElectron(MVector(sin(ARMEvent.Epsilon()), 0.0, -cos(ARMEvent.Epsilon()))), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSPDElectron()", "arm geometry", "SPD electron is zero for a point on the reconstructed electron plane", ARMEvent.GetSPDElectron(ARMEvent.GetPosition() + ARMEvent.GetOrigin()), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAzimuthalScatterAngle()", "arm geometry", "The azimuthal scatter angle is zero on the reference axis", ARMEvent.GetAzimuthalScatterAngle(MVector(0.0, 0.0, 1.0)), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Data()", "arm geometry", "Data() returns the physical-event self pointer", ARMEvent.Data() == static_cast<MPhysicalEvent*>(&ARMEvent)) && Passed;
  Passed = EvaluateTrue("ToString()", "arm geometry", "ToString() includes the Compton event header", ARMEvent.ToString().Contains("Compton event")) && Passed;

  MComptonEvent PhysicsSeed;
  PhysicsSeed.SetEg(420.0);
  PhysicsSeed.SetdEg(4.2);
  PhysicsSeed.SetEe(180.0);
  PhysicsSeed.SetdEe(1.8);
  PhysicsSeed.SetC1(MVector(1.0, 0.0, 0.0));
  PhysicsSeed.SetdC1(MVector(0.1, 0.1, 0.1));
  PhysicsSeed.SetC2(MVector(0.0, 1.0, 0.0));
  PhysicsSeed.SetdC2(MVector(0.2, 0.2, 0.2));
  PhysicsSeed.SetDe(MVector(0.0, 0.0, 1.0));
  PhysicsSeed.SetdDe(MVector(0.01, 0.02, 0.03));
  PhysicsSeed.SetSequenceLength(3);
  PhysicsSeed.SetTrackLength(4);
  Passed = EvaluateTrue("Validate()", "physics seed", "The seeded Compton event validates", PhysicsSeed.Validate()) && Passed;
  Passed = Evaluate("Eg()", "physics seed", "The scattered gamma energy is stored", PhysicsSeed.Eg(), 420.0) && Passed;
  Passed = Evaluate("dEg()", "physics seed", "The scattered gamma energy uncertainty is stored", PhysicsSeed.dEg(), 4.2) && Passed;
  Passed = Evaluate("Ee()", "physics seed", "The recoil electron energy is stored", PhysicsSeed.Ee(), 180.0) && Passed;
  Passed = Evaluate("dEe()", "physics seed", "The recoil electron energy uncertainty is stored", PhysicsSeed.dEe(), 1.8) && Passed;
  Passed = Evaluate("C1()", "physics seed", "The first interaction position is stored", PhysicsSeed.C1(), MVector(1.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("dC1()", "physics seed", "The first interaction position uncertainty is stored", PhysicsSeed.dC1(), MVector(0.1, 0.1, 0.1)) && Passed;
  Passed = Evaluate("C2()", "physics seed", "The second interaction position is stored", PhysicsSeed.C2(), MVector(0.0, 1.0, 0.0)) && Passed;
  Passed = Evaluate("dC2()", "physics seed", "The second interaction position uncertainty is stored", PhysicsSeed.dC2(), MVector(0.2, 0.2, 0.2)) && Passed;
  Passed = Evaluate("De()", "physics seed", "The recoil-electron direction is stored", PhysicsSeed.De(), MVector(0.0, 0.0, 1.0)) && Passed;
  Passed = Evaluate("dDe()", "physics seed", "The recoil-electron direction uncertainty is stored", PhysicsSeed.dDe(), MVector(0.01, 0.02, 0.03)) && Passed;
  Passed = Evaluate("ToF()", "physics seed", "The time of flight is stored", PhysicsSeed.ToF(), 0.0) && Passed;
  Passed = Evaluate("dToF()", "physics seed", "The time of flight uncertainty is stored", PhysicsSeed.dToF(), 0.0) && Passed;
  Passed = Evaluate("ClusteringQualityFactor()", "physics seed", "The clustering quality factor defaults to zero", PhysicsSeed.ClusteringQualityFactor(), 0.0) && Passed;
  Passed = Evaluate("SequenceLength()", "physics seed", "The sequence length is stored", PhysicsSeed.SequenceLength(), 3U) && Passed;
  Passed = Evaluate("ComptonQualityFactor1()", "physics seed", "The first Compton quality factor defaults to zero", PhysicsSeed.ComptonQualityFactor1(), 0.0) && Passed;
  Passed = Evaluate("ComptonQualityFactor2()", "physics seed", "The second Compton quality factor defaults to zero", PhysicsSeed.ComptonQualityFactor2(), 0.0) && Passed;
  Passed = Evaluate("TrackLength()", "physics seed", "The track length is stored", PhysicsSeed.TrackLength(), 4U) && Passed;
  Passed = Evaluate("TrackInitialDeposit()", "physics seed", "The initial track deposit remains at the default without hit-based assimilation", PhysicsSeed.TrackInitialDeposit(), 0.0) && Passed;
  Passed = Evaluate("TrackQualityFactor1()", "physics seed", "The first track quality factor defaults to zero", PhysicsSeed.TrackQualityFactor1(), 0.0) && Passed;
  Passed = Evaluate("TrackQualityFactor2()", "physics seed", "The second track quality factor defaults to zero", PhysicsSeed.TrackQualityFactor2(), 0.0) && Passed;
  Passed = Evaluate("Ei()", "physics seed", "The initial gamma-ray energy is stored", PhysicsSeed.Ei(), 600.0) && Passed;
  Passed = EvaluateNear("dEi()", "physics seed", "The initial gamma-ray uncertainty is derived from Ee and Eg", PhysicsSeed.dEi(), 4.5694638635183455, 1e-12) && Passed;
  Passed = Evaluate("GetEnergy()", "physics seed", "The event energy matches the initial gamma-ray energy", PhysicsSeed.GetEnergy(), 600.0) && Passed;
  Passed = Evaluate("GetPosition()", "physics seed", "The event position is the first interaction", PhysicsSeed.GetPosition(), MVector(1.0, 0.0, 0.0)) && Passed;
  Passed = EvaluateNear("Dg().X()", "physics seed", "The scattered gamma direction x component matches the geometry", PhysicsSeed.Dg().X(), -0.7071067811865475, 1e-12) && Passed;
  Passed = EvaluateNear("Dg().Y()", "physics seed", "The scattered gamma direction y component matches the geometry", PhysicsSeed.Dg().Y(), 0.7071067811865475, 1e-12) && Passed;
  Passed = EvaluateNear("Dg().Z()", "physics seed", "The scattered gamma direction z component matches the geometry", PhysicsSeed.Dg().Z(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Di().X()", "physics seed", "The incoming gamma direction x component matches the kinematics", PhysicsSeed.Di().X(), 0.4738827127559827, 1e-12) && Passed;
  Passed = EvaluateNear("Di().Y()", "physics seed", "The incoming gamma direction y component matches the kinematics", PhysicsSeed.Di().Y(), -0.4738827127559827, 1e-12) && Passed;
  Passed = EvaluateNear("Di().Z()", "physics seed", "The incoming gamma direction z component matches the kinematics", PhysicsSeed.Di().Z(), -0.7422064059963789, 1e-12) && Passed;
  Passed = EvaluateNear("DiOnCone().X()", "physics seed", "The cone-projected incoming direction x component matches the geometry", PhysicsSeed.DiOnCone().X(), 0.4490133111297299, 1e-12) && Passed;
  Passed = EvaluateNear("DiOnCone().Y()", "physics seed", "The cone-projected incoming direction y component matches the geometry", PhysicsSeed.DiOnCone().Y(), -0.4490133111297299, 1e-12) && Passed;
  Passed = EvaluateNear("DiOnCone().Z()", "physics seed", "The cone-projected incoming direction z component matches the geometry", PhysicsSeed.DiOnCone().Z(), -0.7725115486881944, 1e-12) && Passed;
  Passed = EvaluateNear("Theta()", "physics seed", "The total scatter angle from energies matches the reference value", PhysicsSeed.Theta(), 1.6547388192953052, 1e-12) && Passed;
  Passed = EvaluateNear("DeltaTheta()", "physics seed", "The energy-vs-geometry scatter-angle difference matches the reference value", PhysicsSeed.DeltaTheta(), 0.08394249250040864, 1e-12) && Passed;
  Passed = EvaluateNear("FirstLeverArm()", "physics seed", "The first lever arm is the C1-to-C2 distance", PhysicsSeed.FirstLeverArm(), sqrt(2.0), 1e-12) && Passed;
  Passed = EvaluateNear("MinLeverArm()", "physics seed", "The deprecated minimum lever arm mirrors the first lever arm", PhysicsSeed.MinLeverArm(), sqrt(2.0), 1e-12) && Passed;
  Passed = EvaluateNear("AnyLeverArm()", "physics seed", "The generic lever arm helper stores the configured value", PhysicsSeed.AnyLeverArm(), sqrt(2.0), 1e-12) && Passed;
  Passed = EvaluateNear("LeverArm()", "physics seed", "The lever-arm helper is stored", PhysicsSeed.LeverArm(), sqrt(2.0), 1e-12) && Passed;
  Passed = Evaluate("CoincidenceWindow()", "physics seed", "The coincidence window defaults to zero", PhysicsSeed.CoincidenceWindow().GetAsDouble(), 0.0) && Passed;
  Passed = EvaluateTrue("HasTrack()", "physics seed", "A non-zero electron direction creates a track", PhysicsSeed.HasTrack()) && Passed;
  Passed = EvaluateTrue("GetOrigin()", "physics seed", "A tracked Compton event exposes an origin direction", PhysicsSeed.GetOrigin() != g_VectorNotDefined) && Passed;
  Passed = EvaluateNear("CalculateThetaViaAngles()", "physics seed", "The geometric scatter angle matches the reference geometry", PhysicsSeed.CalculateThetaViaAngles(), c_Pi / 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetKleinNishina() const", "physics seed", "The event Klein-Nishina value matches the reference calculation", PhysicsSeed.GetKleinNishina(), 2.2729424385174867e-30, 1e-34) && Passed;
  Passed = EvaluateNear("dPhi()", "physics seed", "The Compton angle uncertainty matches the reference propagation", PhysicsSeed.dPhi(), 0.22377919495214327, 1e-12) && Passed;
  Passed = EvaluateTrue("Data()", "physics seed", "Data() returns the physical-event self pointer", PhysicsSeed.Data() == static_cast<MPhysicalEvent*>(&PhysicsSeed)) && Passed;
  Passed = EvaluateTrue("ToString()", "physics seed", "ToString() includes the Compton event header", PhysicsSeed.ToString().Contains("Compton event")) && Passed;

  MPhysicalEvent* Duplicate = ARMEvent.Duplicate();
  Passed = EvaluateTrue("Duplicate()", "Compton duplicate", "Duplicate keeps the concrete Compton type", dynamic_cast<MComptonEvent*>(Duplicate) != nullptr) && Passed;
  Passed = Evaluate("Duplicate()->GetEnergy()", "Compton duplicate", "Duplicate preserves the total energy", Duplicate->GetEnergy(), ARMEvent.GetEnergy()) && Passed;
  delete Duplicate;

  MComptonEvent ResetEvent;
  ResetEvent.SetEg(111.0);
  ResetEvent.SetEe(222.0);
  ResetEvent.SetC1(MVector(1.0, 2.0, 3.0));
  ResetEvent.SetC2(MVector(4.0, 5.0, 6.0));
  ResetEvent.SetDe(MVector(0.0, 1.0, 0.0));
  ResetEvent.SetSequenceLength(7);
  ResetEvent.SetTrackLength(8);
  ResetEvent.Reset();
  Passed = Evaluate("Reset().Eg()", "reset", "Reset clears the scattered gamma energy", ResetEvent.Eg(), 0.0) && Passed;
  Passed = Evaluate("Reset().Ee()", "reset", "Reset clears the recoil electron energy", ResetEvent.Ee(), 0.0) && Passed;
  Passed = Evaluate("Reset().C1()", "reset", "Reset clears the first interaction position", ResetEvent.C1(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("Reset().C2()", "reset", "Reset clears the second interaction position", ResetEvent.C2(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("Reset().SequenceLength()", "reset", "Reset restores the default sequence length", ResetEvent.SequenceLength(), 2U) && Passed;
  Passed = Evaluate("Reset().TrackLength()", "reset", "Reset restores the default track length", ResetEvent.TrackLength(), 1U) && Passed;
  Passed = Evaluate("Reset().CoincidenceWindow()", "reset", "Reset clears the coincidence window", ResetEvent.CoincidenceWindow().GetAsDouble(), 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test assimilation from hits and text lines
bool UTComptonEvent::TestAssimilation()
{
  bool Passed = true;

  vector<MPhysicalEventHit> Hits;
  MPhysicalEventHit Hit1;
  Hit1.Set(MVector(10.0, 0.0, 0.0), MVector(0.1, 0.1, 0.1), 111.0, 1.1, MTime(1.0), MTime(0.01));
  Hits.push_back(Hit1);
  MPhysicalEventHit Hit2;
  Hit2.Set(MVector(20.0, 0.0, 0.0), MVector(0.2, 0.2, 0.2), 222.0, 2.2, MTime(2.0), MTime(0.02));
  Hits.push_back(Hit2);

  MComptonEvent HitEvent;
  Passed = EvaluateTrue("Assimilate(vector<hit>)", "2 hits", "A Compton event can be assimilated from two hits", HitEvent.Assimilate(Hits)) && Passed;
  Passed = Evaluate("HitEvent.GetEnergy()", "2 hits", "The hit-based Compton energy is the sum of the deposits", HitEvent.GetEnergy(), 333.0) && Passed;
  Passed = Evaluate("HitEvent.GetPosition()", "2 hits", "The first hit becomes the event position", HitEvent.GetPosition(), Hit1.GetPosition()) && Passed;
  Passed = EvaluateSize("HitEvent.GetNHits()", "2 hits", "The event stores both hits", HitEvent.GetNHits(), 2U) && Passed;
  Passed = Evaluate("HitEvent.TrackInitialDeposit()", "2 hits", "The initial track deposit is the first hit energy", HitEvent.TrackInitialDeposit(), 111.0) && Passed;
  Passed = Evaluate("HitEvent.SequenceLength()", "2 hits", "The sequence length reflects the number of hits", HitEvent.SequenceLength(), 2U) && Passed;
  Passed = EvaluateNear("HitEvent.FirstLeverArm()", "2 hits", "The first lever arm is the hit distance", HitEvent.FirstLeverArm(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("HitEvent.AnyLeverArm()", "2 hits", "The lever-arm helper is populated", HitEvent.AnyLeverArm(), 10.0, 1e-12) && Passed;

  MComptonEvent TooFewHits;
  vector<MPhysicalEventHit> OneHit(1, Hit1);
  __merr.Enable(false);
  Passed = EvaluateFalse("Assimilate(vector<hit>)", "1 hit", "At least two hits are required", TooFewHits.Assimilate(OneHit)) && Passed;
  __merr.Enable(true);

  MComptonEvent LineEvent;
  string Basic = "C;1;2;3;4;5;6;0;0;1;100;500;1;2\n";
  vector<char> Buffer(Basic.begin(), Basic.end());
  Buffer.push_back('\0');
  Passed = EvaluateTrue("Assimilate(char*)", "basic line", "The compact C-line representation round-trips", LineEvent.Assimilate(Buffer.data())) && Passed;
  Passed = Evaluate("LineEvent.C1()", "basic line", "The line representation preserves the first interaction", LineEvent.C1(), MVector(1.0, 2.0, 3.0)) && Passed;
  Passed = Evaluate("LineEvent.C2()", "basic line", "The line representation preserves the second interaction", LineEvent.C2(), MVector(4.0, 5.0, 6.0)) && Passed;
  Passed = Evaluate("LineEvent.GetEnergy()", "basic line", "The line representation preserves the total energy", LineEvent.GetEnergy(), 600.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the serialization round-trip
bool UTComptonEvent::TestTraRoundTrip()
{
  bool Passed = true;

  MComptonEvent Event;
  Event.SetId(17);
  Event.SetTime(MTime(12.0));
  Event.SetEg(500.0);
  Event.SetdEg(5.0);
  Event.SetEe(100.0);
  Event.SetdEe(2.0);
  Event.SetC1(MVector(1.0, 0.0, 0.0));
  Event.SetdC1(MVector(0.1, 0.1, 0.1));
  Event.SetC2(MVector(0.0, 1.0, 0.0));
  Event.SetdC2(MVector(0.2, 0.2, 0.2));
  Event.SetDe(MVector(0.0, 0.0, 1.0));
  Event.SetdDe(MVector(0.01, 0.02, 0.03));
  Event.SetToF(3.0);
  Event.SetdToF(0.5);
  Event.SetClusteringQualityFactor(0.4);
  Event.SetSequenceLength(3);
  Event.SetComptonQualityFactor1(0.8);
  Event.SetComptonQualityFactor2(0.6);
  Event.SetTrackLength(4);
  Event.SetTrackInitialDeposit(12.5);
  Event.SetTrackQualityFactor1(0.7);
  Event.SetTrackQualityFactor2(0.3);
  Event.SetLeverArm(9.0);
  Event.SetCoincidenceWindow(MTime(0.25));
  Event.AddHit(MPhysicalEventHit());
  Passed = EvaluateTrue("Validate()", "round-trip seed", "The seeded Compton event validates", Event.Validate()) && Passed;

  MString Tra = Event.ToTraString();
  MComptonEvent Parsed;
  Passed = EvaluateTrue("ParseTraString()", "Compton tra", "The tra-string round-trips through ParseLine", ParseTraString(Parsed, Tra)) && Passed;
  Passed = Evaluate("Parsed.GetId()", "Compton tra", "The round-trip preserves the event id", Parsed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Parsed.GetEnergy()", "Compton tra", "The round-trip preserves the total energy", Parsed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Parsed.C1()", "Compton tra", "The round-trip preserves the first interaction", Parsed.C1(), Event.C1()) && Passed;
  Passed = Evaluate("Parsed.C2()", "Compton tra", "The round-trip preserves the second interaction", Parsed.C2(), Event.C2()) && Passed;
  Passed = Evaluate("Parsed.ToF()", "Compton tra", "The round-trip preserves the time of flight", Parsed.ToF(), Event.ToF()) && Passed;
  Passed = Evaluate("Parsed.dToF()", "Compton tra", "The round-trip preserves the time-of-flight uncertainty", Parsed.dToF(), Event.dToF()) && Passed;
  Passed = Evaluate("Parsed.ClusteringQualityFactor()", "Compton tra", "The round-trip preserves the clustering quality factor", Parsed.ClusteringQualityFactor(), Event.ClusteringQualityFactor()) && Passed;
  Passed = Evaluate("Parsed.SequenceLength()", "Compton tra", "The round-trip preserves the sequence length", Parsed.SequenceLength(), Event.SequenceLength()) && Passed;
  Passed = Evaluate("Parsed.ComptonQualityFactor1()", "Compton tra", "The round-trip preserves the first Compton quality factor", Parsed.ComptonQualityFactor1(), Event.ComptonQualityFactor1()) && Passed;
  Passed = Evaluate("Parsed.ComptonQualityFactor2()", "Compton tra", "The round-trip preserves the second Compton quality factor", Parsed.ComptonQualityFactor2(), Event.ComptonQualityFactor2()) && Passed;
  Passed = Evaluate("Parsed.TrackLength()", "Compton tra", "The round-trip preserves the track length", Parsed.TrackLength(), Event.TrackLength()) && Passed;
  Passed = Evaluate("Parsed.TrackInitialDeposit()", "Compton tra", "The round-trip preserves the initial track deposit", Parsed.TrackInitialDeposit(), Event.TrackInitialDeposit()) && Passed;
  Passed = Evaluate("Parsed.TrackQualityFactor1()", "Compton tra", "The round-trip preserves the first track quality factor", Parsed.TrackQualityFactor1(), Event.TrackQualityFactor1()) && Passed;
  Passed = Evaluate("Parsed.TrackQualityFactor2()", "Compton tra", "The round-trip preserves the second track quality factor", Parsed.TrackQualityFactor2(), Event.TrackQualityFactor2()) && Passed;
  Passed = Evaluate("Parsed.AnyLeverArm()", "Compton tra", "The round-trip preserves the lever arm", Parsed.AnyLeverArm(), Event.AnyLeverArm()) && Passed;
  Passed = Evaluate("Parsed.CoincidenceWindow()", "Compton tra", "The round-trip preserves the coincidence window", Parsed.CoincidenceWindow().GetAsDouble(), Event.CoincidenceWindow().GetAsDouble()) && Passed;
  Passed = Evaluate("Parsed.GetNHits()", "Compton tra", "The round-trip preserves the stored hits", Parsed.GetNHits(), Event.GetNHits()) && Passed;

  MComptonEvent Switched;
  Switched.Assimilate(MVector(0.0, 0.0, 0.0), MVector(0.0, 0.0, 1.0), MVector(0.0, 0.0, 1.0), 300.0, 300.0);
  Passed = EvaluateTrue("SwitchDirection()", "valid event", "SwitchDirection succeeds on a valid event", Switched.SwitchDirection()) && Passed;
  Passed = Evaluate("Switched.Eg()", "valid event", "SwitchDirection preserves the gamma energy for a symmetric event", Switched.Eg(), 300.0) && Passed;
  Passed = Evaluate("Switched.Ee()", "valid event", "SwitchDirection preserves the electron energy for a symmetric event", Switched.Ee(), 300.0) && Passed;
  Passed = Evaluate("Switched.C1()", "valid event", "SwitchDirection reverses the interaction order", Switched.C1(), MVector(0.0, 0.0, 1.0)) && Passed;
  Passed = Evaluate("Switched.C2()", "valid event", "SwitchDirection reverses the interaction order", Switched.C2(), MVector(0.0, 0.0, 0.0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy-style assimilation and file-stream round-trip
bool UTComptonEvent::TestCopyAndStream()
{
  bool Passed = true;

  MComptonEvent Event;
  Event.SetId(9);
  Event.SetTime(MTime(5.0));
  Event.SetEg(420.0);
  Event.SetdEg(4.2);
  Event.SetEe(180.0);
  Event.SetdEe(1.8);
  Event.SetC1(MVector(1.0, 0.0, 0.0));
  Event.SetdC1(MVector(0.1, 0.1, 0.1));
  Event.SetC2(MVector(0.0, 1.0, 0.0));
  Event.SetdC2(MVector(0.2, 0.2, 0.2));
  Event.SetDe(MVector(0.0, 0.0, 1.0));
  Event.SetdDe(MVector(0.01, 0.02, 0.03));
  Event.SetToF(2.5);
  Event.SetdToF(0.25);
  Event.SetClusteringQualityFactor(0.4);
  Event.SetSequenceLength(3);
  Event.SetComptonQualityFactor1(0.8);
  Event.SetComptonQualityFactor2(0.6);
  Event.SetTrackLength(4);
  Event.SetTrackInitialDeposit(12.5);
  Event.SetTrackQualityFactor1(0.7);
  Event.SetTrackQualityFactor2(0.3);
  Event.SetLeverArm(9.0);
  Event.AddHit(MPhysicalEventHit());
  Passed = EvaluateTrue("Validate()", "seeded copy", "The seeded Compton event validates", Event.Validate()) && Passed;

  MComptonEvent CopiedCompton;
  Passed = EvaluateTrue("Assimilate(MComptonEvent*)", "copy", "Copy-style assimilation from the concrete type works", CopiedCompton.Assimilate(&Event)) && Passed;
  Passed = Evaluate("CopiedCompton.GetEnergy()", "copy", "Copy-style assimilation preserves the total energy", CopiedCompton.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("CopiedCompton.C1()", "copy", "Copy-style assimilation preserves the first interaction", CopiedCompton.C1(), Event.C1()) && Passed;

  MPhysicalEvent* CopiedPhysical = new MComptonEvent();
  Passed = EvaluateTrue("Assimilate(MPhysicalEvent*)", "copy", "Copy-style assimilation from a physical pointer works", CopiedPhysical->Assimilate(&Event)) && Passed;
  Passed = Evaluate("CopiedPhysical->GetEnergy()", "copy", "Base-pointer assimilation preserves the total energy", CopiedPhysical->GetEnergy(), Event.GetEnergy()) && Passed;
  delete CopiedPhysical;

  MFile File;
  MString FileName = "/tmp/UTComptonEvent.tra";
  Passed = EvaluateTrue("Open()", "stream write", "The temporary Compton file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream write", "The Compton event write-stream completes at EOF", Event.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  MComptonEvent Streamed;
  Passed = EvaluateTrue("Open()", "stream read", "The temporary Compton file can be reopened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateFalse("Stream()", "stream read", "The Compton event read-stream also returns false at EOF", Streamed.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("Streamed.GetId()", "stream read", "The streamed copy preserves the event id", Streamed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Streamed.GetEnergy()", "stream read", "The streamed copy preserves the total energy", Streamed.GetEnergy(), Event.GetEnergy()) && Passed;
  Passed = Evaluate("Streamed.C1()", "stream read", "The streamed copy preserves the first interaction", Streamed.C1(), Event.C1()) && Passed;
  File.Close();

  Passed = EvaluateTrue("Open()", "stream delayed", "The temporary Compton file can be reopened for delayed reading", File.Open(FileName, MFile::c_Read)) && Passed;
  MComptonEvent Delayed;
  Passed = EvaluateFalse("Stream()", "stream delayed", "Delayed read stores the payload and returns false at EOF", Delayed.Stream(File, 0, true, false, true)) && Passed;
  Passed = EvaluateTrue("ParseDelayed()", "stream delayed", "Delayed parse reconstructs the event", Delayed.ParseDelayed()) && Passed;
  Passed = Evaluate("Delayed.GetId()", "stream delayed", "The delayed copy preserves the event id", Delayed.GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Delayed.GetEnergy()", "stream delayed", "The delayed copy preserves the total energy", Delayed.GetEnergy(), Event.GetEnergy()) && Passed;
  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTComptonEvent Test;
  return Test.Run() ? 0 : 1;
}
