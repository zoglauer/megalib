/*
 * MResponseImagingEfficiencyNearField.cxx
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
// MResponseImagingEfficiencyNearField
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseImagingEfficiencyNearField.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"
#include "MResponseMatrixAxisSpheric.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseImagingEfficiencyNearField)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MResponseImagingEfficiencyNearField::MResponseImagingEfficiencyNearField()
{
  m_ResponseNameSuffix = "listmoderesponsenearfield";
  m_OnlyINITRequired = true;
  
  m_XMin = -10*c_cm;
  m_XMax = 10*c_cm;
  m_XBins = 10;
  
  m_YMin = -10*c_cm;
  m_YMax = 10*c_cm;
  m_YBins = 10;
  
  m_ZMin = -10*c_cm;
  m_ZMax = 10*c_cm;
  m_ZBins = 10;
  
  m_EnergyNBins = 1;
  m_EnergyMinimum = 10*c_keV;
  m_EnergyMaximum = 2000*c_keV;
  m_EnergyBinEdges.clear();
  
  m_AngleBinWidth = 10; // deg
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MResponseImagingEfficiencyNearField::~MResponseImagingEfficiencyNearField()
{
  // Nothing to delete
}


////////////////////////////////////////////////////////////////////////////////


//! Return a brief description of this response class
MString MResponseImagingEfficiencyNearField::Description()
{
  return MString("Binned near-field response & efficiency (e.g. medical or activation detection)");
}


////////////////////////////////////////////////////////////////////////////////


//! Return information on the parsable options for this response class
MString MResponseImagingEfficiencyNearField::Options()
{
  ostringstream out;
  out<<"             xmin:           minimum x-value (default: -20 cm)"<<endl;
  out<<"             xmax:           maximum x-value (default: -0 cm)"<<endl;
  out<<"             xbins:          x bins (default: 40 bins)"<<endl;
  out<<"             ymin:           minimum y-value (default: -20 cm)"<<endl;
  out<<"             ymax:           maximum y-value (default: -0 cm)"<<endl;
  out<<"             ybins:          y bins (default: 40 bins)"<<endl;
  out<<"             zmin:           minimum z-value (default: -20 cm)"<<endl;
  out<<"             zmax:           maximum z-value (default: -0 cm)"<<endl;
  out<<"             zbins:          z bins (default: 40 bins)"<<endl;
  out<<"             emin:           minimum energy (default: 10 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             emax:           maximum energy (default: 2,000 keV; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebins:          number of energy bins between min and max energy (default: 1; cannot be used in combination with ebinedges)"<<endl;
  out<<"             ebinedges:      the energy bin edges as a comma seperated list (default: not used, cannot be used in combination with emin, emax, or ebins)"<<endl;
  out<<"             anglebinwidth:  the width of a CDS bin at the equator (default: 5 deg)"<<endl;
  
  return MString(out);
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the options
bool MResponseImagingEfficiencyNearField::ParseOptions(const MString& Options)
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
    } else if (Split2[i][0] == "xmin") {
      m_XMin = stod(Value);
    } else if (Split2[i][0] == "xmax") {
      m_XMax = stod(Value);
    } else if (Split2[i][0] == "xbins") {
      m_XBins = stod(Value);
    } else if (Split2[i][0] == "ymin") {
      m_YMin = stod(Value);
    } else if (Split2[i][0] == "ymax") {
      m_YMax = stod(Value);
    } else if (Split2[i][0] == "ybins") {
      m_YBins = stod(Value);
    } else if (Split2[i][0] == "zmin") {
      m_ZMin = stod(Value);
    } else if (Split2[i][0] == "zmax") {
      m_ZMax = stod(Value);
    } else if (Split2[i][0] == "zbins") {
      m_ZBins = stod(Value);
    } else if (Split2[i][0] == "anglebinwidth") {
      m_AngleBinWidth = stod(Value);
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
  if (m_XMin >= m_XMax) {
    mout<<"Error: The minimum x-value must be smaller than the maximum x-value"<<endl;
    return false;       
  }
  if (m_XBins <= 0) {
    mout<<"Error: You need at least one x-bin"<<endl;
    return false;       
  }
  if (m_YMin >= m_YMax) {
    mout<<"Error: The minimum y-value must be smaller than the maximum y-value"<<endl;
    return false;       
  }
  if (m_YBins <= 0) {
    mout<<"Error: You need at least one y-bin"<<endl;
    return false;       
  }
  if (m_ZMin >= m_ZMax) {
    mout<<"Error: The minimum z-value must be smaller than the maximum z-value"<<endl;
    return false;       
  }
  if (m_ZBins <= 0) {
    mout<<"Error: You need at least one z-bin"<<endl;
    return false;       
  }
  if (m_AngleBinWidth <= 0) {
    mout<<"Error: You need at give a positive width of the sky bins at the equator"<<endl;
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
    mout<<"  Minimum energy:                                    "<<m_EnergyMinimum<<endl;
    mout<<"  Maximum energy:                                    "<<m_EnergyMaximum<<endl;
    mout<<"  Number of bins energy:                             "<<m_EnergyNBins<<endl;
  }
  mout<<"  Minimum x-value:                                   "<<m_XMin<<endl;
  mout<<"  Maximum x_value:                                   "<<m_XMax<<endl;
  mout<<"  Number of x-bins:                                  "<<m_XBins<<endl;
  mout<<"  Minimum y-value:                                   "<<m_YMin<<endl;
  mout<<"  Maximum y_value:                                   "<<m_YMax<<endl;
  mout<<"  Number of y-bins:                                  "<<m_YBins<<endl;
  mout<<"  Minimum z-value:                                   "<<m_ZMin<<endl;
  mout<<"  Maximum z_value:                                   "<<m_ZMax<<endl;
  mout<<"  Number of z-bins:                                  "<<m_ZBins<<endl;
  mout<<"  Width of sky bins at equator:                      "<<m_AngleBinWidth<<endl;
  mout<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Initialize the response matrices and their generation
bool MResponseImagingEfficiencyNearField::Initialize()
{
  if (MResponseBuilder::Initialize() == false) return false;
  
  MResponseMatrixAxis AxisOriginX("Origin X [cm]");
  AxisOriginX.SetLinear(m_XBins, m_XMin, m_XMax);
  MResponseMatrixAxis AxisOriginY("Origin Y [cm]");
  AxisOriginY.SetLinear(m_YBins, m_YMin, m_YMax);
  MResponseMatrixAxis AxisOriginZ("Origin Z [cm]");
  AxisOriginZ.SetLinear(m_ZBins, m_ZMin, m_ZMax);
  
  MResponseMatrixAxis AxisEnergyInitial("Initial energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyInitial.SetBinEdges(m_EnergyBinEdges);
  } else {
    AxisEnergyInitial.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
  }
  
  MResponseMatrixAxis AxisInteractionX("Interaction X [cm]");
  AxisInteractionX.SetLinear(m_XBins, m_XMin, m_XMax);
  MResponseMatrixAxis AxisInteractionY("Interaction Y [cm]");
  AxisInteractionY.SetLinear(m_YBins, m_YMin, m_YMax);
  MResponseMatrixAxis AxisInteractionZ("Interaction Z [cm]");
  AxisInteractionZ.SetLinear(m_ZBins, m_ZMin, m_ZMax);
  
  MResponseMatrixAxis AxisEnergyMeasured("Measured energy [keV]");
  if (m_EnergyBinEdges.size() > 0) {
    AxisEnergyMeasured.SetBinEdges(m_EnergyBinEdges);
  } else {
    AxisEnergyMeasured.SetLinear(m_EnergyNBins, m_EnergyMinimum, m_EnergyMaximum);
  }
  
  int AngleBins = 4*c_Pi*c_Deg*c_Deg / m_AngleBinWidth / m_AngleBinWidth;
  if (AngleBins < 1) AngleBins = 1;
  
  MResponseMatrixAxis AxisPhi("#phi [deg]");
  AxisPhi.SetLinear(180/m_AngleBinWidth, 0, 180);
  
  MResponseMatrixAxisSpheric AxisScatteredGammaRayCoordinates("#psi [deg]", "#chi [deg]");
  AxisScatteredGammaRayCoordinates.SetFISBEL(AngleBins);
  
  
  m_ResponseEmittedXDetectedAnywhere.SetName("ResponseEmittedXDetectedAnywhere");
  m_ResponseEmittedXDetectedAnywhere.AddAxis(AxisOriginX);
  m_ResponseEmittedXDetectedAnywhere.AddAxis(AxisOriginY);
  m_ResponseEmittedXDetectedAnywhere.AddAxis(AxisOriginZ);
  m_ResponseEmittedXDetectedAnywhere.AddAxis(AxisEnergyInitial);
  cout<<"Created response "<<m_ResponseEmittedXDetectedAnywhere.GetName()<<" with "<<m_ResponseEmittedXDetectedAnywhere.GetNBins()<<" bins"<<endl;
  
  m_ResponseEmittedXDetectedY.SetName("ResponseEmittedXDetectedY");
  m_ResponseEmittedXDetectedY.AddAxis(AxisOriginX);
  m_ResponseEmittedXDetectedY.AddAxis(AxisOriginY);
  m_ResponseEmittedXDetectedY.AddAxis(AxisOriginZ);
  m_ResponseEmittedXDetectedY.AddAxis(AxisEnergyInitial);
  m_ResponseEmittedXDetectedY.AddAxis(AxisInteractionX);
  m_ResponseEmittedXDetectedY.AddAxis(AxisInteractionY);
  m_ResponseEmittedXDetectedY.AddAxis(AxisInteractionZ);
  m_ResponseEmittedXDetectedY.AddAxis(AxisEnergyMeasured);
  cout<<"Created response "<<m_ResponseEmittedXDetectedY.GetName()<<" with "<<m_ResponseEmittedXDetectedY.GetNBins()<<" bins"<<endl;
  
  m_ResponseDetectedYScatteredCDS.SetName("ResponseDetectedYScatteredCDS");
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisInteractionX);
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisInteractionY);
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisInteractionZ);
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisEnergyMeasured);
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisPhi);
  m_ResponseDetectedYScatteredCDS.AddAxis(AxisScatteredGammaRayCoordinates);
  cout<<"Created response "<<m_ResponseDetectedYScatteredCDS.GetName()<<" with "<<m_ResponseDetectedYScatteredCDS.GetNBins()<<" bins"<<endl;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Analyze the current event
bool MResponseImagingEfficiencyNearField::Analyze()
{
  // Initialize next matching event, save if necessary
  if (MResponseBuilder::Analyze() == false) return false;

  MRawEventIncarnationList* REList = m_ReReader->GetRawEventList();

  if (REList->HasOnlyOptimumEvents() == true) {
    MPhysicalEvent* Event = REList->GetOptimumEvents()[0]->GetPhysicalEvent();
    if (Event != 0) {
      if (m_MimrecEventSelector.IsQualifiedEvent(Event) == true) {
        if (Event->GetType() == c_ComptonEvent) {
          MComptonEvent* Compton = (MComptonEvent*) Event;

          if (Compton->IsKinematicsOK() == false) return true;

          // Now get the ideal origin:
          if (m_SiEvent->GetNIAs() > 0) {
            MVector IdealOrigin = m_SiEvent->GetIAAt(0)->GetPosition();
            m_ResponseEmittedXDetectedAnywhere.Add( vector<double>{ IdealOrigin.X(), IdealOrigin.Y(), IdealOrigin.Z(), m_SiEvent->GetIAAt(0)->GetSecondaryEnergy() } );
            m_ResponseEmittedXDetectedY.Add( vector<double>{ IdealOrigin.X(), IdealOrigin.Y(), IdealOrigin.Z(), m_SiEvent->GetIAAt(0)->GetSecondaryEnergy(), Compton->C1().X(), Compton->C1().Y(), Compton->C1().Z(), Compton->GetEnergy() } );
            
            // Get the data space information
            MRotation Rotation = Compton->GetDetectorRotationMatrix();

            double Phi = Compton->Phi()*c_Deg;
            MVector Dg = -Compton->Dg();
            Dg = Rotation*Dg;
            double Chi = Dg.Phi()*c_Deg;
            while (Chi < -180) Chi += 360.0;
            while (Chi > +180) Chi -= 360.0;
            double Psi = Dg.Theta()*c_Deg;       
                       
            m_ResponseDetectedYScatteredCDS.Add( vector<double>{ Compton->C1().X(), Compton->C1().Y(), Compton->C1().Z(), Compton->GetEnergy(), Phi, Psi, Chi } );
          }
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Finalize the response generation (i.e. save the data a final time )
bool MResponseImagingEfficiencyNearField::Finalize()
{
  return MResponseBuilder::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


//! Save the responses
bool MResponseImagingEfficiencyNearField::Save()
{
  MResponseBuilder::Save();

  m_ResponseEmittedXDetectedAnywhere.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ResponseEmittedXDetectedAnywhere.Write(GetFilePrefix() + ".emittedxdetectedanywhere" + m_Suffix, true);

  m_ResponseEmittedXDetectedY.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ResponseEmittedXDetectedY.Write(GetFilePrefix() + ".emittedxdetectedy" + m_Suffix, true);

  m_ResponseDetectedYScatteredCDS.SetSimulatedEvents(m_NumberOfSimulatedEventsThisFile + m_NumberOfSimulatedEventsClosedFiles);
  m_ResponseDetectedYScatteredCDS.Write(GetFilePrefix() + ".detectedyscatteredcds" + m_Suffix, true);

  return true;
}


// MResponseImagingEfficiencyNearField.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
