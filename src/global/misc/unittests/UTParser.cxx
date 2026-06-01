/*
 * UTParser.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MParser.h"
#include "MUnitTest.h"
#include "MStreams.h"

//! Unit test class for the MParser helper
class UTParser : public MUnitTest
{
public:
  //! Default constructor
  UTParser() : MUnitTest("UTParser") {}
  //! Default destructor
  virtual ~UTParser() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test parser exposing Parse() behavior
  class ParserTest : public MParser
  {
  public:
    //! Default constructor
    ParserTest(char Separator = ' ', bool AllowComposed = false) : MParser(Separator, AllowComposed), m_ParseCalls(0), m_ParseResult(true) {}

    //! Number of Parse() calls
    int m_ParseCalls;
    //! Parse() return value
    bool m_ParseResult;

  protected:
    //! Count parse calls and return the configured result
    virtual bool Parse()
    {
      ++m_ParseCalls;
      return m_ParseResult;
    }
  };

  //! Test opening and parsing files
  bool TestOpenAndParse();
  //! Test stored line access and mutation helpers
  bool TestLineAccess();
  //! Test streaming helpers TokenizeLine() and GetFloat()
  bool TestStreamingHelpers();
  //! Test parser edge cases and API contracts
  bool TestEdgeCases();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTParser::Run()
{
  bool AllPassed = true;

  AllPassed = TestOpenAndParse() && AllPassed;
  AllPassed = TestLineAccess() && AllPassed;
  AllPassed = TestStreamingHelpers() && AllPassed;
  AllPassed = TestEdgeCases() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test opening and parsing files
bool UTParser::TestOpenAndParse()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "open temp dir", "The temporary directory for parser open tests can be created", PrepareTemporaryDirectory("open")) && Passed;
  MString FileName = GetTemporaryDirectoryName("open") + "/input.txt";
  Passed = EvaluateTrue("WriteTextFile()", "open input", "The representative parser input file can be written",
                        WriteTextFile(FileName, "alpha beta\nSphere.Source value\n# comment\n")) && Passed;

  ParserTest Parser(' ', true);
  Passed = EvaluateTrue("Open()", "read mode", "Opening a parser in read mode succeeds on valid files", Parser.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("Parse()", "read mode", "Parse is called exactly once during read-mode opening", Parser.m_ParseCalls, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNLines()", "read mode", "All lines are tokenized and stored during read-mode opening", Parser.GetNLines(), 3.0, 1e-12) && Passed;

  ParserTest WriteParser(' ', true);
  Passed = EvaluateTrue("Open()", "write mode", "Opening a parser in write mode succeeds", WriteParser.Open(GetTemporaryDirectoryName("open") + "/write.txt", MFile::c_Write)) && Passed;
  Passed = EvaluateNear("Parse()", "write mode", "Parse is not called during write-mode opening", WriteParser.m_ParseCalls, 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Close()", "write mode", "Write-mode parser files can be closed", WriteParser.Close()) && Passed;

  ParserTest FailingParser(' ', true);
  FailingParser.m_ParseResult = false;
  DisableDefaultStreams();
  Passed = EvaluateFalse("Open()", "parse failure", "Open returns false when Parse() fails", FailingParser.Open(FileName, MFile::c_Read)) && Passed;
  EnableDefaultStreams();
  Passed = EvaluateNear("Parse()", "parse failure", "Parse is still invoked when it reports failure", FailingParser.m_ParseCalls, 1.0, 1e-12) && Passed;

  DisableDefaultStreams();
  Passed = EvaluateFalse("Open()", "missing file", "Opening a missing parser input file fails", Parser.Open(GetTemporaryDirectoryName("open") + "/missing.txt", MFile::c_Read)) && Passed;
  EnableDefaultStreams();

  Parser.Close();
  FailingParser.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test stored line access and mutation helpers
bool UTParser::TestLineAccess()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "line temp dir", "The temporary directory for parser line-access tests can be created", PrepareTemporaryDirectory("lines")) && Passed;
  MString FileName = GetTemporaryDirectoryName("lines") + "/input.txt";
  Passed = EvaluateTrue("WriteTextFile()", "line input", "The representative parser line-access file can be written",
                        WriteTextFile(FileName, "alpha beta\nSphere.Source value\n# comment\n")) && Passed;

  ParserTest Parser(' ', true);
  Passed = EvaluateTrue("Open()", "line access", "Opening a parser for line access succeeds", Parser.Open(FileName, MFile::c_Read)) && Passed;

  Passed = Evaluate("GetLine()", "first line", "GetLine returns the original first line text", Parser.GetLine(0), MString("alpha beta")) && Passed;
  Passed = Evaluate("GetLine()", "second line", "GetLine returns the original second line text", Parser.GetLine(1), MString("Sphere.Source value")) && Passed;

  MTokenizer* First = Parser.GetTokenizerAt(0);
  Passed = EvaluateTrue("GetTokenizerAt()", "first line", "GetTokenizerAt returns a tokenizer for valid indices", First != 0) && Passed;
  if (First != 0) {
    Passed = EvaluateNear("GetNTokens()", "first line", "The first line is tokenized into two tokens", First->GetNTokens(), 2.0, 1e-12) && Passed;
    Passed = Evaluate("GetTokenAt()", "first line", "The first token is preserved", First->GetTokenAt(0), MString("alpha")) && Passed;
  }

  MTokenizer* Second = Parser.GetTokenizerAt(1);
  Passed = EvaluateTrue("GetTokenizerAt()", "composed line", "The second line tokenizer exists", Second != 0) && Passed;
  if (Second != 0) {
    Passed = EvaluateNear("GetNTokens()", "composed line", "AllowComposed(true) splits the first token on '.'", Second->GetNTokens(), 3.0, 1e-12) && Passed;
    Passed = Evaluate("GetTokenAt()", "composed line", "The composed prefix is preserved", Second->GetTokenAt(0), MString("Sphere")) && Passed;
    Passed = Evaluate("GetTokenAt()", "composed line", "The composed suffix is preserved", Second->GetTokenAt(1), MString("Source")) && Passed;
  }

  MTokenizer* Third = Parser.GetTokenizerAt(2);
  Passed = EvaluateTrue("GetTokenizerAt()", "comment line", "Comment lines are still stored as tokenizers", Third != 0) && Passed;
  if (Third != 0) {
    Passed = EvaluateNear("GetNTokens()", "comment line", "A comment-only line has zero tokens", Third->GetNTokens(), 0.0, 1e-12) && Passed;
  }

  Passed = EvaluateTrue("InsertLineBefore()", "insert line", "InsertLineBefore can add a tokenized line", Parser.InsertLineBefore("inserted line", 1)) && Passed;
  Passed = EvaluateNear("GetNLines()", "insert line", "Inserting a line increases the stored line count", Parser.GetNLines(), 4.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "insert line", "Inserted lines are retrievable at the requested position", Parser.GetLine(1), MString("inserted line")) && Passed;

  Passed = EvaluateTrue("RemoveLine()", "remove line", "RemoveLine removes the requested stored line", Parser.RemoveLine(1)) && Passed;
  Passed = EvaluateNear("GetNLines()", "remove line", "Removing a line reduces the stored line count", Parser.GetNLines(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "remove line", "After removal the original second line is visible again", Parser.GetLine(1), MString("Sphere.Source value")) && Passed;

  DisableDefaultStreams();
  Passed = EvaluateTrue("GetTokenizerAt()", "out of bounds", "Out-of-bounds tokenizer access returns null", Parser.GetTokenizerAt(99) == 0) && Passed;
  Passed = Evaluate("GetLine()", "out of bounds", "Out-of-bounds line access returns an empty string", Parser.GetLine(99), MString("")) && Passed;
  EnableDefaultStreams();

  DisableDefaultStreams();
  Parser.Typo(0, "test typo");
  EnableDefaultStreams();

  Parser.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test streaming helpers TokenizeLine() and GetFloat()
bool UTParser::TestStreamingHelpers()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "streaming temp dir", "The temporary directory for parser streaming tests can be created", PrepareTemporaryDirectory("streaming")) && Passed;
  MString TokenFileName = GetTemporaryDirectoryName("streaming") + "/tokenize.txt";
  Passed = EvaluateTrue("WriteTextFile()", "tokenize input", "The representative parser tokenize file can be written",
                        WriteTextFile(TokenFileName, "first second\nSphere.Source value\n")) && Passed;

  ParserTest Parser(' ', true);
  Passed = EvaluateTrue("Open()", "tokenize line", "Opening a parser for TokenizeLine() succeeds", Parser.Open(TokenFileName, MFile::c_Read)) && Passed;

  MTokenizer Slow;
  Passed = EvaluateTrue("TokenizeLine()", "slow mode", "TokenizeLine() can tokenize one line in normal mode", Parser.TokenizeLine(Slow, false)) && Passed;
  Passed = EvaluateNear("GetNTokens()", "slow mode", "TokenizeLine() returns the expected token count in normal mode", Slow.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "slow mode", "TokenizeLine() preserves the first token in normal mode", Slow.GetTokenAt(0), MString("first")) && Passed;

  MTokenizer Fast;
  Passed = EvaluateTrue("TokenizeLine()", "fast mode", "TokenizeLine() can tokenize one line in fast mode", Parser.TokenizeLine(Fast, true)) && Passed;
  Passed = EvaluateNear("GetNTokens()", "fast mode", "Fast tokenization keeps the composed token intact", Fast.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "fast mode", "Fast tokenization preserves the unsplit first token", Fast.GetTokenAt(0), MString("Sphere.Source")) && Passed;

  Passed = EvaluateFalse("TokenizeLine()", "end of file", "TokenizeLine() returns false at end of file", Parser.TokenizeLine(Fast, false)) && Passed;

  MString FloatFileName = GetTemporaryDirectoryName("streaming") + "/float.txt";
  Passed = EvaluateTrue("WriteTextFile()", "float input", "The representative parser float file can be written", WriteTextFile(FloatFileName, "1.5 2.75\n")) && Passed;

  ParserTest FloatParser;
  Passed = EvaluateTrue("Open()", "float streaming", "Opening a parser for GetFloat() succeeds", FloatParser.Open(FloatFileName, MFile::c_Read)) && Passed;
  float Value = 0.0F;
  Passed = EvaluateTrue("GetFloat()", "first float", "GetFloat() reads the first float token", FloatParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "first float", "GetFloat() parses the first float token", Value, 1.5, 1e-6) && Passed;
  Passed = EvaluateTrue("GetFloat()", "second float", "GetFloat() reads the second float token", FloatParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "second float", "GetFloat() parses the second float token", Value, 2.75, 1e-6) && Passed;
  Passed = EvaluateFalse("GetFloat()", "end of file", "GetFloat() returns false at end of file", FloatParser.GetFloat(Value)) && Passed;

  MString ResponseStyleFileName = GetTemporaryDirectoryName("streaming") + "/response_style.txt";
  Passed = EvaluateTrue("WriteTextFile()", "response-style input", "The representative parser response-style file can be written",
                        WriteTextFile(ResponseStyleFileName, "AXIS BIN=4 TYPE=linear\nVALUES\n1.0 2.5\n3.75\n")) && Passed;

  ParserTest ResponseParser(' ', false);
  Passed = EvaluateTrue("Open()", "response-style streaming", "Opening a parser for response-style streaming succeeds", ResponseParser.Open(ResponseStyleFileName, MFile::c_Read)) && Passed;
  MTokenizer Header;
  Passed = EvaluateTrue("TokenizeLine()", "response-style streaming", "Fast tokenization works in response-style streaming loops", ResponseParser.TokenizeLine(Header, true)) && Passed;
  Passed = Evaluate("GetTokenAt()", "response-style streaming", "Fast tokenization preserves the response-style keyword", Header.GetTokenAt(0), MString("AXIS")) && Passed;
  Passed = Evaluate("GetTokenAt()", "response-style streaming", "Fast tokenization preserves later key/value tokens", Header.GetTokenAt(2), MString("TYPE=linear")) && Passed;
  Passed = EvaluateTrue("TokenizeLine()", "response-style values marker", "Fast tokenization can read the values marker before switching to float streaming", ResponseParser.TokenizeLine(Header, true)) && Passed;
  Passed = Evaluate("GetTokenAt()", "response-style values marker", "Fast tokenization preserves the values marker token", Header.GetTokenAt(0), MString("VALUES")) && Passed;
  Passed = EvaluateTrue("GetFloat()", "response-style float first", "GetFloat reads the first numeric token from a response-style file", ResponseParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "response-style float first", "The first response-style numeric token is parsed correctly", Value, 1.0, 1e-6) && Passed;
  Passed = EvaluateTrue("GetFloat()", "response-style float second", "GetFloat continues through multi-line numeric content", ResponseParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "response-style float second", "The second response-style numeric token is parsed correctly", Value, 2.5, 1e-6) && Passed;
  Passed = EvaluateTrue("GetFloat()", "response-style float third", "GetFloat reads the third numeric token from the stream", ResponseParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "response-style float third", "The third response-style numeric token is parsed correctly", Value, 3.75, 1e-6) && Passed;
  Passed = EvaluateFalse("GetFloat()", "response-style end", "GetFloat reaches end-of-file cleanly in response-style streaming", ResponseParser.GetFloat(Value)) && Passed;
  ResponseParser.Close();

  Parser.Close();
  FloatParser.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parser edge cases and API contracts
bool UTParser::TestEdgeCases()
{
  bool Passed = true;

  Passed = EvaluateTrue("PrepareTemporaryDirectory()", "edge temp dir", "The temporary directory for parser edge-case tests can be created", PrepareTemporaryDirectory("edge")) && Passed;

  ParserTest NamedParser;
  NamedParser.SetFileName("relative/test.par");
  Passed = Evaluate("GetFileName()", "set name", "SetFileName() updates the inherited parser file name", NamedParser.GetFileName(), MString("relative/test.par")) && Passed;

  MString EmptyFileName = GetTemporaryDirectoryName("edge") + "/empty_lines.txt";
  Passed = EvaluateTrue("WriteTextFile()", "empty-line input", "The representative empty-line parser file can be written", WriteTextFile(EmptyFileName, "first\n\nthird\n")) && Passed;

  ParserTest EmptyParser(' ', true);
  Passed = EvaluateTrue("Open()", "empty lines", "Opening a parser with genuine empty lines succeeds", EmptyParser.Open(EmptyFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "empty lines", "Genuine empty lines inside the file are preserved", EmptyParser.GetNLines(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "empty lines", "A genuine empty line remains empty after parsing", EmptyParser.GetLine(1), MString("")) && Passed;

  EmptyParser.Rewind();
  MTokenizer EmptyLineTokenizer;
  Passed = EvaluateTrue("TokenizeLine()", "empty line", "TokenizeLine() returns true for a real empty line inside the file", EmptyParser.TokenizeLine(EmptyLineTokenizer, false)) && Passed;
  Passed = EvaluateNear("GetNTokens()", "empty line", "A real empty line currently tokenizes into one empty token", EmptyLineTokenizer.GetNTokens(), 1.0, 1e-12) && Passed;
  EmptyParser.Close();

  MString RewindFileName = GetTemporaryDirectoryName("edge") + "/rewind.txt";
  Passed = EvaluateTrue("WriteTextFile()", "rewind input", "The representative rewind parser file can be written", WriteTextFile(RewindFileName, "alpha beta\ngamma delta\n")) && Passed;

  ParserTest RewindParser(' ', true);
  Passed = EvaluateTrue("Open()", "rewind", "Opening a parser for rewind testing succeeds", RewindParser.Open(RewindFileName, MFile::c_Read)) && Passed;
  MTokenizer RewindTokenizer;
  Passed = EvaluateTrue("TokenizeLine()", "rewind first pass", "The first line can be tokenized before rewinding", RewindParser.TokenizeLine(RewindTokenizer, false)) && Passed;
  Passed = EvaluateTrue("Rewind()", "rewind", "Rewind() succeeds for parser streams", RewindParser.Rewind()) && Passed;
  Passed = EvaluateTrue("TokenizeLine()", "rewind second pass", "After Rewind() the first line can be tokenized again", RewindParser.TokenizeLine(RewindTokenizer, false)) && Passed;
  Passed = Evaluate("GetTokenAt()", "rewind second pass", "Rewinding restores the file position to the beginning", RewindTokenizer.GetTokenAt(0), MString("alpha")) && Passed;
  RewindParser.Close();

  MString FirstFileName = GetTemporaryDirectoryName("edge") + "/reopen_first.txt";
  Passed = EvaluateTrue("WriteTextFile()", "reopen first input", "The representative first reopen parser file can be written", WriteTextFile(FirstFileName, "one\ntwo\n")) && Passed;
  MString SecondFileName = GetTemporaryDirectoryName("edge") + "/reopen_second.txt";
  Passed = EvaluateTrue("WriteTextFile()", "reopen second input", "The representative second reopen parser file can be written", WriteTextFile(SecondFileName, "three\n")) && Passed;

  ParserTest ReopenParser(' ', true);
  Passed = EvaluateTrue("Open()", "reopen first", "Opening the first parser input succeeds", ReopenParser.Open(FirstFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "reopen first", "The first file contributes its expected line count", ReopenParser.GetNLines(), 2.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Open()", "reopen second", "Reopening the same parser on a second file succeeds", ReopenParser.Open(SecondFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "reopen second", "Reopening replaces the old stored lines instead of accumulating them", ReopenParser.GetNLines(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "reopen second", "After reopening only the new file contents remain stored", ReopenParser.GetLine(0), MString("three")) && Passed;
  ReopenParser.Close();

  MString AddLineFileName = GetTemporaryDirectoryName("edge") + "/addline.txt";
  Passed = EvaluateTrue("WriteTextFile()", "add-line input", "The representative add-line parser file can be written", WriteTextFile(AddLineFileName, "existing line\nsecond line\n")) && Passed;

  ParserTest AddLineParser(' ', true);
  Passed = EvaluateTrue("Open()", "add line read mode", "Opening a parser in read mode for AddLine() contract testing succeeds", AddLineParser.Open(AddLineFileName, MFile::c_Read)) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateTrue("AddLine()", "read mode", "AddLine() is a read-mode helper and succeeds there", AddLineParser.AddLine("stored line")) && Passed;
  EnableDefaultStreams();
  Passed = EvaluateNear("GetNLines()", "add line read mode", "AddLine() appends one more tokenized line in read mode", AddLineParser.GetNLines(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "add line read mode", "AddLine() stores the appended text at the end", AddLineParser.GetLine(2), MString("stored line")) && Passed;
  AddLineParser.Close();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParser Test;
  return Test.Run() == true ? 0 : 1;
}
