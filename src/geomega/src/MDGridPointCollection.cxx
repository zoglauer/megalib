/*
 * MDGridPointCollection.cxx
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
// MDGridPointCollection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDGridPointCollection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h" 
#include "MStreams.h"
#include "MDDetector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDGridPointCollection)
#endif


////////////////////////////////////////////////////////////////////////////////


MDGridPointCollection::MDGridPointCollection(const MDVolumeSequence& VolumeSequence) : 
  m_VolumeSequence(VolumeSequence), m_StripMode(false)
{
  // Construct an instance of MDGridPointCollection

  massert(m_VolumeSequence.GetDetector() != nullptr);
}


////////////////////////////////////////////////////////////////////////////////


MDGridPointCollection::MDGridPointCollection(const MDGridPointCollection& Grid)
{
  // Copy constructor

  m_VolumeSequence = Grid.m_VolumeSequence;
  m_Points = Grid.m_Points;
  m_StripMode = Grid.m_StripMode;
}


////////////////////////////////////////////////////////////////////////////////


MDGridPointCollection::~MDGridPointCollection()
{
  // Delete this instance of MDGridPointCollection
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::Add(const MVector& PositionInDetector, const double Energy, 
                                const double Time, const vector<int>& Origins)
{
  // Add and discretize a measurement

  vector<MDGridPoint> NewPoints = m_VolumeSequence.GetDetector()->Grid(PositionInDetector, Energy, Time, m_VolumeSequence.GetDeepestVolume());
  
  vector<MDGridPoint>::iterator NewIter;
  vector<MDGridPoint>::iterator AllIter;

  for (NewIter = NewPoints.begin(); NewIter != NewPoints.end(); ++NewIter) {
    (*NewIter).SetOrigins(Origins);

    bool Added = false;
    for (AllIter = m_Points.begin(); AllIter != m_Points.end(); ++AllIter) {
      if ((*NewIter) == (*AllIter)) {
        (*AllIter) += (*NewIter);
        Added = true;
        break;
      }
    }
    if (Added == false) {
      m_Points.push_back((*NewIter));      
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::AddUndiscretized(const MVector& PositionInDetector)
{
  // Add a measurement

  MDGridPoint NewPoint = 
    m_VolumeSequence.GetDetector()->GetGridPoint(PositionInDetector);
  
  vector<MDGridPoint>::iterator AllIter;

  bool Added = false;
  for (AllIter = m_Points.begin();
       AllIter != m_Points.end(); ++AllIter) {
    if (NewPoint == (*AllIter)) {
      (*AllIter) += NewPoint;
      Added = true;
      break;
    }
  }
  if (Added == false) {
    m_Points.push_back(NewPoint);      
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::Add(MDGridPoint& Point)
{
  // Add another grid point
  //
  // TODO: Ideally we should verify that the volume sequences are identical, 
  //       but the point does not have one, and I cannot create one, since we have no geometry
  
  if (m_Points.size() == 0 || (m_Points.size() > 0 && m_Points[0].GetType() == Point.GetType())) {
      
    bool Added = false;
    for (auto AllIter = m_Points.begin(); AllIter != m_Points.end(); ++AllIter) {
      if (Point == (*AllIter)) {
        (*AllIter) += Point;
        Added = true;
        break;
      }
    }
    if (Added == false) {
      m_Points.push_back(Point);      
    }
  } else {
    merr<<"ERROR: Grid points are not of same size - grid point not added"<<endl; 
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::Add(MDGridPointCollection& Collection)
{
  // Add another GridPointCollection
  
  if (Collection.HasSameDetector(&m_VolumeSequence) == true) {
    
    vector<MDGridPoint>::iterator AllIter;
    for (unsigned int p = 0; p < Collection.GetNGridPoints(); ++p) {
      MDGridPoint NewPoint = Collection.GetGridPointAt(p);
      
      bool Added = false;
      for (AllIter = m_Points.begin();
           AllIter != m_Points.end(); ++AllIter) {
        if (NewPoint == (*AllIter)) {
          (*AllIter) += NewPoint;
          Added = true;
          break;
        }
      }
      if (Added == false) {
        m_Points.push_back(NewPoint);      
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::FinalizeGridding()
{
  // Has to be called after all hits have been added!

  mimp<<"This function isn't doing anything!!!"<<endl;

  // Discretize all c_VoxelDrift points:
  vector<MDGridPoint>::iterator Iter;

  for (Iter = m_Points.begin(); Iter != m_Points.end(); ++Iter) {
    
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MDGridPointCollection::HasSameDetector(MDVolumeSequence* VolumeSequence)
{
  // Check if the volume sequences are identical:

  return m_VolumeSequence.HasSameDetector(VolumeSequence);
}


////////////////////////////////////////////////////////////////////////////////


bool MDGridPointCollection::HasSameDetector(MDGridPointCollection& Grid)
{
  // Check if the grid has the same detector

  return Grid.HasSameDetector(&m_VolumeSequence);
}

////////////////////////////////////////////////////////////////////////////////


MVector MDGridPointCollection::GetWorldPositionGridPointAt(unsigned int i) const
{
  // Return the Position of grid i in world coordinates

  MVector Pos = m_VolumeSequence.GetDetector()->
    GetPositionInDetectorVolume(m_Points[i].GetXGrid(), 
                                m_Points[i].GetYGrid(), 
                                m_Points[i].GetZGrid(), 
                                m_Points[i].GetPosition(),
                                m_Points[i].GetType(),
                                m_VolumeSequence.GetDetectorVolume());

  return m_VolumeSequence.GetPositionInFirstVolume(Pos, m_VolumeSequence.GetDetectorVolume());
}


////////////////////////////////////////////////////////////////////////////////


MVector MDGridPointCollection::GetWorldPosition()
{
  return m_VolumeSequence.GetPositionInFirstVolume(MVector(0, 0, 0), 
                                                   m_VolumeSequence.GetDetectorVolume());
}


////////////////////////////////////////////////////////////////////////////////


double MDGridPointCollection::GetEnergy()
{
  // Return the total energy position in the grid

  double Energy = 0.0;
  for (unsigned int p = 0; p < m_Points.size(); ++p) {
    Energy += m_Points[p].GetEnergy();
  }

  return Energy;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MDGridPointCollection::GetHits()
{
  // Return the total number of hits in the grid

  unsigned int Hits = 0;
  for (unsigned int p = 0; p < m_Points.size(); ++p) {
    Hits += m_Points[p].GetHits();
  }

  return Hits;
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::SetWeight(double Weight)
{
  // Return the total number of hits in the grid

  for (unsigned int p = 0; p < m_Points.size(); ++p) {
    m_Points[p].SetWeight(Weight);
  }
}


////////////////////////////////////////////////////////////////////////////////


double MDGridPointCollection::GetWeightedHits()
{
  // Return the total number of hits in the grid

  double WeightedHits = 0;
  for (unsigned int p = 0; p < m_Points.size(); ++p) {
    WeightedHits += m_Points[p].GetHits()*m_Points[p].GetWeight();
  }

  return WeightedHits;
}


////////////////////////////////////////////////////////////////////////////////


void MDGridPointCollection::RemoveNonReadOuts()
{
  //! Remove not read-out grid points
  
  m_Points.erase(std::remove_if(m_Points.begin(), m_Points.end(), [](const MDGridPoint& P) { return (P.IsReadOut() == false); }), m_Points.end());
}
  
  
// MDGridPointCollection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
