/*
 * UTXmlDocument.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdio>
#include <fstream>
using namespace std;

// MEGAlib:
#include "MStreams.h"
#include "MUnitTest.h"
#include "MXmlAttribute.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"


//! Unit test class for MXmlDocument
class UTXmlDocument : public MUnitTest
{
public:
  UTXmlDocument() : MUnitTest("UTXmlDocument") {}
  virtual ~UTXmlDocument() {}

  virtual bool Run();

private:
  bool WriteTextFile(const MString& FileName, const MString& Content) const;
  bool TestXmlRoundTrip();
  bool TestXmlReload();
  bool TestXmlLoadEdges();
  bool TestXmlRegressionBugs();
};


////////////////////////////////////////////////////////////////////////////////


bool UTXmlDocument::Run()
{
  bool Passed = true;

  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestXmlReload() && Passed;
  Passed = TestXmlLoadEdges() && Passed;
  Passed = TestXmlRegressionBugs() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTXmlDocument::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out<<Content;
  Out.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTXmlDocument::TestXmlRoundTrip()
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

  MString FileName = "/tmp/UTXmlDocument_roundtrip.xml";
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
    MString ExpectedLoadedText =
      "<Config version=\"2\">\n"
      "  <Name>Detector</Name>\n"
      "  <Enabled>true</Enabled>\n"
      "  <Threshold>12.5</Threshold>\n"
      "  <Bounds>\n"
      "    <Min>1.25</Min>\n"
      "    <Max>9.5</Max>\n"
      "  </Bounds>\n"
      "  <TimeWindow>\n"
      "    <Min>3.400000000</Min>\n"
      "    <Max>7.800000000</Max>\n"
      "  </TimeWindow>\n"
      "  <Group kind=\"primary\">\n"
      "    <Item>A</Item>\n"
      "    <Item>B</Item>\n"
      "    <Position>\n"
      "      <X>-1</X>\n"
      "      <Y>0.5</Y>\n"
      "      <Z>2.25</Z>\n"
      "    </Position>\n"
      "  </Group>\n"
      "  <ExtraGroup>\n"
      "    <Item>Nested</Item>\n"
      "    <Label>Inner</Label>\n"
      "  </ExtraGroup>\n"
      "</Config>";
    Passed = Evaluate("ToString()", "round trip exact text", "Serializing a loaded document reproduces the exact deterministic XML text", Loaded.ToString(), ExpectedLoadedText) && Passed;
    Passed = EvaluateTrue("ToString()", "round trip text", "Serializing a loaded document still contains nested content", Loaded.ToString().Contains("<Position>")) && Passed;
  } else {
    Passed = Evaluate("GetName()", "round trip", "The document root name survives a save/load round trip", Loaded.GetName(), MString("Config")) && Passed;
  }

  ::remove(FileName.Data());

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTXmlDocument::TestXmlReload()
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

  MString FirstFileName = "/tmp/UTXmlDocument_reload_first.xml";
  MString SecondFileName = "/tmp/UTXmlDocument_reload_second.xml";
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


bool UTXmlDocument::TestXmlLoadEdges()
{
  bool Passed = true;

  MXmlDocument Default;
  Passed = Evaluate("GetName()", "default constructor", "The default xml document name is empty", Default.GetName(), MString("")) && Passed;
  Passed = Evaluate("GetNNodes()", "default constructor", "The default xml document has no child nodes", Default.GetNNodes(), 0U) && Passed;

  {
    MXmlDocument Missing;
    mout.Enable(false);
    Passed = Evaluate("Load()", "missing file", "Loading a missing XML file fails cleanly", Missing.Load("/tmp/UTXmlDocument_does_not_exist.xml"), false) && Passed;
    mout.Enable(true);
  }

  {
    MString EmptyFileName = "/tmp/UTXmlDocument_empty.xml";
    Passed = EvaluateTrue("WriteTextFile()", "empty file", "An empty XML file can be written", WriteTextFile(EmptyFileName, "")) && Passed;
    MXmlDocument Empty;
    Passed = Evaluate("Load()", "empty file", "Loading an existing empty XML file currently succeeds", Empty.Load(EmptyFileName), true) && Passed;
    Passed = Evaluate("GetName()", "empty file", "Loading an empty XML file leaves the root name empty", Empty.GetName(), MString("")) && Passed;
    Passed = Evaluate("GetNNodes()", "empty file", "Loading an empty XML file leaves the document without child nodes", Empty.GetNNodes(), 0U) && Passed;
    ::remove(EmptyFileName.Data());
  }

  {
    MString BrokenAttributeFileName = "/tmp/UTXmlDocument_broken_attribute.xml";
    Passed = EvaluateTrue("WriteTextFile()", "broken attribute file", "A malformed XML file with a broken root attribute can be written", WriteTextFile(BrokenAttributeFileName, "<Config version=2></Config>\n")) && Passed;
    MXmlDocument Broken;
    mout.Enable(false);
    Passed = Evaluate("Load()", "broken attribute file", "Loading XML with malformed root attributes fails", Broken.Load(BrokenAttributeFileName), false) && Passed;
    mout.Enable(true);
    ::remove(BrokenAttributeFileName.Data());
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTXmlDocument::TestXmlRegressionBugs()
{
  bool Passed = true;

  {
    MXmlDocument Document("Config");
    new MXmlNode(&Document, "Text", MString("A&B<C>"));
    MString FileName = "/tmp/UTXmlDocument_special_text.xml";
    Passed = EvaluateTrue("Save()", "special text setup", "A representative document with XML-special text characters can be saved", Document.Save(FileName)) && Passed;

    MXmlDocument Loaded;
    Passed = Evaluate("Load()", "special text round trip", "Documents written with XML-special text characters should load successfully", Loaded.Load(FileName), true) && Passed;
    if (Loaded.GetNode("Text") != 0) {
      Passed = Evaluate("GetValueAsString()", "special text round trip", "XML-special text characters should survive a save/load round trip unchanged", Loaded.GetNode("Text")->GetValueAsString(), MString("A&B<C>")) && Passed;
    } else {
      Passed = EvaluateTrue("GetNode(name)", "special text round trip", "The special-text node should still be present after loading", false) && Passed;
    }
    ::remove(FileName.Data());
  }

  {
    MXmlDocument Document("Config");
    new MXmlAttribute(&Document, "label", MString("A&B\"C"));
    MString FileName = "/tmp/UTXmlDocument_special_attribute.xml";
    Passed = EvaluateTrue("Save()", "special attribute setup", "A representative document with XML-special attribute characters can be saved", Document.Save(FileName)) && Passed;

    MXmlDocument Loaded;
    Passed = Evaluate("Load()", "special attribute round trip", "Documents written with XML-special attribute characters should load successfully", Loaded.Load(FileName), true) && Passed;
    if (Loaded.GetAttribute("label") != 0) {
      Passed = Evaluate("GetValueAsString()", "special attribute round trip", "XML-special attribute characters should survive a save/load round trip unchanged", Loaded.GetAttribute("label")->GetValueAsString(), MString("A&B\"C")) && Passed;
    } else {
      Passed = EvaluateTrue("GetAttribute(name)", "special attribute round trip", "The special attribute should still be present after loading", false) && Passed;
    }
    ::remove(FileName.Data());
  }

  {
    MXmlDocument Document("Config");
    MXmlNode* Outer = new MXmlNode(&Document, "A");
    new MXmlNode(Outer, "A", MString("Inner"));
    MString FileName = "/tmp/UTXmlDocument_nested_same_name.xml";
    Passed = EvaluateTrue("Save()", "nested same-name setup", "A representative document with nested same-name nodes can be saved", Document.Save(FileName)) && Passed;

    MXmlDocument Loaded;
    __merr.Enable(false);
    Passed = Evaluate("Load()", "nested same-name round trip", "Documents with nested same-name nodes should load successfully", Loaded.Load(FileName), true) && Passed;
    __merr.Enable(true);
    if (Loaded.GetNode("A") != 0 && Loaded.GetNode("A")->GetNode("A") != 0) {
      Passed = Evaluate("GetValueAsString()", "nested same-name round trip", "Nested same-name nodes should survive a save/load round trip", Loaded.GetNode("A")->GetNode("A")->GetValueAsString(), MString("Inner")) && Passed;
    } else {
      Passed = EvaluateTrue("GetNode(name)", "nested same-name round trip", "The nested same-name child node should still be present after loading", false) && Passed;
    }
    ::remove(FileName.Data());
  }

  {
    MString FileName = "/tmp/UTXmlDocument_parse_failure.xml";
    Passed = EvaluateTrue("WriteTextFile()", "parse failure setup", "A malformed XML file that triggers a node-parse failure can be written", WriteTextFile(FileName, "<Config><Child></Child>text<Other></Other></Config>\n")) && Passed;

    MXmlDocument Loaded;
    mout.Enable(false);
    __merr.Enable(false);
    Passed = Evaluate("Load()", "parse failure propagation", "Load should return false when node parsing fails", Loaded.Load(FileName), false) && Passed;
    __merr.Enable(true);
    mout.Enable(true);
    ::remove(FileName.Data());
  }

  {
    MXmlDocument Document("Config");
    new MXmlNode(&Document, "Name", "Value");
    Passed = Evaluate("Save()", "unwritable target", "Save should return false when the target file cannot be created", Document.Save("/tmp/UTXmlDocument_missing_directory/output.xml"), false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTXmlDocument Test;
  return Test.Run() == true ? 0 : 1;
}
