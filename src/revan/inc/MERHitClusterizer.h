/*
 * MERHitClusterizer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERHitClusterizer__
#define __MERHitClusterizer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Clusterize neighboring hits into one 
class MERHitClusterizer : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MERHitClusterizer();
  //! Default destructor
  virtual ~MERHitClusterizer();

  //! Use a voxel based clusterizer
  //! The level is:
  //! 1: 4 clostest neighbors
  //! 2: 8 clostest neighbors
  //! 4: 12 c.n.
  //! 5: 20 c.n.
  //! 8: 24 c.n.
  //! etc.
  virtual bool SetParameters(int Level = 2, double Sigma = 0.0);
  //! Clusterize based on distance
  virtual bool SetParameters(double MinDistanceD1, double MinDistanceD2, 
                             double MinDistanceD3, double MinDistanceD4, 
                             double MinDistanceD5, double MinDistanceD6,
                             double MinDistanceD7, double MinDistanceD8,
                             bool CenterIsReference);
  virtual bool SetParameters(MString BaseResponseFileName);
  
  //! Do the analysis - clusterize all events in the list
  virtual bool Analyze(MRawEventIncarnations* List);

  virtual MString ToString(bool CoreOnly = false) const;

  static const int c_None = 0;
  static const int c_Distance = 1;
  static const int c_Level = 2;
  static const int c_Response = 3;

  // protected methods:
 protected:
  //MERHitClusterizer() {};
  //MERHitClusterizer(const MERHitClusterizer& ERClusterize) {};

  void FindClusters(MRERawEvent* RE, double Distance, int Detector);
  void FindClustersInAdjacentVoxels(MRERawEvent* RE, double Sigma, int Level, int Detector);
  void FindClustersUsingPDF(MRERawEvent* RE);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  int m_Algorithm;

  int m_Level;
  //! All non voxelized dimensions have to be at least so many position resolution sigmas apart 
  double m_Sigma;

  double m_MinDistanceD1;
  double m_MinDistanceD2;
  double m_MinDistanceD3;
  double m_MinDistanceD4;
  double m_MinDistanceD5;
  double m_MinDistanceD6;
  double m_MinDistanceD7;
  double m_MinDistanceD8;

  // Instead of looking for the min distance to each subhit of the cluster, we only look at the center of the already existing cluster
  bool m_CenterIsReference;

  //! PDF for two site clusters
  MResponseMatrixO3 m_DualPDF;
  //! PDF for all clusters
  MResponseMatrixO2 m_AllPDF;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MERHitClusterizer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
