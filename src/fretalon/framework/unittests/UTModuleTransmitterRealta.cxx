/*
 * UTModuleTransmitterRealta.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MAssembly.h"
#include "MModuleTransmitterRealta.h"
#include "MReadOut.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutElementStrip.h"
#include "MTimer.h"
#include "MTransceiverTcpIp.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Standard libs:
// Sockets and process IDs have no C++ standard-library equivalent, so the POSIX headers are
// required here -- the same approach the transceiver unit tests in src/global/misc use.
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TError.h"
#include "TSystem.h"


//! Unit test class for MModuleTransmitterRealta
class UTModuleTransmitterRealta : public MUnitTest
{
public:
  //! Default constructor
  UTModuleTransmitterRealta() : MUnitTest("UTModuleTransmitterRealta") {}
  //! Default destructor
  virtual ~UTModuleTransmitterRealta() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Return a TCP port the operating system currently reports as free
  static int GetFreePort();
  //! Wait until the transceiver reports a connection, or the time out elapses
  static bool WaitForConnected(MTransceiverTcpIp& Transceiver, double TimeOut);
  //! Wait until a complete message has been received, or the time out elapses
  static bool WaitForMessage(MTransceiverTcpIp& Transceiver, MString& Message, double TimeOut);
  //! Test that an analyzed event actually reaches a connected client
  bool TestTransmissionRoundTrip();

  //! Time out for establishing the loop-back connection
  static constexpr double c_ConnectTimeOut = 5.0;
  //! Time out for receiving a transmitted event
  static constexpr double c_ReceiveTimeOut = 5.0;
  //! Create a representative read-out assembly
  MReadOutAssembly* CreateRepresentativeEvent(unsigned long ID = 42);
  //! Test construction-time module metadata and host interface behavior
  bool TestMetadataAndHostSettings();
  //! Test XML configuration helpers
  bool TestXmlConfiguration();
  //! Test transmitter lifecycle and direct event analysis
  bool TestTransmissionLifecycle();
  //! Test inherited queue processing through DoSingleAnalysis
  bool TestInheritedFlow();
};


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::Run()
{
  bool Passed = true;

  Passed = TestMetadataAndHostSettings() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestTransmissionLifecycle() && Passed;
  Passed = TestTransmissionRoundTrip() && Passed;
  Passed = TestInheritedFlow() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTModuleTransmitterRealta::GetFreePort()
{
  // Bind a throw-away socket to port 0 and let the operating system pick a free port, then read it
  // back and release it. This mirrors UTTransceiverTcpIp::GetFreePort() in src/global/misc.
  static int FallbackPort = 55000 + static_cast<int>(getpid() % 5000);
  auto GetFallbackPort = [&]() -> int {
    ++FallbackPort;
    if (FallbackPort > 65000) {
      FallbackPort = 55000;
    }
    return FallbackPort;
  };

  int Socket = socket(AF_INET, SOCK_STREAM, 0);
  if (Socket < 0) {
    return GetFallbackPort();
  }

  sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_family = AF_INET;
  Address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  Address.sin_port = htons(0);

  if (::bind(Socket, reinterpret_cast<sockaddr*>(&Address), sizeof(Address)) != 0) {
    close(Socket);
    return GetFallbackPort();
  }

  socklen_t Length = sizeof(Address);
  if (getsockname(Socket, reinterpret_cast<sockaddr*>(&Address), &Length) != 0) {
    close(Socket);
    return GetFallbackPort();
  }

  const int Port = ntohs(Address.sin_port);
  close(Socket);
  return Port == 0 ? GetFallbackPort() : Port;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* UTModuleTransmitterRealta::CreateRepresentativeEvent(unsigned long ID)
{
  MReadOutAssembly* Event = new MReadOutAssembly();
  Event->SetFilteredOut(false);
  Event->SetID(ID);

  MReadOutElementStrip Element(7, 11);
  MReadOutDataADCValue Data;
  Data.SetADCValue(1234);
  MReadOut ReadOut(Element, Data);
  Event->AddReadOut(ReadOut);

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::TestMetadataAndHostSettings()
{
  bool Passed = true;

  MModuleTransmitterRealta Module;
  Passed = Evaluate("GetName()", "default", "The Realta transmitter exposes its representative module name", Module.GetName(), MString("Transmitter for fully calibrated events to Realta")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The Realta transmitter exposes its representative XML tag", Module.GetXmlTag(), MString("XmlTagTransmitterRealta")) && Passed;

  Passed = EvaluateSize("GetNPreceedingModuleTypes()", "transmitter", "The Realta transmitter registers the expected predecessor requirements", Module.GetNPreceedingModuleTypes(), static_cast<size_t>(2)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "energy calibration", "The first predecessor is energy calibration", Module.GetPreceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = Evaluate("GetPreceedingModuleType()", "position determination", "The second predecessor is position determination", Module.GetPreceedingModuleType(1), static_cast<uint64_t>(MAssembly::c_PositionDetermiation)) && Passed;
  Passed = EvaluateTrue("GetPreceedingModuleHardRequirement()", "energy calibration", "The energy-calibration predecessor is a hard requirement", Module.GetPreceedingModuleHardRequirement(0)) && Passed;
  Passed = EvaluateTrue("GetPreceedingModuleHardRequirement()", "position determination", "The position-determination predecessor is a hard requirement", Module.GetPreceedingModuleHardRequirement(1)) && Passed;

  Passed = EvaluateSize("GetNModuleTypes()", "transmitter", "The Realta transmitter registers one provided module type", Module.GetNModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetModuleType()", "event transmitter", "The Realta transmitter provides the event-transmitter module type", Module.GetModuleType(0), static_cast<uint64_t>(MAssembly::c_EventTransmitter)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "event transmitter", "The Realta transmitter reports that it provides event transmission", Module.ProvidesModuleType(MAssembly::c_EventTransmitter)) && Passed;
  Passed = EvaluateFalse("ProvidesModuleType()", "missing type", "The Realta transmitter rejects a representative unregistered module type", Module.ProvidesModuleType(0x80000000ULL)) && Passed;

  Passed = EvaluateSize("GetNSucceedingModuleTypes()", "transmitter", "The Realta transmitter registers one succeeding module type", Module.GetNSucceedingModuleTypes(), static_cast<size_t>(1)) && Passed;
  Passed = Evaluate("GetSucceedingModuleType()", "no restriction", "The Realta transmitter allows unrestricted succeeding modules", Module.GetSucceedingModuleType(0), static_cast<uint64_t>(MAssembly::c_NoRestriction)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "transmitter", "The Realta transmitter exposes an options GUI", Module.HasOptionsGUI()) && Passed;
  Passed = EvaluateFalse("IsStartModule()", "transmitter", "The Realta transmitter is not a start module", Module.IsStartModule()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "transmitter", "The Realta transmitter allows multi-threading", Module.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "transmitter", "The Realta transmitter does not allow multiple instances", Module.AllowsMultipleInstances()) && Passed;

  Passed = Evaluate("GetHostName()", "default", "The Realta transmitter defaults to localhost", Module.GetHostName(), MString("localhost")) && Passed;
  Passed = Evaluate("GetHostPort()", "default", "The Realta transmitter defaults to the representative Realta port", Module.GetHostPort(), 12354) && Passed;
  const int RepresentativePort = 24601;
  Module.SetHostName("127.0.0.1");
  Module.SetHostPort(RepresentativePort);
  Passed = Evaluate("SetHostName()/GetHostName()", "127.0.0.1", "The host-name setter stores the configured host", Module.GetHostName(), MString("127.0.0.1")) && Passed;
  Passed = Evaluate("SetHostPort()/GetHostPort()", "representative port", "The host-port setter stores the configured port", Module.GetHostPort(), RepresentativePort) && Passed;

  MModuleTransmitterRealta* Clone = Module.Clone();
  Passed = EvaluateTrue("Clone()", "transmitter", "Clone returns a distinct Realta transmitter instance", Clone != nullptr && Clone != &Module) && Passed;
  Passed = Evaluate("Clone()", "transmitter name", "The cloned Realta transmitter has the same representative module name", Clone->GetName(), MString("Transmitter for fully calibrated events to Realta")) && Passed;
  delete Clone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleTransmitterRealta Module;
  MXmlNode Node(nullptr, "XmlTagTransmitterRealta");
  new MXmlNode(&Node, "HostName", MString("127.0.0.1"));
  const int ConfiguredPort = 24602;
  new MXmlNode(&Node, "HostPort", ConfiguredPort);
  Passed = EvaluateTrue("ReadXmlConfiguration()", "HostName and HostPort nodes", "The Realta transmitter reads host settings from XML", Module.ReadXmlConfiguration(&Node)) && Passed;
  Passed = Evaluate("GetHostName()", "after XML read", "The XML reader stores the configured host name", Module.GetHostName(), MString("127.0.0.1")) && Passed;
  Passed = Evaluate("GetHostPort()", "after XML read", "The XML reader stores the configured host port", Module.GetHostPort(), ConfiguredPort) && Passed;

  MXmlNode EmptyNode(nullptr, "XmlTagTransmitterRealta");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "missing nodes", "The Realta transmitter accepts XML without optional host nodes", Module.ReadXmlConfiguration(&EmptyNode)) && Passed;
  Passed = Evaluate("GetHostName()", "after missing nodes", "Missing XML nodes leave the existing host name unchanged", Module.GetHostName(), MString("127.0.0.1")) && Passed;
  Passed = Evaluate("GetHostPort()", "after missing nodes", "Missing XML nodes leave the existing host port unchanged", Module.GetHostPort(), ConfiguredPort) && Passed;

  Module.SetHostName("realta.example");
  Module.SetHostPort(23456);
  MXmlNode* WrittenNode = Module.CreateXmlConfiguration();
  Passed = Evaluate("CreateXmlConfiguration()", "node name", "The Realta transmitter XML writer creates a node with the transmitter XML tag", WrittenNode->GetName(), MString("XmlTagTransmitterRealta")) && Passed;
  Passed = EvaluateSize("CreateXmlConfiguration()", "child nodes", "The Realta transmitter XML writer creates HostName and HostPort children", WrittenNode->GetNNodes(), static_cast<size_t>(2)) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "HostName child name", "The first XML child stores the HostName tag", WrittenNode->GetNode(0)->GetName(), MString("HostName")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "HostName child value", "The first XML child stores the configured host name", WrittenNode->GetNode(0)->GetValue(), MString("realta.example")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "HostPort child name", "The second XML child stores the HostPort tag", WrittenNode->GetNode(1)->GetName(), MString("HostPort")) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "HostPort child value", "The second XML child stores the configured host port", WrittenNode->GetNode(1)->GetValueAsInt(), 23456) && Passed;
  delete WrittenNode;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::TestTransmissionLifecycle()
{
  bool Passed = true;

  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;

  MModuleTransmitterRealta Module;
  Module.SetHostName("localhost");
  Module.SetHostPort(GetFreePort());
  Passed = EvaluateTrue("Initialize()", "local server", "Initialization starts the local raw-event transceiver server", Module.Initialize()) && Passed;

  MReadOutAssembly* Event = CreateRepresentativeEvent(101);
  Passed = EvaluateTrue("AnalyzeEvent()", "representative event", "The Realta transmitter accepts and queues a representative event", Module.AnalyzeEvent(Event)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event-transmitter progress", "The transmitted event is marked with event-transmitter progress", Event->HasAnalysisProgress(MAssembly::c_EventTransmitter)) && Passed;
  Module.Finalize();
  delete Event;

  Module.SetHostPort(GetFreePort());
  Passed = EvaluateTrue("Initialize()", "reinitialize", "The Realta transmitter can initialize again after finalization deletes the previous transceiver", Module.Initialize()) && Passed;
  MReadOutAssembly* ReinitializedEvent = CreateRepresentativeEvent(202);
  Passed = EvaluateTrue("AnalyzeEvent()", "after reinitialize", "The Realta transmitter accepts events after reinitialization", Module.AnalyzeEvent(ReinitializedEvent)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "after reinitialize progress", "Reinitialized transmission marks event-transmitter progress", ReinitializedEvent->HasAnalysisProgress(MAssembly::c_EventTransmitter)) && Passed;
  Module.Finalize();
  delete ReinitializedEvent;

  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::TestInheritedFlow()
{
  bool Passed = true;

  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;

  MModuleTransmitterRealta FlowModule;
  FlowModule.SetHostName("localhost");
  FlowModule.SetHostPort(GetFreePort());
  Passed = EvaluateTrue("Initialize()", "flow transmitter", "Initialization succeeds before inherited flow transmission", FlowModule.Initialize()) && Passed;

  MReadOutAssembly* FlowEvent = CreateRepresentativeEvent(303);
  FlowEvent->SetAnalysisProgress(MAssembly::c_EnergyCalibration);
  FlowEvent->SetAnalysisProgress(MAssembly::c_PositionDetermiation);
  Passed = EvaluateTrue("AddReadOutAssembly()", "flow event", "The inherited input queue accepts a fully calibrated event to transmit", FlowModule.AddReadOutAssembly(FlowEvent)) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "flow event", "The inherited analysis flow transmits one queued event", FlowModule.DoSingleAnalysis()) && Passed;
  Passed = Evaluate("GetNumberOfAnalyzedEvents()", "flow event", "One transmitted event passed through the inherited analysis flow", FlowModule.GetNumberOfAnalyzedEvents(), 1L) && Passed;
  Passed = EvaluateTrue("HasAnalyzedReadOutAssemblies()", "flow event", "The inherited analysis flow queues the transmitted event for downstream modules", FlowModule.HasAnalyzedReadOutAssemblies()) && Passed;
  MReadOutAssembly* TransmittedEvent = FlowModule.GetAnalyzedReadOutAssembly();
  Passed = EvaluateTrue("GetAnalyzedReadOutAssembly()", "flow event", "The transmitted event remains available downstream", TransmittedEvent == FlowEvent) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "flow event progress", "The transmitted event receives event-transmitter progress", TransmittedEvent->HasAnalysisProgress(MAssembly::c_EventTransmitter)) && Passed;
  delete TransmittedEvent;
  FlowModule.Finalize();

  MModuleTransmitterRealta MissingRequirementModule;
  MissingRequirementModule.SetHostName("localhost");
  MissingRequirementModule.SetHostPort(GetFreePort());
  Passed = EvaluateTrue("Initialize()", "missing predecessor progress", "Initialization succeeds before testing inherited requirement filtering", MissingRequirementModule.Initialize()) && Passed;
  MReadOutAssembly* MissingRequirementEvent = CreateRepresentativeEvent(404);
  MissingRequirementEvent->SetAnalysisProgress(MAssembly::c_EnergyCalibration);
  Passed = EvaluateTrue("AddReadOutAssembly()", "missing predecessor progress", "The inherited queue accepts an event missing position-determination progress", MissingRequirementModule.AddReadOutAssembly(MissingRequirementEvent)) && Passed;
  Passed = EvaluateTrue("DoSingleAnalysis()", "missing predecessor progress", "Events missing predecessor progress still pass through inherited queues", MissingRequirementModule.DoSingleAnalysis()) && Passed;
  MReadOutAssembly* UntransmittedEvent = MissingRequirementModule.GetAnalyzedReadOutAssembly();
  Passed = EvaluateFalse("AnalyzeEvent()", "missing predecessor progress", "The Realta transmitter does not mark events missing required predecessor progress", UntransmittedEvent->HasAnalysisProgress(MAssembly::c_EventTransmitter)) && Passed;
  delete UntransmittedEvent;
  MissingRequirementModule.Finalize();

  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::WaitForConnected(MTransceiverTcpIp& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.IsConnected() == true) return true;
    gSystem->Sleep(1);
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::WaitForMessage(MTransceiverTcpIp& Transceiver, MString& Message, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.Receive(Message) == true) return true;
    gSystem->Sleep(1);
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTModuleTransmitterRealta::TestTransmissionRoundTrip()
{
  bool Passed = true;

  const int PreviousErrorIgnoreLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;

  const int Port = GetFreePort();

  // The module owns the server side of the connection, so pair it with a client transceiver on the
  // same port and mode. This follows the in-process server/client pattern of UTTransceiverTcpIp.
  MModuleTransmitterRealta Module;
  Module.SetHostName("localhost");
  Module.SetHostPort(Port);
  Passed = EvaluateTrue("Initialize()", "round-trip server", "Initialization starts the transmitter server for the round trip", Module.Initialize()) && Passed;

  MTransceiverTcpIp Client("UTModuleTransmitterRealtaClient", "localhost", Port, MTransceiverTcpIp::c_ModeRawEventList);
  Client.SetVerbosity(1);
  Client.RequestClient(true);
  Client.Connect(false);

  const bool ClientConnected = WaitForConnected(Client, c_ConnectTimeOut);
  Passed = EvaluateTrue("Connect()", "round-trip client", "A client connects to the port the transmitter module serves", ClientConnected) && Passed;

  if (ClientConnected == true) {
    Passed = EvaluateFalse("IsServer()", "round-trip client", "The connecting side is the client, not the server", Client.IsServer()) && Passed;

    MReadOutAssembly* Event = CreateRepresentativeEvent(4242);
    Passed = EvaluateTrue("AnalyzeEvent()", "round-trip event", "The transmitter accepts the event to be transmitted", Module.AnalyzeEvent(Event)) && Passed;

    MString Message;
    const bool Received = WaitForMessage(Client, Message, c_ReceiveTimeOut);
    Passed = EvaluateTrue("Send()", "round-trip event", "The transmitted event reaches the connected client", Received) && Passed;
    if (Received == true) {
      Passed = EvaluateTrue("StreamEvta()", "round-trip event", "The received message carries the event header of the transmitted event", MString(Message).Contains("SE")) && Passed;
      Passed = EvaluateTrue("StreamEvta()", "round-trip event ID", "The received message carries the ID of the transmitted event", MString(Message).Contains("ID 4242")) && Passed;
    }
    delete Event;
  }

  // Tearing down one side makes the other notice the drop, which MTransceiverTcpIp reports through
  // an ungated cout (MTransceiverTcpIp.cxx:550) that DisableDefaultStreams cannot reach. No
  // assertion runs here, so capturing the raw stream over the teardown is safe.
  ostringstream TeardownOutput;
  streambuf* OriginalCout = cout.rdbuf(TeardownOutput.rdbuf());
  Client.Disconnect(true, 1.0);
  Module.Finalize();
  cout.rdbuf(OriginalCout);

  gErrorIgnoreLevel = PreviousErrorIgnoreLevel;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTModuleTransmitterRealta Test;
  return Test.Run() == true ? 0 : 1;
}
