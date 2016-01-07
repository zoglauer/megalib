/*
 * MCalibrationModel.cxx
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
#include "MCalibrationModel.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"
#include "TH1.h"
#include "TF1.h"
#include "TGraph.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrationModel)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationModel::MCalibrationModel() : m_Fit(0), m_IsFitUpToDate(false), m_Keyword("none"),   m_FitQuality(numeric_limits<double>::max()/1000)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MCalibrationModel::MCalibrationModel(const MCalibrationModel& Fit)
{
  // We don't care about double initilization, so:
  m_Fit = 0;
  *this = Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationModel::~MCalibrationModel()
{
  delete m_Fit;
}


////////////////////////////////////////////////////////////////////////////////


//! The assignment operator
MCalibrationModel& MCalibrationModel::operator= (const MCalibrationModel& CalibrationModel)
{
  delete m_Fit;
  if (CalibrationModel.m_Fit != 0) {
    m_Fit = new TF1(*CalibrationModel.m_Fit);
    for (int i = 0; i < CalibrationModel.m_Fit->GetNpar(); ++i) {
      m_Fit->SetParameter(i, CalibrationModel.m_Fit->GetParameter(i));
    }
  } else {
    m_Fit = 0;
  }
    
  m_IsFitUpToDate = CalibrationModel.m_IsFitUpToDate;
  m_Keyword = CalibrationModel.m_Keyword;
  m_FitQuality = CalibrationModel.m_FitQuality;

  m_ROOTParameters = CalibrationModel.m_ROOTParameters;
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this fit - the returned element must be deleted!
MCalibrationModel* MCalibrationModel::Clone() const
{
  return new MCalibrationModel(*this);
}


////////////////////////////////////////////////////////////////////////////////


//! Mimic ROOT Draw functionality
void MCalibrationModel::Draw(MString Options)
{
  if (m_Fit != 0 && m_IsFitUpToDate == true) {
    m_Fit->DrawCopy(Options);
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error: Fit cannot be drawn since it is not up to date"<<endl; 
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the given histogram in the given range - return the quality of the fit
double MCalibrationModel::Fit(const vector<MCalibrationSpectralPoint> Points)
{
  if (Points.size() < NPar()) {
    if (g_Verbosity >= c_Error) cout<<"Error: We have more fit parameters ("<<NPar()<<") than data points ("<<Points.size()<<")!"<<endl;
    return -1;
  }
  
  m_FitQuality = numeric_limits<double>::max()/1000;
  
  // Clean up the old fit:
  TThread::Lock();
  delete m_Fit;
  TThread::UnLock();
  m_Fit = 0;
  m_IsFitUpToDate = false;
  
  // Prepare the array for ROOT
  m_ROOTParameters.resize(NPar(), 0);
   
  // SEt up the fitter
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);

  ROOT::Fit::BinData TheData;
  TheData.Initialize(Points.size(), 1, ROOT::Fit::BinData::kValueError);
  for (unsigned int p = 0; p < Points.size(); ++p) {
    TheData.Add(Points[p].GetPeak(), Points[p].GetEnergy(), Points[p].GetEnergyFWHM()/2.35);
  }

  ROOT::Fit::Fitter TheFitter; 
  TheFitter.Config().SetMinimizer("Minuit2");
  if (g_Verbosity >= c_Info) TheFitter.Config().MinimizerOptions().SetPrintLevel(1);
  
  TheFitter.SetFunction(*this);
  InitializeFitParameters(TheFitter);
  
  // Fit
  bool ReturnCode = TheFitter.Fit(TheData);
  if (ReturnCode == true) {
    if (TheFitter.CalculateHessErrors() == false) {
      if (TheFitter.CalculateMinosErrors() == false) {
        cout<<"Unable to calculate either Minos or Hess error!"<<endl;
        ReturnCode = false;
      }
    }
  }
  
  // Prepare the results
  const ROOT::Fit::FitResult& TheFitResult = TheFitter.Result(); 
  if (TheFitResult.IsEmpty()) ReturnCode = false;  
  
  if (ReturnCode == true) {
  
    m_ROOTParameters = TheFitResult.Parameters();
    
    m_IsFitUpToDate = true;
        
    // Create a TF1 object for drawing
    TThread::Lock();
    m_Fit = new TF1("", this, 0, 1.1*Points.back().GetPeak(), NPar());
    TThread::UnLock();
    
    m_Fit->SetChisquare(TheFitResult.Chi2());
    m_Fit->SetNDF(TheFitResult.Ndf());
    m_Fit->SetNumberFitPoints(TheData.Size());

    m_Fit->SetParameters( &(TheFitResult.Parameters().front()) ); 
    if (int(TheFitResult.Errors().size()) >= m_Fit->GetNpar()) { 
      m_Fit->SetParErrors( &(TheFitResult.Errors().front()) );
    }

    m_FitQuality = m_Fit->GetChisquare()/m_Fit->GetNDF(); // Used to determine the quality of fit, thus critically important    
  }  
  
  return m_FitQuality;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit value - return 0 if the fit does not exist
double MCalibrationModel::GetFitValue(double Value) const
{
  if (m_Fit != 0 && m_IsFitUpToDate == true) {
    return m_Fit->Eval(Value);
  }
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Return a parsable string
MString MCalibrationModel::ToParsableString(bool WithDescriptor) 
{
  if (m_Fit == 0 || m_IsFitUpToDate == false) return "none";

  ostringstream out;
  if (WithDescriptor == true) out<<m_Keyword<<" ";
  for (int i = 0; i < m_Fit->GetNpar(); ++i) {
    out<<m_Fit->GetParameter(i)<<" ";
  }
  return out.str();
}


// MCalibrationModel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
