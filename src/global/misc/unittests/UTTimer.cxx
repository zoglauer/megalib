/*
 * UTTimer.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MTimer.h"
#include "MTime.h"
#include "MUnitTest.h"


//! Unit test class for the MTimer helper
class UTTimer : public MUnitTest
{
public:
  //! Default constructor
  UTTimer() : MUnitTest("UTTimer") {}
  //! Default destructor
  virtual ~UTTimer() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction, assignment and reset helpers
  bool TestConstructionAndReset();
  //! Test pause and continue behavior
  bool TestPauseAndContinue();
  //! Test timeout behavior
  bool TestTimeouts();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTTimer::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndReset() && AllPassed;
  AllPassed = TestPauseAndContinue() && AllPassed;
  AllPassed = TestTimeouts() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test construction, assignment and reset helpers
bool UTTimer::TestConstructionAndReset()
{
  bool Passed = true;

  MTimer Running;
  MTime::BusyWait(2000);
  Passed = EvaluateTrue("MTimer(bool)", "default running", "The default constructor starts the timer immediately", Running.GetElapsed() > 0.0) && Passed;

  MTimer Paused(false);
  MTime::BusyWait(2000);
  Passed = EvaluateNear("MTimer(false)", "paused constructor", "Constructing with Start = false leaves the timer paused at zero", Paused.GetElapsed(), 0.0, 1e-6) && Passed;

  MTimer WithTimeout(0.25);
  Passed = EvaluateNear("MTimer(double)", "timeout constructor", "The timeout constructor stores the requested timeout", WithTimeout.GetTimeOut(), 0.25, 1e-12) && Passed;

  MTimer Copy(WithTimeout);
  Passed = EvaluateNear("MTimer(const MTimer&)", "copy timeout", "The copy constructor preserves the timeout value", Copy.GetTimeOut(), 0.25, 1e-12) && Passed;

  MTimer Assigned(false);
  Assigned = WithTimeout;
  Passed = EvaluateNear("operator=", "assignment timeout", "Assignment preserves the timeout value", Assigned.GetTimeOut(), 0.25, 1e-12) && Passed;

  Paused.Start();
  MTime::BusyWait(2000);
  Passed = EvaluateTrue("Start()", "start after paused", "Start begins timing from zero on a paused timer", Paused.GetElapsed() > 0.0) && Passed;

  double BeforeReset = Paused.GetElapsed();
  Paused.Reset();
  double AfterReset = Paused.GetElapsed();
  Passed = EvaluateTrue("Reset()", "restart", "Reset restarts timing from approximately zero", AfterReset <= BeforeReset) && Passed;

  Paused.Clear();
  Passed = EvaluateNear("Clear()", "elapsed", "Clear resets the elapsed time to zero", Paused.GetElapsed(), 0.0, 1e-6) && Passed;
  Passed = EvaluateNear("Clear()", "timeout", "Clear resets the timeout to zero", Paused.GetTimeOut(), 0.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test pause and continue behavior
bool UTTimer::TestPauseAndContinue()
{
  bool Passed = true;

  MTimer Timer;
  MTime::BusyWait(3000);
  Timer.Pause();
  double PausedValue = Timer.GetElapsed();
  MTime::BusyWait(3000);
  Passed = EvaluateNear("Pause()", "frozen elapsed time", "Pause freezes the elapsed time while the timer is paused", Timer.GetElapsed(), PausedValue, 1e-4) && Passed;

  Timer.Continue();
  MTime::BusyWait(3000);
  Passed = EvaluateTrue("Continue()", "resume", "Continue resumes timing after a pause", Timer.GetElapsed() > PausedValue) && Passed;

  double BeforeSecondPause = Timer.GetElapsed();
  Timer.Pause();
  Timer.Pause();
  MTime::BusyWait(2000);
  Passed = EvaluateNear("Pause()", "idempotent", "Calling Pause twice leaves the elapsed time unchanged", Timer.GetElapsed(), BeforeSecondPause, 1e-4) && Passed;

  Timer.Continue();
  Timer.Continue();
  MTime::BusyWait(2000);
  Passed = EvaluateTrue("Continue()", "idempotent", "Calling Continue twice keeps the timer running", Timer.GetElapsed() > BeforeSecondPause) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test timeout behavior
bool UTTimer::TestTimeouts()
{
  bool Passed = true;

  MTimer NoTimeout;
  Passed = EvaluateFalse("HasTimedOut()", "default timeout", "A timer without a positive timeout never times out", NoTimeout.HasTimedOut()) && Passed;

  NoTimeout.SetTimeOut(0.01);
  Passed = EvaluateNear("SetTimeOut()", "store timeout", "SetTimeOut stores the requested timeout", NoTimeout.GetTimeOut(), 0.01, 1e-12) && Passed;
  MTime::BusyWait(20000);
  Passed = EvaluateTrue("HasTimedOut()", "configured timeout", "HasTimedOut reports true after the configured timeout elapsed", NoTimeout.HasTimedOut()) && Passed;

  MTimer ExplicitTimeout;
  ExplicitTimeout.SetTimeOut(10.0);
  MTime::BusyWait(2000);
  Passed = EvaluateFalse("HasTimedOut()", "custom override", "HasTimedOut respects the explicit Seconds override", ExplicitTimeout.HasTimedOut(0.5)) && Passed;
  MTime::BusyWait(600000);
  Passed = EvaluateTrue("HasTimedOut()", "custom override", "HasTimedOut with an explicit Seconds value ignores the stored timeout", ExplicitTimeout.HasTimedOut(0.5)) && Passed;

  MTimer NegativeTimeout;
  NegativeTimeout.SetTimeOut(-1.0);
  MTime::BusyWait(2000);
  Passed = EvaluateFalse("HasTimedOut()", "negative timeout", "A non-positive timeout disables timeout detection", NegativeTimeout.HasTimedOut()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTimer Test;
  return Test.Run() == true ? 0 : 1;
}
