/*
 * MSettingsRevan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsRevan__
#define __MSettingsRevan__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:

// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"
#include "MSettingsBasicFiles.h"
#include "MSettingsEventReconstruction.h"
#include "MSettingsSpectralOptions.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsRevan : public MSettings, public MSettingsEventReconstruction, public MSettingsSpectralOptions
{
  // public Session:
 public:
  MSettingsRevan(bool AutoLoad = true);
  virtual ~MSettingsRevan();
  
  void SetExportSpectrumBins(int Value) { m_ExportSpectrumBins = Value; }
  int GetExportSpectrumBins() const { return m_ExportSpectrumBins; }
  void SetExportSpectrumLog(bool Value) { m_ExportSpectrumLog = Value; }
  bool GetExportSpectrumLog() const { return m_ExportSpectrumLog; }
  void SetExportSpectrumMin(double Value) { m_ExportSpectrumMin = Value; }
  double GetExportSpectrumMin() const { return m_ExportSpectrumMin; }
  void SetExportSpectrumMax(double Value) { m_ExportSpectrumMax = Value; }
  double GetExportSpectrumMax() const { return m_ExportSpectrumMax; }
  void SetExportSpectrumFileName(MString Value) { m_ExportSpectrumFileName = Value; }
  MString GetExportSpectrumFileName() const { return m_ExportSpectrumFileName; }

  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:

  // Options for exporting the spectrum
  int m_ExportSpectrumBins;
  bool m_ExportSpectrumLog;
  double m_ExportSpectrumMin;
  double m_ExportSpectrumMax;
  MString m_ExportSpectrumFileName;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MSettingsRevan, 0) // no description
#endif

};


#endif


////////////////////////////////////////////////////////////////////////////////
