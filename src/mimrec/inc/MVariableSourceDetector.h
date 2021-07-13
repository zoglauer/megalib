/*
 * MVariableSourceDetector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MVariableSourceDetector__
#define __MVariableSourceDetector__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <deque>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsImaging.h"
#include "MSettingsEventSelections.h"
#include "MTime.h"
#include "MBPData.h"
#include "MImagerExternallyManaged.h"
#include "MPhysicalEvent.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A simple class holding the information of the detected source
class MVariableSourceLocation
{
 public: 
  //! The location in the given coordinates - x
  unsigned int m_X;
  //! The location in the given coordinates - y
  unsigned int m_Y;
  //! The significance
  double Significance;
};


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MVariableSourceDetector
{
  // public interface:
 public:
  //! Default constructor
  MVariableSourceDetector(MSettingsImaging* ImagingSettings, MSettingsEventSelections* EventSelectionSettings, MDGeometryQuest* Geometry);
  //! Default destuctor 
  virtual ~MVariableSourceDetector();

  //! Save the peak heights
  bool SavePeakHeightHistory(MString FileName);
  //! Save the peak heights
  bool ReadPeakHeightHistory(MString FileName);
  
  //! Add backprojection
  void AddBackprojection(MBPData* Image, MTime Time, bool Own);
  
  //! Create and store a backprojection of the given event
  void BackprojectEvent(MPhysicalEvent* Event);
  
  //! Trim data, i.e. remove data older than the given time
  void Trim(MTime Time);
  
  //! Search for sources and return their locations
  vector<MVariableSourceLocation> Search(bool Learn = true);
  

  // protected methods:
 protected:
  //! Set only the imaging settings
  void SetImagingSettings(MSettingsImaging* Settings) { m_ImagingSettings = Settings; }
  //! Set only the event reconstruction settings
  void SetEventSelectionSettings(MSettingsEventSelections* Settings) { m_EventSelectionSettings = Settings; }
  //! Create the imager
  void CreateImager();
    
  //! The intensive search for a better peak interval
  void IntensiveSearch(MTime& Start, MTime& Stop);
  
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The imaging settings
  MSettingsImaging* m_ImagingSettings;
  //! The event selection setting
  MSettingsEventSelections* m_EventSelectionSettings;
  //! The goemetry
  MDGeometryQuest* m_Geometry;
  
  //! The imager
  MImagerExternallyManaged* m_Imager;
  
  //! The integration times
  vector<double> m_IntegrationTimes;
  
  //! The relative peak height histograms
  vector<TH1D*> m_RelativePeakHeights;
  
  //! The relative peak height histograms
  vector<double> m_TriggerThresholds;
  
  //! The backprojections
  deque<MBPData*> m_Backprojections;
  //! The times
  deque<MTime> m_Times;
  //! Flag indicating we can delete it
  deque<bool> m_Own;
  //! The events themsleves
  deque<MPhysicalEvent*> m_Events;


#ifdef ___CLING___
 public:
  ClassDef(MVariableSourceDetector, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
