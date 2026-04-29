/*
 * UTFileEvents.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib:
#include "MFileEvents.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MFileEvents
class UTFileEvents : public MUnitTest
{
public:
  //! Default constructor
  UTFileEvents() : MUnitTest("UTFileEvents") {}
  //! Default destructor
  virtual ~UTFileEvents() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test helper exposing protected MFileEvents functionality
  class FileEventsTest : public MFileEvents
  {
  public:
    //! Default constructor
    FileEventsTest()
    {
      SetFileType("tra");
      SetVersion(7);
    }

    //! Default destructor
    virtual ~FileEventsTest() {}

    //! Open with a recursively compatible include helper
    virtual bool Open(MString FileName, unsigned int Way, bool IsBinary)
    {
      if (m_IncludeFile != nullptr) {
        delete m_IncludeFile;
        m_IncludeFile = nullptr;
      }

      FileEventsTest* Include = new FileEventsTest();
      Include->SetIsIncludeFile(true);
      Include->SetFileType(GetFileType());
      Include->SetVersion(GetVersion());
      m_IncludeFile = Include;
      m_IncludeFileUsed = false;

      return MFileEvents::Open(FileName, Way, IsBinary);
    }

    //! Open ASCII by default
    virtual bool Open(MString FileName, unsigned int Way)
    {
      return Open(FileName, Way, false);
    }

    //! Open for reading by default
    virtual bool Open(MString FileName)
    {
      return Open(FileName, MFile::c_Read, false);
    }

    //! Public wrapper for OpenNextFile
    bool TestOpenNextFile(const MString& Line) { return OpenNextFile(Line); }
    //! Public wrapper for OpenIncludeFile
    bool TestOpenIncludeFile(const MString& Line) { return OpenIncludeFile(Line); }
    //! Public wrapper for CreateNextFile
    bool TestCreateNextFile() { return CreateNextFile(); }
    //! Public wrapper for CreateIncludeFile
    bool TestCreateIncludeFile() { return CreateIncludeFile(); }
    //! Public wrapper for CreateIncludeFileName
    MString TestCreateIncludeFileName(const MString& FileName) { return CreateIncludeFileName(FileName); }
    //! Public wrapper for ReadFooter
    bool TestReadFooter(bool Continue = false) { return ReadFooter(Continue); }
    //! Public wrapper for CloseIncludeFile
    bool TestCloseIncludeFile() { return CloseIncludeFile(); }

    //! Expose the include file usage state
    bool IsIncludeFileUsed() const { return m_IncludeFileUsed; }
    //! Expose the include file pointer
    MFileEvents* GetIncludeFile() const { return m_IncludeFile; }
    //! Expose start observation flag
    bool HasStartObservationTime() const { return m_HasStartObservationTime; }
    //! Expose end observation flag
    bool HasEndObservationTime() const { return m_HasEndObservationTime; }
    //! Expose observation flag
    bool HasObservationTime() const { return m_HasObservationTime; }
    //! Expose start observation time
    MTime GetStartObservationTime() const { return m_StartObservationTime; }
    //! Expose end observation time
    MTime GetEndObservationTime() const { return m_EndObservationTime; }
    //! Expose the original file name
    MString GetOriginalFileName() const { return m_OriginalFileName; }
  };

  //! Test read-side metadata parsing and observation-time handling
  bool TestOpenAndMetadata();
  //! Test footer parsing variants used by derived event readers
  bool TestFooterParsing();
  //! Test write-side helpers
  bool TestWriting();
  //! Test file-tree helper methods
  bool TestFileTreeHelpers();
  //! Test event counting and rewind behavior
  bool TestCountingAndRewind();

  //! Return the temp test directory
  MString GetTempDirectory() const;
  //! Create a clean temp directory
  bool PrepareTempDirectory() const;
  //! Write ASCII text file content
  bool WriteTextFile(const MString& FileName, const MString& Content) const;
  //! Read ASCII text file content
  MString ReadTextFile(const MString& FileName) const;
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTFileEvents::Run()
{
  bool Passed = true;

  Passed = TestOpenAndMetadata() && Passed;
  Passed = TestFooterParsing() && Passed;
  Passed = TestWriting() && Passed;
  Passed = TestFileTreeHelpers() && Passed;
  Passed = TestCountingAndRewind() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the temp test directory
MString UTFileEvents::GetTempDirectory() const
{
  return "/tmp/UTFileEvents";
}


////////////////////////////////////////////////////////////////////////////////


//! Create a clean temp directory
bool UTFileEvents::PrepareTempDirectory() const
{
  gSystem->Exec(("rm -rf " + GetTempDirectory()).Data());
  return gSystem->mkdir(GetTempDirectory(), true) == 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Write ASCII text file content
bool UTFileEvents::WriteTextFile(const MString& FileName, const MString& Content) const
{
  ofstream Out(FileName.Data());
  Out<<Content;
  return Out.good();
}


////////////////////////////////////////////////////////////////////////////////


//! Read ASCII text file content
MString UTFileEvents::ReadTextFile(const MString& FileName) const
{
  ifstream In(FileName.Data());
  ostringstream Buffer;
  Buffer<<In.rdbuf();
  return Buffer.str().c_str();
}


////////////////////////////////////////////////////////////////////////////////


//! Test read-side metadata parsing and observation-time handling
bool UTFileEvents::TestOpenAndMetadata()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "metadata temp dir", "The temporary directory for MFileEvents tests can be created", PrepareTempDirectory()) && Passed;

  MString FileName = GetTempDirectory() + "/metadata.tra";
  MString Content =
    "Type tra\n"
    "Version 42\n"
    "Geometry ./geom/test.geo.setup\n"
    "Date 2026-04-13 12:00:00\n"
    "MEGAlib 9.9.9\n"
    "TB 10\n"
    "SE 1\n"
    "SE 2\n"
    "TE 25\n";

  Passed = EvaluateTrue("WriteTextFile(metadata)", "metadata file", "The metadata test file can be written", WriteTextFile(FileName, Content)) && Passed;

  FileEventsTest File;
  Passed = EvaluateTrue("MFileEvents::Open(read)", "metadata open", "The event file opens in read mode", File.Open(FileName)) && Passed;
  Passed = Evaluate("GetFileType()", "metadata type", "The event file type is parsed from the header", File.GetFileType(), MString("tra")) && Passed;
  Passed = Evaluate("GetVersion()", "metadata version", "The event file version is parsed from the header", File.GetVersion(), 42) && Passed;
  Passed = Evaluate("GetGeometryFileName()", "metadata geometry exact", "The geometry file name is parsed deterministically from the header", File.GetGeometryFileName(), MString("./geom/test.geo.setup")) && Passed;
  Passed = EvaluateTrue("GetGeometryFileName()", "metadata geometry", "The geometry file name is parsed from the header", File.GetGeometryFileName().EndsWith("geom/test.geo.setup")) && Passed;
  Passed = Evaluate("GetMEGAlibVersion()", "metadata MEGAlib version", "The MEGAlib version string is parsed from the header", File.GetMEGAlibVersion(), MString("9.9.9")) && Passed;
  Passed = Evaluate("HasStartObservationTime()", "metadata start flag", "A TB line marks the start observation time as available", File.HasStartObservationTime(), true) && Passed;
  Passed = EvaluateNear("GetStartObservationTime()", "metadata start time", "The TB line is parsed as the start observation time", File.GetStartObservationTime().GetAsDouble(), 10.0, 1e-12) && Passed;

  MTime ObservationTime = File.GetObservationTime();
  Passed = Evaluate("HasObservationTime()", "metadata observation flag", "Reading the footer populates the observation-time flag", File.HasObservationTime(), true) && Passed;
  Passed = Evaluate("HasEndObservationTime()", "metadata end flag", "Reading the footer populates the end observation-time flag", File.HasEndObservationTime(), true) && Passed;
  Passed = EvaluateNear("GetObservationTime()", "metadata observation time", "Observation time is computed as TE minus TB", ObservationTime.GetAsDouble(), 15.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEndObservationTime()", "metadata end time", "The TE line is parsed as the end observation time", File.GetEndObservationTime().GetAsDouble(), 25.0, 1e-12) && Passed;

  File.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test footer parsing variants used by derived event readers
bool UTFileEvents::TestFooterParsing()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "footer temp dir", "The temporary directory can be recreated for footer tests", PrepareTempDirectory()) && Passed;

  {
    MString FileName = GetTempDirectory() + "/footer_ti.tra";
    MString Content =
      "Type tra\n"
      "Version 3\n"
      "Geometry geo.setup\n"
      "TB 100\n"
      "SE 1\n"
      "TI 130\n";

    Passed = EvaluateTrue("WriteTextFile(footer_ti)", "footer TI file", "The TI fallback footer file can be written", WriteTextFile(FileName, Content)) && Passed;

    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "footer TI open", "The TI fallback footer file opens successfully", File.Open(FileName)) && Passed;
    Passed = EvaluateNear("GetObservationTime()", "footer TI observation time", "If TE is missing, the last TI line is used to determine the observation time", File.GetObservationTime().GetAsDouble(), 30.0, 1e-12) && Passed;
    Passed = Evaluate("HasObservationTime()", "footer TI observation flag", "The TI fallback still marks the observation time as available", File.HasObservationTime(), true) && Passed;
    File.Close();
  }

  {
    MString FileName = GetTempDirectory() + "/footer_continue.tra";
    MString Content =
      "Type tra\n"
      "Version 4\n"
      "Geometry geo.setup\n"
      "TB 20\n"
      "SE 1\n"
      "SE 2\n"
      "TE 35\n";

    Passed = EvaluateTrue("WriteTextFile(footer_continue)", "footer continue file", "The continue-mode footer file can be written", WriteTextFile(FileName, Content)) && Passed;

    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "footer continue open", "The continue-mode footer file opens successfully", File.Open(FileName)) && Passed;
    MString Line;
    while (File.ReadLine(Line) == true) {
    }
    Passed = EvaluateTrue("ReadFooter(true)", "footer continue parse", "ReadFooter(true) parses footer information from the current EOF position", File.TestReadFooter(true)) && Passed;
    Passed = EvaluateNear("ReadFooter(true)", "footer continue observation time", "ReadFooter(true) computes TE minus TB when called after the read loop", File.GetObservationTime().GetAsDouble(), 15.0, 1e-12) && Passed;
    File.Close();
  }

  {
    MString FileName = GetTempDirectory() + "/footer_override.tra";
    MString Content =
      "Type tra\n"
      "Version 5\n"
      "Geometry geo.setup\n"
      "TB 1\n"
      "SE 1\n"
      "TE 999\n";

    Passed = EvaluateTrue("WriteTextFile(footer_override)", "footer override file", "The observation-time override file can be written", WriteTextFile(FileName, Content)) && Passed;

    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "footer override open", "The override file opens successfully", File.Open(FileName)) && Passed;
    File.SetObservationTime(MTime(123.0));
    Passed = EvaluateNear("SetObservationTime()", "footer override observation time", "An explicitly set observation time is returned without reading the footer again", File.GetObservationTime().GetAsDouble(), 123.0, 1e-12) && Passed;
    File.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test write-side helpers
bool UTFileEvents::TestWriting()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "writing temp dir", "The temporary directory can be recreated for write tests", PrepareTempDirectory()) && Passed;

  MString FileName = GetTempDirectory() + "/write.tra";

  FileEventsTest File;
  File.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
  Passed = Evaluate("SetGeometryFileName()", "geometry expansion exact", "SetGeometryFileName stores the exact expanded geometry path", File.GetGeometryFileName(), GetTempDirectory() + "/geometry.setup") && Passed;
  Passed = EvaluateTrue("SetGeometryFileName()", "geometry expansion", "SetGeometryFileName stores the expanded geometry path", File.GetGeometryFileName().EndsWith("/tmp/UTFileEvents/geometry.setup")) && Passed;
  File.SetObservationTime(MTime(123.5));
  Passed = EvaluateTrue("Open(write)", "write open", "The write test file opens successfully", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "write header", "WriteHeader succeeds in write mode", File.WriteHeader()) && Passed;
  Passed = EvaluateTrue("AddFooter(empty)", "empty footer", "AddFooter is a no-op success for empty footer text", File.AddFooter("")) && Passed;
  Passed = EvaluateTrue("AddFooter()", "write footer", "AddFooter succeeds in write mode", File.AddFooter("FooterText")) && Passed;
  Passed = EvaluateTrue("CloseEventList()", "write close event list", "CloseEventList writes the EN and TE trailer", File.CloseEventList()) && Passed;
  Passed = EvaluateTrue("Close()", "write close", "The write test file closes cleanly", File.Close()) && Passed;

  MString Text = ReadTextFile(FileName);
  Passed = EvaluateTrue("WriteHeader()", "header content type", "The header contains the event type line", Text.Contains("Type      tra")) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "header content version", "The header contains the version line", Text.Contains("Version   7")) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "header content geometry", "The header contains the geometry line", Text.Contains("Geometry  ")) && Passed;
  Passed = EvaluateTrue("AddFooter()", "footer content", "The footer markers and payload are written", Text.Contains("FT START") && Text.Contains("FooterText") && Text.Contains("FT STOP")) && Passed;
  Passed = EvaluateTrue("CloseEventList()", "close content", "The event-list trailer contains EN and the observation time", Text.Contains("EN\n") && Text.Contains("TE 123.5")) && Passed;

  {
    MString BinaryFileName = GetTempDirectory() + "/write_binary.tra";
    FileEventsTest BinaryFile;
    BinaryFile.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
    Passed = EvaluateTrue("Open(write,binary)", "binary open", "The binary write test file opens successfully", BinaryFile.Open(BinaryFileName, MFile::c_Write, true)) && Passed;
    Passed = EvaluateTrue("WriteHeader() binary", "binary header", "WriteHeader writes the binary stream marker when the file is binary", BinaryFile.WriteHeader()) && Passed;
    Passed = EvaluateTrue("Close() binary", "binary close", "The binary write test file closes cleanly", BinaryFile.Close()) && Passed;

    MString BinaryText = ReadTextFile(BinaryFileName);
    Passed = EvaluateTrue("WriteHeader() binary", "binary marker content", "The binary header contains the STARTBINARYSTREAM marker", BinaryText.Contains("STARTBINARYSTREAM")) && Passed;

    FileEventsTest BinaryReader;
    Passed = EvaluateTrue("Open(read) binary header", "binary reader open", "A file with STARTBINARYSTREAM can be reopened in read mode", BinaryReader.Open(BinaryFileName)) && Passed;
    Passed = Evaluate("IsBinary()", "binary reader flag", "Opening a file with STARTBINARYSTREAM marks the event file as binary", BinaryReader.IsBinary(), true) && Passed;
    BinaryReader.Close();
  }

  {
    FileEventsTest ReadOnly;
    Passed = EvaluateTrue("Open(read)", "read-only write guards open", "The file can be reopened in read mode for write-guard checks", ReadOnly.Open(FileName)) && Passed;
    DisableDefaultStreams();
#ifdef NDEBUG
    Passed = EvaluateFalse("WriteHeader()", "read-only write guard", "WriteHeader is rejected in read mode", ReadOnly.WriteHeader()) && Passed;
    Passed = EvaluateFalse("AddFooter()", "read-only footer guard", "AddFooter is rejected in read mode", ReadOnly.AddFooter("x")) && Passed;
    Passed = EvaluateFalse("CloseEventList()", "read-only close guard", "CloseEventList is rejected in read mode", ReadOnly.CloseEventList()) && Passed;
#endif
    EnableDefaultStreams();
    ReadOnly.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test file-tree helper methods
bool UTFileEvents::TestFileTreeHelpers()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "helpers temp dir", "The temporary directory can be recreated for helper tests", PrepareTempDirectory()) && Passed;

  MString FirstFile = GetTempDirectory() + "/first.tra";
  MString SecondFile = GetTempDirectory() + "/second.tra";
  MString ThirdFile = GetTempDirectory() + "/third.tra";
  MString IncludeFile = GetTempDirectory() + "/include.tra";

  Passed = EvaluateTrue("WriteTextFile(first)", "first helper file", "The first helper file can be written", WriteTextFile(FirstFile, "Type tra\nVersion 1\nGeometry geo.setup\nTB 1\nSE 1\nTE 4\nNF second.tra\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile(second)", "second helper file", "The second helper file can be written", WriteTextFile(SecondFile, "Type tra\nVersion 2\nGeometry geo.setup\nTB 10\nSE 2\nTE 14\nNF third.tra\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile(third)", "third helper file", "The third helper file can be written", WriteTextFile(ThirdFile, "Type tra\nVersion 3\nGeometry geo.setup\nTB 20\nSE 3\nTE 26\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile(include)", "include helper file", "The include helper file can be written", WriteTextFile(IncludeFile, "Type tra\nVersion 3\nGeometry geo.setup\nTB 30\nSE 3\nTE 41\n")) && Passed;

  {
    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "open-next open", "The first helper file opens successfully", File.Open(FirstFile)) && Passed;
    DisableDefaultStreams();
    bool OpenNext = File.TestOpenNextFile("NF second.tra");
    EnableDefaultStreams();
    Passed = EvaluateTrue("OpenNextFile()", "open next", "OpenNextFile switches to the referenced next file", OpenNext) && Passed;
    Passed = Evaluate("OpenNextFile()", "open next file name exact", "OpenNextFile updates the current file name to the exact next file path", File.GetFileName(), SecondFile) && Passed;
    Passed = Evaluate("OpenNextFile()", "open next original file name exact", "OpenNextFile preserves the exact original root file name across NF transitions", File.GetOriginalFileName(), FirstFile) && Passed;
    Passed = EvaluateTrue("OpenNextFile()", "open next file name", "OpenNextFile updates the current file name to the next file", File.GetFileName().EndsWith("second.tra")) && Passed;
    Passed = EvaluateTrue("OpenNextFile()", "open next original file name", "OpenNextFile preserves the original root file name across NF transitions", File.GetOriginalFileName().EndsWith("first.tra")) && Passed;
    Passed = Evaluate("GetVersion()", "open next version", "OpenNextFile reparses header metadata from the next file", File.GetVersion(), 2) && Passed;
    Passed = EvaluateNear("OpenNextFile()", "open next observation time", "OpenNextFile preserves the original observation time when switching to the next file", File.GetObservationTime().GetAsDouble(), 7.0, 1e-12) && Passed;

    DisableDefaultStreams();
    bool OpenThird = File.TestOpenNextFile("NF third.tra");
    EnableDefaultStreams();
    Passed = EvaluateTrue("OpenNextFile() second", "open third", "A second OpenNextFile call switches to the third file in the chain", OpenThird) && Passed;
    Passed = Evaluate("OpenNextFile() second", "open third file name exact", "The second OpenNextFile call updates the current file name to the exact third file path", File.GetFileName(), ThirdFile) && Passed;
    Passed = Evaluate("OpenNextFile() second", "open third original file name exact", "The original root file name remains the exact first file path across multiple NF transitions", File.GetOriginalFileName(), FirstFile) && Passed;
    Passed = EvaluateTrue("OpenNextFile() second", "open third file name", "The second OpenNextFile call updates the current file name to the third file", File.GetFileName().EndsWith("third.tra")) && Passed;
    Passed = EvaluateTrue("OpenNextFile() second", "open third original file name", "The original root file name remains unchanged across multiple NF transitions", File.GetOriginalFileName().EndsWith("first.tra")) && Passed;
    Passed = Evaluate("OpenNextFile() second", "open third version", "A second OpenNextFile call reparses the header metadata from the third file", File.GetVersion(), 3) && Passed;
    Passed = EvaluateNear("OpenNextFile() second", "open third observation time", "Observation time accumulates across three chained NF files", File.GetObservationTime().GetAsDouble(), 13.0, 1e-12) && Passed;
    File.Close();
  }

  {
    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "open-include open", "The helper file opens successfully for include-file tests", File.Open(FirstFile)) && Passed;
    DisableDefaultStreams();
    bool OpenInclude = File.TestOpenIncludeFile("IN include.tra");
    EnableDefaultStreams();
    Passed = EvaluateTrue("OpenIncludeFile()", "open include", "OpenIncludeFile opens the referenced include file", OpenInclude) && Passed;
    Passed = Evaluate("IsIncludeFileUsed()", "open include used", "Opening an include file marks the include-file state as used", File.IsIncludeFileUsed(), true) && Passed;
    Passed = EvaluateTrue("GetIncludeFile()", "open include pointer", "The include-file helper exists and is open", File.GetIncludeFile() != nullptr && File.GetIncludeFile()->IsOpen()) && Passed;
    if (File.GetIncludeFile() != nullptr) {
      Passed = Evaluate("GetIncludeFile()->GetFileName()", "open include file name exact", "The include file points at the exact requested include file", File.GetIncludeFile()->GetFileName(), IncludeFile) && Passed;
      Passed = EvaluateTrue("GetIncludeFile()->GetFileName()", "open include file name", "The include file points at the requested include file", File.GetIncludeFile()->GetFileName().EndsWith("include.tra")) && Passed;
    }
    Passed = EvaluateTrue("CloseIncludeFile()", "close include", "CloseIncludeFile closes the active include file cleanly", File.TestCloseIncludeFile()) && Passed;
    Passed = Evaluate("HasObservationTime()", "close include observation flag", "Closing an include file contributes its observation time to the parent stream", File.HasObservationTime(), true) && Passed;
    Passed = EvaluateNear("CloseIncludeFile()", "close include observation time", "Closing an include file adds the include observation time to the parent observation time", File.GetObservationTime().GetAsDouble(), 11.0, 1e-12) && Passed;
    File.Close();
  }

  {
    FileEventsTest File;
    File.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "create-next open", "The main file opens in write mode for next-file creation", File.Open(FirstFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "create-next header", "The initial file header can be written before creating a next file", File.WriteHeader()) && Passed;
    DisableDefaultStreams();
    bool Created = File.TestCreateNextFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateNextFile()", "create next", "CreateNextFile closes the current file and opens the next split file", Created) && Passed;
    Passed = Evaluate("CreateNextFile()", "create next file name exact", "CreateNextFile advances to the exact numbered split file path", File.GetFileName(), GetTempDirectory() + "/first.id1.tra") && Passed;
    Passed = EvaluateTrue("CreateNextFile()", "create next file name", "CreateNextFile advances to a numbered split file", File.GetFileName().Contains(".id1.tra")) && Passed;
    Passed = EvaluateTrue("Exists(split file)", "create next split exists", "The newly created split file exists on disk", MFile::Exists(File.GetFileName())) && Passed;
    File.Close();
  }

  {
    FileEventsTest File;
    File.SetGeometryFileName(GetTempDirectory() + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "create-include open", "The main file opens in write mode for include-file creation", File.Open(FirstFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "create-include header", "The initial header can be written before creating an include file", File.WriteHeader()) && Passed;
    DisableDefaultStreams();
    bool Created = File.TestCreateIncludeFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateIncludeFile()", "create include", "CreateIncludeFile creates and opens the first include file", Created) && Passed;
    Passed = Evaluate("IsIncludeFileUsed()", "create include used", "Creating an include file marks the include-file state as used", File.IsIncludeFileUsed(), true) && Passed;
    Passed = EvaluateTrue("GetIncludeFile()", "create include pointer", "Creating an include file opens the include helper", File.GetIncludeFile() != nullptr && File.GetIncludeFile()->IsOpen()) && Passed;
    if (File.GetIncludeFile() != nullptr) {
      Passed = Evaluate("GetIncludeFile()->GetFileName()", "create include file name exact", "The created include file uses the exact first numbered include path", File.GetIncludeFile()->GetFileName(), GetTempDirectory() + "/first.id2.tra") && Passed;
      Passed = EvaluateTrue("GetIncludeFile()->GetFileName()", "create include file name", "The created include file uses the numbered include naming scheme", File.GetIncludeFile()->GetFileName().Contains(".id2.tra")) && Passed;
    }
    DisableDefaultStreams();
    bool CreatedAgain = File.TestCreateIncludeFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateIncludeFile() second", "create include reuse", "A second CreateIncludeFile call rotates the include file to the next numbered include", CreatedAgain) && Passed;
    if (File.GetIncludeFile() != nullptr) {
      Passed = Evaluate("CreateIncludeFile() second", "create include reuse file name exact", "The rotated include file advances to the exact next numbered include path", File.GetIncludeFile()->GetFileName(), GetTempDirectory() + "/first.id3.tra") && Passed;
      Passed = EvaluateTrue("CreateIncludeFile() second", "create include reuse file name", "The rotated include file advances to the next numbered include file", File.GetIncludeFile()->GetFileName().Contains(".id3.tra")) && Passed;
    }
    File.Close();
  }

  {
    FileEventsTest File;
    Passed = Evaluate("CreateIncludeFileName()", "name helper", "CreateIncludeFileName creates the first numbered include-file name", File.TestCreateIncludeFileName("/tmp/base.tra"), MString("/tmp/base.id1.tra")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test event counting and rewind behavior
bool UTFileEvents::TestCountingAndRewind()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTempDirectory()", "counting temp dir", "The temporary directory can be recreated for counting tests", PrepareTempDirectory()) && Passed;

  MString MainFile = GetTempDirectory() + "/count.tra";
  MString IncludeFile = GetTempDirectory() + "/count_include.tra";
  MString NextFile = GetTempDirectory() + "/count_next.tra";

  Passed = EvaluateTrue("WriteTextFile(include)", "count include file", "The include counting file can be written", WriteTextFile(IncludeFile, "Type tra\nVersion 1\nGeometry geo.setup\nSE 3\nSE 4\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile(next)", "count next file", "The next-file counting file can be written", WriteTextFile(NextFile, "Type tra\nVersion 1\nGeometry geo.setup\nSE 5\nSE 6\n")) && Passed;
  Passed = EvaluateTrue("WriteTextFile(main)", "count main file", "The main counting file can be written", WriteTextFile(MainFile, "Type tra\nVersion 1\nGeometry geo.setup\nSE 1\nSE 2\nIN count_include.tra\nNF count_next.tra\n")) && Passed;

  {
    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "count open", "The counting file opens successfully", File.Open(MainFile)) && Passed;
    DisableDefaultStreams();
    int Count = File.GetNEvents(false);
    EnableDefaultStreams();
    Passed = Evaluate("GetNEvents(false)", "count exact", "GetNEvents(false) falls back to full counting across include and next files when needed", Count, 6) && Passed;
    Passed = EvaluateTrue("Rewind()", "count rewind", "Rewind succeeds after event counting changed the active file state", File.Rewind()) && Passed;

    MString Line;
    Passed = EvaluateTrue("ReadLine() after rewind", "rewind first line", "After rewind the file can be read again from the beginning", File.ReadLine(Line)) && Passed;
    Passed = Evaluate("ReadLine() after rewind", "rewind content", "After rewind the first line is the original main-file header", Line, MString("Type tra")) && Passed;
    File.Close();
  }

  {
    MString SimpleFile = GetTempDirectory() + "/simple_count.tra";
    Passed = EvaluateTrue("WriteTextFile(simple)", "simple count file", "The direct-count file can be written", WriteTextFile(SimpleFile, "Type tra\nVersion 1\nGeometry geo.setup\nSE 11\nSE 12\nSE 13\n")) && Passed;

    FileEventsTest File;
    Passed = EvaluateTrue("Open(read)", "simple count open", "The direct-count file opens successfully", File.Open(SimpleFile)) && Passed;
    Passed = Evaluate("GetNEvents(false)", "simple count exact", "GetNEvents(false) returns the highest direct SE event id when no recursion is needed", File.GetNEvents(false), 13) && Passed;
    Passed = Evaluate("GetNEvents(true)", "simple count total", "GetNEvents(true) counts the number of SE records directly", File.GetNEvents(true), 3) && Passed;
    File.Close();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFileEvents Test;
  return Test.Run() == true ? 0 : 1;
}
