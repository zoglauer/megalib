/*
 * UTRevanShared.h
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


#ifndef __UTRevanShared__
#define __UTRevanShared__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MDDetector.h"
#include "MGeometryRevan.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MRESE.h"
#include "MString.h"
#include "MVector.h"

// The geometry fixtures (head of a geometry, scanning from a private file) are shared with the geomega unit tests
#include "../../geomega/unittests/UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! A RESE which counts its living objects, to test who deletes it
class UTRevanCountedRESE : public MRESE
{
public:
  //! Default constructor
  UTRevanCountedRESE() : MRESE() { ++m_NumberOfLivingObjects; }
  //! Default destructor
  virtual ~UTRevanCountedRESE() { --m_NumberOfLivingObjects; }

  //! The number of objects of this class which exist at the moment
  inline static int m_NumberOfLivingObjects = 0;
};


////////////////////////////////////////////////////////////////////////////////


//! Base class of the revan unit tests: a small telescope geometry and hits in it
class UTRevanBase : public UTGeomegaBase
{
public:
  //! Standard constructor with the name of the test
  UTRevanBase(const MString& Name) : UTGeomegaBase(Name) {}
  //! Default destructor
  virtual ~UTRevanBase() {}

protected:
  //! Return the z position of the center of tracker layer 0 to 3 in cm
  double GetTrackerLayerZ(unsigned int Layer) const
  {
    return 1.0*Layer;
  }

  //! Return the test telescope. All detectors have ideal position grids, no failures, and their own trigger:
  //! * Tracker (Strip2D): four silicon wafers of 10 x 10 x 0.05 cm, centered on the z axis at z = 0, 1, 2, 3 cm (layer 0 to 3),
  //!   layer distance (structural pitch in z) 1 cm, 9 x 9 strips of pitch 1 cm (strip centers at -4, -3, ..., 4 cm), 0.5 cm guard ring, noise threshold 10 keV, trigger threshold 30 keV, energy resolution 2 keV (1 sigma)
  //! * Calo (Calorimeter): one CsI block of 10 x 10 x 8 cm centered at (0, 0, -10) cm, noise threshold 20 keV, trigger threshold 50 keV, energy resolution 10 keV (1 sigma)
  //! * Depth (Strip3D): one germanium wafer of 10 x 10 x 2 cm centered at (30, 0, 0) cm, 10 x 10 strips of pitch 1 cm, depth resolution 0.1 cm, noise threshold 10 keV, trigger threshold 30 keV, energy resolution 1 keV (1 sigma)
  //! * Shield (Scintillator): one plastic plate of 20 x 20 x 1 cm centered at (0, 0, 10) cm, noise threshold equals the trigger threshold of 50 keV, ideal energy resolution, a veto
  //! Additional lines (e.g. "DetectorEffectsEngine 2\n") can be appended
  //! With ideal energy resolutions the tracker, the calorimeter, and the depth detector measure the energies exactly
  MString GetTelescopeGeometry(const MString& AdditionalLines = "", bool IdealEnergyResolutions = false) const
  {
    MString Setup = GetGeometryHead();

    // The tracker wafers are placed in holders:
    Setup += "Volume Holder\n";
    Setup += "Holder.Material Vacuum\n";
    Setup += "Holder.Shape BRIK 6.0 6.0 0.1\n";
    Setup += "Volume Wafer\n";
    Setup += "Wafer.Material Silicon\n";
    Setup += "Wafer.Shape BRIK 5.0 5.0 0.025\n";
    Setup += "Wafer.Position 0.0 0.0 0.0\n";
    Setup += "Wafer.Mother Holder\n";
    Setup += "Holder.Copy Holder0\n";
    Setup += "Holder0.Position 0.0 0.0 0.0\n";
    Setup += "Holder0.Mother World\n";
    Setup += "Holder.Copy Holder1\n";
    Setup += "Holder1.Position 0.0 0.0 1.0\n";
    Setup += "Holder1.Mother World\n";
    Setup += "Holder.Copy Holder2\n";
    Setup += "Holder2.Position 0.0 0.0 2.0\n";
    Setup += "Holder2.Mother World\n";
    Setup += "Holder.Copy Holder3\n";
    Setup += "Holder3.Position 0.0 0.0 3.0\n";
    Setup += "Holder3.Mother World\n";

    Setup += "Volume CalorimeterBlock\n";
    Setup += "CalorimeterBlock.Material CsI\n";
    Setup += "CalorimeterBlock.Shape BRIK 5.0 5.0 4.0\n";
    Setup += "CalorimeterBlock.Position 0.0 0.0 -10.0\n";
    Setup += "CalorimeterBlock.Mother World\n";

    Setup += "Volume DepthWafer\n";
    Setup += "DepthWafer.Material Germanium\n";
    Setup += "DepthWafer.Shape BRIK 5.0 5.0 1.0\n";
    Setup += "DepthWafer.Position 30.0 0.0 0.0\n";
    Setup += "DepthWafer.Mother World\n";

    Setup += "Volume ShieldPlate\n";
    Setup += "ShieldPlate.Material NE110\n";
    Setup += "ShieldPlate.Shape BRIK 10.0 10.0 0.5\n";
    Setup += "ShieldPlate.Position 0.0 0.0 10.0\n";
    Setup += "ShieldPlate.Mother World\n";

    Setup += "Strip2D Tracker\n";
    Setup += "Tracker.DetectorVolume Wafer\n";
    Setup += "Tracker.SensitiveVolume Wafer\n";
    Setup += "Tracker.StructuralPitch 0.0 0.0 1.0\n";
    Setup += "Tracker.StructuralOffset 0.0 0.0 0.0\n";
    Setup += "Tracker.Offset 0.5 0.5\n";
    Setup += "Tracker.StripNumber 9 9\n";
    Setup += "Tracker.NoiseThreshold 10\n";
    Setup += "Tracker.TriggerThreshold 30\n";
    if (IdealEnergyResolutions == true) {
      Setup += "Tracker.EnergyResolution Ideal\n";
    } else {
      Setup += "Tracker.EnergyResolution Gauss 10 10 2\n";
      Setup += "Tracker.EnergyResolution Gauss 10000 10000 2\n";
    }
    Setup += "Tracker.FailureRate 0.0\n";

    Setup += "Calorimeter Calo\n";
    Setup += "Calo.DetectorVolume CalorimeterBlock\n";
    Setup += "Calo.SensitiveVolume CalorimeterBlock\n";
    Setup += "Calo.NoiseThreshold 20\n";
    Setup += "Calo.TriggerThreshold 50\n";
    if (IdealEnergyResolutions == true) {
      Setup += "Calo.EnergyResolution Ideal\n";
    } else {
      Setup += "Calo.EnergyResolution Gauss 10 10 10\n";
      Setup += "Calo.EnergyResolution Gauss 10000 10000 10\n";
    }
    Setup += "Calo.FailureRate 0.0\n";

    Setup += "Strip3D Depth\n";
    Setup += "Depth.DetectorVolume DepthWafer\n";
    Setup += "Depth.SensitiveVolume DepthWafer\n";
    Setup += "Depth.Offset 0.0 0.0\n";
    Setup += "Depth.StripNumber 10 10\n";
    Setup += "Depth.DepthResolution 10 0.1 0\n";
    Setup += "Depth.DepthResolution 10000 0.1 0\n";
    Setup += "Depth.NoiseThreshold 10\n";
    Setup += "Depth.TriggerThreshold 30\n";
    if (IdealEnergyResolutions == true) {
      Setup += "Depth.EnergyResolution Ideal\n";
    } else {
      Setup += "Depth.EnergyResolution Gauss 10 10 1\n";
      Setup += "Depth.EnergyResolution Gauss 10000 10000 1\n";
    }
    Setup += "Depth.FailureRate 0.0\n";

    Setup += "Scintillator Shield\n";
    Setup += "Shield.DetectorVolume ShieldPlate\n";
    Setup += "Shield.SensitiveVolume ShieldPlate\n";
    Setup += "Shield.NoiseThresholdEqualsTriggerThreshold true\n";
    Setup += "Shield.TriggerThreshold 50\n";
    Setup += "Shield.EnergyResolution Ideal\n";
    Setup += "Shield.FailureRate 0.0\n";

    Setup += "Trigger TrackerTrigger\n";
    Setup += "TrackerTrigger.Veto false\n";
    Setup += "TrackerTrigger.TriggerByChannel true\n";
    Setup += "TrackerTrigger.Detector Tracker 1\n";
    Setup += "Trigger CalorimeterTrigger\n";
    Setup += "CalorimeterTrigger.Veto false\n";
    Setup += "CalorimeterTrigger.TriggerByChannel true\n";
    Setup += "CalorimeterTrigger.Detector Calo 1\n";
    Setup += "Trigger DepthTrigger\n";
    Setup += "DepthTrigger.Veto false\n";
    Setup += "DepthTrigger.TriggerByChannel true\n";
    Setup += "DepthTrigger.Detector Depth 1\n";
    Setup += "Trigger ShieldVeto\n";
    Setup += "ShieldVeto.Veto true\n";
    Setup += "ShieldVeto.TriggerByChannel true\n";
    Setup += "ShieldVeto.Detector Shield 1\n";

    Setup += AdditionalLines;

    return Setup;
  }

  //! Return the lines which give the tracker of the test telescope a guard ring: trigger threshold 40 keV, and the given energy resolution (keV, 1 sigma)
  MString GetGuardRingLines(double EnergyResolution = 5.0) const
  {
    MString Lines;
    Lines += "Tracker.GuardRingTriggerThreshold 40\n";
    Lines += MString("Tracker.GuardRingEnergyResolution 100 ") + EnergyResolution + "\n";
    Lines += MString("Tracker.GuardRingEnergyResolution 1000 ") + EnergyResolution + "\n";

    return Lines;
  }

  //! Return a Strip3D detector whose detector volume is larger than its sensitive volume:
  //! an aluminium frame of 10 x 10 x 2 cm centered at the origin is the detector volume, a germanium wafer of 6 x 6 x 2 cm in its center the sensitive volume,
  //! 6 x 6 strips of pitch 1 cm, depth resolution 0.1 cm, energy resolution 2 keV (1 sigma); positions with |x| or |y| between 3 and 5 cm are in the detector but not in the sensitive volume
  MString GetFramedDetectorGeometry() const
  {
    MString Setup = GetGeometryHead();
    Setup += "Volume Frame\n";
    Setup += "Frame.Material Aluminium\n";
    Setup += "Frame.Shape BRIK 5.0 5.0 1.0\n";
    Setup += "Frame.Position 0.0 0.0 0.0\n";
    Setup += "Frame.Mother World\n";
    Setup += "Volume Wafer\n";
    Setup += "Wafer.Material Germanium\n";
    Setup += "Wafer.Shape BRIK 3.0 3.0 1.0\n";
    Setup += "Wafer.Position 0.0 0.0 0.0\n";
    Setup += "Wafer.Mother Frame\n";
    Setup += "MDStrip3D Framed\n";
    Setup += "Framed.DetectorVolume Frame\n";
    Setup += "Framed.SensitiveVolume Wafer\n";
    Setup += "Framed.StructuralPitch 0.0 0.0 0.0\n";
    Setup += "Framed.StructuralOffset 0.0 0.0 0.0\n";
    Setup += "Framed.Offset 0.0 0.0\n";
    Setup += "Framed.StripNumber 6 6\n";
    Setup += "Framed.NoiseThreshold 10.0\n";
    Setup += "Framed.TriggerThreshold 30.0\n";
    Setup += "Framed.DepthResolution 100.0 0.1\n";
    Setup += "Framed.DepthResolution 1000.0 0.1\n";
    Setup += "Framed.EnergyResolution Gauss 100.0 100.0 2.0\n";
    Setup += "Framed.EnergyResolution Gauss 1000.0 1000.0 2.0\n";
    Setup += "Trigger FramedTrigger\n";
    Setup += "FramedTrigger.Veto false\n";
    Setup += "FramedTrigger.TriggerByChannel true\n";
    Setup += "FramedTrigger.Detector Framed 1\n";
    return Setup;
  }

  //! Return a material definition that leaves the test telescope without a complete set of cross sections
  MString GetMaterialWithoutCrossSectionsLines() const
  {
    MString Setup;
    Setup += "Material UTNoCrossSections\n";
    Setup += "UTNoCrossSections.Density 1.0\n";
    Setup += "UTNoCrossSections.ComponentByAtoms H 1\n";
    return Setup;
  }

  //! Scan the test telescope into the geometry
  bool ScanTelescopeGeometry(MGeometryRevan& Geometry, const MString& AdditionalLines = "", bool IdealEnergyResolutions = false)
  {
    return ScanGeometry(Geometry, GetTelescopeGeometry(AdditionalLines, IdealEnergyResolutions), "Telescope.geo.setup");
  }

  //! Return a new hit with the given position (cm, world coordinates), energy (keV), time (s), and detector type (e.g. MDDetector::c_Strip2D)
  //! If a geometry is given, the volume sequence and the resolutions are retrieved from it; on failure a nullptr is returned
  //! The caller owns the hit, until it is added to an owning container
  MREHit* CreateHit(const MVector& Position, double Energy, int DetectorType, MGeometryRevan* Geometry = nullptr, double Time = 0.0)
  {
    MREHit* Hit = new MREHit();
    Hit->SetPosition(Position);
    Hit->SetEnergy(Energy);
    Hit->SetTime(Time);
    Hit->SetDetector(DetectorType);
    if (Geometry != nullptr) {
      DisableDefaultStreams();
      bool Retrieved = Hit->RetrieveResolutions(Geometry);
      EnableDefaultStreams();
      if (Retrieved == false) {
        delete Hit;
        return nullptr;
      }
    }

    return Hit;
  }

  //! Return a new raw event with one hit per position (cm) and energy (keV), with resolutions and detector types from the geometry
  //! The hits are in the given order, the event owns them, and the caller owns the event; on failure a nullptr is returned
  MRERawEvent* CreateEvent(MGeometryRevan& Geometry, const vector<MVector>& Positions, const vector<double>& Energies)
  {
    if (Positions.size() != Energies.size()) {
      return nullptr;
    }

    MRERawEvent* Event = new MRERawEvent(&Geometry);
    for (unsigned int i = 0; i < Positions.size(); ++i) {
      MREHit* Hit = CreateHit(Positions[i], Energies[i], MDDetector::c_Strip2D, &Geometry);
      if (Hit == nullptr) {
        delete Event;
        return nullptr;
      }
      Hit->SetDetector(Hit->GetVolumeSequence()->GetDetector()->GetType());
      Event->AddRESE(Hit);
    }

    return Event;
  }
};


#endif


// UTRevanShared.h: the end...
////////////////////////////////////////////////////////////////////////////////
