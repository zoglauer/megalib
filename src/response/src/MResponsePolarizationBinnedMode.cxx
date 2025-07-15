/*
 * MResponsePolarizationBinnedMode.cxx
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
// MResponsePolarizationBinnedMode
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponsePolarizationBinnedMode.h"

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


#ifdef ___CLING___
ClassImp(MResponsePolarizationBinnedMode)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponsePolarizationBinnedMode::MResponsePolarizationBinnedMode() : m_PolarizationResponse(true)
{
  m_ResponseNameSuffix = "binnedpolarization";
  m_OnlyINITRequired = true;
  
  m_AngleBinWidth = 5; // deg
  m_AngleBinWidthElectron = 360; // deg
  m_EnergyNBins = 1;
  m_EnergyMinimum = 10; // keV
  m_EnergyMaximum = 2000; // keV
  m_EnergyBinEdges.clear();
  m_DistanceNBins = 1;
  m_DistanceMinimum = 0; // cm
  m_DistanceMaximum = 1000; // cm
  m_PolarizationAngleNBins = 19; //deg
  
  m_UseAtmosphericAbsorption = false;
  m_AtmosphericAbsorptionFileName = "";
  m_Altitude = 33500;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponsePolarizationBinnedMode::~MResponsePolarizationBinnedMode()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponsePolarizationBinnedMode::Description()
{
  return MString("Binned polarization response");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponsePolarizationBinnedMode::Options()
{
  ostringstream out;
  out<<"             anglebinwidth:           the width of a sky bin at the equator (default: 5 deg)"<<endl;
  out<<"             emin:                    minimum energy (default: 10 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             emax:                    maximum energy (default: 2,000 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebins:                   number of energy bins between min and max energy (default: 1; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebinedges:               the energy bin edges as a comma seperated list (default: not used, cannot be used in combination with emin, emax, or ebins)"<<endl;
  out<<"             anglebinwidthelectron:   the width of a aky bin at the equator (default: 5 deg)"<<endl;
  out<<"             dmin:                    minimum distance (default: 0 cm)"<<endl;
  out<<"             dmax:                    maximum distance (default: 1,000 cm)"<<endl;
  out<<"             dbins:                   number of distance bins between min and max distance (default: 1)"<<endl;
  out<<"             pbins:                   number of polarization bins bins between min and max distance (default: 18)"<<endl;
  out<<"             atabsfile:               the atmopheric absorption file name (default: \"\" (i.e. none))"<<endl;
  out<<"             atabsheight:             altitude for the atmospheric absorption (default: 33500)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponsePolarizationBinnedMode::ParseOptions(const MString& Options)
{
  // Split the different options
  vector<MString> Split1 = Options.Tokenize(":");
  // Split Option <-> Value
  vector<vector<MString>> Split2;
  for (MString S: Split1) {
    Split2.push_back(S.Tokenize("=")); 
  }
  
  // Basic sanity check and "to-lower" for all options
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
  
  // Energy option checks
  bool HasBinEdges = false;
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i][0] == "ebinedges") {
      HasBinEdges = true;
    }
  }
  if (HasBinEdges == true) {
    for (unsigned int i = 0; i < Split2.size(); ++i) {
      if (Split2[i][0] == "emin") {
        mout<<"Error: You can not have ebinedges and emin at the same time as options"<<endl;
        return false;
      } else if (Split2[i][0] == "emax") {
        mout<<"Error: You can not have ebinedges and emax at the same time as options"<<endl;
        return false;
      } else if (Split2[i][0] == "ebins") {
        mout<<"Error: You can not have ebinedges and ebins at the same time as options"<<endl;
        return false;
      }
    }
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
    } else if (Split2[i][0] == "ebinedges") {
      vector<MString> Edges = MString(Value).Tokenize(",");
      m_EnergyBinEdges.clear();
      for (MString S: Edges) m_EnergyBinEdges.push_back(S.ToDouble());
      m_EnergyNBins = 0;
    } else if (Split2[i][0] == "anglebinwidth") {
      m_AngleBinWidth = stod(Value);
    } else if (Split2[i][0] == "dmin") {
      m_DistanceMinimum = stod(Value);
    } else if (Split2[i][0] == "dmax") {
      m_DistanceMaximum = stod(Value);
    } else if (Split2[i][0] == "dbins") {
      m_DistanceNBins = stod(Value);
    } else if (Split2[i][0] == "pbins") {
      m_PolarizationAngleNBins = stod(Value);
    } else if (Split2[i][0] == "anglebinwidthelectron") {
      m_AngleBinWidthElectron = stod(Value);
    } else if (Split2[i][0] == "atabsfilename") {
      m_AtmosphericAbsorptionFileName = Value;
    } else if (Split2[i][0] == "atabsaltitude") {
      m_Altitude = stod(Value);
    } else {
      mout<<"Error: Unrecognized option "<<Split2[i][0]<<endl;
      return false;
    }
  }
  
  // Sanity checks:
  if (HasBinEdges == true) {
    if (m_EnergyBinEdges.size() < 2) {
      mout<<"Error: You need at least 2 bin edges"<<endl;
      return false;    
    }
    if (m_EnergyBinEdges[0] < 0) {
      mout<<"Error: The minimum bin edge cannot be samller than 0"<<endl;
      return false;    
    }
    for (unsigned int e = 1; e < m_EnergyBinEdges.size(); ++e) {
      if (m_EnergyBinEdges[e] <= m_EnergyBinEdges[e-1]) {
        mout<<"Error: The bin edges must be in increasing order"<<endl;
        return false;    
      }
    }
  } else {
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
  }
  if (m_DistanceMinimum < 0 || m_DistanceMaximum < 0) {
    mout<<"Error: All distance values must be non-negative"<<endl;
    return false;    
  }
  if (m_DistanceMinimum >= m_DistanceMaximum) {
    mout<<"Error: The minimum distance must be smaller than the maximum distance"<<endl;
    return false;       
  }
  if (m_DistanceNBins <= 0) {
    mout<<"Error: You need at least one distance bin"<<endl;
    return false;       
  }
  if (m_PolarizationAngleNBins <= 0) {
    mout<<"Error: You need at least one polarizaton angle bin"<<endl;
    return false;       
  }
  if (m_AngleBinWidth <= 0) {
    mout<<"Error: You need at give a positive width of the sky bins at the equator"<<endl;
    return false;       
  }
  if (m_AngleBinWidthElectron <= 0) {
    mout<<"Error: You need at give a positive width of the sky bins at the equator for the recoil electron"<<endl;
    return false;       
  }
  if (m_AtmosphericAbsorptionFileName != "") {
    if (MFile::Exists(m_AtmosphericAbsorptionFileName) == false) {
      mout<<"Error: The file: \""<<m_AtmosphericAbsorptionFileName<<"\" does not exist"<<endl;
      return false;
    } else {
      m_UseAtmosphericAbsorption = true; 
    }
  }
  if (m_Altitude <= 0) {
    mout<<"Error: The altitiude must be positive"<<endl;
    return false;       
  }
  
  // Dump it for user info
  mout<<endl;
  mout<<"Choosen options for binned imaging response:"<<endl;
  if (HasBinEdges == true) {
    mout<<"  Bin edges energy:                                   ";
    for (double E: m_EnergyBinEdges) mout<<E<<" ";
    mout<<endl;
  } else {
    mout<<"  Minimum energy:                                     "<<m_EnergyMinimum<<endl;
    mout<<"  Maximum energy:                                     "<<m_EnergyMaximum<<endl;
    mout<<"  Number of bins energy:                              "<<m_EnergyNBins<<endl;
  }
  mout<<"  Width of sky bins at equator:                       "<<m_AngleBinWidth<<endl;
  mout<<"  Minimum distance:                                   "<<m_DistanceMinimum<<endl;
  mout<<"  Maximum distance:                                   "<<m_DistanceMaximum<<endl;
  mout<<"  Number of bins distance:                            "<<m_DistanceNBins<<endl;
  mout<<"  Number of bins polarization angle:                  "<<m_PolarizationAngleNBins<<endl;
  mout<<"  Width of sky bins at equator for recoild electron:  "<<m_AngleBinWidthElectron<<endl;
  if (m_UseAtmosphericAbsorption == true) {
    mout<<"  Atmospheric absorption file name:                   "<<m_AtmosphericAbsorptionFileName<<endl;
    mout<<"  Atmospheric absorption altitude:                    "<<m_Altitude<<endl;
  }
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponsePolarizationBinnedMode::Initialize()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  int AngleBins = 4*c_Pi*c_Deg*c_Deg / m_AngleBinWidth / m_AngleBinWidth;
  if (AngleBins < 1) AngleBins = 1;
  int AngleBinsElectron = 4*c_Pi*c_Deg*c_Deg / m_AngleBinWidthElectron / m_AngleBinWidthElectron;
  if (AngleBinsElectron < 1) AngleBinsElectron = 1;
  
  MResponseMatrixAxis AxisEnergyInitial("Initial energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyInitial.SetBinEdges(m_EnergyBinEdges);
  } else {
    AxisEnergyInitial.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
  }
  
  MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");
  AxisSkyCoordinates.SetFISBEL(AngleBins);

  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyMeasured.SetBinEdges(m_EnergyBinEdges);
  } else {
    AxisEnergyMeasured.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
  }
  
  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(180/m_AngleBinWidth, 0, 180);
  
  MResponseMatrixAxisSpheric AxisScatteredGammaRayCoordinates("#psi [deg]", "#chi [deg]");
  AxisScatteredGammaRayCoordinates.SetFISBEL(AngleBins);
  
  MResponseMatrixAxisSpheric AxisRecoilElectronCoordinates("#sigma [deg]", "#tau [deg]");
  AxisRecoilElectronCoordinates.SetFISBEL(AngleBinsElectron);
  
  MResponseMatrixAxis AxisDistance("Distance [cm]");
  AxisDistance.SetLinear(m_DistanceNBins, m_DistanceMinimum, m_DistanceMaximum);
  
  MResponseMatrixAxis AxisPolarizationAngle("Polarization Angle [deg]");
  AxisPolarizationAngle.SetLinear(m_PolarizationAngleNBins, 0, 180);
  
  
  
  m_PolarizationResponse.SetName("11D polarization response");
  m_PolarizationResponse.AddAxis(AxisEnergyInitial);
  m_PolarizationResponse.AddAxis(AxisSkyCoordinates);
  m_PolarizationResponse.AddAxis(AxisPolarizationAngle);
  m_PolarizationResponse.AddAxis(AxisEnergyMeasured);
  m_PolarizationResponse.AddAxis(AxisPhi);
  m_PolarizationResponse.AddAxis(AxisScatteredGammaRayCoordinates);
  m_PolarizationResponse.AddAxis(AxisRecoilElectronCoordinates);
  m_PolarizationResponse.AddAxis(AxisDistance);
  if (m_SiReader != nullptr) {
    m_PolarizationResponse.SetFarFieldStartArea(m_SiReader->GetSimulationStartAreaFarField());
  }   


  if (m_UseAtmosphericAbsorption == true) {
    if (m_AtmosphericAbsorption.Read(m_AtmosphericAbsorptionFileName) == false) {
      mout<<"Error: Unable to read atmospheric absorption"<<endl;
      return false;       
    }
  }

  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponsePolarizationBinnedMode::Analyze() 
{ 
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  // We need to have at least an "INIT" in the simulation file per event 
  if (m_SiEvent->GetNIAs() == 0) {
    return true;
  }
  
  // We require a successful reconstruction 
  MRawEventIncarnationList* REList = m_ReReader->GetRawEventList();
  if (REList->HasOnlyOptimumEvents() == false) {
    return true;
  }
    
  // ... leading to an event
  MPhysicalEvent* Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
  if (Event == nullptr) {
    return true;
  }
  
  // ... which needs to be a Compton event
  if (Event->GetType() != c_ComptonEvent) {
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
  
  if (m_UseAtmosphericAbsorption == true) {
    // Deselect gamma rays based on the transmission probability 
    
    double OriginAzimuth = (-m_SiEvent->GetIAAt(0)->GetSecondaryDirection()).Theta() * c_Deg;
    double OriginEnergy = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
    
    double P = m_AtmosphericAbsorption.GetTransmissionProbability(m_Altitude, OriginAzimuth, OriginEnergy);
   
    if (gRandom->Rndm() > P) {
      return true;
    }
  }
  
  // Get the data space information
  MRotation Rotation = Compton->GetDetectorRotationMatrix();

  double Phi = Compton->Phi()*c_Deg;
  MVector Dg = -Compton->Dg();
  Dg = Rotation*Dg;
  double Chi = Dg.Phi()*c_Deg;
  while (Chi < -180) Chi += 360.0;
  while (Chi > +180) Chi -= 360.0;
  double Psi = Dg.Theta()*c_Deg;
  double EnergyMeasured = Compton->Ei();
  
  double Sigma, Tau;
  if (Compton->HasTrack() == true) {
    MVector De = -Compton->De();
    De = Rotation*De;
    Tau = De.Phi()*c_Deg;
    while (Tau < -180) Tau += 360.0;
    while (Tau > +180) Tau -= 360.0;
    Sigma = Dg.Theta()*c_Deg;
  } else {
    Tau = 179.99;
    Sigma = 179.99;
  }
  
  double Distance = Compton->FirstLeverArm();
  
  // Now get the origin information
  MVector IdealOriginDir = -m_SiEvent->GetIAAt(0)->GetSecondaryDirection();
  IdealOriginDir = Rotation*IdealOriginDir;
  double Lambda = IdealOriginDir.Phi()*c_Deg;
  while (Lambda < -180) Lambda += 360.0;
  while (Lambda > +180) Lambda -= 360.0;
  double Nu = IdealOriginDir.Theta()*c_Deg;
  double EnergyInitial = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
  MVector IdealPolDir = m_SiEvent->GetIAAt(0)->GetSecondaryPolarisation();
  IdealPolDir = Rotation*IdealPolDir;
  
  MVector PolAngleReferenceDir = IdealOriginDir;
  if (IdealOriginDir[0] == 0 && IdealOriginDir[1] == 0) {
    PolAngleReferenceDir.SetXYZ(1.0, 0.0, 0.0);
  } else {
    PolAngleReferenceDir[2] = 0;
    PolAngleReferenceDir.Unitize();

    MVector RotationAxis = PolAngleReferenceDir.Cross(IdealOriginDir);
    double RotationAngle = IdealOriginDir.Angle(MVector(0, 0, 1));
  
    PolAngleReferenceDir.RotateAroundVector(RotationAxis, RotationAngle);
  }
  
  // https://stackoverflow.com/questions/14066933/direct-way-of-computing-clockwise-angle-between-2-vectors
  double PolarizationAngle = atan2(PolAngleReferenceDir.Dot(IdealPolDir), IdealOriginDir.Dot(PolAngleReferenceDir.Cross(IdealPolDir)))*c_Deg;
  while (PolarizationAngle < 0) PolarizationAngle += 180.0;
  while (PolarizationAngle > 180) PolarizationAngle -= 180.0;
  
  // And fill the matrices
  m_PolarizationResponse.Add( vector<double>{ EnergyInitial, Nu, Lambda, PolarizationAngle, EnergyMeasured, Phi, Psi, Chi, Sigma, Tau, Distance } );

  //cout<<"Added: "<<Event->GetId()<<":"<<Phi<<":"<<Psi<<":"<<Chi<<endl;
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponsePolarizationBinnedMode::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponsePolarizationBinnedMode::Save()
{
  MResponseBuilder::Save(); 
  
  m_PolarizationResponse.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_PolarizationResponse.Write(GetFilePrefix() + ".11D" + m_Suffix, true);
  
  return true;
}


// MResponsePolarizationBinnedMode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
