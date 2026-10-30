// Integration test: needs a display (or xvfb-run), built MEGAlib and cad_converter.
#include "MGUICADConverter.h"
#include "MGUIGeomegaMain.h"
#include "MInterfaceGeomega.h"
#include "MDGeometry.h"
#include "MGlobal.h"
#include <TApplication.h>
#include <TGButton.h>
#include <TGClient.h>
#include <TGNumberEntry.h>
#include <TImage.h>
#include <TSystem.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <memory>

class TestInterface : public MInterfaceGeomega
{
public:
  MSettingsGeomega* Settings() { return m_Data; }
};

void Require(bool Condition, const char* Message)
{
  if (!Condition) throw std::runtime_error(Message);
}

TGTextButton* Button(TGCompositeFrame* Frame, const char* Label)
{
  TIter Next(Frame->GetList());
  while (auto Element = static_cast<TGFrameElement*>(Next())) {
    if (auto B = dynamic_cast<TGTextButton*>(Element->fFrame)) {
      if (std::string(B->GetText()->GetString()) == Label) return B;
    }
    if (auto Child = dynamic_cast<TGCompositeFrame*>(Element->fFrame)) {
      if (auto B = Button(Child, Label)) return B;
    }
  }
  return nullptr;
}

void NumberEntries(TGCompositeFrame* Frame, std::vector<TGNumberEntry*>& Entries)
{
  TIter Next(Frame->GetList());
  while (auto Element = static_cast<TGFrameElement*>(Next())) {
    if (auto Entry = dynamic_cast<TGNumberEntry*>(Element->fFrame)) Entries.push_back(Entry);
    else if (auto Child = dynamic_cast<TGCompositeFrame*>(Element->fFrame)) NumberEntries(Child, Entries);
  }
}

void Wait(MGUICADConverter& Dialog)
{
  const auto Start = std::chrono::steady_clock::now();
  while (Dialog.IsConverting()) {
    gSystem->ProcessEvents();
    gSystem->Sleep(10);
    Require(std::chrono::steady_clock::now() - Start < std::chrono::seconds(30), "conversion timed out");
  }
}

int main(int argc, char** argv)
{
  if (argc != 3) { std::cerr << "Usage: cad_converter_gui DATA_DIR TEMP_DIR\n"; return 2; }
  const TString Data = argv[1], Temp = argv[2];
  MGlobal::Initialize("CAD GUI integration test");
  // Match MGeomegaMain's application lifetime. ROOT's global GUI teardown
  // cannot safely destroy Geomega's legacy popup menus a second time.
  new TApplication("cad-test", nullptr, nullptr);
  try {
    TestInterface Interface;
    MGUIGeomegaMain Main(&Interface, Interface.Settings());
    Main.Create();
    Main.SetCleanup(kDeepCleanup);
    MGUICADConverter Dialog(gClient->GetRoot(), &Main);
    if (const char* Screenshot = gSystem->Getenv("CAD_CONVERTER_GUI_SCREENSHOT")) {
      for (int i = 0; i < 20; ++i) { gSystem->ProcessEvents(); gSystem->Sleep(10); }
      std::unique_ptr<TImage> Image(TImage::Create());
      Image->FromWindow(Dialog.GetId());
      Image->WriteImage(Screenshot);
    }
    auto View = Button(&Dialog, "View in Geomega");
    auto Geo = dynamic_cast<TGCheckButton*>(Button(&Dialog, "Geomega (.geo.setup + meshes)"));
    auto Obj = dynamic_cast<TGCheckButton*>(Button(&Dialog, "OBJ"));
    auto Stl = dynamic_cast<TGCheckButton*>(Button(&Dialog, "STL"));
    Require(View && Geo && Obj && Stl, "missing controls");
    Require(!View->IsEnabled(), "view enabled before conversion");
    Require(!Dialog.SetInputFile(Temp + "/absent.step"), "accepted missing input");
    for (const char* Model : {"RectangularPrismTest", "AssemblyTest", "CylinderTest", "CollisionTest",
                              "ConeTest", "FilletRectTest", "PartialRevolve", "ShellTest", "SphereTest", "TangentTest"}) {
      Require(Dialog.SetInputFile(Data + "/" + Model + ".step"), "input rejected");
      Require(Dialog.SetOutputDirectory(Temp + "/" + Model), "destination rejected");
      Dialog.OnConvert();
      Require(Dialog.IsConverting(), "converter did not start");
      Require(!Dialog.SetInputFile(Data + "/ConeTest.step"), "input changed during conversion");
      Dialog.OnConvert(); // double-click must not spawn a second process
      Wait(Dialog);
      Require(View->IsEnabled(), "view disabled after successful conversion");
      MDGeometry Geometry;
      Require(Geometry.ScanSetupFile(Dialog.GetOutputFilePath().Data(), false, false, false), "Geomega rejected converted setup");
      Require(Geometry.GetNVolumes() > 1, "no converted solids");
    }
    // Literal paths, uppercase extension, and a dotted model stem.
    const TString Input = Temp + "/my model's.v2.STEP";
    Require(gSystem->CopyFile(Data + "/AssemblyTest.step", Input, true) == 0, "copy failed");
    Require(Dialog.SetInputFile(Input), "uppercase STEP rejected");
    Require(Dialog.SetOutputDirectory(Temp + "/output with spaces"), "destination rejected");
    Geo->SetOn(false); // no formats selected must still generate Geomega
    Dialog.OnConvert();
    Wait(Dialog);
    Require(View->IsEnabled(), "default Geomega conversion failed");
    Require(Dialog.GetOutputFilePath().EndsWith("my model's.v2.geo.setup"), "incorrect output filename");
    {
      MDGeometry Geometry;
      Require(Geometry.ScanSetupFile(Dialog.GetOutputFilePath().Data(), false, false, false), "Geomega rejected path containing spaces");
    }
    Dialog.OnViewGeomega();
    Require(Interface.Settings()->GetCurrentFileName() == Dialog.GetOutputFilePath().Data(), "viewer did not select converted setup");
    Require(Interface.GetGeometry()->IsScanned(), "viewer could not load converted setup");
    Require(gSystem->AccessPathName(Temp + "/output with spaces/cross-sections"), "preview generated simulation cross sections");
    gSystem->ProcessEvents();

    // Exercise the numeric controls through an observable change in geometry.
    std::vector<TGNumberEntry*> Entries;
    NumberEntries(&Dialog, Entries);
    Require(Entries.size() == 3, "missing numeric controls");
    Entries[0]->SetNumber(45);
    Entries[1]->SetNumber(0.1);
    Entries[2]->SetNumber(20000); // removes AssemblyTest's smaller solid
    Dialog.OnConvert(); Wait(Dialog);
    Require(View->IsEnabled(), "conversion with numeric settings failed");
    {
      MDGeometry Filtered;
      Require(Filtered.ScanSetupFile(Dialog.GetOutputFilePath().Data(), false, false, false), "filtered setup rejected");
      Require(Filtered.GetNVolumes() == 2, "minimum volume setting did not remove the small solid");
    }
    Entries[2]->SetNumber(0);

    Geo->SetOn(false); Obj->SetOn(true); Stl->SetOn(true);
    Dialog.OnConvert(); Wait(Dialog);
    Require(!View->IsEnabled() && Dialog.GetOutputFilePath().IsNull(), "mesh-only run enabled view");
    Require(!gSystem->AccessPathName(Temp + "/output with spaces/my model's.v2.obj"), "OBJ missing");
    Require(!gSystem->AccessPathName(Temp + "/output with spaces/my model's.v2.stl"), "STL missing");
    Geo->SetOn(true);

    // Failure must invalidate the previous successful geometry.
    const TString Invalid = Temp + "/invalid.step";
    { std::ofstream File(Invalid.Data()); File << "not STEP\n"; }
    Require(Dialog.SetInputFile(Invalid), "invalid STEP path rejected before conversion");
    Dialog.OnConvert(); Wait(Dialog);
    Require(!View->IsEnabled() && Dialog.GetOutputFilePath().IsNull(), "failed conversion enabled view");

    const TString Slow = Temp + "/slow-converter";
    { std::ofstream File(Slow.Data()); File << "#!/bin/sh\nexec sleep 30\n"; }
    chmod(Slow.Data(), 0700);
    gSystem->Setenv("MEGALIB_CAD_CONVERTER", Slow);
    Dialog.OnConvert();
    Require(Dialog.IsConverting(), "cancellation test did not start");
    Dialog.OnCancel(); Wait(Dialog);
    Require(!View->IsEnabled() && Dialog.GetOutputFilePath().IsNull(), "cancelled conversion enabled view");
    Dialog.OnConvert();
    Require(Dialog.IsConverting(), "close test did not start");
    Dialog.CloseWindow(); Wait(Dialog);
    Require(!View->IsEnabled(), "closing a conversion enabled view");
    Dialog.MapWindow(); // the main window reuses the dialog after closing it
    gSystem->Setenv("MEGALIB_CAD_CONVERTER", "/nonexistent/cad_converter");
    Dialog.OnConvert();
    Require(!Dialog.IsConverting(), "missing executable started");
    Require(!View->IsEnabled(), "missing executable enabled view");
    gSystem->Unsetenv("MEGALIB_CAD_CONVERTER");
    std::cout << "CAD GUI integration checks passed\n";
  } catch (const std::exception& Error) {
    std::cerr << "FAIL: " << Error.what() << '\n';
    return 1;
  }
  return 0;
}
