/*
 * UTFunction3DSpherical.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:
#include <TCanvas.h>
#include <TROOT.h>
#include <TRandom.h>

// MEGAlib:
#include "MExceptions.h"
#include "MFunction3DSpherical.h"
#include "MUnitTest.h"


//! Unit test class for MFunction3DSpherical
class UTFunction3DSpherical : public MUnitTest
{
public:
  UTFunction3DSpherical() : MUnitTest("UTFunction3DSpherical") {}
  virtual ~UTFunction3DSpherical() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFunction3DSpherical::Run()
{
  bool Passed = true;

  system("mkdir -p /tmp/UTFunction3DSpherical");

  MFunction3DSpherical Default;
  Passed = Evaluate("MFunction3DSpherical()", "construction", "A representative MFunction3DSpherical instance can be constructed", true, true) && Passed;
  Passed = EvaluateException<MExceptionEmptyObject>("GetRandom()", "default constructor", "GetRandom throws for a representative empty spherical function", [&]() {
    double X = 0.0;
    double Y = 0.0;
    double Z = 0.0;
    Default.GetRandom(X, Y, Z);
  }) && Passed;

  {
    vector<double> Phi{0.0, 90.0, 180.0};
    vector<double> Theta{0.0, 60.0, 120.0, 180.0};
    vector<double> Energy{1.0, 4.0};
    vector<double> Values(Phi.size()*Theta.size()*Energy.size(), 2.0);

    MFunction3DSpherical Constant;
    Passed = Evaluate("Set()", "representative spherical vectors", "MFunction3DSpherical accepts representative spherical vector data", Constant.Set(Phi, Theta, Energy, Values), true) && Passed;
    MFunction3DSpherical Copied(Constant);
    Passed = EvaluateNear("MFunction3DSpherical(const MFunction3DSpherical&)", "representative spherical copy constructor", "The representative spherical copy constructor preserves integration behavior", Copied.Integrate(), 12.0*c_Pi, 1e-10) && Passed;
    MFunction3DSpherical Assigned;
    Assigned = Constant;
    Passed = EvaluateNear("operator=()", "representative spherical assignment", "The representative spherical assignment operator preserves integration behavior", Assigned.Integrate(), 12.0*c_Pi, 1e-10) && Passed;
    Passed = EvaluateNear("Integrate()", "representative spherical constant field", "Integrate returns the representative half-sphere constant-field volume exactly", Constant.Integrate(), 12.0*c_Pi, 1e-10) && Passed;

    gRandom->SetSeed(61);
    double X1 = 0.0;
    double Y1 = 0.0;
    double Z1 = 0.0;
    double X2 = 0.0;
    double Y2 = 0.0;
    double Z2 = 0.0;
    double X3 = 0.0;
    double Y3 = 0.0;
    double Z3 = 0.0;
    Constant.GetRandom(X1, Y1, Z1);
    Constant.GetRandom(X2, Y2, Z2);
    Constant.GetRandom(X3, Y3, Z3);
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 1 phi", "GetRandom returns the representative first seeded golden phi value", X1, 77.618846744783223, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 1 theta", "GetRandom returns the representative first seeded golden theta value", Y1, 126.20521153593072, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 1 energy", "GetRandom returns the representative first seeded golden energy value", Z1, 1.2075466869436204, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 2 phi", "GetRandom returns the representative second seeded golden phi value", X2, 134.95992042124236, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 2 theta", "GetRandom returns the representative second seeded golden theta value", Y2, 89.51270219361162, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 2 energy", "GetRandom returns the representative second seeded golden energy value", Z2, 3.8947845308464013, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 3 phi", "GetRandom returns the representative third seeded golden phi value", X3, 21.358841852168952, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 3 theta", "GetRandom returns the representative third seeded golden theta value", Y3, 57.440279976195759, 1e-3) && Passed;
    Passed = EvaluateNear("GetRandom()", "representative spherical draw 3 energy", "GetRandom returns the representative third seeded golden energy value", Z3, 3.057554100246204, 1e-3) && Passed;
  }

  {
    vector<double> Phi{0.0, 90.0, 180.0};
    vector<double> Theta{0.0, 60.0, 120.0, 180.0};
    vector<double> Energy{1.0, 4.0};
    vector<double> Values(Phi.size()*Theta.size()*Energy.size(), 0.0);

    MFunction3DSpherical ZeroTotal;
    Passed = Evaluate("Set()", "representative zero-total spherical vectors", "MFunction3DSpherical accepts representative zero-total spherical vector data", ZeroTotal.Set(Phi, Theta, Energy, Values), true) && Passed;
    Passed = EvaluateException<MExceptionInvalidState>("GetRandom()", "zero-total spherical function", "GetRandom throws for a representative spherical function with zero total content", [&]() {
      double X = 0.0;
      double Y = 0.0;
      double Z = 0.0;
      ZeroTotal.GetRandom(X, Y, Z);
    }) && Passed;
  }

  {
    vector<double> Phi{0.0, 180.0};
    vector<double> Theta{0.0, 90.0, 180.0};
    vector<double> Energy{1.0, 3.0};
    vector<double> Values{
      1.0, 2.0,
      3.0, 4.0,
      5.0, 6.0,
      7.0, 8.0,
      9.0, 10.0,
      11.0, 12.0
    };

    MFunction3DSpherical RoundTripSource;
    Passed = Evaluate("Set()", "representative spherical round-trip source", "MFunction3DSpherical accepts representative source data for a file round-trip", RoundTripSource.Set(Phi, Theta, Energy, Values), true) && Passed;
    MString FileName = "/tmp/UTFunction3DSpherical/roundtrip.fun";
    Passed = Evaluate("Save()", "representative spherical round-trip file", "Save writes a representative spherical function file", RoundTripSource.Save(FileName, "AP"), true) && Passed;

    MFunction3DSpherical RoundTripRead;
    Passed = Evaluate("Set()", "representative spherical round-trip file", "Set reads back a representative spherical function file", RoundTripRead.Set(FileName, "AP"), true) && Passed;
    Passed = EvaluateNear("Evaluate()", "representative spherical round-trip interior", "Set preserves a representative spherical interior interpolation after a file round-trip", RoundTripRead.Evaluate(90.0, 45.0, 2.0), 5.5, 1e-10) && Passed;

    MFunction3DSpherical RoundTripCopied(RoundTripRead);
    Passed = EvaluateNear("MFunction3DSpherical(const MFunction3DSpherical&)", "file-loaded non-equidistant source", "The spherical copy constructor preserves representative file-loaded interpolation state", RoundTripCopied.Evaluate(90.0, 45.0, 2.0), 5.5, 1e-10) && Passed;
    MFunction3DSpherical RoundTripAssigned;
    Passed = Evaluate("Set()", "equidistant spherical assignment target", "MFunction3DSpherical accepts a representative equidistant source before assignment checks",
                      RoundTripAssigned.Set(vector<double>{0.0, 90.0, 180.0}, vector<double>{0.0, 90.0, 180.0}, vector<double>{1.0, 2.0}, vector<double>(18, 1.0)), true) && Passed;
    RoundTripAssigned = RoundTripRead;
    Passed = EvaluateNear("operator=()", "file-loaded non-equidistant source", "The spherical assignment operator preserves representative file-loaded interpolation state", RoundTripAssigned.Evaluate(90.0, 45.0, 2.0), 5.5, 1e-10) && Passed;
  }

  {
    MFunction3DSpherical SaveFailure;
    Passed = Evaluate("Set()", "representative spherical save-failure source", "MFunction3DSpherical accepts representative source data for save failure checks",
                      SaveFailure.Set(vector<double>{0.0, 180.0}, vector<double>{0.0, 90.0, 180.0}, vector<double>{1.0, 3.0}, vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0}), true) && Passed;
    DisableDefaultStreams();
    Passed = Evaluate("Save()", "unwritable representative spherical target", "Save returns false for a representative unwritable spherical target path", SaveFailure.Save("/tmp/UTFunction3DSpherical_missing_directory/out.fun"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString InvalidThetaFile = "/tmp/UTFunction3DSpherical/invalid_theta.fun";
    ofstream Out(InvalidThetaFile.Data());
    Out<<"IP LIN\n";
    Out<<"PA 0 180\n";
    Out<<"TA -10 90 180\n";
    Out<<"EA 1 2\n";
    Out<<"AP 0 0 0 1\n";
    Out<<"AP 1 0 0 1\n";
    Out<<"AP 0 1 0 1\n";
    Out<<"AP 1 1 0 1\n";
    Out<<"AP 0 2 0 1\n";
    Out<<"AP 1 2 0 1\n";
    Out<<"AP 0 0 1 1\n";
    Out<<"AP 1 0 1 1\n";
    Out<<"AP 0 1 1 1\n";
    Out<<"AP 1 1 1 1\n";
    Out<<"AP 0 2 1 1\n";
    Out<<"AP 1 2 1 1\n";
    Out.close();

    MFunction3DSpherical InvalidTheta;
    Passed = Evaluate("Set()", "invalid theta axis", "MFunction3DSpherical rejects a representative theta axis outside the physical range", InvalidTheta.Set(InvalidThetaFile, "AP"), false) && Passed;
  }

  {
    MString InvalidIndexFile = "/tmp/UTFunction3DSpherical/invalid_index.fun";
    ofstream Out(InvalidIndexFile.Data());
    Out<<"IP LIN\n";
    Out<<"PA 0 180\n";
    Out<<"TA 0 90 180\n";
    Out<<"EA 1 2\n";
    Out<<"AP 2 0 0 1\n";
    Out.close();

    MFunction3DSpherical InvalidIndex;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "out-of-range spherical file index", "MFunction3DSpherical rejects a representative file index equal to the phi-axis size", InvalidIndex.Set(InvalidIndexFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MString CompleteFile = "/tmp/UTFunction3DSpherical/complete.fun";
    ofstream OutComplete(CompleteFile.Data());
    OutComplete<<"IP LIN\n";
    OutComplete<<"PA 0 180\n";
    OutComplete<<"TA 0 90 180\n";
    OutComplete<<"EA 1 2\n";
    OutComplete<<"AP 0 0 0 1\n";
    OutComplete<<"AP 1 0 0 2\n";
    OutComplete<<"AP 0 1 0 3\n";
    OutComplete<<"AP 1 1 0 4\n";
    OutComplete<<"AP 0 2 0 5\n";
    OutComplete<<"AP 1 2 0 6\n";
    OutComplete<<"AP 0 0 1 7\n";
    OutComplete<<"AP 1 0 1 8\n";
    OutComplete<<"AP 0 1 1 9\n";
    OutComplete<<"AP 1 1 1 10\n";
    OutComplete<<"AP 0 2 1 11\n";
    OutComplete<<"AP 1 2 1 12\n";
    OutComplete.close();

    MString MissingEAFile = "/tmp/UTFunction3DSpherical/missing_ea.fun";
    ofstream OutMissing(MissingEAFile.Data());
    OutMissing<<"IP LIN\n";
    OutMissing<<"PA 0 180\n";
    OutMissing<<"TA 0 90 180\n";
    OutMissing<<"AP 0 0 0 1\n";
    OutMissing<<"AP 1 0 0 2\n";
    OutMissing<<"AP 0 1 0 3\n";
    OutMissing<<"AP 1 1 0 4\n";
    OutMissing<<"AP 0 2 0 5\n";
    OutMissing<<"AP 1 2 0 6\n";
    OutMissing.close();

    MFunction3DSpherical ReRead;
    Passed = Evaluate("Set()", "representative complete spherical file before reread", "MFunction3DSpherical accepts a representative complete file before reread failure checks", ReRead.Set(CompleteFile, "AP"), true) && Passed;
    DisableDefaultStreams();
    Passed = Evaluate("Set()", "representative missing EA on reread", "MFunction3DSpherical rejects a representative reread file that is missing the energy axis instead of reusing stale state", ReRead.Set(MissingEAFile, "AP"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MFunction3DSpherical Plotted;
    Passed = Evaluate("Set()", "representative spherical plot source", "MFunction3DSpherical accepts representative source data for plot checks",
                      Plotted.Set(vector<double>{0.0, 180.0}, vector<double>{0.0, 90.0, 180.0}, vector<double>{1.0, 3.0}, vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0}), true) && Passed;
    Bool_t WasBatch = gROOT->IsBatch();
    int Before = gROOT->GetListOfCanvases()->GetSize();
    gROOT->SetBatch(true);
    Plotted.Plot(false);
    gROOT->SetBatch(WasBatch);
    Passed = EvaluateTrue("Plot()", "representative spherical plot", "Plot creates the representative spherical diagnostics canvases", gROOT->GetListOfCanvases()->GetSize() == Before + 3) && Passed;
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    Passed = EvaluateTrue("Plot()", "representative spherical plotted canvas", "Plot leaves a representative spherical diagnostics canvas accessible through ROOT", Canvas != 0) && Passed;
    if (Canvas != 0) {
      while (gROOT->GetListOfCanvases()->GetSize() > Before) {
        TCanvas* ToClose = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
        if (ToClose == 0) break;
        ToClose->Close();
      }
    }
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFunction3DSpherical Test;
  return Test.Run() == true ? 0 : 1;
}
