/*
 * MGUIDataRevan.cxx
 *
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MGUIDataRevan.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDataRevan.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIDataRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDataRevan::MGUIDataRevan(bool AutoLoad) : MGUIData()
{
  // default constructor
  m_DefaultFileName = TString(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".revan.cfg"));
  m_DataFileName = m_DefaultFileName;

  // General algorithm selection
  m_CoincidenceAlgorithm = 0;
  m_ClusteringAlgorithm = 2;
  m_TrackingAlgorithm = 0;
  m_CSRAlgorithm = 1;
  m_DecayAlgorithm = 0;

  // Coincidence
  m_CoincidenceWindow = 1E-6;

  // Clusterizing
  m_StandardClusterizerMinDistanceD1 = 0.2;
  m_StandardClusterizerMinDistanceD2 = 1.1;
  m_StandardClusterizerMinDistanceD3 = 0.3;
  m_StandardClusterizerMinDistanceD4 = 0.0;
  m_StandardClusterizerMinDistanceD5 = 0.2;
  m_StandardClusterizerMinDistanceD6 = 0.3;

  m_StandardClusterizerCenterIsReference = false;

  m_AdjacentLevel = 2;
  m_AdjacentSigma = -1;

  // Electron tracking
  m_DoTracking = true;
  m_SearchPairs = true;
  m_SearchMIPs = true;
  m_SearchComptons = true;

  m_MaxComptonJump = 2;
  m_NTrackSequencesToKeep = 1;
  m_RejectPurelyAmbiguousTrackSequences = false;
  m_NLayersForVertexSearch = 6;

  // Compton tracking
  m_RejectOneDetectorTypeOnlyEvents = false;
  m_GuaranteeStartD1 = false;
  m_UseComptelTypeEvents = true;

  m_ClassicUndecidedHandling = 0; 

  m_AssumeD1First = false;
  m_AssumeTrackTopBottom = false;
  
  m_CSRThresholdMin = 0;
  m_CSRThresholdMax = 1000;

  m_CSRMaxNHits = 5;

  m_LensCenter = MVector(0.0, 0.0, 10000.0);
  m_FocalSpotCenter = MVector(0.0, 0.0, 0.0);

  m_OriginObjectsFileName = "";
  m_DecayFileName = "";
  m_BayesianComptonFileName = "";
  m_BayesianElectronFileName = "";

  // General options:
  m_TotalEnergyMin = 0;
  m_TotalEnergyMax = 1000000;

  m_LeverArmMin = 0;
  m_LeverArmMax = 1000;

  m_EventIdMin = -1;
  m_EventIdMax = -1;

  m_RejectAllBadEvents = true;

  m_NJobs = 1;

  // Options for exporting the spectrum
  m_ExportSpectrumBins = 100;
  m_ExportSpectrumLog = false;
  m_ExportSpectrumMin = 0.0;
  m_ExportSpectrumMax = 1000.0;
  m_ExportSpectrumFileName = "Export.dat";


  m_MasterNodeName = "RevanConfigurationFile";

  if (AutoLoad == true) {
    ReadData();
  }
}

////////////////////////////////////////////////////////////////////////////////


MGUIDataRevan::~MGUIDataRevan()
{
  // default destructor

  SaveData();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIDataRevan::ReadDataLocal(FILE* Input)
{
  // Load the configuration-file

  const int LineLength = 1000;
  char LineBuffer[LineLength];
  TString Str;

  // Step two find keywords and the belonging data:
  while (fgets(LineBuffer, LineLength, Input) != NULL) {

    if (strstr(LineBuffer, "[ClusteringAlgorithm]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_ClusteringAlgorithm);
    }
  
    if (strstr(LineBuffer, "[TrackingAlgorithm]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_TrackingAlgorithm);
    }
    
    if (strstr(LineBuffer, "[CSRAlgorithm]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_CSRAlgorithm);
    }
    
    if (strstr(LineBuffer, "[DecayAlgorithm]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_DecayAlgorithm);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD1]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD1);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD2]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD2);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD3]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD3);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD4]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD4);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD5]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD5);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerMinDistanceD6]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_StandardClusterizerMinDistanceD6);
    }
  
    if (strstr(LineBuffer, "[StandardClusterizerCenterIsReference]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int i = 0;
      sscanf(LineBuffer, "%i\n", &i);
      m_StandardClusterizerCenterIsReference = bool(i);
    }
  
    if (strstr(LineBuffer, "[AdjacentSigma]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf\n", &m_AdjacentSigma);
    }
  
    if (strstr(LineBuffer, "[AdjacentLevel]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_AdjacentLevel);
    }
  
    if (strstr(LineBuffer, "[DoTracking]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_DoTracking = false : m_DoTracking = true;
    }
  
    if (strstr(LineBuffer, "[SearchPairs]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_SearchPairs = false : m_SearchPairs = true;
    }
  
    if (strstr(LineBuffer, "[SearchMIPs]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_SearchMIPs = false : m_SearchMIPs = true;
    }
    
    if (strstr(LineBuffer, "[SearchComptons]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_SearchComptons = false : m_SearchComptons = true;
    }
    
    if (strstr(LineBuffer, "[MaxComptonJump]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_MaxComptonJump);
    }
    
    if (strstr(LineBuffer, "[NTrackSequencesToKeep]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_NTrackSequencesToKeep);
    }

    if (strstr(LineBuffer, "[RejectPurelyAmbiguousTrackSequences]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_RejectPurelyAmbiguousTrackSequences = false : m_RejectPurelyAmbiguousTrackSequences = true;
    }

    if (strstr(LineBuffer, "[AssumeD1First]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_AssumeD1First = false : m_AssumeD1First = true;
    }
  
    if (strstr(LineBuffer, "[ClassicUndecidedHandling]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_ClassicUndecidedHandling);
    }
  
    if (strstr(LineBuffer, "[AssumeTrackTopBottom]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_AssumeTrackTopBottom = false : m_AssumeTrackTopBottom = true;
    }
    
    if (strstr(LineBuffer, "[UseComptelTypeEvents]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_UseComptelTypeEvents = false : m_UseComptelTypeEvents = true;
    }
    
    if (strstr(LineBuffer, "[GuaranteeStartD1]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_GuaranteeStartD1 = false : m_GuaranteeStartD1 = true;
    }
    
    if (strstr(LineBuffer, "[RejectOneDetectorTypeOnlyEvents]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_RejectOneDetectorTypeOnlyEvents = false : m_RejectOneDetectorTypeOnlyEvents = true;
    }
    
    if (strstr(LineBuffer, "[CSRThreshold]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf %lf\n", &m_CSRThresholdMin, &m_CSRThresholdMax);
    }
  
    if (strstr(LineBuffer, "[CSRMaxNHits]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i\n", &m_CSRMaxNHits);
    }
    
    if (strstr(LineBuffer, "[LensCenter]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      double x, y, z;
      sscanf(LineBuffer, "%lf %lf %lf\n", &x, &y, &z);
      m_LensCenter.SetXYZ(x, y, z);
    }
    
    if (strstr(LineBuffer, "[FocalSpotCenter]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      double x, y, z;
      sscanf(LineBuffer, "%lf %lf %lf\n", &x, &y, &z);
      m_FocalSpotCenter.SetXYZ(x, y, z);
    }

    if (strstr(LineBuffer, "[OriginObjectsFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_OriginObjectsFileName = Str;
    }

    if (strstr(LineBuffer, "[DecayFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_DecayFileName = Str;
    }

    if (strstr(LineBuffer, "[BayesianElectronFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_BayesianElectronFileName = Str;
    }

    if (strstr(LineBuffer, "[BayesianComptonFile]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      Str = TString(LineBuffer);
      Str = Str.Remove(Str.First('\n'));
      m_BayesianComptonFileName = Str;
    }
  
    if (strstr(LineBuffer, "[TotalEnergy]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf %lf\n", &m_TotalEnergyMin, &m_TotalEnergyMax);
    }
    
    if (strstr(LineBuffer, "[LeverArm]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%lf %lf\n", &m_LeverArmMin, &m_LeverArmMax);
    }

    if (strstr(LineBuffer, "[EventId]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      sscanf(LineBuffer, "%i %i\n", &m_EventIdMin, &m_EventIdMax);
    }      
    
    if (strstr(LineBuffer, "[RejectAllBadEvents]") != NULL) {
      if (fgets(LineBuffer, LineLength, Input) == 0)
        break;
      int Boolean = 0;
      sscanf(LineBuffer, "%i\n", &Boolean);
      (Boolean == 0) ? m_RejectAllBadEvents = false : m_RejectAllBadEvents = true;
    }
  }
}

 
////////////////////////////////////////////////////////////////////////////////


bool MGUIDataRevan::UpdateRawEventAnalyzer(MRawEventAnalyzer& Analyzer)
{
  // Transfer all necessary data to the analyzer...

  Analyzer.SetCoincidenceAlgorithm(GetCoincidenceAlgorithm()); 
  Analyzer.SetClusteringAlgorithm(GetClusteringAlgorithm()); 
  Analyzer.SetTrackingAlgorithm(GetTrackingAlgorithm()); 
  Analyzer.SetCSRAlgorithm(GetCSRAlgorithm());
  Analyzer.SetDecayAlgorithm(GetDecayAlgorithm());

  // coincidence
  Analyzer.SetCoincidenceWindow(GetCoincidenceWindow());

  // clustering
  Analyzer.SetStandardClusterizerMinDistanceD1(GetStandardClusterizerMinDistanceD1());
  Analyzer.SetStandardClusterizerMinDistanceD2(GetStandardClusterizerMinDistanceD2());
  Analyzer.SetStandardClusterizerMinDistanceD3(GetStandardClusterizerMinDistanceD3());
  Analyzer.SetStandardClusterizerMinDistanceD4(GetStandardClusterizerMinDistanceD4());
  Analyzer.SetStandardClusterizerMinDistanceD5(GetStandardClusterizerMinDistanceD5());
  Analyzer.SetStandardClusterizerMinDistanceD6(GetStandardClusterizerMinDistanceD6());
  Analyzer.SetStandardClusterizerCenterIsReference(GetStandardClusterizerCenterIsReference());
   
  Analyzer.SetAdjacentLevel(GetAdjacentLevel());
  Analyzer.SetAdjacentSigma(GetAdjacentSigma());

  // electron tracking
  Analyzer.SetSearchPairTracks(GetSearchPairs());
  Analyzer.SetSearchMIPTracks(GetSearchMIPs());
  Analyzer.SetSearchComptonTracks(GetSearchComptons());

  Analyzer.SetBETFileName(GetBayesianElectronFileName());

  Analyzer.SetMaxComptonJump(GetMaxComptonJump());
  Analyzer.SetNTrackSequencesToKeep(GetNTrackSequencesToKeep());
  Analyzer.SetRejectPurelyAmbiguousTrackSequences(GetRejectPurelyAmbiguousTrackSequences());
  Analyzer.SetNLayersForVertexSearch(GetNLayersForVertexSearch());
  
  // compton tracking:
  Analyzer.SetAssumeD1First(GetAssumeD1First());
  Analyzer.SetClassicUndecidedHandling(GetClassicUndecidedHandling());
  Analyzer.SetUseComptelTypeEvents(GetUseComptelTypeEvents());
  Analyzer.SetGuaranteeStartD1(GetGuaranteeStartD1());

  Analyzer.SetRejectOneDetectorTypeOnlyEvents(GetRejectOneDetectorTypeOnlyEvents());

  Analyzer.SetCSRThresholdMin(GetCSRThresholdMin());
  Analyzer.SetCSRThresholdMax(GetCSRThresholdMax());

  Analyzer.SetCSRMaxNHits(GetCSRMaxNHits());

  Analyzer.SetOriginObjectsFileName(GetOriginObjectsFileName());

  Analyzer.SetBCTFileName(GetBayesianComptonFileName());

  Analyzer.SetLensCenter(GetLensCenter());
  Analyzer.SetFocalSpotCenter(GetFocalSpotCenter());

  // decay:
  Analyzer.SetDecayFileName(GetDecayFileName());

  // global options:
  Analyzer.SetTotalEnergyMax(GetTotalEnergyMax());
  Analyzer.SetTotalEnergyMin(GetTotalEnergyMin());

  Analyzer.SetLeverArmMax(GetLeverArmMax());
  Analyzer.SetLeverArmMin(GetLeverArmMin());

  Analyzer.SetEventIdMax(GetEventIdMax());
  Analyzer.SetEventIdMin(GetEventIdMin());

  Analyzer.SetRejectAllBadEvents(GetRejectAllBadEvents());

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MGUIDataRevan::SaveDataLocal(MXmlDocument* Document)
{
  new MXmlNode(Document, "NJobs", m_NJobs);

  new MXmlNode(Document, "CoincidenceAlgorithm", m_CoincidenceAlgorithm);
  new MXmlNode(Document, "ClusteringAlgorithm", m_ClusteringAlgorithm);
  new MXmlNode(Document, "TrackingAlgorithm", m_TrackingAlgorithm);
  new MXmlNode(Document, "CSRAlgorithm", m_CSRAlgorithm);
  new MXmlNode(Document, "DecayAlgorithm", m_DecayAlgorithm);
  new MXmlNode(Document, "CoincidenceWindow", m_CoincidenceWindow);

  new MXmlNode(Document, "StandardClusterizerMinDistanceD1", m_StandardClusterizerMinDistanceD1);
  new MXmlNode(Document, "StandardClusterizerMinDistanceD2", m_StandardClusterizerMinDistanceD2);
  new MXmlNode(Document, "StandardClusterizerMinDistanceD3", m_StandardClusterizerMinDistanceD3);
  new MXmlNode(Document, "StandardClusterizerMinDistanceD4", m_StandardClusterizerMinDistanceD4);
  new MXmlNode(Document, "StandardClusterizerMinDistanceD5", m_StandardClusterizerMinDistanceD5);
  new MXmlNode(Document, "StandardClusterizerMinDistanceD6", m_StandardClusterizerMinDistanceD6);
  new MXmlNode(Document, "StandardClusterizerCenterIsReference", m_StandardClusterizerCenterIsReference);
  new MXmlNode(Document, "AdjacentSigma", m_AdjacentSigma);
  new MXmlNode(Document, "AdjacentLevel", m_AdjacentLevel);

  new MXmlNode(Document, "DoTracking", m_DoTracking);
  new MXmlNode(Document, "SearchPairs", m_SearchPairs);
  new MXmlNode(Document, "SearchMIPs", m_SearchMIPs);
  new MXmlNode(Document, "SearchComptons", m_SearchComptons);
  new MXmlNode(Document, "MaxComptonJump", m_MaxComptonJump);
  new MXmlNode(Document, "NTrackSequencesToKeep", m_NTrackSequencesToKeep);
  new MXmlNode(Document, "RejectPurelyAmbiguousTrackSequences", m_RejectPurelyAmbiguousTrackSequences);
  new MXmlNode(Document, "NLayersForVertexSearch", m_NLayersForVertexSearch);
  
  new MXmlNode(Document, "AssumeD1First", m_AssumeD1First);
  new MXmlNode(Document, "ClassicUndecidedHandling", m_ClassicUndecidedHandling);
  new MXmlNode(Document, "AssumeTrackTopBottom", m_AssumeTrackTopBottom);
  new MXmlNode(Document, "UseComptelTypeEvents", m_UseComptelTypeEvents);
  new MXmlNode(Document, "GuaranteeStartD1", m_GuaranteeStartD1);
  new MXmlNode(Document, "RejectOneDetectorTypeOnlyEvents", m_RejectOneDetectorTypeOnlyEvents);

  new MXmlNode(Document, "CSRThreshold", m_CSRThresholdMin, m_CSRThresholdMax);
  new MXmlNode(Document, "CSRMaxNHits", m_CSRMaxNHits);
  new MXmlNode(Document, "LensCenter", m_LensCenter);
  new MXmlNode(Document, "FocalSpotCenter", m_FocalSpotCenter);
  new MXmlNode(Document, "OriginObjectsFile", m_OriginObjectsFileName);
  new MXmlNode(Document, "DecayFile", m_DecayFileName);
  new MXmlNode(Document, "BayesianComptonFile", m_BayesianComptonFileName);
  new MXmlNode(Document, "BayesianElectronFile", m_BayesianElectronFileName);

  new MXmlNode(Document, "TotalEnergy", m_TotalEnergyMin, m_TotalEnergyMax);
  new MXmlNode(Document, "LeverArm", m_LeverArmMin, m_LeverArmMax);
  new MXmlNode(Document, "EventId", m_EventIdMin, m_EventIdMax);
  new MXmlNode(Document, "RejectAllBadEvents", m_RejectAllBadEvents);

  new MXmlNode(Document, "ExportSpectrumBins", m_ExportSpectrumBins);
  new MXmlNode(Document, "ExportSpectrumLog", m_ExportSpectrumLog);
  new MXmlNode(Document, "ExportSpectrumMin", m_ExportSpectrumMin);
  new MXmlNode(Document, "ExportSpectrumMax", m_ExportSpectrumMax);
  new MXmlNode(Document, "ExportSpectrumFileName", m_ExportSpectrumFileName);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataRevan::ReadDataLocal(MXmlDocument* Document)
{  
  MXmlNode* aNode = 0;

  if ((aNode = Document->GetNode("NJobs")) != 0) {
    m_NJobs = aNode->GetValueAsUnsignedInt();
  }
  if ((aNode = Document->GetNode("ClusteringAlgorithm")) != 0) {
    m_ClusteringAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("CoincidenceAlgorithm")) != 0) {
    m_CoincidenceAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("TrackingAlgorithm")) != 0) {
    m_TrackingAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("CSRAlgorithm")) != 0) {
    m_CSRAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("DecayAlgorithm")) != 0) {
    m_DecayAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("CoincidenceWindow")) != 0) {
    m_CoincidenceWindow = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD1")) != 0) {
    m_StandardClusterizerMinDistanceD1 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD2")) != 0) {
    m_StandardClusterizerMinDistanceD2 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD3")) != 0) {
    m_StandardClusterizerMinDistanceD3 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD4")) != 0) {
    m_StandardClusterizerMinDistanceD4 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD5")) != 0) {
    m_StandardClusterizerMinDistanceD5 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerMinDistanceD6")) != 0) {
    m_StandardClusterizerMinDistanceD6 = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("StandardClusterizerCenterIsReference")) != 0) {
    m_StandardClusterizerCenterIsReference = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("AdjacentSigma")) != 0) {
    m_AdjacentSigma = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("AdjacentLevel")) != 0) {
    m_AdjacentLevel = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("DoTracking")) != 0) {
    m_DoTracking = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("SearchPairs")) != 0) {
    m_SearchPairs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("SearchMIPs")) != 0) {
    m_SearchMIPs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("SearchComptons")) != 0) {
    m_SearchComptons = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("MaxComptonJump")) != 0) {
    m_MaxComptonJump = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("NTrackSequencesToKeep")) != 0) {
    m_NTrackSequencesToKeep = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("RejectPurelyAmbiguousTrackSequences")) != 0) {
    m_RejectPurelyAmbiguousTrackSequences = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("NLayersForVertexSearch")) != 0) {
    m_NLayersForVertexSearch = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("AssumeD1First")) != 0) {
    m_AssumeD1First = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("ClassicUndecidedHandling")) != 0) {
    m_ClassicUndecidedHandling = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("AssumeTrackTopBottom")) != 0) {
    m_AssumeTrackTopBottom = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("UseComptelTypeEvents")) != 0) {
    m_UseComptelTypeEvents = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("GuaranteeStartD1")) != 0) {
    m_GuaranteeStartD1 = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("RejectOneDetectorTypeOnlyEvents")) != 0) {
    m_RejectOneDetectorTypeOnlyEvents = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("CSRThreshold")) != 0) {
    m_CSRThresholdMin = aNode->GetMinValueAsDouble();
    m_CSRThresholdMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Document->GetNode("CSRMaxNHits")) != 0) {
    m_CSRMaxNHits = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("LensCenter")) != 0) {
    m_LensCenter = aNode->GetValueAsVector();
  }
  if ((aNode = Document->GetNode("FocalSpotCenter")) != 0) {
    m_FocalSpotCenter = aNode->GetValueAsVector();
  }
  if ((aNode = Document->GetNode("OriginObjectsFile")) != 0) {
    m_OriginObjectsFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("DecayFile")) != 0) {
    m_DecayFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("BayesianComptonFile")) != 0) {
    m_BayesianComptonFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("BayesianElectronFile")) != 0) {
    m_BayesianElectronFileName = aNode->GetValueAsString();
  }
  if ((aNode = Document->GetNode("TotalEnergy")) != 0) {
    m_TotalEnergyMin = aNode->GetMinValueAsDouble();
    m_TotalEnergyMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Document->GetNode("LeverArm")) != 0) {
    m_LeverArmMin = aNode->GetMinValueAsDouble();
    m_LeverArmMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Document->GetNode("EventId")) != 0) {
    m_EventIdMin = aNode->GetMinValueAsInt();
    m_EventIdMax = aNode->GetMaxValueAsInt();
  }
  if ((aNode = Document->GetNode("RejectAllBadEvents")) != 0) {
    m_RejectAllBadEvents = aNode->GetValueAsBoolean();
  }

  if ((aNode = Document->GetNode("ExportSpectrumBins")) != 0) {
    m_ExportSpectrumBins = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("ExportSpectrumLog")) != 0) {
    m_ExportSpectrumLog = aNode->GetValueAsBoolean();
  }
  if ((aNode = Document->GetNode("ExportSpectrumMin")) != 0) {
    m_ExportSpectrumMin = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("ExportSpectrumMax")) != 0) {
    m_ExportSpectrumMax = aNode->GetValueAsDouble();
  }
  if ((aNode = Document->GetNode("ExportSpectrumFileName")) != 0) {
    m_ExportSpectrumFileName = aNode->GetValueAsString();
  }

  return true;
}
  

// MGUIDataRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
