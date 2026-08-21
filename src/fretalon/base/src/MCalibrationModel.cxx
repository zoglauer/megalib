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
#include <cmath>
using namespace std;

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


#ifdef ___CLING___
ClassImp(MCalibrationModel)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationModel::MCalibrationModel() : m_Type(MCalibrationModelType::c_Energy), m_Fit(nullptr), m_IsFitUpToDate(false), m_Keyword("none"), m_FitQuality(numeric_limits<double>::max()/1000)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Copy constructor
MCalibrationModel::MCalibrationModel(const MCalibrationModel& Fit)
{
  // We don't care about double initilization, so:
  m_Fit = nullptr;
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
  m_Type = CalibrationModel.m_Type;
  
  delete m_Fit;
  if (CalibrationModel.m_Fit != nullptr) {
    m_Fit = new TF1(*CalibrationModel.m_Fit);
    for (int i = 0; i < CalibrationModel.m_Fit->GetNpar(); ++i) {
      m_Fit->SetParameter(i, CalibrationModel.m_Fit->GetParameter(i));
    }
  } else {
    m_Fit = nullptr;
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
   
  // Set up the fitter
  ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(20000);

  // Locally store the data - we need it later
  vector<double> X;
  vector<double> Y;
  vector<double> dY;
  for (unsigned int p = 0; p < Points.size(); ++p) {
    double FWHM = Points[p].GetEnergyFWHM();
    // If we do not yet have a FWHM use a dummy linear calibration through zero
    if (FWHM < 1E-10) {
      FWHM = Points[p].GetFWHM() * Points[p].GetEnergy() / Points[p].GetPeak();
    }
    if (m_Type == MCalibrationModelType::c_LineWidth) {
      if (fabs(Points[p].GetEnergy() - 511) < 0.1) continue; // Always exclude any 511 line, since it is larger than usual (positron range)
      X.push_back(Points[p].GetEnergy());
      Y.push_back(FWHM);
      dY.push_back(0.1*FWHM); // Arbitrary width...
    } else {
      X.push_back(Points[p].GetPeak());
      Y.push_back(Points[p].GetEnergy());
      dY.push_back(FWHM/2.35);
    }
  }

  // Fill the fitting data set
  ROOT::Fit::BinData TheData;
  TheData.Initialize(Points.size(), 1, ROOT::Fit::BinData::kValueError);
  for (unsigned int d = 0; d < X.size(); ++d) {
    TheData.Add(X[d], Y[d], dY[d]);
  }


  ROOT::Fit::Fitter TheFitter; 
  TheFitter.Config().SetMinimizer("Minuit2", "Migrad");
  if (g_Verbosity >= c_Info) TheFitter.Config().MinimizerOptions().SetPrintLevel(2);
  
  TheFitter.SetFunction(*this);
  InitializeFitParameters(TheFitter);
  
  // Fit
  if (g_Verbosity >= c_Info) cout<<endl<<"Round 1 of line fitting started"<<endl;
  bool ReturnCode = TheFitter.Fit(TheData);
  ROOT::Fit::FitResult& TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter.Result());

  if (std::isnan(TheFitResult.Edm()) == true) {
    ReturnCode = false; 
  }
  
  if (ReturnCode == true) {
    if (TheFitter.CalculateHessErrors() == false) {
      if (TheFitter.CalculateMinosErrors() == false) {
        cout<<"Unable to calculate either Minos or Hess error!"<<endl;
        ReturnCode = false;
      }
    }
    if (ReturnCode == true) {
      if (g_Verbosity >= c_Info) cout<<"Successfully fit line model in round 1."<<endl;
      TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter.Result()); 
    }
  } else {
    if (g_Verbosity >= c_Info) cout<<"Unable to fit calibration model in round 1."<<endl;
  }

  // Round 2
  if (ReturnCode == false) {
    // Try a different fitting approach 
    
    if (g_Verbosity >= c_Info) cout<<endl<<"Round 2 of line fitting started"<<endl;
    ROOT::Fit::Fitter TheFitter2; 
    TheFitter2.Config().SetMinimizer("Minuit2", "Minimize");
    if (g_Verbosity >= c_Info) TheFitter2.Config().MinimizerOptions().SetPrintLevel(2);
    
    TheFitter2.SetFunction(*this);
    InitializeFitParameters(TheFitter2);

    ReturnCode = TheFitter2.LikelihoodFit(TheData);
    TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter2.Result());
    
    if (std::isnan(TheFitResult.Edm()) == true) {
      ReturnCode = false; 
    }
    
    if (ReturnCode == true) {
      if (TheFitter2.CalculateHessErrors() == false) {
        if (TheFitter2.CalculateMinosErrors() == false) {
          cout<<"Unable to calculate either Minos or Hess error!"<<endl;
          ReturnCode = false;
        }
      }
      if (ReturnCode == true) {
        TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter2.Result()); 
        if (g_Verbosity >= c_Info) cout<<"Successfully fit line model in round 2 (chisquare: "<<TheFitResult.Chi2()<<")"<<endl;
      }
    } else {
      if (g_Verbosity >= c_Info) cout<<"Unable to fit calibration model in round 2."<<endl;
      //return false;
    }
  }
  

  // Round 3
  if (ReturnCode == false) {
    // Try a different fitting approach 
    
    if (g_Verbosity >= c_Info) cout<<endl<<"Round 3 of line fitting started"<<endl;
    ROOT::Fit::Fitter TheFitter3; 
    TheFitter3.Config().SetMinimizer("Fumili");
    if (g_Verbosity >= c_Info) TheFitter3.Config().MinimizerOptions().SetPrintLevel(2);
    
    TheFitter3.SetFunction(*this);
    InitializeFitParameters(TheFitter3);

    ReturnCode = TheFitter3.LikelihoodFit(TheData);
    TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter3.Result());
    
    if (std::isnan(TheFitResult.Edm()) == true) {
      ReturnCode = false; 
    }
    
    if (ReturnCode == true) {
      if (TheFitter3.CalculateHessErrors() == false) {
        if (TheFitter3.CalculateMinosErrors() == false) {
          cout<<"Unable to calculate either Minos or Hess error!"<<endl;
          ReturnCode = false;
        }
      }
      if (ReturnCode == true) {
        TheFitResult = const_cast<ROOT::Fit::FitResult&>(TheFitter3.Result()); 
        if (g_Verbosity >= c_Info) cout<<"Successfully fit line model in round 3."<<endl;
      }
    } else {
      if (g_Verbosity >= c_Info) cout<<"Unable to fit calibration model in round 3."<<endl;
      //return false;
    }
  }
  
  if (TheFitResult.IsEmpty()) ReturnCode = false;  
  
  if (ReturnCode == true) {
  
    m_ROOTParameters = TheFitResult.Parameters();
    
    m_IsFitUpToDate = true;
        
    // Create a TF1 object for drawing
    TThread::Lock();
    m_Fit = new TF1("", this, 0, 1.1*Points.back().GetPeak(), NPar());
    TThread::UnLock();
    
    m_Fit->SetNumberFitPoints(TheData.Size());
    m_Fit->SetParameters( &(TheFitResult.Parameters().front()) ); 
    if (int(TheFitResult.Errors().size()) >= m_Fit->GetNpar()) { 
      m_Fit->SetParErrors( &(TheFitResult.Errors().front()) );
    }

    // Let's do our own chi-square calculation, since it needs to be uniform over all fit approaches and independent of fit errors
    double Chisquare = 0;
    for (unsigned int d = 0; d < X.size(); ++d) {
      Chisquare += (m_Fit->Eval(X[d]) - Y[d])*(m_Fit->Eval(X[d]) - Y[d]) / (dY[d]*dY[d]);
    }
    m_Fit->SetChisquare(Chisquare);
    m_Fit->SetNDF(TheFitResult.Ndf());


    if (m_Fit->GetNDF() > 0) {
      m_FitQuality = m_Fit->GetChisquare()/m_Fit->GetNDF(); // Used to determine the quality of fit, thus critically important    
      //if (g_Verbosity >= c_Info) cout<<"Fit quality: "<<m_FitQuality<<" (chi-square: "<<m_Fit->GetChisquare()<<", NDF: "<<m_Fit->GetNDF()<<")"<<endl;
    } else {
      m_FitQuality = 0; 
    }
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
  if (WithDescriptor == true) out<<"error ";
  for (int i = 0; i < m_Fit->GetNpar(); ++i) {
    out<<m_Fit->GetParError(i)<<" ";
  }
  return out.str();
}


// MCalibrationModel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
