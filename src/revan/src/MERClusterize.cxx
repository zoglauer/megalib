/*
 * MERClusterize.cxx
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
// MERClusterize
//
// Find all clusters...
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERClusterize.h"

// Standard libs:
#include "MStreams.h"
#include "MRESE.h"
#include "MRECluster.h"

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MERClusterize)
#endif


////////////////////////////////////////////////////////////////////////////////


MERClusterize::MERClusterize()
{
  // Construct an instance of MERClusterize

  m_Algorithm = c_None;

  m_Level = 1;
  m_Sigma = 0;

  m_MinDistanceD1 = 0.0;
  m_MinDistanceD2 = 0.0;
  m_MinDistanceD3 = 0.0;
  m_MinDistanceD4 = 0.0;
  m_MinDistanceD5 = 0.0;
  m_MinDistanceD6 = 0.0;
  m_MinDistanceD7 = 0.0;
  m_MinDistanceD8 = 0.0;

  m_CenterIsReference = false;
}


////////////////////////////////////////////////////////////////////////////////


MERClusterize::~MERClusterize()
{
  // Delete this instance of MERClusterize
}


////////////////////////////////////////////////////////////////////////////////


bool MERClusterize::SetParameters(int Level, double Sigma)
{
  m_Algorithm = c_Level;

  m_Level = Level;
  m_Sigma = Sigma;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERClusterize::SetParameters(double MinDistanceD1, double MinDistanceD2, 
                                  double MinDistanceD3, double MinDistanceD4, 
                                  double MinDistanceD5, double MinDistanceD6,
                                  double MinDistanceD7, double MinDistanceD8,
                                  bool CenterIsReference)
{
  m_Algorithm = c_Distance;

  m_MinDistanceD1 = MinDistanceD1;
  m_MinDistanceD2 = MinDistanceD2;
  m_MinDistanceD3 = MinDistanceD3;
  m_MinDistanceD4 = MinDistanceD4;
  m_MinDistanceD5 = MinDistanceD5;
  m_MinDistanceD6 = MinDistanceD6;
  m_MinDistanceD7 = MinDistanceD7;
  m_MinDistanceD8 = MinDistanceD8;
  m_CenterIsReference = CenterIsReference;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MERClusterize::SetParameters(MString BaseResponseFileName)
{
  if (BaseResponseFileName.IsEmpty()) {
    merr<<"No file for Clustering based on PDF given."<<endl;
    return false;
  }
  
  MString Suffix = ".dualseparable.yes.rsp";
  MString Prefix = BaseResponseFileName.Remove(BaseResponseFileName.Length()-Suffix.Length());

  if (MFile::FileExists(Prefix + Suffix) == true) {
    
    MResponseMatrixO3 DualSeparableYes;
    if (DualSeparableYes.Read(Prefix + ".dualseparable.yes.rsp") == false) {
      merr<<"File \""<<Prefix + ".dualseparable.yes.rsp"<<"\" does not exist."<<endl;
      return false;
    }
    
    MResponseMatrixO3 DualSeparableNo;
    if (DualSeparableNo.Read(Prefix + ".dualseparable.no.rsp") == false) {
      merr<<"File \""<<Prefix + ".dualseparable.no.rsp"<<"\" does not exist."<<endl;
      return false;
    }

    MResponseMatrixO2 AllSeparableYes;
    if (AllSeparableYes.Read(Prefix + ".allseparable.yes.rsp") == false) {
      merr<<"File \""<<Prefix + ".allseparable.yes.rsp"<<"\" does not exist."<<endl;
      return false;
    }

    MResponseMatrixO2 AllSeparableNo;
    if (AllSeparableNo.Read(Prefix + ".allseparable.no.rsp") == false) {
      merr<<"File \""<<Prefix + ".allseparable.no.rsp"<<"\" does not exist."<<endl;
      return false;
    }
    
    
    // Small hack since we have not defined + & / operator just += & /= ...
    DualSeparableNo += DualSeparableYes;
    DualSeparableYes /= DualSeparableNo;
    m_DualPDF = DualSeparableYes;

    AllSeparableNo += AllSeparableYes;
    AllSeparableYes /= AllSeparableNo;
    m_AllPDF = AllSeparableYes;
    
    m_Algorithm = c_Response;

    return true;
  } else {
    merr<<"File \""<<Prefix + Suffix<<"\" does not exist."<<endl;
    return false;
  }
    
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MERClusterize::Analyze(MRawEventList* List)
{
  // Analyze the raw event...
  MERConstruction::Analyze(List);

  mdebug<<"Starting cluster search..."<<endl;
  mdebug<<m_List->ToString()<<endl;

  MRERawEvent* RE;
  for (int e = 0; e < m_List->GetNRawEvents(); ++e) {
    RE = m_List->GetRawEventAt(e);
    if (m_Algorithm == c_Distance) {
      FindClusters(RE, m_MinDistanceD1, MDDetector::c_Strip2D);
      FindClusters(RE, m_MinDistanceD2, MDDetector::c_Calorimeter);
      FindClusters(RE, m_MinDistanceD3, MDDetector::c_Strip3D);
      FindClusters(RE, m_MinDistanceD4, MDDetector::c_Scintillator);
      FindClusters(RE, m_MinDistanceD5, MDDetector::c_DriftChamber);
      FindClusters(RE, m_MinDistanceD6, MDDetector::c_Strip3DDirectional);
      FindClusters(RE, m_MinDistanceD7, MDDetector::c_AngerCamera);
      FindClusters(RE, m_MinDistanceD8, MDDetector::c_Voxel3D);
    } else if (m_Algorithm == c_Level) {
      for (int d = MDDetector::c_MinDetector; d <= MDDetector::c_MaxDetector; ++d) {
        FindClustersInAdjacentVoxels(RE, m_Sigma, m_Level, d);
      }
    } else if (m_Algorithm == c_Response) {
      FindClustersUsingPDF(RE);
    }
  }
  
  mdebug<<"Result of cluster search:"<<endl;
  mdebug<<m_List->ToString()<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERClusterize::FindClusters(MRERawEvent* RE, double Distance, int Detector)
{
  // Find Clusters: Coalesce RESE to clusters

  double TestDistance = 0;
  MRESE* RESE = 0;
  MRECluster* Cluster = 0;

  // Main subelement loop
  for (int h = 0; h < RE->GetNRESEs(); ++h) {
    if (Detector != 0 && RE->GetRESEAt(h)->GetDetector() != Detector) {
      continue;
    }
    // Secondary subelement-loop: Find a partner
    for (int l = h+1; l < RE->GetNRESEs(); ++l) {
      if (Detector != 0 && RE->GetRESEAt(l)->GetDetector() != Detector) {
        continue;
      }
      // if we have a positive cluster-test
      if (m_CenterIsReference == true) {
        TestDistance = (RE->GetRESEAt(h)->GetPosition() - RE->GetRESEAt(l)->GetPosition()).Mag();
      } else {
        TestDistance = RE->GetRESEAt(h)->ComputeMinDistance(RE->GetRESEAt(l));
      }
      if (TestDistance < Distance) {
          
        // Coalesce 2 hits to a cluster
        if (RE->GetRESEAt(h)->GetType() == MRESE::c_Hit 
            && RE->GetRESEAt(l)->GetType() == MRESE::c_Hit) {
          Cluster = new MRECluster();
          Cluster->AddRESE(RE->GetRESEAt(h));
          Cluster->AddRESE(RE->GetRESEAt(l));
          RE->AddRESE(Cluster);
          RE->RemoveRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(l));
            
          // Add a hit to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Cluster
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Hit) {
          RE->GetRESEAt(h)->AddRESE(RE->GetRESEAt(l));
          RE->AddRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(l));
            
          // Add a hit to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Hit
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Cluster) {
          RE->GetRESEAt(l)->AddRESE(RE->GetRESEAt(h));
          RE->AddRESE(RE->GetRESEAt(l));
          RE->RemoveRESE(RE->GetRESEAt(l));
          RE->RemoveRESE(RE->GetRESEAt(h));
            
          // Add a cluster to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Cluster
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Cluster) {
          
          // mdebug<<"Adding a cluster to a cluster!"<<endl;
          for (int i = 0; i < RE->GetRESEAt(l)->GetNRESEs(); i++) {
            RE->GetRESEAt(h)->AddRESE(RE->GetRESEAt(l)->RemoveRESEAt(i));
          }
            
          RE->AddRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(h));
          RESE = RE->RemoveRESE(RE->GetRESEAt(l));
          RESE->DeleteAll();
          delete RESE;

        } else {
          Fatal("MREClusterAnalyzer::AnalyzeEvent(MRERawEvent *RawEvent)",
                "Clustering not allowed for this subelement!");
          break;
        }
          
        RE->CompressRESEs();
        h--;
        break; // Jump back to mainsubelement-loop
      } // end of positive cluster-test loop
    } // end secondary subelement-loop
  } // end main subelement-loop

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MERClusterize::FindClustersInAdjacentVoxels(MRERawEvent* RE, double Sigma, 
                                                 int Level, int Detector)
{
  // Find Clusters: Coalesce RESE to clusters

  MRESE* RESE = 0;
  MRECluster* Cluster = 0;

  mdebug<<"FindClustersInAdjacentVoxels for detector "<<Detector<<endl;

  // Main subelement loop
  for (int h = 0; h < RE->GetNRESEs(); ++h) {
    if (Detector != 0 && RE->GetRESEAt(h)->GetDetector() != Detector) {
      continue;
    }
    // Secondary subelement-loop: Find a partner
    for (int l = h+1; l < RE->GetNRESEs(); ++l) {
      if (Detector != 0 && RE->GetRESEAt(l)->GetDetector() != Detector) {
        continue;
      }
      // if we have a positive cluster-test
      if (RE->GetRESEAt(h)->AreAdjacent(RE->GetRESEAt(l), Sigma, Level) == true) {
          
        // Coalesce 2 hits to a cluster
        if (RE->GetRESEAt(h)->GetType() == MRESE::c_Hit 
            && RE->GetRESEAt(l)->GetType() == MRESE::c_Hit) {
          Cluster = new MRECluster();
          Cluster->AddRESE(RE->GetRESEAt(h));
          Cluster->AddRESE(RE->GetRESEAt(l));
          RE->AddRESE(Cluster);
          RE->RemoveRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(l));
            
          // Add a hit to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Cluster
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Hit) {
          RE->GetRESEAt(h)->AddRESE(RE->GetRESEAt(l));
          RE->AddRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(h));
          RE->RemoveRESE(RE->GetRESEAt(l));
            
          // Add a hit to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Hit
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Cluster) {
          RE->GetRESEAt(l)->AddRESE(RE->GetRESEAt(h));
          RE->AddRESE(RE->GetRESEAt(l));
          RE->RemoveRESE(RE->GetRESEAt(l));
          RE->RemoveRESE(RE->GetRESEAt(h));
            
          // Add a cluster to a cluster
        } else if (RE->GetRESEAt(h)->GetType() == MRESE::c_Cluster
                   && RE->GetRESEAt(l)->GetType() == MRESE::c_Cluster) {
          
          // mdebug<<"Adding a cluster to a cluster!"<<endl;
          for (int i = 0; i < RE->GetRESEAt(l)->GetNRESEs(); i++) {
            RE->GetRESEAt(h)->AddRESE(RE->GetRESEAt(l)->RemoveRESEAt(i));
          }
            
          mimp<<"Strange behaviour!"<<endl;
          RE->AddRESE(RE->GetRESEAt(h)); // ?
          RE->RemoveRESE(RE->GetRESEAt(h)); // ?
          RESE = RE->RemoveRESE(RE->GetRESEAt(l));
          RESE->DeleteAll();
          delete RESE;

        } else {
          Fatal("MREClusterAnalyzer::AnalyzeEvent(MRERawEvent *RawEvent)",
                "Clustering not allowed for this subelement!");
          break;
        }
          
        RE->CompressRESEs();
        h--;
        break; // Jump back to mainsubelement-loop
      } // end of positive cluster-test loop
    } // end secondary subelement-loop
  } // end main subelement-loop

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MERClusterize::FindClustersUsingPDF(MRERawEvent* RE) 
{
  // Step 1: Do a standard 8, no depth cut clustering - 
  // that's what we have used for response generation
  
  for (int d = MDDetector::c_MinDetector; d <= MDDetector::c_MaxDetector; ++d) {
    FindClustersInAdjacentVoxels(RE, -1, 2, d);
  }
  
  // Step 2: Check is we have to split them up again using the PDF
  
  vector<MRECluster*> RemoveList;
  for (int r = 0; r < RE->GetNRESEs(); ++r) {
    if (RE->GetRESEAt(r)->GetType() == MRESE::c_Cluster) {
      MRECluster* Cluster = dynamic_cast<MRECluster*>(RE->GetRESEAt(r));
      int NRESEs = Cluster->GetNRESEs();
      if (NRESEs == 2) {
        // Calculate Depth difference in detector coordiantes
        MVector PosInDet0 = Cluster->GetRESEAt(0)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        MVector PosInDet1 = Cluster->GetRESEAt(1)->GetVolumeSequence()->GetPositionInSensitiveVolume();
        double DepthDifference = fabs(PosInDet0.Z() - PosInDet1.Z());

        double PDF = 0;
        if (Cluster->GetRESEAt(0)->GetEnergy() < Cluster->GetRESEAt(1)->GetEnergy()) {
          PDF = m_DualPDF.Get(Cluster->GetRESEAt(0)->GetEnergy(), Cluster->GetRESEAt(1)->GetEnergy(), DepthDifference);
        } else {
          PDF = m_DualPDF.Get(Cluster->GetRESEAt(1)->GetEnergy(), Cluster->GetRESEAt(0)->GetEnergy(), DepthDifference);
        }
        if (PDF > 0.5) {
          RemoveList.push_back(Cluster);
        }
      } else {
        if (m_AllPDF.Get(Cluster->GetEnergy(), Cluster->GetNRESEs()) > 0.5) {
          RemoveList.push_back(Cluster);
        }
      }
    }
  }
  
  // Step 3: Do the split up
  
  if (RemoveList.size() > 0) {
    mdebug<<"Splitting some clusters again..."<<endl;
    for (MRECluster* C: RemoveList) {
      RE->RemoveRESE(C);
      for (int r = 0; r < C->GetNRESEs(); ++r) {
        RE->AddRESE(C->GetRESEAt(r)); 
      }
      delete C;
    }
    RE->CompressRESEs();
  }
}

////////////////////////////////////////////////////////////////////////////////


MString MERClusterize::ToString(bool CoreOnly) const
{
  // Dump an options string gor the tra file:

  ostringstream out;

  out<<"# Clusterizer options:"<<endl;
  out<<"# "<<endl;
  if (m_Algorithm == c_Distance) {
    out<<"# Clusterizing by distance"<<endl;
    out<<"# Min distance D1: "<<m_MinDistanceD1<<endl;
    out<<"# Min distance D2: "<<m_MinDistanceD2<<endl;
    out<<"# Min distance D3: "<<m_MinDistanceD3<<endl; 
    out<<"# Min distance D4: "<<m_MinDistanceD4<<endl; 
    out<<"# Min distance D5: "<<m_MinDistanceD5<<endl; 
    out<<"# Min distance D6: "<<m_MinDistanceD6<<endl; 
    out<<"# Min distance D7: "<<m_MinDistanceD7<<endl; 
    out<<"# Min distance D8: "<<m_MinDistanceD8<<endl; 
  } else if (m_Algorithm == c_Level) {
    out<<"# Clusterizing by adjacent voxels"<<endl;
    out<<"# Level: "<<m_Level<<endl;
    out<<"# Sigma: "<<m_Sigma<<endl;
  }
  out<<"# "<<endl;

  return out.str().c_str();
}


// MERClusterize.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
