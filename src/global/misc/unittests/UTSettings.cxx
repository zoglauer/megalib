/*
 * UTSettings.cxx
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

// MEGAlib:
#include "MFile.h"
#include "MSettings.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MSettings
class UTSettings : public MUnitTest
{
public:
  UTSettings() : MUnitTest("UTSettings") {}
  virtual ~UTSettings() {}

  virtual bool Run();

  //! Test helper exposing protected MSettings functionality
  class SettingsTest : public MSettings
  {
  public:
    SettingsTest() : MSettings("UTSettingsRoot") {}
    virtual ~SettingsTest() {}

    bool TestReadXml(MXmlNode* Node) { return ReadXml(Node); }
    bool TestWriteXml(MXmlNode* Node) { return WriteXml(Node); }
    void SetDefaultSettingsFileName(const MString& FileName) { m_DefaultSettingsFileName = FileName; }
    void SetMasterNodeName(const MString& Name) { m_NameMasterNode = Name; }
    MString GetMasterNodeName() const { return m_NameMasterNode; }
  };
  //! Return the binary path used for child-process checks
  static MString& BinaryPath()
  {
    static MString Path;
    return Path;
  }

private:
  bool PrepareTempDirectory() const;
  bool WriteTextFile(const MString& FileName, const MString& Content) const;

  bool TestDefaultsAndSetters();
  bool TestXmlRoundTrip();
  bool TestReadWriteFiles();
  bool TestChange();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestReadWriteFiles() && Passed;
  Passed = TestChange() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::PrepareTempDirectory() const
{
  MString Directory = "/tmp/UTSettings";
  MString Command = "mkdir -p " + Directory;

  return system(Command.Data()) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out<<Content;
  Out.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::TestDefaultsAndSetters()
{
  bool Passed = true;

  SettingsTest Settings;
  Passed = Evaluate("GetMasterNodeName()", "default", "The representative test settings use the configured master node name", Settings.GetMasterNodeName(), MString("UTSettingsRoot")) && Passed;
  Passed = Evaluate("GetVersion()", "default", "The default settings version is initialized to 2", Settings.GetVersion(), 2U) && Passed;
  Passed = Evaluate("GetMEGAlibVersion()", "default", "The default MEGAlib version tracks the current global version", Settings.GetMEGAlibVersion(), static_cast<unsigned int>(g_Version)) && Passed;
  Passed = Evaluate("GetSettingsFileName()", "default", "The settings file name starts empty before the first read", Settings.GetSettingsFileName(), MString("")) && Passed;

  Settings.SetSettingsFileName("/tmp/UTSettings/manual.cfg");
  Passed = Evaluate("SetSettingsFileName()", "set", "SetSettingsFileName stores the representative path", Settings.GetSettingsFileName(), MString("/tmp/UTSettings/manual.cfg")) && Passed;

  Settings.SetVersion(17);
  Settings.SetMEGAlibVersion(23);
  Passed = Evaluate("SetVersion()", "set", "SetVersion stores a representative version number", Settings.GetVersion(), 17U) && Passed;
  Passed = Evaluate("SetMEGAlibVersion()", "set", "SetMEGAlibVersion stores a representative MEGAlib version", Settings.GetMEGAlibVersion(), 23U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::TestXmlRoundTrip()
{
  bool Passed = true;

  MString DataFile = "/tmp/UTSettings/xml_current.tra";
  MString GeometryFile = "/tmp/UTSettings/xml_geom.geo.setup";
  PrepareTempDirectory();
  WriteTextFile(DataFile, "data");
  WriteTextFile(GeometryFile, "geometry");

  SettingsTest Settings;
  Settings.SetVersion(19);
  Settings.SetMEGAlibVersion(77);
  Settings.SetCurrentFileName(DataFile);
  Settings.SetGeometryFileName(GeometryFile);

  MXmlDocument Document("UTSettingsRoot");
  Passed = Evaluate("WriteXml()", "direct xml", "WriteXml succeeds on a representative XML document", Settings.TestWriteXml(&Document), true) && Passed;
  Passed = Evaluate("GetNode()", "version", "WriteXml stores the representative settings version", static_cast<unsigned int>(Document.GetNode("Version")->GetValueAsInt()), 19U) && Passed;
  Passed = Evaluate("GetNode()", "megalib version", "WriteXml stores the current global MEGAlib version", static_cast<unsigned int>(Document.GetNode("MEGAlibVersion")->GetValueAsInt()), static_cast<unsigned int>(g_Version)) && Passed;
  Passed = Evaluate("GetNode()", "current file", "WriteXml includes the inherited current file name", Document.GetNode("DataFileName")->GetValueAsString(), DataFile) && Passed;
  Passed = Evaluate("GetNode()", "geometry file", "WriteXml includes the inherited geometry file name", Document.GetNode("GeometryFileName")->GetValueAsString(), GeometryFile) && Passed;

  MXmlDocument ReadDocument("UTSettingsRoot");
  new MXmlNode(&ReadDocument, "Version", 5);
  new MXmlNode(&ReadDocument, "MEGAlibVersion", 9);
  new MXmlNode(&ReadDocument, "DataFileName", DataFile);
  new MXmlNode(&ReadDocument, "GeometryFileName", GeometryFile);

  SettingsTest ReadBack;
  Passed = Evaluate("ReadXml()", "direct xml", "ReadXml accepts a representative XML document", ReadBack.TestReadXml(&ReadDocument), true) && Passed;
  Passed = Evaluate("GetVersion()", "direct xml", "ReadXml restores the representative settings version", ReadBack.GetVersion(), 5U) && Passed;
  Passed = Evaluate("GetMEGAlibVersion()", "direct xml", "ReadXml restores the representative MEGAlib version", ReadBack.GetMEGAlibVersion(), 9U) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "direct xml", "ReadXml restores the inherited current file name", ReadBack.GetCurrentFileName(), DataFile) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "direct xml", "ReadXml restores the inherited geometry file name", ReadBack.GetGeometryFileName(), GeometryFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::TestReadWriteFiles()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "file temp dir", "The temporary directory for settings file tests can be created", PrepareTempDirectory()) && Passed;

  MString DataFile = "/tmp/UTSettings/file_current.tra";
  MString GeometryFile = "/tmp/UTSettings/file_geom.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "file current", "A representative current data file can be written", WriteTextFile(DataFile, "data")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "file geometry", "A representative geometry file can be written", WriteTextFile(GeometryFile, "geometry")) && Passed;

  MString ExplicitFile = "/tmp/UTSettings/explicit_settings";
  MString DefaultFile = "/tmp/UTSettings/default_settings.cfg";
  MString WrongRootFile = "/tmp/UTSettings/wrong_root.cfg";
  MString NonXmlFile = "/tmp/UTSettings/non_xml.cfg";
  MString EmptyFile = "/tmp/UTSettings/empty.cfg";

  SettingsTest Settings;
  Settings.SetCurrentFileName(DataFile);
  Settings.SetGeometryFileName(GeometryFile);
  Settings.SetVersion(21);
  Settings.SetMEGAlibVersion(88);
  Settings.SetDefaultSettingsFileName(DefaultFile);

  Passed = Evaluate("Write(MString)", "explicit file", "Write(MString) succeeds for a representative explicit file without suffix", Settings.Write(ExplicitFile), true) && Passed;
  Passed = EvaluateTrue("Exists()", "explicit file", "Write(MString) appends .cfg when needed", MFile::Exists(ExplicitFile + ".cfg")) && Passed;

  Passed = Evaluate("Write(MString)", "empty file name fallback", "Write(MString) falls back to the configured default file name when given an empty file name", Settings.Write(""), true) && Passed;
  Passed = EvaluateTrue("Exists()", "empty file name fallback", "Write(MString) with an empty file name writes the configured default file", MFile::Exists(DefaultFile)) && Passed;

  Passed = Evaluate("Write(MString)", "undefined file name fallback", "Write(MString) falls back to the configured default file name when given g_StringNotDefined", Settings.Write(g_StringNotDefined), true) && Passed;
  Passed = EvaluateTrue("Exists()", "undefined file name fallback", "Write(MString) with g_StringNotDefined writes the configured default file", MFile::Exists(DefaultFile)) && Passed;

  Passed = Evaluate("Write()", "default file", "Write() succeeds for the configured default settings file", Settings.Write(), true) && Passed;
  Passed = EvaluateTrue("Exists()", "default file", "Write() creates the configured default settings file", MFile::Exists(DefaultFile)) && Passed;

  SettingsTest ReadBack;
  Passed = Evaluate("Read(MString)", "explicit file", "Read(MString) restores a previously written representative settings file", ReadBack.Read(ExplicitFile + ".cfg"), true) && Passed;
  Passed = Evaluate("GetVersion()", "explicit file", "Read(MString) restores the stored settings version", ReadBack.GetVersion(), 21U) && Passed;
  Passed = Evaluate("GetMEGAlibVersion()", "explicit file", "Read(MString) restores the stored MEGAlib version", ReadBack.GetMEGAlibVersion(), static_cast<unsigned int>(g_Version)) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "explicit file", "Read(MString) restores the inherited current data file name", ReadBack.GetCurrentFileName(), DataFile) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "explicit file", "Read(MString) restores the inherited geometry file name", ReadBack.GetGeometryFileName(), GeometryFile) && Passed;

  SettingsTest MissingDefault;
  MissingDefault.SetDefaultSettingsFileName("/tmp/UTSettings/does_not_exist.cfg");
  DisableDefaultStreams();
  Passed = Evaluate("Read()", "missing default", "Read() on a missing default settings file fails cleanly", MissingDefault.Read(), false) && Passed;
  EnableDefaultStreams();

  SettingsTest ExplicitDefaultRead;
  ExplicitDefaultRead.SetDefaultSettingsFileName(DefaultFile);
  Passed = Evaluate("Read(MString)", "empty file name fallback", "Read(MString) falls back to the configured default settings file when given an empty file name", ExplicitDefaultRead.Read(""), true) && Passed;
  Passed = Evaluate("GetVersion()", "empty file name fallback", "Read(MString) with an empty file name reads from the configured default settings file", ExplicitDefaultRead.GetVersion(), 21U) && Passed;

  SettingsTest UndefinedDefaultRead;
  UndefinedDefaultRead.SetDefaultSettingsFileName(DefaultFile);
  Passed = Evaluate("Read(MString)", "undefined file name fallback", "Read(MString) falls back to the configured default settings file when given g_StringNotDefined", UndefinedDefaultRead.Read(g_StringNotDefined), true) && Passed;
  Passed = Evaluate("GetVersion()", "undefined file name fallback", "Read(MString) with g_StringNotDefined reads from the configured default settings file", UndefinedDefaultRead.GetVersion(), 21U) && Passed;

  Passed = EvaluateTrue("WriteTextFile()", "wrong root file", "A settings file with the wrong root node can be written", WriteTextFile(WrongRootFile, "<WrongRoot><Version>1</Version></WrongRoot>\n")) && Passed;
  SettingsTest WrongRoot;
  DisableDefaultStreams();
  Passed = Evaluate("Read(MString)", "wrong root", "Read(MString) rejects settings files with the wrong root node", WrongRoot.Read(WrongRootFile), false) && Passed;
  EnableDefaultStreams();

  Passed = EvaluateTrue("WriteTextFile()", "non xml file", "A legacy non-XML settings file can be written", WriteTextFile(NonXmlFile, "Version=1\n")) && Passed;
  SettingsTest NonXml;
  DisableDefaultStreams();
  Passed = Evaluate("Read(MString)", "non xml file", "Read(MString) treats legacy non-XML settings files as a default-state success", NonXml.Read(NonXmlFile), true) && Passed;
  EnableDefaultStreams();

  Passed = EvaluateTrue("WriteTextFile()", "empty file", "An empty settings file can be written", WriteTextFile(EmptyFile, "")) && Passed;
  SettingsTest Empty;
  DisableDefaultStreams();
  Passed = Evaluate("Read(MString)", "empty file", "Read(MString) rejects empty settings files", Empty.Read(EmptyFile), false) && Passed;
  EnableDefaultStreams();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettings::TestChange()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "change temp dir", "The temporary directory for change() tests can be created", PrepareTempDirectory()) && Passed;

  MString DataFile = "/tmp/UTSettings/change_current.tra";
  MString GeometryFile = "/tmp/UTSettings/change_geom.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "change current file", "A representative current file for change() can be written", WriteTextFile(DataFile, "data")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "change geometry file", "A representative geometry file for change() can be written", WriteTextFile(GeometryFile, "geometry")) && Passed;

  SettingsTest Settings;
  Settings.SetCurrentFileName(DataFile);
  Settings.SetGeometryFileName(GeometryFile);
  Settings.SetVersion(3);
  Settings.SetMEGAlibVersion(4);

  Passed = Evaluate("Change()", "version", "Change() can update the Version node via dotted-path syntax", Settings.Change("Version=11"), true) && Passed;
  Passed = Evaluate("GetVersion()", "version", "Change() updates the representative settings version", Settings.GetVersion(), 11U) && Passed;

  Passed = Evaluate("Change()", "MEGAlibVersion", "Change() can update the MEGAlibVersion node via dotted-path syntax", Settings.Change("MEGAlibVersion=15"), true) && Passed;
  Passed = Evaluate("GetMEGAlibVersion()", "MEGAlibVersion", "Change() updates the representative MEGAlib version", Settings.GetMEGAlibVersion(), 15U) && Passed;

  Passed = Evaluate("Change()", "data file", "Change() can update inherited basic-file nodes", Settings.Change(MString("DataFileName=") + DataFile), true) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "data file", "Change() updates the inherited current data file name", Settings.GetCurrentFileName(), DataFile) && Passed;

  {
    MString LogFileName = "/tmp/UTSettings_change_missing_node.log";
    MString Command = MString("\"") + BinaryPath() + "\" --change-missing-node > " + LogFileName + " 2>&1";
    int Status = system(Command.Data());
    Passed = EvaluateTrue("Change()", "missing node status", "Change() returns failure for a missing dotted-path node", Status == 0) && Passed;

    ifstream In(LogFileName.Data());
    string Content((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
    Passed = EvaluateTrue("Change()", "missing node message", "Change() reports the missing node path", MString(Content).Contains("Error: Unable to find node MissingNode")) && Passed;
  }

  {
    SettingsTest Malformed;
    Malformed.SetVersion(3);
    Passed = Evaluate("Change()", "missing equal status", "Change() currently accepts malformed input without an equals sign when the node itself exists", Malformed.Change("Version"), true) && Passed;
    Passed = Evaluate("GetVersion()", "missing equal result", "Malformed change input without an equals sign rewrites the Version node and reads back as zero", Malformed.GetVersion(), 0U) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  UTSettings::BinaryPath() = argc > 0 ? argv[0] : "";
  if (UTSettings::BinaryPath().IsEmpty() == true) {
    UTSettings::BinaryPath() = "bin/UTSettings";
  }

  if (argc >= 2) {
    MString Argument = argv[1];
    UTSettings::SettingsTest Settings;
    Settings.SetVersion(3);
    if (Argument == "--change-missing-node") {
      return Settings.Change("MissingNode.Value=7") == false ? 0 : 1;
    }
  }

  UTSettings Test;
  return Test.Run() == true ? 0 : 1;
}
