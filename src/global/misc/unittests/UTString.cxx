/*
 * UTString.cxx
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
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib libs:
#include "MExceptions.h"
#include "MString.h"
#include "MUnitTest.h"


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
  //! Test trimming of leading and trailing characters
  bool TestTrim();
  //! Test the constructors from numbers and characters
  bool TestNumericConstruction();
  //! Test the operator+= overloads
  bool TestAppendOperators();
  //! Test the free operators +, ==, !=, and <
  bool TestFreeOperators();
  //! Test boundary positions (n-1, n, n+1) of the position-based helpers
  bool TestBoundaries();
  //! Test the element and string access functions
  bool TestElementAccess();
  //! Test copy, move, assignment, and the references returned by the modifying functions
  bool TestLifecycleAndReferences();
  //! Test the modifying functions which return the string at their boundaries
  bool TestReturningModifiers();
  //! Test the search functions and predicates
  bool TestSearchAndPredicates();
  //! Test calls in which an argument is the string itself
  bool TestAliasing();
  //! Test embedded zeros and bytes above 127
  bool TestBinaryContent();
  //! Test the integer conversions and the type test
  bool TestIntegerConversions();
  //! Test stream functions on reused objects
  bool TestStreamReuse();
  //! Test the number formatting with precision and uncertainty and the matching of printed numbers
  bool TestPrecisionAndUncertainty();

  //! Tokenize and compare the complete ordered result
  bool EvaluateTokens(const MString& Input, const MString& Delimiter, bool IgnoreEmpty, const vector<MString>& Expected);
  //! Test tokenization and extraction helpers
  bool TestTokenizeAndExtract();
  //! Test streaming and type checks
  bool TestReadingAndTypeChecks();
  //! Test floating-point recognition and printed-precision matching
  bool TestFloatingPointMatching();
  //! Test numeric conversion and formatting helpers
  bool TestNumericConversions();
  //! Test edge cases and defensive behavior
  bool TestEdgeCases();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTString::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndAssignment() && AllPassed;
  AllPassed = TestAccessAndSearch() && AllPassed;
  AllPassed = TestModification() && AllPassed;
  AllPassed = TestTrim() && AllPassed;
  AllPassed = TestNumericConstruction() && AllPassed;
  AllPassed = TestAppendOperators() && AllPassed;
  AllPassed = TestFreeOperators() && AllPassed;
  AllPassed = TestBoundaries() && AllPassed;
  AllPassed = TestElementAccess() && AllPassed;
  AllPassed = TestLifecycleAndReferences() && AllPassed;
  AllPassed = TestReturningModifiers() && AllPassed;
  AllPassed = TestSearchAndPredicates() && AllPassed;
  AllPassed = TestAliasing() && AllPassed;
  AllPassed = TestBinaryContent() && AllPassed;
  AllPassed = TestIntegerConversions() && AllPassed;
  AllPassed = TestStreamReuse() && AllPassed;
  AllPassed = TestPrecisionAndUncertainty() && AllPassed;
  AllPassed = TestTokenizeAndExtract() && AllPassed;
  AllPassed = TestReadingAndTypeChecks() && AllPassed;
  AllPassed = TestFloatingPointMatching() && AllPassed;
  AllPassed = TestNumericConversions() && AllPassed;
  AllPassed = TestEdgeCases() && AllPassed;

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

  Passed = Evaluate("Data()", "alpha", "Data returns a C string view of the content", MString(FromCString.Data()), MString("alpha")) && Passed;
  Passed = Evaluate("ToString()", "alpha", "ToString returns a std::string copy", MString(FromCString.ToString()), MString("alpha")) && Passed;

  MString StringRef("delta");
  StringRef.GetStringRef() = "epsilon";
  Passed = Evaluate("GetStringRef()", "delta -> epsilon", "GetStringRef exposes a mutable std::string reference", StringRef, MString("epsilon")) && Passed;

  MString ClearString("to be cleared");
  ClearString.Clear();
  Passed = EvaluateTrue("Clear()", "to be cleared", "Clear removes all content", ClearString.IsEmpty()) && Passed;

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
  Passed = Evaluate("Index(..., Start)", "abracadabra from 1", "Index honors the nonzero start position", static_cast<long>(MString("abracadabra").Index("abra", 1)), 7L) && Passed;
  Passed = Evaluate("FindFirst()", "abracadabra", "FindFirst returns the first substring occurrence", static_cast<long>(MString("abracadabra").FindFirst("abra")), 0L) && Passed;
  Passed = Evaluate("FindFirst()", "alphabet", "FindFirst returns npos if the substring is absent", static_cast<size_t>(String.FindFirst("zz")), MString::npos) && Passed;
  Passed = Evaluate("FindLast()", "abracadabra", "FindLast finds the last substring occurrence", static_cast<long>(MString("abracadabra").FindLast("abra")), 7L) && Passed;
  Passed = Evaluate("FindLast()", "alphabet", "FindLast returns npos if the substring is absent", static_cast<size_t>(String.FindLast("zz")), MString::npos) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "CaseSensitive", "AreIdentical succeeds for identical strings", MString("Case").AreIdentical("Case")) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "IgnoreCase", "AreIdentical(ignore case) succeeds for case-only differences", MString("Case").AreIdentical("cASE", true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "IgnoreCase mismatch", "AreIdentical(ignore case) rejects different text", MString("Case").AreIdentical("cash", true)) && Passed;

  MString Indexed("abc");
  Indexed[0] = 'x';
  Passed = Evaluate("operator[]", "abc", "Non-const operator[] allows indexed mutation", Indexed, MString("xbc")) && Passed;
  const MString ConstIndexed("xyz");
  Passed = Evaluate("operator[] const", "xyz", "Const operator[] returns the requested character", ConstIndexed[1], 'y') && Passed;

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

  MString AppendInPlace("core");
  AppendInPlace.AppendInPlace("_tail");
  Passed = Evaluate("AppendInPlace(const char*)", "core + _tail", "AppendInPlace extends the string at the end", AppendInPlace, MString("core_tail")) && Passed;

  MString AppendObject("core");
  MString Tail("_tail");
  AppendObject.AppendInPlace(Tail);
  Passed = Evaluate("AppendInPlace(MString&)", "core + _tail", "AppendInPlace with MString appends object content", AppendObject, MString("core_tail")) && Passed;

  MString PrependInPlace("tail");
  PrependInPlace.PrependInPlace("pre_");
  Passed = Evaluate("PrependInPlace(const char*)", "pre_ + tail", "PrependInPlace inserts content at the beginning", PrependInPlace, MString("pre_tail")) && Passed;

  MString PrependObject("tail");
  MString Prefix("pre_");
  PrependObject.PrependInPlace(Prefix);
  Passed = Evaluate("PrependInPlace(MString&)", "pre_ + tail", "PrependInPlace with MString inserts object content at the beginning", PrependObject, MString("pre_tail")) && Passed;

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

  MString StripFrontInPlace("__abc__");
  StripFrontInPlace.StripFrontInPlace('_');
  Passed = Evaluate("StripFrontInPlace()", "__abc__", "StripFrontInPlace removes the chosen leading character", StripFrontInPlace, MString("abc__")) && Passed;

  MString StripBack("__abc__");
  StripBack.StripBack('_');
  Passed = Evaluate("StripBack()", "__abc__", "StripBack removes the chosen trailing character", StripBack, MString("__abc")) && Passed;

  MString StripBackInPlace("__abc__");
  StripBackInPlace.StripBackInPlace('_');
  Passed = Evaluate("StripBackInPlace()", "__abc__", "StripBackInPlace removes the chosen trailing character", StripBackInPlace, MString("__abc")) && Passed;

  MString StripInPlace("__abc__");
  StripInPlace.StripInPlace('_');
  Passed = Evaluate("StripInPlace()", "__abc__", "StripInPlace removes the chosen leading and trailing character", StripInPlace, MString("abc")) && Passed;

  MString Lower("AbC123");
  Lower.ToLower();
  Passed = Evaluate("ToLower()", "AbC123", "ToLower converts ASCII letters to lower case", Lower, MString("abc123")) && Passed;

  MString LowerInPlace("AbC123");
  LowerInPlace.ToLowerInPlace();
  Passed = Evaluate("ToLowerInPlace()", "AbC123", "ToLowerInPlace converts ASCII letters to lower case", LowerInPlace, MString("abc123")) && Passed;

  MString Upper("AbC123");
  Upper.ToUpper();
  Passed = Evaluate("ToUpper()", "AbC123", "ToUpper converts ASCII letters to upper case", Upper, MString("ABC123")) && Passed;

  MString UpperInPlace("AbC123");
  UpperInPlace.ToUpperInPlace();
  Passed = Evaluate("ToUpperInPlace()", "AbC123", "ToUpperInPlace converts ASCII letters to upper case", UpperInPlace, MString("ABC123")) && Passed;

  MString RemoveLast("abcdef");
  RemoveLast.RemoveLast(2);
  Passed = Evaluate("RemoveLast()", "abcdef", "RemoveLast truncates from the end", RemoveLast, MString("abcd")) && Passed;

  MString RemoveInPlaceAll("abcdef");
  RemoveInPlaceAll.RemoveInPlace(3);
  Passed = Evaluate("RemoveInPlace(size_t)", "abcdef", "RemoveInPlace(start) removes all characters from start to the end", RemoveInPlaceAll, MString("abc")) && Passed;

  MString RemoveInPlaceRange("abcdef");
  RemoveInPlaceRange.RemoveInPlace(2, 2);
  Passed = Evaluate("RemoveInPlace(size_t, size_t)", "abcdef", "RemoveInPlace(start, size) removes a range", RemoveInPlaceRange, MString("abef")) && Passed;

  MString RemoveLastInPlace("abcdef");
  RemoveLastInPlace.RemoveLastInPlace(2);
  Passed = Evaluate("RemoveLastInPlace()", "abcdef", "RemoveLastInPlace truncates from the end", RemoveLastInPlace, MString("abcd")) && Passed;

  MString ReplaceInPlaceString("abcdef");
  ReplaceInPlaceString.ReplaceInPlace(2, 2, "ZZ");
  Passed = Evaluate("ReplaceInPlace()", "abcdef", "ReplaceInPlace swaps a segment in place", ReplaceInPlaceString, MString("abZZef")) && Passed;

  MString ReplaceAllInPlaceString("one two two");
  ReplaceAllInPlaceString.ReplaceAllInPlace("two", "2");
  Passed = Evaluate("ReplaceAllInPlace()", "one two two", "ReplaceAllInPlace updates all matching substrings", ReplaceAllInPlaceString, MString("one 2 2")) && Passed;

  MString ReplaceAtEndInPlaceString("file.txt");
  ReplaceAtEndInPlaceString.ReplaceAtEndInPlace(".txt", ".dat");
  Passed = Evaluate("ReplaceAtEndInPlace()", "file.txt", "ReplaceAtEndInPlace rewrites a matching suffix", ReplaceAtEndInPlaceString, MString("file.dat")) && Passed;

  MString RemoveAllInPlaceString("one two two");
  RemoveAllInPlaceString.RemoveAllInPlace("two");
  Passed = Evaluate("RemoveAllInPlace()", "one two two", "RemoveAllInPlace removes all matching substrings", RemoveAllInPlaceString, MString("one  ")) && Passed;

  MString OverlapReplace("aaaa");
  OverlapReplace.ReplaceAll("aa", "b");
  Passed = Evaluate("ReplaceAll()", "aaaa", "ReplaceAll handles repeated overlapping source patterns deterministically", OverlapReplace, MString("bb")) && Passed;

  MString OverlapRemove("aaaa");
  OverlapRemove.RemoveAll("aa");
  Passed = Evaluate("RemoveAll()", "aaaa", "RemoveAll handles repeated overlapping source patterns deterministically", OverlapRemove, MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test trimming of leading and trailing characters
bool UTString::TestTrim()
{
  bool Passed = true;

  // The default characters are space, \t, \n, \r, \f, and \v
  MString Both("  \t core text \n\r ");
  Both.Trim();
  Passed = Evaluate("Trim()", "both sides", "Trim removes leading and trailing white space and keeps the inner white space", Both, MString("core text")) && Passed;

  MString Leading("   core");
  Leading.Trim();
  Passed = Evaluate("Trim()", "leading only", "Trim removes only leading white space if there is no trailing white space", Leading, MString("core")) && Passed;

  MString Trailing("core \t\n");
  Trailing.Trim();
  Passed = Evaluate("Trim()", "trailing only", "Trim removes only trailing white space if there is no leading white space", Trailing, MString("core")) && Passed;

  MString Clean("core");
  Clean.Trim();
  Passed = Evaluate("Trim()", "nothing to trim", "Trim does not change a string without leading or trailing white space", Clean, MString("core")) && Passed;

  MString AllCharacters(" \t\n\r\f\v core\f\v ");
  AllCharacters.Trim();
  Passed = Evaluate("Trim()", "all default characters", "Trim removes all six default white-space characters, including form feed and vertical tab", AllCharacters, MString("core")) && Passed;

  MString OnlyWhite(" \t\n ");
  OnlyWhite.Trim();
  Passed = Evaluate("Trim()", "only white space", "Trim of a string consisting only of white space gives an empty string", OnlyWhite, MString("")) && Passed;

  MString Empty("");
  Empty.Trim();
  Passed = Evaluate("Trim()", "empty", "Trim of an empty string stays empty", Empty, MString("")) && Passed;

  MString Utf8("  \xc3\xa4 ");
  Utf8.Trim();
  Passed = Evaluate("Trim()", "non-ASCII bytes", "Trim keeps bytes above 127, which are not white space (UTF-8 umlaut)", Utf8, MString("\xc3\xa4")) && Passed;

  // Custom characters
  MString Custom("xxyaxbyxx");
  Custom.Trim("xy");
  Passed = Evaluate("Trim(MString)", "custom characters", "Trim with a character set removes any of the characters at both ends and keeps them inside", Custom, MString("axb")) && Passed;

  MString CustomKeepsWhite("  xcorex  ");
  CustomKeepsWhite.Trim("x");
  Passed = Evaluate("Trim(MString)", "custom keeps white space", "Trim with custom characters does not remove white space", CustomKeepsWhite, MString("  xcorex  ")) && Passed;

  MString NoCharacters("  core  ");
  NoCharacters.Trim("");
  Passed = Evaluate("Trim(MString)", "empty character set", "Trim with an empty character set removes nothing", NoCharacters, MString("  core  ")) && Passed;

  MString AllCustom("xyxy");
  AllCustom.Trim("xy");
  Passed = Evaluate("Trim(MString)", "all custom", "Trim of a string consisting only of the given characters gives an empty string", AllCustom, MString("")) && Passed;

  // Return value: Trim modifies the string and returns a reference to it
  MString Chained(" core ");
  MString& Returned = Chained.Trim();
  Passed = EvaluateTrue("Trim()", "reference", "Trim returns a reference to the trimmed string itself", &Returned == &Chained) && Passed;
  Passed = Evaluate("Trim()", "chained", "The string is trimmed after a chained call", Chained, MString("core")) && Passed;

  // TrimInPlace
  MString InPlace("  \t core text \n ");
  InPlace.TrimInPlace();
  Passed = Evaluate("TrimInPlace()", "both sides", "TrimInPlace removes leading and trailing white space and keeps the inner white space", InPlace, MString("core text")) && Passed;

  MString InPlaceCustom("--core--");
  InPlaceCustom.TrimInPlace("-");
  Passed = Evaluate("TrimInPlace(MString)", "custom characters", "TrimInPlace with a character set removes it at both ends", InPlaceCustom, MString("core")) && Passed;

  MString InPlaceWhite("   ");
  InPlaceWhite.TrimInPlace();
  Passed = Evaluate("TrimInPlace()", "only white space", "TrimInPlace of a string consisting only of white space gives an empty string", InPlaceWhite, MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the constructors from numbers and characters
bool UTString::TestNumericConstruction()
{
  bool Passed = true;

  // Integers are printed in decimal, limits from numeric_limits
  Passed = Evaluate("MString(short)", "-32768", "A short is printed in decimal (minimum -2^15)", MString(numeric_limits<short>::min()), MString("-32768")) && Passed;
  Passed = Evaluate("MString(unsigned short)", "65535", "An unsigned short is printed in decimal (maximum 2^16 - 1)", MString(numeric_limits<unsigned short>::max()), MString("65535")) && Passed;
  Passed = Evaluate("MString(int)", "-2147483648", "An int is printed in decimal (minimum -2^31)", MString(numeric_limits<int>::min()), MString("-2147483648")) && Passed;
  Passed = Evaluate("MString(unsigned int)", "4294967295", "An unsigned int is printed in decimal (maximum 2^32 - 1)", MString(numeric_limits<unsigned int>::max()), MString("4294967295")) && Passed;
  Passed = Evaluate("MString(long)", "-9223372036854775808", "A long is printed in decimal (minimum of the platform, text from std::to_string)", MString(numeric_limits<long>::min()), MString(to_string(numeric_limits<long>::min()))) && Passed;
  Passed = Evaluate("MString(unsigned long)", "18446744073709551615", "An unsigned long is printed in decimal (maximum of the platform, text from std::to_string)", MString(numeric_limits<unsigned long>::max()), MString(to_string(numeric_limits<unsigned long>::max()))) && Passed;
  Passed = Evaluate("MString(int)", "0", "Zero is printed as 0", MString(0), MString("0")) && Passed;

  // A float is printed with 8 significant digits: 1/3 in float is 0.3333333432674408 -> 0.33333334
  Passed = Evaluate("MString(float)", "1.5f", "An exactly representable float is printed without extra digits", MString(1.5f), MString("1.5")) && Passed;
  Passed = Evaluate("MString(float)", "1/3", "A float is printed with 8 significant digits (float 1/3 = 0.3333333432...)", MString(1.0f/3.0f), MString("0.33333334")) && Passed;
  // A double is printed with 15 significant digits
  Passed = Evaluate("MString(double)", "1/3", "A double is printed with 15 significant digits", MString(1.0/3.0), MString("0.333333333333333")) && Passed;
  Passed = Evaluate("MString(double)", "-2.5e-7", "A small double is printed in scientific notation", MString(-2.5e-7), MString("-2.5e-07")) && Passed;

  // Characters and C strings
  Passed = Evaluate("MString(char)", "x", "A single character gives a string of length one", MString('x'), MString("x")) && Passed;
  Passed = Evaluate("MString(const char*)", "nullptr", "A null pointer gives an empty string", MString(static_cast<const char*>(nullptr)), MString("")) && Passed;
  Passed = Evaluate("MString(const char*, unsigned int)", "N = 0", "Copying zero characters gives an empty string", MString("abc", 0U), MString("")) && Passed;
  Passed = Evaluate("MString(const char*, unsigned int)", "N = 2", "Copying two characters gives the first two", MString("abc", 2U), MString("ab")) && Passed;
  Passed = Evaluate("MString(const char*, unsigned int)", "N = 3", "Copying all characters gives the whole string", MString("abc", 3U), MString("abc")) && Passed;
  Passed = Evaluate("MString(const char*, unsigned int)", "nullptr", "Copying from a null pointer gives an empty string", MString(static_cast<const char*>(nullptr), 3U), MString("")) && Passed;
  Passed = Evaluate("MString(const string&)", "text", "A std::string is copied", MString(string("text")), MString("text")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the operator+= overloads
bool UTString::TestAppendOperators()
{
  bool Passed = true;

  MString S("v=");
  S += static_cast<short>(-5);
  Passed = Evaluate("operator+=(short)", "-5", "A short is appended in decimal", S, MString("v=-5")) && Passed;
  S += static_cast<unsigned short>(6);
  Passed = Evaluate("operator+=(unsigned short)", "6", "An unsigned short is appended in decimal", S, MString("v=-56")) && Passed;
  S += -7;
  Passed = Evaluate("operator+=(int)", "-7", "An int is appended in decimal", S, MString("v=-56-7")) && Passed;
  S += 8U;
  Passed = Evaluate("operator+=(unsigned int)", "8", "An unsigned int is appended in decimal", S, MString("v=-56-78")) && Passed;
  S += -9L;
  Passed = Evaluate("operator+=(long)", "-9", "A long is appended in decimal", S, MString("v=-56-78-9")) && Passed;
  S += 10UL;
  Passed = Evaluate("operator+=(unsigned long)", "10", "An unsigned long is appended in decimal", S, MString("v=-56-78-910")) && Passed;
  S += 'c';
  Passed = Evaluate("operator+=(char)", "c", "A character is appended", S, MString("v=-56-78-910c")) && Passed;
  S += "str";
  Passed = Evaluate("operator+=(const char*)", "str", "A C string is appended", S, MString("v=-56-78-910cstr")) && Passed;
  S += string("ing");
  Passed = Evaluate("operator+=(const string&)", "ing", "A std::string is appended", S, MString("v=-56-78-910cstring")) && Passed;
  S += MString("!");
  Passed = Evaluate("operator+=(const MString&)", "!", "An MString is appended", S, MString("v=-56-78-910cstring!")) && Passed;

  // Floating point: float 8 digits, double 15 digits
  MString F;
  F += 1.0f/3.0f;
  Passed = Evaluate("operator+=(float)", "1/3", "A float is appended with 8 significant digits", F, MString("0.33333334")) && Passed;
  MString D;
  D += 1.0/3.0;
  Passed = Evaluate("operator+=(double)", "1/3", "A double is appended with 15 significant digits", D, MString("0.333333333333333")) && Passed;

  // Digits needed to read it back: 17 for 64 bit, 21 for x87, 36 for quad
  const long double Third = 1.0L/3.0L;
  MString L;
  L += Third;
  // The round trip has to be lossless, so the comparison is exact
  Passed = Evaluate("operator+=(long double)", "1/3", "A long double is appended with enough digits to be read back to the same value", stold(L.Data()), Third) && Passed;
  MString LSimple;
  LSimple += 0.5L;
  Passed = Evaluate("operator+=(long double)", "0.5", "An exactly representable long double is appended without extra digits", LSimple, MString("0.5")) && Passed;

  // The return value is the string itself, so calls can be chained
  MString Chain("a");
  MString& Returned = (Chain += 1) += 'b';
  Passed = EvaluateTrue("operator+=()", "chain", "operator+= returns a reference to the string itself", &Returned == &Chain) && Passed;
  Passed = Evaluate("operator+=()", "chain", "Chained appends are applied in order", Chain, MString("a1b")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the free operators +, ==, !=, and <
bool UTString::TestFreeOperators()
{
  bool Passed = true;

  // Concatenation of strings and characters, in both orders
  Passed = Evaluate("operator+(MString, MString)", "ab + cd", "Two strings are concatenated in order", MString("ab") + MString("cd"), MString("abcd")) && Passed;
  Passed = Evaluate("operator+(MString, const char*)", "ab + cd", "A C string is appended", MString("ab") + "cd", MString("abcd")) && Passed;
  Passed = Evaluate("operator+(const char*, MString)", "ab + cd", "A C string is prepended", "ab" + MString("cd"), MString("abcd")) && Passed;
  Passed = Evaluate("operator+(MString, char)", "ab + c", "A character is appended", MString("ab") + 'c', MString("abc")) && Passed;
  Passed = Evaluate("operator+(char, MString)", "a + bc", "A character is prepended", 'a' + MString("bc"), MString("abc")) && Passed;
  Passed = Evaluate("operator+(MString, MString)", "empty", "Concatenating with an empty string gives the other string", MString("") + MString("x") + MString(""), MString("x")) && Passed;

  // Concatenation with numbers, in both orders (the number is printed as in operator+=)
  Passed = Evaluate("operator+(MString, int)", "n=-1", "An int is appended", MString("n=") + -1, MString("n=-1")) && Passed;
  Passed = Evaluate("operator+(int, MString)", "-1 n", "An int is prepended", -1 + MString("n"), MString("-1n")) && Passed;
  Passed = Evaluate("operator+(MString, unsigned int)", "n=4294967295", "An unsigned int is appended", MString("n=") + 4294967295U, MString("n=4294967295")) && Passed;
  Passed = Evaluate("operator+(unsigned int, MString)", "4294967295 n", "An unsigned int is prepended", 4294967295U + MString("n"), MString("4294967295n")) && Passed;
  Passed = Evaluate("operator+(MString, short)", "n=-3", "A short is appended", MString("n=") + static_cast<short>(-3), MString("n=-3")) && Passed;
  Passed = Evaluate("operator+(short, MString)", "-3 n", "A short is prepended", static_cast<short>(-3) + MString("n"), MString("-3n")) && Passed;
  Passed = Evaluate("operator+(MString, unsigned short)", "n=65535", "An unsigned short is appended", MString("n=") + static_cast<unsigned short>(65535), MString("n=65535")) && Passed;
  Passed = Evaluate("operator+(unsigned short, MString)", "65535 n", "An unsigned short is prepended", static_cast<unsigned short>(65535) + MString("n"), MString("65535n")) && Passed;
  Passed = Evaluate("operator+(MString, long)", "n=-9223372036854775808", "A long is appended", MString("n=") + numeric_limits<long>::min(), MString("n=" + to_string(numeric_limits<long>::min()))) && Passed;
  Passed = Evaluate("operator+(long, MString)", "-9223372036854775808 n", "A long is prepended", numeric_limits<long>::min() + MString("n"), MString(to_string(numeric_limits<long>::min()) + "n")) && Passed;
  Passed = Evaluate("operator+(MString, unsigned long)", "n=18446744073709551615", "An unsigned long is appended", MString("n=") + numeric_limits<unsigned long>::max(), MString("n=" + to_string(numeric_limits<unsigned long>::max()))) && Passed;
  Passed = Evaluate("operator+(unsigned long, MString)", "18446744073709551615 n", "An unsigned long is prepended", numeric_limits<unsigned long>::max() + MString("n"), MString(to_string(numeric_limits<unsigned long>::max()) + "n")) && Passed;

  // The same number gives the same text in both orders: float 8 digits (float 1/3 = 0.3333333432...), double 15 digits
  Passed = Evaluate("operator+(MString, float)", "x + 1/3", "A float is appended with 8 significant digits", MString("x") + 1.0f/3.0f, MString("x0.33333334")) && Passed;
  Passed = Evaluate("operator+(float, MString)", "1/3 + x", "A float is prepended with 8 significant digits, as it is appended", 1.0f/3.0f + MString("x"), MString("0.33333334x")) && Passed;
  Passed = Evaluate("operator+(MString, double)", "x + 1/3", "A double is appended with 15 significant digits", MString("x") + 1.0/3.0, MString("x0.333333333333333")) && Passed;
  Passed = Evaluate("operator+(double, MString)", "1/3 + x", "A double is prepended with 15 significant digits, as it is appended", 1.0/3.0 + MString("x"), MString("0.333333333333333x")) && Passed;

  // Comparison: exact, case sensitive, with MString and C strings in both orders
  Passed = EvaluateTrue("operator==(MString, MString)", "equal", "Equal strings are equal", MString("abc") == MString("abc")) && Passed;
  Passed = EvaluateFalse("operator==(MString, MString)", "case", "The comparison is case sensitive", MString("abc") == MString("ABC")) && Passed;
  Passed = EvaluateFalse("operator==(MString, MString)", "prefix", "A prefix is not equal", MString("abc") == MString("ab")) && Passed;
  Passed = EvaluateTrue("operator==(MString, MString)", "empty", "Two empty strings are equal", MString("") == MString()) && Passed;
  Passed = EvaluateTrue("operator==(MString, const char*)", "equal", "An MString equals the same C string", MString("abc") == "abc") && Passed;
  Passed = EvaluateFalse("operator==(MString, const char*)", "different", "An MString does not equal a different C string", MString("abc") == "abd") && Passed;
  Passed = EvaluateTrue("operator==(const char*, MString)", "equal", "A C string equals the same MString", "abc" == MString("abc")) && Passed;
  Passed = EvaluateFalse("operator==(const char*, MString)", "different", "A C string does not equal a different MString", "abc" == MString("abcd")) && Passed;
  Passed = EvaluateFalse("operator!=(MString, MString)", "equal", "Equal strings are not unequal", MString("abc") != MString("abc")) && Passed;
  Passed = EvaluateTrue("operator!=(MString, MString)", "different", "Different strings are unequal", MString("abc") != MString("abd")) && Passed;
  Passed = EvaluateFalse("operator!=(MString, const char*)", "equal", "An MString is not unequal to the same C string", MString("abc") != "abc") && Passed;
  Passed = EvaluateTrue("operator!=(MString, const char*)", "different", "An MString is unequal to a different C string", MString("abc") != "ab") && Passed;
  Passed = EvaluateFalse("operator!=(const char*, MString)", "equal", "A C string is not unequal to the same MString", "abc" != MString("abc")) && Passed;
  Passed = EvaluateTrue("operator!=(const char*, MString)", "different", "A C string is unequal to a different MString", "ABC" != MString("abc")) && Passed;

  // Ordering: lexicographic by byte value (ASCII: upper case before lower case)
  Passed = EvaluateTrue("operator<", "a < b", "a is smaller than b", MString("a") < MString("b")) && Passed;
  Passed = EvaluateFalse("operator<", "b < a", "b is not smaller than a", MString("b") < MString("a")) && Passed;
  Passed = EvaluateFalse("operator<", "a < a", "A string is not smaller than itself", MString("a") < MString("a")) && Passed;
  Passed = EvaluateTrue("operator<", "ab < abc", "A prefix is smaller than the longer string", MString("ab") < MString("abc")) && Passed;
  Passed = EvaluateFalse("operator<", "abc < ab", "The longer string is not smaller than its prefix", MString("abc") < MString("ab")) && Passed;
  Passed = EvaluateTrue("operator<", "empty < a", "The empty string is smaller than any other", MString("") < MString("a")) && Passed;
  Passed = EvaluateTrue("operator<", "B < a", "Upper case letters (ASCII 66) are smaller than lower case letters (ASCII 97)", MString("B") < MString("a")) && Passed;

  vector<MString> Names = { "pear", "Apple", "apple", "fig", "" };
  sort(Names.begin(), Names.end());
  const vector<MString> Sorted = { "", "Apple", "apple", "fig", "pear" };
  Passed = EvaluateTrue("operator<", "sort", "Sorting with operator< gives the byte-wise lexicographic order", Names == Sorted) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test boundary positions (n-1, n, n+1) of the position-based helpers
bool UTString::TestBoundaries()
{
  bool Passed = true;

  // The string "abcd" has the length n = 4
  const MString Base("abcd");

  Passed = Evaluate("GetSubString(size_t, size_t)", "n-1", "Starting at the last character gives that character", Base.GetSubString(3, 5), MString("d")) && Passed;
  Passed = Evaluate("GetSubString(size_t, size_t)", "n", "Starting at the length gives an empty string", Base.GetSubString(4, 1), MString("")) && Passed;
  Passed = Evaluate("GetSubString(size_t, size_t)", "n+1", "Starting beyond the length gives an empty string", Base.GetSubString(5, 1), MString("")) && Passed;
  Passed = Evaluate("GetSubString(size_t, size_t)", "size 0", "A size of zero gives an empty string", Base.GetSubString(1, 0), MString("")) && Passed;
  Passed = Evaluate("GetSubString(size_t, size_t)", "size n", "A size beyond the end is clipped at the end", Base.GetSubString(1, 4), MString("bcd")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "n-1", "Starting at the last character gives that character", Base.GetSubString(3), MString("d")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "n", "Starting at the length gives an empty string", Base.GetSubString(4), MString("")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "n+1", "Starting beyond the length gives an empty string", Base.GetSubString(5), MString("")) && Passed;
  Passed = Evaluate("GetSubString(size_t)", "0", "Starting at zero gives the whole string", Base.GetSubString(0), MString("abcd")) && Passed;

  MString R1(Base);
  R1.RemoveInPlace(3);
  Passed = Evaluate("RemoveInPlace(size_t)", "n-1", "Removing from the last character leaves the first n-1", R1, MString("abc")) && Passed;
  MString R2(Base);
  R2.RemoveInPlace(4);
  Passed = Evaluate("RemoveInPlace(size_t)", "n", "Removing from the length changes nothing", R2, MString("abcd")) && Passed;
  MString R3(Base);
  R3.RemoveInPlace(5);
  Passed = Evaluate("RemoveInPlace(size_t)", "n+1", "Removing beyond the length changes nothing", R3, MString("abcd")) && Passed;
  MString R4(Base);
  R4.RemoveInPlace(0);
  Passed = Evaluate("RemoveInPlace(size_t)", "0", "Removing from zero leaves an empty string", R4, MString("")) && Passed;

  MString S1(Base);
  S1.RemoveInPlace(3, 10);
  Passed = Evaluate("RemoveInPlace(size_t, size_t)", "n-1, large size", "A size beyond the end is clipped", S1, MString("abc")) && Passed;
  MString S2(Base);
  S2.RemoveInPlace(4, 1);
  Passed = Evaluate("RemoveInPlace(size_t, size_t)", "n", "Removing from the length changes nothing", S2, MString("abcd")) && Passed;
  MString S3(Base);
  S3.RemoveInPlace(1, 0);
  Passed = Evaluate("RemoveInPlace(size_t, size_t)", "size 0", "Removing zero characters changes nothing", S3, MString("abcd")) && Passed;
  MString S4(Base);
  S4.RemoveInPlace(5, 1);
  Passed = Evaluate("RemoveInPlace(size_t, size_t)", "n+1", "Removing beyond the length changes nothing", S4, MString("abcd")) && Passed;

  MString L1(Base);
  L1.RemoveLastInPlace(3);
  Passed = Evaluate("RemoveLastInPlace()", "n-1", "Removing n-1 characters leaves the first character", L1, MString("a")) && Passed;
  MString L2(Base);
  L2.RemoveLastInPlace(4);
  Passed = Evaluate("RemoveLastInPlace()", "n", "Removing n characters leaves an empty string", L2, MString("")) && Passed;
  MString L3(Base);
  L3.RemoveLastInPlace(5);
  Passed = Evaluate("RemoveLastInPlace()", "n+1", "Removing more than n characters leaves an empty string", L3, MString("")) && Passed;
  MString L4(Base);
  L4.RemoveLastInPlace(0);
  Passed = Evaluate("RemoveLastInPlace()", "0", "Removing zero characters changes nothing", L4, MString("abcd")) && Passed;
  MString L5;
  L5.RemoveLastInPlace(1);
  Passed = Evaluate("RemoveLastInPlace()", "empty string", "Removing from an empty string stays empty", L5, MString("")) && Passed;

  MString P1(Base);
  P1.ReplaceInPlace(3, 1, "XY");
  Passed = Evaluate("ReplaceInPlace()", "n-1", "Replacing the last character with a longer text extends the string", P1, MString("abcXY")) && Passed;
  MString P2(Base);
  P2.ReplaceInPlace(3, 5, "Z");
  Passed = Evaluate("ReplaceInPlace()", "n-1, large size", "A size beyond the end is clipped", P2, MString("abcZ")) && Passed;
  MString P3(Base);
  P3.ReplaceInPlace(4, 1, "Z");
  Passed = Evaluate("ReplaceInPlace()", "n", "Replacing from the length changes nothing", P3, MString("abcd")) && Passed;
  MString P4(Base);
  P4.ReplaceInPlace(5, 1, "Z");
  Passed = Evaluate("ReplaceInPlace()", "n+1", "Replacing beyond the length changes nothing", P4, MString("abcd")) && Passed;
  MString P5(Base);
  P5.ReplaceInPlace(1, 0, "Z");
  Passed = Evaluate("ReplaceInPlace()", "size 0", "Replacing zero characters inserts the text", P5, MString("aZbcd")) && Passed;

  // Comparison of strings of different length and case
  Passed = EvaluateTrue("AreIdentical()", "empty", "Two empty strings are identical", MString("").AreIdentical(MString(""))) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "empty ignoring case", "Two empty strings are identical when ignoring the case", MString("").AreIdentical(MString(""), true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "n and n+1", "Strings of different length are not identical", MString("abcd").AreIdentical(MString("abcde"))) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "n and n-1", "Strings of different length are not identical when ignoring the case", MString("abcd").AreIdentical(MString("abc"), true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "case", "A different case is not identical", MString("abcd").AreIdentical(MString("abCd"))) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "case ignored", "A different case is identical when ignoring the case", MString("abcd").AreIdentical(MString("aBCD"), true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "first character", "A difference in the first character is found when ignoring the case", MString("abcd").AreIdentical(MString("xbcd"), true)) && Passed;
  Passed = EvaluateFalse("AreIdentical()", "last character", "A difference in the last character is found when ignoring the case", MString("abcd").AreIdentical(MString("abcx"), true)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the element and string access functions
bool UTString::TestElementAccess()
{
  bool Passed = true;

  // operator[] for all index types, reading and writing
  MString S("abcd");
  const int IntIndex = 1;
  const unsigned int UIntIndex = 2;
  const long LongIndex = 3;
  const unsigned long ULongIndex = 0;
  Passed = Evaluate("operator[](int)", "1", "The int index reads the second character", S[IntIndex], 'b') && Passed;
  Passed = Evaluate("operator[](unsigned int)", "2", "The unsigned int index reads the third character", S[UIntIndex], 'c') && Passed;
  Passed = Evaluate("operator[](long)", "3", "The long index reads the last character", S[LongIndex], 'd') && Passed;
  Passed = Evaluate("operator[](unsigned long)", "0", "The unsigned long index reads the first character", S[ULongIndex], 'a') && Passed;

  S[IntIndex] = 'B';
  S[UIntIndex] = 'C';
  S[LongIndex] = 'D';
  S[ULongIndex] = 'A';
  Passed = Evaluate("operator[]", "write", "Writing through all four index types changes the string", S, MString("ABCD")) && Passed;

  const MString Const("xyz");
  Passed = Evaluate("operator[](int) const", "1", "The const int index reads the second character", Const[IntIndex], 'y') && Passed;
  Passed = Evaluate("operator[](unsigned int) const", "2", "The const unsigned int index reads the third character", Const[UIntIndex], 'z') && Passed;
  Passed = Evaluate("operator[](long) const", "1", "The const long index reads the second character", Const[static_cast<long>(1)], 'y') && Passed;
  Passed = Evaluate("operator[](unsigned long) const", "0", "The const unsigned long index reads the first character", Const[ULongIndex], 'x') && Passed;

  // GetString returns a copy, GetStringRef a reference to the content
  MString Original("text");
  string Copy = Original.GetString();
  Copy += "_changed";
  Passed = Evaluate("GetString()", "copy", "GetString returns the content", Original.GetString(), string("text")) && Passed;
  Passed = Evaluate("GetString()", "independent", "Changing the returned string does not change the MString", Original, MString("text")) && Passed;
  Original.GetStringRef() += "_ref";
  Passed = Evaluate("GetStringRef()", "reference", "Changing the returned reference changes the MString", Original, MString("text_ref")) && Passed;
  Passed = EvaluateTrue("GetStringRef()", "same object", "GetStringRef always returns the same object", &Original.GetStringRef() == &Original.GetStringRef()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Tokenize and compare the complete ordered result
bool UTString::EvaluateTokens(const MString& Input, const MString& Delimiter, bool IgnoreEmpty, const vector<MString>& Expected)
{
  const vector<MString> Tokens = Input.Tokenize(Delimiter, IgnoreEmpty);
  MString Label = "\"" + Input + "\" with delimiter \"" + Delimiter + "\", empty tokens kept";
  if (IgnoreEmpty == true) {
    Label = "\"" + Input + "\" with delimiter \"" + Delimiter + "\", empty tokens ignored";
  }

  return EvaluateTrue("Tokenize()", Label, "Tokenize returns exactly the expected tokens in the expected order", Tokens == Expected);
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy, move, assignment, and the references returned by the modifying functions
bool UTString::TestLifecycleAndReferences()
{
  bool Passed = true;

  // Copy construction and assignment give independent strings
  MString Original("abc");
  MString Copy(Original);
  Original.AppendInPlace("x");
  Passed = Evaluate("MString(const MString&)", "mutation", "A copy is not changed when the original is changed", Copy, MString("abc")) && Passed;
  Copy.PrependInPlace("y");
  Passed = Evaluate("MString(const MString&)", "mutation", "The original is not changed when the copy is changed", Original, MString("abcx")) && Passed;

  MString Target("old content");
  Target = Original;
  Original.RemoveLastInPlace(1);
  Passed = Evaluate("operator=(const MString&)", "over content", "Assignment replaces the existing content", Target, MString("abcx")) && Passed;
  Passed = Evaluate("operator=(const MString&)", "independent", "The assigned string is independent of the source", Original, MString("abc")) && Passed;
  Passed = EvaluateTrue("operator=(const MString&)", "reference", "Assignment returns a reference to the target", &(Target = Original) == &Target) && Passed;

  MString& Alias = Target;
  Target = Alias;
  Passed = Evaluate("operator=(const MString&)", "self", "Assigning a string to itself keeps the content", Target, MString("abc")) && Passed;

  MString FromChar("old");
  FromChar = 'z';
  Passed = Evaluate("operator=(char)", "z", "Assignment from a character replaces the content with that character", FromChar, MString("z")) && Passed;
  FromChar = string("std string");
  Passed = Evaluate("operator=(const string&)", "std string", "Assignment from a std::string replaces the content", FromChar, MString("std string")) && Passed;
  FromChar = "c string";
  Passed = Evaluate("operator=(const char*)", "c string", "Assignment from a C string replaces the content", FromChar, MString("c string")) && Passed;

  // Move construction and assignment, and reuse of the moved-from object
  MString Moved("moved");
  MString Taker(std::move(Moved));
  Passed = Evaluate("MString(MString&&)", "content", "The move constructor takes the content", Taker, MString("moved")) && Passed;
  Moved = "again";
  Moved.AppendInPlace("!");
  Passed = Evaluate("MString(MString&&)", "reuse", "A moved-from string can be assigned and modified again", Moved, MString("again!")) && Passed;
  MString MoveTarget("x");
  MoveTarget = std::move(Taker);
  Passed = Evaluate("operator=(MString&&)", "content", "The move assignment takes the content", MoveTarget, MString("moved")) && Passed;
  Taker = "reuse";
  Passed = Evaluate("operator=(MString&&)", "reuse", "A string which was moved from by assignment can be assigned again", Taker, MString("reuse")) && Passed;

  // Append(MString&) and Prepend(MString&)
  MString Base("core");
  MString Tail("_tail");
  MString Head("head_");
  Passed = EvaluateTrue("Append(MString&)", "reference", "Append returns a reference to the string itself", &Base.Append(Tail) == &Base) && Passed;
  Passed = Evaluate("Append(MString&)", "core + _tail", "Append with an MString appends the content", Base, MString("core_tail")) && Passed;
  Passed = EvaluateTrue("Prepend(MString&)", "reference", "Prepend returns a reference to the string itself", &Base.Prepend(Head) == &Base) && Passed;
  Passed = Evaluate("Prepend(MString&)", "head_ + core_tail", "Prepend with an MString inserts the content at the beginning", Base, MString("head_core_tail")) && Passed;
  Passed = Evaluate("Append(MString&)", "argument", "The appended argument is not changed", Tail, MString("_tail")) && Passed;

  // All modifying functions which return the string return a reference to the string itself
  MString R("  Ab-Cd  ");
  Passed = EvaluateTrue("Append(const char*)", "reference", "Append returns a reference to the string itself", &R.Append("") == &R) && Passed;
  Passed = EvaluateTrue("Prepend(const char*)", "reference", "Prepend returns a reference to the string itself", &R.Prepend("") == &R) && Passed;
  Passed = EvaluateTrue("Remove(size_t)", "reference", "Remove returns a reference to the string itself", &R.Remove(100) == &R) && Passed;
  Passed = EvaluateTrue("Remove(size_t, size_t)", "reference", "Remove returns a reference to the string itself", &R.Remove(100, 1) == &R) && Passed;
  Passed = EvaluateTrue("RemoveLast()", "reference", "RemoveLast returns a reference to the string itself", &R.RemoveLast(0) == &R) && Passed;
  Passed = EvaluateTrue("Replace()", "reference", "Replace returns a reference to the string itself", &R.Replace(100, 1, "x") == &R) && Passed;
  Passed = EvaluateTrue("ReplaceAll()", "reference", "ReplaceAll returns a reference to the string itself", &R.ReplaceAll("Q", "x") == &R) && Passed;
  Passed = EvaluateTrue("ReplaceAtEnd()", "reference", "ReplaceAtEnd returns a reference to the string itself", &R.ReplaceAtEnd("Q", "x") == &R) && Passed;
  Passed = EvaluateTrue("RemoveAll()", "reference", "RemoveAll returns a reference to the string itself", &R.RemoveAll("Q") == &R) && Passed;
  Passed = EvaluateTrue("StripFront()", "reference", "StripFront returns a reference to the string itself", &R.StripFront() == &R) && Passed;
  Passed = Evaluate("StripFront()", "two spaces", "StripFront removes all matching characters at the front", R, MString("Ab-Cd  ")) && Passed;
  Passed = EvaluateTrue("StripBack()", "reference", "StripBack returns a reference to the string itself", &R.StripBack() == &R) && Passed;
  Passed = Evaluate("StripBack()", "two spaces", "StripBack removes all matching characters at the back", R, MString("Ab-Cd")) && Passed;
  Passed = EvaluateTrue("Strip()", "reference", "Strip returns a reference to the string itself", &R.Strip('A') == &R) && Passed;
  Passed = Evaluate("Strip()", "A", "Strip removes the given character at the front", R, MString("b-Cd")) && Passed;
  Passed = EvaluateTrue("ToUpper()", "reference", "ToUpper returns a reference to the string itself", &R.ToUpper() == &R) && Passed;
  Passed = Evaluate("ToUpper()", "b-Cd", "ToUpper converts all letters and keeps other characters", R, MString("B-CD")) && Passed;
  Passed = EvaluateTrue("ToLower()", "reference", "ToLower returns a reference to the string itself", &R.ToLower() == &R) && Passed;
  Passed = Evaluate("ToLower()", "B-CD", "ToLower converts all letters and keeps other characters", R, MString("b-cd")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the modifying functions which return the string at their boundaries
bool UTString::TestReturningModifiers()
{
  bool Passed = true;

  // The string "abcd" has the length n = 4
  const MString Base("abcd");

  Passed = Evaluate("Remove(size_t)", "n-1", "Removing from the last character leaves the first n-1", MString(Base).Remove(3), MString("abc")) && Passed;
  Passed = Evaluate("Remove(size_t)", "n", "Removing from the length changes nothing", MString(Base).Remove(4), MString("abcd")) && Passed;
  Passed = Evaluate("Remove(size_t)", "n+1", "Removing beyond the length changes nothing", MString(Base).Remove(5), MString("abcd")) && Passed;
  Passed = Evaluate("Remove(size_t)", "0", "Removing from zero leaves an empty string", MString(Base).Remove(0), MString("")) && Passed;

  Passed = Evaluate("Remove(size_t, size_t)", "n-1, large size", "A size beyond the end is clipped", MString(Base).Remove(3, 10), MString("abc")) && Passed;
  Passed = Evaluate("Remove(size_t, size_t)", "maximum size", "The largest possible size is clipped as well", MString(Base).Remove(1, numeric_limits<size_t>::max()), MString("a")) && Passed;
  Passed = Evaluate("Remove(size_t, size_t)", "n", "Removing from the length changes nothing", MString(Base).Remove(4, 1), MString("abcd")) && Passed;
  Passed = Evaluate("Remove(size_t, size_t)", "n+1", "Removing beyond the length changes nothing", MString(Base).Remove(5, 1), MString("abcd")) && Passed;
  Passed = Evaluate("Remove(size_t, size_t)", "size 0", "Removing zero characters changes nothing", MString(Base).Remove(1, 0), MString("abcd")) && Passed;
  Passed = Evaluate("Remove(size_t, size_t)", "middle", "Removing a middle segment keeps both ends", MString(Base).Remove(1, 2), MString("ad")) && Passed;

  Passed = Evaluate("RemoveLast()", "0", "Removing zero characters changes nothing", MString(Base).RemoveLast(0), MString("abcd")) && Passed;
  Passed = Evaluate("RemoveLast()", "1", "Removing one character removes the last one", MString(Base).RemoveLast(1), MString("abc")) && Passed;
  Passed = Evaluate("RemoveLast()", "n-1", "Removing n-1 characters leaves the first character", MString(Base).RemoveLast(3), MString("a")) && Passed;
  Passed = Evaluate("RemoveLast()", "n", "Removing n characters leaves an empty string", MString(Base).RemoveLast(4), MString("")) && Passed;
  Passed = Evaluate("RemoveLast()", "n+1", "Removing more than n characters leaves an empty string", MString(Base).RemoveLast(5), MString("")) && Passed;
  Passed = Evaluate("RemoveLast()", "maximum size", "Removing the largest possible number leaves an empty string", MString(Base).RemoveLast(numeric_limits<size_t>::max()), MString("")) && Passed;
  Passed = Evaluate("RemoveLast()", "empty string", "Removing from an empty string stays empty", MString("").RemoveLast(1), MString("")) && Passed;

  Passed = Evaluate("Replace()", "n-1", "Replacing the last character with a longer text extends the string", MString(Base).Replace(3, 1, "XY"), MString("abcXY")) && Passed;
  Passed = Evaluate("Replace()", "n-1, large size", "A size beyond the end is clipped", MString(Base).Replace(3, 5, "Z"), MString("abcZ")) && Passed;
  Passed = Evaluate("Replace()", "n", "Replacing from the length changes nothing", MString(Base).Replace(4, 1, "Z"), MString("abcd")) && Passed;
  Passed = Evaluate("Replace()", "n+1", "Replacing beyond the length changes nothing", MString(Base).Replace(5, 1, "Z"), MString("abcd")) && Passed;
  Passed = Evaluate("Replace()", "size 0", "Replacing zero characters inserts the text", MString(Base).Replace(1, 0, "Z"), MString("aZbcd")) && Passed;
  Passed = Evaluate("Replace()", "empty replacement", "Replacing with an empty text removes the segment", MString(Base).Replace(1, 2, ""), MString("ad")) && Passed;
  Passed = Evaluate("Replace()", "start 0", "Replacing at the start replaces the first character", MString(Base).Replace(0, 1, "Z"), MString("Zbcd")) && Passed;

  // ReplaceAll: all occurrences, left to right, without rescanning the inserted text
  Passed = Evaluate("ReplaceAll()", "growing", "A replacement which contains the pattern is not replaced again", MString("aaa").ReplaceAll("a", "aa"), MString("aaaaaa")) && Passed;
  Passed = Evaluate("ReplaceAll()", "shrinking", "A longer pattern is replaced by a shorter text", MString("abcabc").ReplaceAll("abc", "x"), MString("xx")) && Passed;
  Passed = Evaluate("ReplaceAll()", "absent", "An absent pattern changes nothing", MString("abc").ReplaceAll("x", "y"), MString("abc")) && Passed;
  Passed = Evaluate("ReplaceAll()", "empty pattern", "An empty pattern changes nothing", MString("abc").ReplaceAll("", "y"), MString("abc")) && Passed;
  Passed = Evaluate("ReplaceAll()", "empty replacement", "An empty replacement removes the pattern", MString("a-b-c").ReplaceAll("-", ""), MString("abc")) && Passed;
  Passed = Evaluate("ReplaceAll()", "longer pattern", "A pattern longer than the string changes nothing", MString("ab").ReplaceAll("abc", "x"), MString("ab")) && Passed;
  Passed = Evaluate("ReplaceAll()", "case", "The pattern is case sensitive", MString("aA").ReplaceAll("a", "x"), MString("xA")) && Passed;

  // ReplaceAtEnd: only if the string ends with the pattern, and only once
  Passed = Evaluate("ReplaceAtEnd()", "at end", "The pattern at the end is replaced", MString("file.txt").ReplaceAtEnd(".txt", ".dat"), MString("file.dat")) && Passed;
  Passed = Evaluate("ReplaceAtEnd()", "inside", "A pattern which is not at the end is not replaced", MString("a.txt.b").ReplaceAtEnd(".txt", ".dat"), MString("a.txt.b")) && Passed;
  Passed = Evaluate("ReplaceAtEnd()", "twice at end", "Only the last occurrence at the end is replaced", MString("abab").ReplaceAtEnd("ab", "X"), MString("abX")) && Passed;
  Passed = Evaluate("ReplaceAtEnd()", "whole string", "A pattern equal to the whole string replaces everything", MString("ab").ReplaceAtEnd("ab", "X"), MString("X")) && Passed;
  Passed = Evaluate("ReplaceAtEnd()", "empty pattern", "An empty pattern appends the replacement", MString("ab").ReplaceAtEnd("", "X"), MString("abX")) && Passed;
  Passed = Evaluate("ReplaceAtEnd()", "longer pattern", "A pattern longer than the string changes nothing", MString("b").ReplaceAtEnd("ab", "X"), MString("b")) && Passed;

  // RemoveAll
  Passed = Evaluate("RemoveAll()", "all", "All occurrences are removed", MString("a-b-c").RemoveAll("-"), MString("abc")) && Passed;
  Passed = Evaluate("RemoveAll()", "adjacent", "Adjacent occurrences are all removed", MString("aaaa").RemoveAll("aa"), MString("")) && Passed;
  Passed = Evaluate("RemoveAll()", "odd", "A remaining partial match stays", MString("aaa").RemoveAll("aa"), MString("a")) && Passed;
  Passed = Evaluate("RemoveAll()", "empty pattern", "An empty pattern changes nothing", MString("abc").RemoveAll(""), MString("abc")) && Passed;
  Passed = Evaluate("RemoveAll()", "absent", "An absent pattern changes nothing", MString("abc").RemoveAll("x"), MString("abc")) && Passed;
  Passed = Evaluate("RemoveAll()", "whole string", "A pattern equal to the whole string leaves an empty string", MString("abc").RemoveAll("abc"), MString("")) && Passed;

  // Strip: only the given character, only at the ends
  Passed = Evaluate("Strip()", "default", "The default strips spaces at both ends and keeps inner spaces", MString("  a b  ").Strip(), MString("a b")) && Passed;
  Passed = Evaluate("Strip()", "only that character", "Strip does not remove other white space", MString(" \ta\t ").Strip(), MString("\ta\t")) && Passed;
  Passed = Evaluate("Strip()", "all characters", "A string of only the character becomes empty", MString("xxx").Strip('x'), MString("")) && Passed;
  Passed = Evaluate("Strip()", "empty", "An empty string stays empty", MString("").Strip(), MString("")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the search functions and predicates
bool UTString::TestSearchAndPredicates()
{
  bool Passed = true;

  // The string "abcabc" has the length n = 6
  const MString S("abcabc");

  Passed = EvaluateTrue("Contains()", "inner", "Contains finds an inner substring", S.Contains("bca")) && Passed;
  Passed = EvaluateTrue("Contains()", "empty", "Contains finds the empty string in any string", S.Contains("")) && Passed;
  Passed = EvaluateTrue("Contains()", "whole", "Contains finds the whole string", S.Contains("abcabc")) && Passed;
  Passed = EvaluateFalse("Contains()", "longer", "Contains does not find a pattern which is longer than the string", S.Contains("abcabcd")) && Passed;
  Passed = EvaluateFalse("Contains()", "absent", "Contains does not find an absent pattern", S.Contains("x")) && Passed;
  Passed = EvaluateFalse("Contains()", "case", "Contains is case sensitive", S.Contains("ABC")) && Passed;
  Passed = EvaluateFalse("Contains()", "empty string", "An empty string contains no character", MString("").Contains("a")) && Passed;
  Passed = EvaluateTrue("Contains()", "empty in empty", "An empty string contains the empty string", MString("").Contains("")) && Passed;

  Passed = EvaluateTrue("BeginsWith()", "start", "BeginsWith finds the start", S.BeginsWith("abc")) && Passed;
  Passed = EvaluateTrue("BeginsWith()", "empty", "BeginsWith accepts the empty string", S.BeginsWith("")) && Passed;
  Passed = EvaluateTrue("BeginsWith()", "whole", "BeginsWith accepts the whole string", S.BeginsWith("abcabc")) && Passed;
  Passed = EvaluateFalse("BeginsWith()", "inner", "BeginsWith rejects a pattern which is not at the start", S.BeginsWith("bc")) && Passed;
  Passed = EvaluateFalse("BeginsWith()", "longer", "BeginsWith rejects a pattern which is longer than the string", S.BeginsWith("abcabcd")) && Passed;
  Passed = EvaluateFalse("BeginsWith()", "case", "BeginsWith is case sensitive", S.BeginsWith("A")) && Passed;
  Passed = EvaluateTrue("BeginsWith()", "single", "BeginsWith works for a single character", MString("a").BeginsWith("a")) && Passed;
  Passed = EvaluateFalse("BeginsWith()", "empty string", "An empty string does not begin with a character", MString("").BeginsWith("a")) && Passed;
  Passed = EvaluateTrue("BeginsWith()", "empty string", "An empty string begins with the empty string", MString("").BeginsWith("")) && Passed;

  Passed = EvaluateTrue("EndsWith()", "end", "EndsWith finds the end", S.EndsWith("abc")) && Passed;
  Passed = EvaluateTrue("EndsWith()", "empty", "EndsWith accepts the empty string", S.EndsWith("")) && Passed;
  Passed = EvaluateTrue("EndsWith()", "whole", "EndsWith accepts the whole string", S.EndsWith("abcabc")) && Passed;
  Passed = EvaluateFalse("EndsWith()", "inner", "EndsWith rejects a pattern which is not at the end", S.EndsWith("ab")) && Passed;
  Passed = EvaluateFalse("EndsWith()", "longer", "EndsWith rejects a pattern which is longer than the string", S.EndsWith("xabcabc")) && Passed;
  Passed = EvaluateFalse("EndsWith()", "case", "EndsWith is case sensitive", S.EndsWith("C")) && Passed;
  Passed = EvaluateTrue("EndsWith()", "single", "EndsWith works for a single character", MString("a").EndsWith("a")) && Passed;
  Passed = EvaluateFalse("EndsWith()", "empty string", "An empty string does not end with a character", MString("").EndsWith("a")) && Passed;
  Passed = EvaluateTrue("EndsWith()", "empty string", "An empty string ends with the empty string", MString("").EndsWith("")) && Passed;

  // Positions: the first occurrence is at 0, the second at 3, the string ends at n = 6
  Passed = Evaluate("Index()", "start 0", "Index finds the first occurrence", S.Index("abc"), static_cast<size_t>(0)) && Passed;
  Passed = Evaluate("Index()", "start 1", "Index starting after the first character finds the second occurrence", S.Index("abc", 1), static_cast<size_t>(3)) && Passed;
  Passed = Evaluate("Index()", "start 3", "Index starting at the second occurrence finds it", S.Index("abc", 3), static_cast<size_t>(3)) && Passed;
  Passed = Evaluate("Index()", "start 4", "Index starting after the second occurrence finds nothing", S.Index("abc", 4), MString::npos) && Passed;
  Passed = Evaluate("Index()", "empty at n", "Index of the empty string at the length is the length", S.Index("", 6), static_cast<size_t>(6)) && Passed;
  Passed = Evaluate("Index()", "empty at n+1", "Index of the empty string beyond the length finds nothing", S.Index("", 7), MString::npos) && Passed;
  Passed = Evaluate("First()", "b", "First finds the first occurrence of a character", S.First('b'), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("Last()", "b", "Last finds the last occurrence of a character", S.Last('b'), static_cast<size_t>(4)) && Passed;
  Passed = Evaluate("First()", "absent", "First of an absent character is npos", S.First('x'), MString::npos) && Passed;
  Passed = Evaluate("Last()", "absent", "Last of an absent character is npos", S.Last('x'), MString::npos) && Passed;
  Passed = Evaluate("First()", "empty string", "First in an empty string is npos", MString("").First('a'), MString::npos) && Passed;
  Passed = Evaluate("Last()", "empty string", "Last in an empty string is npos", MString("").Last('a'), MString::npos) && Passed;
  Passed = Evaluate("FindFirst()", "bc", "FindFirst finds the first occurrence", S.FindFirst("bc"), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("FindLast()", "bc", "FindLast finds the last occurrence", S.FindLast("bc"), static_cast<size_t>(4)) && Passed;
  Passed = Evaluate("FindFirst()", "empty", "FindFirst of the empty string is 0", S.FindFirst(""), static_cast<size_t>(0)) && Passed;
  Passed = Evaluate("FindLast()", "empty", "FindLast of the empty string is the length", S.FindLast(""), static_cast<size_t>(6)) && Passed;
  Passed = Evaluate("FindFirst()", "absent", "FindFirst of an absent pattern is npos", S.FindFirst("x"), MString::npos) && Passed;
  Passed = Evaluate("FindLast()", "longer", "FindLast of a longer pattern is npos", S.FindLast("abcabcd"), MString::npos) && Passed;

  // The hash only depends on the content
  Passed = Evaluate("GetHash()", "equal content", "Equal strings have equal hashes", MString("abc").GetHash(), MString(string("abc")).GetHash()) && Passed;
  Passed = Evaluate("GetHash()", "empty", "Two empty strings have equal hashes", MString("").GetHash(), MString().GetHash()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test calls in which an argument is the string itself
bool UTString::TestAliasing()
{
  bool Passed = true;

  // The replacement is the string itself: each occurrence is replaced by the ORIGINAL string
  // "aba" with each "a" replaced by "aba": "aba" + "b" + "aba" = "abababa"
  MString ReplaceAllSelf("aba");
  ReplaceAllSelf.ReplaceAllInPlace("a", ReplaceAllSelf);
  Passed = Evaluate("ReplaceAllInPlace()", "replacement is the string", "A replacement which is the string itself is the original string", ReplaceAllSelf, MString("abababa")) && Passed;

  // "abc" ends with "bc", which is replaced by the original "abc": "a" + "abc"
  MString ReplaceAtEndSelf("abc");
  ReplaceAtEndSelf.ReplaceAtEndInPlace("bc", ReplaceAtEndSelf);
  Passed = Evaluate("ReplaceAtEndInPlace()", "replacement is the string", "A replacement which is the string itself is the original string", ReplaceAtEndSelf, MString("aabc")) && Passed;

  // The pattern is the string itself: the whole string is replaced once
  MString PatternSelf("aa");
  PatternSelf.ReplaceAllInPlace(PatternSelf, "x");
  Passed = Evaluate("ReplaceAllInPlace()", "pattern is the string", "A pattern which is the string itself replaces the whole string", PatternSelf, MString("x")) && Passed;

  MString AtEndPatternSelf("abc");
  AtEndPatternSelf.ReplaceAtEndInPlace(AtEndPatternSelf, "x");
  Passed = Evaluate("ReplaceAtEndInPlace()", "pattern is the string", "A pattern which is the string itself replaces the whole string", AtEndPatternSelf, MString("x")) && Passed;

  // Removing the string from itself leaves an empty string - a regression shows up as a hang, which testdrive stops with its timeout
  MString RemoveSelfInPlace("abc");
  RemoveSelfInPlace.RemoveAllInPlace(RemoveSelfInPlace);
  Passed = Evaluate("RemoveAllInPlace()", "pattern is the string", "Removing the string from itself leaves an empty string", RemoveSelfInPlace, MString("")) && Passed;
  MString RemoveSelf("abc");
  RemoveSelf.RemoveAll(RemoveSelf);
  Passed = Evaluate("RemoveAll()", "pattern is the string", "RemoveAll of the string itself leaves an empty string", RemoveSelf, MString("")) && Passed;

  // Appending and prepending the string to itself doubles it
  MString AppendSelf("ab");
  AppendSelf.AppendInPlace(AppendSelf);
  Passed = Evaluate("AppendInPlace(MString&)", "string itself", "Appending the string to itself doubles it", AppendSelf, MString("abab")) && Passed;
  MString PrependSelf("ab");
  PrependSelf.PrependInPlace(PrependSelf);
  Passed = Evaluate("PrependInPlace(MString&)", "string itself", "Prepending the string to itself doubles it", PrependSelf, MString("abab")) && Passed;
  MString OperatorSelf("ab");
  OperatorSelf += OperatorSelf;
  Passed = Evaluate("operator+=(const MString&)", "string itself", "Adding the string to itself doubles it", OperatorSelf, MString("abab")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test embedded zeros and bytes above 127
bool UTString::TestBinaryContent()
{
  bool Passed = true;

  // A std::string can contain a zero: "a", 0, "b"
  const string WithZero("a\0b", 3);
  MString S(WithZero);
  Passed = Evaluate("MString(const string&)", "embedded zero", "A zero inside the string is kept in the length", S.Length(), static_cast<size_t>(3)) && Passed;
  Passed = Evaluate("GetString()", "embedded zero", "A zero inside the string is kept in the content", S.GetString(), WithZero) && Passed;
  Passed = EvaluateTrue("Contains()", "embedded zero", "A pattern with an embedded zero is found", S.Contains(MString(string("\0b", 2)))) && Passed;
  S.ToUpperInPlace();
  Passed = Evaluate("ToUpperInPlace()", "embedded zero", "Case conversion converts the letters on both sides of the zero", S.GetString(), string("A\0B", 3)) && Passed;
  Passed = EvaluateTrue("Tokenize()", "embedded zero", "A zero can be used as the delimiter",
                        MString(string("a\0b", 3)).Tokenize(MString(string("\0", 1)), true) == vector<MString>({ "a", "b" })) && Passed;

  // Bytes above 127 (UTF-8 umlaut A = 0xC3 0x84) are not letters in the C locale and are not changed
  const string Umlaut("\xc3\x84");
  MString Utf8(Umlaut);
  Utf8.ToLowerInPlace();
  Passed = Evaluate("ToLowerInPlace()", "bytes above 127", "Bytes above 127 are not changed by ToLower", Utf8.GetString(), Umlaut) && Passed;
  Utf8.ToUpperInPlace();
  Passed = Evaluate("ToUpperInPlace()", "bytes above 127", "Bytes above 127 are not changed by ToUpper", Utf8.GetString(), Umlaut) && Passed;
  Passed = EvaluateTrue("AreIdentical()", "bytes above 127", "Strings with bytes above 127 are compared byte by byte when ignoring the case", MString(Umlaut).AreIdentical(MString(Umlaut), true)) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "bytes above 127", "Bytes above 127 are not digits", MString(Umlaut).IsPositiveInteger()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the integer conversions and the type test
bool UTString::TestIntegerConversions()
{
  bool Passed = true;

  // The limits come from std::to_string, so that the test is independent of the width of long
  Passed = Evaluate("ToLong()", "maximum", "ToLong converts the maximum", MString(to_string(numeric_limits<long>::max())).ToLong(), numeric_limits<long>::max()) && Passed;
  Passed = Evaluate("ToLong()", "minimum", "ToLong converts the minimum", MString(to_string(numeric_limits<long>::min())).ToLong(), numeric_limits<long>::min()) && Passed;
  Passed = Evaluate("ToLong()", "leading white space", "ToLong skips leading white space", MString("  42").ToLong(), 42L) && Passed;
  Passed = Evaluate("ToLong()", "plus", "ToLong accepts a plus sign", MString("+7").ToLong(), 7L) && Passed;
  Passed = Evaluate("ToLong()", "minus zero", "ToLong converts minus zero to zero", MString("-0").ToLong(), 0L) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToLong()", "maximum + 1 digit", "ToLong throws if the value is too large", []() { MString(to_string(numeric_limits<long>::max()) + "0").ToLong(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToLong()", "minimum - 1 digit", "ToLong throws if the value is too small", []() { MString(to_string(numeric_limits<long>::min()) + "0").ToLong(); }) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("ToLong()", "text", "ToLong throws on non-numeric input", []() { MString("abc").ToLong(); }) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("ToLong()", "empty", "ToLong throws on an empty string", []() { MString("").ToLong(); }) && Passed;

  Passed = Evaluate("ToUnsignedInt()", "leading white space", "ToUnsignedInt skips leading white space", MString("  42").ToUnsignedInt(), 42U) && Passed;
  Passed = Evaluate("ToUnsignedInt()", "plus", "ToUnsignedInt accepts a plus sign", MString("+7").ToUnsignedInt(), 7U) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("ToUnsignedInt()", "empty", "ToUnsignedInt throws on an empty string", []() { MString("").ToUnsignedInt(); }) && Passed;
  // The first value which does not fit: UINT_MAX + 1 = 4294967296 (the value is only representable in unsigned long)
  if (numeric_limits<unsigned long>::max() > numeric_limits<unsigned int>::max()) {
    const MString TooLarge(to_string(static_cast<unsigned long>(numeric_limits<unsigned int>::max()) + 1UL));
    Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", TooLarge, "ToUnsignedInt throws a MEGAlib exception if the value does not fit into an unsigned int", [&]() { TooLarge.ToUnsignedInt(); }) && Passed;
    const MString TooLargeByTen(to_string(static_cast<unsigned long>(numeric_limits<unsigned int>::max()) * 2UL));
    Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", TooLargeByTen, "ToUnsignedInt throws for 2 * UINT_MAX, which truncates to a valid-looking number", [&]() { TooLargeByTen.ToUnsignedInt(); }) && Passed;
  }
  // A negative number must not wrap around to a huge value (stoul alone would give 2^64 - 1 for "-1")
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", "-1", "ToUnsignedInt throws for a negative number", []() { MString("-1").ToUnsignedInt(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", " -1", "ToUnsignedInt throws for a negative number after white space", []() { MString(" -1").ToUnsignedInt(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", "-0", "ToUnsignedInt throws for a minus sign, also in front of zero", []() { MString("-0").ToUnsignedInt(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedLong()", "-1", "ToUnsignedLong throws for a negative number", []() { MString("-1").ToUnsignedLong(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedLong()", " -1", "ToUnsignedLong throws for a negative number after white space", []() { MString(" -1").ToUnsignedLong(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedLong()", "-0", "ToUnsignedLong throws for a minus sign, also in front of zero", []() { MString("-0").ToUnsignedLong(); }) && Passed;
  Passed = Evaluate("ToUnsignedInt()", "0", "ToUnsignedInt converts zero", MString("0").ToUnsignedInt(), 0U) && Passed;
  Passed = Evaluate("ToUnsignedLong()", "0", "ToUnsignedLong converts zero", MString("0").ToUnsignedLong(), 0UL) && Passed;
  Passed = Evaluate("ToUnsignedLong()", "1-", "A minus sign behind the number does not matter", MString("1-").ToUnsignedLong(), 1UL) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("ToUnsignedLong()", "empty", "ToUnsignedLong throws on an empty string", []() { MString("").ToUnsignedLong(); }) && Passed;
  Passed = Evaluate("ToUnsignedLong()", "plus", "ToUnsignedLong accepts a plus sign", MString("+7").ToUnsignedLong(), 7UL) && Passed;

  Passed = Evaluate("ToInt()", "negative", "ToInt converts a negative number", MString("-42").ToInt(), -42) && Passed;
  Passed = Evaluate("ToInt()", "plus", "ToInt accepts a plus sign", MString("+7").ToInt(), 7) && Passed;
  Passed = Evaluate("ToInt()", "leading white space", "ToInt skips leading white space", MString(" 7").ToInt(), 7) && Passed;
  Passed = EvaluateNear("ToDouble()", "-2.5e-3", "ToDouble converts scientific notation", MString("-2.5e-3").ToDouble(), -0.0025, 1e-15) && Passed;
  Passed = EvaluateNear("ToDouble()", "1500", "ToDouble converts a plain number", MString("1500").ToDouble(), 1500.0, 1e-12) && Passed;

  // IsPositiveInteger: remaining malformed forms
  Passed = EvaluateFalse("IsPositiveInteger()", "empty", "IsPositiveInteger rejects an empty string", MString("").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "1 2", "IsPositiveInteger rejects white space inside the number", MString("1 2").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "++1", "IsPositiveInteger rejects two plus signs", MString("++1").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "+-1", "IsPositiveInteger rejects a plus and a minus sign", MString("+-1").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "1+", "IsPositiveInteger rejects a trailing sign", MString("1+").IsPositiveInteger()) && Passed;
  Passed = EvaluateTrue("IsPositiveInteger()", "007", "IsPositiveInteger accepts leading zeros", MString("007").IsPositiveInteger()) && Passed;
  Passed = EvaluateTrue("IsPositiveInteger()", "very long", "IsPositiveInteger has no range limit", MString("123456789012345678901234567890").IsPositiveInteger()) && Passed;

  // Is<T>: the value has to be readable as T, with optional surrounding white space
  Passed = EvaluateTrue("Is<int>()", "42", "42 is an int", MString("42").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", " 12 ", "An int with surrounding white space is an int", MString(" 12 ").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", "-5", "A negative number is an int", MString("-5").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", "INT_MAX", "The largest int is an int", MString("2147483647").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "INT_MAX+1", "The largest int plus one is not an int", MString("2147483648").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", "INT_MIN", "The smallest int is an int", MString("-2147483648").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "INT_MIN-1", "The smallest int minus one is not an int", MString("-2147483649").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "1.5", "A decimal number is not an int", MString("1.5").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "12x", "A number with trailing text is not an int", MString("12x").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "empty", "An empty string is not an int", MString("").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "text", "Text is not an int", MString("abc").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<unsigned int>()", "UINT_MAX", "The largest unsigned int is an unsigned int", MString("4294967295").Is<unsigned int>()) && Passed;
  Passed = EvaluateFalse("Is<unsigned int>()", "UINT_MAX+1", "The largest unsigned int plus one is not an unsigned int", MString("4294967296").Is<unsigned int>()) && Passed;
  Passed = EvaluateFalse("Is<unsigned int>()", "-1", "A negative number is not an unsigned int", MString("-1").Is<unsigned int>()) && Passed;
  Passed = EvaluateTrue("Is<unsigned int>()", "0", "Zero is an unsigned int", MString("0").Is<unsigned int>()) && Passed;
  Passed = EvaluateTrue("Is<double>()", "1e3", "Scientific notation is a double", MString("1e3").Is<double>()) && Passed;
  Passed = EvaluateTrue("Is<double>()", "-0.5", "A negative decimal is a double", MString("-0.5").Is<double>()) && Passed;
  Passed = EvaluateTrue("Is<double>()", "integer", "An integer is a double", MString("12").Is<double>()) && Passed;
  Passed = EvaluateFalse("Is<double>()", "1.2.3", "Two decimal points are not a double", MString("1.2.3").Is<double>()) && Passed;
  Passed = EvaluateFalse("Is<double>()", "text", "Text is not a double", MString("abc").Is<double>()) && Passed;
  Passed = EvaluateTrue("Is<float>()", "0.25", "0.25 is a float", MString("0.25").Is<float>()) && Passed;
  Passed = EvaluateTrue("Is<short>()", "32767", "The largest short is a short", MString("32767").Is<short>()) && Passed;
  Passed = EvaluateFalse("Is<short>()", "32768", "The largest short plus one is not a short", MString("32768").Is<short>()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test stream functions on reused objects
bool UTString::TestStreamReuse()
{
  bool Passed = true;

  // ReadLine on a populated receiver, repeated until the stream is exhausted, with a blank line
  istringstream Lines("first\n\nthird");
  MString Line("old content");
  Passed = EvaluateTrue("ReadLine()", "stream", "ReadLine returns the stream it reads from", &Line.ReadLine(Lines) == &Lines) && Passed;
  Passed = Evaluate("ReadLine()", "first", "ReadLine replaces the old content with the first line", Line, MString("first")) && Passed;
  Line.ReadLine(Lines);
  Passed = Evaluate("ReadLine()", "blank line", "ReadLine reads a blank line as an empty string", Line, MString("")) && Passed;
  Passed = EvaluateTrue("ReadLine()", "blank line state", "Reading a blank line does not set the failure state", Lines.fail() == false) && Passed;
  Line.ReadLine(Lines);
  Passed = Evaluate("ReadLine()", "last line without newline", "ReadLine reads a last line without terminating newline", Line, MString("third")) && Passed;
  Line = "stale";
  Line.ReadLine(Lines);
  Passed = Evaluate("ReadLine()", "exhausted", "ReadLine on an exhausted stream leaves no stale content", Line, MString("")) && Passed;
  Passed = EvaluateTrue("ReadLine()", "exhausted state", "ReadLine on an exhausted stream sets the failure state", Lines.fail()) && Passed;

  // Read on a populated receiver, then on the exhausted stream
  istringstream All("x\ny");
  MString Everything("stale");
  Everything.Read(All);
  Passed = Evaluate("Read()", "populated receiver", "Read replaces the old content with all lines", Everything, MString("x\ny\n")) && Passed;
  Everything.Read(All);
  Passed = Evaluate("Read()", "exhausted", "Reading an exhausted stream gives an empty string", Everything, MString("")) && Passed;

  // operator>> reads one white-space separated word and can be repeated
  istringstream Words("one  two\nthree");
  MString First;
  MString Second;
  MString Third;
  Passed = EvaluateTrue("operator>>", "stream", "operator>> returns the stream", &(Words >> First) == &Words) && Passed;
  Words >> Second >> Third;
  Passed = Evaluate("operator>>", "first word", "The first word is read", First, MString("one")) && Passed;
  Passed = Evaluate("operator>>", "second word", "The second word is read after skipping white space", Second, MString("two")) && Passed;
  Passed = Evaluate("operator>>", "third word", "The third word is read after a newline", Third, MString("three")) && Passed;
  First = "stale";
  Words >> First;
  // As for std::string, a failed extraction does not touch the target
  Passed = Evaluate("operator>>", "exhausted", "A failed extraction from an exhausted stream leaves the target unchanged", First, MString("stale")) && Passed;
  Passed = EvaluateTrue("operator>>", "exhausted state", "Extraction from an exhausted stream sets the failure state", Words.fail()) && Passed;

  // operator<< returns the stream and can be chained
  ostringstream Out;
  Passed = EvaluateTrue("operator<<", "stream", "operator<< returns the stream", &(Out << MString("ab")) == &Out) && Passed;
  Out << MString("") << MString("cd");
  Passed = Evaluate("operator<<", "chained", "Chained output is written in order", MString(Out.str()), MString("abcd")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the number formatting with precision and uncertainty and the matching of printed numbers
bool UTString::TestPrecisionAndUncertainty()
{
  bool Passed = true;

  // Precision: the number of significant digits, as in %g
  Passed = Evaluate("MString(double, unsigned int)", "pi, 3", "Three significant digits of pi", MString(3.14159265, 3U), MString("3.14")) && Passed;
  Passed = Evaluate("MString(double, unsigned int)", "1234.5678, 6", "Six significant digits", MString(1234.5678, 6U), MString("1234.57")) && Passed;
  Passed = Evaluate("MString(double, unsigned int)", "1234.5678, 3", "Three significant digits of a larger number switch to scientific notation", MString(1234.5678, 3U), MString("1.23e+03")) && Passed;
  Passed = Evaluate("MString(double, unsigned int)", "0", "Zero is printed as 0", MString(0.0, 3U), MString("0")) && Passed;
  Passed = Evaluate("MString(double, unsigned int)", "-0.5", "A negative number keeps its sign", MString(-0.5, 3U), MString("-0.5")) && Passed;
  Passed = Evaluate("MString(double, unsigned int)", "1e-7", "A small number is printed in scientific notation", MString(1e-7, 3U), MString("1e-07")) && Passed;

  // Value and uncertainty, rounded to the first significant digit of the uncertainty,
  // or the first two if the uncertainty starts with 1 or 2 (rounds to less than 3)
  // 1.872 starts with 2 after rounding -> one more digit: 12.345 -> 12.3, 1.872 -> 1.9
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345, 1.872, mm", "Value and uncertainty are rounded with units", MString(12.345, 1.872, "mm"), MString("(12.3 ± 1.9) mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345, 1.872, mm, Latex", "The Latex form uses #pm", MString(12.345, 1.872, "mm", true), MString("(12.3 #pm 1.9) mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345, 1.872", "Without units there are no parentheses", MString(12.345, 1.872), MString("12.3 ± 1.9")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345, 1.872, Latex", "Without units the Latex form has no parentheses", MString(12.345, 1.872, "", true), MString("12.3 #pm 1.9")) && Passed;
  // 0.348: the first digit is at 10^-1, 0.348 rounds to 0.3 (not below 3 at that digit) -> 12.345 -> 12.3
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345, 0.348", "An uncertainty below 1 is rounded at its first digit", MString(12.345, 0.348), MString("12.3 ± 0.3")) && Passed;
  // 187.2 starts with 1 -> two digits: 5432.1 -> 5430, 187.2 -> 190
  Passed = Evaluate("MString(double, double, MString, bool)", "5432.1, 187.2, mm", "A large uncertainty is rounded in front of the decimal point", MString(5432.1, 187.2, "mm"), MString("(5430 ± 190) mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "-12.345, 1.872", "A negative value keeps its sign", MString(-12.345, 1.872), MString("-12.3 ± 1.9")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "0, 0.5", "A value of zero is printed as 0", MString(0.0, 0.5), MString("0.0 ± 0.5")) && Passed;
  // Threshold between one and two digits: 2.49 rounds to 2 (< 3, two digits), 2.5 rounds to 3 (one digit)
  Passed = Evaluate("MString(double, double, MString, bool)", "10, 2.49", "An uncertainty just below the threshold gets two digits", MString(10.0, 2.49), MString("10.0 ± 2.5")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "10, 2.5", "An uncertainty at the threshold gets one digit", MString(10.0, 2.5), MString("10 ± 3")) && Passed;
  // A large value with a small uncertainty: the value must not change, rounding to 3 decimals has nothing to do
  // 1e20 = 2^20 * 5^20 is exactly representable and printed exactly: 100000000000000000000
  Passed = Evaluate("MString(double, double, MString, bool)", "1e20, 0.01", "A large value is not changed by the rounding", MString(1e20, 0.01), MString("100000000000000000000.000 ± 0.010")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "-1e20, 0.01", "A large negative value is not changed by the rounding", MString(-1e20, 0.01), MString("-100000000000000000000.000 ± 0.010")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "1e13, 0.01, mm", "A value above 2^53 in units of the round-off digit is not changed", MString(1e13, 0.01, "mm"), MString("(10000000000000.000 ± 0.010) mm")) && Passed;
  // Just below the limit (1e12 in units of 1e-3 is 1e15 < 2^53 = 9.007e15) the value is still rounded: 1e12 + 0.0004 -> 1e12 + 0.000
  Passed = Evaluate("MString(double, double, MString, bool)", "1e12+0.0004, 0.01", "A value below the limit is still rounded", MString(1e12 + 0.0004, 0.01), MString("1000000000000.000 ± 0.010")) && Passed;
  // Large uncertainty: 1e20 +- 100 starts with 1 -> round to tens (digit 1), the value is 1e20 in units of 10
  Passed = Evaluate("MString(double, double, MString, bool)", "1e20, 100", "A large value with a large uncertainty is not changed", MString(1e20, 100.0), MString("100000000000000000000 ± 100")) && Passed;

  // Large uncertainties: the rounded numbers are digits followed by zeros, not the exact binary value
  // 1.872e22 starts with 2 after rounding -> two digits: 1.2345e23 -> 123e21, 1.872e22 -> 19e21
  Passed = Evaluate("MString(double, double, MString, bool)", "1.2345e23, 1.872e22", "Rounded large numbers are written as digits followed by zeros", MString(1.2345e23, 1.872e22), MString("123000000000000000000000 ± 19000000000000000000000")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "-1.2345e23, 1.872e22", "A rounded large negative value keeps its sign", MString(-1.2345e23, 1.872e22), MString("-123000000000000000000000 ± 19000000000000000000000")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "1.2345e23, 1.872e22, mm, Latex", "Rounded large numbers with unit and Latex", MString(1.2345e23, 1.872e22, "mm", true), MString("(123000000000000000000000 #pm 19000000000000000000000) mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "0, 187.2", "A value of zero with a large uncertainty is 0", MString(0.0, 187.2), MString("0 ± 190")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "-4.4, 187.2", "A value which rounds to zero is 0 without a sign", MString(-4.4, 187.2), MString("0 ± 190")) && Passed;

  // No uncertainty: both numbers are printed unchanged
  Passed = Evaluate("MString(double, double, MString, bool)", "5, 0, mm", "A zero uncertainty prints both numbers", MString(5.0, 0.0, "mm"), MString("5 ± 0 mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "5, 0, mm, Latex", "A zero uncertainty prints both numbers in Latex", MString(5.0, 0.0, "mm", true), MString("5 #pm 0 mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "5, -1", "A negative uncertainty prints both numbers", MString(5.0, -1.0), MString("5 ± -1")) && Passed;
  // No digit above the uncertainty is lost - an uncertainty of 1 gets two digits, as 1.872
  Passed = Evaluate("MString(double, double, MString, bool)", "1234567, 1, mm", "All digits of the value above the uncertainty are printed", MString(1234567.0, 1.0, "mm"), MString("(1234567.0 ± 1.0) mm")) && Passed;
  // Unrounded fallback (15 digits, as for a zero uncertainty): the scale 10^299 overflows for a value of 1.5e10
  Passed = Evaluate("MString(double, double, MString, bool)", "1.5e10, 1e-299, mm", "A scale which overflows gives the unrounded numbers with 15 digits", MString(1.5e10, 1e-299, "mm"), MString("15000000000 ± 1e-299 mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "1.5e10, 1e-299, Latex", "The unrounded fallback in Latex without units", MString(1.5e10, 1e-299, "", true), MString("15000000000 #pm 1e-299")) && Passed;
  // The subnormal 1e-310 is stored as 9.99999999999997e-311 at 15 digits (binary64 has no closer subnormal)
  Passed = Evaluate("MString(double, double, MString, bool)", "1.23456789012345, 1e-310, mm", "A subnormal uncertainty gives the unrounded numbers with 15 digits", MString(1.23456789012345, 1e-310, "mm"), MString("1.23456789012345 ± 9.99999999999997e-311 mm")) && Passed;
  // Both numbers are finite, but the rounded value would not be: 2 / 1e-308 overflows
  // The largest double needs 17 digits to be read back: 1.7976931348623157e+308 (15 digits would read as infinity)
  const MString Largest(numeric_limits<double>::max(), 1e308, "mm");
  Passed = Evaluate("MString(double, double, MString, bool)", "DBL_MAX, 1e308, mm", "A value which overflows during rounding is printed unrounded and can be read back", Largest, MString("1.7976931348623157e+308 ± 1e+308 mm")) && Passed;
  Passed = EvaluateNear("MString(double, double, MString, bool)", "DBL_MAX, 1e308, mm", "The printed value is the largest double again", Largest.GetSubString(0, Largest.Index(" ")).ToDouble(), numeric_limits<double>::max(), 0.0) && Passed;
  // Infinite uncertainty: both numbers unrounded
  Passed = Evaluate("MString(double, double, MString, bool)", "1, inf, mm", "An infinite uncertainty is printed as inf", MString(1.0, numeric_limits<double>::infinity(), "mm"), MString("1 ± inf mm")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "1, inf, Latex", "An infinite uncertainty is printed as inf in Latex", MString(1.0, numeric_limits<double>::infinity(), "", true), MString("1 #pm inf")) && Passed;
  Passed = Evaluate("MString(double, double, MString, bool)", "inf, 1", "An infinite value is printed as inf", MString(numeric_limits<double>::infinity(), 1.0), MString("inf ± 1")) && Passed;

  // Matching at 15 significant digits (the documented limit): the unit of the last digit is 1e-14
  const MString One("1.00000000000000");
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1 unit", "One unit of the last digit is within the default tolerance of two", One.AreNumbersNumericallyMatching("1.00000000000001")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "2 units", "Two units of the last digit are within the default tolerance of two", One.AreNumbersNumericallyMatching("1.00000000000002")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "3 units", "Three units of the last digit are outside the default tolerance of two", One.AreNumbersNumericallyMatching("1.00000000000003")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "3 units, tolerance 3", "Three units of the last digit are within a tolerance of three", One.AreNumbersNumericallyMatching("1.00000000000003", 3)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "2 units, tolerance 0", "Two units of the last digit are outside a tolerance of zero", One.AreNumbersNumericallyMatching("1.00000000000002", 0)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1 unit, tolerance 0", "One unit of the last digit is outside a tolerance of zero", One.AreNumbersNumericallyMatching("1.00000000000001", 0)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "4 units", "Four units of the last digit are outside the default tolerance of two", One.AreNumbersNumericallyMatching("1.00000000000004")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "negative 3 units", "Three units of the last digit are outside the default tolerance for negative numbers", MString("-1.00000000000000").AreNumbersNumericallyMatching("-1.00000000000003")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "same value, tolerance 0", "The same value in different notation matches with a tolerance of zero", MString("1.0").AreNumbersNumericallyMatching("1.00", 0)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "tolerance 0", "Different values do not match with a tolerance of zero", MString("0.5").AreNumbersNumericallyMatching("0.6", 0)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test tokenization and extraction helpers
bool UTString::TestTokenizeAndExtract()
{
  bool Passed = true;

  // Complete token lists, derived by hand: a token ends at every delimiter
  Passed = EvaluateTokens("a,,b,c,", ",", false, { "a", "", "b", "c", "" }) && Passed;
  Passed = EvaluateTokens("a,,b,c,", ",", true, { "a", "b", "c" }) && Passed;
  Passed = EvaluateTokens("abc", "", false, { "abc" }) && Passed;
  Passed = EvaluateTokens("abc", "", true, { "abc" }) && Passed;
  Passed = EvaluateTokens("", "", true, { }) && Passed;
  Passed = EvaluateTokens("", "", false, { "" }) && Passed;
  Passed = EvaluateTokens("/tmp/data/example.tra", "/", true, { "tmp", "data", "example.tra" }) && Passed;
  Passed = EvaluateTokens("/tmp/data/example.tra", "/", false, { "", "tmp", "data", "example.tra" }) && Passed;
  Passed = EvaluateTokens("3.2.1", ".", true, { "3", "2", "1" }) && Passed;
  Passed = EvaluateTokens("Method=Fast:Bins=16", ":", true, { "Method=Fast", "Bins=16" }) && Passed;
  Passed = EvaluateTokens("Bins=16", "=", true, { "Bins", "16" }) && Passed;
  Passed = EvaluateTokens("line1\nline2\nline3", "\n", true, { "line1", "line2", "line3" }) && Passed;

  // Empty input, delimiter-only input, absent delimiter, and multi-character delimiters
  Passed = EvaluateTokens("", ",", false, { "" }) && Passed;
  Passed = EvaluateTokens("", ",", true, { }) && Passed;
  Passed = EvaluateTokens(",", ",", false, { "", "" }) && Passed;
  Passed = EvaluateTokens(",", ",", true, { }) && Passed;
  Passed = EvaluateTokens(",,,", ",", false, { "", "", "", "" }) && Passed;
  Passed = EvaluateTokens(",,,", ",", true, { }) && Passed;
  Passed = EvaluateTokens("abc", ";", false, { "abc" }) && Passed;
  Passed = EvaluateTokens("abc", ";", true, { "abc" }) && Passed;
  Passed = EvaluateTokens("a::b::c", "::", true, { "a", "b", "c" }) && Passed;
  Passed = EvaluateTokens("a:b::c", "::", true, { "a:b", "c" }) && Passed;
  Passed = EvaluateTokens("a::", "::", false, { "a", "" }) && Passed;
  Passed = EvaluateTokens("a::", "::", true, { "a" }) && Passed;
  Passed = EvaluateTokens("::a", "::", false, { "", "a" }) && Passed;
  Passed = EvaluateTokens("a", "ab", true, { "a" }) && Passed;
  // Overlapping delimiter: the first "aa" at position 0 is the delimiter, the remaining "a" is the last token
  Passed = EvaluateTokens("aaa", "aa", false, { "", "a" }) && Passed;
  Passed = EvaluateTokens("aaa", "aa", true, { "a" }) && Passed;
  // The delimiter is case sensitive and a delimiter at the end gives a final empty token
  Passed = EvaluateTokens("aXbxc", "x", true, { "aXb", "c" }) && Passed;

  Passed = Evaluate("Extract()", "prefix[start]suffix", "Extract returns the text between delimiters", MString("prefix[start]suffix").Extract("[", "]"), MString("start")) && Passed;
  Passed = Evaluate("Extract()", "missing before delimiter", "Extract returns an empty string if the opening delimiter is absent", MString("prefixstart]suffix").Extract("[", "]"), MString("")) && Passed;
  Passed = Evaluate("Extract()", "missing after delimiter", "Extract returns an empty string if the closing delimiter is absent", MString("prefix[startsuffix").Extract("[", "]"), MString("")) && Passed;
  Passed = Evaluate("Extract()", "prefix.id17.tra", "Extract supports filename-style delimiter pairs", MString("prefix.id17.tra").Extract(".id", ".tra"), MString("17")) && Passed;
  Passed = Evaluate("Extract()", "empty opening delimiter", "Extract with an empty opening delimiter starts at the beginning of the string", MString("prefix]suffix").Extract("", "]"), MString("prefix")) && Passed;
  Passed = Evaluate("Extract()", "empty closing delimiter", "Extract with an empty closing delimiter returns an empty string", MString("prefix[suffix").Extract("[", ""), MString("")) && Passed;
  Passed = Evaluate("Extract()", "both delimiters empty", "Extract with two empty delimiters returns an empty string", MString("prefix").Extract("", ""), MString("")) && Passed;

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
  Line.ReadLine(SingleLine);
  Passed = Evaluate("ReadLine()", "second line", "ReadLine can be used repeatedly on the same stream", Line, MString("second line")) && Passed;

  istringstream WordStream("alpha beta");
  MString Word;
  WordStream >> Word;
  Passed = Evaluate("operator>>", "alpha beta", "operator>> reads one whitespace-delimited token", Word, MString("alpha")) && Passed;

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
  Passed = EvaluateTrue("IsPositiveInteger()", " 12 ", "IsPositiveInteger accepts leading and trailing spaces", MString(" 12 ").IsPositiveInteger()) && Passed;
  Passed = EvaluateTrue("IsPositiveInteger()", "0", "IsPositiveInteger accepts zero", MString("0").IsPositiveInteger()) && Passed;
  Passed = EvaluateTrue("IsPositiveInteger()", "+12", "IsPositiveInteger accepts an explicit plus sign", MString("+12").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "-12", "IsPositiveInteger rejects negative integers", MString("-12").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "+", "IsPositiveInteger rejects a plus sign without digits", MString("+").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "12.0", "IsPositiveInteger rejects decimal numbers", MString("12.0").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "12x", "IsPositiveInteger rejects trailing non-whitespace", MString("12x").IsPositiveInteger()) && Passed;
  Passed = EvaluateFalse("IsPositiveInteger()", "   ", "IsPositiveInteger rejects whitespace-only strings", MString("   ").IsPositiveInteger()) && Passed;

  Passed = EvaluateTrue("Is<int>()", "123", "Template type check accepts valid integers", MString("123").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", "123 ", "Template type check ignores trailing whitespace", MString("123 ").Is<int>()) && Passed;
  Passed = EvaluateTrue("Is<int>()", " 123", "Template type check ignores leading whitespace", MString(" 123").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<int>()", "123.4", "Template type check rejects mismatched formats", MString("123.4").Is<int>()) && Passed;
  Passed = EvaluateFalse("Is<unsigned int>()", "-1", "Unsigned type checks reject negative values", MString("-1").Is<unsigned int>()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test floating-point recognition and printed-precision matching
bool UTString::TestFloatingPointMatching()
{
  bool Passed = true;

  Passed = EvaluateTrue("IsNumber()", "newlines and tabs around 12.5", "IsNumber accepts all standard leading and trailing whitespace", MString("\n\t12.5\r\v").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "-12.5", "IsFloatingPointNumber accepts a representative decimal value", MString("-12.5").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "1e3", "IsFloatingPointNumber accepts scientific notation without a decimal point", MString("1e3").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", " +1.E-3 ", "IsFloatingPointNumber accepts surrounding whitespace and an uppercase exponent", MString(" +1.E-3 ").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", ".5", "IsFloatingPointNumber accepts a fraction without a leading zero", MString(".5").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "1.", "IsFloatingPointNumber accepts a trailing decimal point", MString("1.").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "12", "IsFloatingPointNumber rejects an integer-only representation", MString("12").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "1e", "IsFloatingPointNumber rejects an incomplete exponent", MString("1e").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "1.23456789012345e-308", "IsFloatingPointNumber rejects a finite subnormal double representation", MString("1.23456789012345e-308").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "-1.23456789012345e-308", "IsFloatingPointNumber rejects a negative finite subnormal double representation", MString("-1.23456789012345e-308").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "1e-400", "IsFloatingPointNumber rejects a representation that underflows to zero", MString("1e-400").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "-1e-400", "IsFloatingPointNumber rejects a negative representation that underflows to zero", MString("-1e-400").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "1e309", "IsFloatingPointNumber rejects a representation outside the finite double range", MString("1e309").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "-1e309", "IsFloatingPointNumber rejects a negative representation outside the finite double range", MString("-1e309").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateFalse("IsFloatingPointNumber()", "empty input", "IsFloatingPointNumber rejects an empty string", MString("").IsFloatingPointNumber()) && Passed;

  Passed = EvaluateTrue("IsNumber()", "2.2250738585072014e-308", "IsNumber accepts the minimum positive normal double", MString("2.2250738585072014e-308").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "-2.2250738585072014e-308", "IsNumber accepts the minimum-magnitude negative normal double", MString("-2.2250738585072014e-308").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "2.225073858507201e-308", "IsNumber rejects a value immediately below the normal double range", MString("2.225073858507201e-308").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "2.2250738585072014e-308", "IsFloatingPointNumber accepts the minimum positive normal double", MString("2.2250738585072014e-308").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "1.7976931348623157e308", "IsNumber accepts the maximum finite double", MString("1.7976931348623157e308").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "1.7976931348623159e308", "IsNumber rejects a value above the maximum finite double", MString("1.7976931348623159e308").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "1.7976931348623157e308", "IsFloatingPointNumber accepts the maximum finite double", MString("1.7976931348623157e308").IsFloatingPointNumber()) && Passed;
  Passed = EvaluateTrue("IsFloatingPointNumber()", "0e-400", "IsFloatingPointNumber accepts an exact zero with a very small exponent", MString("0e-400").IsFloatingPointNumber()) && Passed;

  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "identical 3.14159 representations", "Identical floating-point representations always match", MString("3.14159").AreNumbersNumericallyMatching("3.14159")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "whitespace around 3.14159 and 3.14160", "Matching ignores leading and trailing whitespace around both floating-point representations", MString(" \t3.14159\n").AreNumbersNumericallyMatching("\r3.14160 ")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0.518236 and 0.518237", "The default tolerance accepts floating-point values one last-printed-digit unit apart", MString("0.518236").AreNumbersNumericallyMatching("0.518237")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0.518237 and 0.518236", "Numerical matching is symmetric when the operands are reversed", MString("0.518237").AreNumbersNumericallyMatching("0.518236")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.234e-4 and 1.235e-4", "The default tolerance scales the printed precision by the scientific exponent", MString("1.234e-4").AreNumbersNumericallyMatching("1.235e-4")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "+1.234E+4 and 1.235e4", "Matching accepts uppercase and explicitly signed positive exponents", MString("+1.234E+4").AreNumbersNumericallyMatching("1.235e4")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1000.0 and 1.0000e3", "Matching compares equivalent fixed and scientific representations using their printed precision", MString("1000.0").AreNumbersNumericallyMatching("1.0000e3", 0)) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.234567890123456e300 and 1.234567890123457e300", "The default tolerance accepts a one-last-digit difference for very large finite values", MString("1.234567890123456e300").AreNumbersNumericallyMatching("1.234567890123457e300")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.234567890123456e300 and 1.234567890123460e300", "The default tolerance rejects a multi-last-digit difference for very large finite values", MString("1.234567890123456e300").AreNumbersNumericallyMatching("1.234567890123460e300")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1e308 and -1e308", "Matching rejects large opposite-sign values whose difference overflows double", MString("1e308").AreNumbersNumericallyMatching("-1e308")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.234567890123456e-300 and 1.234567890123457e-300", "The default tolerance accepts a one-last-digit difference for very small finite values", MString("1.234567890123456e-300").AreNumbersNumericallyMatching("1.234567890123457e-300")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.234567890123456e-300 and 1.234567890123460e-300", "The default tolerance rejects a multi-last-digit difference for very small finite values", MString("1.234567890123456e-300").AreNumbersNumericallyMatching("1.234567890123460e-300")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "nearby subnormal values", "Matching rejects finite subnormal double representations", MString("1.23456789012345e-308").AreNumbersNumericallyMatching("1.23456789012346e-308")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "identical subnormal representations", "Matching rejects identical finite subnormal double representations", MString("1.23456789012345e-308").AreNumbersNumericallyMatching("1.23456789012345e-308")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "minimum normal in equivalent scientific representations", "Matching accepts equivalent representations of the minimum positive normal double at zero tolerance", MString("2.2250738585072014e-308").AreNumbersNumericallyMatching("0.22250738585072014e-307", 0)) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "maximum finite in equivalent scientific representations", "Matching accepts equivalent representations of the maximum finite double at zero tolerance", MString("1.7976931348623157e308").AreNumbersNumericallyMatching("17.976931348623157e307", 0)) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.000000000000000 and 1.000000000000002", "The default tolerance accepts a two-unit difference near double-precision digit capacity", MString("1.000000000000000").AreNumbersNumericallyMatching("1.000000000000002")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.000000000000000 and 1.000000000000004", "The default tolerance rejects a four-unit difference near double-precision digit capacity", MString("1.000000000000000").AreNumbersNumericallyMatching("1.000000000000004")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "-2.50 and -2.52", "The default tolerance includes the two-unit boundary for negative values", MString("-2.50").AreNumbersNumericallyMatching("-2.52")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "-2.50 and -2.521", "The default tolerance rejects values beyond two units of the higher printed precision", MString("-2.50").AreNumbersNumericallyMatching("-2.521")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "0.8737 and 0.87365", "Matching uses the smaller last-printed-digit unit from the more precise value", MString("0.8737").AreNumbersNumericallyMatching("0.87365")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "-0.0 and 0.0 with zero tolerance", "Signed zero representations match at zero tolerance", MString("-0.0").AreNumbersNumericallyMatching("0.0", 0)) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0e-2147483649 and 0.0", "Matching handles a zero representation with a negative exponent below the integer range", MString("0e-2147483649").AreNumbersNumericallyMatching("0.0")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0e2147483648 and 0.0", "Matching handles a zero representation with a positive exponent above the integer range", MString("0e2147483648").AreNumbersNumericallyMatching("0.0")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0.0e-2147483648 and 0.0", "Matching handles fractional zero at the minimum integer exponent without signed overflow", MString("0.0e-2147483648").AreNumbersNumericallyMatching("0.0")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "0.0 and 0.2", "The default tolerance includes the two-unit boundary around zero", MString("0.0").AreNumbersNumericallyMatching("0.2")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "0.0 and 0.3", "The default tolerance rejects values beyond two units around zero", MString("0.0").AreNumbersNumericallyMatching("0.3")) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.0 and 1.00 with zero tolerance", "A zero tolerance accepts equal numeric values with different floating-point representations", MString("1.0").AreNumbersNumericallyMatching("1.00", 0)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.0 and 1.1 with zero tolerance", "A zero tolerance rejects unequal numeric values", MString("1.0").AreNumbersNumericallyMatching("1.1", 0)) && Passed;
  Passed = EvaluateTrue("AreNumbersNumericallyMatching()", "1.000 and 1.003 with tolerance 3", "A custom tolerance accepts values at its last-printed-digit boundary", MString("1.000").AreNumbersNumericallyMatching("1.003", 3)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.000 and 1.004 with tolerance 3", "A custom tolerance rejects values beyond its last-printed-digit boundary", MString("1.000").AreNumbersNumericallyMatching("1.004", 3)) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "12 and 12.0", "Matching rejects an integer-only representation", MString("12").AreNumbersNumericallyMatching("12.0")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1e309 and 1e309", "Matching rejects non-finite overflow representations", MString("1e309").AreNumbersNumericallyMatching("1e309")) && Passed;
  Passed = EvaluateFalse("AreNumbersNumericallyMatching()", "1.0 and malformed input", "Matching rejects a malformed floating-point representation", MString("1.0").AreNumbersNumericallyMatching("1.0x")) && Passed;

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
  Passed = Evaluate("MString(double, double, MString, bool)", "12.0 +/- 0.0 mm", "A zero uncertainty is printed unrounded as 12 ± 0 mm", ZeroUncertainty, MString("12 ± 0 mm")) && Passed;

  MString WithUncertaintyLatex(12.345, 1.872, "mm", true);
  Passed = Evaluate("MString(double, double, MString, bool)", "12.345 +/- 1.872 mm, latex", "Value/uncertainty formatting with the Latex flag uses #pm", WithUncertaintyLatex, MString("(12.3 #pm 1.9) mm")) && Passed;

  MString NegativeUncertainty(12.0, -1.0, "mm");
  Passed = Evaluate("MString(double, double, MString, bool)", "12.0 +/- -1.0 mm", "A negative uncertainty is printed unrounded as 12 ± -1 mm", NegativeUncertainty, MString("12 ± -1 mm")) && Passed;

  MString HashA("hash me");
  MString HashB("hash me");
  MString HashC("different");
  Passed = Evaluate("GetHash()", "equal strings", "GetHash returns the same value for equal strings", HashA.GetHash(), HashB.GetHash()) && Passed;
  Passed = EvaluateFalse("GetHash()", "different strings", "GetHash distinguishes simple different strings", HashA.GetHash() == HashC.GetHash()) && Passed;

  ostringstream Out;
  Out << MString("stream");
  Passed = Evaluate("operator<<", "stream", "operator<< writes the string content to a stream", MString(Out.str()), MString("stream")) && Passed;

  // The C string points into the MString, which therefore has to stay alive as long as the pointer is used
  const MString Source("text");
  const char* AsCString = Source;
  Passed = Evaluate("operator const char*()", "text", "operator const char* exposes the string as a C string", MString(AsCString), MString("text")) && Passed;
  Passed = EvaluateTrue("operator const char*()", "same buffer", "The C string is the internal buffer of the MString", AsCString == Source.Data()) && Passed;

  TString RootString = MString("root");
  Passed = Evaluate("operator TString()", "root", "operator TString converts to a ROOT TString", MString(RootString.Data()), MString("root")) && Passed;

  MString NumericAppend("n=");
  NumericAppend += 5;
  NumericAppend += ",u=";
  NumericAppend += static_cast<unsigned int>(7);
  NumericAppend += ",l=";
  NumericAppend += static_cast<long>(9);
  NumericAppend += ",ul=";
  NumericAppend += static_cast<unsigned long>(11);
  NumericAppend += ",f=";
  NumericAppend += 1.5f;
  NumericAppend += ",d=";
  NumericAppend += 2.25;
  Passed = EvaluateTrue("operator+=", "mixed numeric appends", "operator+= appends numeric values in sequence", NumericAppend.BeginsWith("n=5,u=7,l=9,ul=11,f=1.5,d=2.25")) && Passed;
  Passed = Evaluate("operator+=", "mixed numeric appends exact", "operator+= formats the representative mixed numeric append sequence deterministically", NumericAppend, MString("n=5,u=7,l=9,ul=11,f=1.5,d=2.25")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test edge cases and defensive behavior
bool UTString::TestEdgeCases()
{
  bool Passed = true;

  MString ReplaceAllEmpty("abc");
  ReplaceAllEmpty.ReplaceAll("", "x");
  Passed = Evaluate("ReplaceAll()", "empty from pattern", "ReplaceAll is a no-op if the source pattern is empty", ReplaceAllEmpty, MString("abc")) && Passed;

  MString RemoveAllEmpty("abc");
  RemoveAllEmpty.RemoveAll("");
  Passed = Evaluate("RemoveAll()", "empty from pattern", "RemoveAll is a no-op if the source pattern is empty", RemoveAllEmpty, MString("abc")) && Passed;

  MString ReplaceAtEndEmpty("abc");
  ReplaceAtEndEmpty.ReplaceAtEnd("", "x");
  Passed = Evaluate("ReplaceAtEnd()", "empty suffix pattern", "ReplaceAtEnd with an empty suffix appends the replacement", ReplaceAtEndEmpty, MString("abcx")) && Passed;

  MString RemoveTooLarge("abc");
  RemoveTooLarge.Remove(99);
  Passed = Evaluate("Remove(size_t)", "start > length", "Remove(start) is a no-op if start is outside the string", RemoveTooLarge, MString("abc")) && Passed;

  MString RemoveRangeTooLarge("abc");
  RemoveRangeTooLarge.Remove(99, 3);
  Passed = Evaluate("Remove(size_t, size_t)", "start > length", "Remove(start, size) is a no-op if start is outside the string", RemoveRangeTooLarge, MString("abc")) && Passed;

  MString ReplaceTooLarge("abc");
  ReplaceTooLarge.Replace(99, 1, "x");
  Passed = Evaluate("Replace()", "start > length", "Replace is a no-op if start is outside the string", ReplaceTooLarge, MString("abc")) && Passed;

  MString RemoveLastTooLarge("abc");
  RemoveLastTooLarge.RemoveLast(99);
  Passed = Evaluate("RemoveLast()", "size > length", "RemoveLast removes the whole string if the size exceeds the length", RemoveLastTooLarge, MString("")) && Passed;

  MString RemoveLastInPlaceTooLarge("abc");
  RemoveLastInPlaceTooLarge.RemoveLastInPlace(99);
  Passed = Evaluate("RemoveLastInPlace()", "size > length", "RemoveLastInPlace removes the whole string if the size exceeds the length", RemoveLastInPlaceTooLarge, MString("")) && Passed;

  istringstream EmptyStream;
  MString EmptyLine("preset");
  EmptyLine.ReadLine(EmptyStream);
  Passed = Evaluate("ReadLine()", "empty stream", "ReadLine clears the string when no line can be read", EmptyLine, MString("")) && Passed;

  Passed = EvaluateTrue("IsNumber()", "+12", "IsNumber accepts an explicit plus sign", MString("+12").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", ".5", "IsNumber accepts fractional numbers without a leading zero", MString(".5").IsNumber()) && Passed;
  Passed = EvaluateTrue("IsNumber()", "1.", "IsNumber accepts numbers with a trailing decimal point", MString("1.").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "nan", "IsNumber rejects nan text", MString("nan").IsNumber()) && Passed;
  Passed = EvaluateFalse("IsNumber()", "inf", "IsNumber rejects inf text", MString("inf").IsNumber()) && Passed;

  Passed = EvaluateException<MExceptionArbitrary>("ToUnsignedInt()", "invalid input", "ToUnsignedInt throws on non-numeric input", []() { MString("abc").ToUnsignedInt(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedInt()", "overflow", "ToUnsignedInt throws on overflow", []() { MString("999999999999999999999999").ToUnsignedInt(); }) && Passed;
  Passed = EvaluateException<MExceptionArbitrary>("ToUnsignedLong()", "invalid input", "ToUnsignedLong throws on non-numeric input", []() { MString("abc").ToUnsignedLong(); }) && Passed;
  Passed = EvaluateException<MExceptionParameterOutOfRange>("ToUnsignedLong()", "overflow", "ToUnsignedLong throws on overflow", []() { MString("999999999999999999999999").ToUnsignedLong(); }) && Passed;

  MString Long("start");
  for (unsigned int i = 0; i < 100; ++i) {
    Long += "x";
  }
  Passed = Evaluate("operator+=", "long append", "Repeated append operations preserve all characters", static_cast<unsigned int>(Long.Length()), 105U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Main function
int main()
{
  UTString Test;
  return Test.Run() == true ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
