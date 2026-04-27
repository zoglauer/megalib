/*
 * UTXmlData.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MUnitTest.h"
#include "MXmlData.h"


//! Unit test class for MXmlData
class UTXmlData : public MUnitTest
{
public:
  UTXmlData() : MUnitTest("UTXmlData") {}
  virtual ~UTXmlData() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTXmlData::Run()
{
  bool Passed = true;

  MXmlData Empty;
  Passed = Evaluate("GetName()", "default constructor", "The default xml data name is empty", Empty.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetValue()", "default constructor", "The default xml data value is empty", Empty.GetValue(), MString("")) && Passed;

  MXmlData NameOnly(MString("OnlyName"));
  Passed = Evaluate("GetName()", "name-only constructor", "The name-only constructor stores the name", NameOnly.GetName(), MString("OnlyName")) && Passed;
  Passed = Evaluate("GetValue()", "name-only constructor", "The name-only constructor leaves the value empty", NameOnly.GetValue(), MString("")) && Passed;

  MXmlData StringData(MString("Name"), MString("Value"));
  Passed = Evaluate("GetName()", "string value", "The string constructor stores the name", StringData.GetName(), MString("Name")) && Passed;
  Passed = Evaluate("GetValueAsString()", "string value", "The string constructor stores the value", StringData.GetValueAsString(), MString("Value")) && Passed;

  StringData.SetName("Renamed");
  StringData.SetValue("Revalued");
  Passed = Evaluate("SetName()", "string value", "SetName updates the stored name", StringData.GetName(), MString("Renamed")) && Passed;
  Passed = Evaluate("SetValue()", "string value", "SetValue updates the stored value", StringData.GetValue(), MString("Revalued")) && Passed;

  MTime TimeValue(12, 345678901);
  MXmlData TimeData("Time", TimeValue);
  Passed = Evaluate("GetValueAsString()", "time value", "The time constructor stores the long integer representation", TimeData.GetValueAsString(), MString("12.345678901")) && Passed;
  Passed = EvaluateTrue("GetValueAsTime()", "time value", "Time values round-trip through MXmlData", TimeData.GetValueAsTime().GetLongIntsString() == "12.345678901") && Passed;

  MXmlData IntData("Integer", -42);
  Passed = EvaluateNear("GetValueAsInt()", "int value", "Integer values are converted back correctly", IntData.GetValueAsInt(), -42.0, 1e-12) && Passed;

  MXmlData LongData("Long", 1234567890L);
  Passed = EvaluateNear("GetValueAsLong()", "long value", "Long values are converted back correctly", LongData.GetValueAsLong(), 1234567890.0, 1e-12) && Passed;

  MXmlData UnsignedIntData("UnsignedInt", static_cast<unsigned int>(17));
  Passed = EvaluateNear("GetValueAsUnsignedInt()", "unsigned int value", "Unsigned integer values are converted back correctly", UnsignedIntData.GetValueAsUnsignedInt(), 17.0, 1e-12) && Passed;

  MXmlData UnsignedLongData("UnsignedLong", static_cast<unsigned long>(4294967296UL));
  Passed = EvaluateNear("GetValueAsUnsignedLong()", "unsigned long value", "Unsigned long values preserve values above UINT_MAX", UnsignedLongData.GetValueAsUnsignedLong(), 4294967296.0, 1e-12) && Passed;

  MXmlData DoubleData("Double", 3.125);
  Passed = EvaluateNear("GetValueAsDouble()", "double value", "Double values are converted back correctly", DoubleData.GetValueAsDouble(), 3.125, 1e-12) && Passed;

  MXmlData TrueData("True", true);
  Passed = EvaluateTrue("GetValueAsBoolean()", "true value", "Boolean true is parsed correctly", TrueData.GetValueAsBoolean()) && Passed;

  MXmlData UpperTrueData(MString("UpperTrue"), MString("TRUE"));
  Passed = EvaluateTrue("GetValueAsBoolean()", "upper true value", "Upper-case TRUE is parsed correctly", UpperTrueData.GetValueAsBoolean()) && Passed;

  MXmlData FalseData("False", false);
  Passed = EvaluateFalse("GetValueAsBoolean()", "false value", "Boolean false is parsed correctly", FalseData.GetValueAsBoolean()) && Passed;
  FalseData.SetValue("not_true");
  Passed = EvaluateFalse("GetValueAsBoolean()", "non-true value", "Any non-true string is parsed as false", FalseData.GetValueAsBoolean()) && Passed;

  FalseData.Clear();
  Passed = Evaluate("Clear()", "reset name", "Clear resets the name", FalseData.GetName(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "reset value", "Clear resets the value", FalseData.GetValue(), MString("")) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTXmlData Test;
  return Test.Run() == true ? 0 : 1;
}
