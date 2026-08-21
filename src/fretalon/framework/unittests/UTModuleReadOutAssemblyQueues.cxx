/*
 * UTModuleReadOutAssemblyQueues.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MModuleReadOutAssemblyQueues.h"
#include "MReadOutAssembly.h"
#include "MUnitTest.h"

// Standard libs:
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>
using namespace std;


//! Unit test class for MModuleReadOutAssemblyQueues
class UTModuleReadOutAssemblyQueues : public MUnitTest
{
public:
  //! Default constructor
  UTModuleReadOutAssemblyQueues() : MUnitTest("UTModuleReadOutAssemblyQueues") {}
  //! Default destructor
  virtual ~UTModuleReadOutAssemblyQueues() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test default FIFO queue behavior without sorting
  bool TestUnsortedQueues();
  //! Test sorted outgoing queues preserve incoming order
  bool TestSortedQueues();
  //! Test defensive behavior and clearing
  bool TestDefensiveAndClearBehavior();
  //! Test the incoming queue under concurrent producers and consumers
  bool TestConcurrentIncoming();
  //! Test the sorted outgoing queue under concurrent producers
  bool TestConcurrentSortedOutgoing();

  //! Number of threads used to force contention
  static const unsigned int c_Threads = 4;
  //! Number of events each producer thread contributes
  static const unsigned int c_EventsPerThread = 250;
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::Run()
{
  bool Passed = true;

  Passed = TestUnsortedQueues() && Passed;
  Passed = TestSortedQueues() && Passed;
  Passed = TestDefensiveAndClearBehavior() && Passed;
  Passed = TestConcurrentIncoming() && Passed;
  Passed = TestConcurrentSortedOutgoing() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::TestUnsortedQueues()
{
  bool Passed = true;

  MModuleReadOutAssemblyQueues Queues;
  Passed = EvaluateFalse("HasIncoming()", "default", "A new queue has no incoming events", Queues.HasIncoming()) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "default", "A new queue has no outgoing events", Queues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("GetIncoming()", "empty queue", "An empty incoming queue returns no event", Queues.GetIncoming() == nullptr) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "empty queue", "An empty outgoing queue returns no event", Queues.GetOutgoing() == nullptr) && Passed;

  MReadOutAssembly* First = new MReadOutAssembly();
  MReadOutAssembly* Second = new MReadOutAssembly();
  First->SetID(1);
  Second->SetID(2);
  Passed = EvaluateTrue("AddIncoming()", "first event", "Adding the first incoming event succeeds", Queues.AddIncoming(First)) && Passed;
  Passed = EvaluateTrue("AddIncoming()", "second event", "Adding the second incoming event succeeds", Queues.AddIncoming(Second)) && Passed;
  Passed = EvaluateTrue("HasIncoming()", "after add", "The queue reports incoming events after adding them", Queues.HasIncoming()) && Passed;
  Passed = EvaluateTrue("GetIncoming()", "first event", "The first incoming event is returned first", Queues.GetIncoming() == First) && Passed;
  Passed = EvaluateTrue("GetIncoming()", "second event", "The second incoming event is returned second", Queues.GetIncoming() == Second) && Passed;
  Passed = EvaluateFalse("HasIncoming()", "after reads", "The incoming queue is empty after both reads", Queues.HasIncoming()) && Passed;

  Passed = EvaluateTrue("AddOutgoing()", "second event first", "Adding an outgoing event succeeds in unsorted mode", Queues.AddOutgoing(Second)) && Passed;
  Passed = EvaluateTrue("AddOutgoing()", "first event second", "Adding a second outgoing event succeeds in unsorted mode", Queues.AddOutgoing(First)) && Passed;
  Passed = EvaluateTrue("HasOutgoing()", "after outgoing add", "The queue reports outgoing events after adding them", Queues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "second event first", "Unsorted outgoing events are returned in outgoing insertion order", Queues.GetOutgoing() == Second) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "first event second", "Unsorted outgoing events preserve outgoing insertion order", Queues.GetOutgoing() == First) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "after outgoing reads", "The outgoing queue is empty after both reads", Queues.HasOutgoing()) && Passed;

  delete First;
  delete Second;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::TestSortedQueues()
{
  bool Passed = true;

  MModuleReadOutAssemblyQueues Queues;
  Queues.EnableSorting();

  MReadOutAssembly* First = new MReadOutAssembly();
  MReadOutAssembly* Second = new MReadOutAssembly();
  MReadOutAssembly* Third = new MReadOutAssembly();
  First->SetID(1);
  Second->SetID(2);
  Third->SetID(3);

  Queues.AddIncoming(First);
  Queues.AddIncoming(Second);
  Queues.AddIncoming(Third);
  Passed = EvaluateTrue("GetIncoming()", "first sorted event", "Sorted mode still returns incoming events in FIFO order", Queues.GetIncoming() == First) && Passed;
  Passed = EvaluateTrue("GetIncoming()", "second sorted event", "Sorted mode records the second incoming event order", Queues.GetIncoming() == Second) && Passed;
  Passed = EvaluateTrue("GetIncoming()", "third sorted event", "Sorted mode records the third incoming event order", Queues.GetIncoming() == Third) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "placeholders only", "Sorted placeholders are not visible as outgoing events until the first slot is filled", Queues.HasOutgoing()) && Passed;

  Passed = EvaluateTrue("AddOutgoing()", "second first", "Sorted mode accepts an outgoing event before its turn", Queues.AddOutgoing(Second)) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "first slot still empty", "A later sorted outgoing event is held until earlier events arrive", Queues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("AddOutgoing()", "third second", "Sorted mode accepts another out-of-order outgoing event", Queues.AddOutgoing(Third)) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "first slot still empty with later events", "Later sorted outgoing events remain hidden while the first slot is empty", Queues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("AddOutgoing()", "first last", "Sorted mode accepts the first event after later events", Queues.AddOutgoing(First)) && Passed;

  Passed = EvaluateTrue("HasOutgoing()", "first slot filled", "The sorted outgoing queue becomes visible when the first slot is filled", Queues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "first event", "Sorted outgoing events are returned in original incoming order", Queues.GetOutgoing() == First) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "second event", "The second sorted outgoing event is returned after the first", Queues.GetOutgoing() == Second) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "third event", "The third sorted outgoing event is returned after the second", Queues.GetOutgoing() == Third) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "sorted queue empty", "The sorted outgoing queue is empty after all reads", Queues.HasOutgoing()) && Passed;

  delete First;
  delete Second;
  delete Third;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::TestDefensiveAndClearBehavior()
{
  bool Passed = true;

  MModuleReadOutAssemblyQueues Queues;

  // The rejection paths below print diagnostics gated by the global verbosity. Silence only the
  // noisy call itself -- an Evaluate* made while output is suppressed would hide its own failure.
  const int PreviousVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  const bool NullIncomingAccepted = Queues.AddIncoming(nullptr);
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateFalse("AddIncoming()", "nullptr", "Adding a null incoming event is rejected", NullIncomingAccepted) && Passed;
  Passed = EvaluateFalse("HasIncoming()", "after nullptr", "A rejected null incoming event does not change the queue", Queues.HasIncoming()) && Passed;

  MReadOutAssembly* Queued = new MReadOutAssembly();
  Queues.AddIncoming(Queued);
  Queues.EnableSorting();
  Passed = EvaluateTrue("GetIncoming()", "sorting unchanged with non-empty queue", "Enabling sorting while incoming events exist leaves existing FIFO behavior intact", Queues.GetIncoming() == Queued) && Passed;
  Passed = EvaluateTrue("AddOutgoing()", "unsorted after failed sorting change", "The queue remains usable after rejecting a sorting-mode change with queued events", Queues.AddOutgoing(Queued)) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "unsorted after failed sorting change", "Outgoing retrieval remains unsorted after the failed sorting-mode change", Queues.GetOutgoing() == Queued) && Passed;
  delete Queued;

  MModuleReadOutAssemblyQueues SortedQueues;
  SortedQueues.EnableSorting();
  MReadOutAssembly* Known = new MReadOutAssembly();
  MReadOutAssembly* Unknown = new MReadOutAssembly();
  SortedQueues.AddIncoming(Known);
  Passed = EvaluateTrue("GetIncoming()", "known sorted event", "A known sorted event can be read from the incoming queue", SortedQueues.GetIncoming() == Known) && Passed;
  g_Verbosity = c_Quiet;
  const bool UnknownOutgoingAccepted = SortedQueues.AddOutgoing(Unknown);
  g_Verbosity = PreviousVerbosity;
  Passed = EvaluateFalse("AddOutgoing()", "unknown sorted event", "Sorted mode rejects an outgoing event that was not part of the incoming order", UnknownOutgoingAccepted) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "after unknown sorted event", "Rejecting an unknown sorted outgoing event leaves the first sorted slot empty", SortedQueues.HasOutgoing()) && Passed;
  Passed = EvaluateTrue("AddOutgoing()", "known sorted event", "The known sorted event can still be added after rejecting an unknown event", SortedQueues.AddOutgoing(Known)) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "known sorted event", "The known sorted event is returned after being added", SortedQueues.GetOutgoing() == Known) && Passed;
  delete Known;
  delete Unknown;

  MModuleReadOutAssemblyQueues ClearQueues;
  ClearQueues.AddIncoming(new MReadOutAssembly());
  ClearQueues.AddOutgoing(new MReadOutAssembly());
  Passed = EvaluateTrue("HasIncoming()", "before Clear", "The clear test queue has an incoming event before clearing", ClearQueues.HasIncoming()) && Passed;
  Passed = EvaluateTrue("HasOutgoing()", "before Clear", "The clear test queue has an outgoing event before clearing", ClearQueues.HasOutgoing()) && Passed;
  ClearQueues.Clear();
  Passed = EvaluateFalse("HasIncoming()", "after Clear", "Clear removes incoming events", ClearQueues.HasIncoming()) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "after Clear", "Clear removes outgoing events", ClearQueues.HasOutgoing()) && Passed;

  ClearQueues.EnableSorting();
  ClearQueues.Clear();
  MReadOutAssembly* AfterClear = new MReadOutAssembly();
  Passed = EvaluateTrue("AddOutgoing()", "after sorted Clear", "Clear resets sorting so outgoing events can be added without a recorded incoming order", ClearQueues.AddOutgoing(AfterClear)) && Passed;
  Passed = EvaluateTrue("GetOutgoing()", "after sorted Clear", "The post-clear outgoing event is returned normally", ClearQueues.GetOutgoing() == AfterClear) && Passed;
  delete AfterClear;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::TestConcurrentIncoming()
{
  bool Passed = true;

  const unsigned int TotalEvents = c_Threads * c_EventsPerThread;

  // Phase 1: several producers push at the same time, then the queue is drained on this thread.
  // Every event must survive exactly once, so nothing is lost or duplicated under contention.
  {
    MModuleReadOutAssemblyQueues Queues;

    vector<thread> Producers;
    for (unsigned int t = 0; t < c_Threads; ++t) {
      Producers.push_back(thread([&Queues, t]() {
        for (unsigned int e = 0; e < c_EventsPerThread; ++e) {
          MReadOutAssembly* Event = new MReadOutAssembly();
          Event->SetID(t*c_EventsPerThread + e);
          Queues.AddIncoming(Event);
        }
      }));
    }
    for (thread& Producer: Producers) Producer.join();

    vector<unsigned long> Seen;
    while (true) {
      MReadOutAssembly* Event = Queues.GetIncoming();
      if (Event == nullptr) break;
      Seen.push_back(Event->GetID());
      delete Event;
    }
    sort(Seen.begin(), Seen.end());

    Passed = EvaluateSize("AddIncoming()", "concurrent producers", "Concurrent producers add every event exactly once", Seen.size(), static_cast<size_t>(TotalEvents)) && Passed;
    bool AllPresent = (Seen.size() == TotalEvents);
    for (unsigned int i = 0; i < Seen.size() && AllPresent == true; ++i) {
      if (Seen[i] != i) AllPresent = false;
    }
    Passed = EvaluateTrue("AddIncoming()", "concurrent producers", "Every event ID appears exactly once after concurrent adds", AllPresent) && Passed;
    Passed = EvaluateFalse("HasIncoming()", "after concurrent drain", "The incoming queue is empty once every event has been read", Queues.HasIncoming()) && Passed;
  }

  // Phase 2: producers and consumers run at the same time, so adds and reads contend on the
  // same mutex. Each consumer records what it received; together they must account for every
  // event exactly once.
  {
    MModuleReadOutAssemblyQueues Queues;
    atomic<unsigned int> Consumed(0);
    vector<vector<unsigned long>> Received(c_Threads);

    vector<thread> Producers;
    for (unsigned int t = 0; t < c_Threads; ++t) {
      Producers.push_back(thread([&Queues, t]() {
        for (unsigned int e = 0; e < c_EventsPerThread; ++e) {
          MReadOutAssembly* Event = new MReadOutAssembly();
          Event->SetID(t*c_EventsPerThread + e);
          Queues.AddIncoming(Event);
        }
      }));
    }

    vector<thread> Consumers;
    for (unsigned int t = 0; t < c_Threads; ++t) {
      Consumers.push_back(thread([&Queues, &Consumed, &Received, t]() {
        while (Consumed.load() < TotalEvents) {
          MReadOutAssembly* Event = Queues.GetIncoming();
          if (Event != nullptr) {
            Received[t].push_back(Event->GetID());
            delete Event;
            ++Consumed;
          } else {
            this_thread::yield();
          }
        }
      }));
    }

    for (thread& Producer: Producers) Producer.join();
    for (thread& Consumer: Consumers) Consumer.join();

    vector<unsigned long> Seen;
    for (const vector<unsigned long>& Batch: Received) {
      Seen.insert(Seen.end(), Batch.begin(), Batch.end());
    }
    sort(Seen.begin(), Seen.end());

    Passed = EvaluateSize("GetIncoming()", "concurrent producers and consumers", "Concurrent consumers together receive every event exactly once", Seen.size(), static_cast<size_t>(TotalEvents)) && Passed;
    bool AllPresent = (Seen.size() == TotalEvents);
    for (unsigned int i = 0; i < Seen.size() && AllPresent == true; ++i) {
      if (Seen[i] != i) AllPresent = false;
    }
    Passed = EvaluateTrue("GetIncoming()", "concurrent producers and consumers", "No event is lost or handed to two consumers", AllPresent) && Passed;
    Passed = EvaluateFalse("HasIncoming()", "after concurrent transfer", "The incoming queue is empty after every event has been consumed", Queues.HasIncoming()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleReadOutAssemblyQueues::TestConcurrentSortedOutgoing()
{
  bool Passed = true;

  // In sorted mode the outgoing order is pinned by the incoming order, which is what lets several
  // module instances analyze events in parallel and still emit them in the original sequence.
  MModuleReadOutAssemblyQueues Queues;
  Queues.EnableSorting();

  const unsigned int TotalEvents = c_Threads * c_EventsPerThread;

  vector<MReadOutAssembly*> Events;
  for (unsigned int e = 0; e < TotalEvents; ++e) {
    MReadOutAssembly* Event = new MReadOutAssembly();
    Event->SetID(e);
    Events.push_back(Event);
    Queues.AddIncoming(Event);
  }
  while (Queues.GetIncoming() != nullptr) {
    // Draining the incoming queue is what records the expected outgoing order
  }

  // Hand the events back from several threads in interleaved order, so each thread returns a
  // different stride of the sequence and the queue has to re-order them
  vector<thread> Producers;
  for (unsigned int t = 0; t < c_Threads; ++t) {
    Producers.push_back(thread([&Queues, &Events, t]() {
      for (unsigned int e = t; e < TotalEvents; e += c_Threads) {
        Queues.AddOutgoing(Events[e]);
      }
    }));
  }
  for (thread& Producer: Producers) Producer.join();

  vector<unsigned long> Order;
  while (true) {
    MReadOutAssembly* Event = Queues.GetOutgoing();
    if (Event == nullptr) break;
    Order.push_back(Event->GetID());
  }

  Passed = EvaluateSize("AddOutgoing()", "concurrent sorted producers", "Every event returned out of order is emitted again", Order.size(), static_cast<size_t>(TotalEvents)) && Passed;
  bool InOrder = (Order.size() == TotalEvents);
  for (unsigned int i = 0; i < Order.size() && InOrder == true; ++i) {
    if (Order[i] != i) InOrder = false;
  }
  Passed = EvaluateTrue("GetOutgoing()", "concurrent sorted producers", "The sorted queue restores the original incoming order despite concurrent out-of-order returns", InOrder) && Passed;
  Passed = EvaluateFalse("HasOutgoing()", "after concurrent sorted drain", "The sorted outgoing queue is empty after every event has been read", Queues.HasOutgoing()) && Passed;

  for (MReadOutAssembly* Event: Events) delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleReadOutAssemblyQueues Test;
  return Test.Run() == true ? 0 : 1;
}
