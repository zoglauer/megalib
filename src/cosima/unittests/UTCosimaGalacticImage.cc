/*
 * UTCosimaGalacticImage.cc
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

// End-to-end test of the Galactic coordinate conventions:
// A point source at a known Galactic position (l, b) is simulated with Cosima while the detector points into a
// given direction (fixed Galactic orientation file), the events are reconstructed with Revan, and a Galactic
// image is made with Mimrec. The image has to peak at the position of the source.
//
// This verifies that the pointing conventions (left-handed Galactic system, definition of the x- and z-axis) are
// consistent between Cosima (orientation, written pointing), Revan, and Mimrec.
//
// Requires the programs cosima, revan, and mimrec (all three have to be compiled), and $MEGALIB.


// Cosima:
#include "UTFixtureShared.h"

// MEGAlib:
#include "MFile.h"
#include "MGlobal.h"
#include "MProcessManager.h"
#include "MString.h"
#include "MSystem.h"
#include "MUnitTest.h"
#include "MVector.h"

// ROOT:
#include "TApplication.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "TKey.h"
#include "TPad.h"
#include "TROOT.h"

// Standard lib:
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;


//! Unit test class for the Galactic pointing conventions from simulation to image
class UTCosimaGalacticImage : public MUnitTest
{
public:
  //! Default constructor
  UTCosimaGalacticImage() : MUnitTest("UTCosimaGalacticImage") {}
  //! Default destructor
  virtual ~UTCosimaGalacticImage() {}

  //! Run all tests
  virtual bool Run();

private:
  //! One pointing of the detector and the position of the source (all angles in degrees)
  struct Case
  {
    MString Name;
    //! Galactic latitude and longitude of the z-axis of the detector
    double ZLatitude;
    double ZLongitude;
    //! Rotation of the x-axis around the z-axis (any value gives a valid pair of perpendicular axes)
    double Roll;
    //! Galactic latitude and longitude of the source
    double SourceLatitude;
    double SourceLongitude;
    //! The random seed of the simulation
    unsigned int Seed;
  };

  //! Simulate, reconstruct, and image one case - return false if one of the steps fails
  //! (only runs external programs, thus it can be called from several threads - ROOT is not thread-safe)
  bool RunCase(const Case& C, const MString& Directory, const MString& Geometry);
  //! Write the orientation file and the Cosima source file
  bool WriteInput(const Case& C, const MString& Directory, const MString& Geometry);
  //! Find the position of the maximum in the Galactic image saved by Mimrec, return false on failure
  bool FindPeak(const MString& FileName, double& Longitude, double& Latitude);
  //! Unit vector of a Galactic position
  MVector ToVector(double Latitude, double Longitude) const;
  //! Angular distance in degrees between two Galactic positions
  double Separation(double Latitude1, double Longitude1, double Latitude2, double Longitude2) const;
  //! Run a program with its arguments in the given directory with its output in the log file there, return true if it exits normally with 0
  //! The program stays in the process group of the test: the test driver stops it with the test if the test times out; the time limit turns a hang into a failure
  bool Execute(const MString& Directory, const MString& Program, const MString& Arguments, const MString& LogFile) const;
  //! Return true if the file exists and is not empty
  bool IsNonEmptyFile(const MString& FileName) const;

  //! The number of triggers per simulation
  static const unsigned int c_NumberOfTriggers = 40000;
  //! Maximum distance in degrees between the image peak and the true source position (bins are 2 degrees)
  static constexpr double c_MaximumSeparation = 3.0;
};


////////////////////////////////////////////////////////////////////////////////


MVector UTCosimaGalacticImage::ToVector(double Latitude, double Longitude) const
{
  const double d2r = M_PI/180.0;
  return MVector(cos(Latitude*d2r)*cos(Longitude*d2r), cos(Latitude*d2r)*sin(Longitude*d2r), sin(Latitude*d2r));
}


////////////////////////////////////////////////////////////////////////////////


double UTCosimaGalacticImage::Separation(double Latitude1, double Longitude1, double Latitude2, double Longitude2) const
{
  double Dot = ToVector(Latitude1, Longitude1).Dot(ToVector(Latitude2, Longitude2));
  if (Dot > 1.0) Dot = 1.0;
  if (Dot < -1.0) Dot = -1.0;
  return acos(Dot)*180.0/M_PI;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::Execute(const MString& Directory, const MString& Program, const MString& Arguments, const MString& LogFile) const
{
  return MProcessManager::Run(Program, Arguments, Directory + "/" + LogFile, Directory, 100) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::IsNonEmptyFile(const MString& FileName) const
{
  error_code Error;
  const uintmax_t Size = filesystem::file_size(FileName.Data(), Error);
  return Error.value() == 0 && Size > 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::WriteInput(const Case& C, const MString& Directory, const MString& Geometry)
{
  // The x-axis: perpendicular to the z-axis, rotated by Roll around it
  MVector Z = ToVector(C.ZLatitude, C.ZLongitude);
  MVector Helper = (fabs(Z.Z()) < 0.9) ? MVector(0, 0, 1) : MVector(1, 0, 0);
  MVector X0 = Z.Cross(Helper).Unit();
  MVector Y0 = Z.Cross(X0);
  const double Roll = C.Roll*M_PI/180.0;
  MVector X = (X0*cos(Roll) + Y0*sin(Roll)).Unit();
  double XLatitude = asin(X.Z())*180.0/M_PI;
  double XLongitude = atan2(X.Y(), X.X())*180.0/M_PI;
  if (XLongitude < 0) XLongitude += 360.0;

  // The orientation: constant pointing, the second line makes sure the time range is covered
  ofstream Ori((Directory + "/" + C.Name + ".ori").Data());
  if (Ori.is_open() == false) return false;
  Ori<<"Type OrientationsGalactic"<<endl;
  for (double Time : { 0.0, 100000.0 }) {
    Ori<<"OG "<<Time<<" "<<fixed<<setprecision(6)<<XLatitude<<" "<<XLongitude<<" "<<C.ZLatitude<<" "<<C.ZLongitude<<endl;
  }
  Ori.close();

  ofstream Source((Directory + "/" + C.Name + ".source").Data());
  if (Source.is_open() == false) return false;
  Source<<"# Point source in Galactic coordinates, fixed detector pointing from the ori file"<<endl;
  Source<<"Version         1"<<endl;
  Source<<"Geometry        "<<Geometry<<endl;
  Source<<endl;
  Source<<"PhysicsListEM                        LivermorePol"<<endl;
  Source<<"PhysicsListEMActivateFluorescence    false"<<endl;
  Source<<endl;
  Source<<"StoreCalibrated                      true"<<endl;
  Source<<"StoreSimulationInfo                  true"<<endl;
  Source<<"StoreSimulationInfoIonization        false"<<endl;
  Source<<"DiscretizeHits                       true"<<endl;
  Source<<endl;
  Source<<"Run MyRun"<<endl;
  Source<<"MyRun.FileName                  "<<C.Name<<endl;
  Source<<"MyRun.NTriggers                 "<<c_NumberOfTriggers<<endl;
  Source<<"MyRun.OrientationSky            Galactic File NoLoop "<<C.Name<<".ori"<<endl;
  Source<<endl;
  Source<<"MyRun.Source MySource"<<endl;
  Source<<"MySource.ParticleType               1"<<endl;
  Source<<"MySource.Beam                       FarFieldPointSource 0 0"<<endl;
  Source<<"MySource.Orientation                Galactic Fixed "<<fixed<<setprecision(2)<<C.SourceLatitude<<" "<<C.SourceLongitude<<endl;
  Source<<"MySource.Spectrum                   Mono 511"<<endl;
  Source<<"MySource.Flux                       0.05"<<endl;
  Source.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::FindPeak(const MString& FileName, double& Longitude, double& Latitude)
{
  TFile File(FileName.Data());
  if (File.IsZombie() == true) {
    merr<<"Unable to open the image file: "<<FileName<<endl;
    return false;
  }

  TIter NextKey(File.GetListOfKeys());
  TKey* Key = static_cast<TKey*>(NextKey());
  if (Key == nullptr) {
    merr<<"The image file contains no objects: "<<FileName<<endl;
    return false;
  }
  TCanvas* Canvas = dynamic_cast<TCanvas*>(Key->ReadObj());
  if (Canvas == nullptr) {
    merr<<"The first object in the image file is no canvas: "<<Key->GetName()<<" ("<<Key->GetClassName()<<")"<<endl;
    return false;
  }

  // The image is a 2D histogram somewhere in the canvas:
  TH2* Image = nullptr;
  TIter NextObject(Canvas->GetListOfPrimitives());
  while (TObject* Object = NextObject()) {
    if (Object->InheritsFrom("TH2") == true) Image = static_cast<TH2*>(Object);
    if (Object->InheritsFrom("TPad") == true) {
      TIter NextSubObject(static_cast<TPad*>(Object)->GetListOfPrimitives());
      while (TObject* SubObject = NextSubObject()) {
        if (SubObject->InheritsFrom("TH2") == true) Image = static_cast<TH2*>(SubObject);
      }
    }
  }
  if (Image == nullptr) {
    merr<<"The canvas contains no 2D histogram"<<endl;
    return false;
  }

  int BinX = 0, BinY = 0, BinZ = 0;
  Image->GetMaximumBin(BinX, BinY, BinZ);
  double MaxX = Image->GetXaxis()->GetBinCenter(BinX);
  double MaxY = Image->GetYaxis()->GetBinCenter(BinY);
  if (Image->GetMaximum() <= 0) {
    merr<<"The image is empty"<<endl;
    return false;
  }

  // Centroid of the bins within 8 degrees around the maximum
  double SumWeights = 0, SumX = 0, SumY = 0;
  for (int i = 1; i <= Image->GetNbinsX(); ++i) {
    for (int j = 1; j <= Image->GetNbinsY(); ++j) {
      double x = Image->GetXaxis()->GetBinCenter(i);
      double y = Image->GetYaxis()->GetBinCenter(j);
      if (fabs(x - MaxX) < 8 && fabs(y - MaxY) < 8) {
        double Weight = Image->GetBinContent(i, j);
        SumWeights += Weight;
        SumX += Weight*x;
        SumY += Weight*y;
      }
    }
  }
  if (SumWeights <= 0) return false;

  // The x-axis of the Mimrec Galactic histogram is stored inverted (the displayed axis is drawn reversed): longitude = -x
  Longitude = -SumX/SumWeights;
  if (Longitude < 0) Longitude += 360.0;
  Latitude = SumY/SumWeights;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::RunCase(const Case& C, const MString& Directory, const MString& Geometry)
{
  if (WriteInput(C, Directory, Geometry) == false) return false;

  // Simulate:
  if (Execute(Directory, "cosima", MString("-v 0 -s ") + C.Seed + " -z " + MSystem::GetShellQuoted(C.Name + ".source"), "cosima.log") == false) return false;

  // Reconstruct, with the default configuration of Revan:
  if (Execute(Directory, "revan", MString("-f ") + MSystem::GetShellQuoted(C.Name + ".inc1.id1.sim.gz") + " -g " + MSystem::GetShellQuoted(Geometry) + " -c ../revan.cfg -a -n", "revan.log") == false) return false;

  // Image in Galactic coordinates with the default configuration of Mimrec, a 2 degree grid, simple backprojection, and a window around 511 keV:
  MString Arguments = MString("-g ") + MSystem::GetShellQuoted(Geometry) + " -f " + MSystem::GetShellQuoted(C.Name + ".inc1.id1.tra.gz") + " -c ../mimrec.cfg -n -i -o " + MSystem::GetShellQuoted(C.Name + ".image.root");
  Arguments += " -C CoordinateSystem.Type=1";
  Arguments += " -C CoordinateSystem.GalacticLatitude.Min=-90 -C CoordinateSystem.GalacticLatitude.Max=90 -C CoordinateSystem.GalacticLatitudeBins=90";
  Arguments += " -C CoordinateSystem.GalacticLongitude.Min=-180 -C CoordinateSystem.GalacticLongitude.Max=180 -C CoordinateSystem.GalacticLongitudeBins=180";
  Arguments += " -C ImageReconstructionAlgorithms.ListMode.NIterations=1";
  Arguments += " -C EventSelections.FirstEnergyWindow.Min=490 -C EventSelections.FirstEnergyWindow.Max=530";
  return Execute(Directory, "mimrec", Arguments, "mimrec.log");
}


////////////////////////////////////////////////////////////////////////////////


bool UTCosimaGalacticImage::Run()
{
  bool Passed = true;

  // The preconditions: the geometry below $MEGALIB and the temporary directory
  const MString GeometryFixture = "resource/examples/geomega/special/Max.geo.setup";
  if (RequireFixture(*this, Passed, GeometryFixture) == false) {
    Summarize();
    return Passed;
  }
  const MString Geometry = MString(getenv("MEGALIB")) + "/" + GeometryFixture;
  MString Directory = GetTemporaryDirectoryName("GalacticImage");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "GalacticImage", "The temporary directory can be created", PrepareTemporaryDirectory("GalacticImage")) && Passed;
  if (Passed == false) {
    Summarize();
    return Passed;
  }

  // The default configuration files of Revan and Mimrec are generated first, since the parallel runs share them
  // (Revan exits with a non-zero status after saving the configuration, thus check for the files)
  Execute(Directory, "revan", MString("-g ") + MSystem::GetShellQuoted(Geometry) + " --save-cfg -o revan.cfg -n", "revan.save.log");
  Execute(Directory, "mimrec", MString("-g ") + MSystem::GetShellQuoted(Geometry) + " --save-cfg -o mimrec.cfg -n", "mimrec.save.log");
  const bool ConfigurationsCreated = IsNonEmptyFile(Directory + "/revan.cfg") == true && IsNonEmptyFile(Directory + "/mimrec.cfg") == true;
  Passed = EvaluateTrue("Execute()", "revan, mimrec --save-cfg", "The programs revan and mimrec are available and can save their default configuration", ConfigurationsCreated) && Passed;
  if (ConfigurationsCreated == false) {
    Summarize();
    return Passed;
  }

  // Two different pointings with different rolls, so that a wrong handedness or a wrong axis definition moves the image:
  vector<Case> Cases = {
    { "Pointing1", 20, 100, 37, 35, 130, 4711 },
    { "Pointing2", -30, 250, -110, -12, 275, 4712 }
  };

  vector<future<bool>> Futures;
  for (unsigned int c = 0; c < Cases.size(); ++c) {
    MString CaseDirectory = Directory + "/" + Cases[c].Name;
    Passed = EvaluateTrue("CreateDirectory()", Cases[c].Name, "The directory of the case can be created", MFile::CreateDirectory(CaseDirectory)) && Passed;
    Futures.push_back(async(launch::async, [this, &Cases, c, CaseDirectory, Geometry]() {
      return RunCase(Cases[c], CaseDirectory, Geometry);
    }));
  }

  for (unsigned int c = 0; c < Cases.size(); ++c) {
    bool Success = Futures[c].get();
    Passed = EvaluateTrue("RunCase()", Cases[c].Name, "Simulation, reconstruction, and imaging work", Success) && Passed;
    if (Success == false) continue;

    // All ROOT access is in the main thread:
    double Longitude = 0, Latitude = 0;
    bool Found = FindPeak(Directory + "/" + Cases[c].Name + "/" + Cases[c].Name + ".image.root", Longitude, Latitude);
    Passed = EvaluateTrue("FindPeak()", Cases[c].Name, "The image has a peak", Found) && Passed;
    if (Found == false) continue;

    double Distance = Separation(Latitude, Longitude, Cases[c].SourceLatitude, Cases[c].SourceLongitude);
    MString Input = Cases[c].Name + ": source (l=" + Cases[c].SourceLongitude + ", b=" + Cases[c].SourceLatitude + "), image peak (l=" + Longitude + ", b=" + Latitude + ")";
    Passed = EvaluateNear("Galactic image", Input, "The image peak is at the true position of the source", Distance, 0.0, c_MaximumSeparation) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  // Initialize ROOT (needed to read the image), without any graphics:
  TApplication ROOT("ROOT", 0, 0);
  gROOT->SetBatch(true);

  UTCosimaGalacticImage Test;
  return Test.Run() == true ? 0 : 1;
}
