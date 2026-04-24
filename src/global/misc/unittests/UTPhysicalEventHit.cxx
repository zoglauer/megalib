/*
 * UTPhysicalEventHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <string>
using namespace std;

// MEGAlib:
#include "MPhysicalEventHit.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for MPhysicalEventHit
class UTPhysicalEventHit : public MUnitTest
{
public:
  //! Default constructor
  UTPhysicalEventHit() : MUnitTest("UTPhysicalEventHit") {}
  //! Default destructor
  virtual ~UTPhysicalEventHit() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test default state and setters
  bool TestBasics();
  //! Test copy constructor and assignment operator
  bool TestCopyAndAssign();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPhysicalEventHit::Run()
{
  bool Passed = true;

  Passed = TestBasics() && Passed;
  Passed = TestCopyAndAssign() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test default state and setters
bool UTPhysicalEventHit::TestBasics()
{
  bool Passed = true;

  MPhysicalEventHit Hit;
  Passed = Evaluate("GetPosition()", "default", "Default hits start at the origin", Hit.GetPosition(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("GetPositionUncertainty()", "default", "Default hits have zero position uncertainty", Hit.GetPositionUncertainty(), MVector(0.0, 0.0, 0.0)) && Passed;
  Passed = Evaluate("GetEnergy()", "default", "Default hits start with zero energy", Hit.GetEnergy(), 0.0) && Passed;
  Passed = Evaluate("GetEnergyUncertainty()", "default", "Default hits start with zero energy uncertainty", Hit.GetEnergyUncertainty(), 0.0) && Passed;
  Passed = Evaluate("GetTime()", "default", "Default hits start with zero time", Hit.GetTime().GetAsDouble(), 0.0) && Passed;
  Passed = Evaluate("GetTimeUncertainty()", "default", "Default hits start with zero time uncertainty", Hit.GetTimeUncertainty().GetAsDouble(), 0.0) && Passed;

  MVector Position(1.0, 2.0, 3.0);
  MVector PositionUncertainty(0.1, 0.2, 0.3);
  MTime Time(4.5);
  MTime TimeUncertainty(0.5);
  Hit.Set(Position, PositionUncertainty, 6.0, 0.6, Time, TimeUncertainty);

  Passed = Evaluate("GetPosition()", "set/get", "The hit position is stored", Hit.GetPosition(), Position) && Passed;
  Passed = Evaluate("GetPositionUncertainty()", "set/get", "The hit position uncertainty is stored", Hit.GetPositionUncertainty(), PositionUncertainty) && Passed;
  Passed = Evaluate("GetEnergy()", "set/get", "The hit energy is stored", Hit.GetEnergy(), 6.0) && Passed;
  Passed = Evaluate("GetEnergyUncertainty()", "set/get", "The hit energy uncertainty is stored", Hit.GetEnergyUncertainty(), 0.6) && Passed;
  Passed = Evaluate("GetTime()", "set/get", "The hit time is stored", Hit.GetTime().GetAsDouble(), Time.GetAsDouble()) && Passed;
  Passed = Evaluate("GetTimeUncertainty()", "set/get", "The hit time uncertainty is stored", Hit.GetTimeUncertainty().GetAsDouble(), TimeUncertainty.GetAsDouble()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test copy constructor and assignment operator
bool UTPhysicalEventHit::TestCopyAndAssign()
{
  bool Passed = true;

  MPhysicalEventHit Original;
  Original.Set(MVector(7.0, 8.0, 9.0), MVector(0.7, 0.8, 0.9), 10.0, 1.0, MTime(11.0), MTime(1.1));

  MPhysicalEventHit Copied(Original);
  Passed = Evaluate("Copy ctor", "copy", "The copy constructor preserves the position", Copied.GetPosition(), Original.GetPosition()) && Passed;
  Passed = Evaluate("Copy ctor", "copy", "The copy constructor preserves the energy", Copied.GetEnergy(), Original.GetEnergy()) && Passed;
  Passed = Evaluate("Copy ctor", "copy", "The copy constructor preserves the time", Copied.GetTime().GetAsDouble(), Original.GetTime().GetAsDouble()) && Passed;

  MPhysicalEventHit Assigned;
  Assigned = Original;
  Passed = Evaluate("operator=", "assign", "The assignment operator preserves the position", Assigned.GetPosition(), Original.GetPosition()) && Passed;
  Passed = Evaluate("operator=", "assign", "The assignment operator preserves the energy", Assigned.GetEnergy(), Original.GetEnergy()) && Passed;
  Passed = Evaluate("operator=", "assign", "The assignment operator preserves the time", Assigned.GetTime().GetAsDouble(), Original.GetTime().GetAsDouble()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPhysicalEventHit Test;
  return Test.Run() ? 0 : 1;
}

