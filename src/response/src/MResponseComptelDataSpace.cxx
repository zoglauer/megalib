/*
 * MResponseComptelDataSpace.cxx
 *
 *
 * Copyright (C) by Clio Sleator & Andreas Zoglauer.
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
// MResponseComptelDataSpace
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseComptelDataSpace.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrixAxis.h"
#include "MResponseMatrixAxisSpheric.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MResponseComptelDataSpace)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseComptelDataSpace::MResponseComptelDataSpace()
{
  m_ResponseNameSuffix = "";
  m_OnlyINITRequired = true;
  
  m_CDS_ARM = 7.74; // deg
  m_EnergyNBins = 10;
  m_EnergyMinimum = 100; // keV
  m_EnergyMaximum = 3000; // keV
  m_BkgProbabilityFileName = "";
  m_UseBkgProbability = false;

}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseComptelDataSpace::~MResponseComptelDataSpace()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseComptelDataSpace::Description()
{
  return MString("Comptel data space response");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseComptelDataSpace::Options()
{
  ostringstream out;
  out<<"             cdsarm:           size of regions in Comptel data space (default: 7.74 deg)"<<endl;
  out<<"             emin:                    minimum energy (default: 100 keV)"<<endl;
  out<<"             emax:                    maximum energy (default: 3000 keV)"<<endl;
  out<<"             ebins:                   number of energy bins between min and max energy (default: 10)"<<endl;
  out<<"             bkgprobfilename:         background probability file name (default: \"\" (i.e. none))" << endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseComptelDataSpace::ParseOptions(const MString& Options)
{
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("=")); 
  }
  
  // Basic sanity check and to lower for all options
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i].size() == 0) {
      mout<<"Error: Empty option in string "<<Options<<endl;
      return false;
    }    
    if (Split2[i].size() == 1) {
      mout<<"Error: Option has no value: "<<Split2[i][0]<<endl;
      return false;
    }
    if (Split2[i].size() > 2) {
      mout<<"Error: Option has more than one value or you used the wrong separator (not \":\"): "<<Split1[i]<<endl;
      return false;
    }
    Split2[i][0].ToLowerInPlace();
  }
  
  // Parse
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    string Value = Split2[i][1].Data();
    
    if (Split2[i][0] == "emin") {
      m_EnergyMinimum = stod(Value);
    } else if (Split2[i][0] == "emax") {
      m_EnergyMaximum = stod(Value);
    } else if (Split2[i][0] == "ebins") {
      m_EnergyNBins = stod(Value);
    } else if (Split2[i][0] == "cdsarm") {
      m_CDS_ARM = stod(Value);
    } else if (Split2[i][0] == "bkgprobfilename") {
      m_BkgProbabilityFileName = Value;
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (m_EnergyMinimum <= 0 || m_EnergyMaximum <= 0) {
    mout<<"Error: All energy values must be positive (larger than zero)"<<endl;
    return false;    
  }
  if (m_EnergyMinimum >= m_EnergyMaximum) {
    mout<<"Error: The minimum energy must be smaller than the maximum energy"<<endl;
    return false;       
  }
  if (m_EnergyNBins <= 0) {
    mout<<"Error: You need at least one energy bin"<<endl;
    return false;       
  }
  if (m_CDS_ARM <= 0) {
    mout<<"Error: You need at give a positive size of the regions in the data space"<<endl;
    return false;       
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for binned imaging response:"<<endl;
  mout<<"  Minimum energy:                                     "<<m_EnergyMinimum<<endl;
  mout<<"  Maximum energy:                                     "<<m_EnergyMaximum<<endl;
  mout<<"  Number of bins energy:                              "<<m_EnergyNBins<<endl;
  mout<<"  Size of Comptel data space regions:                 "<<m_CDS_ARM<<endl;
  mout<<"  Background probability file name:                   "<<m_BkgProbabilityFileName;
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseComptelDataSpace::Initialize()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  MResponseMatrixAxis AxisElevation("Elevation [deg]");
  AxisElevation.SetLinear(60, 0, 60);
 
  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  AxisEnergyMeasured.SetLogarithmic(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);

  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(65, 0, 65);
  
  MResponseMatrixAxis AxisSourceOrBackground("Src (1) or Bkg (0)");
  AxisSourceOrBackground.SetLinear(2, 0, 2);

  m_CDSResponse.SetName("Comptel data space response");
  m_CDSResponse.AddAxis(AxisElevation);
  m_CDSResponse.AddAxis(AxisEnergyMeasured);
  m_CDSResponse.AddAxis(AxisPhi);
  m_CDSResponse.AddAxis(AxisSourceOrBackground);
  if (m_SiReader != nullptr) {
    m_CDSResponse.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   

  if (m_BkgProbabilityFileName != "") {
    if (MFile::Exists(m_BkgProbabilityFileName) == false) {
      mout<<"Error: The file: \""<<m_BkgProbabilityFileName<<"\" does not exist"<<endl;
      return false;
    }
    else {
      m_UseBkgProbability = true;
      if (!m_LingProbability.Read(m_BkgProbabilityFileName)) {
        mout<<"failed to read \"" << m_BkgProbabilityFileName<<"\", aborting..."<<endl;
        return false;
      }
    }
  }

  InitializeSourcePositions();

  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseComptelDataSpace::Analyze() 
{ 
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require a successful reconstruction 
  MRawEventList* REList = m_ReReader->GetRawEventList();
  if (REList->HasOptimumEvent() == false) {
    return true;
  }
    
  // ... leading to an event
  MPhysicalEvent* Event = REList->GetOptimumEvent()->GetPhysicalEvent();
  if (Event == nullptr) {
    return true;
  }
  
  // ... which needs to be a Compton event
  if (Event->GetType() != MPhysicalEvent::c_Compton) {
    return true;
  }
  
  // ... that passed the event selections
  if (m_MimrecEventSelector.IsQualifiedEvent(Event) == false) {
    return true; 
  }
  
  // ... and has a good kinematics
  MComptonEvent* Compton = (MComptonEvent*) Event;
  if (Compton->IsKinematicsOK() == false) {
    return true;
  }

  double Energy = Compton->GetEnergy();
  double Phi = Compton->Phi()*c_Deg;

  double OriginEnergy = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();

  //loop over all elevation angle bins
  for (int i=0; i<60; i++) {

    double offAxisAngle = i;

    if (m_UseBkgProbability == true) {
      double P = m_LingProbability.Get(vector<double> {offAxisAngle, OriginEnergy});
      if (gRandom->Rndm() > P) {
        continue;
      }
    }

    MVector Dg = -Compton->Dg();
    Dg = m_RTo00[i]*Dg;
    Dg = m_R2.Invert()*Dg;

    double Chi = Dg.Theta()*c_Deg;

    double ARM = Chi-Phi;

    //source region
    if ( fabs(ARM) <= m_CDS_ARM ) {
      m_CDSResponse.Add(vector<double> {offAxisAngle, Energy, Phi, 1});
    }
    else if ( fabs(ARM) > m_CDS_ARM && fabs(ARM) <= 2*m_CDS_ARM ) {
      m_CDSResponse.Add(vector<double> {offAxisAngle, Energy, Phi, 0});
    }
  }

   
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseComptelDataSpace::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseComptelDataSpace::Save()
{
  MResponseBuilder::Save(); 
  
  m_CDSResponse.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_CDSResponse.Write(GetFilePrefix() + ".compteldataspace" + m_Suffix, true);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

//! Initialize MRotations to rotate source position to (0,0) for all detector coordinates
bool MResponseComptelDataSpace::InitializeSourcePositions()
{

  for (int i=0; i<60; i++) {

    double t = i*c_Rad;
    double p = 0*c_Rad;

    double alpha = acos(sin(t)*cos(p));
  
    //then fill rotation matrix
    double norm = sqrt(pow(RoundF(cos(t), 10), 2)+pow(RoundF(sin(t), 10), 2)*pow(RoundF(sin(p), 10), 2));
    if (norm == 0) { norm = 1e-17; }
    MRotation K;
    K.SetXX(0);
    K.SetXY(-RoundF(sin(t), 10)*RoundF(sin(p), 10)/norm);
    K.SetXZ(-RoundF(cos(t), 10)/norm);
  
    K.SetYX(RoundF(sin(t), 10)*RoundF(sin(p), 10)/norm);
    K.SetYY(0);
    K.SetYZ(0);
  
    K.SetZX(RoundF(cos(t), 10)/norm);
    K.SetZY(0);
    K.SetZZ(0);
  
  
    MRotation I;
    I.SetIdentity();
  
    MRotation K1 = K;
    K1 *= sin(alpha);
  
    MRotation K2 = K;
    K2 *= K;
    K2 *= (1-cos(alpha));
    
    MRotation RTo00;
    RTo00.SetXX(I.GetXX()+K1.GetXX()+K2.GetXX());
    RTo00.SetXY(I.GetXY()+K1.GetXY()+K2.GetXY());
    RTo00.SetXZ(I.GetXZ()+K1.GetXZ()+K2.GetXZ());
  
    RTo00.SetYX(I.GetYX()+K1.GetYX()+K2.GetYX());
    RTo00.SetYY(I.GetYY()+K1.GetYY()+K2.GetYY());
    RTo00.SetYZ(I.GetYZ()+K1.GetYZ()+K2.GetYZ());
  
    RTo00.SetZX(I.GetZX()+K1.GetZX()+K2.GetZX());
    RTo00.SetZY(I.GetZY()+K1.GetZY()+K2.GetZY());
    RTo00.SetZZ(I.GetZZ()+K1.GetZZ()+K2.GetZZ());

    m_RTo00.push_back(RTo00);
  
  }

  //make this rotation: rotate cone into plane
  MVector xAxis(0, 0, 1);
  MVector zAxis(1, 0, 0);
  MVector yAxis = zAxis.Cross(xAxis);

  m_R2.SetXX(xAxis.X());
  m_R2.SetXY(xAxis.Y());
  m_R2.SetXZ(xAxis.Z());
  m_R2.SetYX(yAxis.X());
  m_R2.SetYY(yAxis.Y());
  m_R2.SetYZ(yAxis.Z());
  m_R2.SetZX(zAxis.X());
  m_R2.SetZY(zAxis.Y());
  m_R2.SetZZ(zAxis.Z());
  
  return true;

}


////////////////////////////////////////////////////////////////////////////////

//! Round double to precision P
double MResponseComptelDataSpace::RoundF(double N, int P) {

  double P10 = pow(10, P);

  N = N*P10;
  if (N>=0) { N += 0.5; }
  else { N -= 0.5; }

  double N2 = floor(N)/P10;

  return N2;

}

////////////////////////////////////////////////////////////////////////////////


// MResponseComptelDataSpace.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
