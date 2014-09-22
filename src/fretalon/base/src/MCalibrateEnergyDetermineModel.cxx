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


#ifdef ___CINT___
ClassImp(MCalibrateEnergyDetermineModel)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateEnergyDetermineModel::MCalibrateEnergyDetermineModel() : MCalibrateEnergy()
{  
  m_CalibrationModelDeterminationMethod = c_CalibrationModelStepWise;
  m_CalibrationModelDeterminationMethodFittingModel = MCalibrationModel::c_CalibrationModelPoly3;
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
  // Assemble the unique lines:
  vector<MCalibrationSpectralPoint> Points = m_Results.GetUniquePoints();
  if (Points.size() < 2) return true;
  
  if (m_CalibrationModelDeterminationMethod == c_CalibrationModelStepWise) {
    // we are already done since this is used during peak finding 
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model steps"<<endl;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelFit) {
    if (g_Verbosity >= c_Chatty) cout<<"Doing calibration model fit"<<endl;
    
    // Set up the model:
    MCalibrationModel* Model = 0;
    if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1) {
      Model = new MCalibrationModelPoly1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly2) {
      Model = new MCalibrationModelPoly2();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly3) {
      Model = new MCalibrationModelPoly3();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly4) {
      Model = new MCalibrationModelPoly4();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Inv1) {
      Model = new MCalibrationModelPoly1Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp1) {
      Model = new MCalibrationModelPoly1Exp1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp2) {
      Model = new MCalibrationModelPoly1Exp2();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp3) {
      Model = new MCalibrationModelPoly1Exp3();
    } else {
      new MExceptionUnknownMode("fitting model to determine calibration model", m_CalibrationModelDeterminationMethodFittingModel);
      return false;
    }
    
    if (Points.size() < Model->GetNParameters()) {
      if (g_Verbosity >= c_Warning) cout<<"Warning: We have more fit parameters ("<<Model->GetNParameters()<<") than data points ("<<Points.size()<<")!"<<endl;
      return true;
    }
    
    double Quality = Model->Fit(Points);
    if (g_Verbosity >= c_Info) cout<<"Fit quality: "<<Quality<<endl;
    
    m_Results.SetModel(*Model);
    
    //delete Model;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelBestFit) {
    if (g_Verbosity >= c_Info) cout<<"Find best (fitted) calibration model"<<endl;
    
    // Assemble the models
    vector<MCalibrationModel*> Models;
    Models.push_back(new MCalibrationModelPoly1());
    Models.push_back(new MCalibrationModelPoly2());
    Models.push_back(new MCalibrationModelPoly3());
    Models.push_back(new MCalibrationModelPoly4());
    Models.push_back(new MCalibrationModelPoly1Inv1());
    Models.push_back(new MCalibrationModelPoly1Exp1());
    Models.push_back(new MCalibrationModelPoly1Exp2());
    Models.push_back(new MCalibrationModelPoly1Exp3());
    
    vector<double> Results;
    for (unsigned int m = 0; m < Models.size(); ++m) {
      Results.push_back(Models[m]->Fit(Points));
      if (g_Verbosity >= c_Info) cout<<"Model "<<Models[m]->GetName()<<": "<<Results.back()<<endl;
    }
    
    vector<double>::iterator MinI;
    MinI = min_element(Results.begin(), Results.end());
    
    int Min = int(MinI -  Results.begin());
    if (g_Verbosity >= c_Info) cout<<"Best model: "<<Models[Min]->GetName()<<endl;
    
    m_Results.SetModel(*Models[Min]);
    
    for (unsigned int m = 0; m < Models.size(); ++m) {
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
