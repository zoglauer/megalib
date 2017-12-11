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

  
  void SetSpectrumBefore(bool Value) { m_SpectrumBefore = Value; }
  bool GetSpectrumBefore() const { return m_SpectrumBefore; }
  void SetSpectrumAfter(bool Value) { m_SpectrumAfter = Value; }
  bool GetSpectrumAfter() const { return m_SpectrumAfter; }
  
  void SetSpectrumSortByInstrument(bool Value) { m_SpectrumSortByInstrument = Value; }
  bool GetSpectrumSortByInstrument() const { return m_SpectrumSortByInstrument; }
  void SetSpectrumSortByDetectorType(bool Value) { m_SpectrumSortByDetectorType = Value; }
  bool GetSpectrumSortByDetectorType() const { return m_SpectrumSortByDetectorType; }
  void SetSpectrumSortByNamedDetector(bool Value) { m_SpectrumSortByNamedDetector = Value; }
  bool GetSpectrumSortByNamedDetector() const { return m_SpectrumSortByNamedDetector; }
  void SetSpectrumSortByDetector(bool Value) { m_SpectrumSortByDetector = Value; }
  bool GetSpectrumSortByDetector() const { return m_SpectrumSortByDetector; }
  
  void SetSpectrumOutputToScreen(bool Value) { m_SpectrumOutputToScreen = Value; }
  bool GetSpectrumOutputToScreen() const { return m_SpectrumOutputToScreen; }
  void SetSpectrumOutputToFile(bool Value) { m_SpectrumOutputToFile = Value; }
  bool GetSpectrumOutputToFile() const { return m_SpectrumOutputToFile; }
  
  void SetSpectrumCombine(bool Value) { m_SpectrumCombine = Value; }
  bool GetSpectrumCombine() const { return m_SpectrumCombine; }

  void SetSpectrumBins(int Value) { m_SpectrumBins = Value; }
  int GetSpectrumBins() const { return m_SpectrumBins; }
  void SetSpectrumLog(bool Value) { m_SpectrumLog = Value; }
  bool GetSpectrumLog() const { return m_SpectrumLog; }
  void SetSpectrumMin(double Value) { m_SpectrumMin = Value; }
  double GetSpectrumMin() const { return m_SpectrumMin; }
  void SetSpectrumMax(double Value) { m_SpectrumMax = Value; }
  double GetSpectrumMax() const { return m_SpectrumMax; }

  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:

  // General options for the spectrum
  bool m_SpectrumBefore;
  bool m_SpectrumAfter;
  
  bool m_SpectrumSortByInstrument;
  bool m_SpectrumSortByDetectorType;
  bool m_SpectrumSortByNamedDetector;
  bool m_SpectrumSortByDetector;
  
  bool m_SpectrumCombine;
  
  bool m_SpectrumOutputToScreen;
  bool m_SpectrumOutputToFile;
  
  int m_SpectrumBins;
  bool m_SpectrumLog;
  double m_SpectrumMin;
  double m_SpectrumMax;

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
