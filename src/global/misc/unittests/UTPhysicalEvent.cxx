/*
 * UTPhysicalEvent.cxx
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
#include <sstream>
#include <string>
using namespace std;

// MEGAlib:
#include "MExceptions.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MPhysicalEventHit.h"
#include "MString.h"
#include "MUnitTest.h"


//! Unit test class for the base physical event class
class UTPhysicalEvent : public MUnitTest
{
public:
  //! Default constructor
  UTPhysicalEvent() : MUnitTest("UTPhysicalEvent") {}
  //! Default destructor
  virtual ~UTPhysicalEvent() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Parse a tra-string line-by-line into an event object
  bool ParseTraString(MPhysicalEvent& Event, const MString& Tra, bool Fast = false);
  //! Test the base class state, flags, comments, hits, and duplication
  bool TestBaseEvent();
  //! Return the OI line of a tra-string (without line break)
  MString GetOILine(const MString& Tra);
  //! Test the optional particle ID in the OI information
  bool TestOIParticleID();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPhysicalEvent::Run()
{
  bool Passed = true;

  Passed = TestBaseEvent() && Passed;
  Passed = TestOIParticleID() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a tra-string line-by-line into an event object
bool UTPhysicalEvent::ParseTraString(MPhysicalEvent& Event, const MString& Tra, bool Fast)
{
  istringstream In(Tra.ToString());
  string Line;
  while (getline(In, Line)) {
    if (Line.empty() == true) {
      continue;
    }
    int Ret = Event.ParseLine(Line.c_str(), Fast);
    if (Ret == 1) {
      return false;
    }
  }
  return Event.Validate();
}


////////////////////////////////////////////////////////////////////////////////


//! Test the base class state, flags, comments, hits, and duplication
bool UTPhysicalEvent::TestBaseEvent()
{
  bool Passed = true;

  MPhysicalEvent Event;
  Passed = Evaluate("GetType()", "default", "Default physical events are unknown", Event.GetType(), MPhysicalEvent::c_Unknown) && Passed;
  Passed = Evaluate("GetEventType()", "default", "GetEventType() mirrors GetType()", Event.GetEventType(), MPhysicalEvent::c_Unknown) && Passed;
  Passed = Evaluate("GetTypeString()", "default", "Default physical events report an unknown type string", Event.GetTypeString(), MString("Unknown")) && Passed;
  Passed = Evaluate("GetEnergy()", "default", "Default physical events report no defined energy", Event.GetEnergy(), g_DoubleNotDefined) && Passed;
  Passed = Evaluate("Ei()", "default", "Ei() mirrors the default energy", Event.Ei(), g_DoubleNotDefined) && Passed;
  Passed = Evaluate("GetPosition()", "default", "Default physical events have no position", Event.GetPosition(), g_VectorNotDefined) && Passed;
  Passed = Evaluate("GetOrigin()", "default", "Default physical events have no origin direction", Event.GetOrigin(), g_VectorNotDefined) && Passed;
  Passed = Evaluate("ToString()", "default", "Default physical events stringify as a base event", Event.ToString(), MString("I am a physical event!")) && Passed;
  Passed = EvaluateFalse("Validate()", "default", "Base physical events do not validate successfully", Event.Validate()) && Passed;

  Event.SetTime(MTime(12.5));
  Event.SetId(42);
  Event.SetTimeWalk(7);
  Event.SetDecay(true);
  Event.AddBadFlag("bad event");
  Event.SetAllHitsGood(false);

  MVector OIPosition(1.0, 2.0, 3.0);
  MVector OIDirection(4.0, 5.0, 6.0);
  MVector OIPolarization(7.0, 8.0, 9.0);
  Event.SetOIInformation(OIPosition, OIDirection, OIPolarization, 10.0);

  MString Comment("comment");
  Event.AddComment(Comment);

  MPhysicalEventHit Hit;
  Hit.Set(MVector(11.0, 12.0, 13.0), MVector(0.1, 0.2, 0.3), 14.0, 0.4, MTime(15.0), MTime(0.5));
  Event.AddHit(Hit);

  Passed = Evaluate("GetTime()", "set/get", "SetTime stores the event time", Event.GetTime().GetAsDouble(), 12.5) && Passed;
  Passed = Evaluate("GetId()", "set/get", "SetId stores the event id", Event.GetId(), 42L) && Passed;
  Passed = Evaluate("GetTimeWalk()", "set/get", "SetTimeWalk stores the event time walk", Event.GetTimeWalk(), 7) && Passed;
  Passed = EvaluateTrue("IsDecay()", "set/get", "SetDecay stores the decay flag", Event.IsDecay()) && Passed;
  Passed = EvaluateTrue("IsBad()", "set/get", "AddBadFlag makes the event bad", Event.IsBad()) && Passed;
  Passed = Evaluate("GetNBadFlags()", "set/get", "One bad flag is stored", Event.GetNBadFlags(), 1U) && Passed;
  Passed = Evaluate("GetBadFlag()", "set/get", "AddBadFlag stores the bad flag", Event.GetBadFlag(0), MString("bad event")) && Passed;
  Passed = EvaluateTrue("HasBadFlag()", "set/get", "HasBadFlag finds the stored flag", Event.HasBadFlag("bad event")) && Passed;
  Passed = EvaluateFalse("HasBadFlag()", "set/get", "HasBadFlag does not find another flag", Event.HasBadFlag("other")) && Passed;
  Passed = EvaluateFalse("IsGoodEvent()", "set/get", "AddBadFlag does not imply a good event", Event.IsGoodEvent()) && Passed;
  Passed = EvaluateFalse("AllHitsGood()", "set/get", "SetAllHitsGood(false) updates the flag", Event.AllHitsGood()) && Passed;
  Passed = Evaluate("GetNComments()", "set/get", "One comment is stored", Event.GetNComments(), 1U) && Passed;
  Passed = Evaluate("GetComment()", "set/get", "Stored comment round-trips", Event.GetComment(0), Comment) && Passed;
  Passed = Evaluate("GetNHits()", "set/get", "One hit is stored", Event.GetNHits(), 1U) && Passed;
  Passed = Evaluate("GetHit().GetEnergy()", "set/get", "Stored hit round-trips", Event.GetHit(0).GetEnergy(), 14.0) && Passed;
  Passed = Evaluate("GetOIPosition()", "set/get", "OI position is stored", Event.GetOIPosition(), OIPosition) && Passed;
  Passed = Evaluate("GetOIDirection()", "set/get", "OI direction is stored", Event.GetOIDirection(), OIDirection) && Passed;
  Passed = Evaluate("GetOIPolarization()", "set/get", "OI polarization is stored", Event.GetOIPolarization(), OIPolarization) && Passed;
  Passed = Evaluate("GetOIEnergy()", "set/get", "OI energy is stored", Event.GetOIEnergy(), 10.0) && Passed;
  Event.SetAllHitsGood(true);
  Passed = EvaluateTrue("AllHitsGood()", "set/get", "SetAllHitsGood(true) updates the flag", Event.AllHitsGood()) && Passed;

  MString Tra = Event.ToTraString();
  MString ExpectedTra = MString("ET Unkown\nID 42\nTI ") + Event.GetTime().GetLongIntsString() + "\nTW 7\nBD bad event\nDC\nOI 1 2 3 4 5 6 7 8 9 10\nCC comment\n";
  Passed = Evaluate("ToTraString()", "base exact", "The base tra-string serialization is deterministic for representative event metadata", Tra, ExpectedTra) && Passed;
  Passed = Evaluate("ParseLine()", "base id", "The base parser accepts an id line", Event.ParseLine("ID 17", false), 0) && Passed;
  Passed = Evaluate("GetId()", "base id", "Parsing an id line stores the identifier", Event.GetId(), 17L) && Passed;
  Passed = Evaluate("ParseLine()", "base comment", "The base parser accepts a comment line", Event.ParseLine("CC parsed comment", false), 0) && Passed;
  Passed = Evaluate("GetComment()", "base comment", "Parsing a comment line stores the comment", Event.GetComment(Event.GetNComments()-1), MString("parsed comment")) && Passed;
  Passed = Evaluate("ParseLine()", "base end marker", "The base parser recognizes the end-of-event marker", Event.ParseLine("SE", false), -1) && Passed;
  Passed = Evaluate("ParseLine()", "base unknown line", "The base parser reports unrelated lines as unparsed", Event.ParseLine("XX something", false), 2) && Passed;

  MPhysicalEvent* Duplicate = Event.Duplicate();
  Passed = Evaluate("Duplicate()->GetType()", "base", "Duplicate preserves the event type", Duplicate->GetType(), Event.GetType()) && Passed;
  Passed = Evaluate("Duplicate()->GetId()", "base", "Duplicate preserves the event id", Duplicate->GetId(), Event.GetId()) && Passed;
  Passed = Evaluate("Duplicate()->GetTime()", "base", "Duplicate preserves the event time", Duplicate->GetTime().GetAsDouble(), Event.GetTime().GetAsDouble()) && Passed;
  Passed = Evaluate("Duplicate()->GetTimeWalk()", "base", "Duplicate preserves the time walk", Duplicate->GetTimeWalk(), Event.GetTimeWalk()) && Passed;
  Passed = Evaluate("Duplicate()->GetNBadFlags()", "base", "Duplicate preserves the number of bad flags", Duplicate->GetNBadFlags(), Event.GetNBadFlags()) && Passed;
  Passed = Evaluate("Duplicate()->GetBadFlag()", "base", "Duplicate preserves the bad flag", Duplicate->GetBadFlag(0), Event.GetBadFlag(0)) && Passed;
  Passed = Evaluate("Duplicate()->GetNComments()", "base", "Duplicate preserves comments", Duplicate->GetNComments(), Event.GetNComments()) && Passed;
  Passed = Evaluate("Duplicate()->GetNHits()", "base", "Duplicate preserves hits", Duplicate->GetNHits(), Event.GetNHits()) && Passed;
  Passed = Evaluate("Duplicate()->GetOIPosition()", "base", "Duplicate preserves the OI position", Duplicate->GetOIPosition(), Event.GetOIPosition()) && Passed;
  Passed = Evaluate("Duplicate()->GetOIEnergy()", "base", "Duplicate preserves the OI energy", Duplicate->GetOIEnergy(), Event.GetOIEnergy()) && Passed;
  delete Duplicate;

  MPhysicalEvent Flags;
  Passed = EvaluateFalse("IsBad()", "no flags", "An event without bad flags is not bad", Flags.IsBad()) && Passed;
  Flags.AddBadFlag("first");
  Flags.AddBadFlag("second (reason)");
  Passed = Evaluate("GetNBadFlags()", "two flags", "Both bad flags are stored", Flags.GetNBadFlags(), 2U) && Passed;
  Passed = Evaluate("GetBadFlag()", "two flags", "The first flag is kept in order", Flags.GetBadFlag(0), MString("first")) && Passed;
  Passed = Evaluate("GetBadFlag()", "two flags", "The second flag is kept in order", Flags.GetBadFlag(1), MString("second (reason)")) && Passed;
  MString FlagsTra = Flags.ToTraString();
  MString ExpectedFlagsTra = MString("ET Unkown\nID 0\nTI ") + Flags.GetTime().GetLongIntsString() + "\nBD first\nBD second (reason)\n";
  Passed = Evaluate("ToTraString()", "two flags", "Each bad flag is written as its own BD line", FlagsTra, ExpectedFlagsTra) && Passed;
  Passed = Evaluate("ParseLine()", "BD line", "The base parser accepts a BD line", Flags.ParseLine("BD third", false), 0) && Passed;
  Passed = Evaluate("GetNBadFlags()", "BD line", "A parsed BD line adds a flag instead of replacing one", Flags.GetNBadFlags(), 3U) && Passed;
  Passed = Evaluate("GetBadFlag()", "BD line", "The parsed flag is stored without the keyword", Flags.GetBadFlag(2), MString("third")) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetBadFlag()", "out-of-bounds", "Bad flag access outside the vector throws", [&](){ Flags.GetBadFlag(3); }) && Passed;
  Flags.ClearBadFlags();
  Passed = Evaluate("GetNBadFlags()", "clear bad flags", "ClearBadFlags removes all bad flags", Flags.GetNBadFlags(), 0U) && Passed;
  Passed = EvaluateFalse("IsBad()", "clear bad flags", "An event is no longer bad after ClearBadFlags", Flags.IsBad()) && Passed;
  Passed = EvaluateFalse("ToTraString()", "clear bad flags", "No BD line is written without bad flags", Flags.ToTraString().Contains("BD")) && Passed;

  MPhysicalEvent CommentOnly;
  MString AnotherComment("another comment");
  CommentOnly.AddComment(AnotherComment);
  Passed = Evaluate("GetNComments()", "clear comments", "One comment is stored before clearing", CommentOnly.GetNComments(), 1U) && Passed;
  CommentOnly.ClearComments();
  Passed = Evaluate("GetNComments()", "clear comments", "ClearComments removes all comments", CommentOnly.GetNComments(), 0U) && Passed;

  MFile File;
  MString FileName = GetTemporaryFileName("UTPhysicalEvent.tra");
  MPhysicalEvent StreamWrite;
  StreamWrite.SetId(23);
  StreamWrite.SetTime(MTime(3.5));
  Passed = EvaluateTrue("Open()", "base stream write", "The base event stream file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "base stream write", "The base event write-stream completes at EOF", StreamWrite.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  Passed = EvaluateTrue("WriteTextFile()", "base stream read setup", "The representative base event stream file can be written", WriteTextFile(FileName, "ID 19\nCC delayed\nSE\n")) && Passed;

  MPhysicalEvent StreamRead;
  Passed = EvaluateTrue("Open()", "base stream read", "The base event stream file can be opened for reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateTrue("Stream()", "base stream read", "The base event read-stream returns true at an explicit end marker", StreamRead.Stream(File, 0, true, false, false)) && Passed;
  Passed = Evaluate("StreamRead.GetId()", "base stream read", "The base read-stream preserves the parsed id", StreamRead.GetId(), 19L) && Passed;
  Passed = Evaluate("StreamRead.GetComment()", "base stream read", "The base read-stream preserves parsed comments", StreamRead.GetComment(0), MString("delayed")) && Passed;
  File.Close();

  MPhysicalEvent Delayed;
  Passed = EvaluateTrue("Open()", "base delayed read", "The base event stream file can be reopened for delayed reading", File.Open(FileName, MFile::c_Read)) && Passed;
  Passed = EvaluateTrue("Stream()", "base delayed read", "The base event delayed read buffers the event and returns true at the end marker", Delayed.Stream(File, 0, true, false, true)) && Passed;
  Passed = EvaluateTrue("ParseDelayed()", "base delayed read", "The delayed base event can be parsed explicitly", Delayed.ParseDelayed()) && Passed;
  Passed = Evaluate("Delayed.GetId()", "base delayed read", "Delayed parsing preserves the id", Delayed.GetId(), 19L) && Passed;
  Passed = Evaluate("Delayed.GetComment()", "base delayed read", "Delayed parsing preserves comments", Delayed.GetComment(0), MString("delayed")) && Passed;
  File.Close();

  Event.Reset();
  Passed = Evaluate("Reset()->GetNHits()", "base", "Reset clears hits", Event.GetNHits(), 0U) && Passed;
  Passed = Evaluate("Reset()->GetNComments()", "base", "Reset clears comments", Event.GetNComments(), 0U) && Passed;
  Passed = EvaluateFalse("Reset()->IsBad()", "base", "Reset clears the bad flag", Event.IsBad()) && Passed;
  Passed = EvaluateFalse("Reset()->IsDecay()", "base", "Reset clears the decay flag", Event.IsDecay()) && Passed;
  Passed = Evaluate("Reset()->GetTimeWalk()", "base", "Reset restores the default time walk", Event.GetTimeWalk(), -1) && Passed;
  Passed = Evaluate("Reset()->GetOIEnergy()", "base", "Reset clears the OI energy", Event.GetOIEnergy(), g_DoubleNotDefined) && Passed;

  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetComment()", "base out-of-bounds", "Comment access outside the vector throws", [&](){ Event.GetComment(1); }) && Passed;
  Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetHit()", "base out-of-bounds", "Hit access outside the vector throws", [&](){ Event.GetHit(1); }) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the OI line of a tra-string (without line break)
MString UTPhysicalEvent::GetOILine(const MString& Tra)
{
  istringstream In(Tra.ToString());
  string Line;
  while (getline(In, Line)) {
    if (Line.rfind("OI ", 0) == 0) {
      return Line.c_str();
    }
  }
  return "";
}


////////////////////////////////////////////////////////////////////////////////


//! Test the optional particle ID in the OI information
bool UTPhysicalEvent::TestOIParticleID()
{
  bool Passed = true;

  MVector OIPosition(1.0, 2.0, 3.0);
  MVector OIDirection(4.0, 5.0, 6.0);
  MVector OIPolarization(7.0, 8.0, 9.0);

  MPhysicalEvent Default;
  Passed = Evaluate("GetOIParticleID()", "OI id default", "Default physical events have no OI particle ID", Default.GetOIParticleID(), g_IntNotDefined) && Passed;

  // Setting without an ID keeps the old 10-value OI line
  MPhysicalEvent NoID;
  NoID.SetOIInformation(OIPosition, OIDirection, OIPolarization, 10.0);
  Passed = Evaluate("GetOIParticleID()", "OI id not set", "SetOIInformation without an ID leaves the particle ID undefined", NoID.GetOIParticleID(), g_IntNotDefined) && Passed;
  Passed = Evaluate("ToTraString()", "OI id not set", "Without an ID the OI line has the old 10 values", NoID.ToTraString(), MString("ET Unkown\nID 0\nTI ") + NoID.GetTime().GetLongIntsString() + "\nOI 1 2 3 4 5 6 7 8 9 10\n") && Passed;

  // Setting with an ID appends it as 11th value
  MPhysicalEvent WithID;
  WithID.SetOIInformation(OIPosition, OIDirection, OIPolarization, 10.0, 1);
  Passed = Evaluate("GetOIParticleID()", "OI id set", "SetOIInformation stores the particle ID", WithID.GetOIParticleID(), 1) && Passed;
  Passed = Evaluate("ToTraString()", "OI id set", "With an ID the OI line has the ID as 11th value", WithID.ToTraString(), MString("ET Unkown\nID 0\nTI ") + WithID.GetTime().GetLongIntsString() + "\nOI 1 2 3 4 5 6 7 8 9 10 1\n") && Passed;

  // Parse old and new OI lines with the slow and the fast parser
  for (bool Fast: { false, true }) {
    MString Mode = Fast ? "fast" : "slow";

    MPhysicalEvent Old;
    Passed = Evaluate("ParseLine()", "OI 10 values " + Mode, "OI lines without particle ID (older files) are accepted", Old.ParseLine("OI 1 2 3 4 5 6 7 8 9 10", Fast), 0) && Passed;
    Passed = Evaluate("GetOIEnergy()", "OI 10 values " + Mode, "The OI energy is read from OI lines without particle ID", Old.GetOIEnergy(), 10.0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI 10 values " + Mode, "OI lines without particle ID leave the ID undefined", Old.GetOIParticleID(), g_IntNotDefined) && Passed;

    MPhysicalEvent New;
    Passed = Evaluate("ParseLine()", "OI 11 values " + Mode, "OI lines with particle ID are accepted", New.ParseLine("OI 1 2 3 4 5 6 7 8 9 10 26056", Fast), 0) && Passed;
    Passed = Evaluate("GetOIEnergy()", "OI 11 values " + Mode, "The OI energy is read from OI lines with particle ID", New.GetOIEnergy(), 10.0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI 11 values " + Mode, "The particle ID is read from OI lines with particle ID", New.GetOIParticleID(), 26056) && Passed;

    // A later line without ID must not keep the ID of an earlier line
    Passed = Evaluate("ParseLine()", "OI id overwrite " + Mode, "An OI line without particle ID is accepted after one with ID", New.ParseLine("OI 1 2 3 4 5 6 7 8 9 20", Fast), 0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI id overwrite " + Mode, "An OI line without particle ID resets a previously parsed ID", New.GetOIParticleID(), g_IntNotDefined) && Passed;

    MPhysicalEvent Trailing;
    Passed = Evaluate("ParseLine()", "OI trailing space " + Mode, "OI lines with trailing whitespace after the energy are accepted", Trailing.ParseLine("OI 1 2 3 4 5 6 7 8 9 10 ", Fast), 0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI trailing space " + Mode, "Trailing whitespace is not read as particle ID", Trailing.GetOIParticleID(), g_IntNotDefined) && Passed;

    // Written OI lines read back identically
    MPhysicalEvent RoundTripID;
    Passed = Evaluate("ParseLine()", "OI id round trip " + Mode, "A written OI line with particle ID can be parsed", RoundTripID.ParseLine(GetOILine(WithID.ToTraString()), Fast), 0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI id round trip " + Mode, "The particle ID survives writing and reading", RoundTripID.GetOIParticleID(), 1) && Passed;
    MPhysicalEvent RoundTripNoID;
    Passed = Evaluate("ParseLine()", "OI no id round trip " + Mode, "A written OI line without particle ID can be parsed", RoundTripNoID.ParseLine(GetOILine(NoID.ToTraString()), Fast), 0) && Passed;
    Passed = Evaluate("GetOIEnergy()", "OI no id round trip " + Mode, "The OI energy survives writing and reading without particle ID", RoundTripNoID.GetOIEnergy(), 10.0) && Passed;
    Passed = Evaluate("GetOIParticleID()", "OI no id round trip " + Mode, "An undefined particle ID stays undefined after writing and reading", RoundTripNoID.GetOIParticleID(), g_IntNotDefined) && Passed;
  }

  MPhysicalEvent Broken;
  Passed = Evaluate("ParseLine()", "OI 9 values slow", "The slow parser rejects OI lines with too few values", Broken.ParseLine("OI 1 2 3 4 5 6 7 8 9", false), 1) && Passed;

  // Duplicate (Assimilate) and Reset
  MPhysicalEvent* Duplicate = WithID.Duplicate();
  Passed = Evaluate("Duplicate()->GetOIParticleID()", "OI id duplicate", "Duplicate preserves the OI particle ID", Duplicate->GetOIParticleID(), 1) && Passed;
  Passed = Evaluate("Duplicate()->GetOIDirection()", "OI id duplicate", "Duplicate preserves the OI direction", Duplicate->GetOIDirection(), OIDirection) && Passed;
  Passed = Evaluate("Duplicate()->GetOIPolarization()", "OI id duplicate", "Duplicate preserves the OI polarization", Duplicate->GetOIPolarization(), OIPolarization) && Passed;
  delete Duplicate;

  WithID.Reset();
  Passed = Evaluate("Reset()->GetOIParticleID()", "OI id reset", "Reset clears the OI particle ID", WithID.GetOIParticleID(), g_IntNotDefined) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPhysicalEvent Test;
  return Test.Run() == true ? 0 : 1;
}
