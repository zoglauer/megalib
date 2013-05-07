/*
 * MSettingsRealta.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsRealta__
#define __MSettingsRealta__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"
#include "MSettingsEventSelections.h"
#include "MSettingsImaging.h"
#include "MSettingsEventReconstruction.h"
#include "MSettingsSpectralOptions.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsRealta : public MSettings, public MSettingsEventSelections, public MSettingsImaging, public MSettingsEventReconstruction, public MSettingsSpectralOptions
{
  // public Session:
 public:
  //! Standard constructor
  MSettingsRealta(bool AutoLoad = true);
  //! default destructor
  virtual ~MSettingsRealta();

  //! Get the name of the host
  MString GetHostName() const { return m_HostName; }
  //! Set the name of the host
  void SetHostName(MString HostName) { m_HostName = HostName; }

  //! Get the port
  int GetPort() const { return m_Port; }
  //! Set the port
  void SetPort(int Port) { m_Port = Port; }

  //! True if the connection is established immediately after the lauch of the application
  bool GetConnectOnStart() const { return m_ConnectOnStart; }
  //! Set if the connection should be established immediately after the lauch of the application
  void SetConnectOnStart(bool ConnectOnStart) { m_ConnectOnStart = ConnectOnStart; }

  //! Return the transceiver mode: MTransceiverTcpIp::c_ModeASCIIText or MTransceiverTcpIp::c_ModeRawEventList
  unsigned int GetTransceiverMode() const { return m_TransceiverMode; }
  //! Set the transceiver mode: MTransceiverTcpIp::c_ModeASCIIText or MTransceiverTcpIp::c_ModeRawEventList
  void SetTransceiverMode(unsigned int TransceiverMode) { m_TransceiverMode = TransceiverMode; }
  
  //! Get the accumulation time
  double GetAccumulationTime() const { return m_AccumulationTime; }
  //! Set the accumulation time
  void SetAccumulationTime(double AccumulationTime) { m_AccumulationTime = AccumulationTime; }
  
  //! Get the bins in the count rate histogram
  int GetBinsCountRate() const { return m_BinsCountRate; }
  //! Set the bins in the count rate histogram
  void SetBinsCountRate(int BinsCountRate) { m_BinsCountRate = BinsCountRate; }
  
  //! Get the bins in the spectrum
  int GetBinsSpectrum() const { return m_BinsSpectrum; }
  //! Set the bins in the spectrum
  void SetBinsSpectrum(int BinsSpectrum) { m_BinsSpectrum = BinsSpectrum; }
  
  //! Get the accumulation file name 
  MString GetAccumulationFileName() const { return m_AccumulationFileName; }
  //! Set the accumulation file name
  void SetAccumulationFileName(MString AccumulationFileName) { m_AccumulationFileName = AccumulationFileName; }
  
  //! Return true if add date and time should be added to the accumulation file name
  bool GetAccumulationFileNameAddDateAndTime() const { return m_AccumulationFileNameAddDateAndTime; }
  //! Set if add date and time should be added to the accumulation file name
  void SetAccumulationFileNameAddDateAndTime(bool AccumulationFileNameAddDateAndTime) { m_AccumulationFileNameAddDateAndTime = AccumulationFileNameAddDateAndTime; }

  
  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);

  // private members:
 private:
  //! The name of the host
  MString m_HostName;
  //! The port of the host
  int m_Port;
  //! True if the connection is established immediately after the lauch of the application
  bool m_ConnectOnStart;
  
  //! The connection mode (represents one of MTransceiverTcpIp::c_ModeASCIIText, MTransceiverTcpIp::c_ModeRawEventList)
  unsigned int m_TransceiverMode;
  
  //! The event accumulation time
  double m_AccumulationTime;
  
  //! The bins in the count rate histogram
  int m_BinsCountRate;
  //! The bins in the spectrum
  int m_BinsSpectrum;
  
  //! The event accumulation file name
  MString m_AccumulationFileName;
  //! If true add date and time to the accumulation file name
  bool m_AccumulationFileNameAddDateAndTime;

#ifdef ___CINT___
 public:
  ClassDef(MSettingsRealta, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

