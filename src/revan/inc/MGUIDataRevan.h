/*
 * MGUIDataRevan.h
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDataRevan__
#define __MGUIDataRevan__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIData.h"
#include "MRawEventAnalyzer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDataRevan : public MGUIData
{
  // public Session:
 public:
  MGUIDataRevan(bool AutoLoad = true);
  virtual ~MGUIDataRevan();

  bool UpdateRawEventAnalyzer(MRawEventAnalyzer& Analyzer);

  void SetNJobs(unsigned int ID) { m_NJobs = ID; }
  unsigned int GetNJobs() { return m_NJobs; }

  void SetCoincidenceAlgorithm(int ID) { m_CoincidenceAlgorithm = ID; }
  int GetCoincidenceAlgorithm() { return m_CoincidenceAlgorithm; }

  void SetClusteringAlgorithm(int ID) { m_ClusteringAlgorithm = ID; }
  int GetClusteringAlgorithm() { return m_ClusteringAlgorithm; }

  void SetTrackingAlgorithm(int ID) { m_TrackingAlgorithm = ID; }
  int GetTrackingAlgorithm() { return m_TrackingAlgorithm; }

  void SetCSRAlgorithm(int ID) { m_CSRAlgorithm = ID; }
  int GetCSRAlgorithm() { return m_CSRAlgorithm; }

  void SetDecayAlgorithm(int ID) { m_DecayAlgorithm = ID; }
  int GetDecayAlgorithm() { return m_DecayAlgorithm; }
  
  void SetCoincidenceWindow(double Time) {
    m_CoincidenceWindow = Time;
  }
  double GetCoincidenceWindow() {
    return m_CoincidenceWindow;
  }
  
  void SetStandardClusterizerMinDistanceD1(double MinDistance) {
    m_StandardClusterizerMinDistanceD1 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD1() {
    return m_StandardClusterizerMinDistanceD1;
  }

  void SetStandardClusterizerMinDistanceD2(double MinDistance) {
    m_StandardClusterizerMinDistanceD2 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD2() {
    return m_StandardClusterizerMinDistanceD2;
  }

  void SetStandardClusterizerMinDistanceD3(double MinDistance) {
    m_StandardClusterizerMinDistanceD3 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD3() {
    return m_StandardClusterizerMinDistanceD3;
  }

  void SetStandardClusterizerMinDistanceD4(double MinDistance) {
    m_StandardClusterizerMinDistanceD4 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD4() {
    return m_StandardClusterizerMinDistanceD4;
  }

  void SetStandardClusterizerMinDistanceD5(double MinDistance) {
    m_StandardClusterizerMinDistanceD5 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD5() {
    return m_StandardClusterizerMinDistanceD5;
  }

  void SetStandardClusterizerMinDistanceD6(double MinDistance) {
    m_StandardClusterizerMinDistanceD6 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD6() {
    return m_StandardClusterizerMinDistanceD6;
  }

  void SetStandardClusterizerCenterIsReference(bool CenterIsReference) {
    m_StandardClusterizerCenterIsReference = CenterIsReference;
  }
  bool GetStandardClusterizerCenterIsReference() {
    return m_StandardClusterizerCenterIsReference;
  }

  void SetAdjacentSigma(const double AdjacentSigma){
    m_AdjacentSigma = AdjacentSigma;
  }
  double GetAdjacentSigma() const {
    return m_AdjacentSigma;
  }

  void SetAdjacentLevel(const int AdjacentLevel){
    m_AdjacentLevel = AdjacentLevel;
  }
  int GetAdjacentLevel() const {
    return m_AdjacentLevel;
  }

  void SetDoTracking(bool Do) { m_DoTracking = Do; }
  bool GetDoTracking() { return m_DoTracking; }

  void SetSearchPairs(bool Search) { m_SearchPairs = Search; }
  bool GetSearchPairs() { return m_SearchPairs; }
  void SetSearchMIPs(bool Search) { m_SearchMIPs = Search; }
  bool GetSearchMIPs() { return m_SearchMIPs; }
  void SetSearchComptons(bool Search) { m_SearchComptons = Search; }
  bool GetSearchComptons() { return m_SearchComptons; }

  void SetMaxComptonJump(int Value) { m_MaxComptonJump = Value; }
  int GetMaxComptonJump() { return m_MaxComptonJump; }

  void SetNTrackSequencesToKeep(int Value) { m_NTrackSequencesToKeep = Value; }
  int GetNTrackSequencesToKeep() { return m_NTrackSequencesToKeep; }

  void SetRejectPurelyAmbiguousTrackSequences(bool Flag) { m_RejectPurelyAmbiguousTrackSequences = Flag; }
  bool GetRejectPurelyAmbiguousTrackSequences() { return m_RejectPurelyAmbiguousTrackSequences; }

  void SetNLayersForVertexSearch(int Value) { m_NLayersForVertexSearch = Value; }
  int GetNLayersForVertexSearch() { return m_NLayersForVertexSearch; }



  void SetAssumeD1First(bool Flag) { m_AssumeD1First = Flag; }
  bool GetAssumeD1First() { return m_AssumeD1First; }
  void SetClassicUndecidedHandling(int Flag) { m_ClassicUndecidedHandling = Flag; }
  int GetClassicUndecidedHandling() { return m_ClassicUndecidedHandling; }
  void SetAssumeTrackTopBottom(bool Flag) { m_AssumeTrackTopBottom = Flag; }
  bool GetAssumeTrackTopBottom() { return m_AssumeTrackTopBottom; }

  void SetUseComptelTypeEvents(bool Flag) { m_UseComptelTypeEvents = Flag; }
  bool GetUseComptelTypeEvents() { return m_UseComptelTypeEvents; }
  void SetGuaranteeStartD1(bool Flag) { m_GuaranteeStartD1 = Flag; }
  bool GetGuaranteeStartD1() { return m_GuaranteeStartD1; }
  void SetRejectOneDetectorTypeOnlyEvents(bool Flag) { m_RejectOneDetectorTypeOnlyEvents = Flag; }
  bool GetRejectOneDetectorTypeOnlyEvents() { return m_RejectOneDetectorTypeOnlyEvents; }
  
  void SetCSRThresholdMin(double Value) { m_CSRThresholdMin = Value; }
  double GetCSRThresholdMin() { return m_CSRThresholdMin; }
  
  void SetCSRThresholdMax(double Value) { m_CSRThresholdMax = Value; }
  double GetCSRThresholdMax() { return m_CSRThresholdMax; }

  void SetCSRMaxNHits(int Value) { m_CSRMaxNHits = Value; }
  int GetCSRMaxNHits() { return m_CSRMaxNHits; }

  void SetBayesianElectronFileName(TString Name) { m_BayesianElectronFileName = Name; }
  TString GetBayesianElectronFileName() { return m_BayesianElectronFileName; }

  void SetBayesianComptonFileName(TString Name) { m_BayesianComptonFileName = Name; }
  TString GetBayesianComptonFileName() { return m_BayesianComptonFileName; }

  void SetOriginObjectsFileName(TString Name) { m_OriginObjectsFileName = Name; }
  TString GetOriginObjectsFileName() { return m_OriginObjectsFileName; }

  void SetLensCenter(MVector Value) { m_LensCenter = Value; }
  MVector GetLensCenter() { return m_LensCenter; }

  void SetFocalSpotCenter(MVector Value) { m_FocalSpotCenter = Value; }
  MVector GetFocalSpotCenter() { return m_FocalSpotCenter; }

  void SetDecayFileName(TString Name) { m_DecayFileName = Name; }
  TString GetDecayFileName() { return m_DecayFileName; }

  // General options
  void SetTotalEnergyMax(double Value) { m_TotalEnergyMax = Value; }
  double GetTotalEnergyMax() { return m_TotalEnergyMax; }
  void SetTotalEnergyMin(double Value) { m_TotalEnergyMin = Value; }
  double GetTotalEnergyMin() { return m_TotalEnergyMin; }
  
  void SetLeverArmMax(double Value) { m_LeverArmMax = Value; }
  double GetLeverArmMax() { return m_LeverArmMax; }
  void SetLeverArmMin(double Value) { m_LeverArmMin = Value; }
  double GetLeverArmMin() { return m_LeverArmMin; }
  
  void SetEventIdMax(int Value) { m_EventIdMax = Value; }
  int GetEventIdMax() { return m_EventIdMax; }
  void SetEventIdMin(int Value) { m_EventIdMin = Value; }
  int GetEventIdMin() { return m_EventIdMin; }

  void SetRejectAllBadEvents(bool Flag) { m_RejectAllBadEvents = Flag; }
  bool GetRejectAllBadEvents() { return m_RejectAllBadEvents; }


  void SetExportSpectrumBins(int Value) { m_ExportSpectrumBins = Value; }
  int GetExportSpectrumBins() const { return m_ExportSpectrumBins; }
  void SetExportSpectrumLog(bool Value) { m_ExportSpectrumLog = Value; }
  bool GetExportSpectrumLog() const { return m_ExportSpectrumLog; }
  void SetExportSpectrumMin(double Value) { m_ExportSpectrumMin = Value; }
  double GetExportSpectrumMin() const { return m_ExportSpectrumMin; }
  void SetExportSpectrumMax(double Value) { m_ExportSpectrumMax = Value; }
  double GetExportSpectrumMax() const { return m_ExportSpectrumMax; }
  void SetExportSpectrumFileName(TString Value) { m_ExportSpectrumFileName = Value; }
  TString GetExportSpectrumFileName() const { return m_ExportSpectrumFileName; }


  // protected members:
 protected:
  virtual void ReadDataLocal(FILE* File);
  virtual bool ReadDataLocal(MXmlDocument* Node);
  virtual bool SaveDataLocal(MXmlDocument* Node);


  // private members:
 private:
  unsigned int m_NJobs;

  int m_CoincidenceAlgorithm;
  int m_ClusteringAlgorithm;
  int m_TrackingAlgorithm;
  int m_CSRAlgorithm;
  int m_DecayAlgorithm;

  double m_CoincidenceWindow;

  double m_StandardClusterizerMinDistanceD1;
  double m_StandardClusterizerMinDistanceD2;
  double m_StandardClusterizerMinDistanceD3;
  double m_StandardClusterizerMinDistanceD4;
  double m_StandardClusterizerMinDistanceD5;
  double m_StandardClusterizerMinDistanceD6;

  double m_StandardClusterizerCenterIsReference;

  int m_AdjacentLevel;
  double m_AdjacentSigma;

  bool m_DoTracking;
  bool m_SearchPairs;
  bool m_SearchMIPs;
  bool m_SearchComptons;

  int m_MaxComptonJump;
  int m_NTrackSequencesToKeep;
  bool m_RejectPurelyAmbiguousTrackSequences;
  int m_NLayersForVertexSearch;

  bool m_AssumeD1First;
  int m_ClassicUndecidedHandling;

  bool m_AssumeTrackTopBottom;


  bool m_UseComptelTypeEvents;
  bool m_GuaranteeStartD1;
  bool m_RejectOneDetectorTypeOnlyEvents;
  
  double m_CSRThresholdMin;
  double m_CSRThresholdMax;
  int m_CSRMaxNHits;

  MVector m_LensCenter;
  MVector m_FocalSpotCenter;

  TString m_OriginObjectsFileName;

  TString m_DecayFileName;
  TString m_BayesianElectronFileName;
  TString m_BayesianComptonFileName;

  // General options:
  double m_TotalEnergyMin;
  double m_TotalEnergyMax;

  double m_LeverArmMin;
  double m_LeverArmMax;

  int m_EventIdMin;
  int m_EventIdMax;

  bool m_RejectAllBadEvents;


  // Options for exporting the spectrum
  int m_ExportSpectrumBins;
  bool m_ExportSpectrumLog;
  double m_ExportSpectrumMin;
  double m_ExportSpectrumMax;
  TString m_ExportSpectrumFileName;


#ifdef ___CINT___
 public:
  ClassDef(MGUIDataRevan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
