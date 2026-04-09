/*
 * UTTokenizer.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MTokenizer.h"
#include "MUnitTest.h"
#include "MStreams.h"

// Standard lib:
#include <sstream>
using namespace std;


//! Unit test class for the MTokenizer helper
class UTTokenizer : public MUnitTest
{
public:
  //! Default constructor
  UTTokenizer() : MUnitTest("UTTokenizer") {}
  //! Default destructor
  virtual ~UTTokenizer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction, tokenization and simple access
  bool TestConstructionAndTokenization();
  //! Test typed conversion helpers
  bool TestTypedAccessors();
  //! Test vector and array conversion helpers
  bool TestContainerAccessors();
  //! Test maths handling and diagnostics helpers
  bool TestMathsAndDiagnostics();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTTokenizer::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndTokenization() && AllPassed;
  AllPassed = TestTypedAccessors() && AllPassed;
  AllPassed = TestContainerAccessors() && AllPassed;
  AllPassed = TestMathsAndDiagnostics() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test construction, tokenization and simple access
bool UTTokenizer::TestConstructionAndTokenization()
{
  bool Passed = true;

  MTokenizer Default;
  Passed = EvaluateNear("GetNTokens()", "default constructor", "The default tokenizer starts empty", Default.GetNTokens(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("GetText()", "default constructor", "The default tokenizer has empty source text", Default.GetText(), MString("")) && Passed;
  Passed = EvaluateFalse("IsComposited()", "default constructor", "The default tokenizer is not composited", Default.IsComposited()) && Passed;

  MTokenizer Basic(' ', true);
  Passed = EvaluateTrue("Analyze()", "simple tokens", "Analyze tokenizes a space-separated string", Basic.Analyze("alpha beta gamma")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "simple tokens", "Analyze finds all simple tokens", Basic.GetNTokens(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "simple tokens", "The first token is preserved", Basic.GetTokenAt(0), MString("alpha")) && Passed;
  Passed = Evaluate("GetTokenAt()", "simple tokens", "The second token is preserved", Basic.GetTokenAt(1), MString("beta")) && Passed;
  Passed = Evaluate("GetTokenAfterAsString()", "simple tokens", "GetTokenAfterAsString returns all remaining tokens joined by spaces", Basic.GetTokenAfterAsString(1), MString("beta gamma")) && Passed;
  Passed = EvaluateTrue("IsTokenAt()", "case sensitive", "IsTokenAt matches an identical token", Basic.IsTokenAt(1, "beta")) && Passed;
  Passed = EvaluateFalse("IsTokenAt()", "case sensitive", "IsTokenAt respects case by default", Basic.IsTokenAt(1, "Beta")) && Passed;
  Passed = EvaluateTrue("IsTokenAt()", "ignore case", "IsTokenAt can ignore case on request", Basic.IsTokenAt(1, "Beta", true)) && Passed;
  Passed = Evaluate("GetText()", "simple tokens", "GetText returns the original analyzed text", Basic.GetText(), MString("alpha beta gamma")) && Passed;

  MTokenizer Automatic("one two", ' ', true);
  Passed = EvaluateNear("MTokenizer(text,...)", "automatic analysis", "The text constructor analyzes immediately", Automatic.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAtAsString()", "automatic analysis", "The text constructor stores the parsed tokens", Automatic.GetTokenAtAsString(1), MString("two")) && Passed;

  MTokenizer Configurable;
  Configurable.SetSeparator(';');
  Configurable.AllowComposed(false);
  Passed = EvaluateTrue("Analyze()", "custom separator", "SetSeparator changes the token separator", Configurable.Analyze("alpha;beta;gamma")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "custom separator", "A custom separator is honored during tokenization", Configurable.GetNTokens(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "custom separator", "Tokens are split using the configured separator", Configurable.GetTokenAt(1), MString("beta")) && Passed;
  Passed = EvaluateTrue("Analyse()", "alias", "Analyse is an alias for Analyze", Configurable.Analyse("Sphere.Source;value")) && Passed;
  Passed = Evaluate("GetTokenAt()", "alias", "AllowComposed(false) keeps the first token unsplit when using Analyse", Configurable.GetTokenAt(0), MString("Sphere.Source")) && Passed;

  MTokenizer Comment;
  Passed = EvaluateTrue("Analyze()", "hash comment", "Analyze succeeds on lines with comments", Comment.Analyze("token ## comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "hash comment", "A ## token terminates tokenization as a comment", Comment.GetNTokens(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "slash comment", "Analyze succeeds on // comments", Comment.Analyze("token // comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "slash comment", "Slash comments terminate tokenization", Comment.GetNTokens(), 1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "leading hash comment", "A line beginning with # is treated as a comment", Comment.Analyze("# full comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "leading hash comment", "A leading # comment produces no tokens", Comment.GetNTokens(), 0.0, 1e-12) && Passed;
  Passed = EvaluateTrue("Analyze()", "bang comment", "A line beginning with ! is treated as a comment", Comment.Analyze("! full comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "bang comment", "A leading ! comment produces no tokens", Comment.GetNTokens(), 0.0, 1e-12) && Passed;

  MTokenizer Composed;
  Passed = EvaluateTrue("Analyze()", "composed token", "Analyze accepts a composed first token", Composed.Analyze("Sphere.Source value")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "composed token", "A composed first token is split into two tokens", Composed.GetNTokens(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "composed token", "The part before the dot becomes the first token", Composed.GetTokenAt(0), MString("Sphere")) && Passed;
  Passed = Evaluate("GetTokenAt()", "composed token", "The part after the dot becomes the second token", Composed.GetTokenAt(1), MString("Source")) && Passed;
  Passed = EvaluateTrue("IsComposited()", "composed token", "IsComposited reports a composed first token", Composed.IsComposited()) && Passed;

  MTokenizer NotComposed(' ', false);
  Passed = EvaluateTrue("Analyze()", "disallow composed", "Analyze succeeds when composed tokens are disabled", NotComposed.Analyze("Sphere.Source value")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "disallow composed", "Disabling composed tokens preserves the original first token", NotComposed.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "disallow composed", "The first token remains unsplit when composed tokens are disabled", NotComposed.GetTokenAt(0), MString("Sphere.Source")) && Passed;

  MTokenizer Fast(' ', true);
  Passed = EvaluateTrue("AnalyzeFast()", "basic", "AnalyzeFast tokenizes simple text", Fast.AnalyzeFast("1 2 3")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "basic fast", "AnalyzeFast finds the expected number of tokens", Fast.GetNTokens(), 3.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "basic fast", "AnalyzeFast preserves token order", Fast.GetTokenAt(2), MString("3")) && Passed;
  Passed = EvaluateTrue("AnalyzeFast()", "overwrite", "AnalyzeFast can be reused on the same tokenizer", Fast.AnalyzeFast("7 8")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "overwrite", "AnalyzeFast replaces the previous token list", Fast.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "overwrite", "AnalyzeFast stores the new token list", Fast.GetTokenAt(0), MString("7")) && Passed;
  Passed = EvaluateTrue("AnalyzeFast()", "whitespace and comment", "AnalyzeFast handles tabs and stops at comments", Fast.AnalyzeFast("\t9\t10 // comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "whitespace and comment", "AnalyzeFast ignores trailing comments", Fast.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "whitespace and comment", "AnalyzeFast keeps tokens before the comment", Fast.GetTokenAt(1), MString("10")) && Passed;
  Passed = EvaluateTrue("AnalyzeFast()", "leading hash comment", "AnalyzeFast treats # as a comment start", Fast.AnalyzeFast("# comment")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "leading hash comment", "AnalyzeFast produces no tokens for a leading hash comment", Fast.GetNTokens(), 0.0, 1e-12) && Passed;

  MTokenizer Brackets;
  Passed = EvaluateTrue("Analyze()", "bracket token", "Text enclosed in brackets is kept as one token", Brackets.Analyze("[alpha beta] gamma")) && Passed;
  Passed = EvaluateNear("GetNTokens()", "bracket token", "Bracket-enclosed text stays combined", Brackets.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "bracket token", "The bracket token is preserved verbatim", Brackets.GetTokenAt(0), MString("[alpha beta]")) && Passed;
  Passed = EvaluateTrue("Analyze()", "nested braces in token", "Analyze keeps nested math expressions together as one token", Brackets.Analyze("{sin({1+2})} gamma", false)) && Passed;
  Passed = EvaluateNear("GetNTokens()", "nested braces in token", "Nested math braces stay combined when math evaluation is disabled", Brackets.GetNTokens(), 2.0, 1e-12) && Passed;
  Passed = Evaluate("GetTokenAt()", "nested braces in token", "The nested math token is preserved verbatim", Brackets.GetTokenAt(0), MString("{sin({1+2})}")) && Passed;

  mout.Enable(false);
  Passed = Evaluate("GetTokenAt()", "out of bounds", "Out-of-bounds string access returns an empty string", Basic.GetTokenAt(99), MString("")) && Passed;
  Passed = Evaluate("GetTokenAtAsString()", "out of bounds", "Out-of-bounds string conversion returns an empty string", Basic.GetTokenAtAsString(99), MString("")) && Passed;
  Passed = Evaluate("GetTokenAfterAsString()", "out of bounds", "Out-of-bounds trailing-string access returns an empty string", Basic.GetTokenAfterAsString(99), MString("")) && Passed;
  mout.Enable(true);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test typed conversion helpers
bool UTTokenizer::TestTypedAccessors()
{
  bool Passed = true;

  MTokenizer Types(' ', false);
  Passed = EvaluateTrue("Analyze()", "typed tokens", "Analyze succeeds on numeric and boolean tokens", Types.Analyze("3.5 7 42 123456789 255 true false")) && Passed;

  Passed = EvaluateNear("GetTokenAtAsDouble()", "double", "GetTokenAtAsDouble parses floating-point tokens", Types.GetTokenAtAsDouble(0), 3.5, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsFloat()", "float", "GetTokenAtAsFloat parses floating-point tokens", Types.GetTokenAtAsFloat(0), 3.5, 1e-6) && Passed;
  Passed = EvaluateNear("GetTokenAtAsInt()", "int", "GetTokenAtAsInt parses integer tokens", Types.GetTokenAtAsInt(1), 7.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedInt()", "unsigned int", "GetTokenAtAsUnsignedInt parses non-negative tokens", Types.GetTokenAtAsUnsignedInt(2), 42.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedIntFast()", "unsigned int fast", "GetTokenAtAsUnsignedIntFast parses digit-only tokens", Types.GetTokenAtAsUnsignedIntFast(4), 255.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsLong()", "long", "GetTokenAtAsLong parses long tokens", Types.GetTokenAtAsLong(3), 123456789.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedLong()", "unsigned long", "GetTokenAtAsUnsignedLong parses unsigned long tokens", Types.GetTokenAtAsUnsignedLong(4), 255.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetTokenAtAsBoolean()", "true token", "GetTokenAtAsBoolean recognizes true", Types.GetTokenAtAsBoolean(5)) && Passed;
  Passed = EvaluateFalse("GetTokenAtAsBoolean()", "false token", "GetTokenAtAsBoolean recognizes false", Types.GetTokenAtAsBoolean(6)) && Passed;

  MTokenizer BooleanNumbers;
  BooleanNumbers.Analyze("0 1");
  Passed = EvaluateFalse("GetTokenAtAsBoolean()", "numeric false", "GetTokenAtAsBoolean maps zero to false", BooleanNumbers.GetTokenAtAsBoolean(0)) && Passed;
  Passed = EvaluateTrue("GetTokenAtAsBoolean()", "numeric true", "GetTokenAtAsBoolean maps nonzero values to true", BooleanNumbers.GetTokenAtAsBoolean(1)) && Passed;

  MTokenizer TimeTokenizer(' ', false);
  TimeTokenizer.Analyze("TI 12.25");
  MTime Time = TimeTokenizer.GetTokenAtAsTime(1);
  Passed = EvaluateTrue("GetTokenAtAsTime()", "time token", "GetTokenAtAsTime parses TI-style time tokens", Time.GetLongIntsString() == "12.250000000") && Passed;

  MTokenizer NegativeUnsigned(' ', false);
  NegativeUnsigned.Analyze("-1");
  mout.Enable(false);
  Passed = EvaluateNear("GetTokenAtAsUnsignedInt()", "negative token", "Negative unsigned conversion returns zero", NegativeUnsigned.GetTokenAtAsUnsignedInt(0), 0.0, 1e-12) && Passed;
  mout.Enable(true);

  mout.Enable(false);
  Passed = EvaluateNear("GetTokenAtAsInt()", "out of bounds", "Out-of-bounds numeric conversion returns zero", Types.GetTokenAtAsInt(99), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedIntFast()", "out of bounds", "Out-of-bounds fast unsigned conversion returns zero", Types.GetTokenAtAsUnsignedIntFast(99), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedLong()", "out of bounds", "Out-of-bounds unsigned long conversion returns zero", Types.GetTokenAtAsUnsignedLong(99), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("GetTokenAtAsBoolean()", "out of bounds", "Out-of-bounds boolean conversion returns false", Types.GetTokenAtAsBoolean(99)) && Passed;
  mout.Enable(true);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test vector and array conversion helpers
bool UTTokenizer::TestContainerAccessors()
{
  bool Passed = true;

  MTokenizer Numbers;
  Numbers.Analyze("1 2 3 4");

  TArrayI IntArray = Numbers.GetTokenAtAsIntArray(1);
  Passed = EvaluateNear("GetTokenAtAsIntArray()", "array size", "GetTokenAtAsIntArray returns all tokens from the requested index onward", IntArray.GetSize(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsIntArray()", "array content", "GetTokenAtAsIntArray preserves numeric order", IntArray[0], 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsIntArray()", "array content", "GetTokenAtAsIntArray preserves numeric order", IntArray[2], 4.0, 1e-12) && Passed;

  TArrayD DoubleArray = Numbers.GetTokenAtAsDoubleArray(2);
  Passed = EvaluateNear("GetTokenAtAsDoubleArray()", "array size", "GetTokenAtAsDoubleArray returns the expected size", DoubleArray.GetSize(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTokenAtAsDoubleArray()", "array content", "GetTokenAtAsDoubleArray preserves numeric order", DoubleArray[0], 3.0, 1e-12) && Passed;

  vector<double> DoubleVector = Numbers.GetTokenAtAsDoubleVector(1);
  Passed = EvaluateSize("GetTokenAtAsDoubleVector()", "vector size", "GetTokenAtAsDoubleVector returns the expected number of elements", DoubleVector.size(), 3) && Passed;
  Passed = EvaluateNear("GetTokenAtAsDoubleVector()", "vector content", "GetTokenAtAsDoubleVector preserves numeric order", DoubleVector[1], 3.0, 1e-12) && Passed;

  vector<float> FloatVector = Numbers.GetTokenAtAsFloatVector(2);
  Passed = EvaluateSize("GetTokenAtAsFloatVector()", "vector size", "GetTokenAtAsFloatVector returns the expected number of elements", FloatVector.size(), 2) && Passed;
  Passed = EvaluateNear("GetTokenAtAsFloatVector()", "vector content", "GetTokenAtAsFloatVector preserves numeric order", FloatVector[0], 3.0, 1e-6) && Passed;

  vector<int> IntVector = Numbers.GetTokenAtAsIntVector(0);
  Passed = EvaluateSize("GetTokenAtAsIntVector()", "vector size", "GetTokenAtAsIntVector returns the expected number of elements", IntVector.size(), 4) && Passed;
  Passed = EvaluateNear("GetTokenAtAsIntVector()", "vector content", "GetTokenAtAsIntVector preserves numeric order", IntVector[3], 4.0, 1e-12) && Passed;

  vector<unsigned int> UnsignedVector = Numbers.GetTokenAtAsUnsignedIntVector(1);
  Passed = EvaluateSize("GetTokenAtAsUnsignedIntVector()", "vector size", "GetTokenAtAsUnsignedIntVector returns the expected number of elements", UnsignedVector.size(), 3) && Passed;
  Passed = EvaluateNear("GetTokenAtAsUnsignedIntVector()", "vector content", "GetTokenAtAsUnsignedIntVector preserves numeric order", UnsignedVector[0], 2.0, 1e-12) && Passed;

  MTokenizer Strings;
  Strings.Analyze("alpha beta gamma");
  vector<MString> PlainStrings = Strings.GetTokenAtAsStringVector(1, false);
  Passed = EvaluateSize("GetTokenAtAsStringVector()", "plain strings", "GetTokenAtAsStringVector returns all plain string tokens", PlainStrings.size(), 2) && Passed;
  Passed = Evaluate("GetTokenAtAsStringVector()", "plain strings", "GetTokenAtAsStringVector preserves plain string content", PlainStrings[0], MString("beta")) && Passed;

  MTokenizer Quoted;
  Quoted.Analyze("\"hello world\" \"MEGAlib test\"");
  vector<MString> QuotedStrings = Quoted.GetTokenAtAsStringVector(0, true);
  Passed = EvaluateSize("GetTokenAtAsStringVector()", "quoted strings", "Quoted strings are reconstructed across token boundaries", QuotedStrings.size(), 2) && Passed;
  Passed = Evaluate("GetTokenAtAsStringVector()", "quoted strings", "Quoted strings preserve their inner spaces", QuotedStrings[0], MString("hello world")) && Passed;
  Passed = Evaluate("GetTokenAtAsStringVector()", "quoted strings", "Quoted strings preserve their inner spaces", QuotedStrings[1], MString("MEGAlib test")) && Passed;

  MTokenizer BrokenQuotes;
  BrokenQuotes.Analyze("hello \"broken string");
  mout.Enable(false);
  vector<MString> InvalidQuoted = BrokenQuotes.GetTokenAtAsStringVector(1, true);
  mout.Enable(true);
  Passed = EvaluateSize("GetTokenAtAsStringVector()", "broken quotes", "Malformed quoted strings return an empty vector", InvalidQuoted.size(), 0) && Passed;

  mout.Enable(false);
  vector<int> InvalidIntVector = Numbers.GetTokenAtAsIntVector(99);
  vector<MString> InvalidStringVector = Strings.GetTokenAtAsStringVector(99, false);
  mout.Enable(true);
  Passed = EvaluateSize("GetTokenAtAsIntVector()", "out of bounds", "Out-of-bounds integer vector conversion returns an empty vector", InvalidIntVector.size(), 0) && Passed;
  Passed = EvaluateSize("GetTokenAtAsStringVector()", "out of bounds", "Out-of-bounds string vector conversion returns an empty vector", InvalidStringVector.size(), 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test maths handling and diagnostics helpers
bool UTTokenizer::TestMathsAndDiagnostics()
{
  bool Passed = true;

  Passed = EvaluateTrue("IsMaths()", "math token", "IsMaths recognizes braces as a math environment", MTokenizer::IsMaths("{1+2}")) && Passed;
  Passed = EvaluateFalse("IsMaths()", "plain token", "IsMaths ignores non-math tokens", MTokenizer::IsMaths("1+2")) && Passed;

  Passed = EvaluateTrue("CheckMaths()", "simple math", "CheckMaths accepts recognized math expressions", MTokenizer::CheckMaths("{sin(pi/2)+2}")) && Passed;
  Passed = EvaluateTrue("CheckMaths()", "sqrt", "CheckMaths accepts sqrt expressions", MTokenizer::CheckMaths("{sqrt(9)}")) && Passed;
  Passed = EvaluateTrue("CheckMaths()", "atan", "CheckMaths accepts atan expressions", MTokenizer::CheckMaths("{atan(1)}")) && Passed;
  Passed = EvaluateTrue("CheckMaths()", "ln", "CheckMaths accepts ln expressions", MTokenizer::CheckMaths("{ln(e)}")) && Passed;
  Passed = EvaluateTrue("CheckMaths()", "mixed functions", "CheckMaths accepts mixed supported functions", MTokenizer::CheckMaths("{ceil(atan(1)+sqrt(9)+exp(0))}")) && Passed;
  mout.Enable(false);
  Passed = EvaluateFalse("CheckMaths()", "unknown symbol", "CheckMaths rejects unknown math identifiers", MTokenizer::CheckMaths("{unknownsymbol}")) && Passed;
  mout.Enable(true);

  MString MathToken = "{1+2}";
  Passed = EvaluateTrue("EvaluateMaths()", "simple math", "EvaluateMaths succeeds on valid expressions", MTokenizer::EvaluateMaths(MathToken)) && Passed;
  Passed = EvaluateTrue("EvaluateMaths()", "simple math result", "EvaluateMaths replaces the token with the numeric result", MathToken.BeginsWith("3.0000000000000000e+00")) && Passed;

  MString SqrtToken = "{sqrt(9)}";
  Passed = EvaluateTrue("EvaluateMaths()", "sqrt", "EvaluateMaths succeeds on sqrt expressions", MTokenizer::EvaluateMaths(SqrtToken)) && Passed;
  Passed = EvaluateTrue("EvaluateMaths()", "sqrt result", "EvaluateMaths returns the expected sqrt result", SqrtToken.BeginsWith("3.0000000000000000e+00")) && Passed;

  MString AtanToken = "{atan(1)}";
  Passed = EvaluateTrue("EvaluateMaths()", "atan", "EvaluateMaths succeeds on atan expressions", MTokenizer::EvaluateMaths(AtanToken)) && Passed;
  Passed = EvaluateTrue("EvaluateMaths()", "atan result", "EvaluateMaths returns the expected atan result", AtanToken.BeginsWith("7.8539816339744828e-01")) && Passed;

  MString MixedMathToken = "{ceil(atan(1)+sqrt(9)+exp(0))}";
  Passed = EvaluateTrue("EvaluateMaths()", "mixed functions", "EvaluateMaths succeeds on mixed supported functions", MTokenizer::EvaluateMaths(MixedMathToken)) && Passed;
  Passed = EvaluateTrue("EvaluateMaths()", "mixed function result", "EvaluateMaths returns the expected mixed-function result", MixedMathToken.BeginsWith("5.0000000000000000e+00")) && Passed;

  MString NotMath = "1+2";
  Passed = EvaluateFalse("EvaluateMaths()", "plain token", "EvaluateMaths returns false for non-math tokens", MTokenizer::EvaluateMaths(NotMath)) && Passed;

  MTokenizer MathTokenizer;
  Passed = EvaluateTrue("Analyze()", "math token", "Analyze evaluates braced math expressions when AllowMaths is true", MathTokenizer.Analyze("alpha {1+2}")) && Passed;
  Passed = Evaluate("GetTokenAt()", "math token", "Analyze replaces math expressions by their evaluated value", MathTokenizer.GetTokenAt(1), MString("3.0000000000000000e+00")) && Passed;
  Passed = EvaluateTrue("CheckAllMaths()", "evaluated token list", "CheckAllMaths succeeds for a token list without invalid math", MathTokenizer.CheckAllMaths()) && Passed;

  MTokenizer InvalidMathList;
  InvalidMathList.Analyze("alpha {unknownsymbol}", false);
  mout.Enable(false);
  Passed = EvaluateFalse("CheckAllMaths()", "invalid token list", "CheckAllMaths fails when any token contains invalid math", InvalidMathList.CheckAllMaths()) && Passed;
  mout.Enable(true);

  MTokenizer NoMath;
  Passed = EvaluateTrue("Analyze(..., false)", "preserve math token", "Analyze can preserve math expressions when AllowMaths is false", NoMath.Analyze("alpha {1+2}", false)) && Passed;
  Passed = Evaluate("GetTokenAt()", "preserve math token", "AllowMaths = false keeps the original math token", NoMath.GetTokenAt(1), MString("{1+2}")) && Passed;

  mout.Enable(false);
  Passed = EvaluateFalse("Analyze()", "unbalanced math", "Analyze rejects mismatched math braces", NoMath.Analyze("alpha {1+2")) && Passed;
  mout.Enable(true);

  MTokenizer Description;
  Description.Analyze("one two");
  Passed = Evaluate("ToCompactString()", "compact string", "ToCompactString joins tokens with separators", Description.ToCompactString(), MString("one | two")) && Passed;
  Passed = EvaluateTrue("ToString()", "diagnostics", "ToString lists the token content", Description.ToString().Contains("Tokenizer content (2 Tokens):")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTokenizer Test;
  return Test.Run() == true ? 0 : 1;
}
