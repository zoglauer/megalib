/*
 * UTSettingsBasicFiles.cxx
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
#include "MExceptions.h"
#include "MFile.h"
#include "MSettingsBasicFiles.h"
#include "MUnitTest.h"
#include "MXmlDocument.h"


//! Test helper exposing protected MSettingsBasicFiles functionality
class UTSettingsBasicFiles_Test : public MSettingsBasicFiles
{
public:
  UTSettingsBasicFiles_Test() : MSettingsBasicFiles() {}
  virtual ~UTSettingsBasicFiles_Test() {}

  bool TestReadXml(MXmlNode* Node) { return ReadXml(Node); }
  bool TestWriteXml(MXmlNode* Node) { return WriteXml(Node); }
};


//! Unit test class for MSettingsBasicFiles
class UTSettingsBasicFiles : public MUnitTest
{
public:
  UTSettingsBasicFiles() : MUnitTest("UTSettingsBasicFiles") {}
  virtual ~UTSettingsBasicFiles() {}

  virtual bool Run();

private:
  bool PrepareTempDirectory() const;
  bool WriteTextFile(const MString& FileName, const MString& Content) const;

  bool TestDefaultsAndSetters();
  bool TestHistories();
  bool TestXmlRoundTrip();
  bool TestHistoryBounds();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::Run()
{
  bool Passed = true;

  Passed = TestDefaultsAndSetters() && Passed;
  Passed = TestHistories() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestHistoryBounds() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::PrepareTempDirectory() const
{
  MString Directory = "/tmp/UTSettingsBasicFiles";
  MString Command = "mkdir -p " + Directory;

  return system(Command.Data()) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  Out<<Content;
  Out.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::TestDefaultsAndSetters()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "defaults temp dir", "The temporary directory for settings-basic-files tests can be created", PrepareTempDirectory()) && Passed;

  UTSettingsBasicFiles_Test Settings;
  Passed = Evaluate("GetCurrentFileName()", "default", "The default current file name is empty", Settings.GetCurrentFileName(), MString("")) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "default", "The default geometry file name uses the MEGAlib dummy geometry", Settings.GetGeometryFileName(), MString("$(MEGALIB)/resource/examples/geomega/special/Dummy.geo.setup")) && Passed;
  Passed = Evaluate("GetNFileHistories()", "default", "The default file history is empty", Settings.GetNFileHistories(), 0U) && Passed;
  Passed = Evaluate("GetNGeometryHistories()", "default", "The default geometry history is empty", Settings.GetNGeometryHistories(), 0U) && Passed;

  MString DataFile = "/tmp/UTSettingsBasicFiles/current.tra";
  MString GeometryFile = "/tmp/UTSettingsBasicFiles/geom.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "data file", "A representative current data file can be written", WriteTextFile(DataFile, "data")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "geometry file", "A representative geometry file can be written", WriteTextFile(GeometryFile, "geometry")) && Passed;

  Passed = Evaluate("SetCurrentFileName()", "missing file", "SetCurrentFileName rejects missing files", Settings.SetCurrentFileName("/tmp/UTSettingsBasicFiles/missing.tra"), false) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "missing file", "A rejected current file does not change the stored file name", Settings.GetCurrentFileName(), MString("")) && Passed;

  Passed = Evaluate("SetGeometryFileName()", "missing file", "SetGeometryFileName rejects missing files", Settings.SetGeometryFileName("/tmp/UTSettingsBasicFiles/missing.geo.setup"), false) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "missing file", "A rejected geometry file does not change the stored geometry file name", Settings.GetGeometryFileName(), MString("$(MEGALIB)/resource/examples/geomega/special/Dummy.geo.setup")) && Passed;

  Passed = Evaluate("SetCurrentFileName()", "existing file", "SetCurrentFileName accepts an existing representative file", Settings.SetCurrentFileName(DataFile), true) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "existing file", "SetCurrentFileName stores the expanded file name", Settings.GetCurrentFileName(), DataFile) && Passed;
  Passed = Evaluate("GetModificationLevel(false)", "after current file", "Setting the current file raises the modification level", Settings.GetModificationLevel(false), 4) && Passed;

  Passed = Evaluate("SetGeometryFileName()", "existing file", "SetGeometryFileName accepts an existing representative geometry file", Settings.SetGeometryFileName(GeometryFile), true) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "existing file", "SetGeometryFileName stores the expanded geometry file name", Settings.GetGeometryFileName(), GeometryFile) && Passed;
  Passed = Evaluate("GetModificationLevel()", "after geometry file", "Setting the geometry file keeps the representative modification level", Settings.GetModificationLevel(), 4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::TestHistories()
{
  bool Passed = true;

  UTSettingsBasicFiles_Test Settings;

  Settings.AddFileHistory(g_StringNotDefined);
  Passed = Evaluate("GetNFileHistories()", "ignore undefined", "AddFileHistory ignores the undefined sentinel", Settings.GetNFileHistories(), 0U) && Passed;

  Settings.AddFileHistory("/tmp/a.tra");
  Settings.AddFileHistory("/tmp/b.tra");
  Settings.AddFileHistory("/tmp/a.tra");
  Passed = Evaluate("GetNFileHistories()", "deduplicate", "AddFileHistory keeps only unique file names", Settings.GetNFileHistories(), 2U) && Passed;
  Passed = Evaluate("GetFileHistoryAt()", "deduplicate first", "Re-adding a file moves it to the front of the history", Settings.GetFileHistoryAt(0), MString("/tmp/a.tra")) && Passed;
  Passed = Evaluate("GetFileHistoryAt()", "deduplicate second", "The older unique file remains after the moved entry", Settings.GetFileHistoryAt(1), MString("/tmp/b.tra")) && Passed;

  for (unsigned int i = 0; i < MSettingsBasicFiles::m_MaxHistory + 5; ++i) {
    Settings.AddFileHistory(MString("/tmp/file_") + i + ".tra");
  }
  Passed = Evaluate("GetNFileHistories()", "max history", "AddFileHistory caps the history length at m_MaxHistory", Settings.GetNFileHistories(), MSettingsBasicFiles::m_MaxHistory) && Passed;
  Passed = Evaluate("GetFileHistoryAt()", "max history front", "The newest file stays at the front after capping", Settings.GetFileHistoryAt(0), MString("/tmp/file_24.tra")) && Passed;
  Passed = Evaluate("GetFileHistoryAt()", "max history back", "The oldest retained file is the most recent one still inside the capped window", Settings.GetFileHistoryAt(MSettingsBasicFiles::m_MaxHistory - 1), MString("/tmp/file_5.tra")) && Passed;

  Settings.AddGeometryHistory(g_StringNotDefined);
  Passed = Evaluate("GetNGeometryHistories()", "ignore undefined", "AddGeometryHistory ignores the undefined sentinel", Settings.GetNGeometryHistories(), 0U) && Passed;

  Settings.AddGeometryHistory("/tmp/a.geo.setup");
  Settings.AddGeometryHistory("/tmp/b.geo.setup");
  Settings.AddGeometryHistory("/tmp/a.geo.setup");
  Passed = Evaluate("GetNGeometryHistories()", "deduplicate", "AddGeometryHistory keeps only unique geometry file names", Settings.GetNGeometryHistories(), 2U) && Passed;
  Passed = Evaluate("GetGeometryHistoryAt()", "deduplicate first", "Re-adding a geometry file moves it to the front", Settings.GetGeometryHistoryAt(0), MString("/tmp/a.geo.setup")) && Passed;
  Passed = Evaluate("GetGeometryHistoryAt()", "deduplicate second", "The older unique geometry file remains after the moved entry", Settings.GetGeometryHistoryAt(1), MString("/tmp/b.geo.setup")) && Passed;

  for (unsigned int i = 0; i < MSettingsBasicFiles::m_MaxHistory + 3; ++i) {
    Settings.AddGeometryHistory(MString("/tmp/geom_") + i + ".geo.setup");
  }
  Passed = Evaluate("GetNGeometryHistories()", "max history", "AddGeometryHistory caps the history length at m_MaxHistory", Settings.GetNGeometryHistories(), MSettingsBasicFiles::m_MaxHistory) && Passed;
  Passed = Evaluate("GetGeometryHistoryAt()", "max history front", "The newest geometry file stays at the front after capping", Settings.GetGeometryHistoryAt(0), MString("/tmp/geom_22.geo.setup")) && Passed;
  Passed = Evaluate("GetGeometryHistoryAt()", "max history back", "The oldest retained geometry file is the most recent one still inside the capped window", Settings.GetGeometryHistoryAt(MSettingsBasicFiles::m_MaxHistory - 1), MString("/tmp/geom_3.geo.setup")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::TestXmlRoundTrip()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "xml temp dir", "The temporary directory for settings-basic-files xml tests can be created", PrepareTempDirectory()) && Passed;

  MString DataFile = "/tmp/UTSettingsBasicFiles/xml_current.tra";
  MString GeometryFile = "/tmp/UTSettingsBasicFiles/xml_geom.geo.setup";
  MString OldDataFile = "/tmp/UTSettingsBasicFiles/xml_old.tra";
  MString OldGeometryFile = "/tmp/UTSettingsBasicFiles/xml_old.geo.setup";
  Passed = EvaluateTrue("WriteTextFile()", "xml data file", "The representative XML current data file can be written", WriteTextFile(DataFile, "data")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "xml geometry file", "The representative XML geometry file can be written", WriteTextFile(GeometryFile, "geometry")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "xml old data file", "The representative old data file can be written", WriteTextFile(OldDataFile, "olddata")) && Passed;
  Passed = EvaluateTrue("WriteTextFile()", "xml old geometry file", "The representative old geometry file can be written", WriteTextFile(OldGeometryFile, "oldgeometry")) && Passed;

  UTSettingsBasicFiles_Test Settings;
  Settings.SetCurrentFileName(DataFile);
  Settings.SetGeometryFileName(GeometryFile);
  Settings.AddFileHistory(OldDataFile);
  Settings.AddGeometryHistory(OldGeometryFile);

  MXmlDocument Document("Settings");
  Passed = Evaluate("WriteXml()", "direct xml", "WriteXml succeeds on a representative XML document", Settings.TestWriteXml(&Document), true) && Passed;
  Passed = Evaluate("GetNode()", "direct xml geometry", "WriteXml stores the representative geometry file name", Document.GetNode("GeometryFileName")->GetValueAsString(), GeometryFile) && Passed;
  Passed = Evaluate("GetNode()", "direct xml data", "WriteXml stores the representative current data file name", Document.GetNode("DataFileName")->GetValueAsString(), DataFile) && Passed;
  Passed = Evaluate("GetNode()", "direct xml data history", "WriteXml stores the newest data history item first", Document.GetNode("DataFileHistory")->GetNode(0)->GetValueAsString(), OldDataFile) && Passed;
  Passed = Evaluate("GetNode()", "direct xml geometry history", "WriteXml stores the newest geometry history item first", Document.GetNode("GeometryFileHistory")->GetNode(0)->GetValueAsString(), OldGeometryFile) && Passed;

  MXmlDocument ReadDocument("Settings");
  new MXmlNode(&ReadDocument, "GeometryFileName", GeometryFile);
  new MXmlNode(&ReadDocument, "DataFileName", DataFile);
  MXmlNode* DataHistory = new MXmlNode(&ReadDocument, "DataFileHistory");
  new MXmlNode(DataHistory, "DataFileHistoryItem", DataFile);
  new MXmlNode(DataHistory, "DataFileHistoryItem", "/tmp/UTSettingsBasicFiles/does_not_exist.tra");
  MXmlNode* GeometryHistory = new MXmlNode(&ReadDocument, "GeometryFileHistory");
  new MXmlNode(GeometryHistory, "GeometryFileHistoryItem", GeometryFile);
  new MXmlNode(GeometryHistory, "GeometryFileHistoryItem", "/tmp/UTSettingsBasicFiles/does_not_exist.geo.setup");

  UTSettingsBasicFiles_Test ReadBack;
  Passed = Evaluate("ReadXml()", "direct xml", "ReadXml accepts a representative XML document", ReadBack.TestReadXml(&ReadDocument), true) && Passed;
  Passed = Evaluate("GetCurrentFileName()", "direct xml", "ReadXml restores the representative current data file name", ReadBack.GetCurrentFileName(), DataFile) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "direct xml", "ReadXml restores the representative geometry file name", ReadBack.GetGeometryFileName(), GeometryFile) && Passed;
  Passed = Evaluate("GetNFileHistories()", "direct xml", "ReadXml keeps only existing data-history files", ReadBack.GetNFileHistories(), 1U) && Passed;
  Passed = Evaluate("GetFileHistoryAt()", "direct xml", "ReadXml restores the existing data-history item", ReadBack.GetFileHistoryAt(0), DataFile) && Passed;
  Passed = Evaluate("GetNGeometryHistories()", "direct xml", "ReadXml keeps only existing geometry-history files", ReadBack.GetNGeometryHistories(), 1U) && Passed;
  Passed = Evaluate("GetGeometryHistoryAt()", "direct xml", "ReadXml restores the existing geometry-history item", ReadBack.GetGeometryHistoryAt(0), GeometryFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsBasicFiles::TestHistoryBounds()
{
  bool Passed = true;

  {
    UTSettingsBasicFiles_Test Settings;
    MString ExceptionText = "";
    try {
      Settings.GetFileHistoryAt(0);
    } catch (MExceptionIndexOutOfBounds& E) {
      ExceptionText = E.what();
    }
    Passed = Evaluate("GetFileHistoryAt()", "out of bounds message", "Out-of-bounds file-history access throws the expected index exception", ExceptionText, MString("Index out of bounds - allowed: [0..0[ - you are trying to access with index: 0\n")) && Passed;
  }

  {
    UTSettingsBasicFiles_Test Settings;
    MString ExceptionText = "";
    try {
      Settings.GetGeometryHistoryAt(0);
    } catch (MExceptionIndexOutOfBounds& E) {
      ExceptionText = E.what();
    }
    Passed = Evaluate("GetGeometryHistoryAt()", "out of bounds message", "Out-of-bounds geometry-history access throws the expected index exception", ExceptionText, MString("Index out of bounds - allowed: [0..0[ - you are trying to access with index: 0\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsBasicFiles Test;
  return Test.Run() == true ? 0 : 1;
}
