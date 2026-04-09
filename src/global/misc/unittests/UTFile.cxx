/*
 * UTFile.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MFile.h"
#include "MUnitTest.h"
#include "MStreams.h"

// Standard libs:
#include <sstream>
using namespace std;


//! Unit test class for the MFile helper
class UTFile : public MUnitTest
{
public:
  //! Default constructor
  UTFile() : MUnitTest("UTFile") {}
  //! Default destructor
  virtual ~UTFile() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test static file and path helpers
  bool TestStaticHelpers();
  //! Test ASCII file I/O, line handling, and seeking
  bool TestAsciiIO();
  //! Test binary store I/O
  bool TestBinaryIO();
  //! Test gzip file I/O and compressed position helpers
  bool TestGzipIO();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTFile::Run()
{
  bool AllPassed = true;

  AllPassed = TestStaticHelpers() && AllPassed;
  AllPassed = TestAsciiIO() && AllPassed;
  AllPassed = TestBinaryIO() && AllPassed;
  AllPassed = TestGzipIO() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test static file and path helpers
bool UTFile::TestStaticHelpers()
{
  bool Passed = true;

  MString WorkingDirectory = MFile::GetWorkingDirectory();
  Passed = EvaluateTrue("GetWorkingDirectory()", "working directory", "The working directory is not empty", WorkingDirectory != "") && Passed;

  MString RelativeFile = "build/test.txt";
  Passed = EvaluateTrue("ExpandFileName()", "relative file", "Relative file names can be expanded", MFile::ExpandFileName(RelativeFile)) && Passed;
  Passed = EvaluateTrue("ExpandFileName()", "relative file", "Expanded file names are absolute", RelativeFile.BeginsWith("/")) && Passed;

  MString AbsoluteFile = "/tmp/UTFile_absolute.txt";
  MString OriginalAbsoluteFile = AbsoluteFile;
  Passed = EvaluateTrue("ExpandFileName()", "absolute file", "Absolute file names are accepted", MFile::ExpandFileName(AbsoluteFile)) && Passed;
  Passed = Evaluate("ExpandFileName()", "absolute file", "Absolute file names remain unchanged", AbsoluteFile, OriginalAbsoluteFile) && Passed;

  MString AppliedFile = "results.tra";
  Passed = EvaluateTrue("ApplyPath()", "relative file", "Relative file names accept an applied base path", MFile::ApplyPath(AppliedFile, "/tmp/config/source.dat")) && Passed;
  Passed = Evaluate("ApplyPath()", "relative file", "ApplyPath uses the directory portion of the working path", AppliedFile, MString("/tmp/config/results.tra")) && Passed;

  MString AlreadyAbsolute = "/tmp/results.tra";
  Passed = EvaluateTrue("ApplyPath()", "absolute file", "Absolute file names are left unchanged by ApplyPath", MFile::ApplyPath(AlreadyAbsolute, "/tmp/config/source.dat")) && Passed;
  Passed = Evaluate("ApplyPath()", "absolute file", "ApplyPath leaves absolute file names unchanged", AlreadyAbsolute, MString("/tmp/results.tra")) && Passed;

  Passed = Evaluate("GetDirectoryName()", "normal file", "GetDirectoryName returns the directory portion", MFile::GetDirectoryName("/tmp/data/example.tra"), MString("/tmp/data")) && Passed;
  Passed = Evaluate("GetBaseName()", "normal file", "GetBaseName returns the base file name", MFile::GetBaseName("/tmp/data/example.tra"), MString("example.tra")) && Passed;
  Passed = Evaluate("GetBaseName()", "root path", "GetBaseName keeps the root path unchanged", MFile::GetBaseName("/"), MString("/")) && Passed;
  Passed = Evaluate("RelativeFileName()", "same directory", "RelativeFileName returns a same-directory relative path", MFile::RelativeFileName("/tmp/test/sub/file.tra", "/tmp/test/base.tra"), MString("./sub/file.tra")) && Passed;
  Passed = Evaluate("RelativeFileName()", "parent directory", "RelativeFileName adds parent traversals when needed", MFile::RelativeFileName("/tmp/other/file.tra", "/tmp/test/base.tra"), MString("../other/file.tra")) && Passed;

  Passed = EvaluateTrue("ProgramExists()", "existing program", "ProgramExists finds shell programs on PATH", MFile::ProgramExists("sh")) && Passed;
  Passed = EvaluateFalse("ProgramExists()", "missing program", "ProgramExists returns false for missing programs", MFile::ProgramExists("definitely_not_a_real_megalib_test_program")) && Passed;

  MString TemporaryFile = "/tmp/UTFile_exists.txt";
  {
    ofstream Out(TemporaryFile.Data());
    Out<<"temporary"<<endl;
  }
  Passed = EvaluateTrue("Exists()", "existing file", "Exists recognizes readable files", MFile::Exists(TemporaryFile)) && Passed;
  Passed = EvaluateTrue("Remove()", "existing file", "Remove deletes existing files", MFile::Remove(TemporaryFile)) && Passed;
  Passed = EvaluateFalse("Exists()", "removed file", "Removed files no longer exist", MFile::Exists(TemporaryFile)) && Passed;

  mout.Enable(false);
  mgui.Enable(false);
  Passed = EvaluateFalse("Exists()", "empty file name", "Empty file names are rejected", MFile::Exists("")) && Passed;
  Passed = EvaluateFalse("Remove()", "directory path", "Directories are not removed through MFile::Remove", MFile::Remove("/tmp/")) && Passed;
  mgui.Enable(true);
  mout.Enable(true);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test ASCII file I/O, line handling, and seeking
bool UTFile::TestAsciiIO()
{
  bool Passed = true;

  MString FileName = "/tmp/UTFile_ascii.txt";
  MFile::Remove(FileName);

  MFile File;
  File.SetFileName(FileName);
  File.SetFileType("tra");
  File.SetVersion(7);

  Passed = Evaluate("GetFileName()", "set file name", "The configured file name is stored", File.GetFileName(), FileName) && Passed;
  Passed = Evaluate("GetFileType()", "set file type", "The configured file type is stored", File.GetFileType(), MString("tra")) && Passed;
  Passed = EvaluateNear("GetVersion()", "set version", "The configured version is stored", File.GetVersion(), 7.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Open()", "ascii write", "ASCII files can be opened for writing", File.Open(FileName, MFile::c_Write, false)) && Passed;
  Passed = EvaluateTrue("IsOpen()", "ascii write", "Opened files report IsOpen() == true", File.IsOpen()) && Passed;
  Passed = EvaluateFalse("IsBinary()", "ascii write", "ASCII files are not marked as binary", File.IsBinary()) && Passed;

  File.WriteLine("alpha");
  File.Write("beta");
  File.WriteLine();
  ostringstream Tail;
  Tail<<"gamma";
  File.WriteLine(Tail);
  File.Flush();

  Passed = EvaluateTrue("Close()", "ascii write", "ASCII files can be closed after writing", File.Close()) && Passed;
  Passed = EvaluateTrue("Exists()", "ascii file", "Written ASCII files exist on disk", MFile::Exists(FileName)) && Passed;

  Passed = EvaluateTrue("Open()", "ascii read", "ASCII files can be reopened for reading", File.Open(FileName, MFile::c_Read, false)) && Passed;
  Passed = EvaluateTrue("CheckFileExtension()", "matching extension", "The file extension check is case-insensitive", File.CheckFileExtension("TXT")) && Passed;
  Passed = EvaluateFalse("CheckFileExtension()", "non-matching extension", "The file extension check rejects non-matching extensions", File.CheckFileExtension("tra")) && Passed;
  Passed = EvaluateNear("GetFileLength()", "ascii length", "ASCII file length is tracked correctly", File.GetFileLength(), 17.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetUncompressedFileLength()", "ascii length", "Uncompressed ASCII file length matches the on-disk length", File.GetUncompressedFileLength(), 17.0, 1e-12) && Passed;

  char Character = '\0';
  Passed = EvaluateTrue("Get(char)", "first character", "Get(char) reads the first character", File.Get(Character)) && Passed;
  Passed = Evaluate("Get(char)", "first character", "Get(char) returns the expected first character", MString(Character), MString("a")) && Passed;
  Passed = EvaluateNear("GetFilePosition()", "first character", "GetFilePosition advances after reading one character", File.GetFilePosition(), 1.0, 1e-12) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind returns to the file beginning", File.Rewind()) && Passed;
  MString Line;
  Passed = EvaluateTrue("ReadLine(MString)", "first line", "ReadLine(MString) reads the first line", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "first line", "ReadLine(MString) returns the expected text", Line, MString("alpha")) && Passed;
  Passed = EvaluateTrue("ReadLine(MString)", "second line", "ReadLine(MString) reads the second line", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "second line", "ReadLine(MString) returns the expected second line", Line, MString("beta")) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind can be called repeatedly", File.Rewind()) && Passed;
  File.Seek(static_cast<streampos>(6));
  Passed = EvaluateNear("GetFilePosition()", "seek absolute", "Seek(streampos) moves to the requested absolute position", File.GetFilePosition(), 6.0, 1e-12) && Passed;
  Passed = EvaluateTrue("ReadLine(MString)", "seek absolute", "ReadLine(MString) reads from the absolute seek position", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "seek absolute", "Absolute seeking lands on the expected line", Line, MString("beta")) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind resets the file before relative seeks", File.Rewind()) && Passed;
  File.Seek(11, ios_base::beg);
  Passed = EvaluateNear("GetFilePosition()", "seek relative", "Seek(offset, beg) moves to the requested position", File.GetFilePosition(), 11.0, 1e-12) && Passed;
  char Buffer[16];
  Passed = EvaluateTrue("ReadLine(char*)", "delimited line", "ReadLine(char*) reads text until the delimiter", File.ReadLine(Buffer, 16, '\n')) && Passed;
  Passed = Evaluate("ReadLine(char*)", "delimited line", "ReadLine(char*) returns the expected delimited text", MString(Buffer), MString("gamma")) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind resets the file before current-relative seeks", File.Rewind()) && Passed;
  File.Seek(6, ios_base::cur);
  Passed = EvaluateNear("GetFilePosition()", "seek current", "Seek(offset, cur) moves relative to the current position", File.GetFilePosition(), 6.0, 1e-12) && Passed;
  Passed = EvaluateTrue("ReadLine(MString)", "seek current", "ReadLine(MString) works after current-relative seeking", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "seek current", "Current-relative seeking lands on the expected line", Line, MString("beta")) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind resets the file before end-relative seeks", File.Rewind()) && Passed;
  File.Seek(-6, ios_base::end);
  Passed = EvaluateTrue("ReadLine(MString)", "seek end", "ReadLine(MString) works after end-relative seeking", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "seek end", "End-relative seeking lands on the expected trailing line", Line, MString("gamma")) && Passed;

  Passed = EvaluateTrue("Rewind()", "ascii read", "Rewind resets the file before float parsing", File.Rewind()) && Passed;
  MString FloatFileName = "/tmp/UTFile_float.txt";
  MFile::Remove(FloatFileName);
  MFile FloatFile;
  Passed = EvaluateTrue("Open()", "float ascii write", "ASCII files for float parsing can be opened for writing", FloatFile.Open(FloatFileName, MFile::c_Create, false)) && Passed;
  FloatFile.Write(MString("1.5 2.75\n"));
  Passed = EvaluateTrue("Close()", "float ascii write", "Float test files can be closed after writing", FloatFile.Close()) && Passed;
  Passed = EvaluateTrue("Open()", "float ascii read", "ASCII files for float parsing can be reopened for reading", FloatFile.Open(FloatFileName, MFile::c_Read, false)) && Passed;
  float FirstFloat = 0.0F;
  float SecondFloat = 0.0F;
  Passed = EvaluateTrue("Get(float)", "first float", "Get(float) reads the first float token", FloatFile.Get(FirstFloat)) && Passed;
  Passed = EvaluateNear("Get(float)", "first float", "Get(float) parses the expected first float", FirstFloat, 1.5, 1e-6) && Passed;
  Passed = EvaluateTrue("Get(float)", "second float", "Get(float) reads the second float token", FloatFile.Get(SecondFloat)) && Passed;
  Passed = EvaluateNear("Get(float)", "second float", "Get(float) parses the expected second float", SecondFloat, 2.75, 1e-6) && Passed;
  Passed = EvaluateTrue("Close()", "float ascii read", "Float test files can be closed after reading", FloatFile.Close()) && Passed;

  MString DelimiterFileName = "/tmp/UTFile_delimiter.txt";
  MFile::Remove(DelimiterFileName);
  MFile DelimiterFile;
  Passed = EvaluateTrue("Open()", "delimiter write", "ASCII files for delimiter tests can be opened for writing", DelimiterFile.Open(DelimiterFileName, MFile::c_Write, false)) && Passed;
  DelimiterFile.Write(MString("abcdef|rest"));
  Passed = EvaluateTrue("Close()", "delimiter write", "Delimiter test files can be closed after writing", DelimiterFile.Close()) && Passed;
  Passed = EvaluateTrue("Open()", "delimiter read", "ASCII files for delimiter tests can be reopened for reading", DelimiterFile.Open(DelimiterFileName, MFile::c_Read, false)) && Passed;
  char SmallBuffer[4];
  Passed = EvaluateTrue("ReadLine(char*)", "small buffer", "ReadLine(char*) succeeds when the buffer fills before the delimiter", DelimiterFile.ReadLine(SmallBuffer, 4, '|')) && Passed;
  Passed = Evaluate("ReadLine(char*)", "small buffer", "ReadLine(char*) truncates to fit the supplied buffer", MString(SmallBuffer), MString("abc")) && Passed;
  Passed = EvaluateTrue("Close()", "delimiter read", "Delimiter test files can be closed after reading", DelimiterFile.Close()) && Passed;

  MFile LengthFile;
  LengthFile.SetCompressionLevel(0);
  Passed = EvaluateTrue("Open()", "compression clamp low", "Compression levels below one are clamped and still allow file creation", LengthFile.Open("/tmp/UTFile_clamp_low.gz", MFile::c_Write, false)) && Passed;
  Passed = EvaluateTrue("Close()", "compression clamp low", "Clamped low compression files can be closed", LengthFile.Close()) && Passed;
  LengthFile.SetCompressionLevel(10);
  Passed = EvaluateTrue("Open()", "compression clamp high", "Compression levels above nine are clamped and still allow file creation", LengthFile.Open("/tmp/UTFile_clamp_high.gz", MFile::c_Write, false)) && Passed;
  Passed = EvaluateTrue("Close()", "compression clamp high", "Clamped high compression files can be closed", LengthFile.Close()) && Passed;
  MFile::Remove("/tmp/UTFile_clamp_low.gz");
  MFile::Remove("/tmp/UTFile_clamp_high.gz");

  Passed = EvaluateTrue("Close()", "ascii read", "ASCII files can be closed after reading", File.Close()) && Passed;
  File.Reset();
  Passed = EvaluateFalse("IsOpen()", "reset", "Reset closes the file", File.IsOpen()) && Passed;
  Passed = Evaluate("GetFileName()", "reset", "Reset clears the stored file name", File.GetFileName(), MString("")) && Passed;

  MString EmptyFileName = "/tmp/UTFile_empty.txt";
  MFile::Remove(EmptyFileName);
  MFile EmptyFile;
  Passed = EvaluateTrue("Open()", "empty file write", "Empty files can be created", EmptyFile.Open(EmptyFileName, MFile::c_Create, false)) && Passed;
  Passed = EvaluateTrue("Close()", "empty file write", "Empty files can be closed after creation", EmptyFile.Close()) && Passed;
  Passed = EvaluateTrue("Open()", "empty file read", "Empty files can be reopened for reading", EmptyFile.Open(EmptyFileName, MFile::c_Read, false)) && Passed;
  Passed = EvaluateNear("GetFileLength()", "empty file", "Empty files report zero length", EmptyFile.GetFileLength(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Get(char)", "empty file", "Reading a character from an empty file fails", EmptyFile.Get(Character)) && Passed;
  EmptyFile.Clear();
  Passed = EvaluateTrue("IsGood()", "empty file", "Clearing an empty file resets the stream state flags", EmptyFile.IsGood()) && Passed;
  Passed = EvaluateTrue("Close()", "empty file read", "Empty files can be closed after reading", EmptyFile.Close()) && Passed;

  mgui.Enable(false);
  Passed = EvaluateFalse("Open()", "missing file", "Opening a missing file for reading fails", File.Open("/tmp/UTFile_missing.txt", MFile::c_Read, false)) && Passed;
  mgui.Enable(true);

  MFile::Remove(FileName);
  MFile::Remove(FloatFileName);
  MFile::Remove(DelimiterFileName);
  MFile::Remove(EmptyFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test binary store I/O
bool UTFile::TestBinaryIO()
{
  bool Passed = true;

  MString FileName = "/tmp/UTFile_binary.dat";
  MFile::Remove(FileName);

  MFile File;
  Passed = EvaluateTrue("Open()", "binary write", "Binary files can be opened for writing", File.Open(FileName, MFile::c_Write, true)) && Passed;
  Passed = EvaluateTrue("IsBinary()", "binary write", "Binary files are marked as binary", File.IsBinary()) && Passed;

  MBinaryStore Output;
  Output.AddUInt8(1);
  Output.AddUInt8(2);
  Output.AddUInt8(3);
  Output.AddUInt8(255);
  File.Write(Output);
  File.Flush();

  Passed = EvaluateNear("GetFileLength()", "binary write", "Binary file length reflects the written byte count", File.GetFileLength(), 4.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Close()", "binary write", "Binary files can be closed after writing", File.Close()) && Passed;

  Passed = EvaluateTrue("Open()", "binary read", "Binary files can be reopened for reading", File.Open(FileName, MFile::c_Read, true)) && Passed;
  MBinaryStore Input;
  Passed = EvaluateTrue("Read(MBinaryStore)", "binary read", "Binary stores can be read back from disk", File.Read(Input, 10)) && Passed;
  Passed = EvaluateSize("Read(MBinaryStore)", "binary size", "The requested binary bytes are read back correctly", Input.GetArraySize(), 4) && Passed;
  Passed = EvaluateNear("Read(MBinaryStore)", "binary byte 0", "The first binary byte round-trips correctly", Input.GetArrayValue(0), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("Read(MBinaryStore)", "binary byte 1", "The second binary byte round-trips correctly", Input.GetArrayValue(1), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("Read(MBinaryStore)", "binary byte 2", "The third binary byte round-trips correctly", Input.GetArrayValue(2), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("Read(MBinaryStore)", "binary byte 3", "The fourth binary byte round-trips correctly", Input.GetArrayValue(3), 255.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetUncompressedFilePosition()", "binary read", "The uncompressed file position matches the consumed byte count", File.GetUncompressedFilePosition(), 4.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Close()", "binary read", "Binary files can be closed after reading", File.Close()) && Passed;

  MFile::Remove(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test gzip file I/O and compressed position helpers
bool UTFile::TestGzipIO()
{
  bool Passed = true;

  MString FileName = "/tmp/UTFile_zipped.txt.gz";
  MFile::Remove(FileName);

  MFile File;
  File.SetCompressionLevel(6);
  Passed = EvaluateTrue("Open()", "gzip write", "Gzip files can be opened for writing", File.Open(FileName, MFile::c_Write, false)) && Passed;
  Passed = EvaluateTrue("CheckFileExtension()", "gzip extension", "Gzip file names are recognized by extension", File.CheckFileExtension("gz")) && Passed;
  File.WriteLine("alpha");
  File.Write("beta");
  File.Write('\n');
  File.Write(3.5);
  File.Write('\n');
  MBinaryStore Output;
  Output.AddUInt8(65);
  Output.AddUInt8(66);
  File.Write(Output);
  Passed = EvaluateTrue("Close()", "gzip write", "Gzip files can be closed after writing", File.Close()) && Passed;

  Passed = EvaluateTrue("Open()", "gzip read", "Gzip files can be reopened for reading", File.Open(FileName, MFile::c_Read, false)) && Passed;
  Passed = EvaluateTrue("IsOpen()", "gzip read", "Reopened gzip files report IsOpen() == true", File.IsOpen()) && Passed;
  Passed = EvaluateNear("GetFileLength()", "gzip read", "Compressed gzip file length is nonzero after reopening", File.GetFileLength() > 0 ? 1.0 : 0.0, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetUncompressedFileLength()", "gzip read", "Uncompressed gzip file length is nonzero after reopening", File.GetUncompressedFileLength() > 0 ? 1.0 : 0.0, 1.0, 1e-12) && Passed;

  MString Line;
  Passed = EvaluateTrue("ReadLine(MString)", "gzip first line", "ReadLine(MString) works on gzip files", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "gzip first line", "The first gzip line round-trips correctly", Line, MString("alpha")) && Passed;
  Passed = EvaluateTrue("ReadLine(MString)", "gzip second line", "ReadLine(MString) can read the second gzip line", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "gzip second line", "The second gzip line round-trips correctly", Line, MString("beta")) && Passed;
  Passed = EvaluateTrue("ReadLine(MString)", "gzip third line", "ReadLine(MString) can read the third gzip line", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "gzip third line", "The third gzip line round-trips correctly", Line, MString("3.5")) && Passed;

  Passed = EvaluateTrue("Rewind()", "gzip read", "Gzip files can be rewound", File.Rewind()) && Passed;
  Passed = EvaluateNear("GetFilePosition()", "gzip rewind", "Compressed file position resets near the beginning after rewind", File.GetFilePosition(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetUncompressedFilePosition()", "gzip rewind", "Uncompressed file position resets to the beginning after rewind", File.GetUncompressedFilePosition(), 0.0, 1e-12) && Passed;

  File.Seek(static_cast<streampos>(6));
  Passed = EvaluateTrue("ReadLine(MString)", "gzip seek absolute", "ReadLine(MString) can continue after absolute gzip seeking", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "gzip seek absolute", "Absolute gzip seeking lands on the expected line", Line, MString("beta")) && Passed;

  Passed = EvaluateTrue("Rewind()", "gzip binary read", "Gzip files can be rewound before binary reads", File.Rewind()) && Passed;
  MBinaryStore Input;
  Passed = EvaluateTrue("Read(MBinaryStore)", "gzip binary read", "Binary reads from gzip files consume the requested bytes", File.Read(Input, 4)) && Passed;
  Passed = EvaluateSize("Read(MBinaryStore)", "gzip binary size", "Binary reads from gzip files return the requested byte count", Input.GetArraySize(), 4) && Passed;
  Passed = Evaluate("Read(MBinaryStore)", "gzip binary byte 0", "Binary reads from gzip files preserve the first byte", MString(static_cast<char>(Input.GetArrayValue(0))), MString("a")) && Passed;
  Passed = Evaluate("Read(MBinaryStore)", "gzip binary byte 3", "Binary reads from gzip files preserve later bytes", MString(static_cast<char>(Input.GetArrayValue(3))), MString("h")) && Passed;

  Passed = EvaluateTrue("Rewind()", "gzip first character", "Gzip files can be rewound before character reads", File.Rewind()) && Passed;
  char GzipCharacter = '\0';
  Passed = EvaluateTrue("Get(char)", "gzip first character", "Get(char) reads characters from gzip files", File.Get(GzipCharacter)) && Passed;
  Passed = Evaluate("Get(char)", "gzip first character", "Get(char) returns the expected first gzip character", MString(GzipCharacter), MString("a")) && Passed;

  Passed = EvaluateTrue("Rewind()", "gzip seek current", "Gzip files can be rewound before current-relative seeks", File.Rewind()) && Passed;
  File.Seek(6, ios_base::cur);
  Passed = EvaluateTrue("ReadLine(MString)", "gzip seek current", "ReadLine(MString) works after current-relative gzip seeking", File.ReadLine(Line)) && Passed;
  Passed = Evaluate("ReadLine(MString)", "gzip seek current", "Current-relative gzip seeking lands on the expected line", Line, MString("beta")) && Passed;

  MString FloatGzipFileName = "/tmp/UTFile_float.txt.gz";
  MFile::Remove(FloatGzipFileName);
  MFile FloatFile;
  FloatFile.SetCompressionLevel(6);
  Passed = EvaluateTrue("Open()", "gzip float write", "Gzip files can be created for float parsing", FloatFile.Open(FloatGzipFileName, MFile::c_Write, false)) && Passed;
  FloatFile.Write(MString("1.25 4.5\n"));
  Passed = EvaluateTrue("Close()", "gzip float write", "Gzip float files can be closed after writing", FloatFile.Close()) && Passed;
  Passed = EvaluateTrue("Open()", "gzip float read", "Gzip float files can be reopened for reading", FloatFile.Open(FloatGzipFileName, MFile::c_Read, false)) && Passed;
  float GzipFloat1 = 0.0F;
  float GzipFloat2 = 0.0F;
  Passed = EvaluateTrue("Get(float)", "gzip first float", "Get(float) parses the first float from gzip files", FloatFile.Get(GzipFloat1)) && Passed;
  Passed = EvaluateNear("Get(float)", "gzip first float", "The first gzip float is parsed correctly", GzipFloat1, 1.25, 1e-6) && Passed;
  Passed = EvaluateTrue("Get(float)", "gzip second float", "Get(float) parses the second float from gzip files", FloatFile.Get(GzipFloat2)) && Passed;
  Passed = EvaluateNear("Get(float)", "gzip second float", "The second gzip float is parsed correctly", GzipFloat2, 4.5, 1e-6) && Passed;
  Passed = EvaluateTrue("Close()", "gzip float read", "Gzip float files can be closed after reading", FloatFile.Close()) && Passed;

  MString EmptyGzipFileName = "/tmp/UTFile_empty.txt.gz";
  MFile::Remove(EmptyGzipFileName);
  MFile EmptyFile;
  EmptyFile.SetCompressionLevel(6);
  Passed = EvaluateTrue("Open()", "empty gzip write", "Empty gzip files can be created", EmptyFile.Open(EmptyGzipFileName, MFile::c_Write, false)) && Passed;
  Passed = EvaluateTrue("Close()", "empty gzip write", "Empty gzip files can be closed after creation", EmptyFile.Close()) && Passed;
  Passed = EvaluateTrue("Open()", "empty gzip read", "Empty gzip files can be reopened for reading", EmptyFile.Open(EmptyGzipFileName, MFile::c_Read, false)) && Passed;
  Passed = EvaluateNear("GetFileLength()", "empty gzip", "Empty gzip files have a nonnegative compressed length", EmptyFile.GetFileLength() >= 0 ? 1.0 : 0.0, 1.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Get(char)", "empty gzip", "Reading a character from an empty gzip file fails", EmptyFile.Get(GzipCharacter)) && Passed;
  Passed = EvaluateTrue("Close()", "empty gzip read", "Empty gzip files can be closed after reading", EmptyFile.Close()) && Passed;

  Passed = EvaluateTrue("Close()", "gzip read", "Gzip files can be closed after reading", File.Close()) && Passed;
  MFile::Remove(FileName);
  MFile::Remove(FloatGzipFileName);
  MFile::Remove(EmptyGzipFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTFile Test;
  return Test.Run() == true ? 0 : 1;
}
