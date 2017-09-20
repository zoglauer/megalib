/*
 * MSettingsEventReconstruction.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MSettingsEventReconstruction.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSettingsEventReconstruction.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSettingsEventReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MSettingsEventReconstruction::MSettingsEventReconstruction() : MSettingsInterface()
{
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
  m_StandardClusterizerMinDistanceD7 = 0.3;
  m_StandardClusterizerMinDistanceD8 = 0.3;

  m_StandardClusterizerCenterIsReference = false;

  m_AdjacentLevel = 2;
  m_AdjacentSigma = -1;

  m_PDFClusterizerBaseFileName = "";

  // Electron tracking
  m_DoTracking = true;
  m_SearchPairs = true;
  m_SearchMIPs = true;
  m_SearchComptons = true;

  m_MaxComptonJump = 2;
  m_NTrackSequencesToKeep = 1;
  m_RejectPurelyAmbiguousTrackSequences = false;
  m_NLayersForVertexSearch = 6;

  m_ElectronTrackingDetectors.clear();
  
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

  m_TMVAFileName = "";
  
  // General options:
  m_TotalEnergyMin = 0;
  m_TotalEnergyMax = 1000000;

  m_LeverArmMin = 0;
  m_LeverArmMax = 1000;

  m_EventIdMin = -1;
  m_EventIdMax = -1;

  m_RejectAllBadEvents = true;

  m_NJobs = 1;
  m_SaveOI = false;

  m_SpecialMode = false;
  
  m_TMVAMethods.SetUsedMethods("BDTD");
}

////////////////////////////////////////////////////////////////////////////////


MSettingsEventReconstruction::~MSettingsEventReconstruction()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsEventReconstruction::WriteXml(MXmlNode* Node)
{
  new MXmlNode(Node, "NJobs", m_NJobs);
  // Not stored! new MXmlNode(Node, "SaveOI", m_SaveOI);

  new MXmlNode(Node, "CoincidenceAlgorithm", m_CoincidenceAlgorithm);
  new MXmlNode(Node, "ClusteringAlgorithm", m_ClusteringAlgorithm);
  new MXmlNode(Node, "TrackingAlgorithm", m_TrackingAlgorithm);
  new MXmlNode(Node, "CSRAlgorithm", m_CSRAlgorithm);
  new MXmlNode(Node, "DecayAlgorithm", m_DecayAlgorithm);
  new MXmlNode(Node, "CoincidenceWindow", m_CoincidenceWindow);

  new MXmlNode(Node, "StandardClusterizerMinDistanceD1", m_StandardClusterizerMinDistanceD1);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD2", m_StandardClusterizerMinDistanceD2);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD3", m_StandardClusterizerMinDistanceD3);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD4", m_StandardClusterizerMinDistanceD4);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD5", m_StandardClusterizerMinDistanceD5);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD6", m_StandardClusterizerMinDistanceD6);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD7", m_StandardClusterizerMinDistanceD7);
  new MXmlNode(Node, "StandardClusterizerMinDistanceD8", m_StandardClusterizerMinDistanceD8);
  new MXmlNode(Node, "StandardClusterizerCenterIsReference", m_StandardClusterizerCenterIsReference);
  new MXmlNode(Node, "AdjacentSigma", m_AdjacentSigma);
  new MXmlNode(Node, "AdjacentLevel", m_AdjacentLevel);
  new MXmlNode(Node, "PDFClusterizerBaseFileName", m_PDFClusterizerBaseFileName);

  new MXmlNode(Node, "DoTracking", m_DoTracking);
  new MXmlNode(Node, "SearchPairs", m_SearchPairs);
  new MXmlNode(Node, "SearchMIPs", m_SearchMIPs);
  new MXmlNode(Node, "SearchComptons", m_SearchComptons);
  new MXmlNode(Node, "MaxComptonJump", m_MaxComptonJump);
  new MXmlNode(Node, "NTrackSequencesToKeep", m_NTrackSequencesToKeep);
  new MXmlNode(Node, "RejectPurelyAmbiguousTrackSequences", m_RejectPurelyAmbiguousTrackSequences);
  new MXmlNode(Node, "NLayersForVertexSearch", m_NLayersForVertexSearch);
  
  MXmlNode* aNode = new MXmlNode(Node, "ElectronTrackingDetectors");
  for (unsigned int i = 0; i < m_ElectronTrackingDetectors.size(); ++i) {
    new MXmlNode(aNode, "ElectronTrackingDetector",  m_ElectronTrackingDetectors[i]);
  }

  
  new MXmlNode(Node, "AssumeD1First", m_AssumeD1First);
  new MXmlNode(Node, "ClassicUndecidedHandling", m_ClassicUndecidedHandling);
  new MXmlNode(Node, "AssumeTrackTopBottom", m_AssumeTrackTopBottom);
  new MXmlNode(Node, "UseComptelTypeEvents", m_UseComptelTypeEvents);
  new MXmlNode(Node, "GuaranteeStartD1", m_GuaranteeStartD1);
  new MXmlNode(Node, "RejectOneDetectorTypeOnlyEvents", m_RejectOneDetectorTypeOnlyEvents);

  new MXmlNode(Node, "CSRThreshold", m_CSRThresholdMin, m_CSRThresholdMax);
  new MXmlNode(Node, "CSRMaxNHits", m_CSRMaxNHits);
  new MXmlNode(Node, "LensCenter", m_LensCenter);
  new MXmlNode(Node, "FocalSpotCenter", m_FocalSpotCenter);
  new MXmlNode(Node, "OriginObjectsFile", CleanPath(m_OriginObjectsFileName));
  new MXmlNode(Node, "DecayFile", CleanPath(m_DecayFileName));
  
  new MXmlNode(Node, "BayesianComptonFile", CleanPath(m_BayesianComptonFileName));
  new MXmlNode(Node, "BayesianElectronFile", CleanPath(m_BayesianElectronFileName));
  
  new MXmlNode(Node, "TMVAFile", CleanPath(m_TMVAFileName));
  new MXmlNode(Node, "TMVAMethods", m_TMVAMethods.GetUsedMethodsString());
  
  new MXmlNode(Node, "TotalEnergy", m_TotalEnergyMin, m_TotalEnergyMax);
  new MXmlNode(Node, "LeverArm", m_LeverArmMin, m_LeverArmMax);
  new MXmlNode(Node, "EventId", m_EventIdMin, m_EventIdMax);
  new MXmlNode(Node, "RejectAllBadEvents", m_RejectAllBadEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSettingsEventReconstruction::ReadXml(MXmlNode* Node)
{  
  MXmlNode* aNode = 0;
  MXmlNode* bNode = 0;

  if ((aNode = Node->GetNode("NJobs")) != 0) {
    m_NJobs = aNode->GetValueAsUnsignedInt();
  }
  /* No reading!
  if ((aNode = Node->GetNode("SaveOI")) != 0) {
    m_Save = aNode->GetValueAsBoolean();
  }
  */
  
  if ((aNode = Node->GetNode("ClusteringAlgorithm")) != 0) {
    m_ClusteringAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("CoincidenceAlgorithm")) != 0) {
    m_CoincidenceAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("TrackingAlgorithm")) != 0) {
    m_TrackingAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("CSRAlgorithm")) != 0) {
    m_CSRAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("DecayAlgorithm")) != 0) {
    m_DecayAlgorithm = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("CoincidenceWindow")) != 0) {
    m_CoincidenceWindow = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD1")) != 0) {
    m_StandardClusterizerMinDistanceD1 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD2")) != 0) {
    m_StandardClusterizerMinDistanceD2 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD3")) != 0) {
    m_StandardClusterizerMinDistanceD3 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD4")) != 0) {
    m_StandardClusterizerMinDistanceD4 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD5")) != 0) {
    m_StandardClusterizerMinDistanceD5 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD6")) != 0) {
    m_StandardClusterizerMinDistanceD6 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD7")) != 0) {
    m_StandardClusterizerMinDistanceD7 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerMinDistanceD8")) != 0) {
    m_StandardClusterizerMinDistanceD8 = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("StandardClusterizerCenterIsReference")) != 0) {
    m_StandardClusterizerCenterIsReference = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("AdjacentSigma")) != 0) {
    m_AdjacentSigma = aNode->GetValueAsDouble();
  }
  if ((aNode = Node->GetNode("AdjacentLevel")) != 0) {
    m_AdjacentLevel = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("PDFClusterizerBaseFileName")) != 0) {
    m_PDFClusterizerBaseFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("DoTracking")) != 0) {
    m_DoTracking = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SearchPairs")) != 0) {
    m_SearchPairs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SearchMIPs")) != 0) {
    m_SearchMIPs = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("SearchComptons")) != 0) {
    m_SearchComptons = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("MaxComptonJump")) != 0) {
    m_MaxComptonJump = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("NTrackSequencesToKeep")) != 0) {
    m_NTrackSequencesToKeep = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("RejectPurelyAmbiguousTrackSequences")) != 0) {
    m_RejectPurelyAmbiguousTrackSequences = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("NLayersForVertexSearch")) != 0) {
    m_NLayersForVertexSearch = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("ElectronTrackingDetectors")) != 0) {
    m_ElectronTrackingDetectors.clear();
    for (unsigned int n = 0; n < aNode->GetNNodes(); ++n) {
      if ((bNode = aNode->GetNode(n)) != 0) {
        m_ElectronTrackingDetectors.push_back(bNode->GetValue());
      }
    }
  }
  if ((aNode = Node->GetNode("AssumeD1First")) != 0) {
    m_AssumeD1First = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("ClassicUndecidedHandling")) != 0) {
    m_ClassicUndecidedHandling = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("AssumeTrackTopBottom")) != 0) {
    m_AssumeTrackTopBottom = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("UseComptelTypeEvents")) != 0) {
    m_UseComptelTypeEvents = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("GuaranteeStartD1")) != 0) {
    m_GuaranteeStartD1 = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("RejectOneDetectorTypeOnlyEvents")) != 0) {
    m_RejectOneDetectorTypeOnlyEvents = aNode->GetValueAsBoolean();
  }
  if ((aNode = Node->GetNode("CSRThreshold")) != 0) {
    m_CSRThresholdMin = aNode->GetMinValueAsDouble();
    m_CSRThresholdMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Node->GetNode("CSRMaxNHits")) != 0) {
    m_CSRMaxNHits = aNode->GetValueAsInt();
  }
  if ((aNode = Node->GetNode("LensCenter")) != 0) {
    m_LensCenter = aNode->GetValueAsVector();
  }
  if ((aNode = Node->GetNode("FocalSpotCenter")) != 0) {
    m_FocalSpotCenter = aNode->GetValueAsVector();
  }
  if ((aNode = Node->GetNode("OriginObjectsFile")) != 0) {
    m_OriginObjectsFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("DecayFile")) != 0) {
    m_DecayFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("BayesianComptonFile")) != 0) {
    m_BayesianComptonFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("BayesianElectronFile")) != 0) {
    m_BayesianElectronFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("TMVAFile")) != 0) {
    m_TMVAFileName = aNode->GetValueAsString();
  }
  if ((aNode = Node->GetNode("TMVAMethods")) != 0) {
    MString Methods = aNode->GetValueAsString();
    m_TMVAMethods.ResetUsedMethods();
    m_TMVAMethods.SetUsedMethods(Methods);
  }
  if ((aNode = Node->GetNode("TotalEnergy")) != 0) {
    m_TotalEnergyMin = aNode->GetMinValueAsDouble();
    m_TotalEnergyMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Node->GetNode("LeverArm")) != 0) {
    m_LeverArmMin = aNode->GetMinValueAsDouble();
    m_LeverArmMax = aNode->GetMaxValueAsDouble();
  }
  if ((aNode = Node->GetNode("EventId")) != 0) {
    m_EventIdMin = aNode->GetMinValueAsLong();
    m_EventIdMax = aNode->GetMaxValueAsLong();
  }
  if ((aNode = Node->GetNode("RejectAllBadEvents")) != 0) {
    m_RejectAllBadEvents = aNode->GetValueAsBoolean();
  }

  return true;
}
  

// MSettingsEventReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
