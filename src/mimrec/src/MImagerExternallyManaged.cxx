/*
 * MImagerExternallyManaged.cxx
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
// MImagerExternallyManaged.cxx
//
//
// Computes and stores the data of the system-matrix.
// Make sure to set all data properly in advance.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImagerExternallyManaged.h"

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
#include "MBackprojectionSphere.h"
#include "MBackprojectionCartesian.h"
#include "MBPDataSparseImage.h"
#include "MBPDataSparseImageOneByte.h"
#include "MPhysicalEvent.h"
#include "MBPDataImage.h"
#include "MBPDataImageOneByte.h"
#include "MGUIProgressBar.h"
#include "MSystem.h"
#include "MResponse.h"
#include "MResponseGaussian.h"
#include "MResponsePRM.h"
#include "MResponseEnergyLeakage.h"
#include "MSensitivity.h" 
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
ClassImp(MImagerExternallyManaged)
#endif


////////////////////////////////////////////////////////////////////////////////


MImagerExternallyManaged::MImagerExternallyManaged(int CoordinateSystem) : MImager(CoordinateSystem, 1)
{
  m_UseGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MImagerExternallyManaged::~MImagerExternallyManaged()
{
}

////////////////////////////////////////////////////////////////////////////////


bool MImagerExternallyManaged::Initialize()
{
  //! Call before the response slice calculation after all options are set

  for (unsigned int t = 0; t < m_NThreads; ++t) {
    m_BPs[t]->PrepareBackprojection();
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


vector<MImage*> MImagerExternallyManaged::Deconvolve(vector<MBPData*> ResponseSlices)
{
  // Do the imaging

  vector<MImage*> Images;

  // Set the response to the EM algorithm 
  m_EM->SetResponseSlices(ResponseSlices, m_NBins);

  m_EM->EnableGUIInteractions(m_UseGUI);
  m_EM->ResetStopCriterion();

  // Image display
  MImage* Image = 0;
  
  // Display first backprojection for the three different coordinate systems:
  if (m_CoordinateSystem == MProjection::c_Spheric) {
    Image = new MImageSpheric("Image - Iteration: 0", 
                              m_EM->GetInitialImage(),
                              "Phi [deg]", 
                              m_x1Min*c_Deg,
                              m_x1Max*c_Deg, 
                              m_x1NBins,
                              "Theta [deg]", 
                              m_x2Min*c_Deg, 
                              m_x2Max*c_Deg, 
                              m_x2NBins, 
                              m_Palette, 
                              m_DrawMode,
                              m_SourceCatalog);
  } else if (m_CoordinateSystem == MProjection::c_Galactic) {
    Image = new MImageGalactic("Image - Iteration: 0", 
                               m_EM->GetInitialImage(), 
                               "Longitude [deg]", 
                               m_x1Min*c_Deg,
                               m_x1Max*c_Deg, 
                               m_x1NBins,
                               "Latitude [deg]", 
                               m_x2Min*c_Deg-90, 
                               m_x2Max*c_Deg-90, 
                               m_x2NBins, 
                               m_Palette, 
                               m_DrawMode,
                               m_SourceCatalog);
  } else if (m_CoordinateSystem == MProjection::c_Cartesian2D) {
    if (m_TwoDAxis == 0) {
      Image = new MImage2D("Image - Iteration: 0",
                           m_EM->GetInitialImage(),
                           "y [cm]",
                           m_x2Min, 
                           m_x2Max, 
                           m_x2NBins,
                           "z [cm]",
                           m_x3Min, 
                           m_x3Max, 
                           m_x3NBins, 
                           m_Palette, 
                           m_DrawMode);

    } else if (m_TwoDAxis == 1) {
      Image = new MImage2D("Image - Iteration: 0",
                           m_EM->GetInitialImage(),
                           "x [cm]",
                           m_x1Min, 
                           m_x1Max, 
                           m_x1NBins,
                           "z [cm]",
                           m_x3Min, 
                           m_x3Max, 
                           m_x3NBins, 
                           m_Palette, 
                           m_DrawMode);
      
    } else {
      Image = new MImage2D("Image - Iteration: 0",
                           m_EM->GetInitialImage(),
                           "x [cm]",
                           m_x1Min, 
                           m_x1Max, 
                           m_x1NBins,
                           "y [cm]",
                           m_x2Min, 
                           m_x2Max, 
                           m_x2NBins, 
                           m_Palette, 
                           m_DrawMode);
    }

  } else if (m_CoordinateSystem == MProjection::c_Cartesian3D) {
    Image = new MImage3D("Image - Iteration: 0",
                         m_EM->GetInitialImage(),
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
                         m_Palette, 
                         m_DrawMode);
  } else {
    merr<<"Unknown coordinate system ID: "<<m_CoordinateSystem<<fatal;
    return Images;
  }


  // Store the images:
  Images.push_back(Image->Clone());
 

  // Return if no iterations have to be performed
  if (m_EM->IsStopCriterionFullfilled() == true) {
    return Images;
  }  

  MTimer IterationTimer;
  
  // Now iterate...
  MGUIProgressBar* Progress = 0;
  if (m_UseGUI == true) {
    Progress = new MGUIProgressBar();
    Progress->SetTitles("Progress", "Progress of deconvolution iterations");
    Progress->SetMinMax(0, m_EM->GetMaxNIterations());
  }
  int CurrentIteration = 0;
  while (true) {
    m_EM->DoOneIteration();
    ++CurrentIteration;
    if (m_UseGUI == true) {
      Progress->SetValue(CurrentIteration);
      gSystem->ProcessEvents();
    }
    
    ostringstream Title;
    Title<<"Image - iteration: "<<CurrentIteration;
    
    Image->SetTitle(Title.str().c_str());
    Image->SetImageArray(m_EM->GetImage());
    Images.push_back(Image->Clone());
   
    if (m_EM->IsStopCriterionFullfilled() == true) {
      break;
    }
    if (m_UseGUI == true && Progress->TestCancel() == true) {
      break;
    }
  }
  delete Progress;

  
  IterationTimer.Pause();
  //mout<<"Performed "<<CurrentIteration<<" iterations in "<<IterationTimer.GetElapsed()<<" seconds ("<<CurrentIteration/IterationTimer.GetElapsed()<<" iterations/second)"<<endl;
  //mout<<endl;

  // end iteration part

  return Images;
}


////////////////////////////////////////////////////////////////////////////////


MBPData* MImagerExternallyManaged::CalculateResponseSlice(MPhysicalEvent* Event)
{
  // Calculate the response slice for the given event

  MBPData* Data = 0;

  // Switch to four byte storage if we exceed 2^16 bins:
  if (m_NBins >= 65536) {
    m_ComputationAccuracy = 1;
  }

  // IsQualified is NOT reentrant --- but the only thing modified are its counters, which we do not use here...
  if (m_Selector.IsQualifiedEvent(Event) == true) {
    // Reinitialize the array keeping the events backprojection 
    // Memcopy is only faster if the parallism of modern CPUs cannot be used. With gcc -O3 this is fastest:
    //for (int i = 0; i < m_NBins; ++i) BackprojectionImage[i] = 0.0; 

    bool EnoughMemory = true;

    double* BackprojectionImage = new double[m_NBins];
    int* BackprojectionBins = new int[m_NBins];
    
    // Try to backproject the data and store the computed t_ij in BackprojectionImage
    int NUsedBins = 0;
    double Maximum = 0;
    if (m_BPs[0]->Backproject(Event, BackprojectionImage, BackprojectionBins, NUsedBins, Maximum) == true && NUsedBins > 0) {

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
        merr<<"m_ComputationAccuracy must be 0 (1 byte storage) or 1 (4 byte storage): "<<m_ComputationAccuracy<<fatal;
      }
    }

    delete [] BackprojectionImage;
    delete [] BackprojectionBins;
 
    if (EnoughMemory == false) {
      cout<<"Out of memory..."<<endl;
    }
    
  }

  return Data;
}


// MImagerExternallyManaged: the end...
////////////////////////////////////////////////////////////////////////////////
