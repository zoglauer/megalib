/*
 * UTXmlAttribute.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MUnitTest.h"
#include "MXmlAttribute.h"
#include "MXmlNode.h"


//! Unit test class for MXmlAttribute
class UTXmlAttribute : public MUnitTest
{
public:
  UTXmlAttribute() : MUnitTest("UTXmlAttribute") {}
  virtual ~UTXmlAttribute() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTXmlAttribute::Run()
{
  bool Passed = true;

  MXmlAttribute Empty;
  Passed = Evaluate("GetName()", "default constructor", "The default attribute name is empty", Empty.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetValue()", "default constructor", "The default attribute value is empty", Empty.GetValue(), MString("")) && Passed;

  MXmlNode Root(0, MString("Root"));
  MXmlAttribute* NameOnly = new MXmlAttribute(&Root, MString("flag"));
  MXmlAttribute* Version = new MXmlAttribute(&Root, MString("version"), MString("1"));
  MXmlAttribute* Count = new MXmlAttribute(&Root, MString("count"), 7);
  MXmlAttribute* UnsignedCount = new MXmlAttribute(&Root, MString("unsignedCount"), static_cast<unsigned int>(9));
  MXmlAttribute* Threshold = new MXmlAttribute(&Root, MString("threshold"), 2.5);
  MXmlAttribute* Enabled = new MXmlAttribute(&Root, MString("enabled"), true);
  MXmlAttribute Detached(0, MString("detached"), MString("value"));

  Passed = EvaluateNear("GetNAttributes()", "attribute count", "Attributes are added to the mother node", Root.GetNAttributes(), 6.0, 1e-12) && Passed;
  Passed = Evaluate("ToString()", "name-only attribute", "The name-only constructor creates an empty-valued attribute", NameOnly->ToString(), MString("flag=\"\"")) && Passed;
  Passed = Evaluate("GetName()", "string attribute", "The string attribute stores its name", Version->GetName(), MString("version")) && Passed;
  Passed = Evaluate("GetValueAsString()", "string attribute", "The string attribute stores its value", Version->GetValueAsString(), MString("1")) && Passed;
  Passed = Evaluate("ToString()", "int attribute", "Integer attributes serialize deterministically", Count->ToString(), MString("count=\"7\"")) && Passed;
  Passed = EvaluateNear("GetValueAsUnsignedInt()", "unsigned int attribute", "Unsigned integer attributes preserve their value", UnsignedCount->GetValueAsUnsignedInt(), 9.0, 1e-12) && Passed;
  Passed = Evaluate("ToString()", "double attribute", "Double attributes serialize deterministically", Threshold->ToString(), MString("threshold=\"2.5\"")) && Passed;
  Passed = Evaluate("GetName()", "boolean attribute", "The boolean attribute stores its name", Enabled->GetName(), MString("enabled")) && Passed;
  Passed = EvaluateTrue("GetValueAsBoolean()", "boolean attribute", "The boolean attribute stores its value", Enabled->GetValueAsBoolean()) && Passed;
  Passed = Evaluate("ToString()", "detached attribute", "Attributes without a mother node still store their own data", Detached.ToString(), MString("detached=\"value\"")) && Passed;
  Passed = Evaluate("GetAttribute(name)", "attribute lookup", "Attributes can be retrieved by name", Root.GetAttribute("version")->GetValueAsString(), MString("1")) && Passed;
  Passed = EvaluateTrue("GetAttribute(index)", "attribute lookup", "Attributes can be retrieved by index", Root.GetAttribute(1) != 0) && Passed;
  Passed = EvaluateTrue("GetAttribute(out of bounds)", "attribute lookup", "Out-of-bounds attribute lookups return null", Root.GetAttribute(7) == 0) && Passed;
  Passed = EvaluateTrue("GetAttribute(missing)", "missing attribute", "Missing attributes return null", Root.GetAttribute("missing") == 0) && Passed;
  Passed = Evaluate("ToString()", "attribute serialization", "Attributes are written into the opening tag", Root.ToString(), MString("<Root flag=\"\" version=\"1\" count=\"7\" unsignedCount=\"9\" threshold=\"2.5\" enabled=\"true\" />")) && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTXmlAttribute Test;
  return Test.Run() == true ? 0 : 1;
}
