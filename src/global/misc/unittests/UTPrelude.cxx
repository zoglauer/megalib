/*
 * UTPrelude.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <filesystem>

// MEGAlib:
#include "MFile.h"
#include "MPrelude.h"
#include "MUnitTest.h"


//! Unit test class for MPrelude
class UTPrelude : public MUnitTest
{
public:
  //! Default constructor
  UTPrelude() : MUnitTest("UTPrelude") {}
  //! Default destructor
  virtual ~UTPrelude() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Restore an environment variable after the test
  class ScopedEnvironment
  {
  public:
    ScopedEnvironment(const char* Name, const MString& Value) : m_Name(Name), m_HadValue(false) {
      const char* OldValue = getenv(Name);
      if (OldValue != nullptr) {
        m_HadValue = true;
        m_OldValue = OldValue;
      }
      setenv(Name, Value.Data(), 1);
    }

    ~ScopedEnvironment() {
      if (m_HadValue == true) {
        setenv(m_Name.Data(), m_OldValue.Data(), 1);
      } else {
        unsetenv(m_Name.Data());
      }
    }

  private:
    MString m_Name;
    MString m_OldValue;
    bool m_HadValue;
  };

};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPrelude::Run()
{
  bool Passed = true;

  const MString HomeDirectory = GetTemporaryDirectoryName("Home");
  const MString MegaDirectory = GetTemporaryDirectoryName("Mega");
  const MString FailureHomeDirectory = GetTemporaryDirectoryName("FailureHome");
  const MString FailureMegaDirectory = GetTemporaryDirectoryName("FailureMega");
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "home", "A temporary HOME directory can be created for MPrelude tests", PrepareTemporaryDirectory("Home")) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "megalib", "A temporary MEGALIB directory can be created for MPrelude tests", PrepareTemporaryDirectory("Mega")) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "failure home", "A temporary failure HOME directory can be created for MPrelude tests", PrepareTemporaryDirectory("FailureHome")) && Passed;
  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "failure megalib", "A temporary failure MEGALIB directory can be created for MPrelude tests", PrepareTemporaryDirectory("FailureMega")) && Passed;
  if (MFile::CreateDirectory(HomeDirectory) == false || MFile::CreateDirectory(MegaDirectory) == false || MFile::CreateDirectory(FailureHomeDirectory) == false || MFile::CreateDirectory(FailureMegaDirectory) == false) {
    Summarize();
    return false;
  }

  const MString SettingsFile = HomeDirectory + "/.megalib.cfg";
  MFile::Remove(SettingsFile);

  MString FirstContent;
  {
    // Keep the temporary MEGALIB tree empty so the GUI-backed license and
    // changelog branches stay out of the headless unit test.
    ScopedEnvironment HomeEnv("HOME", HomeDirectory);
    ScopedEnvironment MegaEnv("MEGALIB", MegaDirectory);

    MPrelude Prelude;
    Passed = EvaluateTrue("Play()", "startup", "MPrelude::Play succeeds when no license or changelog files need prompting", Prelude.Play()) && Passed;
    Passed = EvaluateTrue("MFile::Exists()", "settings file", "MPrelude::Play creates the global settings file in the HOME directory", MFile::Exists(SettingsFile)) && Passed;

    FirstContent = ReadTextFile(SettingsFile);
    Passed = EvaluateTrue("Play()", "settings content", "MPrelude::Play writes the default global settings content", FirstContent.Contains("<LicenseHash>0</LicenseHash>") && FirstContent.Contains("<ChangeLogHash>0</ChangeLogHash>") && FirstContent.Contains("<FontScaler>normal</FontScaler>")) && Passed;

    MPrelude SecondPrelude;
    Passed = EvaluateTrue("Play()", "repeat startup", "A repeated MPrelude::Play call also succeeds with unchanged inputs", SecondPrelude.Play()) && Passed;
  }

  const MString FailureSettingsFile = FailureHomeDirectory + "/.megalib.cfg";
  MFile::Remove(FailureSettingsFile);
  Passed = EvaluateTrue("WriteTextFile()", "bad settings", "A malformed settings file can be created for the failure-path test", WriteTextFile(FailureSettingsFile, "<NotMEGAlib><LicenseHash>1</LicenseHash></NotMEGAlib>\n")) && Passed;
  {
    ScopedEnvironment HomeEnv("HOME", FailureHomeDirectory);
    ScopedEnvironment MegaEnv("MEGALIB", FailureMegaDirectory);

    MPrelude FailingPrelude;
    DisableDefaultStreams();
    Passed = EvaluateTrue("Play()", "bad settings", "MPrelude::Play recovers from a malformed settings file by starting with the defaults", FailingPrelude.Play()) && Passed;
    EnableDefaultStreams();
  }
  const MString FailureContent = ReadTextFile(FailureSettingsFile);
  Passed = EvaluateTrue("ReadTextFile()", "bad settings recovered", "MPrelude::Play rewrites a malformed settings file with the default configuration", FailureContent.Contains("<LicenseHash>0</LicenseHash>") && FailureContent.Contains("<ChangeLogHash>0</ChangeLogHash>") && FailureContent.Contains("<FontScaler>normal</FontScaler>")) && Passed;
  Passed = EvaluateFalse("ReadTextFile()", "bad settings recovered", "The malformed XML root is replaced during recovery", FailureContent.Contains("<NotMEGAlib>")) && Passed;

  const MString SecondContent = ReadTextFile(SettingsFile);
  Passed = Evaluate("Play()", "repeat content", "Repeated startup keeps the saved settings file stable", SecondContent, FirstContent) && Passed;

  Passed = EvaluateTrue("MFile::Remove()", "settings cleanup", "The temporary settings file can be removed", MFile::Remove(SettingsFile)) && Passed;
  Passed = EvaluateFalse("MFile::Exists()", "settings cleanup", "The temporary settings file is gone after cleanup", MFile::Exists(SettingsFile)) && Passed;
  MFile::Remove(FailureSettingsFile);

  Passed = EvaluateTrue("std::filesystem::remove()", "home cleanup", "The temporary HOME directory can be removed", std::filesystem::remove(HomeDirectory.Data())) && Passed;
  Passed = EvaluateTrue("std::filesystem::remove()", "megalib cleanup", "The temporary MEGALIB directory can be removed", std::filesystem::remove(MegaDirectory.Data())) && Passed;
  Passed = EvaluateTrue("std::filesystem::remove()", "failure home cleanup", "The temporary failure HOME directory can be removed", std::filesystem::remove(FailureHomeDirectory.Data())) && Passed;
  Passed = EvaluateTrue("std::filesystem::remove()", "failure megalib cleanup", "The temporary failure MEGALIB directory can be removed", std::filesystem::remove(FailureMegaDirectory.Data())) && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTPrelude Test;
  return Test.Run() == true ? 0 : 1;
}
