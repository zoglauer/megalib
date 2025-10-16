/*
 * MSettingsEventReconstruction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsEventReconstruction__
#define __MSettingsEventReconstruction__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsInterface.h"
#include "MERCSRTMVAMethods.h"


// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


class MSettingsEventReconstruction : public MSettingsInterface
{
  // public Session:
 public:
  MSettingsEventReconstruction();
  virtual ~MSettingsEventReconstruction();

  void SetSaveOI(bool SaveOI) { m_SaveOI = SaveOI; }
  bool GetSaveOI() const { return m_SaveOI; }

  void SetNJobs(unsigned int ID) { m_NJobs = ID; }
  unsigned int GetNJobs() { return m_NJobs; }

  void SetCoincidenceAlgorithm(int ID) { m_CoincidenceAlgorithm = ID; }
  int GetCoincidenceAlgorithm() { return m_CoincidenceAlgorithm; }
  
  void SetEventClusteringAlgorithm(int ID) { m_EventClusteringAlgorithm = ID; }
  int GetEventClusteringAlgorithm() { return m_EventClusteringAlgorithm; }
  
  void SetHitClusteringAlgorithm(int ID) { m_HitClusteringAlgorithm = ID; }
  int GetHitClusteringAlgorithm() { return m_HitClusteringAlgorithm; }
  
  void SetTrackingAlgorithm(int ID) { m_TrackingAlgorithm = ID; }
  int GetTrackingAlgorithm() { return m_TrackingAlgorithm; }

  void SetPairAlgorithm(int ID) { m_PairAlgorithm = ID; }
  int GetPairAlgorithm() { return m_PairAlgorithm; }

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
  
  
  
  void SetEventClusteringDistanceCutOff(double Distance) { m_EventClusteringDistanceCutOff = Distance; }
  double GetEventClusteringDistanceCutOff() { return m_EventClusteringDistanceCutOff; }
  
  void SetEventClusteringTMVAFileName(MString Name) { m_EventClusteringTMVAFileName = Name; }
  MString GetEventClusteringTMVAFileName() { return m_EventClusteringTMVAFileName; }
  
  void SetEventClusteringTMVAMethods(MERCSRTMVAMethods EventClusteringTMVAMethods) { m_EventClusteringTMVAMethods = EventClusteringTMVAMethods; }
  MERCSRTMVAMethods GetEventClusteringTMVAMethods() { return m_EventClusteringTMVAMethods; }
  
  
  
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

  void SetStandardClusterizerMinDistanceD7(double MinDistance) {
    m_StandardClusterizerMinDistanceD7 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD7() {
    return m_StandardClusterizerMinDistanceD7;
  }

  void SetStandardClusterizerMinDistanceD8(double MinDistance) {
    m_StandardClusterizerMinDistanceD8 = MinDistance;
  }
  double GetStandardClusterizerMinDistanceD8() {
    return m_StandardClusterizerMinDistanceD8;
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

  void SetPDFClusterizerBaseFileName(MString BaseFileName) {
    m_PDFClusterizerBaseFileName = BaseFileName;
  }
  MString GetPDFClusterizerBaseFileName() const {
    return m_PDFClusterizerBaseFileName;
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

  void SetHeightX0(double Value) { m_HeightX0 = Value; }
  double GetHeightX0() { return m_HeightX0; }
  void SetSigmaHitPos(double Value) { m_SigmaHitPos = Value; }
  double GetSigmaHitPos() { return m_SigmaHitPos; }


  void RemoveAllElectronTrackingDetectors() { m_ElectronTrackingDetectors.clear(); }
  void AddElectronTrackingDetector(const MString& ElectronTracking) { m_ElectronTrackingDetectors.push_back(ElectronTracking); }
  unsigned int GetNElectronTrackingDetectors() const { return m_ElectronTrackingDetectors.size(); }
  MString GetElectronTrackingDetector(unsigned int d) const { if (d < m_ElectronTrackingDetectors.size()) return m_ElectronTrackingDetectors[d]; return ""; }
  vector<MString> GetElectronTrackingDetectors() const { return m_ElectronTrackingDetectors; }


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

  void SetBayesianElectronFileName(MString Name) { m_BayesianElectronFileName = Name; }
  MString GetBayesianElectronFileName() { return m_BayesianElectronFileName; }

  void SetBayesianComptonFileName(MString Name) { m_BayesianComptonFileName = Name; }
  MString GetBayesianComptonFileName() { return m_BayesianComptonFileName; }
  
  void SetCSRTMVAFileName(MString Name) { m_CSRTMVAFileName = Name; }
  MString GetCSRTMVAFileName() { return m_CSRTMVAFileName; }
  
  void SetCSRTMVAMethods(MERCSRTMVAMethods CSRTMVAMethods) { m_CSRTMVAMethods = CSRTMVAMethods; }
  MERCSRTMVAMethods GetCSRTMVAMethods() { return m_CSRTMVAMethods; }
  
  void SetOriginObjectsFileName(MString Name) { m_OriginObjectsFileName = Name; }
  MString GetOriginObjectsFileName() { return m_OriginObjectsFileName; }

  void SetLensCenter(MVector Value) { m_LensCenter = Value; }
  MVector GetLensCenter() { return m_LensCenter; }

  void SetFocalSpotCenter(MVector Value) { m_FocalSpotCenter = Value; }
  MVector GetFocalSpotCenter() { return m_FocalSpotCenter; }

  void SetDecayFileName(MString Name) { m_DecayFileName = Name; }
  MString GetDecayFileName() { return m_DecayFileName; }

  // General options
  void SetTotalEnergyMax(double Value) { m_TotalEnergyMax = Value; }
  double GetTotalEnergyMax() { return m_TotalEnergyMax; }
  void SetTotalEnergyMin(double Value) { m_TotalEnergyMin = Value; }
  double GetTotalEnergyMin() { return m_TotalEnergyMin; }

  void SetLeverArmMax(double Value) { m_LeverArmMax = Value; }
  double GetLeverArmMax() { return m_LeverArmMax; }
  void SetLeverArmMin(double Value) { m_LeverArmMin = Value; }
  double GetLeverArmMin() { return m_LeverArmMin; }

  void SetEventIdMax(long Value) { m_EventIdMax = Value; }
  long GetEventIdMax() { return m_EventIdMax; }
  void SetEventIdMin(long Value) { m_EventIdMin = Value; }
  long GetEventIdMin() { return m_EventIdMin; }

  void SetRejectAllBadEvents(bool Flag) { m_RejectAllBadEvents = Flag; }
  bool GetRejectAllBadEvents() { return m_RejectAllBadEvents; }


  //! Set the special GUI mode (this is not saved to file!)
  void SetSpecialMode(const bool SpecialMode) { m_SpecialMode = SpecialMode; }
  //! Get the special GUI mode (this is not saved to file!)
  bool GetSpecialMode() const { return m_SpecialMode; }


  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 protected:
  unsigned int m_NJobs;

  //! Save the OI information
  bool m_SaveOI;

  // Reconstruction options:
  int m_CoincidenceAlgorithm;
  int m_EventClusteringAlgorithm;
  int m_HitClusteringAlgorithm;
  int m_TrackingAlgorithm;
  int m_PairAlgorithm;
  int m_CSRAlgorithm;
  int m_DecayAlgorithm;

  // Coincidence
  double m_CoincidenceWindow;

  // Event clustering
  
  //! The distance cut off
  double m_EventClusteringDistanceCutOff;
  
  //! The TMVA file name
  MString m_EventClusteringTMVAFileName;
  //! The TMVA methods
  MERCSRTMVAMethods m_EventClusteringTMVAMethods;
  
  // Hit clustering:
  double m_StandardClusterizerMinDistanceD1;
  double m_StandardClusterizerMinDistanceD2;
  double m_StandardClusterizerMinDistanceD3;
  double m_StandardClusterizerMinDistanceD4;
  double m_StandardClusterizerMinDistanceD5;
  double m_StandardClusterizerMinDistanceD6;
  double m_StandardClusterizerMinDistanceD7;
  double m_StandardClusterizerMinDistanceD8;

  double m_StandardClusterizerCenterIsReference;

  int m_AdjacentLevel;
  double m_AdjacentSigma;

  MString m_PDFClusterizerBaseFileName;

  // Electron tracking:
  bool m_DoTracking;
  bool m_SearchPairs;
  bool m_SearchMIPs;
  bool m_SearchComptons;

  int m_MaxComptonJump;
  int m_NTrackSequencesToKeep;
  bool m_RejectPurelyAmbiguousTrackSequences;
  int m_NLayersForVertexSearch;
  double m_HeightX0;
  double m_SigmaHitPos;

  MString m_BayesianElectronFileName;

  vector<MString> m_ElectronTrackingDetectors;

  // Compton tracking:
  bool m_AssumeD1First;
  int m_ClassicUndecidedHandling;
  bool m_AssumeTrackTopBottom;
  bool m_UseComptelTypeEvents;
  bool m_GuaranteeStartD1;
  bool m_RejectOneDetectorTypeOnlyEvents;

  double m_CSRThresholdMin;
  double m_CSRThresholdMax;
  int m_CSRMaxNHits;

  MString m_BayesianComptonFileName;

  //! The TMVA file name
  MString m_CSRTMVAFileName;
  //! The TMVA methods

  MERCSRTMVAMethods m_CSRTMVAMethods;

  MVector m_LensCenter;
  MVector m_FocalSpotCenter;
  MString m_OriginObjectsFileName;

  // Decay
  MString m_DecayFileName;

  // General options:
  double m_TotalEnergyMin;
  double m_TotalEnergyMax;

  double m_LeverArmMin;
  double m_LeverArmMax;

  long m_EventIdMin;
  long m_EventIdMax;

  bool m_RejectAllBadEvents;


  //! The special GUI mode flag
  bool m_SpecialMode;


#ifdef ___CLING___
 public:
  ClassDef(MSettingsEventReconstruction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
