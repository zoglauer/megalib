/*
 * UTExceptions.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <fstream>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MExceptions.h"
#include "MUnitTest.h"

//! Unit test class for the exception hierarchy
class UTExceptions : public MUnitTest
{
public:
  //! Default constructor
  UTExceptions() : MUnitTest("UTExceptions") {}
  //! Default destructor
  virtual ~UTExceptions() {}

  //! Run all tests
  virtual bool Run();
  //! Return the binary path used for child-process checks
  static MString& BinaryPath()
  {
    static MString Path = "";
    return Path;
  }
  //! Helper entry point for the abort-check child process
  static int AbortCheck()
  {
    MException::UseAbort(true);
    new MExceptionUnknownMode("abort-check");
    return 1;
  }
private:
  //! Test the base class and polymorphic throwing behavior
  bool TestBaseBehavior();
  //! Test all standard exception message constructors
  bool TestSpecificMessages();
  //! Test edge cases and mutable setters
  bool TestEdgeCases();
  //! Test the exception usage patterns found in MEGAlib
  bool TestUsagePatterns();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTExceptions::Run()
{
  bool Passed = true;

  Passed = TestBaseBehavior() && Passed;
  Passed = TestSpecificMessages() && Passed;
  Passed = TestEdgeCases() && Passed;
  Passed = TestUsagePatterns() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the base class and polymorphic throwing behavior
bool UTExceptions::TestBaseBehavior()
{
  bool Passed = true;

  MException::UseAbort(false);

  MException Base;
  Passed = Evaluate("MException()", "default description", "The default base exception text is available via what()", MString(Base.what()), MString("No description of the exception available.")) && Passed;

  try {
    throw MExceptionDivisionByZero();
  } catch (const MException& Exception) {
    Passed = Evaluate("throw MExceptionDivisionByZero()", "catch base", "Derived exceptions can be caught polymorphically as MException", MString(Exception.what()), MString("Division by zero!")) && Passed;
  } catch (...) {
    Passed = EvaluateTrue("throw MExceptionDivisionByZero()", "catch base", "Derived exceptions can be caught polymorphically as MException", false) && Passed;
  }

  try {
    throw MExceptionArbitrary("Polymorphic arbitrary");
  } catch (const exception& Exception) {
    Passed = EvaluateTrue("throw MExceptionArbitrary()", "catch std::exception", "The exception hierarchy derives from std::exception", MString(Exception.what()).Contains("Polymorphic arbitrary")) && Passed;
  } catch (...) {
    Passed = EvaluateTrue("throw MExceptionArbitrary()", "catch std::exception", "The exception hierarchy derives from std::exception", false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test all standard exception message constructors
bool UTExceptions::TestSpecificMessages()
{
  bool Passed = true;

  {
    MExceptionTestFailed Exception("Axis size", 2.0, "!=", 3.0);
    Passed = Evaluate("MExceptionTestFailed(...)", "message", "The test-failed exception includes the description and comparison", MString(Exception.what()), MString("Test failed: Axis size: 2 != 3\n")) && Passed;
  }

  {
    MExceptionParameterOutOfRange Exception(5.0, 1.0, 4.0, "Energy");
    Passed = Evaluate("MExceptionParameterOutOfRange(...)", "message", "The parameter-out-of-range exception lists allowed interval and value", MString(Exception.what()), MString("Parameter \"Energy\" out of range - allowed: [1..4] - you have: 5\n")) && Passed;
  }

  {
    MExceptionIndexOutOfBounds Exception(0, 7, 8);
    Passed = Evaluate("MExceptionIndexOutOfBounds(...)", "message", "The index-out-of-bounds exception lists allowed range and invalid index", MString(Exception.what()), MString("Index out of bounds - allowed: [0..7[ - you are trying to access with index: 8\n")) && Passed;
  }

  {
    double Value = 1.5;
    MExceptionValueNotFound Exception(Value, "vector");
    Passed = Evaluate("MExceptionValueNotFound(double)", "message", "The value-not-found exception formats double values", MString(Exception.what()), MString("The value '1.5' cannot be found in vector.\n")) && Passed;
  }

  {
    int Value = -7;
    MExceptionValueNotFound Exception(Value, "array");
    Passed = Evaluate("MExceptionValueNotFound(int)", "message", "The value-not-found exception formats signed integers", MString(Exception.what()), MString("The value '-7' cannot be found in array.\n")) && Passed;
  }

  {
    unsigned int Value = 9;
    MExceptionValueNotFound Exception(Value, "map");
    Passed = Evaluate("MExceptionValueNotFound(unsigned int)", "message", "The value-not-found exception formats unsigned integers", MString(Exception.what()), MString("The value '9' cannot be found in map.\n")) && Passed;
  }

  {
    MString Value = "alpha";
    MExceptionValueNotFound Exception(Value, "store");
    Passed = Evaluate("MExceptionValueNotFound(MString)", "message", "The value-not-found exception formats string values", MString(Exception.what()), MString("The value 'alpha' cannot be found in store.\n")) && Passed;
  }

  {
    MExceptionEmptyArray Exception("m_Times");
    Passed = Evaluate("MExceptionEmptyArray(...)", "message", "The empty-array exception lists the array name", MString(Exception.what()), MString("The array \"m_Times\" has no elements.\n")) && Passed;
  }

  {
    MExceptionEmptyObject Exception("function values");
    Passed = Evaluate("MExceptionEmptyObject(...)", "message", "The empty-object exception lists the empty object name", MString(Exception.what()), MString("The object \"function values\" is empty.\n")) && Passed;
  }

  {
    MExceptionValueOutOfBounds Exception(1.0, 2.0, 3.0);
    Passed = Evaluate("MExceptionValueOutOfBounds(min,max,value)", "message", "The value-out-of-bounds exception lists allowed interval and value", MString(Exception.what()), MString("Value out of bounds - allowed: [1..2] - your's: 3\n")) && Passed;
  }

  {
    MExceptionDivisionByZero Exception;
    Passed = Evaluate("MExceptionDivisionByZero()", "message", "The division-by-zero exception has the expected default text", MString(Exception.what()), MString("Division by zero!")) && Passed;
  }

  {
    MExceptionNumberNotFinite Exception;
    Passed = Evaluate("MExceptionNumberNotFinite()", "message", "The non-finite-number exception has the expected default text", MString(Exception.what()), MString("Number not finite!")) && Passed;
  }

  {
    MExceptionObjectDoesNotExist Exception("MyKey");
    Passed = Evaluate("MExceptionObjectDoesNotExist(...)", "message", "The missing-object exception lists the missing key", MString(Exception.what()), MString("Object/Key MyKey not found in the list/vector/array/etc. !\n")) && Passed;
  }

  {
    MExceptionObjectsNotIdentical Exception("A", "B");
    Passed = Evaluate("MExceptionObjectsNotIdentical(...)", "message", "The non-identical-objects exception lists both object names", MString(Exception.what()), MString("The object A and B are not identical!\n")) && Passed;
  }

  {
    MExceptionUnknownMode Exception("Read");
    Passed = Evaluate("MExceptionUnknownMode(mode)", "message", "The unknown-mode exception includes the mode name", MString(Exception.what()), MString("Unknown mode Read!\n")) && Passed;
  }

  {
    MExceptionInvalidState Exception("This function contains no positive values");
    Passed = Evaluate("MExceptionInvalidState(...)", "message", "The invalid-state exception includes the supplied description", MString(Exception.what()), MString("This function contains no positive values\n")) && Passed;
  }

  {
    MExceptionUnknownMode Exception("calibration", 3);
    Passed = Evaluate("MExceptionUnknownMode(type,int)", "message", "The typed unknown-mode exception includes the type and value", MString(Exception.what()), MString("Unknown calibration mode 3!\n")) && Passed;
  }

  {
    MExceptionNeverReachThatLineOfCode Exception;
    Passed = Evaluate("MExceptionNeverReachThatLineOfCode()", "message", "The never-reach exception has the expected default text", MString(Exception.what()), MString("We should have never reached that line of code!")) && Passed;
  }

  {
    MExceptionArbitrary Exception("Any detail");
    Passed = Evaluate("MExceptionArbitrary(...)", "message", "The arbitrary exception includes the provided description", MString(Exception.what()), MString("An exception was triggered: \nAny detail\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test edge cases and mutable setters
bool UTExceptions::TestEdgeCases()
{
  bool Passed = true;

  {
    MExceptionParameterOutOfRange Exception;
    Passed = Evaluate("MExceptionParameterOutOfRange()", "default", "The default parameter-out-of-range text is available", MString(Exception.what()), MString("Index out of bounds!")) && Passed;
    Exception.SetMinSizeIndex(-1.0, -5.0, 5.0, "Offset");
    Passed = Evaluate("SetMinSizeIndex()", "updated", "SetMinSizeIndex updates the parameter-out-of-range description", MString(Exception.what()), MString("Parameter \"Offset\" out of range - allowed: [-5..5] - you have: -1\n")) && Passed;
  }

  {
    MExceptionIndexOutOfBounds Exception;
    Passed = Evaluate("MExceptionIndexOutOfBounds()", "default", "The default index-out-of-bounds text is available", MString(Exception.what()), MString("Index out of bounds!")) && Passed;
    Exception.SetMinSizeIndex(3, 9, 11);
    Passed = Evaluate("SetMinSizeIndex()", "updated", "SetMinSizeIndex updates the index-out-of-bounds description", MString(Exception.what()), MString("Index out of bounds - allowed: [3..9[ - you are trying to access with index: 11\n")) && Passed;
  }

  {
    MExceptionValueOutOfBounds Exception(3.5);
    Passed = EvaluateTrue("MExceptionValueOutOfBounds(value)", "single value", "The single-value constructor includes the offending value in the message", MString(Exception.what()).Contains("3.5")) && Passed;
    Passed = Evaluate("MExceptionValueOutOfBounds(value)", "single value exact", "The single-value constructor formats the full deterministic message", MString(Exception.what()), MString("Value out of bounds: 3.5")) && Passed;
  }

  {
    MExceptionObjectDoesNotExist Exception;
    Passed = Evaluate("MExceptionObjectDoesNotExist()", "default", "The default missing-object text is available", MString(Exception.what()), MString("Object/Key not found in the list/vector/array/etc. !")) && Passed;
    Exception.SetName("SecondaryKey");
    Passed = Evaluate("SetName()", "updated", "SetName updates the missing-object description", MString(Exception.what()), MString("Object/Key SecondaryKey not found in the list/vector/array/etc. !\n")) && Passed;
  }

  {
    MExceptionObjectsNotIdentical Exception("", "");
    Passed = Evaluate("MExceptionObjectsNotIdentical(empty)", "default fallback", "Empty object names fall back to the generic message", MString(Exception.what()), MString("Objects not identical!")) && Passed;
  }

  {
    MExceptionEmptyObject Exception;
    Passed = Evaluate("MExceptionEmptyObject()", "default", "The default empty-object text is available", MString(Exception.what()), MString("The object is empty!")) && Passed;
  }

  {
    MExceptionInvalidState Exception;
    Passed = Evaluate("MExceptionInvalidState()", "default", "The default invalid-state text is available", MString(Exception.what()), MString("Invalid object state!")) && Passed;
  }

  {
    MExceptionNeverReachThatLineOfCode Exception("Extra context");
    Passed = EvaluateTrue("MExceptionNeverReachThatLineOfCode(description)", "custom detail", "A custom never-reach exception includes the supplied description", MString(Exception.what()).Contains("Extra context")) && Passed;
    Passed = Evaluate("MExceptionNeverReachThatLineOfCode(description)", "custom detail exact", "A custom never-reach exception formats the full deterministic message", MString(Exception.what()), MString("We should have never reached that line of code: \nExtra context\n")) && Passed;
  }

  {
    MExceptionArbitrary Exception;
    Passed = Evaluate("MExceptionArbitrary()", "default", "The default arbitrary exception text is available", MString(Exception.what()), MString("An exception was triggered!")) && Passed;
    Exception.SetDescription("Specific reason");
    Passed = Evaluate("SetDescription()", "updated", "SetDescription updates the arbitrary exception description", MString(Exception.what()), MString("An exception was triggered: \nSpecific reason\n")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test the exception usage patterns found in MEGAlib
bool UTExceptions::TestUsagePatterns()
{
  bool Passed = true;

  {
    MExceptionIndexOutOfBounds Exception;
    Passed = Evaluate("MExceptionIndexOutOfBounds()", "MCOrientation usage", "The default index-out-of-bounds constructor used in MCOrientation has the expected text", MString(Exception.what()), MString("Index out of bounds!")) && Passed;
  }

  {
    MExceptionNeverReachThatLineOfCode Exception("Dimension of the axis not handled!");
    Passed = Evaluate("MExceptionNeverReachThatLineOfCode(...)", "MResponseMatrixON usage", "The never-reach exception keeps the explicit diagnostic used in MResponseMatrixON", MString(Exception.what()), MString("We should have never reached that line of code: \nDimension of the axis not handled!\n")) && Passed;
  }

  {
    MExceptionArbitrary Exception("The size of the input vector is not the same as the number of matrix axes!");
    Passed = Evaluate("MExceptionArbitrary(...)", "MResponseMatrixON usage", "The arbitrary exception keeps the caller-supplied message used in matrix code", MString(Exception.what()), MString("An exception was triggered: \nThe size of the input vector is not the same as the number of matrix axes!\n")) && Passed;
  }

  {
    MExceptionObjectsNotIdentical Exception("Response Matrix A", "Response matrix B");
    Passed = Evaluate("MExceptionObjectsNotIdentical(...)", "MResponseMatrixON usage", "The objects-not-identical exception matches the matrix comparison usage", MString(Exception.what()), MString("The object Response Matrix A and Response matrix B are not identical!\n")) && Passed;
  }

  {
    MExceptionUnknownMode Exception("calibration model determination method", 7);
    Passed = Evaluate("MExceptionUnknownMode(...)", "fretalon usage", "The typed unknown-mode exception matches the fretalon usage pattern", MString(Exception.what()), MString("Unknown calibration model determination method mode 7!\n")) && Passed;
  }

  {
    MExceptionEmptyObject Exception("function x values");
    Passed = Evaluate("MExceptionEmptyObject(...)", "MFunction usage", "The empty-object exception matches the new MFunction empty-state usage pattern", MString(Exception.what()), MString("The object \"function x values\" is empty.\n")) && Passed;
  }

  {
    MExceptionInvalidState Exception("This function contains no positive values");
    Passed = Evaluate("MExceptionInvalidState(...)", "MFunction3DSpherical usage", "The invalid-state exception matches the new spherical random-generation usage pattern", MString(Exception.what()), MString("This function contains no positive values\n")) && Passed;
  }

  {
    MException::UseAbort(false);

    int Status = RunChildProcess(BinaryPath(), "--abort-check", "/tmp/UTExceptions_abort_check.log");

    Passed = EvaluateTrue("MException::UseAbort(true)", "child status", "The abort mode terminates the child process with a non-zero status", Status != 0 && Status != -1) && Passed;

    ifstream In("/tmp/UTExceptions_abort_check.log");
    ostringstream Output;
    Output<<In.rdbuf();
    MString Content = Output.str().c_str();
    Passed = EvaluateTrue("MException::UseAbort(true)", "child message", "Abort mode prints the exception message before terminating", Content.Contains("Unknown mode abort-check!")) && Passed;

    remove("/tmp/UTExceptions_abort_check.log");
    MException::UseAbort(false);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (argc > 1) {
    MString Argument = argv[1];
    if (Argument == "--abort-check") {
      return UTExceptions::AbortCheck();
    }
  }

  UTExceptions::BinaryPath() = argv[0];
  UTExceptions Test;
  return Test.Run() == true ? 0 : 1;
}
