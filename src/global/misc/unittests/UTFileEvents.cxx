/*
 * UTFileEvents.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


// Standard libs:
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>

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


//! Test read-side metadata parsing and observation-time handling
bool UTFileEvents::TestOpenAndMetadata()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "metadata temp dir", "The temporary directory for MFileEvents tests can be created", PrepareTemporaryDirectory("metadata")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("metadata");

  MString FileName = TemporaryDirectory + "/metadata.tra";
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

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "footer temp dir", "The temporary directory can be recreated for footer tests", PrepareTemporaryDirectory("footer")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("footer");

  {
    MString FileName = TemporaryDirectory + "/footer_ti.tra";
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
    MString FileName = TemporaryDirectory + "/footer_continue.tra";
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
    MString FileName = TemporaryDirectory + "/footer_override.tra";
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

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "writing temp dir", "The temporary directory can be recreated for write tests", PrepareTemporaryDirectory("writing")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("writing");

  MString FileName = TemporaryDirectory + "/write.tra";

  FileEventsTest File;
  File.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
  Passed = Evaluate("SetGeometryFileName()", "geometry expansion exact", "SetGeometryFileName stores the exact expanded geometry path", File.GetGeometryFileName(), TemporaryDirectory + "/geometry.setup") && Passed;
  File.SetObservationTime(MTime(123.5));
  Passed = EvaluateTrue("Open(write)", "write open", "The write test file opens successfully", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "write header", "WriteHeader succeeds in write mode", File.WriteHeader()) && Passed;
  Passed = EvaluateTrue("AddFooter(empty)", "empty footer", "AddFooter is a no-op success for empty footer text", File.AddFooter("")) && Passed;
  Passed = EvaluateTrue("AddFooter()", "write footer", "AddFooter succeeds in write mode", File.AddFooter("FooterText")) && Passed;
  Passed = EvaluateTrue("WriteFooter()", "write footer block", "WriteFooter writes the EN line, the TE trailer, and the footer text", File.WriteFooter()) && Passed;
  Passed = EvaluateTrue("Close()", "write close", "The write test file closes cleanly", File.Close()) && Passed;

  MString Text = ReadTextFile(FileName);
  // Expected header begin: type, version, geometry, blank line, date line
  const MString ExpectedHeaderBegin = MString("Type      tra\nVersion   7\nGeometry  ") + TemporaryDirectory + "/geometry.setup\n\nDate      ";
  Passed = EvaluateTrue("WriteHeader()", "header content begin", "The file begins with the exact type, version, and geometry lines followed by the date line", Text.BeginsWith(ExpectedHeaderBegin)) && Passed;
  Passed = EvaluateTrue("WriteHeader()", "header content version string", "The header contains the MEGAlib version line followed by a blank line", Text.Contains(MString("\nMEGAlib   ") + g_VersionString + "\n\n")) && Passed;
  // Header date: "Date      YYYY-MM-DD HH:MM:SS" in UTC, written now
  {
    const char* DateLine = strstr(Text.Data(), "\nDate      ");
    int Year = 0, Month = 0, Day = 0, Hour = 0, Minute = 0, Second = 0;
    bool DateParsed = false;
    if (DateLine != nullptr) {
      DateParsed = (sscanf(DateLine + 11, "%d-%d-%d %d:%d:%d", &Year, &Month, &Day, &Hour, &Minute, &Second) == 6);
    }
    Passed = EvaluateTrue("WriteHeader()", "date parsed", "The header contains a date line of the form YYYY-MM-DD HH:MM:SS", DateParsed) && Passed;
    struct tm Parsed;
    Parsed.tm_year = Year - 1900;
    Parsed.tm_mon = Month - 1;
    Parsed.tm_mday = Day;
    Parsed.tm_hour = Hour;
    Parsed.tm_min = Minute;
    Parsed.tm_sec = Second;
    Parsed.tm_isdst = 0;
    const double HeaderTime = static_cast<double>(timegm(&Parsed));
    const double Difference = HeaderTime - static_cast<double>(time(nullptr));
    Passed = EvaluateTrue("WriteHeader()", "date is now", "The header date is the current UTC time (within one minute of the clock)", fabs(Difference) < 60.0) && Passed;
  }
  // Footer: EN, two blank lines, TE (the observation time, since there is no end time), blank line, FT block, blank line
  Passed = EvaluateTrue("WriteFooter()", "footer and trailer content", "The file ends with the EN line, the TE trailer, and the exact footer block", Text.EndsWith("EN\n\n\nTE 123.500000000\n\nFT START\nFooterText\nFT STOP\n\n")) && Passed;

  {
    MString BinaryFileName = TemporaryDirectory + "/write_binary.tra";
    FileEventsTest BinaryFile;
    BinaryFile.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write,binary)", "binary open", "The binary write test file opens successfully", BinaryFile.Open(BinaryFileName, MFile::c_Write, true)) && Passed;
    Passed = EvaluateTrue("WriteHeader() binary", "binary header", "WriteHeader writes the binary stream marker when the file is binary", BinaryFile.WriteHeader()) && Passed;
    Passed = EvaluateTrue("Close() binary", "binary close", "The binary write test file closes cleanly", BinaryFile.Close()) && Passed;

    MString BinaryText = ReadTextFile(BinaryFileName);
    Passed = EvaluateTrue("WriteHeader() binary", "binary marker content", "The binary header ends with the STARTBINARYSTREAM marker line after a blank line", BinaryText.EndsWith("\n\nSTARTBINARYSTREAM\n")) && Passed;

    FileEventsTest BinaryReader;
    Passed = EvaluateTrue("Open(read) binary header", "binary reader open", "A file with STARTBINARYSTREAM can be reopened in read mode", BinaryReader.Open(BinaryFileName)) && Passed;
    Passed = Evaluate("IsBinary()", "binary reader flag", "Opening a file with STARTBINARYSTREAM marks the event file as binary", BinaryReader.IsBinary(), true) && Passed;
    BinaryReader.Close();
  }

  {
    // A Geant4 line without a version is reported as such
    MString BadGeant4FileName = TemporaryDirectory + "/bad_geant4.tra";
    Passed = EvaluateTrue("WriteTextFile()", "bad Geant4 file", "The file with a Geant4 line without version can be written",
                          WriteTextFile(BadGeant4FileName, "Type      tra\nVersion   7\nGeometry  bad.setup\n\nGeant4\nMEGAlib   1.00.00\n\nSE\nEN\n")) && Passed;
    MString BadGeant4LogName = TemporaryDirectory + "/bad_geant4.log";
    mout.DumpToStdOut(false);
    mout.Connect(BadGeant4LogName);
    FileEventsTest BadGeant4;
    BadGeant4.Open(BadGeant4FileName);
    mout.Disconnect(BadGeant4LogName);
    mout.DumpToStdOut(true);
    BadGeant4.Close();
    MString BadGeant4Log = ReadTextFile(BadGeant4LogName);
    Passed = EvaluateTrue("Open()", "Geant4 error message", "A Geant4 line without version reports that the Geant4 version cannot be read", BadGeant4Log.Contains("Unable to read Geant4 version.")) && Passed;
    Passed = EvaluateFalse("Open()", "Geant4 error not geometry", "The Geant4 error message does not talk about the geometry name", BadGeant4Log.Contains("geometry name")) && Passed;
    Passed = Evaluate("HasGeant4Version()", "bad Geant4 file", "A Geant4 line without version leaves the Geant4 version unset", BadGeant4.HasGeant4Version(), false) && Passed;
  }

  {
    // CloseEventList: without an end time, TE is the start time plus the observation time
    MString NoEndFileName = TemporaryDirectory + "/close_no_end.tra";
    FileEventsTest NoEnd;
    NoEnd.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    NoEnd.SetStartObservationTime(MTime(10.0));
    NoEnd.SetObservationTime(MTime(20.0));
    Passed = EvaluateTrue("Open(write)", "close no end open", "The file without end time opens in write mode", NoEnd.Open(NoEndFileName, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "close no end header", "WriteHeader succeeds without an end time", NoEnd.WriteHeader()) && Passed;
    Passed = EvaluateTrue("CloseEventList()", "close no end list", "CloseEventList succeeds without an end time", NoEnd.CloseEventList()) && Passed;
    Passed = EvaluateTrue("Close()", "close no end close", "The file without end time closes cleanly", NoEnd.Close()) && Passed;
    // End time = start + observation time = 10 + 20
    Passed = EvaluateTrue("CloseEventList()", "close no end TE", "TE is the start time plus the observation time", ReadTextFile(NoEndFileName).Contains("\nTE 30.000000000\n")) && Passed;
  }

  {
    // The header starts the binary stream, the footer ends it
    MString BinaryFooterFileName = TemporaryDirectory + "/binary_footer.tra";
    FileEventsTest BinaryFooter;
    BinaryFooter.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    BinaryFooter.SetEndObservationTime(MTime(5.0));
    BinaryFooter.SetSimulatedEvents(7);
    Passed = EvaluateTrue("Open(write,binary)", "binary footer open", "The binary file for the footer test opens in write mode", BinaryFooter.Open(BinaryFooterFileName, MFile::c_Write, true)) && Passed;
    Passed = EvaluateTrue("WriteHeader() binary", "binary footer header", "WriteHeader succeeds for the binary footer test", BinaryFooter.WriteHeader()) && Passed;
    Passed = EvaluateTrue("WriteFooter() binary", "binary footer", "WriteFooter succeeds for a binary file", BinaryFooter.WriteFooter()) && Passed;
    Passed = EvaluateTrue("Close() binary", "binary footer close", "The binary file for the footer test closes cleanly", BinaryFooter.Close()) && Passed;

    MString BinaryFooterText = ReadTextFile(BinaryFooterFileName);
    unsigned int StartMarkers = 0;
    for (const char* Position = strstr(BinaryFooterText.Data(), "STARTBINARYSTREAM"); Position != nullptr; Position = strstr(Position + 1, "STARTBINARYSTREAM")) {
      ++StartMarkers;
    }
    Passed = Evaluate("WriteFooter() binary", "single start marker", "The binary stream is started once (in the header), not again in the footer", StartMarkers, 1U) && Passed;
    Passed = EvaluateTrue("WriteFooter() binary", "end marker", "The footer of a binary file ends the binary stream with ENDBINARYSTREAM (as MFileEventsSim::CloseEventList does)", BinaryFooterText.Contains("\nENDBINARYSTREAM\n")) && Passed;

    FileEventsTest BinaryFooterReader;
    Passed = EvaluateTrue("Open(read) binary footer", "binary footer reader open", "The binary file with a footer opens in read mode", BinaryFooterReader.Open(BinaryFooterFileName)) && Passed;
    Passed = EvaluateTrue("ReadFooter() binary", "binary footer read", "The footer of the binary file can be read", BinaryFooterReader.TestReadFooter()) && Passed;
    Passed = Evaluate("ReadFooter() binary", "binary end time flag", "The end time is found in the footer of the binary file", BinaryFooterReader.HasEndObservationTime(), true) && Passed;
    Passed = EvaluateNear("ReadFooter() binary", "binary end time", "The end time of the binary file is read exactly", BinaryFooterReader.GetEndObservationTime().GetAsSeconds(), 5.0, 1e-12) && Passed;
    Passed = Evaluate("ReadFooter() binary", "binary simulated events flag", "The number of simulated events is found in the footer of the binary file", BinaryFooterReader.HasSimulatedEvents(), true) && Passed;
    Passed = Evaluate("ReadFooter() binary", "binary simulated events", "The number of simulated events of the binary file is read exactly", BinaryFooterReader.GetSimulatedEvents(), 7L) && Passed;
    BinaryFooterReader.Close();
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

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "helpers temp dir", "The temporary directory can be recreated for helper tests", PrepareTemporaryDirectory("helpers")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("helpers");

  MString FirstFile = TemporaryDirectory + "/first.tra";
  MString SecondFile = TemporaryDirectory + "/second.tra";
  MString ThirdFile = TemporaryDirectory + "/third.tra";
  MString IncludeFile = TemporaryDirectory + "/include.tra";

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
    File.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "create-next open", "The main file opens in write mode for next-file creation", File.Open(FirstFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "create-next header", "The initial file header can be written before creating a next file", File.WriteHeader()) && Passed;
    DisableDefaultStreams();
    bool Created = File.TestCreateNextFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateNextFile()", "create next", "CreateNextFile closes the current file and opens the next split file", Created) && Passed;
    Passed = Evaluate("CreateNextFile()", "create next file name exact", "CreateNextFile advances to the exact numbered split file path", File.GetFileName(), TemporaryDirectory + "/first.id1.tra") && Passed;
    Passed = EvaluateTrue("Exists(split file)", "create next split exists", "The newly created split file exists on disk", MFile::Exists(File.GetFileName())) && Passed;
    File.Close();
  }

  {
    FileEventsTest File;
    File.SetGeometryFileName(TemporaryDirectory + "/geometry.setup");
    Passed = EvaluateTrue("Open(write)", "create-include open", "The main file opens in write mode for include-file creation", File.Open(FirstFile, MFile::c_Write)) && Passed;
    Passed = EvaluateTrue("WriteHeader()", "create-include header", "The initial header can be written before creating an include file", File.WriteHeader()) && Passed;
    DisableDefaultStreams();
    bool Created = File.TestCreateIncludeFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateIncludeFile()", "create include", "CreateIncludeFile creates and opens the first include file", Created) && Passed;
    Passed = Evaluate("IsIncludeFileUsed()", "create include used", "Creating an include file marks the include-file state as used", File.IsIncludeFileUsed(), true) && Passed;
    Passed = EvaluateTrue("GetIncludeFile()", "create include pointer", "Creating an include file opens the include helper", File.GetIncludeFile() != nullptr && File.GetIncludeFile()->IsOpen()) && Passed;
    if (File.GetIncludeFile() != nullptr) {
      Passed = Evaluate("GetIncludeFile()->GetFileName()", "create include file name exact", "The created include file uses the exact first numbered include path", File.GetIncludeFile()->GetFileName(), TemporaryDirectory + "/first.id2.tra") && Passed;
    }
    DisableDefaultStreams();
    bool CreatedAgain = File.TestCreateIncludeFile();
    EnableDefaultStreams();
    Passed = EvaluateTrue("CreateIncludeFile() second", "create include reuse", "A second CreateIncludeFile call rotates the include file to the next numbered include", CreatedAgain) && Passed;
    if (File.GetIncludeFile() != nullptr) {
      Passed = Evaluate("CreateIncludeFile() second", "create include reuse file name exact", "The rotated include file advances to the exact next numbered include path", File.GetIncludeFile()->GetFileName(), TemporaryDirectory + "/first.id3.tra") && Passed;
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

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "counting temp dir", "The temporary directory can be recreated for counting tests", PrepareTemporaryDirectory("counting")) && Passed;
  const MString TemporaryDirectory = GetTemporaryDirectoryName("counting");

  MString MainFile = TemporaryDirectory + "/count.tra";
  MString IncludeFile = TemporaryDirectory + "/count_include.tra";
  MString NextFile = TemporaryDirectory + "/count_next.tra";

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
    MString SimpleFile = TemporaryDirectory + "/simple_count.tra";
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
