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
MCalibrationModel::MCalibrationModel() : m_Fit(0), m_IsFitUpToDate(false), m_Keyword("none")
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
  if (Points.size() < GetNParameters()) {
    if (g_Verbosity >= c_Error) cout<<"Error: We have more fit parameters ("<<GetNParameters()<<") than data points ("<<Points.size()<<")!"<<endl;
    return -1;
  }
  
  // Create a TGraph and fit it
  TGraph* Graph = new TGraph(Points.size());
  
  for (unsigned int p = 0; p < Points.size(); ++p) {
    Graph->SetPoint(p, Points[p].GetPeak(), Points[p].GetEnergy());
  }
  
  delete m_Fit;
  m_Fit = new TF1("", this, 0, Points.back().GetPeak(), GetNParameters());
  InitializeFitParameters();
  
  MString Options = "RNI S"; 
  if (g_Verbosity < c_Chatty) Options += " Q";
  TFitResultPtr FitResult = Graph->Fit(m_Fit, Options);
  
  m_IsFitUpToDate = true;
  
  return m_Fit->GetChisquare()/m_Fit->GetNDF(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit value - if the fit doesn't exist throw MExceptionObjectDoesNotExist
double MCalibrationModel::GetFitValue(double Value) const
{
  if (m_Fit != 0 && m_IsFitUpToDate == true) {
    return m_Fit->Eval(Value);
  }
  
  throw MExceptionObjectDoesNotExist("Fit does not exist!");
  
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
