/*
 * MCalibrateEnergyDetermineModel.cxx
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


// Include the header:
#include "MCalibrateEnergyDetermineModel.h"

// Standard libs:
#include <algorithm>
#include <cmath>
using namespace std;

// ROOT libs:
#include "TF1.h"
#include "TMath.h"
#include "TBox.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MCalibrationSpectralPoint.h"
#include "MReadOutDataADCValue.h"
#include "MCalibrationFit.h"
#include "MCalibrationFitGaussian.h"
#include "MCalibrationFitGaussLandau.h"
#include "MCalibrationModel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibrateEnergyDetermineModel)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateEnergyDetermineModel::MCalibrateEnergyDetermineModel() : MCalibrateEnergy()
{  
  m_CalibrationModelDeterminationMethod = c_CalibrationModelStepWise;
  m_CalibrationModelDeterminationMethodFittingEnergyModel = MCalibrationModel::c_CalibrationModelPoly3;
  m_CalibrationModelDeterminationMethodFittingFWHMModel = MCalibrationModel::c_CalibrationModelPoly1;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrateEnergyDetermineModel::~MCalibrateEnergyDetermineModel()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyDetermineModel::Calibrate()
{
  bool Return = true;
  
  if (CalibrateEnergyModel() == false) Return = false;
  if (CalibrateFWHMModel() == false) Return = false;
  
  return Return;
}
  
  
////////////////////////////////////////////////////////////////////////////////
  
  
//! Perform the calibration
bool MCalibrateEnergyDetermineModel::CalibrateEnergyModel()
{
  // Clean up the results:
  m_Results.RemoveEnergyModel();
  
  // Assemble the unique lines:
  vector<MCalibrationSpectralPoint> Points = m_Results.GetUniquePoints();
  if (Points.size() < 1) {
    cout<<"Not enough points to determine a calibration model. Only "<<Points.size()<<" calibration point(s) available."<<endl;  
    return true;
  }
  
  if (m_CalibrationModelDeterminationMethod == c_CalibrationModelStepWise) {
    // we are already done since this is used during peak finding 
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model steps"<<endl;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelFit) {
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model fit"<<endl;
    
    // Set up the model:
    MCalibrationModel* Model = 0;
    if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Zero) {
      Model = new MCalibrationModelPoly1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1) {
      Model = new MCalibrationModelPoly1();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly2) {
      Model = new MCalibrationModelPoly2();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly3) {
      Model = new MCalibrationModelPoly3();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly4) {
      Model = new MCalibrationModelPoly4();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Inv1Zero) {
      Model = new MCalibrationModelPoly1Inv1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Inv1) {
      Model = new MCalibrationModelPoly1Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly2Inv1Zero) {
      Model = new MCalibrationModelPoly2Inv1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly2Inv1) {
      Model = new MCalibrationModelPoly2Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Exp1) {
      Model = new MCalibrationModelPoly1Exp1();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Exp2) {
      Model = new MCalibrationModelPoly1Exp2();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Exp3) {
      Model = new MCalibrationModelPoly1Exp3();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly1Log1) {
      Model = new MCalibrationModelPoly1Log1();
    } else if (m_CalibrationModelDeterminationMethodFittingEnergyModel == MCalibrationModel::c_CalibrationModelPoly2Log1) {
      Model = new MCalibrationModelPoly2Log1();
    } else {
      new MExceptionUnknownMode("fitting model to determine calibration model", m_CalibrationModelDeterminationMethodFittingEnergyModel);
      return false;
    }
    
    if (Points.size() < Model->GetNParameters()) {
      if (g_Verbosity >= c_Warning) cout<<"Warning: We have more fit parameters ("<<Model->GetNParameters()<<") than data points ("<<Points.size()<<")!"<<endl;
      return true;
    }
    
    double Quality = Model->Fit(Points);
    if (g_Verbosity >= c_Info) cout<<"Fit quality: "<<Quality<<endl;
    
    m_Results.SetEnergyModel(*Model);
    
    //delete Model;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelBestFit) {
    if (g_Verbosity >= c_Info) cout<<"Find best (fitted) calibration model"<<endl;
    
    // Assemble the models
    vector<MCalibrationModel*> Models;
    Models.push_back(new MCalibrationModelPoly1Zero());
    Models.push_back(new MCalibrationModelPoly1());
    Models.push_back(new MCalibrationModelPoly2());
    Models.push_back(new MCalibrationModelPoly3());
    Models.push_back(new MCalibrationModelPoly4());
    Models.push_back(new MCalibrationModelPoly1Inv1Zero());
    Models.push_back(new MCalibrationModelPoly1Inv1());
    Models.push_back(new MCalibrationModelPoly2Inv1Zero());
    Models.push_back(new MCalibrationModelPoly2Inv1());
    Models.push_back(new MCalibrationModelPoly1Exp1());
    Models.push_back(new MCalibrationModelPoly1Exp2());
    Models.push_back(new MCalibrationModelPoly1Exp3());
    Models.push_back(new MCalibrationModelPoly1Log1());
    Models.push_back(new MCalibrationModelPoly2Log1());
    
    vector<double> Results;
    for (unsigned int m = 0; m < Models.size(); ++m) {
      double Result = Models[m]->Fit(Points);
      
      if (Points.size() < Models[m]->NPar()) {
        Results.push_back(numeric_limits<double>::max());
        continue;
      }
      
      cout<<"Model "<<Models[m]->GetName()<<": (chi-square="<<Result<<")"<<endl;
      if (Result >= 0 && isfinite(Result) && Result < numeric_limits<double>::max()) {
        Results.push_back(Models[m]->Fit(Points));
        if (g_Verbosity >= c_Info) cout<<"Model "<<Models[m]->GetName()<<": Good fit! (chi-square="<<Results.back()<<")"<<endl;
      } else {
        Results.push_back(numeric_limits<double>::max());
        if (g_Verbosity >= c_Info) cout<<"Model "<<Models[m]->GetName()<<": Bad fit! (chi-square="<<Results.back()<<")"<<endl;        
      }
    }
    
    vector<double>::iterator MinI;
    MinI = min_element(Results.begin(), Results.end());
    
    int Min = int(MinI -  Results.begin());
    if (g_Verbosity >= c_Info) cout<<"Best model: "<<Models[Min]->GetName()<<" with Result: "<<Results[Min]<<endl;
    
    m_Results.SetEnergyModel(*Models[Min]);
    
    for (unsigned int m = 0; m < Models.size(); ++m) {
      //delete Models[m];
    }
  } else {
    new MExceptionUnknownMode("calibration model determination method", m_CalibrationModelDeterminationMethod);
    return false;
  }
  
  // Finally set the FWHM of each of the good points
  for (unsigned int r = 0; r < m_Results.GetNumberOfReadOutDataGroups(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == true) {
        MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(r, p);
        if (P.HasFit() == true) {
          MCalibrationFit& Fit = P.GetFit();
          double FWHM = Fit.GetFWHM();
          P.SetEnergyFWHM(m_Results.GetEnergyModel().GetFitValue(P.GetPeak() + 0.5*FWHM) - m_Results.GetEnergyModel().GetFitValue(P.GetPeak() - 0.5*FWHM));
        }

        // We cannot do this, since the old FWHM was estimated and set before the fit
        //P.SetEnergyFWHM(m_Results.GetEnergyModel().GetFitValue(P.GetPeak() + 0.5*P.GetFWHM()) - m_Results.GetEnergyModel().GetFitValue(P.GetPeak() - 0.5*P.GetFWHM()));
      }
    }
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyDetermineModel::CalibrateFWHMModel()
{
  // Clean up the results:
  m_Results.RemoveFWHMModel();
  
  // Assemble the unique lines:
  vector<MCalibrationSpectralPoint> Points = m_Results.GetUniquePoints();
  if (Points.size() < 1) {
    cout<<"Not enough points to determine a line-width calibration model. Only "<<Points.size()<<" calibration point(s) available."<<endl;  
    return true;
  }
  
  if (m_CalibrationModelDeterminationMethod == c_CalibrationModelStepWise) {
    // we are already done since this is used during peak finding 
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model steps"<<endl;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelFit) {
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model fit"<<endl;
    
    // Set up the model:
    MCalibrationModel* Model = 0;
    if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Zero) {
      Model = new MCalibrationModelPoly1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1) {
      Model = new MCalibrationModelPoly1();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly2) {
      Model = new MCalibrationModelPoly2();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly3) {
      Model = new MCalibrationModelPoly3();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly4) {
      Model = new MCalibrationModelPoly4();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Inv1Zero) {
      Model = new MCalibrationModelPoly1Inv1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Inv1) {
      Model = new MCalibrationModelPoly1Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly2Inv1Zero) {
      Model = new MCalibrationModelPoly2Inv1Zero();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly2Inv1) {
      Model = new MCalibrationModelPoly2Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Exp1) {
      Model = new MCalibrationModelPoly1Exp1();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Exp2) {
      Model = new MCalibrationModelPoly1Exp2();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Exp3) {
      Model = new MCalibrationModelPoly1Exp3();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly1Log1) {
      Model = new MCalibrationModelPoly1Log1();
    } else if (m_CalibrationModelDeterminationMethodFittingFWHMModel == MCalibrationModel::c_CalibrationModelPoly2Log1) {
      Model = new MCalibrationModelPoly2Log1();
    } else {
      new MExceptionUnknownMode("fitting model to determine calibration model", m_CalibrationModelDeterminationMethodFittingFWHMModel);
      return false;
    }
    
    if (Points.size() < Model->GetNParameters()) {
      if (g_Verbosity >= c_Warning) cout<<"Warning: We have more fit parameters ("<<Model->GetNParameters()<<") than data points ("<<Points.size()<<")!"<<endl;
      return true;
    }
    
    Model->SetType(MCalibrationModelType::c_LineWidth);
    
    double Quality = Model->Fit(Points);
    if (g_Verbosity >= c_Info) cout<<"Fit quality: "<<Quality<<endl;
    
    m_Results.SetFWHMModel(*Model);
    
    //delete Model;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelBestFit) {
    if (g_Verbosity >= c_Info) cout<<"Find best (fitted) calibration model"<<endl;
    
    // Assemble the models
    vector<MCalibrationModel*> Models;
    Models.push_back(new MCalibrationModelPoly1Zero());
    Models.push_back(new MCalibrationModelPoly1());
    Models.push_back(new MCalibrationModelPoly2());
    Models.push_back(new MCalibrationModelPoly3());
    Models.push_back(new MCalibrationModelPoly4());
    Models.push_back(new MCalibrationModelPoly1Inv1Zero());
    Models.push_back(new MCalibrationModelPoly1Inv1());
    Models.push_back(new MCalibrationModelPoly2Inv1Zero());
    Models.push_back(new MCalibrationModelPoly2Inv1());
    Models.push_back(new MCalibrationModelPoly1Exp1());
    Models.push_back(new MCalibrationModelPoly1Exp2());
    Models.push_back(new MCalibrationModelPoly1Exp3());
    Models.push_back(new MCalibrationModelPoly1Log1());
    Models.push_back(new MCalibrationModelPoly2Log1());
    
    vector<double> Results;
    for (unsigned int m = 0; m < Models.size(); ++m) {
      Models[m]->SetType(MCalibrationModelType::c_LineWidth);
      
      if (Points.size() < Models[m]->NPar()) {
        Results.push_back(numeric_limits<double>::max());
        continue;
      }
      
      double Result = Models[m]->Fit(Points);
      if (Result >= 0 && isfinite(Result) && Result < numeric_limits<double>::max()) {
        Results.push_back(Models[m]->Fit(Points));
        if (g_Verbosity >= c_Info) cout<<"Model "<<Models[m]->GetName()<<": Good fit! (chi-square="<<Results.back()<<")"<<endl;
      } else {
        Results.push_back(numeric_limits<double>::max());
        if (g_Verbosity >= c_Info) cout<<"Model "<<Models[m]->GetName()<<": Bad fit! (chi-square="<<Results.back()<<")"<<endl;        
      }
    }
    
    vector<double>::iterator MinI;
    MinI = min_element(Results.begin(), Results.end());
    
    int Min = int(MinI -  Results.begin());
    if (g_Verbosity >= c_Info) cout<<"Best model: "<<Models[Min]->GetName()<<" with Result: "<<Results[Min]<<endl;
    
    m_Results.SetFWHMModel(*Models[Min]);
    
    for (unsigned int m = 0; m < Models.size(); ++m) {
      //TODO: Fix memory leak
      //delete Models[m];
    }
  } else {
    new MExceptionUnknownMode("calibration model determination method", m_CalibrationModelDeterminationMethod);
    return false;
  }
  
  return true;
}


// MCalibrateEnergyDetermineModel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
