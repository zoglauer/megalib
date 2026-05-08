/*
 * UTInterface.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <cerrno>
#include <fstream>
#include <limits>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TF1.h>

// MEGAlib:
#include "MFile.h"
#include "MInterface.h"
#include "MVector.h"
#include "MUnitTest.h"


//! Unit test class for MInterface
class UTInterface : public MUnitTest
{
public:
  UTInterface() : MUnitTest("UTInterface") {}
  virtual ~UTInterface() {}

  virtual bool Run();

private:
  class InterfaceTest : public MInterface
  {
  public:
    InterfaceTest() : MInterface() {}
    virtual ~InterfaceTest() {}

    bool ParseCommandLine(int argc, char** argv) override
    {
      m_LastParseArgc = argc;
      m_LastParseArgv0 = (argc > 0 && argv != nullptr && argv[0] != nullptr) ? argv[0] : "";
      return true;
    }

    bool SetGeometry(MString FileName, bool UpdateGui = true) override
    {
      m_LastGeometryFileName = FileName;
      m_LastGeometryUpdateGui = UpdateGui;
      return FileName.IsEmpty() == false;
    }

    bool LoadConfiguration(MString FileName = g_StringNotDefined) override
    {
      m_LastLoadConfigurationFileName = FileName;
      if (FileName == g_StringNotDefined || MFile::Exists(FileName) == false) {
        return false;
      }

      ifstream In(FileName.Data());
      if (In.is_open() == false) {
        return false;
      }

      string Content;
      getline(In, Content);
      m_LoadedContent = Content.c_str();
      return true;
    }

    bool SaveConfiguration(MString FileName = g_StringNotDefined) override
    {
      m_LastSaveConfigurationFileName = FileName;
      if (FileName == g_StringNotDefined) {
        return false;
      }

      ofstream Out(FileName.Data());
      if (Out.is_open() == false) {
        return false;
      }

      Out << "UTInterface";
      Out.close();
      return true;
    }

    void SetUseGuiForTest(bool UseGui) { m_UseGui = UseGui; }

    int m_LastParseArgc = -1;
    MString m_LastParseArgv0;
    MString m_LastGeometryFileName;
    bool m_LastGeometryUpdateGui = true;
    MString m_LastLoadConfigurationFileName;
    MString m_LastSaveConfigurationFileName;
    MString m_LoadedContent;
  };

  static MString CreateTempDirectory(const char* Prefix)
  {
    char Template[256];
    snprintf(Template, sizeof(Template), "/tmp/%s_%ld_XXXXXX", Prefix, static_cast<long>(getpid()));
    char* Directory = mkdtemp(Template);
    if (Directory == nullptr) {
      return "";
    }
    return Directory;
  }
};


////////////////////////////////////////////////////////////////////////////////


bool UTInterface::Run()
{
  bool Passed = true;

  InterfaceTest Interface;
  Passed = EvaluateTrue("MInterface()", "default use ui", "A default MInterface-derived object starts with GUI use enabled", Interface.UseUI()) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "default geometry", "A default MInterface-derived object starts without a geometry", Interface.GetGeometry() == nullptr) && Passed;

  Interface.SetUseGuiForTest(false);
  Passed = EvaluateFalse("UseUI()", "toggle", "The derived helper can disable GUI usage for representative interface tests", Interface.UseUI()) && Passed;

  Passed = EvaluateTrue("ParseCommandLine()", "zero args", "ParseCommandLine accepts a representative empty argument list", Interface.ParseCommandLine(0, nullptr)) && Passed;
  Passed = Evaluate("ParseCommandLine()", "zero args count", "ParseCommandLine stores the representative empty argc value", Interface.m_LastParseArgc, 0) && Passed;

  char Arg0[] = "bin/UTInterface";
  char Arg1[] = "--example";
  char* Argv[] = { Arg0, Arg1, nullptr };
  Passed = EvaluateTrue("ParseCommandLine()", "two args", "ParseCommandLine accepts a representative command line", Interface.ParseCommandLine(2, Argv)) && Passed;
  Passed = Evaluate("ParseCommandLine()", "argv[0]", "ParseCommandLine stores the representative executable name", Interface.m_LastParseArgv0, MString("bin/UTInterface")) && Passed;

  MString DummyGeometry = "$(MEGALIB)/resource/examples/geomega/special/Dummy.geo.setup";
  Passed = EvaluateTrue("MFile::Exists()", "dummy geometry", "The representative dummy geometry file exists", MFile::Exists(DummyGeometry)) && Passed;
  MFile::ExpandFileName(DummyGeometry);

  Passed = EvaluateTrue("SetGeometry()", "first call", "SetGeometry accepts the representative dummy geometry file", Interface.SetGeometry(DummyGeometry)) && Passed;
  Passed = Evaluate("SetGeometry()", "file name", "SetGeometry stores the representative dummy geometry file name", Interface.m_LastGeometryFileName, DummyGeometry) && Passed;
  Passed = Evaluate("SetGeometry()", "update gui flag", "The representative SetGeometry call keeps the default GUI-update flag enabled", Interface.m_LastGeometryUpdateGui, true) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "after first SetGeometry", "GetGeometry still returns no geometry for the dummy interface", Interface.GetGeometry() == nullptr) && Passed;

  Passed = EvaluateTrue("SetGeometry()", "second call", "SetGeometry can replace an existing representative geometry object", Interface.SetGeometry(DummyGeometry, false)) && Passed;
  Passed = Evaluate("SetGeometry()", "second file name", "SetGeometry stores the second representative geometry file name", Interface.m_LastGeometryFileName, DummyGeometry) && Passed;
  Passed = Evaluate("SetGeometry()", "second update gui flag", "SetGeometry accepts a representative false update-gui flag", Interface.m_LastGeometryUpdateGui, false) && Passed;
  Passed = EvaluateTrue("GetGeometry()", "after second SetGeometry", "GetGeometry still returns no geometry for the dummy interface after repeated calls", Interface.GetGeometry() == nullptr) && Passed;

  const MString TempDirectory = CreateTempDirectory("UTInterface");
  Passed = EvaluateTrue("CreateTempDirectory()", "directory", "A temporary directory for MInterface tests can be created", TempDirectory.IsEmpty() == false) && Passed;
  if (TempDirectory.IsEmpty() == true) {
    Summarize();
    return false;
  }

  const MString SaveFile = TempDirectory + "/interface.cfg";
  const MString MissingFile = TempDirectory + "/missing.cfg";

  Passed = EvaluateTrue("SaveConfiguration()", "file write", "SaveConfiguration writes a representative configuration file", Interface.SaveConfiguration(SaveFile)) && Passed;
  Passed = EvaluateTrue("MFile::Exists()", "file write", "SaveConfiguration creates the representative configuration file", MFile::Exists(SaveFile)) && Passed;
  Passed = Evaluate("SaveConfiguration()", "file name", "SaveConfiguration stores the representative file name", Interface.m_LastSaveConfigurationFileName, MString(SaveFile)) && Passed;
  Passed = EvaluateFalse("SaveConfiguration()", "sentinel", "SaveConfiguration rejects the representative undefined file sentinel", Interface.SaveConfiguration(g_StringNotDefined)) && Passed;

  Interface.m_LoadedContent = "";
  Passed = EvaluateTrue("LoadConfiguration()", "file read", "LoadConfiguration reads a representative configuration file", Interface.LoadConfiguration(SaveFile)) && Passed;
  Passed = Evaluate("LoadConfiguration()", "file name", "LoadConfiguration stores the representative file name", Interface.m_LastLoadConfigurationFileName, MString(SaveFile)) && Passed;
  Passed = Evaluate("LoadConfiguration()", "file content", "LoadConfiguration preserves the representative configuration payload", Interface.m_LoadedContent, MString("UTInterface")) && Passed;
  Passed = EvaluateFalse("LoadConfiguration()", "sentinel", "LoadConfiguration rejects the representative undefined file sentinel", Interface.LoadConfiguration(g_StringNotDefined)) && Passed;

  Passed = EvaluateFalse("LoadConfiguration()", "missing file", "LoadConfiguration rejects a missing representative configuration file", Interface.LoadConfiguration(MissingFile)) && Passed;

  {
    DisableDefaultStreams();
    vector<MVector> EmptyPositions;
    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;
    double zMin = 0;
    double zMax = 0;
    Passed = EvaluateFalse("DetermineAxis()", "empty", "DetermineAxis rejects an empty position list", MInterface::DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, EmptyPositions)) && Passed;
    EnableDefaultStreams();
  }

  {
    vector<MVector> Positions;
    Positions.push_back(MVector(-1.0, -1.0, -1.0));
    Positions.push_back(MVector(1.0, 1.0, 1.0));
    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;
    double zMin = 0;
    double zMax = 0;
    Passed = EvaluateTrue("DetermineAxis()", "representative", "DetermineAxis succeeds for a representative symmetric point cloud", MInterface::DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions)) && Passed;
    Passed = Evaluate("DetermineAxis()", "x min", "DetermineAxis computes the representative x minimum", xMin, -1.2) && Passed;
    Passed = Evaluate("DetermineAxis()", "x max", "DetermineAxis computes the representative x maximum", xMax, 1.2) && Passed;
    Passed = Evaluate("DetermineAxis()", "y min", "DetermineAxis computes the representative y minimum", yMin, -1.2) && Passed;
    Passed = Evaluate("DetermineAxis()", "y max", "DetermineAxis computes the representative y maximum", yMax, 1.2) && Passed;
    Passed = Evaluate("DetermineAxis()", "z min", "DetermineAxis computes the representative z minimum", zMin, -1.2) && Passed;
    Passed = Evaluate("DetermineAxis()", "z max", "DetermineAxis computes the representative z maximum", zMax, 1.2) && Passed;
  }

  {
    DisableDefaultStreams();
    vector<MVector> Positions;
    Positions.push_back(MVector(0.0, 0.0, 0.0));
    Positions.push_back(MVector(numeric_limits<double>::quiet_NaN(), 1.0, 2.0));
    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;
    double zMin = 0;
    double zMax = 0;
    Passed = EvaluateFalse("DetermineAxis()", "non-finite", "DetermineAxis rejects a position list containing a non-finite coordinate", MInterface::DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions)) && Passed;
    EnableDefaultStreams();
  }

  {
    vector<MVector> Positions;
    Positions.push_back(MVector(1.0, 1.0, 1.0));
    double xMin = 0;
    double xMax = 0;
    double yMin = 0;
    double yMax = 0;
    double zMin = 0;
    double zMax = 0;
    Passed = EvaluateTrue("DetermineAxis()", "single point", "DetermineAxis succeeds for a single repeated position", MInterface::DetermineAxis(xMin, xMax, yMin, yMax, zMin, zMax, Positions)) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single x min", "DetermineAxis expands the single-point x minimum", xMin, 0.99999, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single x max", "DetermineAxis expands the single-point x maximum", xMax, 1.00001, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single y min", "DetermineAxis expands the single-point y minimum", yMin, 0.99999, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single y max", "DetermineAxis expands the single-point y maximum", yMax, 1.00001, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single z min", "DetermineAxis expands the single-point z minimum", zMin, 0.99999, 1e-12) && Passed;
    Passed = EvaluateNear("DetermineAxis()", "single z max", "DetermineAxis expands the single-point z maximum", zMax, 1.00001, 1e-12) && Passed;
  }

  {
    double* LinearBins = MInterface::CreateAxisBins(0.0, 4.0, 2, false);
    Passed = EvaluateTrue("CreateAxisBins()", "linear", "CreateAxisBins returns a representative linear binning", LinearBins != nullptr) && Passed;
    if (LinearBins != nullptr) {
      Passed = Evaluate("CreateAxisBins()", "linear edge 0", "CreateAxisBins computes the representative first linear edge", LinearBins[0], 0.0) && Passed;
      Passed = Evaluate("CreateAxisBins()", "linear edge 1", "CreateAxisBins computes the representative middle linear edge", LinearBins[1], 2.0) && Passed;
      Passed = Evaluate("CreateAxisBins()", "linear edge 2", "CreateAxisBins computes the representative last linear edge", LinearBins[2], 4.0) && Passed;
      delete [] LinearBins;
    }
  }

  {
    double* LogBins = MInterface::CreateAxisBins(1.0, exp(2.0), 2, true);
    Passed = EvaluateTrue("CreateAxisBins()", "log", "CreateAxisBins returns a representative logarithmic binning", LogBins != nullptr) && Passed;
    if (LogBins != nullptr) {
      Passed = EvaluateNear("CreateAxisBins()", "log edge 0", "CreateAxisBins computes the representative first logarithmic edge", LogBins[0], 1.0, 1e-12) && Passed;
      Passed = EvaluateNear("CreateAxisBins()", "log edge 1", "CreateAxisBins computes the representative middle logarithmic edge", LogBins[1], exp(1.0), 1e-12) && Passed;
      Passed = EvaluateNear("CreateAxisBins()", "log edge 2", "CreateAxisBins computes the representative last logarithmic edge", LogBins[2], exp(2.0), 1e-12) && Passed;
      delete [] LogBins;
    }
  }

  {
    double* LogClampBins = MInterface::CreateAxisBins(0.0, exp(2.0), 2, true);
    Passed = EvaluateTrue("CreateAxisBins()", "log clamp", "CreateAxisBins clamps non-positive logarithmic minima", LogClampBins != nullptr) && Passed;
    if (LogClampBins != nullptr) {
      Passed = EvaluateNear("CreateAxisBins()", "log clamp edge 0", "CreateAxisBins clamps the first logarithmic edge to a small positive value", LogClampBins[0], 1E-9, 1e-15) && Passed;
      delete [] LogClampBins;
    }
  }

  {
    double* FixedBins = MInterface::CreateAxisBins(5.0, 5.0, 0, false);
    Passed = EvaluateTrue("CreateAxisBins()", "edge case", "CreateAxisBins still returns a representative array for degenerate input", FixedBins != nullptr) && Passed;
    if (FixedBins != nullptr) {
      Passed = Evaluate("CreateAxisBins()", "edge case 0", "CreateAxisBins adjusts the representative lower edge for degenerate input", FixedBins[0], 5.0) && Passed;
      Passed = Evaluate("CreateAxisBins()", "edge case 1", "CreateAxisBins adjusts the representative upper edge for degenerate input", FixedBins[1], 6.0) && Passed;
      delete [] FixedBins;
    }
  }

  {
    TF1 Gaussian("Gaussian", "[0]*exp(-0.5*((x-[1])/[2])*((x-[1])/[2]))", -20.0, 20.0);
    Gaussian.SetParameters(1.0, 0.0, 2.0);
    const double ExpectedFWHM = 2.0*sqrt(2.0*log(2.0))*2.0;
    Passed = EvaluateNear("GetFWHM()", "gaussian default bounds", "GetFWHM uses the fallback range when no bounds are given", MInterface::GetFWHM(&Gaussian), ExpectedFWHM, 1e-2) && Passed;
    Passed = EvaluateNear("GetFWHM()", "gaussian", "GetFWHM returns the representative Gaussian full width at half maximum", MInterface::GetFWHM(&Gaussian, -20.0, 20.0), ExpectedFWHM, 1e-2) && Passed;
    Passed = Evaluate("GetFWHM()", "null function", "GetFWHM rejects a null function pointer", MInterface::GetFWHM(nullptr, -20.0, 20.0), numeric_limits<double>::max()) && Passed;
  }

  {
    DisableDefaultStreams();
    TF1 NegativeConstant("NegativeConstant", "[0]", -1.0, 1.0);
    NegativeConstant.SetParameter(0, -2.0);
    Passed = Evaluate("GetFWHM()", "negative constant", "GetFWHM returns the fallback width for an all-negative function", MInterface::GetFWHM(&NegativeConstant, -1.0, 1.0), numeric_limits<double>::max()) && Passed;
    EnableDefaultStreams();
  }

  {
    DisableDefaultStreams();
    TF1 ZeroWidth("ZeroWidth", "[0]", 1.0, 1.0);
    ZeroWidth.SetParameter(0, 2.0);
    Passed = Evaluate("GetFWHM()", "zero width", "GetFWHM rejects zero-width function ranges", MInterface::GetFWHM(&ZeroWidth, 0.0, 0.0), numeric_limits<double>::max()) && Passed;
    EnableDefaultStreams();
  }

  {
    DisableDefaultStreams();
    TF1 Gaussian("GaussianInvertedBounds", "[0]*exp(-0.5*((x-[1])/[2])*((x-[1])/[2]))", -20.0, 20.0);
    Gaussian.SetParameters(1.0, 0.0, 2.0);
    Passed = Evaluate("GetFWHM()", "inverted bounds", "GetFWHM rejects inverted integration bounds", MInterface::GetFWHM(&Gaussian, 10.0, -10.0), numeric_limits<double>::max()) && Passed;
    EnableDefaultStreams();
  }

  Passed = EvaluateTrue("MFile::Remove()", "save cleanup", "The representative configuration file can be removed", MFile::Remove(SaveFile)) && Passed;
  Passed = EvaluateFalse("MFile::Exists()", "save cleanup", "The representative configuration file is gone after cleanup", MFile::Exists(SaveFile)) && Passed;
  Passed = EvaluateTrue("rmdir()", "temp cleanup", "The temporary MInterface directory can be removed", rmdir(TempDirectory.Data()) == 0) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTInterface Test;
  return Test.Run() == true ? 0 : 1;
}
