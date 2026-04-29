/*
 * UTSettingsInterface.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MFile.h"
#include "MSettingsInterface.h"
#include "MUnitTest.h"
#include "MXmlDocument.h"


//! Unit test class for MSettingsInterface
class UTSettingsInterface : public MUnitTest
{
public:
  UTSettingsInterface() : MUnitTest("UTSettingsInterface") {}
  virtual ~UTSettingsInterface() {}

  virtual bool Run();

private:
  //! Concrete test helper for MSettingsInterface
  class SettingsInterfaceTest : public MSettingsInterface
  {
  public:
    SettingsInterfaceTest() : MSettingsInterface() {}
    virtual ~SettingsInterfaceTest() {}

    void TestModify(int Level) { Modify(Level); }
    MString TestCleanPath(MString Path) { return CleanPath(Path); }

  protected:
    virtual bool ReadXml(MXmlNode* Node) { return Node != nullptr; }
    virtual bool WriteXml(MXmlNode* Node) { return Node != nullptr; }
  };

  bool TestModificationLevel();
  bool TestCleanPath();
};


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsInterface::Run()
{
  bool Passed = true;

  Passed = TestModificationLevel() && Passed;
  Passed = TestCleanPath() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsInterface::TestModificationLevel()
{
  bool Passed = true;

  SettingsInterfaceTest Settings;
  Passed = Evaluate("GetModificationLevel()", "default", "The default modification level is zero", Settings.GetModificationLevel(false), 0) && Passed;

  Settings.TestModify(2);
  Passed = Evaluate("GetModificationLevel(false)", "after modify", "Modify stores a representative modification level", Settings.GetModificationLevel(false), 2) && Passed;

  Settings.TestModify(1);
  Passed = Evaluate("GetModificationLevel(false)", "lower modify ignored", "Modify does not decrease the modification level", Settings.GetModificationLevel(false), 2) && Passed;

  Settings.TestModify(5);
  Passed = Evaluate("GetModificationLevel()", "reset", "GetModificationLevel(true) returns the highest modification level before resetting", Settings.GetModificationLevel(), 5) && Passed;
  Passed = Evaluate("GetModificationLevel(false)", "after reset", "GetModificationLevel(true) resets the stored modification level", Settings.GetModificationLevel(false), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTSettingsInterface::TestCleanPath()
{
  bool Passed = true;

  SettingsInterfaceTest Settings;

  MString ExpandedMegalib = "$(MEGALIB)";
  Passed = EvaluateTrue("ExpandFileName()", "$(MEGALIB)", "The MEGAlib environment path can be expanded for the CleanPath test", MFile::ExpandFileName(ExpandedMegalib)) && Passed;

  Passed = Evaluate("CleanPath()", "outside megalib", "CleanPath leaves unrelated absolute paths unchanged", Settings.TestCleanPath("/tmp/outside/path.cfg"), MString("/tmp/outside/path.cfg")) && Passed;
  Passed = Evaluate("CleanPath()", "plain megalib path", "CleanPath replaces the expanded MEGAlib root with $(MEGALIB)", Settings.TestCleanPath(ExpandedMegalib + "/resource/libraries/Calibration.isotopes"), MString("$(MEGALIB)/resource/libraries/Calibration.isotopes")) && Passed;
  Passed = Evaluate("CleanPath()", "megalib suffix path", "CleanPath also normalizes supported MEGAlib clone suffixes", Settings.TestCleanPath(ExpandedMegalib + "_github/resource/test.dat"), MString("$(MEGALIB)/resource/test.dat")) && Passed;
  Passed = Evaluate("CleanPath()", "non-matching suffix", "CleanPath keeps paths unchanged when no supported replacement matches", Settings.TestCleanPath(ExpandedMegalib + "_custom/resource/test.dat"), ExpandedMegalib + "_custom/resource/test.dat") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTSettingsInterface Test;
  return Test.Run() == true ? 0 : 1;
}
