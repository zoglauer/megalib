/*
 * UTXmlNode.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MStreams.h"
#include "MUnitTest.h"
#include "MXmlAttribute.h"
#include "MXmlNode.h"


//! Unit test class for MXmlNode
class UTXmlNode : public MUnitTest
{
public:
  UTXmlNode() : MUnitTest("UTXmlNode") {}
  virtual ~UTXmlNode() {}

  virtual bool Run();

private:
  //! Test helper exposing protected MXmlNode parser helpers
  class XmlNodeTest : public MXmlNode
  {
  public:
    XmlNodeTest() : MXmlNode() {}
    virtual ~XmlNodeTest() {}

    bool TestParse(const MString& Text) { return Parse(Text); }
    bool TestIsClosed(const MString& Text) { return IsClosed(Text); }
  };
};


////////////////////////////////////////////////////////////////////////////////


bool UTXmlNode::Run()
{
  bool Passed = true;

  MXmlNode Default;
  Passed = Evaluate("GetName()", "default constructor", "The default node name is empty", Default.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetValue()", "default constructor", "The default node value is empty", Default.GetValue(), MString("")) && Passed;

  MXmlNode Root(0, MString("Root"));
  new MXmlAttribute(&Root, MString("version"), MString("1"));
  new MXmlAttribute(&Root, MString("enabled"), true);
  new MXmlNode(&Root, MString("Title"), MString("Example"));
  new MXmlNode(&Root, MString("Count"), 7);
  new MXmlNode(&Root, MString("LongCount"), 1234567890L);
  new MXmlNode(&Root, MString("UnsignedCount"), static_cast<unsigned int>(11));
  new MXmlNode(&Root, MString("UnsignedLongCount"), static_cast<unsigned long>(4294967296UL));
  new MXmlNode(&Root, MString("Threshold"), 2.5);
  new MXmlNode(&Root, MString("Enabled"), true);
  new MXmlNode(&Root, MString("Range"), 2, 5);
  new MXmlNode(&Root, MString("LongRange"), 1234567890L, 1234567895L);
  new MXmlNode(&Root, MString("DoubleRange"), 1.25, 9.5);
  new MXmlNode(&Root, MString("Point"), MVector(1.0, 2.0, 3.0));
  new MXmlNode(&Root, MString("When"), MTime(5, 600000000));
  new MXmlNode(&Root, MString("TimeRange"), MTime(3, 400000000), MTime(7, 800000000));

  Passed = EvaluateNear("GetNAttributes()", "attribute count", "Attributes are added to the mother node", Root.GetNAttributes(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNNodes()", "node count", "Child nodes are added to the mother node", Root.GetNNodes(), 13.0, 1e-12) && Passed;
  Passed = Evaluate("GetNode(name)", "node lookup", "Nodes can be retrieved by name", Root.GetNode("Title")->GetValueAsString(), MString("Example")) && Passed;
  Passed = Evaluate("GetNode(index)", "node lookup", "Nodes can be retrieved by index", Root.GetNode(0)->GetName(), MString("Title")) && Passed;
  Passed = EvaluateTrue("GetNode(out of bounds)", "node lookup", "Out-of-bounds node lookups return null", Root.GetNode(20) == 0) && Passed;
  Passed = EvaluateTrue("GetNode(missing)", "missing node", "Missing nodes return null", Root.GetNode("missing") == 0) && Passed;
  Passed = Evaluate("GetAttribute(index)", "attribute lookup", "Attributes can be retrieved by index", Root.GetAttribute(0)->GetName(), MString("version")) && Passed;
  Passed = Evaluate("GetAttribute(name)", "attribute lookup", "Attributes can be retrieved by name", Root.GetAttribute("enabled")->GetValueAsString(), MString("true")) && Passed;
  Passed = EvaluateTrue("GetAttribute(out of bounds)", "attribute lookup", "Out-of-bounds attribute lookups return null", Root.GetAttribute(5) == 0) && Passed;
  Passed = EvaluateTrue("GetAttribute(missing)", "attribute lookup", "Missing attribute lookups return null", Root.GetAttribute("missing") == 0) && Passed;

  Passed = EvaluateNear("GetValueAsInt()", "int node", "Integer child values are parsed correctly", Root.GetNode("Count")->GetValueAsInt(), 7.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsLong()", "long node", "Long child values are parsed correctly", Root.GetNode("LongCount")->GetValueAsLong(), 1234567890.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsUnsignedInt()", "unsigned int node", "Unsigned integer child values are parsed correctly", Root.GetNode("UnsignedCount")->GetValueAsUnsignedInt(), 11.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsUnsignedLong()", "unsigned long node", "Unsigned long child values are parsed correctly", Root.GetNode("UnsignedLongCount")->GetValueAsUnsignedLong(), 4294967296.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsDouble()", "double node", "Double child values are parsed correctly", Root.GetNode("Threshold")->GetValueAsDouble(), 2.5, 1e-12) && Passed;
  Passed = EvaluateTrue("GetValueAsBoolean()", "boolean node", "Boolean child values are parsed correctly", Root.GetNode("Enabled")->GetValueAsBoolean()) && Passed;
  Passed = EvaluateNear("GetMinValueAsInt()", "min/max int", "Integer min values are parsed correctly", Root.GetNode("Range")->GetMinValueAsInt(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxValueAsInt()", "min/max int", "Integer max values are parsed correctly", Root.GetNode("Range")->GetMaxValueAsInt(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsLong()", "min/max long", "Long min values are parsed correctly", Root.GetNode("LongRange")->GetMinValueAsLong(), 1234567890.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxValueAsLong()", "min/max long", "Long max values are parsed correctly", Root.GetNode("LongRange")->GetMaxValueAsLong(), 1234567895.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsDouble()", "min/max double", "Double min values are parsed correctly", Root.GetNode("DoubleRange")->GetMinValueAsDouble(), 1.25, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxValueAsDouble()", "min/max double", "Double max values are parsed correctly", Root.GetNode("DoubleRange")->GetMaxValueAsDouble(), 9.5, 1e-12) && Passed;
  Passed = EvaluateTrue("GetMinValueAsTime()", "min/max time", "Time min values are parsed correctly", Root.GetNode("TimeRange")->GetMinValueAsTime().GetLongIntsString() == "3.400000000") && Passed;
  Passed = EvaluateTrue("GetMaxValueAsTime()", "min/max time", "Time max values are parsed correctly", Root.GetNode("TimeRange")->GetMaxValueAsTime().GetLongIntsString() == "7.800000000") && Passed;

  MVector Point = Root.GetNode("Point")->GetValueAsVector();
  Passed = EvaluateNear("GetValueAsVector()", "vector x", "Vector X components are reconstructed correctly", Point.X(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsVector()", "vector y", "Vector Y components are reconstructed correctly", Point.Y(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsVector()", "vector z", "Vector Z components are reconstructed correctly", Point.Z(), 3.0, 1e-12) && Passed;

  Passed = EvaluateTrue("GetValueAsTime()", "time node", "Time child values are reconstructed correctly", Root.GetNode("When")->GetValueAsTime().GetLongIntsString() == "5.600000000") && Passed;
  MString ExpectedRootText =
    "<Root version=\"1\" enabled=\"true\">\n"
    "  <Title>Example</Title>\n"
    "  <Count>7</Count>\n"
    "  <LongCount>1234567890</LongCount>\n"
    "  <UnsignedCount>11</UnsignedCount>\n"
    "  <UnsignedLongCount>4294967296</UnsignedLongCount>\n"
    "  <Threshold>2.5</Threshold>\n"
    "  <Enabled>true</Enabled>\n"
    "  <Range>\n"
    "    <Min>2</Min>\n"
    "    <Max>5</Max>\n"
    "  </Range>\n"
    "  <LongRange>\n"
    "    <Min>1234567890</Min>\n"
    "    <Max>1234567895</Max>\n"
    "  </LongRange>\n"
    "  <DoubleRange>\n"
    "    <Min>1.25</Min>\n"
    "    <Max>9.5</Max>\n"
    "  </DoubleRange>\n"
    "  <Point>\n"
    "    <X>1</X>\n"
    "    <Y>2</Y>\n"
    "    <Z>3</Z>\n"
    "  </Point>\n"
    "  <When>5.600000000</When>\n"
    "  <TimeRange>\n"
    "    <Min>3.400000000</Min>\n"
    "    <Max>7.800000000</Max>\n"
    "  </TimeRange>\n"
    "</Root>";
  Passed = Evaluate("ToString()", "full serialization", "Representative XML nodes serialize deterministically", Root.ToString(), ExpectedRootText) && Passed;
  Passed = Evaluate("ToString()", "indented serialization", "Representative XML nodes honor explicit indentation", Root.ToString(2).GetSubString(0, 7), MString("  <Root")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty node serialization", "Empty nodes use the self-closing tag form", MXmlNode(0, MString("Empty")).ToString() == "<Empty />") && Passed;

  DisableDefaultStreams();
  Passed = EvaluateNear("GetValueAsVector()", "non-vector node", "Non-vector nodes return a default vector", Root.GetNode("Count")->GetValueAsVector().Mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsInt()", "non-range node", "Non-range nodes return zero for min values", Root.GetNode("Count")->GetMinValueAsInt(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxValueAsLong()", "non-range node", "Non-range nodes return zero for long max values", Root.GetNode("Count")->GetMaxValueAsLong(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsDouble()", "non-range node", "Non-range nodes return zero for double min values", Root.GetNode("Count")->GetMinValueAsDouble(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetMaxValueAsTime()", "non-range node", "Non-range nodes return zero time for time max values", Root.GetNode("Count")->GetMaxValueAsTime().GetLongIntsString() == "0.000000000") && Passed;
  EnableDefaultStreams();

  {
    MXmlNode RootWithValue(0, "HasValue", "text");
    MXmlNode* Added = new MXmlNode(0, "Added", "child");
    RootWithValue.AddNode(Added);
    Passed = Evaluate("AddNode()", "clears plain value", "Adding a child node clears the parent plain-text value", RootWithValue.GetValue(), MString("")) && Passed;
    Passed = Evaluate("GetNNodes()", "manual add", "AddNode appends the node to the child list", RootWithValue.GetNNodes(), 1U) && Passed;
  }

  {
    MXmlNode RootWithoutAttributes(0, "ManualAttributes");
    MXmlAttribute* Added = new MXmlAttribute(0, "manual", MString("value"));
    RootWithoutAttributes.AddAttribute(Added);
    Passed = Evaluate("GetNAttributes()", "manual add", "AddAttribute appends the attribute to the attribute list", RootWithoutAttributes.GetNAttributes(), 1U) && Passed;
    Passed = Evaluate("GetAttribute(name)", "manual add", "AddAttribute makes the attribute retrievable by name", RootWithoutAttributes.GetAttribute("manual")->GetValueAsString(), MString("value")) && Passed;
  }

  {
    XmlNodeTest Parsed;
    Parsed.SetName("Parent");
    Passed = Evaluate("TestParse()", "representative child list", "Parse accepts representative nested XML content", Parsed.TestParse("<Child flag=\"1\">Value</Child><Empty />"), true) && Passed;
    Passed = Evaluate("GetNNodes()", "representative child list", "Parse creates the expected number of child nodes", Parsed.GetNNodes(), 2U) && Passed;
    Passed = Evaluate("GetNode(name)", "representative child list", "Parse restores nested text values", Parsed.GetNode("Child")->GetValueAsString(), MString("Value")) && Passed;
    Passed = Evaluate("GetAttribute(name)", "representative child list", "Parse restores nested attributes", Parsed.GetNode("Child")->GetAttribute("flag")->GetValueAsString(), MString("1")) && Passed;
    Passed = EvaluateTrue("TestIsClosed()", "empty node", "IsClosed detects self-closing XML tags", Parsed.TestIsClosed("<Empty />")) && Passed;
    Passed = EvaluateFalse("TestIsClosed()", "normal node", "IsClosed rejects non-self-closing XML tags", Parsed.TestIsClosed("<Node>Value</Node>")) && Passed;
  }

  {
    XmlNodeTest Parsed;
    Parsed.SetName("Parent");
    DisableDefaultStreams();
    Passed = Evaluate("TestParse()", "mixed text and subnodes", "Parse rejects XML content that mixes text and child nodes", Parsed.TestParse("<Child></Child>text<Other></Other>"), false) && Passed;
    EnableDefaultStreams();
  }

  {
    MXmlNode ClearNode(0, "ClearMe", "Value");
    new MXmlAttribute(&ClearNode, "flag", "1");
    new MXmlNode(&ClearNode, "Child", "text");
    ClearNode.Clear();
    Passed = Evaluate("Clear()", "reset name", "Clear resets the node name", ClearNode.GetName(), MString("")) && Passed;
    Passed = Evaluate("Clear()", "reset value", "Clear resets the node value", ClearNode.GetValue(), MString("")) && Passed;
    Passed = Evaluate("Clear()", "reset children", "Clear resets the child-node list", ClearNode.GetNNodes(), 0U) && Passed;
    Passed = Evaluate("Clear()", "reset attributes", "Clear resets the attribute list", ClearNode.GetNAttributes(), 0U) && Passed;
  }

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTXmlNode Test;
  return Test.Run() == true ? 0 : 1;
}
