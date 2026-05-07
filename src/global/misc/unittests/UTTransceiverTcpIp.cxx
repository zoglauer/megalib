/*
 * UTTransceiverTcpIp.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <fcntl.h>
#include <fstream>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFileEventsTra.h"
#include "MTimer.h"
#include "MTransceiverTcpIp.h"
#include "MUnitTest.h"


static MString g_ExecutablePath;


//! Start one child process and return its PID.
static pid_t StartChildProcess(const MString& Executable, const MString& Argument, const MString& OutputFileName)
{
  pid_t Child = fork();
  if (Child < 0) {
    perror("fork");
    return Child;
  }
  if (Child == 0) {
    if (OutputFileName.IsEmpty() == false) {
      int Log = open(OutputFileName.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (Log >= 0) {
        dup2(Log, STDOUT_FILENO);
        dup2(Log, STDERR_FILENO);
        close(Log);
      }
    }

    execl(Executable.Data(), Executable.Data(), Argument.Data(), static_cast<char*>(nullptr));
    _exit(127);
  }

  return Child;
}


//! Wait until the send queue has drained or the timeout expires.
static bool WaitForSendQueueEmpty(MTransceiverTcpIp& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.GetNStringsToSend() == 0) {
      return true;
    }
    gSystem->Sleep(10);
  }

  return false;
}


//! Wait until a message is available in the receive buffer or the timeout expires.
static bool WaitForMessage(MTransceiverTcpIp& Transceiver, MString& Message, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.Receive(Message) == true) {
      return true;
    }
    gSystem->Sleep(10);
  }

  return false;
}


//! Wait until a file appears or the timeout expires.
static bool WaitForFile(const MString& FileName, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (gSystem->AccessPathName(FileName) == false) {
      return true;
    }
    gSystem->Sleep(10);
  }

  return false;
}


//! Return the path to the ready marker for a test role.
static MString GetReadyFile(unsigned int Port, const MString& Role)
{
  return MString("/tmp/UTTransceiverTcpIp_ready_") + Role + "_" + Port + ".txt";
}


//! Create a file to signal readiness.
static bool TouchFile(const MString& FileName)
{
  ofstream Out(FileName.Data());
  return Out.is_open() == true;
}


//! Wait for a child process to exit, with a timeout.
static bool WaitForChildProcess(pid_t Child, int& Status, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    pid_t Result = waitpid(Child, &Status, WNOHANG);
    if (Result == Child) {
      return true;
    }
    if (Result < 0) {
      return false;
    }
    gSystem->Sleep(10);
  }

  kill(Child, SIGKILL);
  waitpid(Child, &Status, 0);
  return false;
}


//! Return the path to a representative tra fixture.
static MString GetTraFixtureFile()
{
  return MString(gSystem->pwd()) + "/src/global/misc/unittests/data/UTFileEventsTra/ObsTime_1sec_complete.inc1.id1.tra.gz";
}


//! Build a raw event-list payload from the tra fixture.
static bool BuildRawPayload(MString& Payload, MString& ExpectedMessage)
{
  MFileEventsTra Reader;
  if (Reader.Open(GetTraFixtureFile()) == false) {
    return false;
  }

  MPhysicalEvent* Event = Reader.GetNextEvent();
  if (Event == nullptr) {
    Reader.Close();
    return false;
  }

  ExpectedMessage = Event->ToTraString();
  Payload = ExpectedMessage;
  Payload += "EN\n";

  delete Event;
  Reader.Close();
  return true;
}


//! Build a raw event-list payload that exceeds the receive-side safety limit.
static bool BuildRawOverflowPayload(MString& Payload)
{
  const size_t TargetLength = MTransceiverTcpIp::c_MaxRawMessageLength + 256;
  std::string Data(TargetLength, 'A');
  Payload = Data.c_str();
  return true;
}


//! Wait until the transceiver reports a connected state.
static bool WaitForConnected(MTransceiverTcpIp& Transceiver, double TimeOut)
{
  MTimer Passed;
  while (Passed.GetElapsed() <= TimeOut) {
    if (Transceiver.IsConnected() == true) {
      return true;
    }
    gSystem->Sleep(10);
  }

  return false;
}


//! Run a raw in-process round-trip that mirrors the event-list usage.
static bool RunRawInProcess(MUnitTest& Test, unsigned int Port)
{
  bool Passed = true;
  MTransceiverTcpIp Server("RawServer", "localhost", Port, MTransceiverTcpIp::c_ModeRawEventList);
  MTransceiverTcpIp Client("RawClient", "localhost", Port, MTransceiverTcpIp::c_ModeRawEventList);
  Server.SetVerbosity(2);
  Client.SetVerbosity(1);
  Server.RequestServer(true);
  Client.RequestClient(true);

  Passed = Test.EvaluateTrue("Raw in-process", "server connect", "The raw server starts connecting", Server.Connect(false)) && Passed;
  if (Test.EvaluateTrue("Raw in-process", "client connect", "The raw client connects", Client.Connect(true, 60.0)) == false) {
    cout<<"Raw in-process: client connect failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Raw in-process", "server connected", "The raw server reports connected", WaitForConnected(Server, 60.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "client connected", "The raw client reports connected", WaitForConnected(Client, 60.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: connect timeout"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Raw in-process", "server is connected", "The raw server is connected", Server.IsConnected()) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "server is server", "The raw server is marked as server", Server.IsServer()) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "client is connected", "The raw client is connected", Client.IsConnected()) && Passed;
  Passed = Test.EvaluateFalse("Raw in-process", "client is server", "The raw client is not marked as server", Client.IsServer()) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: connection state mismatch"<<endl;
    return false;
  }

  MString SendPayload1;
  MString ExpectedMessage;
  if (Test.EvaluateTrue("Raw in-process", "payload build", "The raw payload can be built", BuildRawPayload(SendPayload1, ExpectedMessage)) == false) {
    cout<<"Raw in-process: unable to build payload"<<endl;
    return false;
  }
  const MString SendPayload2 = SendPayload1;

  Passed = Test.EvaluateTrue("Raw in-process", "first send", "The raw client sends the first payload", Client.Send(SendPayload1)) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "second send", "The raw client sends the second payload", Client.Send(SendPayload2)) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "client queue drained", "The raw client send queue drains", WaitForSendQueueEmpty(Client, 20.0)) == false) {
    cout<<"Raw in-process: send queue timeout"<<endl;
    return false;
  }

  MString Received;
  for (unsigned int i = 0; i < 2; ++i) {
    if (Test.EvaluateTrue("Raw in-process", "server receive", "The raw server receives a message", WaitForMessage(Server, Received, 20.0)) == false) {
      cout<<"Raw in-process: receive timeout"<<endl;
      return false;
    }
    Passed = Test.Evaluate("Raw in-process", "server message", "The raw server receives the expected payload", Received, ExpectedMessage) && Passed;
    if (Passed == false) {
      cout<<"Raw in-process: message mismatch"<<endl;
      return false;
    }
  }

  if (Test.EvaluateTrue("Raw in-process", "reply send", "The raw server sends the reply", Server.Send("ServerToClientEN")) == false) {
    cout<<"Raw in-process: reply send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reply queue drained", "The raw server send queue drains", WaitForSendQueueEmpty(Server, 20.0)) == false) {
    cout<<"Raw in-process: reply queue timeout"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reply receive", "The raw client receives the reply", WaitForMessage(Client, Received, 20.0)) == false) {
    cout<<"Raw in-process: reply receive timeout"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Raw in-process", "reply message", "The raw client receives the expected reply", Received, MString("ServerToClient")) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: reply mismatch"<<endl;
    return false;
  }

  Passed = Test.EvaluateTrue("Raw in-process", "client disconnect", "The raw client disconnects without waiting", Client.Disconnect(false, 20.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "server disconnect", "The raw server disconnects without waiting", Server.Disconnect(false, 20.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: disconnect failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateFalse("Raw in-process", "client disconnected", "The raw client is disconnected after Disconnect(false)", Client.IsConnected()) && Passed;
  Passed = Test.EvaluateFalse("Raw in-process", "server disconnected", "The raw server is disconnected after Disconnect(false)", Server.IsConnected()) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: still connected after disconnect"<<endl;
    return false;
  }

  if (Test.EvaluateTrue("Raw in-process", "server reconnect connect", "The raw server reconnects", Server.Connect(false)) == false) {
    cout<<"Raw in-process: reconnect server start failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "client reconnect connect", "The raw client reconnects", Client.Connect(true, 60.0)) == false) {
    cout<<"Raw in-process: reconnect client failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Raw in-process", "server reconnect connected", "The raw server reconnects to a connected state", WaitForConnected(Server, 60.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "client reconnect connected", "The raw client reconnects to a connected state", WaitForConnected(Client, 60.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: reconnect timeout"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect send", "The raw client sends after reconnect", Client.Send(SendPayload1)) == false) {
    cout<<"Raw in-process: reconnect send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect queue drained", "The raw client send queue drains after reconnect", WaitForSendQueueEmpty(Client, 20.0)) == false) {
    cout<<"Raw in-process: reconnect send queue timeout"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect receive", "The raw server receives after reconnect", WaitForMessage(Server, Received, 20.0)) == false) {
    cout<<"Raw in-process: reconnect receive timeout"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Raw in-process", "reconnect message", "The raw server receives the expected reconnect payload", Received, ExpectedMessage) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: reconnect receive mismatch"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect reply send", "The raw server sends the reconnect reply", Server.Send("ServerToClientEN")) == false) {
    cout<<"Raw in-process: reconnect reply send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect reply queue drained", "The raw server send queue drains after reconnect", WaitForSendQueueEmpty(Server, 20.0)) == false) {
    cout<<"Raw in-process: reconnect reply queue timeout"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw in-process", "reconnect reply receive", "The raw client receives the reconnect reply", WaitForMessage(Client, Received, 20.0)) == false) {
    cout<<"Raw in-process: reconnect reply receive timeout"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Raw in-process", "reconnect reply message", "The raw client receives the expected reconnect reply", Received, MString("ServerToClient")) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: reconnect reply mismatch"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Raw in-process", "reconnect client disconnect", "The raw client disconnects after reconnect", Client.Disconnect(false, 20.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw in-process", "reconnect server disconnect", "The raw server disconnects after reconnect", Server.Disconnect(false, 20.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw in-process: reconnect disconnect failed"<<endl;
    return false;
  }

  return true;
}


//! Run a raw in-process overflow regression.
static bool RunRawOverflowInProcess(MUnitTest& Test, unsigned int Port)
{
  bool Passed = true;
  MTransceiverTcpIp Server("RawOverflowServer", "localhost", Port, MTransceiverTcpIp::c_ModeRawEventList);
  MTransceiverTcpIp Client("RawOverflowClient", "localhost", Port, MTransceiverTcpIp::c_ModeRawEventList);
  Server.SetVerbosity(2);
  Client.SetVerbosity(1);
  Server.RequestServer(true);
  Client.RequestClient(true);

  Passed = Test.EvaluateTrue("Raw overflow", "server connect", "The raw overflow server starts connecting", Server.Connect(false)) && Passed;
  if (Test.EvaluateTrue("Raw overflow", "client connect", "The raw overflow client connects", Client.Connect(true, 60.0)) == false) {
    cout<<"Raw overflow: client connect failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Raw overflow", "server connected", "The raw overflow server reports connected", WaitForConnected(Server, 60.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw overflow", "client connected", "The raw overflow client reports connected", WaitForConnected(Client, 60.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw overflow: connect timeout"<<endl;
    return false;
  }

  MString OverflowPayload;
  if (Test.EvaluateTrue("Raw overflow", "payload build", "The oversized raw payload can be built", BuildRawOverflowPayload(OverflowPayload)) == false) {
    cout<<"Raw overflow: unable to build payload"<<endl;
    return false;
  }

  if (Test.EvaluateTrue("Raw overflow", "send", "The raw overflow client sends the oversized payload", Client.Send(OverflowPayload)) == false) {
    cout<<"Raw overflow: send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Raw overflow", "send queue drained", "The raw overflow client send queue drains", WaitForSendQueueEmpty(Client, 20.0)) == false) {
    cout<<"Raw overflow: send queue timeout"<<endl;
    return false;
  }

  MString Unexpected;
  if (Test.EvaluateFalse("Raw overflow", "unexpected receive", "The raw overflow server does not receive a message", WaitForMessage(Server, Unexpected, 2.0)) == false) {
    cout<<"Raw overflow: unexpected message received"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Raw overflow", "server receive queue", "The raw overflow server receive queue stays empty", Server.GetNStringsToReceive(), 0u) && Passed;
  Passed = Test.Evaluate("Raw overflow", "server received count", "The raw overflow server received count stays zero", Server.GetNReceivedStrings(), 0ul) && Passed;
  if (Passed == false) {
    cout<<"Raw overflow: overflow message was not dropped"<<endl;
    return false;
  }

  Passed = Test.EvaluateTrue("Raw overflow", "client disconnect", "The raw overflow client disconnects", Client.Disconnect(true, 20.0)) && Passed;
  Passed = Test.EvaluateTrue("Raw overflow", "server disconnect", "The raw overflow server disconnects", Server.Disconnect(true, 20.0)) && Passed;
  if (Passed == false) {
    cout<<"Raw overflow: disconnect failed"<<endl;
    return false;
  }

  return true;
}


//! Run a default dual-role negotiation test without forcing a role.
static bool RunDefaultNegotiation(MUnitTest& Test, unsigned int Port)
{
  bool Passed = true;
  MTransceiverTcpIp Server("DefaultServer", "localhost", Port, MTransceiverTcpIp::c_ModeASCIIText);
  MTransceiverTcpIp Client("DefaultClient", "localhost", Port, MTransceiverTcpIp::c_ModeASCIIText);
  Server.SetVerbosity(2);
  Client.SetVerbosity(1);

  Passed = Test.EvaluateTrue("Default negotiation", "server connect", "The default server starts connecting", Server.Connect(false)) && Passed;
  if (Test.EvaluateTrue("Default negotiation", "client connect", "The default client connects", Client.Connect(true, 60.0)) == false) {
    cout<<"Default negotiation: client connect failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Default negotiation", "server connected", "The default server reports connected", WaitForConnected(Server, 60.0)) && Passed;
  Passed = Test.EvaluateTrue("Default negotiation", "client connected", "The default client reports connected", WaitForConnected(Client, 60.0)) && Passed;
  if (Passed == false) {
    cout<<"Default negotiation: connect timeout"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Default negotiation", "server connected state", "The default server is connected", Server.IsConnected()) && Passed;
  Passed = Test.EvaluateTrue("Default negotiation", "client connected state", "The default client is connected", Client.IsConnected()) && Passed;
  Passed = Test.EvaluateTrue("Default negotiation", "server role", "The default server is marked as server", Server.IsServer()) && Passed;
  Passed = Test.EvaluateFalse("Default negotiation", "client role", "The default client is not marked as server", Client.IsServer()) && Passed;
  if (Passed == false) {
    cout<<"Default negotiation: connection state mismatch"<<endl;
    return false;
  }

  if (Test.EvaluateTrue("Default negotiation", "client send", "The default client sends to the server", Client.Send("DefaultClientToServer")) == false) {
    cout<<"Default negotiation: client send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Default negotiation", "client queue drained", "The default client send queue drains", WaitForSendQueueEmpty(Client, 20.0)) == false) {
    cout<<"Default negotiation: client send queue timeout"<<endl;
    return false;
  }
  MString Received;
  if (Test.EvaluateTrue("Default negotiation", "server receive", "The default server receives a message", WaitForMessage(Server, Received, 20.0)) == false) {
    cout<<"Default negotiation: server receive timeout"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Default negotiation", "server message", "The default server receives the expected payload", Received, MString("DefaultClientToServer")) && Passed;
  if (Passed == false) {
    cout<<"Default negotiation: server receive mismatch"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Default negotiation", "server send", "The default server sends to the client", Server.Send("DefaultServerToClient")) == false) {
    cout<<"Default negotiation: server send failed"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Default negotiation", "server queue drained", "The default server send queue drains", WaitForSendQueueEmpty(Server, 20.0)) == false) {
    cout<<"Default negotiation: server send queue timeout"<<endl;
    return false;
  }
  if (Test.EvaluateTrue("Default negotiation", "client receive", "The default client receives a message", WaitForMessage(Client, Received, 20.0)) == false) {
    cout<<"Default negotiation: client receive timeout"<<endl;
    return false;
  }
  Passed = Test.Evaluate("Default negotiation", "client message", "The default client receives the expected payload", Received, MString("DefaultServerToClient")) && Passed;
  if (Passed == false) {
    cout<<"Default negotiation: client receive mismatch"<<endl;
    return false;
  }

  Passed = Test.EvaluateTrue("Default negotiation", "client disconnect", "The default client disconnects without waiting", Client.Disconnect(false, 20.0)) && Passed;
  Passed = Test.EvaluateTrue("Default negotiation", "server disconnect", "The default server disconnects without waiting", Server.Disconnect(false, 20.0)) && Passed;
  if (Passed == false) {
    cout<<"Default negotiation: disconnect failed"<<endl;
    return false;
  }

  return true;
}


//! Force a remote-side connection loss and verify reset accounting.
static bool RunForcedReset(MUnitTest& Test, unsigned int Port)
{
  bool Passed = true;
  const MString ServerArgument = MString("--server-ascii:") + Port;
  const pid_t ServerPid = StartChildProcess(g_ExecutablePath, ServerArgument, MString("/tmp/UTTransceiverTcpIp_reset_server_") + Port + ".log");
  if (ServerPid < 0) {
    cout<<"Forced reset: unable to start server child"<<endl;
    return false;
  }

  const MString ServerReady = GetReadyFile(Port, "server");
  if (WaitForFile(ServerReady, 60.0) == false) {
    cout<<"Forced reset: server ready timeout"<<endl;
    return false;
  }

  MTransceiverTcpIp Client("ResetClient", "localhost", Port, MTransceiverTcpIp::c_ModeASCIIText);
  Client.SetVerbosity(1);
  Client.RequestClient(true);
  if (Test.EvaluateTrue("Forced reset", "client connect", "The reset client connects", Client.Connect(true, 60.0)) == false) {
    cout<<"Forced reset: client connect failed"<<endl;
    return false;
  }
  Passed = Test.EvaluateTrue("Forced reset", "client connected", "The reset client reports connected", WaitForConnected(Client, 60.0)) && Passed;
  Passed = Test.EvaluateTrue("Forced reset", "client connected state", "The reset client is connected", Client.IsConnected()) && Passed;
  if (Passed == false) {
    cout<<"Forced reset: client connect timeout"<<endl;
    return false;
  }

  kill(ServerPid, SIGKILL);
  int ServerStatus = -1;
  if (WaitForChildProcess(ServerPid, ServerStatus, 20.0) == false) {
    cout<<"Forced reset: server child did not exit"<<endl;
    return false;
  }

  MTimer Elapsed;
  while (Elapsed.GetElapsed() <= 20.0) {
    if (Client.GetNResets() > 0 && Client.IsConnected() == false) {
      break;
    }
    gSystem->Sleep(10);
  }
  Passed = Test.EvaluateTrue("Forced reset", "reset count", "The reset client observes at least one reset", Client.GetNResets() > 0) && Passed;
  Passed = Test.EvaluateFalse("Forced reset", "client disconnected", "The reset client is disconnected after the peer loss", Client.IsConnected()) && Passed;
  if (Passed == false) {
    cout<<"Forced reset: reset counter not incremented"<<endl;
    return false;
  }

  if (Test.EvaluateTrue("Forced reset", "client disconnect", "The reset client disconnects after the forced loss", Client.Disconnect(false, 20.0)) == false) {
    cout<<"Forced reset: client disconnect failed"<<endl;
    return false;
  }
  return true;
}


//! Run one server role in a child process.
static int RunServerRole(const MString& Argument)
{
  const bool IsRaw = Argument.BeginsWith("--server-raw:");
  const bool IsAscii = Argument.BeginsWith("--server-ascii:");
  const bool IsInvalid = Argument.BeginsWith("--server-invalid:");
  if (IsRaw == false && IsAscii == false && IsInvalid == false) {
    return 1;
  }

  const size_t PrefixLength = IsRaw ? MString("--server-raw:").Length() : (IsInvalid ? MString("--server-invalid:").Length() : MString("--server-ascii:").Length());
  const unsigned int Port = static_cast<unsigned int>(strtoul(Argument.GetSubString(PrefixLength).Data(), nullptr, 10));
  const unsigned int Mode = IsRaw ? MTransceiverTcpIp::c_ModeRawEventList : (IsInvalid ? 123456 : MTransceiverTcpIp::c_ModeASCIIText);

  MTransceiverTcpIp Server("Server", "localhost", Port, Mode);
  Server.SetVerbosity(3);
  Server.RequestServer(true);

  Server.Connect(false);
  if (TouchFile(GetReadyFile(Port, "server")) == false) {
    cout<<"Server role: unable to create ready marker"<<endl;
    return 1;
  }
  if (WaitForConnected(Server, 60.0) == false) {
    cout<<"Server role: connect timeout"<<endl;
    return 1;
  }
  if (Server.IsConnected() == false || Server.IsServer() == false) {
    cout<<"Server role: connection state mismatch"<<endl;
    return 1;
  }
  if (Server.GetNResets() != 0) {
    cout<<"Server role: unexpected resets"<<endl;
    return 1;
  }
  cout<<"Server role: connected"<<endl;

  MString Received;
  const unsigned int MessageCount = 2;
  for (unsigned int i = 0; i < MessageCount; ++i) {
    cout<<"Server role: waiting for message "<<i<<endl;
    if (WaitForMessage(Server, Received, IsRaw ? 20.0 : 10.0) == false) {
      cout<<"Server role: receive timeout"<<endl;
      return 1;
    }
    cout<<"Server role: received '"<<Received<<"'"<<endl;

    MString Expected;
    if (IsRaw == true) {
      MString Payload;
      if (BuildRawPayload(Payload, Expected) == false) {
        cout<<"Server role: unable to build raw expectation"<<endl;
        return 1;
      }
    } else {
      Expected = MString("ClientToServer") + (i+1);
    }
    if (Received != Expected) {
      cout<<"Server role: message mismatch"<<endl;
      return 1;
    }
  }

  const MString Reply = IsRaw ? MString("ServerToClientEN") : MString("ServerToClient");
  cout<<"Server role: sending reply '"<<Reply<<"'"<<endl;
  if (Server.Send(Reply) == false) {
    cout<<"Server role: send failed"<<endl;
    return 1;
  }
  if (WaitForSendQueueEmpty(Server, 20.0) == false) {
    cout<<"Server role: reply queue timeout"<<endl;
    return 1;
  }
  if (WaitForFile(GetReadyFile(Port, "client_reply"), 20.0) == false) {
    cout<<"Server role: reply receipt timeout"<<endl;
    return 1;
  }

  if (Server.Disconnect(true, 20.0) == false) {
    cout<<"Server role: disconnect failed"<<endl;
    return 1;
  }
  if (Server.IsConnected() == true) {
    cout<<"Server role: still connected after disconnect"<<endl;
    return 1;
  }
  cout<<"Server role: disconnected"<<endl;

  if (IsRaw == false) {
    const MString ReconnectReady = GetReadyFile(Port, "server_reconnect");
    const MString ReconnectReplyReady = GetReadyFile(Port, "client_reply2");
    Server.Connect(false);
    if (TouchFile(ReconnectReady) == false) {
      cout<<"Server role: unable to create reconnect ready marker"<<endl;
      return 1;
    }
    if (WaitForConnected(Server, 60.0) == false) {
      cout<<"Server role: reconnect timeout"<<endl;
      return 1;
    }
    if (Server.IsConnected() == false || Server.IsServer() == false) {
      cout<<"Server role: reconnect state mismatch"<<endl;
      return 1;
    }

    if (WaitForMessage(Server, Received, 20.0) == false) {
      cout<<"Server role: reconnect receive timeout"<<endl;
      return 1;
    }
    if (Received != "ReconnectClientToServer") {
      cout<<"Server role: reconnect message mismatch"<<endl;
      return 1;
    }
    if (Server.Send("ReconnectServerToClient") == false) {
      cout<<"Server role: reconnect send failed"<<endl;
      return 1;
    }
    if (WaitForSendQueueEmpty(Server, 20.0) == false) {
      cout<<"Server role: reconnect send queue timeout"<<endl;
      return 1;
    }
    if (WaitForFile(ReconnectReplyReady, 60.0) == false) {
      cout<<"Server role: reconnect reply receipt timeout"<<endl;
      return 1;
    }
    if (Server.Disconnect(true, 20.0) == false) {
      cout<<"Server role: reconnect disconnect failed"<<endl;
      return 1;
    }
  }
  return 0;
}


//! Run one client role in a child process.
static int RunClientRole(const MString& Argument)
{
  const bool IsRaw = Argument.BeginsWith("--client-raw:");
  const bool IsAscii = Argument.BeginsWith("--client-ascii:");
  const bool IsInvalid = Argument.BeginsWith("--client-invalid:");
  if (IsRaw == false && IsAscii == false && IsInvalid == false) {
    return 1;
  }

  const size_t PrefixLength = IsRaw ? MString("--client-raw:").Length() : (IsInvalid ? MString("--client-invalid:").Length() : MString("--client-ascii:").Length());
  const unsigned int Port = static_cast<unsigned int>(strtoul(Argument.GetSubString(PrefixLength).Data(), nullptr, 10));
  const unsigned int Mode = IsRaw ? MTransceiverTcpIp::c_ModeRawEventList : (IsInvalid ? 123456 : MTransceiverTcpIp::c_ModeASCIIText);

  MTransceiverTcpIp Client("Client", "localhost", Port, Mode);
  Client.SetVerbosity(3);
  Client.RequestClient(true);

  if (Client.Connect(true, 60.0) == false) {
    cout<<"Client role: connect failed"<<endl;
    return 1;
  }
  if (Client.IsConnected() == false || Client.IsServer() == true) {
    cout<<"Client role: connection state mismatch"<<endl;
    return 1;
  }
  if (TouchFile(GetReadyFile(Port, "client")) == false) {
    cout<<"Client role: unable to create ready marker"<<endl;
    return 1;
  }
  cout<<"Client role: connected"<<endl;

  MString SendPayload1;
  MString SendPayload2;
  if (IsRaw == true) {
    MString ExpectedReply;
    if (BuildRawPayload(SendPayload1, ExpectedReply) == false) {
      cout<<"Client role: unable to build raw payload"<<endl;
      return 1;
    }
    SendPayload2 = SendPayload1;
  } else {
    SendPayload1 = "ClientToServer1";
    SendPayload2 = "ClientToServer2";
  }

  cout<<"Client role: sending '"<<SendPayload1<<"'"<<endl;
  if (Client.Send(SendPayload1) == false) {
    cout<<"Client role: send failed"<<endl;
    return 1;
  }
  if (SendPayload2.IsEmpty() == false) {
    cout<<"Client role: sending '"<<SendPayload2<<"'"<<endl;
    if (Client.Send(SendPayload2) == false) {
      cout<<"Client role: second send failed"<<endl;
      return 1;
    }
  }

  cout<<"Client role: waiting for send queue to drain"<<endl;
  if (WaitForSendQueueEmpty(Client, 20.0) == false) {
    cout<<"Client role: send queue timeout"<<endl;
    return 1;
  }

  MString Received;
  cout<<"Client role: waiting for reply"<<endl;
  if (WaitForMessage(Client, Received, 20.0) == false) {
    cout<<"Client role: reply timeout"<<endl;
    return 1;
  }
  cout<<"Client role: received '"<<Received<<"'"<<endl;
  if (Received != "ServerToClient") {
    cout<<"Client role: reply mismatch"<<endl;
    return 1;
  }
  if (TouchFile(GetReadyFile(Port, "client_reply")) == false) {
    cout<<"Client role: unable to create reply marker"<<endl;
    return 1;
  }

  if (Client.GetNStringsToReceive() != 0) {
    cout<<"Client role: receive queue not empty"<<endl;
    return 1;
  }
  const unsigned int ExpectedSent = 2u;
  if (Client.GetNSentStrings() != ExpectedSent || Client.GetNReceivedStrings() < 1 || Client.GetNResets() != 0) {
    cout<<"Client role: counter mismatch"<<endl;
    return 1;
  }

  if (Client.Disconnect(true, 20.0) == false) {
    cout<<"Client role: disconnect failed"<<endl;
    return 1;
  }
  if (Client.IsConnected() == true) {
    cout<<"Client role: still connected after disconnect"<<endl;
    return 1;
  }
  if (Client.GetNResets() != 0) {
    cout<<"Client role: unexpected resets after clean disconnect"<<endl;
    return 1;
  }
  cout<<"Client role: disconnected"<<endl;

  if (IsRaw == false) {
    const MString ReconnectReady = GetReadyFile(Port, "server_reconnect");
    if (WaitForFile(ReconnectReady, 60.0) == false) {
      cout<<"Client role: reconnect ready timeout"<<endl;
      return 1;
    }
    if (Client.Connect(true, 60.0) == false) {
      cout<<"Client role: reconnect connect failed"<<endl;
      return 1;
    }
    if (Client.IsConnected() == false || Client.IsServer() == true) {
      cout<<"Client role: reconnect state mismatch"<<endl;
      return 1;
    }
    if (Client.Send("ReconnectClientToServer") == false) {
      cout<<"Client role: reconnect send failed"<<endl;
      return 1;
    }
    if (WaitForSendQueueEmpty(Client, 20.0) == false) {
      cout<<"Client role: reconnect send queue timeout"<<endl;
      return 1;
    }
    if (WaitForMessage(Client, Received, 20.0) == false) {
      cout<<"Client role: reconnect reply timeout"<<endl;
      return 1;
    }
    if (Received != "ReconnectServerToClient") {
      cout<<"Client role: reconnect reply mismatch"<<endl;
      return 1;
    }
    if (TouchFile(GetReadyFile(Port, "client_reply2")) == false) {
      cout<<"Client role: unable to create reconnect reply marker"<<endl;
      return 1;
    }
    if (Client.Disconnect(true, 20.0) == false) {
      cout<<"Client role: reconnect disconnect failed"<<endl;
      return 1;
    }
  }
  return 0;
}


//! Unit test class for MTransceiverTcpIp
class UTTransceiverTcpIp : public MUnitTest
{
public:
  UTTransceiverTcpIp() : MUnitTest("UTTransceiverTcpIp") {}
  virtual ~UTTransceiverTcpIp() {}

  virtual bool Run();

private:
  static unsigned int GetFreePort();
  bool RunMode(bool RawMode, bool InvalidMode, unsigned int Port);
};


////////////////////////////////////////////////////////////////////////////////


unsigned int UTTransceiverTcpIp::GetFreePort()
{
  int Socket = socket(AF_INET, SOCK_STREAM, 0);
  if (Socket < 0) {
    return 0;
  }

  sockaddr_in Address;
  memset(&Address, 0, sizeof(Address));
  Address.sin_family = AF_INET;
  Address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  Address.sin_port = htons(0);

  if (::bind(Socket, reinterpret_cast<sockaddr*>(&Address), sizeof(Address)) != 0) {
    close(Socket);
    return 0;
  }

  socklen_t Length = sizeof(Address);
  if (getsockname(Socket, reinterpret_cast<sockaddr*>(&Address), &Length) != 0) {
    close(Socket);
    return 0;
  }

  const unsigned int Port = ntohs(Address.sin_port);
  close(Socket);
  return Port;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTransceiverTcpIp::RunMode(bool RawMode, bool InvalidMode, unsigned int Port)
{
  bool Passed = true;
  const MString ServerArgument = RawMode ? MString("--server-raw:") + Port : (InvalidMode ? MString("--server-invalid:") + Port : MString("--server-ascii:") + Port);
  const MString ClientArgument = RawMode ? MString("--client-raw:") + Port : (InvalidMode ? MString("--client-invalid:") + Port : MString("--client-ascii:") + Port);
  const MString ServerReady = GetReadyFile(Port, "server");
  const MString ClientReady = GetReadyFile(Port, "client");
  const MString ClientReply = GetReadyFile(Port, "client_reply");
  const MString ServerReconnect = GetReadyFile(Port, "server_reconnect");
  const MString ClientReply2 = GetReadyFile(Port, "client_reply2");

  gSystem->Unlink(ServerReady);
  gSystem->Unlink(ClientReady);
  gSystem->Unlink(ClientReply);
  gSystem->Unlink(ServerReconnect);
  gSystem->Unlink(ClientReply2);

  const pid_t ServerPid = StartChildProcess(g_ExecutablePath, ServerArgument, MString("/tmp/UTTransceiverTcpIp_server_") + Port + ".log");
  Passed = EvaluateTrue("Server child process", RawMode ? "raw server" : "ASCII server", "The server child process can be started", ServerPid >= 0) && Passed;
  if (ServerPid < 0) {
    return false;
  }
  Passed = EvaluateTrue("Server ready", RawMode ? "raw server" : "ASCII server", "The server child becomes ready before the client starts", WaitForFile(ServerReady, 60.0)) && Passed;
  const pid_t ClientPid = StartChildProcess(g_ExecutablePath, ClientArgument, MString("/tmp/UTTransceiverTcpIp_client_") + Port + ".log");
  Passed = EvaluateTrue("Client child process", RawMode ? "raw client" : "ASCII client", "The client child process can be started", ClientPid >= 0) && Passed;
  if (ClientPid < 0) {
    return false;
  }

  int ServerStatus = -1;
  int ClientStatus = -1;
  if (WaitForChildProcess(ServerPid, ServerStatus, 120.0) == false) {
    Passed = EvaluateTrue("Server child process", RawMode ? "raw server" : "ASCII server", "The server child process exits successfully", false) && Passed;
  } else {
    Passed = EvaluateTrue("Server child process", RawMode ? "raw server" : "ASCII server", "The server child process exits successfully", WIFEXITED(ServerStatus) && WEXITSTATUS(ServerStatus) == 0) && Passed;
  }
  if (WaitForChildProcess(ClientPid, ClientStatus, 120.0) == false) {
    Passed = EvaluateTrue("Client child process", RawMode ? "raw client" : "ASCII client", "The client child process exits successfully", false) && Passed;
  } else {
    Passed = EvaluateTrue("Client child process", RawMode ? "raw client" : "ASCII client", "The client child process exits successfully", WIFEXITED(ClientStatus) && WEXITSTATUS(ClientStatus) == 0) && Passed;
  }

  gSystem->Unlink(ServerReady);
  gSystem->Unlink(ClientReady);
  gSystem->Unlink(ClientReply);
  gSystem->Unlink(ServerReconnect);
  gSystem->Unlink(ClientReply2);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTTransceiverTcpIp::Run()
{
  bool Passed = true;

  gROOT->SetBatch(true);

  {
    MTransceiverTcpIp Default;
    Default.SetVerbosity(0);
    Passed = Evaluate("Default ctor", "name", "The default transceiver name is set", Default.GetName(), MString("A transceiver")) && Passed;
    Passed = Evaluate("Default ctor", "host", "The default transceiver host is set", Default.GetHost(), MString("localhost")) && Passed;
    Passed = Evaluate("Default ctor", "port", "The default transceiver port is set", Default.GetPort(), 9090u) && Passed;
    Passed = EvaluateFalse("Default ctor", "connected", "The default transceiver starts disconnected", Default.IsConnected()) && Passed;
    Passed = EvaluateFalse("Default ctor", "server", "The default transceiver starts in client-neutral state", Default.IsServer()) && Passed;
    Default.SetName("Renamed");
    Default.SetHost("example.org");
    Default.SetPort(12345);
    Passed = Evaluate("SetName()", "name", "The transceiver name setter works", Default.GetName(), MString("Renamed")) && Passed;
    Passed = Evaluate("SetHost()", "host", "The transceiver host setter works", Default.GetHost(), MString("example.org")) && Passed;
    Passed = Evaluate("SetPort()", "port", "The transceiver port setter works", Default.GetPort(), 12345u) && Passed;
    Default.ClearBuffers();
  }

  {
    MTransceiverTcpIp Queue("Queue");
    Queue.SetVerbosity(1);
    Queue.SetMaximumBufferSize(1);
    Queue.Send("First");
    Queue.Send("Second");
    Queue.Send("Third");
    Passed = Evaluate("SetMaximumBufferSize()", "queue", "The send queue keeps only the newest message when the maximum buffer size is one", Queue.GetNStringsToSend(), 1u) && Passed;
    Queue.ClearBuffers();
    Passed = Evaluate("ClearBuffers()", "queue", "ClearBuffers empties the queued messages", Queue.GetNStringsToSend(), 0u) && Passed;
  }

  {
    MTransceiverTcpIp Roles("Roles");
    Roles.SetVerbosity(2);
    Roles.RequestClient(false);
    Roles.RequestServer(false);
    Passed = EvaluateFalse("RequestClient()/RequestServer()", "disabled", "Disabling both roles keeps the transceiver disconnected", Roles.Connect(false)) && Passed;
    Passed = EvaluateFalse("RequestClient()/RequestServer()", "disabled", "Disabling both roles keeps the transceiver disconnected", Roles.IsConnected()) && Passed;
  }

  const unsigned int FailurePort = GetFreePort();
  {
    MTransceiverTcpIp Failure("Failure", "localhost", FailurePort, MTransceiverTcpIp::c_ModeASCIIText);
    Failure.SetVerbosity(0);
    Failure.RequestClient(true);
    Passed = EvaluateFalse("Connect()", "failure", "Connect(true) fails on an unused port", Failure.Connect(true, 1.0)) && Passed;
    Passed = EvaluateFalse("IsConnected()", "failure", "The transceiver remains disconnected after a failed connect", Failure.IsConnected()) && Passed;
  }

  Passed = RunMode(false, false, GetFreePort()) && Passed;
  Passed = RunMode(true, false, GetFreePort()) && Passed;
  Passed = RunRawInProcess(*this, GetFreePort()) && Passed;
  Passed = RunRawOverflowInProcess(*this, GetFreePort()) && Passed;
  Passed = RunDefaultNegotiation(*this, GetFreePort()) && Passed;
  Passed = RunForcedReset(*this, GetFreePort()) && Passed;
  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (argc == 2) {
    MString Argument = argv[1];
    if (Argument.BeginsWith("--server-ascii:") || Argument.BeginsWith("--server-raw:") || Argument.BeginsWith("--server-invalid:")) {
      return RunServerRole(Argument);
    }
    if (Argument.BeginsWith("--client-ascii:") || Argument.BeginsWith("--client-raw:") || Argument.BeginsWith("--client-invalid:")) {
      return RunClientRole(Argument);
    }
  }

  g_ExecutablePath = argc > 0 ? argv[0] : "bin/UTTransceiverTcpIp";

  UTTransceiverTcpIp Test;
  return Test.Run() == true ? 0 : 1;
}
