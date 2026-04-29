/*
 * UTSettingsGlobal.cxx
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
#include "MSettingsGlobal.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Test helper exposing protected MSettingsGlobal functionality
class UTSettingsGlobal_Test : public MSettingsGlobal
{
public:
  UTSettingsGlobal_Test() : MSettingsGlobal() {}
  virtual ~UTSettingsGlobal_Test() {}

  bool TestReadXml(MXmlNode* Node) { return ReadXml(Node); }
  bool TestWriteXml(MXmlNode* Node) { return WriteXml(Node); }
  void SetTestSettingsFileName(const MString& FileName) { m_SettingsFileName = FileName; }
  MString GetTestSettingsFileName() const { return m_SettingsFileName; }
  void SetTestMasterNodeName(const MString& Name) { m_NameMasterNode = Name; }
  MString GetTestMasterNodeName() const { return m_NameMasterNode; }
};


//! Unit test class for MSettingsGlobal
class UTSettingsGlobal : public MUnitTest
{
public:
  UTSettingsGlobal() : MUnitTest("UTSettingsGlobal") {}
  virtual ~UTSettingsGlobal() {}

  virtual bool Run();

private:
  bool PrepareTempDirectory() const;
  bool WriteTextFile(const MString& FileName, const MString& Content) const;

  bool TestDefaultsAndSetters();
  bool TestXmlRoundTrip();
  bool TestReadWriteFiles();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestReadWriteFiles() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::PrepareTempDirectory() const
{
  MString Directory = "/tmp/UTSettingsGlobal";
  MString Command = "mkdir -p " + Directory;

  return system(Command.Data()) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out<<Content;
  Out.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::TestDefaultsAndSetters()
{
  bool Passed = true;

  UTSettingsGlobal_Test Settings;
  Passed = Evaluate("GetLicenseHash()", "default", "The default license hash is zero", Settings.GetLicenseHash(), 0L) && Passed;
  Passed = Evaluate("GetChangeLogHash()", "default", "The default changelog hash is zero", Settings.GetChangeLogHash(), 0L) && Passed;
  Passed = Evaluate("GetFontScaler()", "default", "The default font scaler is normal", Settings.GetFontScaler(), MString("normal")) && Passed;
  Passed = Evaluate("GetTestMasterNodeName()", "default", "The default global-settings master node is MEGAlib", Settings.GetTestMasterNodeName(), MString("MEGAlib")) && Passed;

  Settings.SetLicenseHash(1234);
  Settings.SetChangeLogHash(5678);
  Settings.SetFontScaler("gigantic");
  Passed = Evaluate("SetLicenseHash()", "set", "SetLicenseHash stores a representative value", Settings.GetLicenseHash(), 1234L) && Passed;
  Passed = Evaluate("SetChangeLogHash()", "set", "SetChangeLogHash stores a representative value", Settings.GetChangeLogHash(), 5678L) && Passed;
  Passed = Evaluate("SetFontScaler()", "set", "SetFontScaler stores a representative value", Settings.GetFontScaler(), MString("gigantic")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::TestXmlRoundTrip()
{
  bool Passed = true;

  UTSettingsGlobal_Test Settings;
  Settings.SetLicenseHash(11);
  Settings.SetChangeLogHash(22);
  Settings.SetFontScaler("huge");

  MXmlDocument Document("MEGAlib");
  Passed = Evaluate("WriteXml()", "direct xml", "WriteXml succeeds on a representative XML document", Settings.TestWriteXml(&Document), true) && Passed;
  Passed = Evaluate("GetNode()", "license", "WriteXml stores the representative license hash", static_cast<long>(Document.GetNode("LicenseHash")->GetValueAsLong()), 11L) && Passed;
  Passed = Evaluate("GetNode()", "changelog", "WriteXml stores the representative changelog hash", static_cast<long>(Document.GetNode("ChangeLogHash")->GetValueAsLong()), 22L) && Passed;
  Passed = Evaluate("GetNode()", "font scaler", "WriteXml stores the representative font scaler", Document.GetNode("FontScaler")->GetValueAsString(), MString("huge")) && Passed;

  MXmlDocument ReadDocument("MEGAlib");
  new MXmlNode(&ReadDocument, "LicenseHash", 101L);
  new MXmlNode(&ReadDocument, "ChangeLogHash", 202L);
  new MXmlNode(&ReadDocument, "FontScaler", MString("large"));

  UTSettingsGlobal_Test ReadBack;
  Passed = Evaluate("ReadXml()", "direct xml", "ReadXml accepts a representative XML document", ReadBack.TestReadXml(&ReadDocument), true) && Passed;
  Passed = Evaluate("GetLicenseHash()", "direct xml", "ReadXml restores the representative license hash", ReadBack.GetLicenseHash(), 101L) && Passed;
  Passed = Evaluate("GetChangeLogHash()", "direct xml", "ReadXml restores the representative changelog hash", ReadBack.GetChangeLogHash(), 202L) && Passed;
  Passed = Evaluate("GetFontScaler()", "direct xml", "ReadXml restores the representative font scaler", ReadBack.GetFontScaler(), MString("large")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsGlobal::TestReadWriteFiles()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "file temp dir", "The temporary directory for global-settings file tests can be created", PrepareTempDirectory()) && Passed;

  MString SettingsFile = "/tmp/UTSettingsGlobal/global.cfg";
  MString WrongRootFile = "/tmp/UTSettingsGlobal/wrong_root.cfg";
  MString EmptyFile = "/tmp/UTSettingsGlobal/empty.cfg";

  UTSettingsGlobal_Test Settings;
  Settings.SetTestSettingsFileName(SettingsFile);
  Settings.SetLicenseHash(333);
  Settings.SetChangeLogHash(444);
  Settings.SetFontScaler("huge");
  Passed = Evaluate("Write()", "representative file", "Write() stores the representative global settings file", Settings.Write(), true) && Passed;
  Passed = EvaluateTrue("Exists()", "representative file", "Write() creates the representative global settings file", MFile::Exists(SettingsFile)) && Passed;

  UTSettingsGlobal_Test ReadBack;
  ReadBack.SetTestSettingsFileName(SettingsFile);
  Passed = Evaluate("Read()", "representative file", "Read() restores a previously written representative file", ReadBack.Read(), true) && Passed;
  Passed = Evaluate("GetLicenseHash()", "representative file", "Read() restores the stored license hash", ReadBack.GetLicenseHash(), 333L) && Passed;
  Passed = Evaluate("GetChangeLogHash()", "representative file", "Read() restores the stored changelog hash", ReadBack.GetChangeLogHash(), 444L) && Passed;
  Passed = Evaluate("GetFontScaler()", "representative file", "Read() restores the stored font scaler", ReadBack.GetFontScaler(), MString("huge")) && Passed;

  UTSettingsGlobal_Test Missing;
  Missing.SetTestSettingsFileName("/tmp/UTSettingsGlobal/does_not_exist.cfg");
  Passed = Evaluate("Read()", "missing file", "Read() treats a missing global settings file as a clean default-state success", Missing.Read(), true) && Passed;
  Passed = Evaluate("GetLicenseHash()", "missing file", "A missing global settings file leaves the default license hash intact", Missing.GetLicenseHash(), 0L) && Passed;

  Passed = EvaluateTrue("WriteTextFile()", "wrong root file", "A global settings file with the wrong XML root can be written", WriteTextFile(WrongRootFile, "<NotMEGAlib><LicenseHash>1</LicenseHash></NotMEGAlib>\n")) && Passed;
  UTSettingsGlobal_Test WrongRoot;
  WrongRoot.SetTestSettingsFileName(WrongRootFile);
  DisableDefaultStreams();
  Passed = Evaluate("Read()", "wrong root", "Read() rejects global settings files with the wrong XML root", WrongRoot.Read(), false) && Passed;
  EnableDefaultStreams();

  Passed = EvaluateTrue("WriteTextFile()", "empty file", "An empty global settings file can be written", WriteTextFile(EmptyFile, "")) && Passed;
  UTSettingsGlobal_Test Empty;
  Empty.SetTestSettingsFileName(EmptyFile);
  DisableDefaultStreams();
  Passed = Evaluate("Read()", "empty file", "Read() rejects an existing empty global settings file", Empty.Read(), false) && Passed;
  EnableDefaultStreams();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsGlobal Test;
  return Test.Run() == true ? 0 : 1;
}
