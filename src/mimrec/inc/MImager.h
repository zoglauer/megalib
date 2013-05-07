/*
 * MImager.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImager__
#define __MImager__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMatrix.h>
#include <TThread.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MBPDataImage.h"
#include "MBackprojection.h"
#include "MEventSelector.h"
#include "MFileEventsTra.h"
#include "MSensitivity.h"
#include "MImage.h"
#include "MLMLAlgorithms.h"
#include "MSettingsImaging.h"
#include "MSettingsMimrec.h"
#include "MSettingsEventSelections.h"

// Forward declarations:
class MBPData;


////////////////////////////////////////////////////////////////////////////////


class MImager
{
  // Public Interface:
 public:
  //! Standard constructor
  MImager(int CoordinateSystem, unsigned int NThreads = 1);
  //! Default destructor
  virtual ~MImager();


  // Set the settings
  
  //! Set all settings
  bool SetSettings(MSettingsMimrec* Settings);
  //! Set only the imaging settings
  bool SetImagingSettings(MSettingsImaging* Settings);
  //! Set only the event reconstruction settings
  bool SetEventSelectionSettings(MSettingsEventSelections* Settings);


  // Image part:

  //! Set the image dimensions
  void SetViewport(double xMin, double xMax, int xNBins, 
                   double yMin, double yMax, int yNBins,
                   double zMin = 0, double zMax = 0, int zNBins = 1,
                   MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  //! Set the draw mode
  void SetDrawMode(const int DrawMode) { m_DrawMode = DrawMode; }

  //! Set the palette
  void SetPalette(const int Palette) { m_Palette = Palette; }

  //! Set the source catalog
  void SetSourceCatalog(const MString SourceCatalog) { m_SourceCatalog = SourceCatalog; }


  // Response part:

  //! Set the Gaussian response 
  void SetResponseGaussian(const double Transversal, const double Longitudinal, const double Pair, 
                           const double CutOff, const bool UseAbsorptions);
  void SetResponseGaussianByUncertainties();
  void SetResponseEnergyLeakage(const double Longitudinal, const double Transversal);
  //! Set response matrices
  bool SetResponsePRM(const MString& ComptonTrans, const MString& ComptonLong, const MString& Pair);

  //! Set if absorption probabilities should be use
  void UseAbsorptions(bool UseAbsorptions = true);

  // Deconvolution part:
  
  //! Use the classic EM algorithm for deconvolution
  void SetDeconvolutionAlgorithmClassicEM();
  //! Use the OSEM algorithm for deconvolution
  void SetDeconvolutionAlgorithmOSEM(unsigned int NSubsets);

  //! Use a stop criterion by 
  void SetStopCriterionByIterations(int NIterations);
  //! Reset the stop criterion
  void ResetStopCriterion() { if (m_EM != 0) m_EM->ResetStopCriterion(); }

  // All the rest:

  //! Set the sensitivity matrix
  void SetSensitivity(MSensitivity* Sensitivity);
  //! Set the pointsources which are cut out of the picture
  void SetDeselectedPointSources(TObjArray* DeselectedPS);

  //! Set the memory managment features
  void SetMemoryManagment(int MaxRAM, int MaxSwap, int Exhausted, int Accuracy);

  //! Set the maths approximation
  void SetApproximatedMaths(bool Approximated);

  //! Set the event selector 
  void SetEventSelector(const MEventSelector& Selector);
  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry);
  //! Open the event file
  bool SetFileName(MString FileName, bool FastFileParsing = false); 

  
  // The animation

  //! ID representing no animation at all
  static const int c_AnimateNothing;
  //! ID representing an animation of the backprojections
  static const int c_AnimateBackprojections;
  //! ID representing an animation of the iterations
  static const int c_AnimateIterations;

  //! Set the animation mode
  void SetAnimationMode(const int AnimationMode) { m_AnimationMode = AnimationMode; }
  //! Set the time between frames
  void SetAnimationFrameTime(const double AnimationFrameTime) { m_AnimationFrameTime = AnimationFrameTime; }
  //! Set the output file namespace
  void SetAnimationFileName(const MString AnimationFileName) { m_AnimationFileName = AnimationFileName; }
  
  // The real analysis:

  //! The main imaging routine
  //! Before calling this all data needs to be set correctly
  bool Analyze(bool CalculateResponse = true);

  //! Return the number of stored images
  unsigned int GetNImages() const { return m_Images.size(); }
  //! Return a specific image
  MImage* GetImage(unsigned int i) { if (i < GetNImages()) return m_Images[i]; return 0; }


  // Better protected or really needed?

  //! Compute all response slices
  bool ComputeResponseSlices();

  //! Entry function for the threads
  void* ResponseSliceComputationThread(unsigned int ThreadID);

  //! Return the number of events the response is made of
  unsigned int GetNEvents();
  //! Return the number of bins of the image-space
  int GetNImageBins();

  //! Return the response slice corresponding to the i-th event
  MBPData* GetResponseSlice(unsigned int i);
  //! Add a response slice
  void AddResponseSlice(MBPData* Slice);

  //! Return the amount of RAM currently used for the response slices
  unsigned long GetUsedBytes() { return m_UsedBytes; }


  // protected methods:
 protected:

  // The image:
  
  //! x1-axis: Minimum
  double m_x1Min;
  //! x1-axis: Maximum
  double m_x1Max;
  //! x1-axis: NBins
  int m_x1NBins;

  //! x2-axis: Minimum
  double m_x2Min;
  //! x2-axis: Maximum
  double m_x2Max;
  //! x2-axis: NBins
  int m_x2NBins;

  //! x3-axis: Minimum
  double m_x3Min;
  //! x3-axis: Maximum
  double m_x3Max;
  //! x3-axis: NBins
  int m_x3NBins;

  //! The ID of the palette
  int m_Palette;
  //! The ID of the draw mode
  int m_DrawMode;
  //! The name of the source catalog
  MString m_SourceCatalog;

  //! All produced images:
  vector<MImage*> m_Images;


  // Response calculation:

  //! Number of bins (x*y*z) of the image
  int m_NBins;  
  //! The coordinate system
  int m_CoordinateSystem;
  //! Which of the axes is the 2D axis 
  int m_TwoDAxis;


  // Animation:
  
  //! The animation mode (animate backprojections or animate iterations)
  int m_AnimationMode;
  //! The time between frames in seconds of observation time
  double m_AnimationFrameTime;
  //! The name of the to be genarted animated gif file 
  MString m_AnimationFileName;

  
  // Response calculation:

  //! The response slices in list-mode
  vector<MBPData*> m_BPEvents;
  //! The backprojection algorithm classs --- one per thread
  vector<MBackprojection*> m_BPs;
 
  //! Use absorption probabilities
  bool m_UseAbsorptions;
  
  //! The event selector
  MEventSelector m_Selector;
  //! The event file
  MFileEventsTra m_EventFile;

  //! True if fast file parsing is enabled
  bool m_FastFileParsing;

  // Deconvolution:

  //! The EM algorithm
  MLMLAlgorithms* m_EM;


  // Multi-threading:

  //! Number of threads
  unsigned int m_NThreads;
  //! Storing the threads:
  vector<TThread*> m_Threads;
  //! Storing a flag that the thread is running
  vector<bool> m_ThreadIsInitialized;
  //! Storing a flag that the thread is finished
  vector<bool> m_ThreadIsFinished;


  // Memory management:

  //! Maximum amount of RAM in bytes to be used on this computer
  unsigned long m_MaxBytes;
  //! Flag indicating the storage accuracy of the response slices
  int m_ComputationAccuracy;
  
  //! Flag indicating we went out of memory
  bool m_OutOfMemory;
  
  //! Currently used bytes for the response
  unsigned long m_UsedBytes;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MImager, 0) // Computes and stores system matrix
#endif

};

////////////////////////////////////////////////////////////////////////////////

class MImager_ThreadCaller
{
 public:
  //! Standard constructor
  MImager_ThreadCaller(MImager* S, unsigned int ThreadID) {
    m_Storage = S;
    m_ThreadID = ThreadID;
  }

  //! Return the calling class
  MImager* GetThreadCaller() { return m_Storage; }
  //! Return the thread ID
  unsigned int GetThreadID() { return m_ThreadID; }

 private:
  //! Store the calling class for retrieval
  MImager* m_Storage;
  //! ID of the worker thread
  unsigned int m_ThreadID;
};

////////////////////////////////////////////////////////////////////////////////

void MImager_CallThread(void* address);

#endif


////////////////////////////////////////////////////////////////////////////////
