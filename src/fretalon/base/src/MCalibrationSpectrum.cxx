/*
 * MCalibrationSpectrum.cxx
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
#include "MCalibrationSpectrum.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"
#include "MCalibrateEnergyFindLines.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibrationSpectrum)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrationSpectrum::MCalibrationSpectrum()
{
  m_EnergyModel = nullptr;
  m_FWHMModel = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrationSpectrum::~MCalibrationSpectrum()
{
  //delete m_EnergyModel;
  //delete m_FWHMModel;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content
void MCalibrationSpectrum::Clear()
{
  m_SpectralPoints.clear();
  delete m_EnergyModel;
  delete m_FWHMModel;
  m_EnergyModel = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this calibration - the returned calibration must be deleted!
MCalibrationSpectrum* MCalibrationSpectrum::Clone() const
{
  MCalibrationSpectrum* C = new MCalibrationSpectrum();
  C->m_IsCalibrated = m_IsCalibrated;
  C->m_SpectralPoints = m_SpectralPoints;
  if (m_EnergyModel != nullptr) {
    C->SetEnergyModel(*m_EnergyModel);
  }
  if (m_FWHMModel != nullptr) {
    C->SetFWHMModel(*m_FWHMModel);
  }
  
  return C;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a spectral point to the read-out data group
void MCalibrationSpectrum::AddSpectralPoint(unsigned int ROG, const MCalibrationSpectralPoint& Point) 
{ 
  if (ROG < m_SpectralPoints.size()) {
    m_SpectralPoints[ROG].push_back(Point);
    return;
  }

  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), ROG);
}

  
////////////////////////////////////////////////////////////////////////////////


//! Get the number of spectral points in the roud-out data group
unsigned int MCalibrationSpectrum::GetNumberOfSpectralPoints(unsigned int ROG) const 
{ 
  if (ROG < m_SpectralPoints.size()) {
    return m_SpectralPoints[ROG].size(); 
  }
  
  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), ROG);
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Get a spectral point 
//! If it does not exist return an MExceptionIndexOutOfBounds exception
MCalibrationSpectralPoint& MCalibrationSpectrum::GetSpectralPoint(unsigned int ROG, unsigned int i)
{
  if (ROG < m_SpectralPoints.size()) {
    if (i < m_SpectralPoints[ROG].size()) {
      return m_SpectralPoints[ROG][i];
    }

    throw MExceptionIndexOutOfBounds(0, m_SpectralPoints[ROG].size(), i);
  
    // Might crash, but will never reach this point unless exceptions are turned off
    return m_SpectralPoints[0][0];
  }
  
  throw MExceptionIndexOutOfBounds(0, m_SpectralPoints.size(), i);
  
  // Might crash, but will never reach this point unless exceptions are turned off
  return m_SpectralPoints[0][0];
}


////////////////////////////////////////////////////////////////////////////////


//! Remove a point - the check if they are indentical is soly based on the peak position.
void MCalibrationSpectrum::RemoveAllBadSpectralPoints()
{
  for (unsigned int rogs = 0; rogs < m_SpectralPoints.size(); ++rogs) {
    for (auto Iter = m_SpectralPoints[rogs].begin(); Iter != m_SpectralPoints[rogs].end(); ) {
      if ((*Iter).IsGood() == false) {
        Iter = m_SpectralPoints[rogs].erase(Iter);
      } else {
        ++Iter; 
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return the calibration in a saveable format
//! Mode: pak  - points ADC to keV
//! Mode: pakw - points ADC to keV and fwhm
//! Mode: model - show the model (none, poly2, poly3, poly2gauss, etc.)
MString MCalibrationSpectrum::ToParsableString(const MString& Mode, bool WithDescriptor)
{
  if (Mode != "pak" && Mode != "pakw" && Mode != "energymodel" && Mode != "fwhmmodel" && Mode != "peakparametrization" && Mode != "peakparametrizationshort") {
    throw MExceptionUnknownMode(Mode);
    return "";
  }
  
  ostringstream out;
  
  // Make a single list of the points and store them for sorting
  vector<MCalibrationSpectralPoint> Points = GetUniquePoints();
   
  if (Mode == "pak") {
    if (WithDescriptor == true) {
      out<<"pak "; 
    }
    out<<Points.size()<<" ";

    for (unsigned int p = 0; p < Points.size(); ++p) {
      out<<Points[p].GetPeak()<<" "<<Points[p].GetEnergy()<<" "; 
    }
    out<<endl;
  } else if (Mode == "pakw") {
    if (WithDescriptor == true) {
      out<<"pakw "; 
    }
    out<<Points.size()<<" ";

    for (unsigned int p = 0; p < Points.size(); ++p) {
      out<<Points[p].GetPeak()<<" "<<Points[p].GetEnergy()<<" "<<Points[p].GetEnergyFWHM()<<"  ";
    }     
  } else if (Mode == "energymodel") {
    if (HasEnergyModel() == true) {
      out<<m_EnergyModel->ToParsableString(WithDescriptor); 
    } else {
      out<<"none"; 
    }
  } else if (Mode == "fwhmmodel") {
    if (HasFWHMModel() == true) {
      out<<m_FWHMModel->ToParsableString(WithDescriptor); 
    } else {
      out<<"none"; 
    }
  } else if (Mode == "peakparametrization") {
    out<<"Method:";
    if (Points.size() == 0) {
      out<<"Unknown";
    } else {
      if (Points[0].HasFit() == false) {
        out<<"BayesianBlock";
      } else {
        MCalibrationFit& Fit = Points[0].GetFit();
        out<<"FittedPeak";
        if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelNone) {
          out<<" BackgroundModel:None";
        } else if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelFlat) {
          out<<" BackgroundModel:Flat";
        } else if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelLinear) {
          out<<" BackgroundModel:Linear";
        } else {
          out<<" BackgroundModel:Unknown";
        }

        if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelNone) {
          out<<" EnergyLossModel:None";
        } else if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunction) {
          out<<" EnergyLossModel:GaussianConvolvedDeltaFunction";
        } else if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
          out<<" EnergyLossModel:GaussianConvolvedDeltaFunctionWithExponentialDecay";
        } else {
          out<<" EnergyLossModel:Unknown";
        }

        if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelNone) {
          out<<" PeakShapeModel:None";
        } else if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelGaussian) {
          out<<" PeakShapeModel:Gaussian";
        } else if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelGaussLandau) {
          out<<" PeakShapeModel:GaussLandau";
        } else {
          out<<" PeakShapeModel:Unknown";
        }
      }
    }
  } else if (Mode == "peakparametrizationshort") {
    out<<"M:";
    if (Points.size() == 0) {
      out<<"X";
    } else {
      if (Points[0].HasFit() == false) {
        out<<"BB";
      } else {
        MCalibrationFit& Fit = Points[0].GetFit();
        out<<"PF";
        if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelNone) {
          out<<"-B:X";
        } else if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelFlat) {
          out<<"-B:F";
        } else if (Fit.GetBackgroundModel() == MCalibrationFit::c_BackgroundModelLinear) {
          out<<"-B:L";
        } else {
          out<<"-B:?";
        }

        if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelNone) {
          out<<"-L:X";
        } else if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunction) {
          out<<"-L:GCDF";
        } else if (Fit.GetEnergyLossModel() == MCalibrationFit::c_EnergyLossModelGaussianConvolvedDeltaFunctionWithExponentialDecay) {
          out<<"-L:GCDFED";
        } else {
          out<<"-L:?";
        }

        if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelNone) {
          out<<"-P:N";
        } else if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelGaussian) {
          out<<"-P:G";
        } else if (Fit.GetPeakShapeModel() == MCalibrationFit::c_PeakShapeModelGaussLandau) {
          out<<"-P:GL";
        } else {
          out<<"-P:?";
        }
      }
    }
  }
  
  return out.str();
}  
  

////////////////////////////////////////////////////////////////////////////////


//! Get a list of all unique spectral points in this spectrum
vector<MCalibrationSpectralPoint> MCalibrationSpectrum::GetUniquePoints()
{
  // Make a single list of the points and store them for sorting
  vector<MCalibrationSpectralPoint> Points;
  for (unsigned int rog = 0; rog < m_SpectralPoints.size(); ++rog) {
    for (unsigned int p = 0; p < m_SpectralPoints[rog].size(); ++p) {
      if (GetSpectralPoint(rog, p).IsGood()) {
        Points.push_back(GetSpectralPoint(rog, p));
      }
    }
  }
  
  // Sort the points by energy:
  auto comp = [](MCalibrationSpectralPoint a,  MCalibrationSpectralPoint b) -> bool { return a.GetEnergy() < b.GetEnergy(); };
  sort(Points.begin(), Points.end(), comp);
        
  // If we have multiple points with the same energy -- warn for now because we first need to have such an example to test it...


  return Points;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the energy calibration model
void MCalibrationSpectrum::SetEnergyModel(MCalibrationModel& EnergyModel)
{
  delete m_EnergyModel;
  m_EnergyModel = EnergyModel.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Remove the energy calibration model
void MCalibrationSpectrum::RemoveEnergyModel()
{
  delete m_EnergyModel;
  m_EnergyModel = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
MCalibrationModel& MCalibrationSpectrum::GetEnergyModel() 
{ 
  if (m_EnergyModel == 0) {
    throw MExceptionObjectDoesNotExist("Model does not exist!");
  }
  
  return *m_EnergyModel; 
}


////////////////////////////////////////////////////////////////////////////////


//! Set the FWHM calibration model
void MCalibrationSpectrum::SetFWHMModel(MCalibrationModel& FWHMModel)
{
  delete m_FWHMModel;
  m_FWHMModel = FWHMModel.Clone();
}


////////////////////////////////////////////////////////////////////////////////


//! Remove the FWHM model
void MCalibrationSpectrum::RemoveFWHMModel()
{
  delete m_FWHMModel;
  m_FWHMModel = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the fit, if it doesn't exist throw MExceptionObjectDoesNotExist
//! Bad design since a new fit can be set any time...
MCalibrationModel& MCalibrationSpectrum::GetFWHMModel() 
{ 
  if (m_FWHMModel == nullptr) {
    throw MExceptionObjectDoesNotExist("Line-width model does not exist!");
  }
  
  return *m_FWHMModel; 
}


// MCalibrationSpectrum.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
