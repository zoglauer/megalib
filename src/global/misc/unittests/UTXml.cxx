/*
 * UTXml.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MXmlData.h"
#include "MXmlAttribute.h"
#include "MXmlNode.h"
#include "MXmlDocument.h"
#include "MUnitTest.h"
#include "MStreams.h"

// Standard libs:
#include <cstdio>
using namespace std;


//! Unit test class for the MXml helper classes
class UTXml : public MUnitTest
{
public:
  //! Default constructor
  UTXml() : MUnitTest("UTXml") {}
  //! Default destructor
  virtual ~UTXml() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test the shared MXmlData value conversions
  bool TestXmlData();
  //! Test node and attribute construction helpers
  bool TestXmlNodeAndAttribute();
  //! Test document save/load round-tripping for MEGAlib XML
  bool TestXmlRoundTrip();
  //! Test settings-style repeated loading on the same document
  bool TestXmlReload();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTXml::Run()
{
  bool AllPassed = true;

  AllPassed = TestXmlData() && AllPassed;
  AllPassed = TestXmlNodeAndAttribute() && AllPassed;
  AllPassed = TestXmlRoundTrip() && AllPassed;
  AllPassed = TestXmlReload() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the shared MXmlData value conversions
bool UTXml::TestXmlData()
{
  bool Passed = true;

  MXmlData Empty;
  Passed = Evaluate("GetName()", "default constructor", "The default xml data name is empty", Empty.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetValue()", "default constructor", "The default xml data value is empty", Empty.GetValue(), MString("")) && Passed;

  MXmlData StringData(MString("Name"), MString("Value"));
  Passed = Evaluate("GetName()", "string value", "The string constructor stores the name", StringData.GetName(), MString("Name")) && Passed;
  Passed = Evaluate("GetValueAsString()", "string value", "The string constructor stores the value", StringData.GetValueAsString(), MString("Value")) && Passed;

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

  FalseData.Clear();
  Passed = Evaluate("Clear()", "reset name", "Clear resets the name", FalseData.GetName(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "reset value", "Clear resets the value", FalseData.GetValue(), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test node and attribute construction helpers
bool UTXml::TestXmlNodeAndAttribute()
{
  bool Passed = true;

  MXmlNode Root(0, MString("Root"));
  new MXmlAttribute(&Root, MString("version"), MString("1"));
  new MXmlAttribute(&Root, MString("enabled"), true);
  new MXmlNode(&Root, MString("Title"), MString("Example"));
  new MXmlNode(&Root, MString("Count"), 7);
  new MXmlNode(&Root, MString("Range"), 2, 5);
  new MXmlNode(&Root, MString("Point"), MVector(1.0, 2.0, 3.0));
  new MXmlNode(&Root, MString("When"), MTime(5, 600000000));

  Passed = EvaluateNear("GetNAttributes()", "attribute count", "Attributes are added to the mother node", Root.GetNAttributes(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNNodes()", "node count", "Child nodes are added to the mother node", Root.GetNNodes(), 5.0, 1e-12) && Passed;
  Passed = Evaluate("GetAttribute(name)", "attribute lookup", "Attributes can be retrieved by name", Root.GetAttribute("version")->GetValueAsString(), MString("1")) && Passed;
  Passed = EvaluateTrue("GetAttribute(index)", "attribute lookup", "Attributes can be retrieved by index", Root.GetAttribute(1) != 0) && Passed;
  Passed = EvaluateTrue("GetAttribute(missing)", "missing attribute", "Missing attributes return null", Root.GetAttribute("missing") == 0) && Passed;

  Passed = Evaluate("GetNode(name)", "node lookup", "Nodes can be retrieved by name", Root.GetNode("Title")->GetValueAsString(), MString("Example")) && Passed;
  Passed = EvaluateTrue("GetNode(index)", "node lookup", "Nodes can be retrieved by index", Root.GetNode(0) != 0) && Passed;
  Passed = EvaluateTrue("GetNode(missing)", "missing node", "Missing nodes return null", Root.GetNode("missing") == 0) && Passed;

  Passed = EvaluateNear("GetValueAsInt()", "int node", "Integer child values are parsed correctly", Root.GetNode("Count")->GetValueAsInt(), 7.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsInt()", "min/max int", "Integer min values are parsed correctly", Root.GetNode("Range")->GetMinValueAsInt(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaxValueAsInt()", "min/max int", "Integer max values are parsed correctly", Root.GetNode("Range")->GetMaxValueAsInt(), 5.0, 1e-12) && Passed;

  MVector Point = Root.GetNode("Point")->GetValueAsVector();
  Passed = EvaluateNear("GetValueAsVector()", "vector x", "Vector X components are reconstructed correctly", Point.X(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsVector()", "vector y", "Vector Y components are reconstructed correctly", Point.Y(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetValueAsVector()", "vector z", "Vector Z components are reconstructed correctly", Point.Z(), 3.0, 1e-12) && Passed;

  Passed = EvaluateTrue("GetValueAsTime()", "time node", "Time child values are reconstructed correctly", Root.GetNode("When")->GetValueAsTime().GetLongIntsString() == "5.600000000") && Passed;

  Passed = EvaluateTrue("ToString()", "attribute serialization", "Attributes are written into the opening tag", Root.ToString().Contains("version=\"1\"")) && Passed;
  Passed = EvaluateTrue("ToString()", "child serialization", "Child nodes are serialized into xml text", Root.ToString().Contains("<Title>Example</Title>")) && Passed;
  Passed = EvaluateTrue("ToString()", "empty node serialization", "Empty nodes use the self-closing tag form", MXmlNode(0, MString("Empty")).ToString() == "<Empty />") && Passed;

  DisableDefaultStreams();
  Passed = EvaluateNear("GetValueAsVector()", "non-vector node", "Non-vector nodes return a default vector", Root.GetNode("Count")->GetValueAsVector().Mag(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinValueAsInt()", "non-range node", "Non-range nodes return zero for min values", Root.GetNode("Count")->GetMinValueAsInt(), 0.0, 1e-12) && Passed;
  EnableDefaultStreams();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test document save/load round-tripping for MEGAlib XML
bool UTXml::TestXmlRoundTrip()
{
  bool Passed = true;

  MXmlDocument Document(MString("Config"));
  new MXmlAttribute(&Document, MString("version"), MString("2"));
  new MXmlNode(&Document, MString("Name"), MString("Detector"));
  new MXmlNode(&Document, MString("Enabled"), true);
  new MXmlNode(&Document, MString("Threshold"), 12.5);
  new MXmlNode(&Document, MString("Bounds"), 1.25, 9.5);
  new MXmlNode(&Document, MString("TimeWindow"), MTime(3, 400000000), MTime(7, 800000000));

  MXmlNode* Group = new MXmlNode(&Document, MString("Group"));
  new MXmlAttribute(Group, MString("kind"), MString("primary"));
  new MXmlNode(Group, MString("Item"), MString("A"));
  new MXmlNode(Group, MString("Item"), MString("B"));
  new MXmlNode(Group, MString("Position"), MVector(-1.0, 0.5, 2.25));

  MXmlNode* ExtraGroup = new MXmlNode(&Document, MString("ExtraGroup"));
  new MXmlNode(ExtraGroup, MString("Item"), MString("Nested"));
  new MXmlNode(ExtraGroup, MString("Label"), MString("Inner"));

  MString FileName = "/tmp/UTXml_roundtrip.xml";
  Passed = EvaluateTrue("Save()", "round trip", "Documents can be saved to disk", Document.Save(FileName)) && Passed;

  MXmlDocument Loaded;
  Passed = EvaluateTrue("Load()", "round trip", "Previously saved MEGAlib xml can be loaded again", Loaded.Load(FileName)) && Passed;
  if (Loaded.GetName() == "Config") {
    Passed = Evaluate("GetName()", "round trip", "The document root name survives a save/load round trip", Loaded.GetName(), MString("Config")) && Passed;

    MXmlAttribute* Version = Loaded.GetAttribute("version");
    Passed = EvaluateTrue("GetAttribute(name)", "round trip", "Root attributes survive a save/load round trip", Version != 0) && Passed;
    if (Version != 0) {
      Passed = Evaluate("GetAttribute(name)", "round trip", "Root attribute values survive a save/load round trip", Version->GetValueAsString(), MString("2")) && Passed;
    }

    MXmlNode* NameNode = Loaded.GetNode("Name");
    Passed = EvaluateTrue("GetNode(name)", "round trip", "Simple text nodes survive a save/load round trip", NameNode != 0) && Passed;
    if (NameNode != 0) {
      Passed = Evaluate("GetNode(name)", "round trip", "Simple text node values survive a save/load round trip", NameNode->GetValueAsString(), MString("Detector")) && Passed;
    }

    MXmlNode* EnabledNode = Loaded.GetNode("Enabled");
    if (EnabledNode != 0) {
      Passed = EvaluateTrue("GetValueAsBoolean()", "round trip", "Boolean nodes survive a save/load round trip", EnabledNode->GetValueAsBoolean()) && Passed;
    } else {
      Passed = EvaluateTrue("GetValueAsBoolean()", "round trip", "Boolean nodes survive a save/load round trip", false) && Passed;
    }

    MXmlNode* ThresholdNode = Loaded.GetNode("Threshold");
    if (ThresholdNode != 0) {
      Passed = EvaluateNear("GetValueAsDouble()", "round trip", "Double nodes survive a save/load round trip", ThresholdNode->GetValueAsDouble(), 12.5, 1e-12) && Passed;
    } else {
      Passed = EvaluateTrue("GetValueAsDouble()", "round trip", "Double nodes survive a save/load round trip", false) && Passed;
    }

    MXmlNode* BoundsNode = Loaded.GetNode("Bounds");
    if (BoundsNode != 0) {
      Passed = EvaluateNear("GetMinValueAsDouble()", "round trip", "Double min/max nodes preserve their minimum value", BoundsNode->GetMinValueAsDouble(), 1.25, 1e-12) && Passed;
      Passed = EvaluateNear("GetMaxValueAsDouble()", "round trip", "Double min/max nodes preserve their maximum value", BoundsNode->GetMaxValueAsDouble(), 9.5, 1e-12) && Passed;
    } else {
      Passed = EvaluateTrue("GetMinValueAsDouble()", "round trip", "Double min/max nodes preserve their minimum value", false) && Passed;
      Passed = EvaluateTrue("GetMaxValueAsDouble()", "round trip", "Double min/max nodes preserve their maximum value", false) && Passed;
    }

    MXmlNode* TimeWindowNode = Loaded.GetNode("TimeWindow");
    if (TimeWindowNode != 0) {
      Passed = EvaluateTrue("GetMinValueAsTime()", "round trip", "Time min/max nodes preserve their minimum value", TimeWindowNode->GetMinValueAsTime().GetLongIntsString() == "3.400000000") && Passed;
      Passed = EvaluateTrue("GetMaxValueAsTime()", "round trip", "Time min/max nodes preserve their maximum value", TimeWindowNode->GetMaxValueAsTime().GetLongIntsString() == "7.800000000") && Passed;
    } else {
      Passed = EvaluateTrue("GetMinValueAsTime()", "round trip", "Time min/max nodes preserve their minimum value", false) && Passed;
      Passed = EvaluateTrue("GetMaxValueAsTime()", "round trip", "Time min/max nodes preserve their maximum value", false) && Passed;
    }

    MXmlNode* LoadedGroup = Loaded.GetNode("Group");
    Passed = EvaluateTrue("GetNode(name)", "round trip nested", "Nested groups can be loaded again", LoadedGroup != 0) && Passed;
    if (LoadedGroup != 0) {
      MXmlAttribute* Kind = LoadedGroup->GetAttribute("kind");
      Passed = EvaluateTrue("GetAttribute(name)", "round trip nested", "Nested attributes survive a save/load round trip", Kind != 0) && Passed;
      if (Kind != 0) {
        Passed = Evaluate("GetAttribute(name)", "round trip nested", "Nested attribute values survive a save/load round trip", Kind->GetValueAsString(), MString("primary")) && Passed;
      }
      MXmlNode* Item = LoadedGroup->GetNode("Item");
      Passed = EvaluateTrue("GetNode(name)", "round trip nested", "Repeated nested node names remain accessible by name", Item != 0) && Passed;
      if (Item != 0) {
        Passed = Evaluate("GetNode(name)", "round trip nested", "The first repeated nested node keeps its value", Item->GetValueAsString(), MString("A")) && Passed;
      }
      MXmlNode* PositionNode = LoadedGroup->GetNode("Position");
      Passed = EvaluateTrue("GetNode(name)", "round trip nested vector", "Nested vector nodes can be retrieved again", PositionNode != 0) && Passed;
      if (PositionNode != 0) {
        MVector LoadedPosition = PositionNode->GetValueAsVector();
        Passed = EvaluateNear("GetValueAsVector()", "round trip nested x", "Nested vector nodes preserve X", LoadedPosition.X(), -1.0, 1e-12) && Passed;
        Passed = EvaluateNear("GetValueAsVector()", "round trip nested y", "Nested vector nodes preserve Y", LoadedPosition.Y(), 0.5, 1e-12) && Passed;
        Passed = EvaluateNear("GetValueAsVector()", "round trip nested z", "Nested vector nodes preserve Z", LoadedPosition.Z(), 2.25, 1e-12) && Passed;
      }
    }

    MXmlNode* ExtraGroupNode = Loaded.GetNode("ExtraGroup");
    Passed = EvaluateTrue("GetNode(name)", "round trip extra group", "Additional sibling groups survive a save/load round trip", ExtraGroupNode != 0) && Passed;
    if (ExtraGroupNode != 0) {
      Passed = Evaluate("GetNode(name)", "round trip extra group", "Additional sibling group values survive a save/load round trip", ExtraGroupNode->GetNode("Label")->GetValueAsString(), MString("Inner")) && Passed;
    }

    Passed = EvaluateNear("GetNNodes()", "round trip root count", "The root node count survives round-tripping", Loaded.GetNNodes(), 7.0, 1e-12) && Passed;
    Passed = EvaluateTrue("ToString()", "round trip text", "Serializing a loaded document still contains nested content", Loaded.ToString().Contains("<Position>")) && Passed;
  } else {
    Passed = Evaluate("GetName()", "round trip", "The document root name survives a save/load round trip", Loaded.GetName(), MString("Config")) && Passed;
  }

  ::remove(FileName.Data());

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test settings-style repeated loading on the same document
bool UTXml::TestXmlReload()
{
  bool Passed = true;

  MXmlDocument First(MString("Settings"));
  new MXmlAttribute(&First, MString("version"), MString("1"));
  new MXmlNode(&First, MString("GeometryFileName"), MString("first.geo.setup"));
  MXmlNode* FirstHistory = new MXmlNode(&First, MString("DataFileHistory"));
  new MXmlNode(FirstHistory, MString("DataFileHistoryItem"), MString("first.tra"));
  new MXmlNode(FirstHistory, MString("DataFileHistoryItem"), MString("second.tra"));

  MXmlDocument Second(MString("Settings"));
  new MXmlAttribute(&Second, MString("version"), MString("2"));
  new MXmlNode(&Second, MString("GeometryFileName"), MString("second.geo.setup"));
  new MXmlNode(&Second, MString("DataFileName"), MString("current.tra"));

  MString FirstFileName = "/tmp/UTXml_reload_first.xml";
  MString SecondFileName = "/tmp/UTXml_reload_second.xml";
  Passed = EvaluateTrue("Save()", "reload first", "The first settings-style xml document can be saved", First.Save(FirstFileName)) && Passed;
  Passed = EvaluateTrue("Save()", "reload second", "The second settings-style xml document can be saved", Second.Save(SecondFileName)) && Passed;

  MXmlDocument Loaded;
  Passed = EvaluateTrue("Load()", "reload first", "The first settings-style xml document can be loaded", Loaded.Load(FirstFileName)) && Passed;
  Passed = EvaluateTrue("Load()", "reload second", "The same document object can be reused for a second load", Loaded.Load(SecondFileName)) && Passed;
  Passed = Evaluate("GetAttribute(name)", "reload second", "Reloading updates root attributes instead of keeping stale ones", Loaded.GetAttribute("version")->GetValueAsString(), MString("2")) && Passed;
  Passed = Evaluate("GetNode(name)", "reload second", "Reloading updates existing node values", Loaded.GetNode("GeometryFileName")->GetValueAsString(), MString("second.geo.setup")) && Passed;
  Passed = Evaluate("GetNode(name)", "reload second", "Reloading keeps new nodes from the replacement document", Loaded.GetNode("DataFileName")->GetValueAsString(), MString("current.tra")) && Passed;
  Passed = EvaluateTrue("GetNode(name)", "reload second", "Reloading clears nodes that only existed in the first document", Loaded.GetNode("DataFileHistory") == 0) && Passed;
  Passed = EvaluateNear("GetNNodes()", "reload second", "Reloading replaces the root child set instead of accumulating it", Loaded.GetNNodes(), 2.0, 1e-12) && Passed;

  ::remove(FirstFileName.Data());
  ::remove(SecondFileName.Data());

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTXml Test;
  return Test.Run() == true ? 0 : 1;
}
