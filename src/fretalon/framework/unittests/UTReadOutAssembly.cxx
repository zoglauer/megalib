/*
 * UTReadOutAssembly.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MReadOut.h"
#include "MReadOutAssembly.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutElementStrip.h"
#include "MReadOutFileFormat.h"
#include "MStreams.h"
#include "MUnitTest.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for MReadOutAssembly
class UTReadOutAssembly : public MUnitTest
{
public:
  //! Default constructor
  UTReadOutAssembly() : MUnitTest("UTReadOutAssembly") {}
  //! Default destructor
  virtual ~UTReadOutAssembly() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Add a representative read-out to the assembly
  void AddRepresentativeReadOut(MReadOutAssembly& Assembly);
  //! Test construction defaults and clearing
  bool TestDefaultsAndClear();
  //! Test filtered and analysis-progress flags
  bool TestFlags();
  //! Test parsing delegated to the assembly and base read-out sequence
  bool TestParsing();
  //! Test EVTA and ROA streaming
  bool TestStreaming();
};


////////////////////////////////////////////////////////////////////////////////


bool UTReadOutAssembly::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndClear() && Passed;
  Passed = TestFlags() && Passed;
  Passed = TestParsing() && Passed;
  Passed = TestStreaming() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTReadOutAssembly::AddRepresentativeReadOut(MReadOutAssembly& Assembly)
{
  MReadOutElementStrip Element(7, 11);
  MReadOutDataADCValue Data;
  Data.SetADCValue(1234);
  MReadOut ReadOut(Element, Data);
  Assembly.AddReadOut(ReadOut);
}


////////////////////////////////////////////////////////////////////////////////


bool UTReadOutAssembly::TestDefaultsAndClear()
{
  bool Passed = true;

  MReadOutAssembly First;
  MReadOutAssembly Second;
  Passed = EvaluateTrue("GetAssemblyID()", "two assemblies", "Assembly identifiers are monotonically increasing", Second.GetAssemblyID() > First.GetAssemblyID()) && Passed;
  Passed = Evaluate("GetID()", "default", "The default event ID is undefined after construction", First.GetID(), static_cast<unsigned long>(g_UnsignedIntNotDefined)) && Passed;
  Passed = EvaluateSize("GetNumberOfReadOuts()", "default", "A new read-out assembly has no read-outs", First.GetNumberOfReadOuts(), static_cast<size_t>(0)) && Passed;

  First.SetID(42);
  AddRepresentativeReadOut(First);
  First.SetFilteredOut(true);
  First.SetAnalysisProgress(0x1);
  First.Clear();
  Passed = Evaluate("Clear()", "ID reset", "Clear resets the event ID to undefined", First.GetID(), static_cast<unsigned long>(g_UnsignedIntNotDefined)) && Passed;
  Passed = EvaluateFalse("Clear()", "filtered flag reset", "Clear resets the filtered-out flag", First.IsFilteredOut()) && Passed;
  Passed = Evaluate("Clear()", "analysis progress reset", "Clear resets the analysis progress mask", First.GetAnalysisProgress(), static_cast<uint64_t>(0)) && Passed;
  Passed = EvaluateSize("Clear()", "read-outs dropped", "Clear drops the stored read-outs through the base sequence", First.GetNumberOfReadOuts(), static_cast<size_t>(0)) && Passed;
  Passed = EvaluateNear("Clear()", "time reset", "Clear resets the time through the base sequence", First.GetTime().GetAsSeconds(), 0.0, 1.0e-12) && Passed;

  // Regression: a reused assembly must not accumulate read-outs across successive fills
  AddRepresentativeReadOut(First);
  First.Clear();
  AddRepresentativeReadOut(First);
  Passed = EvaluateSize("Clear()", "reused assembly", "A cleared assembly holds only the read-outs added after the clear", First.GetNumberOfReadOuts(), static_cast<size_t>(1)) && Passed;
  Passed = EvaluateTrue("Clear()", "assembly ID stable", "Clear preserves the unique assembly identifier", First.GetAssemblyID() < Second.GetAssemblyID()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTReadOutAssembly::TestFlags()
{
  bool Passed = true;

  MReadOutAssembly Assembly;
  Assembly.SetFilteredOut(false);
  Passed = EvaluateFalse("IsFilteredOut()", "cleared flag", "The filtered-out flag can be cleared", Assembly.IsFilteredOut()) && Passed;
  Assembly.SetFilteredOut();
  Passed = EvaluateTrue("SetFilteredOut()", "default flag", "SetFilteredOut() marks the assembly as filtered", Assembly.IsFilteredOut()) && Passed;
  Assembly.SetFilteredOut(false);
  Passed = EvaluateFalse("SetFilteredOut(false)", "cleared flag", "SetFilteredOut(false) clears the filtered flag", Assembly.IsFilteredOut()) && Passed;
  Passed = EvaluateTrue("IsGood()", "default", "The current assembly quality helper always reports good", Assembly.IsGood()) && Passed;
  Passed = EvaluateFalse("IsBad()", "default", "The current assembly quality helper reports not bad", Assembly.IsBad()) && Passed;

  Assembly.SetAnalysisProgress(0x1);
  Passed = EvaluateTrue("HasAnalysisProgress()", "first bit", "A set progress bit is reported as present", Assembly.HasAnalysisProgress(0x1)) && Passed;
  Passed = EvaluateFalse("HasAnalysisProgress()", "missing bit", "A missing progress bit is reported as absent", Assembly.HasAnalysisProgress(0x2)) && Passed;
  Assembly.SetAnalysisProgress(0x4);
  Passed = EvaluateTrue("HasAnalysisProgress()", "combined mask", "Multiple progress bits can be accumulated", Assembly.HasAnalysisProgress(0x5)) && Passed;
  Passed = Evaluate("GetAnalysisProgress()", "combined mask", "The accumulated progress mask is returned", Assembly.GetAnalysisProgress(), static_cast<uint64_t>(0x5)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTReadOutAssembly::TestParsing()
{
  bool Passed = true;

  MReadOutAssembly Assembly;
  Assembly.SetFilteredOut(false);

  MString IDLine("ID 77");
  Passed = EvaluateTrue("Parse()", "ID line", "The assembly parser consumes ID lines through the base sequence", Assembly.Parse(IDLine)) && Passed;
  Passed = Evaluate("GetID()", "ID line", "The parsed ID is stored on the assembly", Assembly.GetID(), static_cast<unsigned long>(77)) && Passed;

  MString TimeLine("TI 12.5");
  Passed = EvaluateTrue("Parse()", "TI line", "The assembly parser consumes time lines through the base sequence", Assembly.Parse(TimeLine)) && Passed;
  Passed = EvaluateNear("GetTime()", "TI line", "The parsed time is stored on the assembly", Assembly.GetTime().GetAsSeconds(), 12.5, 1.0e-12) && Passed;

  MString BareBadLine("BD");
  Passed = EvaluateTrue("Parse()", "bare BD line", "A bad-data line without flags is consumed by the assembly parser", Assembly.Parse(BareBadLine)) && Passed;
  Passed = EvaluateTrue("IsFilteredOut()", "bare BD line", "A bad-data line without flags marks the assembly as filtered out", Assembly.IsFilteredOut()) && Passed;

  MReadOutAssembly FlaggedAssembly;
  MString FlaggedBadLine("BD OneFlag AnotherFlag");
  Passed = EvaluateTrue("Parse()", "BD line with flags", "A bad-data line listing flags is consumed by the assembly parser", FlaggedAssembly.Parse(FlaggedBadLine)) && Passed;
  Passed = EvaluateTrue("IsFilteredOut()", "BD line with flags", "A bad-data line listing flags marks the assembly as filtered out", FlaggedAssembly.IsFilteredOut()) && Passed;

  MReadOutAssembly VersionedAssembly;
  MString VersionedIDLine("ID 99");
  Passed = EvaluateTrue("Parse()", "explicit version", "An explicit format version is accepted and forwarded to the base sequence parser", VersionedAssembly.Parse(VersionedIDLine, 2)) && Passed;
  Passed = Evaluate("GetID()", "explicit version", "The ID is parsed when an explicit format version is supplied", VersionedAssembly.GetID(), static_cast<unsigned long>(99)) && Passed;

  MString UnknownLine("ZZ representative ignored line");
  Passed = EvaluateTrue("Parse()", "unknown line", "Unknown lines are tolerantly consumed by the base read-out sequence parser", Assembly.Parse(UnknownLine)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTReadOutAssembly::TestStreaming()
{
  bool Passed = true;

  MReadOutAssembly Assembly;
  Assembly.SetFilteredOut(false);
  Assembly.SetID(42);
  AddRepresentativeReadOut(Assembly);

  ostringstream Evta;
  Assembly.StreamEvta(Evta);
  Passed = Evaluate("StreamEvta()", "representative assembly", "EVTA streaming writes the event header, ID, and default time", MString(Evta.str()), MString("SE\nID 42\nTI 0.000000000\n")) && Passed;

  // Read-out file format matching the representative read-out
  MReadOutFileFormat ROFF;
  ROFF.AddReadOutUnit(Assembly.GetReadOut(0).GetReadOutElement().GetType(), Assembly.GetReadOut(0).GetReadOutData().GetCombinedType());

  ostringstream RoaWithoutDescriptor;
  Passed = EvaluateTrue("StreamRoa()", "defined read-out unit", "ROA streaming succeeds when the read-out unit is defined", Assembly.StreamRoa(RoaWithoutDescriptor, ROFF)) && Passed;
  Passed = Evaluate("StreamRoa()", "without descriptor", "ROA streaming writes the event header and the read-out under its keyword",
                    MString(RoaWithoutDescriptor.str()), MString("SE\nID 42\nTI 0.000000000\nUH 7 11 1234 \n")) && Passed;

  ostringstream RoaWithDescriptor;
  Passed = EvaluateTrue("StreamRoa()", "with descriptor", "ROA streaming with descriptors succeeds", Assembly.StreamRoa(RoaWithDescriptor, ROFF, true)) && Passed;
  Passed = EvaluateTrue("StreamRoa()", "with descriptor", "ROA streaming with descriptors writes the read-out type descriptors",
                        MString(RoaWithDescriptor.str()).Contains("UH sss 7 11") && MString(RoaWithDescriptor.str()).Contains("adc 1234")) && Passed;

  // Keyword of a second unit
  MReadOutFileFormat SecondROFF;
  SecondROFF.AddReadOutUnit("U7", "doublesidedstrip", "adc");
  SecondROFF.AddReadOutUnit("U8", Assembly.GetReadOut(0).GetReadOutElement().GetType(), Assembly.GetReadOut(0).GetReadOutData().GetCombinedType());
  ostringstream RoaSecondUnit;
  Passed = EvaluateTrue("StreamRoa()", "second read-out unit", "ROA streaming succeeds with the read-out unit at the second position", Assembly.StreamRoa(RoaSecondUnit, SecondROFF)) && Passed;
  Passed = EvaluateTrue("StreamRoa()", "second read-out unit", "The read-out is written under the keyword of its own read-out unit",
                        MString(RoaSecondUnit.str()).Contains("\nU8 7 11 1234") && !MString(RoaSecondUnit.str()).Contains("U7 ")) && Passed;

  // No read-out unit for this read-out
  MReadOutFileFormat WrongROFF;
  WrongROFF.AddReadOutUnit("doublesidedstrip", "adc");
  ostringstream RoaUndefined;
  const int PreviousVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  const bool UndefinedStreamed = Assembly.StreamRoa(RoaUndefined, WrongROFF);
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateFalse("StreamRoa()", "undefined read-out unit", "ROA streaming fails when a read-out has no read-out unit", UndefinedStreamed) && Passed;
  Passed = EvaluateTrue("StreamRoa()", "undefined read-out unit", "Nothing is written when a read-out has no read-out unit", RoaUndefined.str().empty()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTReadOutAssembly Test;
  return Test.Run() == true ? 0 : 1;
}
