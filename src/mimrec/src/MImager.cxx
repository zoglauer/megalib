/*
 * MImager.cxx
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
// MImager.cxx
//
//
// Computes and stores the data of the system-matrix.
// Make sure to set all data properly in advance.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImager.h"

// Standard libs:
#include <iostream>
#include <iomanip>
#include <limits>
#include <cstring>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MGUIProgressBar.h"
#include "MBackprojection.h"
#include "MBackprojectionFarField.h"
#include "MBackprojectionNearField.h"
#include "MBPDataSparseImage.h"
#include "MBPDataSparseImageOneByte.h"
#include "MPhysicalEvent.h"
#include "MBPDataImage.h"
#include "MBPDataImageOneByte.h"
#include "MGUIProgressBar.h"
#include "MSystem.h"
#include "MResponse.h"
#include "MResponseGaussian.h"
#include "MResponseGaussianByUncertainties.h"
#include "MResponseConeShapes.h"
#include "MResponsePRM.h"
#include "MResponseEnergyLeakage.h"
#include "MExposure.h" 
#include "MImage.h"
#include "MImage2D.h"
#include "MImage3D.h"
#include "MImageGalactic.h"
#include "MImageSpheric.h"
#include "MLMLAlgorithms.h"
#include "MLMLClassicEM.h"
#include "MLMLOSEM.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MImager)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MImager::c_AnimateNothing = 0;
const int MImager::c_AnimateBackprojections = 1;
const int MImager::c_AnimateIterations = 2;


////////////////////////////////////////////////////////////////////////////////


void MImager_CallThread(void* Address)
{
  MImager* Storage = ((MImager_ThreadCaller*) Address)->GetThreadCaller();
  Storage->ResponseSliceComputationThread(((MImager_ThreadCaller*) Address)->GetThreadID());
}


////////////////////////////////////////////////////////////////////////////////


MImager::MImager(MCoordinateSystem CoordinateSystem, unsigned int NThreads)
{
  // Initialize the system-matrix:
  //
  // Algorithm:         1: EM, 2: OS-EM, 3: SAGE
  // CoordinateSystem:  1: Spherical, 2: 2D-Cartesian 3: 3D-Cartesian

  m_NThreads = NThreads;
  if (m_NThreads < 1) m_NThreads = 1;

  m_CoordinateSystem = CoordinateSystem;
  for (unsigned int t = 0; t < m_NThreads; ++t) {
    // Initialize the backprojection-class
    if (m_CoordinateSystem == MCoordinateSystem::c_Spheric || m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionFarField(m_CoordinateSystem)));
    }
    else if (m_CoordinateSystem == MCoordinateSystem::c_Cartesian2D || m_CoordinateSystem == MCoordinateSystem::c_Cartesian3D) {
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionNearField(m_CoordinateSystem)));
    }
    else {
      mgui<<"Unknown coordinate system."<<endl;
      mgui<<"I will use a spherical coordinate system!"<<error;
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionFarField()));   
      m_CoordinateSystem = MCoordinateSystem::c_Spheric;
    }      
    m_Threads.push_back(0);
    m_ThreadIsInitialized.push_back(false);
    m_ThreadShouldFinish.push_back(false);
    m_ThreadIsFinished.push_back(false);
  }

  m_Exposure = new MExposure();
  
  m_EM = nullptr;
  
  m_UseAbsorptions = false;

  m_BPEvents.clear();

  m_UsedBytes = 0;
  m_MaxBytes = numeric_limits<unsigned long>::max();
  m_ComputationAccuracy = 1;

  m_OutOfMemory = false;

  m_DrawMode = MImage::c_COLCONT4Z;
  m_Palette = MImage::c_Thesis;
  m_SourceCatalog = "";

  m_FastFileParsing = false;
  
  m_AnimationMode = c_AnimateNothing;
  m_AnimationFrameTime = 10;
  m_AnimationFileName = "MyAnimated.gif";
}


////////////////////////////////////////////////////////////////////////////////


MImager::~MImager()
{
  // standard destructor

  for (unsigned int i = 0; i < m_BPEvents.size(); ++i) {
    delete m_BPEvents[i];
  }
  m_BPEvents.clear();

  for (unsigned int t = 0; t < m_NThreads; ++t) {
    delete m_BPs[t];
  }

  delete m_EM;
  delete m_Exposure;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetSettings(MSettingsMimrec* Settings)
{
  //! Set all settings

  if (SetImagingSettings(dynamic_cast<MSettingsImaging*>(Settings)) == false) return false;
  if (SetEventSelectionSettings(dynamic_cast<MSettingsEventSelections*>(Settings)) == false) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////



bool MImager::SetImagingSettings(MSettingsImaging* Settings)
{
  //! Set only the imaging settings

  m_NThreads = Settings->GetNThreads();
  if (m_NThreads < 1) m_NThreads = 1;

  m_CoordinateSystem = Settings->GetCoordinateSystem();
  for (unsigned int t = 0; t < m_NThreads; ++t) {
    // Initialize the backprojection-class
    if (m_CoordinateSystem == MCoordinateSystem::c_Spheric || m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionFarField(m_CoordinateSystem)));
    }
    else if (m_CoordinateSystem == MCoordinateSystem::c_Cartesian2D || m_CoordinateSystem == MCoordinateSystem::c_Cartesian3D) {
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionNearField(m_CoordinateSystem)));
    }
    else {
      mgui<<"Unknown coordinate system."<<endl;
      mgui<<"I will use a spherical coordinate system!"<<error;
      m_BPs.push_back(dynamic_cast<MBackprojection*>(new MBackprojectionFarField()));   
      m_CoordinateSystem = MCoordinateSystem::c_Spheric;
    }      
    m_Threads.push_back(0);
    m_ThreadIsInitialized.push_back(false);
    m_ThreadShouldFinish.push_back(false);
    m_ThreadIsFinished.push_back(false);
  }
    
  // Maths:
  SetApproximatedMaths(Settings->GetApproximatedMaths());
    
  // Set the dimensions of the image
  if (Settings->GetCoordinateSystem() == MCoordinateSystem::c_Spheric) {
    SetViewport(Settings->GetPhiMin()*c_Rad,
                Settings->GetPhiMax()*c_Rad, 
                Settings->GetBinsPhi(),
                Settings->GetThetaMin()*c_Rad,
                Settings->GetThetaMax()*c_Rad,
                Settings->GetBinsTheta(),
                c_FarAway/10, 
                c_FarAway, 
                1, 
                Settings->GetImageRotationXAxis(), 
                Settings->GetImageRotationZAxis());
  } else if (Settings->GetCoordinateSystem() == MCoordinateSystem::c_Galactic) {
    SetViewport(Settings->GetGalLongitudeMin()*c_Rad, 
                Settings->GetGalLongitudeMax()*c_Rad, 
                Settings->GetBinsGalLongitude(),
                (Settings->GetGalLatitudeMin()+90)*c_Rad,
                (Settings->GetGalLatitudeMax()+90)*c_Rad,
                Settings->GetBinsGalLatitude(),
                c_FarAway/10, 
                c_FarAway, 
                1);
  } else if (Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian2D ||
             Settings->GetCoordinateSystem() == MCoordinateSystem::c_Cartesian3D){
    SetViewport(Settings->GetXMin(), 
                Settings->GetXMax(), 
                Settings->GetBinsX(),
                Settings->GetYMin(), 
                Settings->GetYMax(), 
                Settings->GetBinsY(),
                Settings->GetZMin(), 
                Settings->GetZMax(), 
                Settings->GetBinsZ());
  } else {
    merr<<"Unknown coordinate system ID: "<<Settings->GetCoordinateSystem()<<error;
    return false;
  }
    
  // Set the draw modes
  SetDrawMode(Settings->GetImageDrawMode());
  SetPalette(Settings->GetImagePalette());
  SetSourceCatalog(Settings->GetImageSourceCatalog());
    
  // No animation by default
  SetAnimationMode(MImager::c_AnimateNothing); 
    
  // Set the response type:
  if (Settings->GetResponseType() == MResponseType::Gauss1D) {
    SetResponseGaussian(Settings->GetFitParameterComptonTransSphere(), 
                        Settings->GetFitParameterComptonLongSphere(),
                        Settings->GetFitParameterPair(),
                        Settings->GetGauss1DCutOff(),
                        Settings->GetUseAbsorptions());
  } else if (Settings->GetResponseType() == MResponseType::GaussByUncertainties) {
    SetResponseGaussianByUncertainties(Settings->GetGaussianByUncertaintiesIncrease());
  } else if (Settings->GetResponseType() == MResponseType::GaussByEnergyLeakage) {
    SetResponseEnergyLeakage(Settings->GetFitParameterComptonTransSphere(), 
                             Settings->GetFitParameterComptonLongSphere());
  } else if (Settings->GetResponseType() == MResponseType::ConeShapes) {
    if (SetResponseConeShapes(Settings->GetImagingResponseConeShapesFileName()) == false) {
      merr<<"Cannot set \"cone-shapes\" response! Aborting imaging!"<<show;
      return false;
    }
  } else if (Settings->GetResponseType() == MResponseType::PRM) {
    if (SetResponsePRM(Settings->GetImagingResponseComptonTransversalFileName(),
                       Settings->GetImagingResponseComptonLongitudinalFileName(),
                       Settings->GetImagingResponsePairRadialFileName()) == false) {
      merr<<"Cannot set PRM response! Aborting imaging!"<<show;
      return false;
    }
  } else {
    merr<<"Unknown response type: "<<Settings->GetResponseType()<<show;
    return false;
  }
  UseAbsorptions(Settings->GetUseAbsorptions());

  // Exposure
  if (Settings->GetExposureMode() == MExposureMode::CalculateFromEfficiency) {
    if (SetExposureEfficiencyFile(Settings->GetExposureEfficiencyFile()) == false) {
      return false; 
    }
  }
  
  // Memory management... 
  SetMemoryManagment(Settings->GetRAM(),
                     Settings->GetSwap(),
                     Settings->GetMemoryExhausted(),
                     Settings->GetBytes());


  if (Settings->GetLHAlgorithm() == MLMLAlgorithms::c_ClassicEM) {
    SetDeconvolutionAlgorithmClassicEM();
  } else if (Settings->GetLHAlgorithm() == MLMLAlgorithms::c_OSEM) {
    SetDeconvolutionAlgorithmOSEM(Settings->GetOSEMSubSets());
  } else {
    merr<<"Unknown deconvolution algorithm. Using classic EM."<<error;
    SetDeconvolutionAlgorithmClassicEM();
  }

  if (Settings->GetLHStopCriteria() == 0) {
    SetStopCriterionByIterations(Settings->GetNIterations());
  } else {
    merr<<"Unknown stop criterion. Stopping after 0 iterations."<<error;
    SetStopCriterionByIterations(0);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetEventSelectionSettings(MSettingsEventSelections* Settings)
{
  //! Set only the event reconstruction settings

  m_Selector.SetSettings(Settings);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetViewport(double x1Min, double x1Max, int x1NBins, 
                          double x2Min, double x2Max, int x2NBins,
                          double x3Min, double x3Max, int x3NBins,
                          MVector xAxis, MVector zAxis) 
{
  // Set the viewport of this event
  // In spherical coordinates x means theta and y means phi

  m_x1Min = x1Min;
  m_x1Max = x1Max;
  m_x1NBins = x1NBins;
  m_x2Min = x2Min;
  m_x2Max = x2Max;
  m_x2NBins = x2NBins;
  m_x3Min = x3Min;
  m_x3Max = x3Max;
  m_x3NBins = x3NBins;

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetDimensions(x1Min, x1Max, x1NBins, 
                            x2Min, x2Max, x2NBins, 
                            x3Min, x3Max, x3NBins,
                            xAxis, zAxis);
  }
   
  m_NBins = x1NBins*x2NBins*x3NBins;

  // Determine the 1-bin-axis for nearfield 2D imaging:
  m_TwoDAxis = 2;
  if (x1NBins == 1) {
    m_TwoDAxis = 0;
  } else if (x2NBins == 1) {
    m_TwoDAxis = 1;
  } else { 
    m_TwoDAxis = 2;
  }
  
  // Set the viewport also for the exposure calculation
  m_Exposure->SetDimensions(x1Min, x1Max, x1NBins, 
                            x2Min, x2Max, x2NBins, 
                            x3Min, x3Max, x3NBins,
                            xAxis, zAxis);
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetDeselectedPointSources(TObjArray* DeselectedPS)
{
  // Set the pointsources which are cut out of the picture

//   for (unsigned int t= 0; t < m_NThreads; ++t) {
//     m_BPs[t]->SetDeselectedPointSources(DeselectedPS);
//   }
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetResponseGaussian(const double Transversal, const double Longitudinal, const double Pair, const double CutOff, const bool UseAbsorptions)
{
  // Set the Gaussian response parameters

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponseGaussian* Response = new MResponseGaussian(Transversal, Longitudinal, Pair);
    Response->SetThreshold(CutOff);

    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetResponseGaussianByUncertainties(const double Increase)
{
  // Set the Gaussian response parameters

  for (unsigned int t = 0; t < m_NThreads; ++t) {
    MResponseGaussianByUncertainties* Response = new MResponseGaussianByUncertainties();
    Response->SetIncrease(Increase);
    Response->SetThreshold(2.5);

    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetExposureEfficiencyFile(MString FileName)
{
  // Set the exposure mode efficiency file
  
  if (m_Exposure->SetEfficiencyFile(FileName) == false) return false;
  
  for (unsigned int t = 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetEfficiency(m_Exposure->GetEfficiency());
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetApproximatedMaths(bool ApproximatedMaths)
{
  // Set the Gaussian response parameters

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetApproximatedMaths(ApproximatedMaths);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetResponseEnergyLeakage(double Electron, double Gamma)
{
  // Set the energy leakage response parameters

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponseEnergyLeakage* Response = new MResponseEnergyLeakage(Electron, Gamma);
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }
}

////////////////////////////////////////////////////////////////////////////////


void MImager::UseAbsorptions(bool UseAbsorptions) 
{ 
  //! Set if absorption probabilities should be use

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetUseAbsorptions(UseAbsorptions);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetResponseConeShapes(const MString& FileName)
{
  // Set the response matrices

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponseConeShapes* Response = new MResponseConeShapes();
    if (Response->LoadResponseFile(FileName) == false) {
      mgui<<"Unable to load response file!"<<endl;
      delete Response;
      return false;
    }
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetResponsePRM(const MString& ComptonTrans, 
                                const MString& ComptonLong, 
                                const MString& PairRadial)
{
  // Set the response matrices

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponsePRM* Response = new MResponsePRM();
    if (Response->LoadResponseFiles(ComptonTrans, ComptonLong, PairRadial) == false) {
      mgui<<"Unable to load response files!"<<endl;
      delete Response;
      return false;
    }
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetGeometry(MDGeometryQuest* Geometry)
{
  // Set the geometry

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetGeometry(Geometry);
  }
  m_Selector.SetGeometry(Geometry);
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetEventSelector(const MEventSelector& Selector)
{
  // Set all event parameters
  
  m_Selector = Selector;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::SetFileName(MString FileName, bool FastFileParsing)
{
  // Open the event file
  // Make sure to catch a false here, otherwise the class is not correctly initialized!

  if (m_EventFile.IsOpen() == true) {
    m_EventFile.Close();
  }
  if (m_EventFile.Open(FileName) == false) {
    return false;
  }
  m_FastFileParsing = FastFileParsing;
  m_EventFile.SetFastFileParsing(m_FastFileParsing);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetMemoryManagment(int MaxRAM, int MaxSwap, int Exhausted, int Accuracy)
{
  // Set the memory managment features

  m_MaxBytes = (unsigned long) MaxRAM * 1024 * 1024;
  m_ComputationAccuracy = Accuracy;
}


////////////////////////////////////////////////////////////////////////////////


MBPData* MImager::GetResponseSlice(unsigned int i)
{
  // Return the i-th event of the list-mode system-matrix
  
  massert(i < m_BPEvents.size());

  return m_BPEvents[i];
}
  

////////////////////////////////////////////////////////////////////////////////


void MImager::SetDeconvolutionAlgorithmClassicEM()
{
  //! Use the classic EM algorithm for deconvolution

  m_EM = new MLMLClassicEM();
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetDeconvolutionAlgorithmOSEM(unsigned int NSubSets)
{
 //! Use the OSEM algorithm for deconvolution

  MLMLOSEM* EM = new MLMLOSEM();
  EM->SetNSubSets(NSubSets);

  m_EM = dynamic_cast<MLMLAlgorithms*>(EM);
}


////////////////////////////////////////////////////////////////////////////////


void MImager::SetStopCriterionByIterations(int NIterations)
{
  //! Use a stop criterion by 

  if (m_EM == 0) {
    merr<<"You need to set a EM algorithm first!"<<show;
  } else {
    m_EM->UseStopCriterionByIterations(NIterations);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImager::AddResponseSlice(MBPData* Slice)
{
  // Add a response slice

  m_BPEvents.push_back(Slice);
  m_UsedBytes += Slice->GetUsedBytes();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MImager::GetNEvents()
{
  // Return the number of events the response is made of

  return m_BPEvents.size();
}
  
 
////////////////////////////////////////////////////////////////////////////////


int MImager::GetNImageBins()
{
  // return the number of bins of the image-space

  return m_NBins;
}


////////////////////////////////////////////////////////////////////////////////


MImage* MImager::CreateImage(MString Title, double* Data)
{
  //! Create an image

  MImage* Image = nullptr;
  
   // Display first backprojection for the three different coordinate systems:
  if (m_CoordinateSystem == MCoordinateSystem::c_Spheric) {
    Image = new MImageSpheric(Title, 
                              Data,
                              "Phi [deg]", 
                              m_x1Min*c_Deg,
                              m_x1Max*c_Deg, 
                              m_x1NBins,
                              "Theta [deg]", 
                              m_x2Min*c_Deg, 
                              m_x2Max*c_Deg, 
                              m_x2NBins,
                              "Intensity [a.u.]",
                              m_Palette, 
                              m_DrawMode);
  } else if (m_CoordinateSystem == MCoordinateSystem::c_Galactic) {
    Image = new MImageGalactic(Title, 
                               Data, 
                               "Galactic Longitude [deg]", 
                               m_x1Min*c_Deg,
                               m_x1Max*c_Deg, 
                               m_x1NBins,
                               "Galactic Latitude [deg]", 
                               m_x2Min*c_Deg-90, 
                               m_x2Max*c_Deg-90, 
                               m_x2NBins, 
                               "Intensity [a.u.]",
                               m_Palette, 
                               m_DrawMode,
                               m_SourceCatalog);
  } else if (m_CoordinateSystem == MCoordinateSystem::c_Cartesian2D) {
    if (m_TwoDAxis == 0) {
      Image = new MImage2D(Title,
                           Data,
                           "y [cm]",
                           m_x2Min, 
                           m_x2Max, 
                           m_x2NBins,
                           "z [cm]",
                           m_x3Min, 
                           m_x3Max, 
                           m_x3NBins, 
                           "Intensity [a.u.]",
                           m_Palette, 
                           m_DrawMode);
    } else if (m_TwoDAxis == 1) {
      Image = new MImage2D(Title,
                           Data,
                           "x [cm]",
                           m_x1Min, 
                           m_x1Max, 
                           m_x1NBins,
                           "z [cm]",
                           m_x3Min, 
                           m_x3Max, 
                           m_x3NBins, 
                           "Intensity [a.u.]",
                           m_Palette, 
                           m_DrawMode);
      
    } else {
      Image = new MImage2D(Title,
                           Data,
                           "x [cm]",
                           m_x1Min, 
                           m_x1Max, 
                           m_x1NBins,
                           "y [cm]",
                           m_x2Min, 
                           m_x2Max, 
                           m_x2NBins, 
                           "Intensity [a.u.]",
                           m_Palette, 
                           m_DrawMode);
    }
  } else if (m_CoordinateSystem == MCoordinateSystem::c_Cartesian3D) {
    Image = new MImage3D(Title,
                         Data,
                         "x [cm]",
                         m_x1Min, 
                         m_x1Max, 
                         m_x1NBins,
                         "y [cm]",
                         m_x2Min, 
                         m_x2Max, 
                         m_x2NBins,
                         "z [cm]",
                         m_x3Min, 
                         m_x3Max, 
                         m_x3NBins, 
                         "Intensity [a.u.]",
                         m_Palette, 
                         m_DrawMode);
  } else {
    merr<<"Unknown coordinate system ID: "<<m_CoordinateSystem<<fatal;
  } 
  
  return Image;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::Analyze(bool CalculateResponse)
{
  // Do the imaging

  if (m_AnimationMode == c_AnimateBackprojections) {
    CalculateResponse = true;
  }

  // Start with some sanity checks:
  for (unsigned int t= 0; t < m_NThreads; ++t) {
    if (m_BPs[t]->GetResponse() == nullptr) {
      merr<<"The fit-parameters have not been initialized: "
        "Call \"void MImager::SetResponse...(...)\" in advance."<<show;
      return false;
    }
  }

  MString Prefix;
  if (m_AnimationMode != c_AnimateNothing) {
    // Create a temporary file suffix:
    TRandom R;
    R.SetSeed(0); // 0 = Random seed defined by clock time
    for (unsigned int i = 0; i < 20; ++i) {
      Prefix += char(int('a') + R.Integer(26)); 
    }
    Prefix += "_";
  }

  //! Since we are in list-mode, compute the response slices:
  if (CalculateResponse == true || GetNEvents() == 0) {
    if (m_EventFile.IsOpen() == false) {
      merr<<"The event file is not open"<<show;
      return false;
    }

    m_EventFile.ShowProgress(true);
    if (m_AnimationMode == c_AnimateBackprojections) {
      mgui<<"Animation information: "<<endl;
      mgui<<"Do not cancel the response file creation process or the times will be wrong!"<<endl;
      mgui<<"However, later you can stop the gif file generation by simply closing the image."<<info;
    }
    ComputeResponseSlices();
    m_EventFile.ShowProgress(false);
    gSystem->ProcessEvents();
  }

  if (GetNEvents() == 0) {
    mgui<<"Sorry! No events passed the event selections!"<<info;
    return false;
  }
  
  mout<<"Preparing the first image... Please stand by..."<<endl;
  
  // Set the response to the EM algorithm 
  m_EM->SetResponseSlices(m_BPEvents, m_NBins);

  // Set the exposure
  m_EM->SetExposure(m_Exposure);

  if (m_Exposure->GetMode() != MExposureMode::Flat) {
    // Display the exposure map
    double* Map = m_Exposure->GetExposure();
    MImage* ExposureMap = CreateImage("Exposure Map", Map);
    delete [] Map;
    if (ExposureMap == nullptr) {
      // Error message already displayed
      return false; 
    }
    ExposureMap->Display();
  }
  
  // Display the initial image
  MImage* Image = CreateImage("Image - Iteration: 0", m_EM->GetInitialImage());
  if (Image == nullptr) {
    // Error message already displayed
    return false; 
  }
  Image->Normalize(true);
  Image->Display();

  
  // Making a back projection movie hack:
  
  if (m_AnimationMode == c_AnimateBackprojections) { 
    // Determine the total observation time:
    MTime Time = m_EventFile.GetObservationTime();
    double EventFraction = m_BPEvents.size()/Time.GetAsSeconds() * m_AnimationFrameTime;
    cout<<"Events per "<<m_AnimationFrameTime<<" sec: "<<EventFraction<<endl;

    double PictureID = 0;
    double* ImageArray = new double[m_NBins];
    for (int i = 0; i < m_NBins; ++i) ImageArray[i] = 0.0;
    ostringstream t;
    t<<"t = "<<PictureID*m_AnimationFrameTime<<"sec";
    Image->SetTitle(t.str().c_str());
    Image->SetImageArray(ImageArray);
    Image->Display();
    ostringstream s;
    s<<Prefix<<setw(5)<<setfill('0')<<0<<".gif";
    Image->SaveAs(s.str().c_str());

    MString NameFirst = s.str().c_str();
    MString NameLast = "";

    for (unsigned int e = 0; e < m_BPEvents.size(); ++e) {
      m_BPEvents[e]->Sum(ImageArray, m_NBins);

      // Time
      //if (e == 0) PictureID = e-1;
      
      while (e > PictureID*EventFraction) {
        PictureID += 1.0;
        //if (PictureID*m_AnimationFrameTime > 300) break;
        
        ostringstream Title;
        Title<<"t = "<<PictureID*m_AnimationFrameTime<<"sec";
        if (Image->CanvasExists() == false) break;
        Image->SetTitle(Title.str().c_str());
        Image->SetImageArray(ImageArray);
        
        ostringstream Save;
        Save<<Prefix<<setw(5)<<setfill('0')<<int(PictureID)<<".gif";
        Image->SaveAs(Save.str().c_str());
        NameLast = Save.str().c_str();

        if (e == 0) break;
        gSystem->ProcessEvents();
      }
      if (Image->CanvasExists() == false) break;
      //if (PictureID*m_AnimationFrameTime >= 300) break;
    }
    
    // Concatenate images
    mout<<"Started creating animation... please wait a while..."<<endl;
    ostringstream command;
    command<<"convert -loop 2 -delay 20 "<<Prefix<<"*.gif "<<m_AnimationFileName<<endl;
    gSystem->Exec(command.str().c_str());
    
    MString First = m_AnimationFileName;
    First = First.ReplaceAll(".gif", ".first.gif");
    if (First == m_AnimationFileName) First.Append(".first.gif");
    gSystem->Rename(NameFirst, First);
    
    MString Last = m_AnimationFileName;
    Last = Last.ReplaceAll(".gif", ".last.gif");
    if (Last == m_AnimationFileName) Last.Append(".last.gif");
    gSystem->Rename(NameLast, Last);
    
    
    mgui<<"The file "<<m_AnimationFileName<<" has been generated."<<endl;
    mgui<<"Since MEGAlib does not want to delete any files, you have to delete the intermediary files"<<endl;
    mgui<<Prefix<<"*.gif by yourself."<<info;
    
    return true;
  } else if (m_AnimationMode == c_AnimateIterations) {
    ostringstream s;
    s<<Prefix<<setw(5)<<setfill('0')<<0<<".gif";
    Image->SaveAs(s.str().c_str());    
  }


  // Store the images:
  for (unsigned int i = 0; i < m_Images.size(); ++i) {
    delete m_Images[i];
  }
  m_Images.clear();
  m_Images.push_back(Image->Clone());
 

  // Return if no iterations have to be performed
  if (m_EM->IsStopCriterionFullfilled() == true) {
    mout<<endl;
    delete Image;
    return true;
  }  

  MTimer IterationTimer;
  
  // Now iterate...
  MGUIProgressBar* Progress = nullptr;
  if (gROOT->IsBatch() == false) {
    Progress = new MGUIProgressBar();
    Progress->SetTitles("Progress", "Progress of deconvolution iterations");
    Progress->SetMinMax(0, m_EM->GetMaxNIterations());
  }
  int CurrentIteration = 0;
  while (true) {
    m_EM->DoOneIteration();
    ++CurrentIteration;
    
    if (gROOT->IsBatch() == false) {
      Progress->SetValue(CurrentIteration);
      gSystem->ProcessEvents();
    }
    
    ostringstream Title;
    Title<<"Image - iteration: "<<CurrentIteration;
    
    if (Image->CanvasExists() == false) break;
    Image->SetTitle(Title.str().c_str());
    Image->SetImageArray(m_EM->GetImage());
    if (m_AnimationMode == c_AnimateIterations) {
      ostringstream s;
      s<<Prefix<<setw(5)<<setfill('0')<<CurrentIteration<<".gif";
      Image->SaveAs(s.str().c_str());
    }
    m_Images.push_back(Image->Clone());
   
    if (m_EM->IsStopCriterionFullfilled() == true || 
        (gROOT->IsBatch() == false && Progress->TestCancel() == true)) {
      break;
    }
  }
  delete Progress;
  
    
  if (m_AnimationMode == c_AnimateIterations) {
    // Concatenate images
    mout<<"Started creating animation... please wait a while..."<<endl;
    ostringstream command;
    command<<"convert -loop 1 -delay 100 "<<Prefix<<"*.gif "<<m_AnimationFileName<<endl;
    gSystem->Exec(command.str().c_str());
    
    mgui<<"The file "<<m_AnimationFileName<<" has been generated."<<endl;
    mgui<<"Since MEGAlib des not want to delete any files, you have to delete the intermediary files"<<endl;
    mgui<<Prefix<<"*.gif by yourself."<<info;
  }

  
  IterationTimer.Pause();
  if (CurrentIteration > 0) {
    mout<<"Performed "<<CurrentIteration<<" iterations in "<<IterationTimer.GetElapsed()<<" seconds ("<<IterationTimer.GetElapsed()/CurrentIteration<<" seconds/iteration)"<<endl;
  }
  mout<<endl;
  
  // end iteration part

  delete Image;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MImager::ComputeResponseSlices()
{
  // Computes and stores the system-matrix

  // Test if everything has been initialized:

  // Available memory:
  MSystem System;

  // Switch to four byte storage if we exceed 2^16 bins:
  if (m_NBins >= 65536) {
    m_ComputationAccuracy = 1;
  }

  // Prepare data-storage:

  // Stop the time ...
  TTime time;
  time = gSystem->Now();


  // Prepare the response calculation
  for (unsigned int t = 0; t < m_NThreads; ++t) {
    m_BPs[t]->PrepareBackprojection();
  }

  m_EventFile.ShowProgress(true);
  m_EventFile.SetProgressTitle("Progress", "Progress of response slice generation");

  if (m_NThreads > 1) {
    m_EventFile.SetAutomaticProgressUpdates(false);
    m_EventFile.SetDelayedFileParsing(true);
    m_EventFile.StartThread();
    

    // Start threads

    for (unsigned int t = 0; t < m_NThreads; ++t) {
      MString Name = "Imaging thread #";
      Name += t;
      TThread* Thread = new TThread(Name, (void(*) (void *)) &MImager_CallThread, (void*) new MImager_ThreadCaller(this, t));
      m_Threads[t] = Thread;
      m_ThreadIsInitialized[t] = false;
      m_ThreadIsFinished[t] = false;
      m_ThreadShouldFinish[t] = false;

      Thread->Run();
      
      // Wait until thread is initialized:
      while (m_ThreadIsInitialized[t] == false) {
        // Sleep for a while...
        TThread::Sleep(0, 10000000);
      }    

      cout<<Name<<" is running"<<endl;
    }
    
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a while...
      TThread::Sleep(0, 10000000);
      
      if (m_EventFile.UpdateProgress() == false) {
        for (unsigned int t = 0; t < m_NThreads; ++t) {
          m_ThreadShouldFinish[t] = true; 
        }
      }
      
      ThreadsAreRunning = false;
      for (unsigned int t = 0; t < m_NThreads; ++t) {
        if (m_ThreadIsFinished[t] == false) {
          ThreadsAreRunning = true;
          break;
        }
      }
    }

    // None of the threads are running any more --- kill them
    for (unsigned int t = 0; t < m_NThreads; ++t) {
      m_Threads[t]->Kill();
      m_Threads[t] = 0;
    }
  }
  // Non-threaded mode
  else {
    ResponseSliceComputationThread(0);
  }
  m_EventFile.Close();

  if (m_OutOfMemory == true) {
    // Free some space --- remove 5% of the stored BPs
    unsigned int NEventsToDelete = m_BPEvents.size()/20;
    mout<<"Out of memory condition: Erasing "<<NEventsToDelete<<" events to free some memory..."<<endl;
    for (unsigned int i = 0; i < NEventsToDelete; ++i) {
      vector<MBPData*>::iterator Iter = m_BPEvents.begin();
      m_UsedBytes -= (*Iter)->GetUsedBytes();
      delete *Iter;
      m_BPEvents.erase(Iter);
    }
  }

  double Elapsed = 0.001*long(gSystem->Now() - time);
  mout<<endl;
  mout<<"Response calculation finished after "<<Elapsed<< " seconds ("<<m_BPEvents.size()<<" event"<<((m_BPEvents.size() != 1) ? "s" : "")<<", "<<m_BPEvents.size()/Elapsed<<" events/sec)";
  if (m_UsedBytes > 0 && m_BPEvents.size() > 0) {
    mout<<" using ~"<<m_UsedBytes/1024/1024<<" MB of RAM ("<<m_UsedBytes/m_BPEvents.size()<<" Bytes/event)."<<endl;
  } 

  return (m_BPEvents.size() > 0) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////


void* MImager::ResponseSliceComputationThread(unsigned int ThreadID)
{
  // This function is part of the thread
  // Lock all pieces of this class which might be accessed by any other thread!

  double Maximum = 0;
  int NUsedBins = 0;

  bool EnoughMemory = true;
  MPhysicalEvent* Event;
  MBPData* Data = 0;

  double* BackprojectionImage = new double[m_NBins];
  int* BackprojectionBins = new int[m_NBins];

  // Create a local copy of the backprojection class 
  
  m_Mutex.Lock();
  m_ThreadIsInitialized[ThreadID] = true;
  m_Mutex.UnLock();


  // The reconstruction loop
  while (m_ThreadShouldFinish[ThreadID] == false) {
    if (m_OutOfMemory == true) break;

    /*
    if (GetNEvents() >= 2) {
      m_ThreadShouldFinish[ThreadID] = true;
      break;
    }
    */
    
    Event = m_EventFile.GetNextEvent();

    // If we don't get an event a serious error ocurred, or we are finished
    if (Event == 0) break;

    // We activated delayed file parsing for multiple threads:
    if (m_NThreads > 1) {
      Event->ParseDelayed(m_FastFileParsing);
    }

    /// IsQualified is NOT reentrant --- but the only thing modified are its counters, which we do not use here...
    if (m_Selector.IsQualifiedEventFast(Event) == true) {
      // Reinitialize the array keeping the events backprojection 
      // Memcopy is only faster if the parallism of modern CPUs cannot be used. With gcc -O3 this is fastest:
      //for (int i = 0; i < m_NBins; ++i) BackprojectionImage[i] = 0.0; 

      // Try to backproject the data and store the computed t_ij in BackprojectionImage
      NUsedBins = 0;
      if (m_BPs[ThreadID]->Backproject(Event, BackprojectionImage, BackprojectionBins, NUsedBins, Maximum) == true && NUsedBins > 0) {

        // It might happen that we go out of memory during imaging, catch it!
        // 1-byte-storage:
        if (m_ComputationAccuracy == 0) {
          // Test if we can store it as sparse matrix:
          if (NUsedBins < 0.33*m_NBins) {
            Data = new(nothrow) MBPDataSparseImageOneByte();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(BackprojectionImage, BackprojectionBins, m_NBins, NUsedBins, Maximum);
            } else {
              EnoughMemory = false;
            }
          } else { // no sparse matrix
            Data = new(nothrow) MBPDataImageOneByte();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(BackprojectionImage, BackprojectionBins, m_NBins, NUsedBins, Maximum);
            } else {
              EnoughMemory = false;
            }
          }
        } 
        // 4-byte storage:
        else if (m_ComputationAccuracy == 1) {
          if (NUsedBins < 0.5*m_NBins) {
            Data = new(nothrow) MBPDataSparseImage();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(BackprojectionImage, BackprojectionBins, m_NBins, NUsedBins, Maximum);
            } else {
              EnoughMemory = false;
            }
          }
          else { // no sparse matrix
            Data = new(nothrow) MBPDataImage();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(BackprojectionImage, BackprojectionBins, m_NBins, NUsedBins, Maximum);
            } else {
              EnoughMemory = false;
            }
         }
        } else {
          // "merr" not thread safe --- but we crash anyway ;-)
          merr<<"m_ComputationAccuracy must be 0 (1 byte storage) or 1 (4 byte storage): "<<m_ComputationAccuracy<<fatal;
        }

        if (EnoughMemory == false) {
          cout<<"Thread "<<ThreadID<<": Out of memory --- finishing..."<<endl;
          delete Event;
          break;
        }
          
        m_Mutex.Lock();
        AddResponseSlice(Data);
        if (GetUsedBytes() > m_MaxBytes) {
          cout<<"Thread "<<ThreadID<<": Used RAM exceeds the user set maximum ("<<m_MaxBytes/1024/1024<<" MB)  --- finishing..."<<endl;
          m_Mutex.UnLock();
          break;
        }
        m_Mutex.UnLock();
      }
      
      // Hack for not multi-threading...
      if (ThreadID == 0) {
        m_Exposure->Expose(Event); 
      }
    }
    
    //m_Mutex.Lock();
    delete Event;
    //m_Mutex.UnLock();
  }

  //m_Mutex.Lock();
  delete [] BackprojectionImage;
  delete [] BackprojectionBins;
  //m_Mutex.UnLock();
  
  m_Mutex.Lock();
  m_ThreadIsFinished[ThreadID] = true;
  if (EnoughMemory == false) {
    m_OutOfMemory = true;
  }
  m_Mutex.UnLock();

  return 0;
}


// MImager: the end...
////////////////////////////////////////////////////////////////////////////////
