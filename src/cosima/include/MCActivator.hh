/*
 * MCActivator.hh
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */



/******************************************************************************
 *
 * Class calculating the long term activation and/or cool-down
 *
 */

#ifndef ___MCActivator___
#define ___MCActivator___

// Geant4:

// Cosima:
#include "MCIsotopeStore.hh"
#include "MCActivatorParticle.hh"

// MEGAlib:

// Standard lib:
#include <vector>
using namespace std;

// Forward declarations:


/******************************************************************************/

class MCActivator
{
  // public interface:
public:
  /// Default constructor
  MCActivator();
  /// Default destructor
  virtual ~MCActivator();

  /// Set the name of this activator
  void SetName(MString Name) { m_Name = Name; }
  /// Get the name of this activator
  MString GetName() const { return m_Name; }

  /// Set the activation mode ConstantIrradiation 
  bool SetConstantIrradiation(double ActivationTime);
  /// Set the activation mode ConstantIrradiationWithCoolDown 
  bool SetConstantIrradiationWithCooldown(double ActivationTime, double CoolDownTime);
  /// Set the activation mode TimeProfile
  bool SetTimeProfile(G4String TimeProfile, double ActivationTime);

  /// Set the half life cut off, i.e. the detector time constant during which decays are considered coincident
  void SetHalfLifeCutOff(double CutOff) { m_HalfLifeCutOff = CutOff; }

  /// Set one of the files containing the counts
  /// You can add as many files as you wish
  bool AddCountsFile(const MString& File);

  /// Loads all the counts files
  bool LoadCountsFiles();
  
  /// Calculate the activation rates after X time activation
  bool CalculateEquilibriumRates();

  /// Set the output file name of this activator
  bool SetOutputFileName(MString OutputFileName) { m_OutputFileName = OutputFileName; return true; }
  /// Get the output file name of this activator
  MString GetOutputFileName() const { return m_OutputFileName; }

  /// Set the output mode to activities
  bool SetOutputModeActivities() { m_OutputModeActivities = true; return true; }
  /// Are we in actiovities output mode?
  bool GetOutputModeActivities() const { return (m_OutputModeActivities == true) ? true : false; }
  /// Set the output mode to particles
  bool SetOutputModeParticles() { m_OutputModeActivities = false; return true; }
  /// Are we in actiovities output mode?
  bool GetOutputModeParticles() const { return (m_OutputModeActivities == false) ? true : false; }

  /// Set the output file name of this activator
  bool SetActivationTime(double ActivationTime) { m_ActivationTime = ActivationTime; return true; }
  /// Get the output file name of this activator
  double GetActivationTime() const { return m_ActivationTime; }

  /// Save the rates file
  /// Make sure the rates have been calculated 
  bool SaveOutputFile();
  
  /// ID representing activation after constant irradiation
  static const unsigned int c_ConstantIrradiation;
  /// ID representing activation after constant irradiation and cooldown
  static const unsigned int c_ConstantIrradiationWithCooldown;
  /// ID representing activation after non-constant irradiation
  static const unsigned int c_TimeProfile;

  
  /// Return the half life of the particle
  bool DetermineHalfLife(G4ParticleDefinition* ParticleDef, double& HalfLife, double& ExcitationEnergy, bool IgnoreCutOff);


  // protected methods:
protected:

  /// Calculate the activation using equations (works up to 5 elements)
  bool ActivateByEquations(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime);
  /// Calculate the activation using numerical integration
  bool ActivateByNumericalIntegration(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime);
  // Calculate the activation using random numbers
  bool ActivateBySimulation(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime);
  // Calculate the activation using random numbers
  bool ActivateByPartialSimulation(vector<MCActivatorParticle>& P, double ActivationTime, double CooldownTime);
  
  /// Clean the trees calculated during decay chain determination
  bool CleanDecayChains(vector<vector<MCActivatorParticle> >& Tree);

  /// Determine the counts of the first produced element after Time t 
  double CountsO1(double R, double D1, double t);
  /// Determine the activation of the first produced element after Time t 
  double ActivationO1(double R, double D1, double t); 
  /// Determine the cooldown of the first produced element after Time t 
  double CooldownO1(double A1, double D1, double t);
  /// Determine the counts of the second produced element after Time t
  double CountsO2(double R, double D1, double Branching12, double D2, double t); 
  /// Determine the activation of the second produced element after Time t 
  double ActivationO2(double R, double D1, double Branching12, double D2, double t); 
  /// Determine the cooldown of the second produced element after Time t 
  double CooldownO2(double A1, double D1, double Branching12, double A2, double D2, double t) ;
  /// Determine the counts of the third produced element after Time t 
  double CountsO3(double R, double D1, double Branching12, double D2, double Branching23, double D3, double t); 
  /// Determine the activation of the third produced element after Time t 
  double ActivationO3(double R, double D1, double Branching12, double D2, double Branching23, double D3, double t); 
  /// Determine the cooldown of the third produced element after Time t 
  double CooldownO3(double A1, double D1, double Branching12, double A2, double D2, double Branching23, double A3, double D3, double t) ;
  /// Determine the counts of the fourth produced element after Time t 
  double CountsO4(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double t); 
  /// Determine the activation of the fourth produced element after Time t 
  double ActivationO4(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double t);
  /// Determine the cooldown of the fourth produced element after Time t 
  double CooldownO4(double A1, double D1, double Branching12, double A2, double D2, double Branching23, double A3, double D3, double Branching34, double A4, double D4, double t) ;
  /// Determine the counts of the fifth produced element after Time t 
  double CountsO5(double R, double D1, double Branching12, double D2, double Branching23, double D3, double Branching34, double D4, double Branching45, double D5, double t);


  // protected members:
protected:


  // private members:
private:
  /// Name of this activator
  MString m_Name;
  /// List of all the counts files
  vector<MString> m_CountsFiles;
  /// Name of this output file
  MString m_OutputFileName;
  /// Are we in paticle or activity output mode?
  bool m_OutputModeActivities;

  /// The Activation mode, one of c_ConstantIrradiation, c_ConstantIrradiationWithCooldown, c_TimeProfile
  unsigned int m_ActivationMode;
       
  /// Activation time
  double m_ActivationTime;
  /// Activation time
  double m_CooldownTime;


  /// Storing the input rates
  MCIsotopeStore m_Rates;
  /// Store for the output activation
  MCIsotopeStore m_Activation;

  /// All half life values smaller than this value are set to zero
  double m_HalfLifeCutOff;
};

#endif


/*
 * MCActivator.hh: the end...
 ******************************************************************************/
