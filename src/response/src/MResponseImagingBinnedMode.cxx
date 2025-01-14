/*
 * MResponseImagingBinnedMode.cxx
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
// MResponseImagingBinnedMode
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingBinnedMode.h"

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
ClassImp(MResponseImagingBinnedMode)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingBinnedMode::MResponseImagingBinnedMode() : m_ImagingResponse(true)
{
  m_ResponseNameSuffix = "binnedimaging";
  m_OnlyINITRequired = true;

  // TODO: New default response parameters
  m_ParametrizationMode = "rel";
  m_EpsilonNBins = 1;
  m_EpsilonBinMode = "lin";
  m_EpsilonMinimum = -1;
  m_EpsilonMaximum = 1;
  m_EpsilonBinEdges.clear();
  m_PhiBinEdges.clear(); // deg
  m_ThetaBinEdges.clear(); // deg
  m_ZetaBinEdges.clear(); // deg
  
  m_AngleBinWidth = 5; // deg
  m_AngleBinWidthElectron = 360; // deg
  m_AngleBinMode = "fisbel"; 
  m_EnergyNBins = 1;
  m_EnergyBinMode = "lin";
  m_EnergyMinimum = 10; // keV
  m_EnergyMaximum = 2000; // keV
  m_EnergyBinEdges.clear();
  m_DistanceNBins = 1;
  m_DistanceMinimum = 0; // cm
  m_DistanceMaximum = 1000; // cm
  
  m_UseAtmosphericAbsorption = false;
  m_AtmosphericAbsorptionFileName = "";
  m_Altitude = 33500;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingBinnedMode::~MResponseImagingBinnedMode()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseImagingBinnedMode::Description()
{
  return MString("Binned imaging response");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseImagingBinnedMode::Options()
{
  ostringstream out;

  // TODO: Additional response parameters
  out<<"             parammode:               use either rel (relative) or abs (absolute) response parametrization (default: rel)"<<endl;
  out<<"             epsmin                   minimum epsilon (default: -1; cannot be used in combination with epsbinedges)"<<endl;
  out<<"             epsmax                   maximum epsilon (default: 1; cannot be used in combination with epsbinedges)"<<endl;
  out<<"             epsbins                  number of epsilon bins between min and max epsilon (default: 1; cannot be used in combination with epsbinedges)"<<endl;
  out<<"             epsbinedges              the epsilon bin edges as a comma seperated list (default: not used, cannot be used in combination with epsmin, epsmax, epsbins)"<<endl;
  out<<"             phibinedges              the phi bin edges as a comma seperated list (default: not used)"<<endl;
  out<<"             thetabinedges            the theta bin edges as a comma seperated list (default: not used)"<<endl;
  out<<"             zetabinedges             the zeta bin edges as a comma seperated list (default: not used)"<<endl;

  out<<"             anglebinmode:            Use either FISBEL or HEALPix for all spherical coordinates (default: FISBEL)"<<endl;
  out<<"             anglebinwidth:           Approximate width of a sky bin at the equator (default: 5 deg)"<<endl;
  out<<"                                      In HEALpix mode, the resolution will be downgraded to the closest order, e.g."<<endl;
  out<<"                                      14.659 - 29.316 deg --> 29.316 deg (order: 1, bins: 48)"<<endl;
  out<<"                                       7.330 - 14.658 deg --> 14.658 deg (order: 2, bins: 192)"<<endl;
  out<<"                                       3.665 -  7.329 deg -->  7.329 deg (order: 3, bins: 768)"<<endl;
  out<<"                                       1.833 -  3.664 deg -->  3.664 deg (order: 4, bins: 3072)"<<endl;
  out<<"                                       0.917 -  1.832 deg -->  1.832 deg (order: 5, bins: 12288)"<<endl;
  out<<"                                       0.459 -  0.916 deg -->  0.916 deg (order: 6, bins: 49152)"<<endl;
  out<<"             emin:                    minimum energy (default: 10 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             emax:                    maximum energy (default: 2,000 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebins:                   number of energy bins between min and max energy (default: 1; cannot be used in combination with ebinedges)"<<endl;
  out<<"             emode:                   One of: lin (linear), log (logarithmic) (default: lin; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebinedges:               the energy bin edges as a comma seperated list (default: not used, cannot be used in combination with emin, emax, or ebins)"<<endl;
  out<<"             anglebinwidthelectron:   the width of a spherial bin at the equator for recoil electrons (default: 360 deg - just one overall bin)"<<endl;
  out<<"             dmin:                    minimum distance (default: 0 cm)"<<endl;
  out<<"             dmax:                    maximum distance (default: 1,000 cm)"<<endl;
  out<<"             dbins:                   number of distance bins between min and max distance (default: 1)"<<endl;
  out<<"             atabsfile:               the atmospheric absorption file name (default: \"\" (i.e. none))"<<endl;
  out<<"             atabsheight:             altitude for the atmospheric absorption (default: 33500 m)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseImagingBinnedMode::ParseOptions(const MString& Options)
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

  // TODO: Epsilon option checks
  bool HasEpsBinEdges = false;
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i][0] == "epsbinedges") {
      HasEpsBinEdges = true;
    }
  }
  if (HasEpsBinEdges == true) {
    for (unsigned int i = 0; i < Split2.size(); ++i) {
      if (Split2[i][0] == "epsmin") {
        mout<<"Error: You can not have epsbinedges and epsmin at the same time as options"<<endl;
        return false;
      } else if (Split2[i][0] == "epsmax") {
        mout<<"Error: You can not have epsbinedges and epsmax at the same time as options"<<endl;
        return false;
      } else if (Split2[i][0] == "epsbins") {
        mout<<"Error: You can not have epsbinedges and epsbins at the same time as options"<<endl;
        return false;
      }
    }
  }
  // TODO: Phi option checks
  bool HasPhiBinEdges = false;
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i][0] == "phibinedges") {
      HasPhiBinEdges = true;
    }
  }
  // TODO: Theta option checks
  bool HasThetaBinEdges = false;
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i][0] == "thetabinedges") {
      HasThetaBinEdges = true;
    }
  }
  // TODO: Zeta option checks
  bool HasZetaBinEdges = false;
  for (unsigned int i = 0; i < Split2.size(); ++i) {
    if (Split2[i][0] == "zetabinedges") {
      HasZetaBinEdges = true;
    }
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
    MString MValue = Split2[i][1];
    string Value = MValue.Data();

    // TODO: Response mode, epsilon, phi, theta and zeta parse
    if (Split2[i][0] == "parammode") {
      m_ParametrizationMode = MValue.ToLower();
    // TODO: Remove Seq Option
    } else if (Split2[i][0] == "seqmin") {
      m_SeqMin = stod(Value);
    } else if (Split2[i][0] == "seqmax") {
      m_SeqMax = stod(Value);
    } else if (Split2[i][0] == "epsmin") {
      m_EpsilonMinimum = stod(Value);
    } else if (Split2[i][0] == "epsmax") {
      m_EpsilonMaximum = stod(Value);
    } else if (Split2[i][0] == "epsbins") {
      m_EpsilonNBins = stod(Value);
    } else if (Split2[i][0] == "epsbinmode") {
      m_EpsilonBinMode = MValue.ToLower();
    } else if (Split2[i][0] == "epsbinedges") {
      vector<MString> EpsEdges = MString(Value).Tokenize(",");
      m_EpsilonBinEdges.clear();
      for (MString S: EpsEdges) m_EpsilonBinEdges.push_back(S.ToDouble());
      m_EpsilonNBins = 0;
    } else if (Split2[i][0] == "phibinedges") {
      vector<MString> PhiEdges = MString(Value).Tokenize(",");
      m_PhiBinEdges.clear();
      for (MString S: PhiEdges) m_PhiBinEdges.push_back(S.ToDouble());
    } else if (Split2[i][0] == "thetabinedges") {
      vector<MString> ThetaEdges = MString(Value).Tokenize(",");
      m_ThetaBinEdges.clear();
      for (MString S: ThetaEdges) {
        if (static_cast<unsigned char>(S.ToString()[0]) == static_cast<unsigned char>(226)) {
          S = MString("-" + S.ToString().substr(3));
        }
        m_ThetaBinEdges.push_back(S.ToDouble());
      }
    } else if (Split2[i][0] == "zetabinedges") {
      vector<MString> ZetaEdges = MString(Value).Tokenize(",");
      m_ZetaBinEdges.clear();
      for (MString S: ZetaEdges) m_ZetaBinEdges.push_back(S.ToDouble());
    } else if (Split2[i][0] == "emin") {
      m_EnergyMinimum = stod(Value);
    } else if (Split2[i][0] == "emax") {
      m_EnergyMaximum = stod(Value);
    } else if (Split2[i][0] == "ebins") {
      m_EnergyNBins = stod(Value);
    } else if (Split2[i][0] == "ebinmode") {
      m_EnergyBinMode = MValue.ToLower();
    } else if (Split2[i][0] == "ebinedges") {
      vector<MString> Edges = MString(Value).Tokenize(",");
      m_EnergyBinEdges.clear();
      for (MString S: Edges) m_EnergyBinEdges.push_back(S.ToDouble());
      m_EnergyNBins = 0;
    } else if (Split2[i][0] == "anglebinwidth") {
      m_AngleBinWidth = stod(Value);
    } else if (Split2[i][0] == "anglebinmode") {
      m_AngleBinMode = MValue.ToLower();
    } else if (Split2[i][0] == "dmin") {
      m_DistanceMinimum = stod(Value);
    } else if (Split2[i][0] == "dmax") {
      m_DistanceMaximum = stod(Value);
    } else if (Split2[i][0] == "dbins") {
      m_DistanceNBins = stod(Value);
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

  // TODO: Response mode, epsilon, phi, theta and zeta sanity check
  if (m_ParametrizationMode != "rel" && m_ParametrizationMode != "abs") {
      mout<<"Error: The parametrization mode is either 'rel' or 'abs'"<<endl;
      return false; 
  }
  if (HasEpsBinEdges == true) {
    if (m_EpsilonBinEdges.size() < 2) {
      mout<<"Error: You need at least 2 epsilon bin edges"<<endl;
      return false;    
    }
    for (unsigned int e = 1; e < m_EpsilonBinEdges.size(); ++e) {
      if (m_EpsilonBinEdges[e] <= m_EpsilonBinEdges[e-1]) {
        mout<<"Error: The epsilon bin edges must be in increasing order"<<endl;
        return false;    
      }
    }
  } else {
    if (m_EpsilonMinimum >= m_EpsilonMaximum) {
      mout<<"Error: The minimum epsilon must be smaller than the maximum epsilon"<<endl;
      return false;       
    }
    if (m_EpsilonNBins <= 0) {
      mout<<"Error: You need at least one epsilon bin"<<endl;
      return false;       
    }
    if (m_EpsilonBinMode != "lin" && m_EpsilonBinMode != "log") {
      mout<<"Error: Epsilon bins only support lin and log modes"<<endl;
      return false;       
    }
  }
  if (HasPhiBinEdges == true) {
    if (m_PhiBinEdges.size() < 2) {
      mout<<"Error: You need at least 2 phi bin edges"<<endl;
      return false;    
    }
    for (unsigned int e = 1; e < m_PhiBinEdges.size(); ++e) {
      if (m_PhiBinEdges[e] <= m_PhiBinEdges[e-1]) {
        mout<<"Error: The phi bin edges must be in increasing order"<<endl;
        return false;    
      }
    }
  }
  if (HasThetaBinEdges == true) {
    if (m_ThetaBinEdges.size() < 2) {
      mout<<"Error: You need at least 2 theta bin edges"<<endl;
      return false;    
    }
    for (unsigned int e = 1; e < m_ThetaBinEdges.size(); ++e) {
      if (m_ThetaBinEdges[e] <= m_ThetaBinEdges[e-1]) {
        mout<<"Error: The theta bin edges must be in increasing order"<<endl;
        return false;    
      }
    }
  }
  if (HasZetaBinEdges == true) {
    if (m_ZetaBinEdges.size() < 2) {
      mout<<"Error: You need at least 2 zeta bin edges"<<endl;
      return false;    
    }
    for (unsigned int e = 1; e < m_ZetaBinEdges.size(); ++e) {
      if (m_ZetaBinEdges[e] <= m_ZetaBinEdges[e-1]) {
        mout<<"Error: The zeta bin edges must be in increasing order"<<endl;
        return false;    
      }
    }
  }
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
    if (m_EnergyBinMode != "lin" && m_EnergyBinMode != "log") {
      mout<<"Error: Energy bins only support lin and log modes"<<endl;
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
  if (m_AngleBinWidth <= 0) {
    mout<<"Error: You need at give a positive width of the sky bins at the equator"<<endl;
    return false;       
  }
  if (m_AngleBinWidthElectron <= 0) {
    mout<<"Error: You need at give a positive width of the sky bins at the equator for the recoil electron"<<endl;
    return false;       
  }
  if (m_AngleBinMode != "fisbel" && m_AngleBinMode != "healpix") {
    mout<<"Error: Sky bins only support fisbel and healpix modes"<<endl;
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

  // TODO: Output response mode, epsilon, phi, theta, zeta
  mout<<"  Response parametrization mode:                      "<<m_ParametrizationMode<<endl;
  if (HasEpsBinEdges == true) {
    mout<<"  Bin edges epsilon:                                  ";
    for (double E: m_EpsilonBinEdges) mout<<E<<" ";
    mout<<endl;
  } else {
    mout<<"  Minimum epsilon:                                    "<<m_EpsilonMinimum<<endl;
    mout<<"  Maximum epsilon:                                    "<<m_EpsilonMaximum<<endl;
    mout<<"  Number of bins epsilon:                             "<<m_EpsilonNBins<<endl;
    mout<<"  Epsilon binning mode:                               "<<m_EpsilonBinMode<<endl;
  }
  if (HasBinEdges == true) {
    mout<<"  Bin edges energy:                                   ";
    for (double E: m_EnergyBinEdges) mout<<E<<" ";
    mout<<endl;
  } else {
    mout<<"  Minimum energy:                                     "<<m_EnergyMinimum<<endl;
    mout<<"  Maximum energy:                                     "<<m_EnergyMaximum<<endl;
    mout<<"  Number of bins energy:                              "<<m_EnergyNBins<<endl;
    mout<<"  Energy binning mode:                                "<<m_EnergyBinMode<<endl;
  }
  if (HasPhiBinEdges == true) {
    mout<<"  Bin edges phi:                                      ";
    for (double E: m_PhiBinEdges) mout<<E<<" ";
    mout<<endl;
  }
  if (HasThetaBinEdges == true) {
    mout<<"  Bin edges theta:                                    ";
    for (double E: m_ThetaBinEdges) mout<<E<<" ";
    mout<<endl;
  }
  if (HasZetaBinEdges == true) {
    mout<<"  Bin edges zeta:                                     ";
    for (double E: m_ZetaBinEdges) mout<<E<<" ";
    mout<<endl;
  }
  mout<<"  Width of sky bins at equator:                       "<<m_AngleBinWidth<<endl;
  mout<<"  Sky bins binning mode:                              "<<m_AngleBinMode<<endl;
  mout<<"  Minimum distance:                                   "<<m_DistanceMinimum<<endl;
  mout<<"  Maximum distance:                                   "<<m_DistanceMaximum<<endl;
  mout<<"  Number of bins distance:                            "<<m_DistanceNBins<<endl;
  mout<<"  Width of sky bins at equator for recoil electron:  "<<m_AngleBinWidthElectron<<endl;
  if (m_UseAtmosphericAbsorption == true) {
    mout<<"  Atmospheric absorption file name:                   "<<m_AtmosphericAbsorptionFileName<<endl;
    mout<<"  Atmospheric absorption altitude:                    "<<m_Altitude<<endl;
  }
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingBinnedMode::Initialize()
{  
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Initialize() == false) return false;

  // Cast the reader for easier information extraction
  MFileEvents* Reader = nullptr;
  if (m_SiReader != nullptr) {
    Reader = dynamic_cast<MFileEvents*>(m_SiReader);
  } else if (m_TraReader != nullptr) {
    Reader = dynamic_cast<MFileEvents*>(m_TraReader);
  }

  MResponseMatrixAxis AxisEnergyInitial("Initial energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyInitial.SetBinEdges(m_EnergyBinEdges);
  } else {
    if (m_EnergyBinMode == "log") {
      AxisEnergyInitial.SetLogarithmic(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
    } else {
      AxisEnergyInitial.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
    }
  }
  
  MResponseMatrixAxisSpheric AxisSkyCoordinates("#nu [deg]", "#lambda [deg]");

  if (m_AngleBinMode == "fisbel") {
    AxisSkyCoordinates.SetFISBELByPixelSize(m_AngleBinWidth);
  } else {
    AxisSkyCoordinates.SetHEALPixByPixelSize(m_AngleBinWidth);
    cout<<"Using HEALPix for sky with "<<AxisSkyCoordinates.GetNumberOfBins()<<" bins"<<endl;
  }
    
  // TODO: Define epsilon axis
  MResponseMatrixAxis AxisEpsilon("Epsilon");
  if (m_EpsilonBinEdges.size() > 0) {
    AxisEpsilon.SetBinEdges(m_EpsilonBinEdges);
  } else {
    if (m_EpsilonBinMode == "log") {
      AxisEpsilon.SetLogarithmic(m_EpsilonNBins, m_EpsilonMinimum, m_EpsilonMaximum);
    } else {
      AxisEpsilon.SetLinear(m_EpsilonNBins, m_EpsilonMinimum, m_EpsilonMaximum);
    }
  }

  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyMeasured.SetBinEdges(m_EnergyBinEdges);
  } else {
    if (m_EnergyBinMode == "log") {
      AxisEnergyMeasured.SetLogarithmic(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
    } else {
      AxisEnergyMeasured.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
    }
  }

  // TODO: Define angles
  MResponseMatrixAxis AxisPhi("#phi [deg]");
  if (m_PhiBinEdges.size() > 0) {
    AxisPhi.SetBinEdges(m_PhiBinEdges);
  } else {
    AxisPhi.SetLinear(180/m_AngleBinWidth, 0, 180);
  }

  MResponseMatrixAxis AxisTheta("#theta [deg]");
  if (m_ThetaBinEdges.size() > 0) {
    AxisTheta.SetBinEdges(m_ThetaBinEdges);
  } else {
    AxisTheta.SetLinear(180/m_AngleBinWidth, -180, 180);
  }
  
  MResponseMatrixAxis AxisZeta("#zeta [deg]");
  if (m_ZetaBinEdges.size() > 0) {
    AxisZeta.SetBinEdges(m_ZetaBinEdges);
  } else {
    AxisZeta.SetLinear(180/m_AngleBinWidth, 0, 360);
  }
  
  MResponseMatrixAxisSpheric AxisScatteredGammaRayCoordinates("#psi [deg]", "#chi [deg]");
  if (m_AngleBinMode == "fisbel") {
    AxisScatteredGammaRayCoordinates.SetFISBELByPixelSize(m_AngleBinWidth);
  } else {
    AxisScatteredGammaRayCoordinates.SetHEALPixByPixelSize(m_AngleBinWidth);
    cout<<"Using HEALPix for scattered gamma ray with "<<AxisScatteredGammaRayCoordinates.GetNumberOfBins()<<" bins"<<endl;
  }
  
  MResponseMatrixAxisSpheric AxisRecoilElectronCoordinates("#sigma [deg]", "#tau [deg]");
  if (m_AngleBinMode == "fisbel") {
    AxisRecoilElectronCoordinates.SetFISBELByPixelSize(m_AngleBinWidthElectron);
  } else {
    AxisRecoilElectronCoordinates.SetHEALPixByPixelSize(m_AngleBinWidthElectron);
    cout<<"Using HEALPix for recoil electron with "<<AxisRecoilElectronCoordinates.GetNumberOfBins()<<" bins"<<endl;
  }
    
  MResponseMatrixAxis AxisDistance("Distance [cm]");
  AxisDistance.SetLinear(m_DistanceNBins, m_DistanceMinimum, m_DistanceMaximum);
  
  m_ImagingResponse.SetName("10D imaging response");
  m_ImagingResponse.AddAxis(AxisEnergyInitial);
  m_ImagingResponse.AddAxis(AxisSkyCoordinates);

  // TODO: Add correct energy and angle axis
  if (m_ParametrizationMode == "rel") {
    m_ImagingResponse.AddAxis(AxisEpsilon);
  } else {
    m_ImagingResponse.AddAxis(AxisEnergyMeasured);
  }

  m_ImagingResponse.AddAxis(AxisPhi);

  if (m_ParametrizationMode == "rel") {
    m_ImagingResponse.AddAxis(AxisTheta);
    m_ImagingResponse.AddAxis(AxisZeta);
  } else {
    m_ImagingResponse.AddAxis(AxisScatteredGammaRayCoordinates);
  }
  m_ImagingResponse.AddAxis(AxisRecoilElectronCoordinates);
  m_ImagingResponse.AddAxis(AxisDistance);
  if (Reader != nullptr) {
    m_ImagingResponse.SetFarFieldStartArea(Reader->GetSimulationStartAreaFarField());
    m_ImagingResponse.SetSpectralType(Reader->GetSpectralType());
    m_ImagingResponse.SetBeamType(Reader->GetBeamType());
  }

  m_Exposure.SetName("Exposure");
  m_Exposure.AddAxis(AxisEnergyInitial);
  m_Exposure.AddAxis(AxisSkyCoordinates);
  if (Reader != nullptr) {
    m_Exposure.SetFarFieldStartArea(Reader->GetSimulationStartAreaFarField());
    m_Exposure.SetSpectralType(Reader->GetSpectralType());
    m_Exposure.SetBeamType(Reader->GetBeamType());
  }

  m_EnergyResponse4D.SetName("Energy response 4D");
  m_EnergyResponse4D.AddAxis(AxisEnergyInitial);
  m_EnergyResponse4D.AddAxis(AxisSkyCoordinates);

  // TODO: Add correct energy axis
  if (m_ParametrizationMode == "rel") {
    m_EnergyResponse4D.AddAxis(AxisEpsilon);
  } else {
    m_EnergyResponse4D.AddAxis(AxisEnergyMeasured);
  }
  if (Reader != nullptr) {
    m_EnergyResponse4D.SetFarFieldStartArea(Reader->GetSimulationStartAreaFarField());
    m_EnergyResponse4D.SetSpectralType(Reader->GetSpectralType());
    m_EnergyResponse4D.SetBeamType(Reader->GetBeamType());
  }

  m_EnergyResponse2D.SetName("Energy response 2D");
  m_EnergyResponse2D.AddAxis(AxisEnergyInitial);

  // TODO: Add correct energy axis
  if (m_ParametrizationMode == "rel") {
    m_EnergyResponse2D.AddAxis(AxisEpsilon);
  } else {
    m_EnergyResponse2D.AddAxis(AxisEnergyMeasured);
  }
  if (Reader != nullptr) {
    m_EnergyResponse2D.SetFarFieldStartArea(Reader->GetSimulationStartAreaFarField());
    m_EnergyResponse2D.SetSpectralType(Reader->GetSpectralType());
    m_EnergyResponse2D.SetBeamType(Reader->GetBeamType());
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
bool MResponseImagingBinnedMode::Analyze() 
{ 
  // Initialize the next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;
  
  MPhysicalEvent* Event = nullptr;
  MVector IdealOriginDir;
  double EnergyInitial;
  if (m_Mode == MResponseBuilderReadMode::SimFile || m_Mode == MResponseBuilderReadMode::SimEventByEvent) {
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
    Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
    if (Event == nullptr) {
      return true;
    }
    
    // and thew origin information
    IdealOriginDir = -m_SiEvent->GetIAAt(0)->GetSecondaryDirection();
    EnergyInitial = m_SiEvent->GetIAAt(0)->GetSecondaryEnergy();
    
  } else if (m_Mode == MResponseBuilderReadMode::TraFile) {
    Event = m_TraEvent;
    if (Event == nullptr) {
      return true;
    }
    IdealOriginDir = -Event->GetOIDirection();
    if (IdealOriginDir == g_VectorNotDefined) {
      mout<<"You have a tra event without origin information"<<endl;
      return true;
    }
    EnergyInitial = Event->GetOIEnergy();
    if (IdealOriginDir == g_DoubleNotDefined) {
      mout<<"You have a tra event without origin information"<<endl;
      return true;
    }    
    
  } else {
    cout<<"Unknown mode in MResponseImagingBinnedMode::Analyze"<<endl;
    return false;
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
  
  if (m_UseAtmosphericAbsorption == true) {
    // Deselect gamma rays based on the transmission probability 
    
    double OriginAzimuth = IdealOriginDir.Theta() * c_Deg;
    double OriginEnergy = EnergyInitial;
    
    double P = m_AtmosphericAbsorption.GetTransmissionProbability(m_Altitude, OriginAzimuth, OriginEnergy);
   
    if (gRandom->Rndm() > P) {
      return true;
    }
  }
  // TODO: Remove Selection
  if (Compton->SequenceLength() < m_SeqMin || Compton->SequenceLength() > m_SeqMax) {
    mout<<"Wrong sequence length: "<<(Compton->SequenceLength())<<endl;
    return true;
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
    Sigma = De.Theta()*c_Deg;
  } else {
    Tau = 179.99;
    Sigma = 179.99;
  }
  
  double Distance = Compton->FirstLeverArm();
  
  // Now enhance the origin information
  IdealOriginDir = Rotation*IdealOriginDir;
  double Lambda = IdealOriginDir.Phi()*c_Deg;
  while (Lambda < -180) Lambda += 360.0;
  while (Lambda > +180) Lambda -= 360.0;
  double Nu = IdealOriginDir.Theta()*c_Deg;

  // TODO: Relative energy implementation
  double Epsilon = (EnergyMeasured - EnergyInitial)/EnergyInitial;

  // TODO: Theta implementation;
  // MVector IdealOriginPosition = m_SiEvent->GetIAAt(0)->GetPosition();
  // double Theta = Compton->GetARMGamma(IdealOriginPosition)*c_Deg;
  double PhiGeomtric = IdealOriginDir.Angle(Dg)*c_Deg;
  double Theta = PhiGeomtric - Phi;
  // mout<<"My: ("<<Theta_2<<") vs. MEGAlib: ("<<Theta<<")"<<endl;
  // mout<<"File: "<<IdealOriginDir<<" vs. MEGAlib: "<<(IdealOriginPosition - Compton->m_C1).Unit()<<endl;

  // TODO: Zeta implementation - RelativeX convention
  MVector Axis = MVector(1.0, 0.0, 0.0);
  MVector PolarizationZ = -IdealOriginDir;
  MVector PolarizationX = PolarizationZ.Cross(Axis).Unit();
  MVector PolarizationY = PolarizationZ.Cross(PolarizationX).Unit();

  MVector ProjectedDg = PolarizationZ.Cross(Dg);
  MRotation ChangeBasis = MRotation(PolarizationX.GetX(), PolarizationX.GetY(), PolarizationX.GetZ(),
                                    PolarizationY.GetX(), PolarizationY.GetY(), PolarizationY.GetZ(),
                                    PolarizationZ.GetX(), PolarizationZ.GetY(), PolarizationZ.GetZ());
  ProjectedDg = ChangeBasis * ProjectedDg;
  double Zeta = ProjectedDg.Phi()*c_Deg - 90;
  if (Zeta < 0) Zeta += 360;

  // TODO: Zeta Alternative
  // double Zeta_2 = 0;
  // double YAngle = PolarizationZ.Cross(Dg).Angle(PolarizationY)*c_Deg;
  // double XAngle = PolarizationZ.Cross(Dg).Angle(PolarizationX)*c_Deg;
  // if (YAngle <= 90) {
  //   if (XAngle <= 90) {
  //     Zeta_2 = 270 + XAngle;
  //   } else {
  //     Zeta_2 = YAngle;
  //   }
  // } else {
  //   if (XAngle <= 90) {
  //     Zeta_2 = 270 - XAngle;
  //   } else {
  //     Zeta_2 = YAngle;
  //   }
  // }

  // mout << "Zeta 01 ("<<Zeta<<") - Zeta 02 ("<<Zeta_2<<")"<<endl;
  
  // And fill the matrices

  // TODO: Relative parametrization matrix fill
  if (m_ParametrizationMode == "rel") {
    m_ImagingResponse.Add( vector<double>{ EnergyInitial, Nu, Lambda, Epsilon, Phi, Theta, Zeta, Sigma, Tau, Distance } );
    m_Exposure.Add( vector<double>{ EnergyInitial, Nu, Lambda } );
    m_EnergyResponse4D.Add( vector<double>{ EnergyInitial, Nu, Lambda, Epsilon } );
    m_EnergyResponse2D.Add( vector<double>{ EnergyInitial, Epsilon } );
  } else {
    m_ImagingResponse.Add( vector<double>{ EnergyInitial, Nu, Lambda, EnergyMeasured, Phi, Psi, Chi, Sigma, Tau, Distance } );
    m_Exposure.Add( vector<double>{ EnergyInitial, Nu, Lambda } );
    m_EnergyResponse4D.Add( vector<double>{ EnergyInitial, Nu, Lambda, EnergyMeasured } );
    m_EnergyResponse2D.Add( vector<double>{ EnergyInitial, EnergyMeasured } );
  }
            
  //cout<<"Added: "<<Event->GetId()<<":"<<Phi<<":"<<Psi<<":"<<Chi<<endl;
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseImagingBinnedMode::Finalize() 
{ 
  return MResponseBuilder::Finalize(); 
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingBinnedMode::Save()
{
  MResponseBuilder::Save(); 
  
  m_ImagingResponse.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ImagingResponse.Write(GetFilePrefix() + ".imagingresponse" + m_Suffix, true);
  
  m_Exposure.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_Exposure.Write(GetFilePrefix() + ".exposure" + m_Suffix, true);
  
  m_EnergyResponse4D.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyResponse4D.Write(GetFilePrefix() + ".energyresponse4d" + m_Suffix, true);
  
  m_EnergyResponse2D.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_EnergyResponse2D.Write(GetFilePrefix() + ".energyresponse2d" + m_Suffix, true);
  
  return true;
}


// MResponseImagingBinnedMode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
