/*
 * UTPhysicalEvent.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
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
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTPhysicalEvent::Run()
{
  bool Passed = true;

  Passed = TestBaseEvent() && Passed;

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
  Event.SetBad(true, "bad event");
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
  Passed = EvaluateTrue("IsBad()", "set/get", "SetBad stores the bad flag", Event.IsBad()) && Passed;
  Passed = Evaluate("GetBadString()", "set/get", "SetBad stores the bad description", Event.GetBadString(), MString("bad event")) && Passed;
  Passed = EvaluateFalse("IsGoodEvent()", "set/get", "SetBad does not imply a good event", Event.IsGoodEvent()) && Passed;
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
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string starts with an event type line", Tra.BeginsWith("ET")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the id", Tra.Contains("ID 42")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the time", Tra.Contains("TI")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the time walk", Tra.Contains("TW 7")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the bad flag", Tra.Contains("BD bad event")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the OI block", Tra.Contains("OI 1")) && Passed;
  Passed = EvaluateTrue("ToTraString()", "base", "The base tra-string contains the comment", Tra.Contains("CC comment")) && Passed;
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
  Passed = Evaluate("Duplicate()->GetBadString()", "base", "Duplicate preserves the bad description", Duplicate->GetBadString(), Event.GetBadString()) && Passed;
  Passed = Evaluate("Duplicate()->GetNComments()", "base", "Duplicate preserves comments", Duplicate->GetNComments(), Event.GetNComments()) && Passed;
  Passed = Evaluate("Duplicate()->GetNHits()", "base", "Duplicate preserves hits", Duplicate->GetNHits(), Event.GetNHits()) && Passed;
  Passed = Evaluate("Duplicate()->GetOIPosition()", "base", "Duplicate preserves the OI position", Duplicate->GetOIPosition(), Event.GetOIPosition()) && Passed;
  Passed = Evaluate("Duplicate()->GetOIEnergy()", "base", "Duplicate preserves the OI energy", Duplicate->GetOIEnergy(), Event.GetOIEnergy()) && Passed;
  delete Duplicate;

  MPhysicalEvent CommentOnly;
  MString AnotherComment("another comment");
  CommentOnly.AddComment(AnotherComment);
  Passed = Evaluate("GetNComments()", "clear comments", "One comment is stored before clearing", CommentOnly.GetNComments(), 1U) && Passed;
  CommentOnly.ClearComments();
  Passed = Evaluate("GetNComments()", "clear comments", "ClearComments removes all comments", CommentOnly.GetNComments(), 0U) && Passed;

  MFile File;
  MString FileName = "/tmp/UTPhysicalEvent.tra";
  MPhysicalEvent StreamWrite;
  StreamWrite.SetId(23);
  StreamWrite.SetTime(MTime(3.5));
  Passed = EvaluateTrue("Open()", "base stream write", "The base event stream file can be opened for writing", File.Open(FileName, MFile::c_Write)) && Passed;
  Passed = EvaluateFalse("Stream()", "base stream write", "The base event write-stream completes at EOF", StreamWrite.Stream(File, 0, false, false, false)) && Passed;
  File.Close();

  {
    ofstream Out(FileName);
    Out << "ID 19\n";
    Out << "CC delayed\n";
    Out << "SE\n";
  }

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

  EvaluateException<MExceptionIndexOutOfBounds>("GetComment()", "base out-of-bounds", "Comment access outside the vector throws", [&](){ Event.GetComment(1); });
  EvaluateException<MExceptionIndexOutOfBounds>("GetHit()", "base out-of-bounds", "Hit access outside the vector throws", [&](){ Event.GetHit(1); });

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


// Main entry point
int main()
{
  UTPhysicalEvent Test;
  return Test.Run() ? 0 : 1;
}
