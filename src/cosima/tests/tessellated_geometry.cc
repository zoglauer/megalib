// Exercise the real Geomega parser and Cosima construction, without a GUI.
#include "MGlobal.h"
#include "MCParameterFile.hh"
#include "MCRunManager.hh"
#include "MCDetectorConstruction.hh"
#include "MDShapeTessellated.h"
#include "MDVolume.h"
#include "G4Box.hh"
#include "G4TessellatedSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Navigator.hh"
#include "G4GeometryManager.hh"
#include "G4SystemOfUnits.hh"
#include "TGeoTessellated.h"
#include "TClass.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

void Require(bool Value, const std::string& Message)
{
  if (!Value) throw std::runtime_error(Message);
}

void Near(double Actual, double Expected, const std::string& Message, double Tolerance = 1.e-5)
{
  Require(std::isfinite(Actual) && std::abs(Actual - Expected) <= Tolerance,
          Message + ": got " + std::to_string(Actual) + ", expected " + std::to_string(Expected));
}

G4VSolid* Solid(const std::string& Name)
{
  auto* Volume = G4LogicalVolumeStore::GetInstance()->GetVolume(Name + "Log", false);
  Require(Volume != nullptr, "Missing Geant4 logical volume " + Name);
  return Volume->GetSolid();
}

void CheckBox(MDShape* Root, G4VSolid* Geant)
{
  Require(Root != nullptr, "Missing ROOT box");
  G4Box Reference("Reference", 1*cm, 2*cm, 3*cm);
  Near(Root->GetVolume(), 48, "ROOT box volume (cm3)");
  Near(Geant->GetCubicVolume()/(cm*cm*cm), 48, "Geant4 box volume (cm3)");
  // Non-axis-aligned interior rays and rays entering each of the six faces.
  for (int i = 0; i < 60; ++i) {
    const double x = 0.8*std::sin(i + 0.3), y = 1.7*std::cos(i + 0.2), z = 2.5*std::sin(2*i + 0.1);
    const G4ThreeVector P(x*cm, y*cm, z*cm);
    const G4ThreeVector D = G4ThreeVector(std::cos(i + 0.7), std::sin(i + 0.4), 0.7).unit();
    Require(Root->IsInside(MVector(x,y,z)), "ROOT interior classification");
    Require(Geant->Inside(P) == kInside, "Geant4 interior classification");
    const double Expected = Reference.DistanceToOut(P, D)/cm;
    Near(Root->DistanceInsideOut(MVector(x,y,z), MVector(D.x(),D.y(),D.z())), Expected, "ROOT interior ray");
    Near(Geant->DistanceToOut(P, D)/cm, Expected, "Geant4 interior ray");
  }
  for (int Axis = 0; Axis < 3; ++Axis) {
    for (int Sign : {-1, 1}) {
      G4ThreeVector P(0.17*cm,0.23*cm,0.31*cm), D;
      P[Axis] = Sign*(Axis+3)*cm;
      D[Axis] = -Sign;
      Require(!Root->IsInside(MVector(P.x()/cm,P.y()/cm,P.z()/cm)), "ROOT exterior classification");
      Require(Geant->Inside(P) == kOutside, "Geant4 exterior classification");
      Near(Root->DistanceOutsideIn(MVector(P.x()/cm,P.y()/cm,P.z()/cm), MVector(D.x(),D.y(),D.z())), 2, "ROOT exterior ray");
      Near(Geant->DistanceToIn(P,D)/cm, 2, "Geant4 exterior ray");
    }
  }
  double Center[3] = {0,0,0};
  Near(Root->GetRootShape()->Safety(Center), 1, "ROOT safety");
  Near(Geant->DistanceToOut(G4ThreeVector())/cm, 1, "Geant4 safety");
  const double Outside[3] = {4,0,0}, Away[3] = {1,0,0};
  Require(Root->GetRootShape()->DistFromOutside(Outside,Away) > 1.e20, "ROOT missed ray");
  Require(Geant->DistanceToIn(G4ThreeVector(4*cm,0,0),G4ThreeVector(1,0,0)) > 1.e20, "Geant4 missed ray");
}

int main(int argc, char** argv)
{
  if (argc != 3) { std::cerr<<"Usage: tessellated_geometry SOURCE {fixtures|assembly|reject}\n"; return 2; }
  MGlobal::Initialize("Tessellated solid tests");
  try {
    MCParameterFile Parameters;
    Require(Parameters.Open(argv[1]), "Unable to parse test source");
    MCRunManager Manager(Parameters);
    MCDetectorConstruction Detector(Parameters);
    const bool Valid = Detector.Initialize();
    const std::string Mode = argv[2];
    if (Mode == "reject") {
      Require(!Valid, "Malformed mesh unexpectedly accepted");
      std::cout<<"PASS: invalid mesh rejected\n";
      return 0;
    }
    Require(Valid, "Cosima geometry initialization failed");
    Require(TClass::GetClass("MDShapeTessellated") != nullptr, "ROOT dictionary missing");
    auto* Geometry = Detector.GetGeometry();
    size_t Meshes = 0, Facets = 0;
    for (unsigned int v = 0; v < Geometry->GetNVolumes(); ++v) {
      auto* Volume = Geometry->GetVolumeAt(v);
      auto* Mesh = dynamic_cast<MDShapeTessellated*>(Volume->GetShape());
      if (Mesh == nullptr) continue;
      auto* Root = dynamic_cast<TGeoTessellated*>(Mesh->GetRootShape());
      // Copies share their template's logical volume.
      auto* Template = Volume->GetCloneTemplate();
      auto* Geant = dynamic_cast<G4TessellatedSolid*>(Solid((Template ? Template : Volume)->GetName().Data()));
      Require(Root && Root->IsClosedBody(), "Missing/invalid ROOT tessellated solid");
      Require(Geant && Geant->GetSolidClosed(), "Missing/unfinalized Geant4 tessellated solid");
      Require(static_cast<size_t>(Geant->GetNumberOfFacets()) == Mesh->GetFacets().size(), "Geant4 facet count differs");
      const double VolumeCM3 = Mesh->GetVolume();
      Near(Geant->GetCubicVolume()/(cm*cm*cm), VolumeCM3, "ROOT/Geant4 volume agreement", std::max(1.e-6, VolumeCM3*1.e-6));
      ++Meshes;
      Facets += Mesh->GetFacets().size();
    }
    Require(Meshes > 0, "No tessellated volumes exercised");
    if (Mode == "fixtures") {
      for (const char* Name : {"Mesh", "Inline", "STL", "ASCII", "Named", "Analytic"}) {
        CheckBox(Geometry->GetVolume(Name)->GetShape(), Solid(Name));
      }
      auto* Scaled = Geometry->GetVolume("Scaled")->GetShape();
      Near(Scaled->GetVolume(), 48*4, "Scaled ROOT volume");
      Near(Solid("Scaled")->GetCubicVolume()/(cm*cm*cm), 48*4, "Scaled Geant4 volume");
      Near(Scaled->DistanceInsideOut(MVector(0,0,0), MVector(1,0,0)), 2, "Scaled ROOT boundary");
      Near(Solid("Scaled")->DistanceToOut(G4ThreeVector(),G4ThreeVector(1,0,0))/cm, 2, "Scaled Geant4 boundary");
      auto* Concave = Geometry->GetVolume("Concave")->GetShape();
      auto* GConcave = Solid("Concave");
      Near(Concave->GetVolume(), 6, "Concave ROOT volume");
      Near(GConcave->GetCubicVolume()/(cm*cm*cm), 6, "Concave Geant4 volume");
      Require(Concave->IsInside(MVector(0.5,1.5,0)), "Concave ROOT interior");
      Require(!Concave->IsInside(MVector(1.5,1.5,0)), "Concave ROOT notch");
      Require(GConcave->Inside(G4ThreeVector(0.5*cm,1.5*cm,0)) == kInside, "Concave Geant4 interior");
      Require(GConcave->Inside(G4ThreeVector(1.5*cm,1.5*cm,0)) == kOutside, "Concave Geant4 notch");
      Near(Concave->DistanceOutsideIn(MVector(1.5,1.5,0), MVector(-1,0,0)), 0.5, "ROOT notch entry");
      Near(GConcave->DistanceToIn(G4ThreeVector(1.5*cm,1.5*cm,0), G4ThreeVector(-1,0,0))/cm, 0.5, "Geant4 notch entry");
      auto* World = Detector.Construct();
      for (int i = 0; i < World->GetLogicalVolume()->GetNoDaughters(); ++i) {
        Require(!World->GetLogicalVolume()->GetDaughter(i)->CheckOverlaps(1000, 1.e-6*cm, false), "Fixture overlaps");
      }
      G4GeometryManager::GetInstance()->CloseGeometry();
      G4Navigator Navigator;
      Navigator.SetWorldVolume(World);
      auto* Placed = Navigator.LocateGlobalPointAndSetup(G4ThreeVector(11.5*cm,0.5*cm,0));
      Require(Placed && Placed->GetLogicalVolume()->GetSolid() == Solid("Mesh"), "Rotated copy placement or solid sharing");
      double Safety = 0;
      Near(Navigator.ComputeStep(G4ThreeVector(11.5*cm,0.5*cm,0), G4ThreeVector(1,0,0), 100*cm, Safety)/cm, 0.5, "Rotated copy boundary");
      G4GeometryManager::GetInstance()->OpenGeometry();
    }
    std::cout<<"PASS: ROOT and Geant4 constructed "<<Meshes<<" tessellated volumes, "<<Facets<<" facets\n";
    return 0;
  } catch (const std::exception& Error) {
    std::cerr<<"FAIL: "<<Error.what()<<'\n';
    return 1;
  }
}
