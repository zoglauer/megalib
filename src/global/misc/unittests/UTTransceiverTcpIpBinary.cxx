/*
 * UTTransceiverTcpIpBinary.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <cstring>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MTimer.h"
#define private public
#include "MTransceiverTcpIpBinary.h"
#undef private
#include "MUnitTest.h"


//! Build a representative packet.
static vector<unsigned char> BuildPacket(unsigned char Start, unsigned int Count)
{
  vector<unsigned char> Packet;
  for (unsigned int i = 0; i < Count; ++i) {
    Packet.push_back(static_cast<unsigned char>(Start + i));
  }
  return Packet;
}


//! Compare two packets for exact equality.
static bool PacketsMatch(const vector<unsigned char>& A, const vector<unsigned char>& B)
{
  return A == B;
}


//! Wait until the transceiver reports a connected state.
static bool WaitForConnected(MTransceiverTcpIpBinary& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.IsConnected() == true) {
      return true;
    }
    gSystem->Sleep(1);
  }

  return false;
}


//! Wait until the send queue has drained or the timeout expires.
static bool WaitForSendQueueEmpty(MTransceiverTcpIpBinary& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.GetNPacketsToSend() == 0) {
      return true;
    }
    gSystem->Sleep(1);
  }

  return false;
}


//! Wait until the receive queue has at least the requested number of bytes.
static bool WaitForReceiveBytes(MTransceiverTcpIpBinary& Transceiver, unsigned long Bytes, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.GetNPacketsToReceive() >= Bytes) {
      return true;
    }
    gSystem->Sleep(1);
  }

  return false;
}


//! Wait until the transceiver reports at least one reset.
static bool WaitForReset(MTransceiverTcpIpBinary& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.GetNResets() > 0) {
      return true;
    }
    gSystem->Sleep(1);
  }

  return false;
}


//! Fill the private receive queue for direct receive-path tests.
static void FillReceiveQueue(MTransceiverTcpIpBinary& Transceiver, const vector<unsigned char>& Bytes)
{
  Transceiver.m_ReceiveMutex.Lock();
  Transceiver.m_PacketsToReceive.clear();
  Transceiver.m_PacketsToReceive.insert(Transceiver.m_PacketsToReceive.end(), Bytes.begin(), Bytes.end());
  Transceiver.m_NPacketsToReceive = Bytes.size();
  Transceiver.m_ReceiveMutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


class UTTransceiverTcpIpBinary : public MUnitTest
{
public:
  UTTransceiverTcpIpBinary() : MUnitTest("UTTransceiverTcpIpBinary") {}
  virtual ~UTTransceiverTcpIpBinary() {}

  virtual bool Run();
};


////////////////////////////////////////////////////////////////////////////////


bool UTTransceiverTcpIpBinary::Run()
{
  bool Passed = true;
  const double LiveTimeOut = 0.5;
  const double FastTimeOut = 0.1;
  const double FailureTimeOut = 0.05;

  gROOT->SetBatch(true);

  {
    MTransceiverTcpIpBinary Default;
    Default.SetVerbosity(0);
    Passed = Evaluate("Default ctor", "name", "The default binary transceiver name is set", Default.GetName(), MString("A TCP/IP transceiver in binary mode")) && Passed;
    Passed = Evaluate("Default ctor", "host", "The default binary transceiver host is set", Default.GetHost(), MString("localhost")) && Passed;
    Passed = Evaluate("Default ctor", "port", "The default binary transceiver port is set", Default.GetPort(), 9090u) && Passed;
    Passed = EvaluateFalse("Default ctor", "connected", "The default binary transceiver starts disconnected", Default.IsConnected()) && Passed;
    Passed = Evaluate("Default ctor", "send queue", "The default binary transceiver starts with an empty send queue", Default.GetNPacketsToSend(), 0ul) && Passed;
    Passed = Evaluate("Default ctor", "receive queue", "The default binary transceiver starts with an empty receive queue", Default.GetNPacketsToReceive(), 0ul) && Passed;
    Default.SetName("Renamed");
    Default.SetHost("example.org");
    Default.SetPort(12345);
    Passed = Evaluate("SetName()", "name", "The binary transceiver name setter works", Default.GetName(), MString("Renamed")) && Passed;
    Passed = Evaluate("SetHost()", "host", "The binary transceiver host setter works", Default.GetHost(), MString("example.org")) && Passed;
    Passed = Evaluate("SetPort()", "port", "The binary transceiver port setter works", Default.GetPort(), 12345u) && Passed;
    Passed = EvaluateTrue("AutomaticReconnection()", "default", "Automatic reconnection is enabled by default", Default.GetAutomaticReconnection()) && Passed;
    Passed = EvaluateTrue("Default ctor", "wish client", "The default binary transceiver wishes to connect as a client by default", Default.m_WishClient.load()) && Passed;
    Passed = EvaluateTrue("Default ctor", "wish server", "The default binary transceiver wishes to connect as a server by default", Default.m_WishServer.load()) && Passed;
    Default.AutomaticReconnection(false);
    Passed = EvaluateFalse("AutomaticReconnection()", "setter", "Automatic reconnection can be disabled", Default.GetAutomaticReconnection()) && Passed;
    Default.SetHost("localhost");
    Default.SetPort(63001);
    Passed = EvaluateTrue("Connect()", "no-wait", "Connect(false) starts cleanly even with automatic reconnection disabled", Default.Connect(false)) && Passed;
    Passed = EvaluateFalse("AutomaticReconnection()", "connect preserves", "Connect(false) must not re-enable automatic reconnection", Default.GetAutomaticReconnection()) && Passed;
    Passed = EvaluateTrue("Disconnect()", "cleanup", "Disconnect(false) stops the default transceiver cleanly", Default.Disconnect(false)) && Passed;
    Default.SetMaximumBufferSize(4);
    vector<unsigned char> PacketA = BuildPacket(0x10, 3);
    vector<unsigned char> PacketB = BuildPacket(0x20, 3);
    Passed = EvaluateTrue("SetMaximumBufferSize()", "first send", "A packet can be queued before the buffer limit is reached", Default.Send(PacketA)) && Passed;
    Passed = EvaluateTrue("SetMaximumBufferSize()", "second send", "A second packet can be queued and oldest data is dropped when the byte limit is exceeded", Default.Send(PacketB)) && Passed;
    Passed = Evaluate("SetMaximumBufferSize()", "queue size", "The send queue keeps only the newest packet when the maximum buffer size is small", Default.GetNPacketsToSend(), 1ul) && Passed;
    vector<unsigned char> EmptyPacket;
    Passed = EvaluateTrue("Send()", "empty packet", "An empty packet can be queued without crashing", Default.Send(EmptyPacket)) && Passed;
    Passed = Evaluate("Send()", "empty packet queue", "An empty packet is counted in the send queue before the worker drains it", Default.GetNPacketsToSend(), 2ul) && Passed;
    Passed = EvaluateTrue("Disconnect()", "send queue cleanup", "Disconnect(false) clears pending send packets", Default.Disconnect(false)) && Passed;
    Passed = Evaluate("Disconnect()", "send queue size", "The pending send queue is empty after Disconnect(false)", Default.GetNPacketsToSend(), 0ul) && Passed;
    Passed = Evaluate("Disconnect()", "receive queue size", "The pending receive queue is empty after Disconnect(false)", Default.GetNPacketsToReceive(), 0ul) && Passed;
    Passed = Evaluate("Disconnect()", "reset counter", "A clean Disconnect(false) does not count as a reset", Default.GetNResets(), 0ul) && Passed;
    Passed = EvaluateTrue("Connect()", "restart", "Connect(false) can be called again after Disconnect(false)", Default.Connect(false)) && Passed;
    Passed = EvaluateTrue("Disconnect()", "restart cleanup", "Disconnect(false) still succeeds after a restart", Default.Disconnect(false)) && Passed;
    Default.ClearBuffers();
    Passed = Evaluate("ClearBuffers()", "queue size", "ClearBuffers empties the queued packets", Default.GetNPacketsToSend(), 0ul) && Passed;
    Passed = Evaluate("c_ReadPacketSize", "value", "The named binary receive chunk size is pinned", MTransceiverTcpIpBinary::c_ReadPacketSize, 1024*1024) && Passed;
  }

  {
    MTransceiverTcpIpBinary Synced("Synced", "localhost", 63020);
    Synced.SetVerbosity(0);
    vector<unsigned char> Sync = BuildPacket(0xA0, 2);
    vector<unsigned char> Buffer;
    Buffer.push_back(0x01);
    Buffer.insert(Buffer.end(), Sync.begin(), Sync.end());
    Buffer.push_back(0x10);
    Buffer.push_back(0x11);
    Buffer.insert(Buffer.end(), Sync.begin(), Sync.end());
    Buffer.push_back(0x20);
    FillReceiveQueue(Synced, Buffer);

    vector<unsigned char> Packet = BuildPacket(0xF0, 3);
    Passed = EvaluateTrue("SyncedReceive()", "success", "SyncedReceive finds a packet between two sync words", Synced.SyncedReceive(Packet, Sync)) && Passed;
    vector<unsigned char> Expected;
    Expected.push_back(0xA0);
    Expected.push_back(0xA1);
    Expected.push_back(0x10);
    Expected.push_back(0x11);
    Passed = EvaluateTrue("SyncedReceive()", "payload", "SyncedReceive replaces a reused output vector with the extracted payload", PacketsMatch(Packet, Expected)) && Passed;
    Passed = Evaluate("SyncedReceive()", "remaining bytes", "SyncedReceive removes the extracted bytes from the receive queue", Synced.GetNPacketsToReceive(), 4ul) && Passed;

    vector<unsigned char> Previous = Packet;
    vector<unsigned char> MissingSync = BuildPacket(0xB0, 2);
    Passed = EvaluateFalse("SyncedReceive()", "missing sync", "SyncedReceive fails when the sync word is absent", Synced.SyncedReceive(Packet, MissingSync)) && Passed;
    Passed = EvaluateTrue("SyncedReceive()", "failure output", "SyncedReceive leaves the output vector unchanged on failure", PacketsMatch(Packet, Previous)) && Passed;

    vector<unsigned char> EmptySync;
    Passed = EvaluateFalse("SyncedReceive()", "empty sync", "SyncedReceive rejects an empty sync word", Synced.SyncedReceive(Packet, EmptySync)) && Passed;
    Passed = EvaluateTrue("SyncedReceive()", "empty sync output", "SyncedReceive leaves the output vector unchanged when the sync word is empty", PacketsMatch(Packet, Previous)) && Passed;

    vector<unsigned char> OneSyncBuffer;
    OneSyncBuffer.push_back(0x01);
    OneSyncBuffer.insert(OneSyncBuffer.end(), Sync.begin(), Sync.end());
    OneSyncBuffer.push_back(0x30);
    FillReceiveQueue(Synced, OneSyncBuffer);
    Passed = EvaluateFalse("SyncedReceive()", "single sync", "SyncedReceive fails when there is no closing sync word", Synced.SyncedReceive(Packet, Sync)) && Passed;
    Passed = EvaluateTrue("SyncedReceive()", "single sync output", "SyncedReceive leaves the output vector unchanged without a closing sync word", PacketsMatch(Packet, Previous)) && Passed;

    vector<unsigned char> MultiSyncBuffer;
    MultiSyncBuffer.insert(MultiSyncBuffer.end(), Sync.begin(), Sync.end());
    MultiSyncBuffer.push_back(0x40);
    MultiSyncBuffer.insert(MultiSyncBuffer.end(), Sync.begin(), Sync.end());
    MultiSyncBuffer.push_back(0x41);
    MultiSyncBuffer.insert(MultiSyncBuffer.end(), Sync.begin(), Sync.end());
    MultiSyncBuffer.push_back(0x42);
    FillReceiveQueue(Synced, MultiSyncBuffer);
    Passed = EvaluateTrue("SyncedReceive()", "max packets", "SyncedReceive honors the MaxPackets sync-search limit", Synced.SyncedReceive(Packet, Sync, 1)) && Passed;
    vector<unsigned char> ExpectedMaxPackets;
    ExpectedMaxPackets.push_back(0xA0);
    ExpectedMaxPackets.push_back(0xA1);
    ExpectedMaxPackets.push_back(0x40);
    Passed = EvaluateTrue("SyncedReceive()", "max packets payload", "SyncedReceive stops at the first closing sync when MaxPackets is one", PacketsMatch(Packet, ExpectedMaxPackets)) && Passed;
  }

  {
    MTransceiverTcpIpBinary Receiver("Receiver", "localhost", 63030);
    Receiver.SetVerbosity(0);
    vector<unsigned char> Payload = BuildPacket(0x50, 5);
    FillReceiveQueue(Receiver, Payload);
    vector<unsigned char> Received = BuildPacket(0xF0, 2);
    Passed = EvaluateTrue("Receive()", "payload", "Receive returns queued payload bytes", Receiver.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Receive()", "payload contents", "Receive replaces the output vector with the queued payload", PacketsMatch(Received, Payload)) && Passed;
    Passed = Evaluate("Receive()", "queue size", "Receive clears the receive queue", Receiver.GetNPacketsToReceive(), 0ul) && Passed;
    Passed = EvaluateFalse("Receive()", "empty queue", "Receive returns false when no bytes are queued", Receiver.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Receive()", "empty output", "Receive leaves the output vector unchanged when no bytes are queued", PacketsMatch(Received, Payload)) && Passed;
  }

  {
    const unsigned int LivePort = 63120;
    MTransceiverTcpIpBinary Server("LiveServer", "localhost", LivePort);
    MTransceiverTcpIpBinary Client("LiveClient", "localhost", LivePort);
    Server.SetVerbosity(0);
    Client.SetVerbosity(0);
    Server.RequestServer(true);
    Client.RequestClient(true);

    Passed = EvaluateTrue("Live loopback", "server start", "The binary server starts connecting", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client connect", "The binary client connects to the server", Client.Connect(true, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server connected", "The binary server reaches connected state", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client connected", "The binary client reaches connected state", WaitForConnected(Client, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server role", "The binary server is marked as server", Server.m_IsServer.load()) && Passed;
    Passed = EvaluateFalse("Live loopback", "client role", "The binary client is not marked as server", Client.m_IsServer.load()) && Passed;

    vector<unsigned char> ClientToServer = BuildPacket(0x70, 8);
    Passed = EvaluateTrue("Live loopback", "client send", "The binary client queues a packet for the server", Client.Send(ClientToServer)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client queue drained", "The binary client send queue drains", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server receive wait", "The binary server receives bytes", WaitForReceiveBytes(Server, ClientToServer.size(), FastTimeOut)) && Passed;
    vector<unsigned char> Received;
    Passed = EvaluateTrue("Live loopback", "server receive", "The binary server reads the received packet", Server.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server payload", "The binary server receives the expected packet", PacketsMatch(Received, ClientToServer)) && Passed;
    Passed = Evaluate("Live loopback", "client sent bytes", "The binary client sent-byte counter is updated", Client.GetNSentBytes(), static_cast<unsigned long>(ClientToServer.size())) && Passed;
    Passed = Evaluate("Live loopback", "server received bytes", "The binary server received-byte counter is updated", Server.GetNReceivedBytes(), static_cast<unsigned long>(ClientToServer.size())) && Passed;

    vector<unsigned char> LiveSync = BuildPacket(0xC0, 2);
    vector<unsigned char> LiveSyncedPayload;
    LiveSyncedPayload.insert(LiveSyncedPayload.end(), LiveSync.begin(), LiveSync.end());
    LiveSyncedPayload.push_back(0xC2);
    LiveSyncedPayload.push_back(0xC3);
    LiveSyncedPayload.insert(LiveSyncedPayload.end(), LiveSync.begin(), LiveSync.end());
    Passed = EvaluateTrue("Live loopback", "synced send", "The binary client sends a sync-delimited byte stream", Client.Send(LiveSyncedPayload)) && Passed;
    Passed = EvaluateTrue("Live loopback", "synced queue drained", "The binary client sync send queue drains", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "synced receive wait", "The binary server receives the sync-delimited byte stream", WaitForReceiveBytes(Server, LiveSyncedPayload.size(), FastTimeOut)) && Passed;
    vector<unsigned char> LiveSyncedReceived = BuildPacket(0xF0, 2);
    Passed = EvaluateTrue("Live loopback", "synced receive", "The binary server extracts a live sync-delimited packet", Server.SyncedReceive(LiveSyncedReceived, LiveSync)) && Passed;
    vector<unsigned char> ExpectedLiveSynced;
    ExpectedLiveSynced.push_back(0xC0);
    ExpectedLiveSynced.push_back(0xC1);
    ExpectedLiveSynced.push_back(0xC2);
    ExpectedLiveSynced.push_back(0xC3);
    Passed = EvaluateTrue("Live loopback", "synced payload", "The binary server receives the expected live synced payload", PacketsMatch(LiveSyncedReceived, ExpectedLiveSynced)) && Passed;

    vector<unsigned char> ServerToClient = BuildPacket(0x80, 6);
    Passed = EvaluateTrue("Live loopback", "server send", "The binary server queues a packet for the client", Server.Send(ServerToClient)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server queue drained", "The binary server send queue drains", WaitForSendQueueEmpty(Server, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client receive wait", "The binary client receives bytes", WaitForReceiveBytes(Client, ServerToClient.size(), FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client receive", "The binary client reads the received packet", Client.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client payload", "The binary client receives the expected packet", PacketsMatch(Received, ServerToClient)) && Passed;
    Passed = Evaluate("Live loopback", "server sent bytes", "The binary server sent-byte counter is updated", Server.GetNSentBytes(), static_cast<unsigned long>(ServerToClient.size())) && Passed;
    Passed = Evaluate("Live loopback", "client received bytes", "The binary client received-byte counter is updated", Client.GetNReceivedBytes(), static_cast<unsigned long>(ServerToClient.size())) && Passed;

    vector<unsigned char> EmptyPacket;
    Passed = EvaluateTrue("Live loopback", "empty send", "The binary client queues an empty packet while connected", Client.Send(EmptyPacket)) && Passed;
    Passed = EvaluateTrue("Live loopback", "empty queue drained", "The binary worker drains an empty packet without sending bytes", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = Evaluate("Live loopback", "empty sent bytes", "The empty packet does not change the sent-byte counter", Client.GetNSentBytes(), static_cast<unsigned long>(ClientToServer.size() + LiveSyncedPayload.size())) && Passed;

    Passed = Evaluate("Live loopback", "client resets", "The binary client has no resets before clean disconnect", Client.GetNResets(), 0ul) && Passed;
    Passed = Evaluate("Live loopback", "server resets", "The binary server has no resets before clean disconnect", Server.GetNResets(), 0ul) && Passed;
    Passed = EvaluateTrue("Live loopback", "client disconnect", "The binary client disconnects cleanly", Client.Disconnect(false, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server disconnect", "The binary server disconnects cleanly", Server.Disconnect(false, FastTimeOut)) && Passed;

    Passed = EvaluateTrue("Live loopback", "server reconnect start", "The binary server reconnects after a clean disconnect", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client reconnect", "The binary client reconnects after a clean disconnect", Client.Connect(true, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "server reconnected", "The binary server reports connected after reconnect", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "client reconnected", "The binary client reports connected after reconnect", WaitForConnected(Client, LiveTimeOut)) && Passed;
    vector<unsigned char> ReconnectPacket = BuildPacket(0x90, 4);
    Passed = EvaluateTrue("Live loopback", "reconnect send", "The binary client sends after reconnect", Client.Send(ReconnectPacket)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect queue drained", "The binary client send queue drains after reconnect", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect receive wait", "The binary server receives after reconnect", WaitForReceiveBytes(Server, ReconnectPacket.size(), FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect receive", "The binary server reads the reconnect packet", Server.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect payload", "The binary server receives the expected reconnect packet", PacketsMatch(Received, ReconnectPacket)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect client disconnect", "The binary client disconnects after reconnect", Client.Disconnect(false, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live loopback", "reconnect server disconnect", "The binary server disconnects after reconnect", Server.Disconnect(false, FastTimeOut)) && Passed;
  }

  {
    const unsigned int FailurePort = 63000;
    MTransceiverTcpIpBinary Failure("Failure", "localhost", FailurePort);
    Failure.SetVerbosity(0);
    Failure.RequestClient(true);
    Passed = EvaluateFalse("Connect()", "failure", "Connect(true) fails on an unused port", Failure.Connect(true, FailureTimeOut)) && Passed;
    Passed = EvaluateFalse("IsConnected()", "failure", "The binary transceiver remains disconnected after a failed connect", Failure.IsConnected()) && Passed;
    Passed = EvaluateFalse("IsConnectionWished()", "failure", "A failed connect clears the desired connection state", Failure.IsConnectionWished()) && Passed;
    Passed = Evaluate("Connect()", "failure resets", "A failed connect attempt does not count as a reset", Failure.GetNResets(), 0ul) && Passed;
  }

  {
    const unsigned int AutoReconnectPort = 63141;
    MTransceiverTcpIpBinary Server("AutoReconnectServer", "localhost", AutoReconnectPort);
    MTransceiverTcpIpBinary Client("AutoReconnectClient", "localhost", AutoReconnectPort);
    Server.SetVerbosity(0);
    Client.SetVerbosity(0);
    Server.RequestServer(true);
    Client.RequestClient(true);

    Passed = EvaluateTrue("Auto reconnect", "server start", "The binary auto-reconnect server starts connecting", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client connect", "The binary auto-reconnect client connects", Client.Connect(true, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server connected", "The binary auto-reconnect server reaches connected state", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client connected", "The binary auto-reconnect client reaches connected state", WaitForConnected(Client, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server disconnect", "The binary auto-reconnect server disconnects to force a reconnect", Server.Disconnect(false, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client reset observed", "The binary auto-reconnect client observes the peer loss", WaitForReset(Client, LiveTimeOut)) && Passed;
    vector<unsigned char> AutoReconnectPacket = BuildPacket(0xE0, 4);
    Passed = EvaluateTrue("Auto reconnect", "client send queued", "The binary auto-reconnect client accepts a packet while the peer is down", Client.Send(AutoReconnectPacket)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client queue pending", "The binary auto-reconnect client keeps the packet queued while the peer is down", Client.GetNPacketsToSend() > 0) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server restart", "The binary auto-reconnect server comes back up", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client reconnects", "The binary auto-reconnect client reconnects automatically after peer loss", WaitForConnected(Client, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server reconnects", "The binary auto-reconnect server reaches connected state again", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client connected", "The binary auto-reconnect client remains connected after reconnect", Client.IsConnected()) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client queue drained", "The binary auto-reconnect client sends the queued packet after reconnect", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server receive wait", "The binary auto-reconnect server receives the packet after reconnect", WaitForReceiveBytes(Server, AutoReconnectPacket.size(), FastTimeOut)) && Passed;
    vector<unsigned char> Received;
    Passed = EvaluateTrue("Auto reconnect", "server receive", "The binary auto-reconnect server reads the packet after reconnect", Server.Receive(Received)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "payload", "The binary auto-reconnect server receives the expected packet after reconnect", PacketsMatch(Received, AutoReconnectPacket)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "client disconnect", "The binary auto-reconnect client disconnects cleanly after reconnect", Client.Disconnect(false, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Auto reconnect", "server disconnect", "The binary auto-reconnect server disconnects cleanly after reconnect", Server.Disconnect(false, FastTimeOut)) && Passed;
  }

  {
    const unsigned int OverflowPort = 63150;
    MTransceiverTcpIpBinary Server("OverflowServer", "localhost", OverflowPort);
    MTransceiverTcpIpBinary Client("OverflowClient", "localhost", OverflowPort);
    Server.SetVerbosity(0);
    Client.SetVerbosity(0);
    Server.SetMaximumBufferSize(5);
    Server.RequestServer(true);
    Client.RequestClient(true);

    Passed = EvaluateTrue("Live overflow", "server start", "The binary overflow server starts connecting", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Live overflow", "client connect", "The binary overflow client connects", Client.Connect(true, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow", "server connected", "The binary overflow server reaches connected state", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow", "client connected", "The binary overflow client reaches connected state", WaitForConnected(Client, LiveTimeOut)) && Passed;
    vector<unsigned char> OverflowPacket = BuildPacket(0xB0, 8);
    Passed = EvaluateTrue("Live overflow", "client send", "The binary overflow client sends more bytes than the receive buffer limit", Client.Send(OverflowPacket)) && Passed;
    Passed = EvaluateTrue("Live overflow", "client queue drained", "The binary overflow client send queue drains", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow", "server receive wait", "The binary overflow server receives capped bytes", WaitForReceiveBytes(Server, 5, FastTimeOut)) && Passed;
    Passed = Evaluate("Live overflow", "server receive queue", "The binary overflow server receive queue is capped at the maximum buffer size", Server.GetNPacketsToReceive(), 5ul) && Passed;
    vector<unsigned char> Received;
    Passed = EvaluateTrue("Live overflow", "server receive", "The binary overflow server reads the capped payload", Server.Receive(Received)) && Passed;
    vector<unsigned char> ExpectedOverflow;
    for (unsigned int i = 3; i < OverflowPacket.size(); ++i) {
      ExpectedOverflow.push_back(OverflowPacket[i]);
    }
    Passed = EvaluateTrue("Live overflow", "payload", "The binary overflow server keeps the newest bytes", PacketsMatch(Received, ExpectedOverflow)) && Passed;
    Passed = EvaluateTrue("Live overflow", "client disconnect", "The binary overflow client disconnects cleanly", Client.Disconnect(false, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow", "server disconnect", "The binary overflow server disconnects cleanly", Server.Disconnect(false, FastTimeOut)) && Passed;
  }

  {
    const unsigned int OverflowExistingPort = 63160;
    MTransceiverTcpIpBinary Server("OverflowExistingServer", "localhost", OverflowExistingPort);
    MTransceiverTcpIpBinary Client("OverflowExistingClient", "localhost", OverflowExistingPort);
    Server.SetVerbosity(0);
    Client.SetVerbosity(0);
    Server.SetMaximumBufferSize(6);
    Server.RequestServer(true);
    Client.RequestClient(true);

    Passed = EvaluateTrue("Live overflow existing", "server start", "The binary overflow-existing server starts connecting", Server.Connect(false)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "client connect", "The binary overflow-existing client connects", Client.Connect(true, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "server connected", "The binary overflow-existing server reaches connected state", WaitForConnected(Server, LiveTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "client connected", "The binary overflow-existing client reaches connected state", WaitForConnected(Client, LiveTimeOut)) && Passed;

    vector<unsigned char> FirstPacket = BuildPacket(0xC0, 4);
    vector<unsigned char> SecondPacket = BuildPacket(0xD0, 4);
    Passed = EvaluateTrue("Live overflow existing", "first send", "The binary overflow-existing client sends the first packet", Client.Send(FirstPacket)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "first queue drained", "The binary overflow-existing first send queue drains", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "first receive wait", "The binary overflow-existing server receives the first packet", WaitForReceiveBytes(Server, FirstPacket.size(), FastTimeOut)) && Passed;
    Passed = Evaluate("Live overflow existing", "first queue size", "The binary overflow-existing server buffered the first packet", Server.GetNPacketsToReceive(), static_cast<unsigned long>(FirstPacket.size())) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "second send", "The binary overflow-existing client sends a second packet that exceeds the buffer", Client.Send(SecondPacket)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "second queue drained", "The binary overflow-existing second send queue drains", WaitForSendQueueEmpty(Client, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "second receive wait", "The binary overflow-existing server receives capped bytes", WaitForReceiveBytes(Server, 6, FastTimeOut)) && Passed;
    Passed = Evaluate("Live overflow existing", "server receive queue", "The binary overflow-existing server receive queue is capped", Server.GetNPacketsToReceive(), 6ul) && Passed;
    vector<unsigned char> Received;
    Passed = EvaluateTrue("Live overflow existing", "server receive", "The binary overflow-existing server reads the capped payload", Server.Receive(Received)) && Passed;
    vector<unsigned char> ExpectedExistingOverflow;
    ExpectedExistingOverflow.push_back(FirstPacket[2]);
    ExpectedExistingOverflow.push_back(FirstPacket[3]);
    ExpectedExistingOverflow.insert(ExpectedExistingOverflow.end(), SecondPacket.begin(), SecondPacket.end());
    Passed = EvaluateTrue("Live overflow existing", "payload", "The binary overflow-existing server keeps the newest buffered bytes", PacketsMatch(Received, ExpectedExistingOverflow)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "client disconnect", "The binary overflow-existing client disconnects cleanly", Client.Disconnect(false, FastTimeOut)) && Passed;
    Passed = EvaluateTrue("Live overflow existing", "server disconnect", "The binary overflow-existing server disconnects cleanly", Server.Disconnect(false, FastTimeOut)) && Passed;
  }

  {
    MTransceiverTcpIpBinary NoRoles("NoRoles", "localhost", 63010);
    NoRoles.SetVerbosity(0);
    NoRoles.RequestClient(false);
    NoRoles.RequestServer(false);
    Passed = EvaluateFalse("Connect()", "no roles", "Connect(true) rejects the impossible no-role state", NoRoles.Connect(true, FailureTimeOut)) && Passed;
    Passed = EvaluateFalse("IsConnected()", "no roles", "The binary transceiver stays disconnected with no roles enabled", NoRoles.IsConnected()) && Passed;
    Passed = EvaluateFalse("RequestClient()", "disabled", "RequestClient(false) disables the client role", NoRoles.m_WishClient.load()) && Passed;
    Passed = EvaluateFalse("RequestServer()", "disabled", "RequestServer(false) disables the server role", NoRoles.m_WishServer.load()) && Passed;
    NoRoles.RequestClient(true);
    Passed = EvaluateTrue("RequestClient()", "enabled", "RequestClient(true) enables the client role", NoRoles.m_WishClient.load()) && Passed;
    Passed = EvaluateFalse("RequestClient()", "server disabled", "RequestClient(true) disables the server role", NoRoles.m_WishServer.load()) && Passed;
    NoRoles.RequestServer(true);
    Passed = EvaluateTrue("RequestServer()", "enabled", "RequestServer(true) enables the server role", NoRoles.m_WishServer.load()) && Passed;
    Passed = EvaluateFalse("RequestServer()", "client disabled", "RequestServer(true) disables the client role", NoRoles.m_WishClient.load()) && Passed;
  }

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  gROOT->SetBatch(true);

  UTTransceiverTcpIpBinary Test;
  return Test.Run() == true ? 0 : 1;
}
