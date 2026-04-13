/*
 * UTString.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MString.h"
#include "MUnitTest.h"

// Standard lib:
#include <limits>
#include <sstream>
using namespace std;


//! Unit test class for the MString helper
class UTString : public MUnitTest
{
public:
  //! Default constructor
  UTString() : MUnitTest("UTString") {}
  //! Default destructor
  virtual ~UTString() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test constructors and assignment operators
  bool TestConstructionAndAssignment();
  //! Test search and access helpers
  bool TestAccessAndSearch();
  //! Test string modification helpers
  bool TestModification();
  //! Test tokenization and extraction helpers
  bool TestTokenizeAndExtract();
  //! Test streaming and type checks
  bool TestReadingAndTypeChecks();
  //! Test numeric conversion and formatting helpers
  bool TestNumericConversions();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTString::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndAssignment() && AllPassed;
  AllPassed = TestAccessAndSearch() && AllPassed;
  AllPassed = TestModification() && AllPassed;
  AllPassed = TestTokenizeAndExtract() && AllPassed;
  AllPassed = TestReadingAndTypeChecks() && AllPassed;
  AllPassed = TestNumericConversions() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test constructors and assignment operators
bool UTString::TestConstructionAndAssignment()
{
  bool Passed = true;

  MString Empty;
  Passed = EvaluateTrue("MString()", "default", "Default constructor creates an empty string", Empty.IsEmpty()) && Passed;

  MString FromCString("alpha");
  Passed = Evaluate("MString(const char*)", "alpha", "Construct from a C string", FromCString, MString("alpha")) && Passed;

  MString NullString(static_cast<const char*>(nullptr));
  Passed = Evaluate("MString(const char*)", "nullptr", "Null C-string constructor keeps the string empty", NullString, MString("")) && Passed;

  MString Partial("alphabet", 5);
  Passed = Evaluate("MString(const char*, unsigned int)", "alphabet, 5", "Copy the requested prefix", Partial, MString("alpha")) && Passed;

  MString Copy(FromCString);
  Passed = Evaluate("MString(const MString&)", "alpha", "Copy constructor duplicates content", Copy, MString("alpha")) && Passed;

  MString Assigned;
  Assigned = "beta";
  Passed = Evaluate("operator=(const char*)", "beta", "Assign from C string", Assigned, MString("beta")) && Passed;

  Assigned = static_cast<const char*>(nullptr);
  Passed = Evaluate("operator=(const char*)", "nullptr", "Assigning nullptr leaves the existing content unchanged", Assigned, MString("beta")) && Passed;

  MString Numeric(42);
  Passed = Evaluate("MString(int)", "42", "Numeric constructor stringifies integers", Numeric, MString("42")) && Passed;

  ostringstream Stream;
  Stream << "gamma";
  MString FromStream(Stream);
  Passed = Evaluate("MString(const ostringstream&)", "gamma", "Construct from ostringstream", FromStream, MString("gamma")) && Passed;

  MString MoveSource("moved");
  MString MoveConstructed(std::move(MoveSource));
  Passed = Evaluate("MString(MString&&)", "moved", "Move constructor transfers the string content", MoveConstructed, MString("moved")) && Passed;

  MString MoveAssignedSource("assigned");
  MString MoveAssigned;
  MoveAssigned = std::move(MoveAssignedSource);
  Passed = Evaluate("operator=(MString&&)", "assigned", "Move assignment transfers the string content", MoveAssigned, MString("assigned")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test search and access helpers
bool UTString::TestAccessAndSearch()
{
  bool Passed = true;

  MString String("alphabet");
  Passed = Evaluate("Length()", "alphabet", "Length returns the number of characters", static_cast<int>(String.Length()), 8) && Passed;
  Passed = Evaluate("GetSubString(size_t, size_t)", "alphabet", "Substring with size extracts the requested range", String.GetSubString(2, 3), MString("pha")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "alphabet", "Substring to end extracts the suffix", String.GetSubString(5), MString("bet")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "alphabet", "Out-of-range substring is empty", String.GetSubString(99), MString("")) && Passed;

  Passed = EvaluateTrue("Contains()", "alphabet", "Contains finds an embedded substring", String.Contains("pha")) && Passed;
  Passed = EvaluateTrue("BeginsWith()", "alphabet", "BeginsWith matches the prefix", String.BeginsWith("alpha")) && Passed;
  Passed = EvaluateTrue("EndsWith()", "alphabet", "EndsWith matches the suffix", String.EndsWith("bet")) && Passed;
  Passed = Evaluate("Index()", "alphabet", "Index finds the first occurrence", static_cast<long>(String.Index("ha")), 3L) && Passed;
  Passed = Evaluate("First()", "alphabet", "First finds the first character occurrence", static_cast<long>(String.First('a')), 0L) && Passed;
  Passed = Evaluate("Last()", "alphabet", "Last finds the last character occurrence", static_cast<long>(String.Last('a')), 4L) && Passed;
  Passed = Evaluate("First()", "alphabet", "First returns npos if the character is absent", static_cast<size_t>(String.First('z')), MString::npos) && Passed;
  Passed = Evaluate("Last()", "alphabet", "Last returns npos if the character is absent", static_cast<size_t>(String.Last('z')), MString::npos) && Passed;
  Passed = Evaluate("Index()", "alphabet", "Index returns npos if the substring is absent", static_cast<size_t>(String.Index("zz")), MString::npos) && Passed;
  Passed = Evaluate("FindFirst()", "alphabet", "FindFirst returns npos if the substring is absent", static_cast<size_t>(String.FindFirst("zz")), MString::npos) && Passed;
  Passed = Evaluate("FindLast()", "abracadabra", "FindLast finds the last substring occurrence", static_cast<long>(MString("abracadabra").FindLast("abra")), 7L) && Passed;
  Passed = Evaluate("FindLast()", "alphabet", "FindLast returns npos if the substring is absent", static_cast<size_t>(String.FindLast("zz")), MString::npos) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "CaseSensitive", "AreIdentical succeeds for identical strings", MString("Case").AreIdentical("Case")) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "IgnoreCase", "AreIdentical(ignore case) succeeds for case-only differences", MString("Case").AreIdentical("cASE", true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "IgnoreCase mismatch", "AreIdentical(ignore case) rejects different text", MString("Case").AreIdentical("cash", true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test string modification helpers
bool UTString::TestModification()
{
  bool Passed = true;

  MString String("core");
  String.Append("_tail");
  Passed = Evaluate("Append()", "core + _tail", "Append extends the string at the end", String, MString("core_tail")) && Passed;

  String.Prepend("pre_");
  Passed = Evaluate("Prepend()", "pre_ + core_tail", "Prepend inserts content at the beginning", String, MString("pre_core_tail")) && Passed;

  String.Remove(4, 5);
  Passed = Evaluate("Remove(size_t, size_t)", "pre_core_tail", "Remove erases a middle segment", String, MString("pre_tail")) && Passed;

  String.Replace(4, 4, "fix");
  Passed = Evaluate("Replace(size_t, size_t, const MString&)", "pre_tail", "Replace swaps a segment in place", String, MString("pre_fix")) && Passed;

  MString AllStrings("one two two three");
  AllStrings.ReplaceAll("two", "2");
  Passed = Evaluate("ReplaceAll()", "one two two three", "ReplaceAll updates all matching substrings", AllStrings, MString("one 2 2 three")) && Passed;

  AllStrings.RemoveAll(" ");
  Passed = Evaluate("RemoveAll()", "one 2 2 three", "RemoveAll deletes every occurrence", AllStrings, MString("one22three")) && Passed;

  MString AtEnd("file.txt");
  AtEnd.ReplaceAtEnd(".txt", ".dat");
  Passed = Evaluate("ReplaceAtEnd()", "file.txt", "ReplaceAtEnd only rewrites a matching suffix", AtEnd, MString("file.dat")) && Passed;

  MString NoEndMatch("file.txt");
  NoEndMatch.ReplaceAtEnd(".csv", ".dat");
  Passed = Evaluate("ReplaceAtEnd()", "file.txt", "ReplaceAtEnd leaves the string unchanged if the suffix is absent", NoEndMatch, MString("file.txt")) && Passed;

  MString Strip("  padded  ");
  Strip.Strip();
  Passed = Evaluate("Strip()", "  padded  ", "Strip removes leading and trailing spaces", Strip, MString("padded")) && Passed;

  MString StripFront("__abc__");
  StripFront.StripFront('_');
  Passed = Evaluate("StripFront()", "__abc__", "StripFront removes the chosen leading character", StripFront, MString("abc__")) && Passed;

  MString StripBack("__abc__");
  StripBack.StripBack('_');
  Passed = Evaluate("StripBack()", "__abc__", "StripBack removes the chosen trailing character", StripBack, MString("__abc")) && Passed;

  MString Lower("AbC123");
  Lower.ToLower();
  Passed = Evaluate("ToLower()", "AbC123", "ToLower converts ASCII letters to lower case", Lower, MString("abc123")) && Passed;

  MString Upper("AbC123");
  Upper.ToUpper();
  Passed = Evaluate("ToUpper()", "AbC123", "ToUpper converts ASCII letters to upper case", Upper, MString("ABC123")) && Passed;

  MString RemoveLast("abcdef");
  RemoveLast.RemoveLast(2);
  Passed = Evaluate("RemoveLast()", "abcdef", "RemoveLast truncates from the end", RemoveLast, MString("abcd")) && Passed;

  MString OverlapReplace("aaaa");
  OverlapReplace.ReplaceAll("aa", "b");
  Passed = Evaluate("ReplaceAll()", "aaaa", "ReplaceAll handles repeated overlapping source patterns deterministically", OverlapReplace, MString("bb")) && Passed;

  MString OverlapRemove("aaaa");
  OverlapRemove.RemoveAll("aa");
  Passed = Evaluate("RemoveAll()", "aaaa", "RemoveAll handles repeated overlapping source patterns deterministically", OverlapRemove, MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test tokenization and extraction helpers
bool UTString::TestTokenizeAndExtract()
{
  bool Passed = true;

  vector<MString> Tokens = MString("a,,b,c,").Tokenize(",", false);
  Passed = EvaluateSize("Tokenize()", "a,,b,c,", "Tokenize keeps empty fields when requested", Tokens.size(), 5) && Passed;
  if (Tokens.size() > 1) {
    Passed = Evaluate("Tokenize()", "a,,b,c,", "Tokenize preserves the second empty token", Tokens[1], MString("")) && Passed;
  } else {
    Passed = EvaluateSize("Tokenize()", "a,,b,c,", "Tokenize should return enough elements to inspect the second token", Tokens.size(), 2) && Passed;
  }
  if (Tokens.size() > 4) {
    Passed = Evaluate("Tokenize()", "a,,b,c,", "Tokenize preserves the trailing empty token", Tokens[4], MString("")) && Passed;
  } else {
    Passed = EvaluateSize("Tokenize()", "a,,b,c,", "Tokenize should return enough elements to inspect the trailing token", Tokens.size(), 5) && Passed;
  }

  vector<MString> Compact = MString("a,,b,c,").Tokenize(",", true);
  Passed = EvaluateSize("Tokenize()", "a,,b,c,", "Tokenize drops empty fields by default", Compact.size(), 3) && Passed;
  Passed = Evaluate("Tokenize()", "a,,b,c,", "Tokenize returns the expected compact tokens", Compact[2], MString("c")) && Passed;

  Passed = Evaluate("Extract()", "prefix[start]suffix", "Extract returns the text between delimiters", MString("prefix[start]suffix").Extract("[", "]"), MString("start")) && Passed;
  Passed = Evaluate("Extract()", "missing before delimiter", "Extract returns an empty string if the opening delimiter is absent", MString("prefixstart]suffix").Extract("[", "]"), MString("")) && Passed;
  Passed = Evaluate("Extract()", "missing after delimiter", "Extract returns an empty string if the closing delimiter is absent", MString("prefix[startsuffix").Extract("[", "]"), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test streaming and type checks
bool UTString::TestReadingAndTypeChecks()
{
  bool Passed = true;

  istringstream SingleLine("first line\nsecond line");
  MString Line;
  Line.ReadLine(SingleLine);
  Passed = Evaluate("ReadLine()", "first line\\nsecond line", "ReadLine reads exactly one line", Line, MString("first line")) && Passed;

  istringstream AllLines("first\nsecond");
  MString Full;
  Full.Read(AllLines);
  Passed = Evaluate("Read()", "first\\nsecond", "Read concatenates all lines and restores separators", Full, MString("first\nsecond\n")) && Passed;

  istringstream EmptyInput("");
  MString Empty;
  Empty.Read(EmptyInput);
  Passed = Evaluate("Read()", "empty input", "Read keeps the string empty for empty input", Empty, MString("")) && Passed;

  istringstream SingleNoTerminator("single");
  MString Single;
  Single.Read(SingleNoTerminator);
  Passed = Evaluate("Read()", "single", "Read appends a newline after the final line", Single, MString("single\n")) && Passed;

  Passed = EvaluateTrue("IsNumber()", " 12.5 ", "IsNumber accepts leading and trailing spaces", MString(" 12.5 ").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "12\\t", "IsNumber accepts trailing horizontal whitespace", MString("12\t").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "-12.5", "IsNumber accepts negative numbers", MString("-12.5").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "1e3", "IsNumber accepts scientific notation", MString("1e3").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "12x", "IsNumber rejects trailing non-whitespace", MString("12x").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "   ", "IsNumber rejects whitespace-only strings", MString("   ").IsNumber()) && Passed;

  Passed = EvaluateTrue("Is<int>()", "123", "Template type check accepts valid integers", MString("123").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", "123 ", "Template type check ignores trailing whitespace", MString("123 ").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", " 123", "Template type check ignores leading whitespace", MString(" 123").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "123.4", "Template type check rejects mismatched formats", MString("123.4").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<unsigned int>()", "-1", "Unsigned type checks reject negative values", MString("-1").Is<unsigned int>()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test numeric conversion and formatting helpers
bool UTString::TestNumericConversions()
{
  bool Passed = true;

  Passed = Evaluate("ToInt()", "-17", "ToInt converts signed integers", MString("-17").ToInt(), -17) && Passed;
  Passed = Evaluate("ToLong()", "-123456", "ToLong converts signed long values", MString("-123456").ToLong(), -123456L) && Passed;
  Passed = Evaluate("ToUnsignedInt()", "42", "ToUnsignedInt converts unsigned integers", static_cast<unsigned long>(MString("42").ToUnsignedInt()), 42UL) && Passed;
  Passed = EvaluateNear("ToDouble()", "3.25", "ToDouble converts floating-point values", MString("3.25").ToDouble(), 3.25, 1e-12) && Passed;

  {
    ostringstream Out;
    Out << numeric_limits<unsigned long>::max();
    unsigned long Value = MString(Out.str()).ToUnsignedLong();
    Passed = Evaluate("ToUnsignedLong()", Out.str().c_str(), "ToUnsignedLong preserves the full unsigned long range", Value, numeric_limits<unsigned long>::max()) && Passed;
  }

  {
    ostringstream Out;
    Out << numeric_limits<unsigned int>::max();
    unsigned int Value = MString(Out.str()).ToUnsignedInt();
    Passed = Evaluate("ToUnsignedInt()", Out.str().c_str(), "ToUnsignedInt preserves the full unsigned int range", Value, numeric_limits<unsigned int>::max()) && Passed;
  }

  MString Rounded(12.345, static_cast<unsigned int>(3));
  Passed = Evaluate("MString(double, unsigned int)", "12.345, precision 3", "Precision constructor uses stream precision", Rounded, MString("12.3")) && Passed;

  MString WithUncertainty(12.345, 1.872, "mm");
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345 +/- 1.872 mm", "Value/uncertainty formatting uses scientific rounding", WithUncertainty, MString("(12.3 ± 1.9) mm")) && Passed;

  MString ZeroUncertainty(12.0, 0.0, "mm");
  Passed = EvaluateFalse("MString(double, double, MString, bool)", "12.0 +/- 0.0 mm", "Zero uncertainty should produce a finite formatted string", ZeroUncertainty.Contains("nan") || ZeroUncertainty.Contains("inf")) && Passed;

  MString NegativeUncertainty(12.0, -1.0, "mm");
  Passed = EvaluateFalse("MString(double, double, MString, bool)", "12.0 +/- -1.0 mm", "Negative uncertainty should produce a finite formatted string", NegativeUncertainty.Contains("nan") || NegativeUncertainty.Contains("inf")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Main function
int main()
{
  UTString Test;
  bool Passed = Test.Run();

  return Passed ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
