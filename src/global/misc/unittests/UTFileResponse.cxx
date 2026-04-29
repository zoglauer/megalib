/*
 * UTFileResponse.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fstream>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MBinaryStore.h"
#include "MFileResponse.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixON.h"
#include "MUnitTest.h"


//! Unit test class for MFileResponse
class UTFileResponse : public MUnitTest
{
public:
  UTFileResponse() : MUnitTest("UTFileResponse") {}
  virtual ~UTFileResponse() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTFileResponse::Run()
{
  bool Passed = true;

  system("mkdir -p /tmp/UTFileResponse");

  MFileResponse Default;
  Passed = Evaluate("GetName()", "default constructor", "The default response-file parser starts with an undefined representative name", Default.GetName(), g_StringNotDefined) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "default constructor", "The default response-file parser starts with zero representative simulated events", Default.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetFarFieldStartArea()", "default constructor", "The default response-file parser starts with zero representative far-field area", Default.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetSpectralType()", "default constructor", "The default response-file parser starts with an empty representative spectral type", Default.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("GetSpectralParameters()", "default constructor", "The default response-file parser starts with no representative spectral parameters", Default.GetSpectralParameters().size(), 0UL) && Passed;
  Passed = Evaluate("AreValuesCentered()", "default constructor", "The default response-file parser assumes representative values are centered", Default.AreValuesCentered(), true) && Passed;
  Passed = Evaluate("GetHash()", "default constructor", "The default response-file parser starts with hash zero", Default.GetHash(), 0UL) && Passed;
  MBinaryStore Store;
  Passed = Evaluate("Read(MBinaryStore,...)", "disabled overload", "The disabled MBinaryStore overload returns false", Default.Read(Store, 0), false) && Passed;

  MString HeaderFile = "/tmp/UTFileResponse/header_only.rsp";
  {
    ofstream Out(HeaderFile.Data());
    Out<<"Version 1\n";
    Out<<"Type ResponseMatrixO1Stream\n";
    Out<<"NM HeaderOnly\n";
    Out<<"TS 88\n";
    Out<<"SA 7.5\n";
    Out<<"SM Mono 511\n";
    Out<<"HA 1234\n";
    Out<<"CE false\n";
    Out<<"StartStream 0\n";
  }

  MFileResponse HeaderReader;
  Passed = Evaluate("Open()", "representative header", "Open reads the representative response header successfully", HeaderReader.Open(HeaderFile), true) && Passed;
  Passed = Evaluate("GetName()", "representative header", "Open stores the representative matrix name", HeaderReader.GetName(), MString("HeaderOnly")) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "representative header", "Open stores the representative simulated-event count", HeaderReader.GetSimulatedEvents(), 88L) && Passed;
  Passed = EvaluateNear("GetFarFieldStartArea()", "representative header", "Open stores the representative far-field area", HeaderReader.GetFarFieldStartArea(), 7.5, 1e-12) && Passed;
  Passed = Evaluate("GetSpectralType()", "representative header", "Open stores the representative spectral type", HeaderReader.GetSpectralType(), MString("Mono")) && Passed;
  Passed = Evaluate("GetSpectralParameters()", "representative header", "Open stores one representative spectral parameter", HeaderReader.GetSpectralParameters().size(), 1UL) && Passed;
  Passed = EvaluateNear("GetSpectralParameters()", "representative header", "Open stores the representative spectral parameter value", HeaderReader.GetSpectralParameters()[0], 511.0, 1e-12) && Passed;
  Passed = Evaluate("AreValuesCentered()", "representative header", "Open stores the representative centered flag", HeaderReader.AreValuesCentered(), false) && Passed;
  Passed = Evaluate("GetHash()", "representative header", "Open stores the representative hash", HeaderReader.GetHash(), 1234UL) && Passed;
  HeaderReader.Close();

  MString PartialHeaderFile = "/tmp/UTFileResponse/header_partial.rsp";
  {
    ofstream Out(PartialHeaderFile.Data());
    Out<<"Version 1\n";
    Out<<"Type ResponseMatrixO1Stream\n";
    Out<<"CE true\n";
    Out<<"StartStream 0\n";
  }
  Passed = Evaluate("Open()", "reused parser first header", "Open reads the first representative header before the parser is reused", HeaderReader.Open(HeaderFile), true) && Passed;
  HeaderReader.Close();
  Passed = Evaluate("Open()", "reused parser second header", "Open also succeeds on a representative partial second header", HeaderReader.Open(PartialHeaderFile), true) && Passed;
  Passed = Evaluate("GetName()", "reused parser second header", "Reusing the parser does not keep a stale representative matrix name", HeaderReader.GetName(), g_StringNotDefined) && Passed;
  Passed = Evaluate("GetHash()", "reused parser second header", "Reusing the parser does not keep a stale representative hash", HeaderReader.GetHash(), 0UL) && Passed;
  Passed = Evaluate("GetSimulatedEvents()", "reused parser second header", "Reusing the parser does not keep a stale representative simulated-event count", HeaderReader.GetSimulatedEvents(), 0L) && Passed;
  Passed = EvaluateNear("GetFarFieldStartArea()", "reused parser second header", "Reusing the parser does not keep a stale representative far-field area", HeaderReader.GetFarFieldStartArea(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetSpectralType()", "reused parser second header", "Reusing the parser does not keep a stale representative spectral type", HeaderReader.GetSpectralType(), MString("")) && Passed;
  Passed = Evaluate("GetSpectralParameters()", "reused parser second header", "Reusing the parser does not keep stale representative spectral parameters", HeaderReader.GetSpectralParameters().size(), 0UL) && Passed;
  HeaderReader.Close();

  MResponseMatrixO1 O1("RepresentativeO1", vector<float>{0.0f, 1.0f, 2.0f});
  O1.SetAxisNames("X");
  O1.SetBinContent(0, 3.0f);
  O1.SetSimulatedEvents(21);
  O1.SetFarFieldStartArea(4.0);
  O1.SetSpectrum("Mono", vector<double>{511.0});
  O1.SetHash(999UL);
  MString O1File = "/tmp/UTFileResponse/o1_stream.rsp";
  Passed = Evaluate("Write()", "representative O1 stream file", "The representative O1 matrix can be written for MFileResponse dispatch", O1.Write(O1File, true), true) && Passed;

  MFileResponse O1Reader;
  MResponseMatrix* O1ReadBack = O1Reader.Read(O1File);
  Passed = EvaluateTrue("Read()", "representative O1 dispatch", "MFileResponse dispatches the representative O1 file to a concrete matrix", O1ReadBack != 0) && Passed;
  if (O1ReadBack != 0) {
    Passed = Evaluate("Read()", "representative O1 dispatch type", "MFileResponse creates the representative O1 response type", dynamic_cast<MResponseMatrixO1*>(O1ReadBack) != 0, true) && Passed;
    Passed = EvaluateNear("Read()", "representative O1 dispatch content", "The representative O1 bin content survives dispatch reading", dynamic_cast<MResponseMatrixO1*>(O1ReadBack)->GetBinContent(0), 3.0, 1e-6) && Passed;
    delete O1ReadBack;
  }

  MResponseMatrixON ON("RepresentativeON");
  ON.AddAxisLinear("Energy", 2, 0.0, 2.0);
  ON.AddAxisLinear("Phi", 2, 0.0, 2.0);
  ON.Set(vector<unsigned long>{0, 1}, 5.0f);
  ON.SetSimulatedEvents(9);
  ON.SetFarFieldStartArea(2.0);
  ON.SetSpectrum("Linear", vector<double>{1.0, 2.0});
  ON.SetHash(777UL);
  MString ONFile = "/tmp/UTFileResponse/on_sparse.rsp";
  {
    DisableDefaultStreams();
    Passed = Evaluate("Write()", "representative ON sparse file", "The representative ON matrix can be written for MFileResponse dispatch", ON.Write(ONFile, false), true) && Passed;
    EnableDefaultStreams();
  }

  MFileResponse ONReader;
  MResponseMatrix* ONReadBack = 0;
  {
    DisableDefaultStreams();
    ONReadBack = ONReader.Read(ONFile);
    EnableDefaultStreams();
  }
  Passed = EvaluateTrue("Read()", "representative ON dispatch", "MFileResponse dispatches the representative ON file to a concrete matrix", ONReadBack != 0) && Passed;
  if (ONReadBack != 0) {
    Passed = Evaluate("Read()", "representative ON dispatch type", "MFileResponse creates the representative ON response type", dynamic_cast<MResponseMatrixON*>(ONReadBack) != 0, true) && Passed;
    Passed = EvaluateNear("Read()", "representative ON dispatch content", "The representative ON bin content survives dispatch reading", dynamic_cast<MResponseMatrixON*>(ONReadBack)->Get(vector<unsigned long>{0, 1}), 5.0, 1e-6) && Passed;
    delete ONReadBack;
  }

  MString UnknownFile = "/tmp/UTFileResponse/unknown.rsp";
  {
    ofstream Out(UnknownFile.Data());
    Out<<"Version 1\n";
    Out<<"Type UnknownMatrix\n";
    Out<<"NM Unknown\n";
    Out<<"TS 1\n";
    Out<<"SA 0\n";
    Out<<"SM \n";
    Out<<"HA 1\n";
    Out<<"CE true\n";
  }
  MFileResponse UnknownReader;
  {
    DisableDefaultStreams();
    Passed = EvaluateTrue("Read()", "unknown type", "MFileResponse rejects a representative unknown response type", UnknownReader.Read(UnknownFile) == 0) && Passed;
    EnableDefaultStreams();
  }

  MString MissingCEFile = "/tmp/UTFileResponse/missing_ce.rsp";
  {
    ofstream Out(MissingCEFile.Data());
    Out<<"Version 1\n";
    Out<<"Type ResponseMatrixO1Stream\n";
    Out<<"NM MissingCE\n";
    Out<<"TS 1\n";
    Out<<"SA 0\n";
    Out<<"SM Mono 511\n";
    Out<<"HA 1\n";
    Out<<"StartStream 0\n";
  }
  MFileResponse MissingCEReader;
  Passed = Evaluate("Open()", "missing CE", "Open accepts a representative header without CE and falls back to centered values", MissingCEReader.Open(MissingCEFile), true) && Passed;
  Passed = Evaluate("AreValuesCentered()", "missing CE", "Open falls back to representative centered values when CE is missing", MissingCEReader.AreValuesCentered(), true) && Passed;
  MissingCEReader.Close();

  MFileResponse MissingReader;
  {
    DisableDefaultStreams();
    Passed = Evaluate("Open()", "missing file", "Open returns false for a representative missing response file", MissingReader.Open("/tmp/UTFileResponse/does_not_exist.rsp"), false) && Passed;
    EnableDefaultStreams();
  }

  ostringstream Buffer;
  Buffer<<"Alpha";
  MString WriteFile = "/tmp/UTFileResponse/write_stream.txt";
  MFileResponse Writer;
  Passed = Evaluate("Open()", "writer", "Open can create a representative output file", Writer.Open(WriteFile, MFile::c_Write), true) && Passed;
  if (Writer.IsOpen() == true) {
    Writer.Write(Buffer);
    Passed = Evaluate("Write(ostringstream&)", "clear stream", "Write(ostringstream&) clears the representative source stream after writing", MString(Buffer.str()), MString("")) && Passed;
    Writer.Write(1.5);
    Writer.Close();
    ifstream In(WriteFile.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = Evaluate("Write(const double)", "representative output", "Write(const double) appends the representative value followed by a space", MString(Content.c_str()), MString("Alpha1.5 ")) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileResponse Test;
  return Test.Run() == true ? 0 : 1;
}
