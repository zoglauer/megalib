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

// Standard libs:
#include <fstream>
using namespace std;


//! Test parser exposing Parse() behavior
class UTParser_Test : public MParser
{
public:
  //! Default constructor
  UTParser_Test(char Separator = ' ', bool AllowComposed = false) : MParser(Separator, AllowComposed), m_ParseCalls(0), m_ParseResult(true) {}

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

  MString FileName = "/tmp/UTParser_open.txt";
  {
    ofstream Out(FileName.Data());
    Out<<"alpha beta"<<endl;
    Out<<"Sphere.Source value"<<endl;
    Out<<"# comment"<<endl;
  }

  UTParser_Test Parser(' ', true);
  Passed = EvaluateTrue("Open()", "read mode", "Opening a parser in read mode succeeds on valid files", Parser.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("Parse()", "read mode", "Parse is called exactly once during read-mode opening", Parser.m_ParseCalls, 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNLines()", "read mode", "All lines are tokenized and stored during read-mode opening", Parser.GetNLines(), 3.0, 1e-12) && Passed;

  UTParser_Test WriteParser(' ', true);
  Passed = EvaluateTrue("Open()", "write mode", "Opening a parser in write mode succeeds", WriteParser.Open("/tmp/UTParser_write.txt", MFile::c_Write)) && Passed;
  Passed = EvaluateNear("Parse()", "write mode", "Parse is not called during write-mode opening", WriteParser.m_ParseCalls, 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Close()", "write mode", "Write-mode parser files can be closed", WriteParser.Close()) && Passed;

  UTParser_Test FailingParser(' ', true);
  FailingParser.m_ParseResult = false;
  mlog.Enable(false);
  Passed = EvaluateFalse("Open()", "parse failure", "Open returns false when Parse() fails", FailingParser.Open(FileName, MFile::c_Read)) && Passed;
  mlog.Enable(true);
  Passed = EvaluateNear("Parse()", "parse failure", "Parse is still invoked when it reports failure", FailingParser.m_ParseCalls, 1.0, 1e-12) && Passed;

  mgui.Enable(false);
  mlog.Enable(false);
  Passed = EvaluateFalse("Open()", "missing file", "Opening a missing parser input file fails", Parser.Open("/tmp/UTParser_missing.txt", MFile::c_Read)) && Passed;
  mlog.Enable(true);
  mgui.Enable(true);

  Parser.Close();
  FailingParser.Close();
  MFile::Remove(FileName);
  MFile::Remove("/tmp/UTParser_write.txt");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test stored line access and mutation helpers
bool UTParser::TestLineAccess()
{
  bool Passed = true;

  MString FileName = "/tmp/UTParser_lines.txt";
  {
    ofstream Out(FileName.Data());
    Out<<"alpha beta"<<endl;
    Out<<"Sphere.Source value"<<endl;
    Out<<"# comment"<<endl;
  }

  UTParser_Test Parser(' ', true);
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

  merr.Enable(false);
  Passed = EvaluateTrue("GetTokenizerAt()", "out of bounds", "Out-of-bounds tokenizer access returns null", Parser.GetTokenizerAt(99) == 0) && Passed;
  Passed = Evaluate("GetLine()", "out of bounds", "Out-of-bounds line access returns an empty string", Parser.GetLine(99), MString("")) && Passed;
  merr.Enable(true);

  mlog.Enable(false);
  Parser.Typo(0, "test typo");
  mlog.Enable(true);

  Parser.Close();
  MFile::Remove(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test streaming helpers TokenizeLine() and GetFloat()
bool UTParser::TestStreamingHelpers()
{
  bool Passed = true;

  MString TokenFileName = "/tmp/UTParser_tokenize.txt";
  {
    ofstream Out(TokenFileName.Data());
    Out<<"first second"<<endl;
    Out<<"Sphere.Source value"<<endl;
  }

  UTParser_Test Parser(' ', true);
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

  MString FloatFileName = "/tmp/UTParser_float.txt";
  {
    ofstream Out(FloatFileName.Data());
    Out<<"1.5 2.75"<<endl;
  }

  UTParser_Test FloatParser;
  Passed = EvaluateTrue("Open()", "float streaming", "Opening a parser for GetFloat() succeeds", FloatParser.Open(FloatFileName, MFile::c_Read)) && Passed;
  float Value = 0.0F;
  Passed = EvaluateTrue("GetFloat()", "first float", "GetFloat() reads the first float token", FloatParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "first float", "GetFloat() parses the first float token", Value, 1.5, 1e-6) && Passed;
  Passed = EvaluateTrue("GetFloat()", "second float", "GetFloat() reads the second float token", FloatParser.GetFloat(Value)) && Passed;
  Passed = EvaluateNear("GetFloat()", "second float", "GetFloat() parses the second float token", Value, 2.75, 1e-6) && Passed;
  Passed = EvaluateFalse("GetFloat()", "end of file", "GetFloat() returns false at end of file", FloatParser.GetFloat(Value)) && Passed;

  Parser.Close();
  FloatParser.Close();
  MFile::Remove(TokenFileName);
  MFile::Remove(FloatFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test parser edge cases and API contracts
bool UTParser::TestEdgeCases()
{
  bool Passed = true;

  UTParser_Test NamedParser;
  NamedParser.SetFileName("relative/test.par");
  Passed = Evaluate("GetFileName()", "set name", "SetFileName() updates the inherited parser file name", NamedParser.GetFileName(), MString("relative/test.par")) && Passed;

  MString EmptyFileName = "/tmp/UTParser_empty_lines.txt";
  {
    ofstream Out(EmptyFileName.Data());
    Out<<"first"<<endl;
    Out<<endl;
    Out<<"third"<<endl;
  }

  UTParser_Test EmptyParser(' ', true);
  Passed = EvaluateTrue("Open()", "empty lines", "Opening a parser with genuine empty lines succeeds", EmptyParser.Open(EmptyFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "empty lines", "Genuine empty lines inside the file are preserved", EmptyParser.GetNLines(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "empty lines", "A genuine empty line remains empty after parsing", EmptyParser.GetLine(1), MString("")) && Passed;

  EmptyParser.Rewind();
  MTokenizer EmptyLineTokenizer;
  Passed = EvaluateTrue("TokenizeLine()", "empty line", "TokenizeLine() returns true for a real empty line inside the file", EmptyParser.TokenizeLine(EmptyLineTokenizer, false)) && Passed;
  Passed = EvaluateNear("GetNTokens()", "empty line", "A real empty line tokenizes to zero tokens", EmptyLineTokenizer.GetNTokens(), 1.0, 1e-12) && Passed;
  EmptyParser.Close();
  MFile::Remove(EmptyFileName);

  MString RewindFileName = "/tmp/UTParser_rewind.txt";
  {
    ofstream Out(RewindFileName.Data());
    Out<<"alpha beta"<<endl;
    Out<<"gamma delta"<<endl;
  }

  UTParser_Test RewindParser(' ', true);
  Passed = EvaluateTrue("Open()", "rewind", "Opening a parser for rewind testing succeeds", RewindParser.Open(RewindFileName, MFile::c_Read)) && Passed;
  MTokenizer RewindTokenizer;
  Passed = EvaluateTrue("TokenizeLine()", "rewind first pass", "The first line can be tokenized before rewinding", RewindParser.TokenizeLine(RewindTokenizer, false)) && Passed;
  Passed = EvaluateTrue("Rewind()", "rewind", "Rewind() succeeds for parser streams", RewindParser.Rewind()) && Passed;
  Passed = EvaluateTrue("TokenizeLine()", "rewind second pass", "After Rewind() the first line can be tokenized again", RewindParser.TokenizeLine(RewindTokenizer, false)) && Passed;
  Passed = Evaluate("GetTokenAt()", "rewind second pass", "Rewinding restores the file position to the beginning", RewindTokenizer.GetTokenAt(0), MString("alpha")) && Passed;
  RewindParser.Close();
  MFile::Remove(RewindFileName);

  MString FirstFileName = "/tmp/UTParser_reopen_first.txt";
  {
    ofstream Out(FirstFileName.Data());
    Out<<"one"<<endl;
    Out<<"two"<<endl;
  }
  MString SecondFileName = "/tmp/UTParser_reopen_second.txt";
  {
    ofstream Out(SecondFileName.Data());
    Out<<"three"<<endl;
  }

  UTParser_Test ReopenParser(' ', true);
  Passed = EvaluateTrue("Open()", "reopen first", "Opening the first parser input succeeds", ReopenParser.Open(FirstFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "reopen first", "The first file contributes its expected line count", ReopenParser.GetNLines(), 2.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Open()", "reopen second", "Reopening the same parser on a second file succeeds", ReopenParser.Open(SecondFileName, MFile::c_Read)) && Passed;
  Passed = EvaluateNear("GetNLines()", "reopen second", "Reopening replaces the old stored lines instead of accumulating them", ReopenParser.GetNLines(), 1.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "reopen second", "After reopening only the new file contents remain stored", ReopenParser.GetLine(0), MString("three")) && Passed;
  ReopenParser.Close();
  MFile::Remove(FirstFileName);
  MFile::Remove(SecondFileName);

  MString AddLineFileName = "/tmp/UTParser_addline.txt";
  {
    ofstream Out(AddLineFileName.Data());
    Out<<"existing line"<<endl;
    Out<<"second line"<<endl;
  }

  UTParser_Test AddLineParser(' ', true);
  Passed = EvaluateTrue("Open()", "add line read mode", "Opening a parser in read mode for AddLine() contract testing succeeds", AddLineParser.Open(AddLineFileName, MFile::c_Read)) && Passed;
  merr.Enable(false);
  Passed = EvaluateTrue("AddLine()", "read mode", "AddLine() is a read-mode helper and succeeds there", AddLineParser.AddLine("stored line")) && Passed;
  merr.Enable(true);
  Passed = EvaluateNear("GetNLines()", "add line read mode", "AddLine() appends one more tokenized line in read mode", AddLineParser.GetNLines(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetLine()", "add line read mode", "AddLine() stores the appended text at the end", AddLineParser.GetLine(2), MString("stored line")) && Passed;
  AddLineParser.Close();
  MFile::Remove(AddLineFileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTParser Test;
  return Test.Run() == true ? 0 : 1;
}
