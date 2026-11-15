/*
 * UTDDebugInfo.cxx
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
#include <functional>
#include <iostream>
#include <sstream>

// POSIX libs: process control has no equivalent in the C++ standard library
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include <TSystem.h>

// MEGAlib libs:
#include "MDDebugInfo.h"
#include "MTokenizer.h"
#include "MUnitTest.h"
#include "UTGeomegaShared.h"


////////////////////////////////////////////////////////////////////////////////


//! Unit test of the line of a setup file with its origin MDDebugInfo
class UTDDebugInfo : public UTGeomegaBase
{
public:
  //! Default constructor
  UTDDebugInfo() : UTGeomegaBase("UTDDebugInfo") {}
  //! Default destructor
  virtual ~UTDDebugInfo() {}

  //! Run all tests
  virtual bool Run();

private:

  //! Test the constructors and the getters
  bool TestConstruction();
  //! Test the setters
  bool TestSetters();
  //! Test copy and assignment
  bool TestCopy();
  //! Test Contains and Prepend
  bool TestContainsAndPrepend();
  //! Test the replacement of all occurrences
  bool TestReplaceAll();
  //! Test the replacement of whole words
  bool TestReplaceWholeWords();
  //! Test the replacement of the first occurrence with a number
  bool TestReplaceFirst();
  //! Test the two tokenizers
  bool TestTokenizers();
  //! Test the validity flag of the tokenizers
  bool TestTokenizerValidity();
  //! Test the error message
  bool TestError();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTDDebugInfo::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestSetters() && Passed;
  Passed = TestCopy() && Passed;
  Passed = TestContainsAndPrepend() && Passed;
  Passed = TestReplaceAll() && Passed;
  Passed = TestReplaceWholeWords() && Passed;
  Passed = TestReplaceFirst() && Passed;
  Passed = TestTokenizers() && Passed;
  Passed = TestTokenizerValidity() && Passed;
  Passed = TestError() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors and the getters
bool UTDDebugInfo::TestConstruction()
{
  bool Passed = true;

  MDDebugInfo Default;
  Passed = EvaluateTrue("MDDebugInfo()", "default", "The text is empty", Default.GetText() == "") && Passed;
  Passed = EvaluateTrue("MDDebugInfo()", "default", "The file name is empty", Default.GetFileName() == "") && Passed;
  Passed = Evaluate("MDDebugInfo()", "default", "The line is 0", Default.GetLine(), 0) && Passed;
  Passed = EvaluateTrue("MDDebugInfo()", "default", "The tokenizer is valid", Default.IsTokenizerValid()) && Passed;
  Passed = Evaluate("MDDebugInfo()", "default", "The tokenizer has no tokens", Default.GetTokenizer().GetNTokens(), 0U) && Passed;

  MDDebugInfo Standard("A.Position 1 2 3", "File.geo", 17);
  Passed = EvaluateTrue("MDDebugInfo(Text, File, Line)", "text", "The text is stored", Standard.GetText() == "A.Position 1 2 3") && Passed;
  Passed = EvaluateTrue("MDDebugInfo(Text, File, Line)", "file", "The file name is stored", Standard.GetFileName() == "File.geo") && Passed;
  Passed = Evaluate("MDDebugInfo(Text, File, Line)", "line", "The line is stored", Standard.GetLine(), 17) && Passed;

  MDDebugInfo Negative("", "", -1);
  Passed = Evaluate("MDDebugInfo(Text, File, Line)", "negative line", "A negative line is kept", Negative.GetLine(), -1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the setters
bool UTDDebugInfo::TestSetters()
{
  bool Passed = true;

  MDDebugInfo Info("Old", "OldFile", 1);
  Info.SetText("New");
  Info.SetFileName("NewFile");
  Info.SetLine(2);
  Passed = EvaluateTrue("SetText()", "New", "The text is replaced", Info.GetText() == "New") && Passed;
  Passed = EvaluateTrue("SetFileName()", "NewFile", "The file name is replaced", Info.GetFileName() == "NewFile") && Passed;
  Passed = Evaluate("SetLine()", "2", "The line is replaced", Info.GetLine(), 2) && Passed;

  Info.SetText("");
  Passed = EvaluateTrue("SetText()", "empty", "The text can be empty", Info.GetText() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy and assignment
bool UTDDebugInfo::TestCopy()
{
  bool Passed = true;

  MDDebugInfo Original("A 1 2", "File", 3);
  Passed = Evaluate("GetTokenizer()", "original", "The original has three tokens", Original.GetTokenizer().GetNTokens(), 3U) && Passed;

  MDDebugInfo Copy(Original);
  Passed = EvaluateTrue("MDDebugInfo(Copy)", "text", "The text is copied", Copy.GetText() == "A 1 2") && Passed;
  Passed = EvaluateTrue("MDDebugInfo(Copy)", "file", "The file name is copied", Copy.GetFileName() == "File") && Passed;
  Passed = Evaluate("MDDebugInfo(Copy)", "line", "The line is copied", Copy.GetLine(), 3) && Passed;
  Passed = Evaluate("MDDebugInfo(Copy)", "tokenizer", "The copy has its own valid tokens", Copy.GetTokenizer().GetNTokens(), 3U) && Passed;

  // The copy is independent:
  Copy.SetText("B");
  Passed = EvaluateTrue("MDDebugInfo(Copy)", "changed copy", "The original text stays", Original.GetText() == "A 1 2") && Passed;
  Passed = Evaluate("MDDebugInfo(Copy)", "changed copy", "The tokens of the original stay", Original.GetTokenizer().GetNTokens(), 3U) && Passed;
  Passed = Evaluate("MDDebugInfo(Copy)", "changed copy", "The copy has the new tokens", Copy.GetTokenizer().GetNTokens(), 1U) && Passed;

  // An assignment replaces everything, also the tokens which have already been made:
  MDDebugInfo Assigned("X Y Z W", "Other", 9);
  Passed = Evaluate("GetTokenizer()", "before assignment", "The tokens are made", Assigned.GetTokenizer().GetNTokens(), 4U) && Passed;
  Assigned = Original;
  Passed = EvaluateTrue("operator=", "text", "The text is copied", Assigned.GetText() == "A 1 2") && Passed;
  Passed = EvaluateTrue("operator=", "file", "The file name is copied", Assigned.GetFileName() == "File") && Passed;
  Passed = Evaluate("operator=", "line", "The line is copied", Assigned.GetLine(), 3) && Passed;
  Passed = Evaluate("operator=", "tokenizer", "The tokens are the new ones", Assigned.GetTokenizer().GetNTokens(), 3U) && Passed;

  // Self assignment:
  MDDebugInfo& Reference = Assigned;
  Assigned = Reference;
  Passed = EvaluateTrue("operator=", "self", "The text stays", Assigned.GetText() == "A 1 2") && Passed;
  Passed = Evaluate("operator=", "self", "The tokens stay", Assigned.GetTokenizer().GetNTokens(), 3U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test Contains and Prepend
bool UTDDebugInfo::TestContainsAndPrepend()
{
  bool Passed = true;

  MDDebugInfo Info("Volume Box", "File", 1);
  Passed = EvaluateTrue("Contains()", "Box", "The text contains Box", Info.Contains("Box")) && Passed;
  Passed = EvaluateTrue("Contains()", "ume B", "The text contains a part over a blank", Info.Contains("ume B")) && Passed;
  Passed = EvaluateFalse("Contains()", "box", "The search is case sensitive", Info.Contains("box")) && Passed;
  Passed = EvaluateFalse("Contains()", "longer than the text", "A longer text is not contained", Info.Contains("Volume Box and more")) && Passed;

  // Prepend changes the tokens too:
  Passed = Evaluate("GetTokenizer()", "before Prepend", "There are two tokens", Info.GetTokenizer().GetNTokens(), 2U) && Passed;
  Info.Prepend("Extra ");
  Passed = EvaluateTrue("Prepend()", "Extra", "The text is in front", Info.GetText() == "Extra Volume Box") && Passed;
  Passed = Evaluate("Prepend()", "Extra", "The tokens are made again", Info.GetTokenizer().GetNTokens(), 3U) && Passed;
  Passed = Evaluate("Prepend()", "Extra", "The tokens without maths are made again", Info.GetTokenizer(false).GetNTokens(), 3U) && Passed;
  Info.Prepend("");
  Passed = EvaluateTrue("Prepend()", "empty", "An empty text changes nothing", Info.GetText() == "Extra Volume Box") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the replacement of all occurrences
bool UTDDebugInfo::TestReplaceAll()
{
  bool Passed = true;

  MDDebugInfo Info("L_%I.Position $I $I2 %I", "File", 1);
  Info.Replace("%I", "3");
  Passed = EvaluateTrue("Replace()", "all occurrences", "All occurrences are replaced", Info.GetText() == "L_3.Position $I $I2 3") && Passed;
  // Without the flag parts of words are replaced, this is why $I2 changes:
  Info.Replace("$I", "7");
  Passed = EvaluateTrue("Replace()", "parts of words", "All occurrences are replaced, also in longer words", Info.GetText() == "L_3.Position 7 72 3") && Passed;

  MDDebugInfo Same("A A", "File", 1);
  Same.Replace("A", "AA");
  Passed = EvaluateTrue("Replace()", "the new text contains the old one", "The new text is not replaced again", Same.GetText() == "AA AA") && Passed;

  MDDebugInfo NotThere("B C", "File", 1);
  Passed = Evaluate("GetTokenizer()", "before Replace", "The tokens are made", NotThere.GetTokenizer().GetNTokens(), 2U) && Passed;
  NotThere.Replace("A", "X");
  Passed = EvaluateTrue("Replace()", "no occurrence", "The text stays", NotThere.GetText() == "B C") && Passed;
  NotThere.Replace("B", "X Y");
  Passed = Evaluate("Replace()", "tokens", "The tokens are made again", NotThere.GetTokenizer().GetNTokens(), 3U) && Passed;

  MDDebugInfo Empty("", "File", 1);
  Empty.Replace("A", "X");
  Passed = EvaluateTrue("Replace()", "empty text", "The empty text stays empty", Empty.GetText() == "") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the replacement of whole words
bool UTDDebugInfo::TestReplaceWholeWords()
{
  bool Passed = true;

  // Letters, digits, and underscores are parts of words, everything else is a border:
  MDDebugInfo Info("A AB BA A_ _A 1A A1 A.B B.A {A+1} (A) A", "File", 1);
  Info.Replace("A", "XY", true);
  Passed = EvaluateTrue("Replace()", "whole words", "Only the whole words are replaced", Info.GetText() == "XY AB BA A_ _A 1A A1 XY.B B.XY {XY+1} (XY) XY") && Passed;

  // Begin and end of the text, single word:
  MDDebugInfo Single("A", "File", 1);
  Single.Replace("A", "B", true);
  Passed = EvaluateTrue("Replace()", "the word is the text", "The word is replaced", Single.GetText() == "B") && Passed;

  // The new word contains the old word:
  MDDebugInfo Grow("A A", "File", 1);
  Grow.Replace("A", "A A", true);
  Passed = EvaluateTrue("Replace()", "the new word contains the old one", "The new word is not replaced again", Grow.GetText() == "A A A A") && Passed;

  // The new word is shorter or empty:
  MDDebugInfo Shrink("LongWord LongWord", "File", 1);
  Shrink.Replace("LongWord", "", true);
  Passed = EvaluateTrue("Replace()", "empty new word", "The word is removed", Shrink.GetText() == " ") && Passed;

  // A repeated pattern in one word:
  MDDebugInfo Repeated("AAA AA", "File", 1);
  Repeated.Replace("AA", "B", true);
  Passed = EvaluateTrue("Replace()", "repeated pattern", "Only the whole word is replaced", Repeated.GetText() == "AAA B") && Passed;

  // No word at all, and an empty text:
  MDDebugInfo Empty("", "File", 1);
  Empty.Replace("A", "B", true);
  Passed = EvaluateTrue("Replace()", "empty text", "The empty text stays empty", Empty.GetText() == "") && Passed;

  // The tokens are made again:
  MDDebugInfo Tokens("A B", "File", 1);
  Passed = Evaluate("GetTokenizer()", "before Replace", "The tokens are made", Tokens.GetTokenizer().GetNTokens(), 2U) && Passed;
  Tokens.Replace("A", "X Y", true);
  Passed = Evaluate("Replace()", "tokens", "The tokens are made again", Tokens.GetTokenizer().GetNTokens(), 3U) && Passed;

  // Defect GEO-A-59: An empty search word never ends in the replacement of whole words
  MDDebugInfo EmptyWholeWord("A B", "File", 1);
  Passed = ExpectedToCrash(EvaluateTrue("Replace()", "empty search word, whole words", "The text stays and the call ends", (EmptyWholeWord.Replace("", "X", true), EmptyWholeWord.GetText() == "A B"))) && Passed;

  // Replacing all occurrences of an empty search word ends and keeps the text:
  MDDebugInfo EmptyAll("A B", "File", 1);
  EmptyAll.Replace("", "X");
  Passed = Evaluate("Replace()", "empty search word, all occurrences", "The text stays and the call ends", EmptyAll.GetText(), MString("A B")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the replacement of the first occurrence with a number
bool UTDDebugInfo::TestReplaceFirst()
{
  bool Passed = true;

  // The first token is split at a dot, thus the number is not the first token:
  MDDebugInfo Info("K R R R", "File", 1);
  Passed = Evaluate("GetTokenizer()", "before ReplaceFirst", "The tokens are made", Info.GetTokenizer().GetNTokens(), 4U) && Passed;
  Info.ReplaceFirst("R", 1.5);
  Passed = EvaluateTrue("ReplaceFirst()", "first", "Only the first occurrence is replaced", Info.GetText() == "K 1.5 R R") && Passed;
  Passed = EvaluateNear("ReplaceFirst()", "first", "The tokens are made again", Info.GetTokenizer().GetTokenAtAsDouble(1), 1.5, 1e-12) && Passed;
  Info.ReplaceFirst("R", 2.5);
  Info.ReplaceFirst("R", 3.5);
  Passed = EvaluateTrue("ReplaceFirst()", "three times", "The occurrences are replaced one after the other", Info.GetText() == "K 1.5 2.5 3.5") && Passed;
  Info.ReplaceFirst("R", 4.5);
  Passed = EvaluateTrue("ReplaceFirst()", "no occurrence", "The text stays", Info.GetText() == "K 1.5 2.5 3.5") && Passed;

  MDDebugInfo Empty("", "File", 1);
  Empty.ReplaceFirst("R", 1.0);
  Passed = EvaluateTrue("ReplaceFirst()", "empty text", "The empty text stays empty", Empty.GetText() == "") && Passed;

  // The number has 15 significant digits: 1/3 = 0.333333333333333, by hand
  MDDebugInfo Precision("R", "File", 1);
  Precision.ReplaceFirst("R", 1.0/3.0);
  Passed = EvaluateTrue("ReplaceFirst()", "1/3", "The number has 15 significant digits", Precision.GetText() == "0.333333333333333") && Passed;

  // Negative and small numbers keep sign and exponent:
  MDDebugInfo Small("K R", "File", 1);
  Small.ReplaceFirst("R", -2.5E-20);
  Passed = EvaluateNear("ReplaceFirst()", "-2.5E-20", "The number is the same", Small.GetTokenizer().GetTokenAtAsDouble(1), -2.5E-20, 1e-30) && Passed;

  // A part of a word is also replaced - RandomDouble works this way:
  MDDebugInfo Part("ABRC", "File", 1);
  Part.ReplaceFirst("R", 1.0);
  Passed = EvaluateTrue("ReplaceFirst()", "part of a word", "A part of a word is replaced", Part.GetText() == "AB1C") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the two tokenizers
bool UTDDebugInfo::TestTokenizers()
{
  bool Passed = true;

  // With maths: 1 + 2 = 3, without maths the token stays a text
  MDDebugInfo Info("A.Position { 1 + 2 } 4", "File", 1);
  MTokenizer& WithMaths = Info.GetTokenizer(true);
  Passed = Evaluate("GetTokenizer(true)", "maths", "The first token is split at the dot", WithMaths.GetNTokens(), 4U) && Passed;
  Passed = EvaluateTrue("GetTokenizer(true)", "maths", "The name is the first token", WithMaths.GetTokenAt(0) == "A") && Passed;
  Passed = EvaluateTrue("GetTokenizer(true)", "maths", "The keyword is the second token", WithMaths.GetTokenAt(1) == "Position") && Passed;
  Passed = EvaluateNear("GetTokenizer(true)", "maths", "The maths is evaluated", WithMaths.GetTokenAtAsDouble(2), 3.0, 1e-12) && Passed;

  MTokenizer& WithoutMaths = Info.GetTokenizer(false);
  Passed = Evaluate("GetTokenizer(false)", "maths", "The number of tokens is the same", WithoutMaths.GetNTokens(), 4U) && Passed;
  Passed = EvaluateTrue("GetTokenizer(false)", "maths", "The maths is not evaluated", WithoutMaths.GetTokenAt(2).Contains("{") ) && Passed;

  // The default is with maths:
  Passed = EvaluateNear("GetTokenizer()", "default", "The default evaluates the maths", Info.GetTokenizer().GetTokenAtAsDouble(2), 3.0, 1e-12) && Passed;

  // The result is stored until the text changes:
  Passed = EvaluateTrue("GetTokenizer()", "second call", "The same tokenizer is returned", &Info.GetTokenizer(true) == &WithMaths) && Passed;
  Info.SetText("B 1");
  Passed = Evaluate("GetTokenizer(true)", "after SetText", "The tokens with maths are made again", Info.GetTokenizer(true).GetNTokens(), 2U) && Passed;
  Passed = Evaluate("GetTokenizer(false)", "after SetText", "The tokens without maths are made again", Info.GetTokenizer(false).GetNTokens(), 2U) && Passed;

  // Comments and empty text:
  MDDebugInfo Comment("A 1 // Comment", "File", 1);
  Passed = Evaluate("GetTokenizer()", "comment", "The comment gives no tokens", Comment.GetTokenizer().GetNTokens(), 2U) && Passed;
  Comment.SetText("# Comment");
  Passed = Evaluate("GetTokenizer()", "comment line", "A comment line has no tokens", Comment.GetTokenizer().GetNTokens(), 0U) && Passed;
  Comment.SetText("");
  Passed = Evaluate("GetTokenizer()", "empty", "An empty text has no tokens", Comment.GetTokenizer().GetNTokens(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the validity flag of the tokenizers
bool UTDDebugInfo::TestTokenizerValidity()
{
  bool Passed = true;

  MDDebugInfo Valid("A { 1 + 2 }", "File", 1);
  Valid.GetTokenizer(true);
  Passed = EvaluateTrue("IsTokenizerValid()", "good maths", "Good maths is valid", Valid.IsTokenizerValid()) && Passed;

  // Bad maths does not count before the maths is evaluated:
  MDDebugInfo Invalid("A { sqrt(-1) }", "File", 1);
  Invalid.GetTokenizer(false);
  Passed = EvaluateTrue("IsTokenizerValid()", "bad maths, not evaluated", "Without maths the line is valid", Invalid.IsTokenizerValid()) && Passed;
  DisableDefaultStreams();
  Invalid.GetTokenizer(true);
  EnableDefaultStreams();
  Passed = EvaluateFalse("IsTokenizerValid()", "bad maths, evaluated", "Maths which is not a number is invalid", Invalid.IsTokenizerValid()) && Passed;

  // The copy keeps the state:
  MDDebugInfo Copy(Invalid);
  Passed = EvaluateFalse("IsTokenizerValid()", "copy of a bad line", "The copy is also invalid", Copy.IsTokenizerValid()) && Passed;

  // Defect GEO-A-58: The validity flag is never set back, also not when the line is replaced by a good one
  Invalid.SetText("A { 1 + 2 }");
  Passed = EvaluateNear("GetTokenizer(true)", "good text after bad text", "The new maths is evaluated", Invalid.GetTokenizer(true).GetTokenAtAsDouble(1), 3.0, 1e-12) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("IsTokenizerValid()", "good text after bad text", "The good line is valid", Invalid.IsTokenizerValid())) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the error message
bool UTDDebugInfo::TestError()
{
  bool Passed = true;

  MDDebugInfo Info("The line", "The file", 23);
  BeginCaptureOutput();
  Info.Error("The message");
  MString Text = EndCaptureOutput();

  Passed = EvaluateTrue("Error()", "file", "The file name is shown", Text.Contains("The file")) && Passed;
  Passed = EvaluateTrue("Error()", "line number", "The line number is shown", Text.Contains("at line 23")) && Passed;
  Passed = EvaluateTrue("Error()", "text", "The text of the line is shown", Text.Contains("\"The line\"")) && Passed;
  Passed = EvaluateTrue("Error()", "message", "The message is shown", Text.Contains("The message")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTDDebugInfo Test;
  return Test.Run() == true ? 0 : 1;
}


// UTDDebugInfo.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
