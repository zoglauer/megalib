/*
 * MComptonEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MComptonEvent__
#define __MComptonEvent__


////////////////////////////////////////////////////////////////////////////////

#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


class MComptonEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  MComptonEvent();
  virtual ~MComptonEvent();

  // Initilisations
  bool Assimilate(MComptonEvent* Compton);
  bool Assimilate(MPhysicalEvent* Event); 
  bool Assimilate(const MVector& C1, const MVector& C2, const MVector& De, const double Ee, const double Eg);
  bool Assimilate(char* LineBuffer);
  virtual bool Stream(fstream& Stream, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  virtual int ParseLine(const char* Line, bool Fast = false);
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();


  // Basic data:

  //! Set the energy of the scattered gamma-ray
  void SetEg(const double Eg) { m_Eg = Eg; }
  //! Set the energy error of the scattered gamma-ray
  void SetdEg(const double dEg) { m_dEg = dEg; }

  //! Set the energy of the recoil electron
  void SetEe(const double Ee) { m_Ee = Ee; }
  //! Set the energy error of the recoil electron
  void SetdEe(const double dEe) { m_dEe = dEe; }

  //! Set the position of the first Compton interaction
  void SetC1(const MVector& C1) { m_C1 = C1; }
  //! Set the position error of the first Compton interaction
  void SetdC1(const MVector& dC1) { m_dC1 = dC1; }

  //! Set the position of the second (Compton) interaction
  void SetC2(const MVector& C2) { m_C2 = C2; }
  //! Set the position error of the second (Compton) interaction
  void SetdC2(const MVector& dC2) { m_dC2 = dC2; }

  //! Set the direction of the recoil electron
  void SetDe(const MVector& De) { m_De = De; }
  //! Set the direction error of the recoil electron
  void SetdDe(const MVector& dDe) { m_dDe = dDe; }

  //! Set the time of flight between first and second interaction
  void SetToF(const double& ToF) { m_ToF = ToF; }
  //! Set the time of flight error between first and second interaction
  void SetdToF(const double& dToF) { m_dToF = dToF; }

  //! Set the Compton quality factor resultion for the event reconstruction
  void SetClusteringQualityFactor(const double ClusteringQualityFactor) { m_ClusteringQualityFactor = ClusteringQualityFactor; }

  //! Set the length of the Compton sequence (number of Compton intercations)
  void SetSequenceLength(const unsigned int Length) { m_SequenceLength = Length; }
  //! Set the Compton quality factor resultion for the event reconstruction
  void SetComptonQualityFactor1(const double ComptonQualityFactor1) { m_ComptonQualityFactor1 = ComptonQualityFactor1; }
  //! Set the Compton quality factor of the second best Compton interaction sequence
  void SetComptonQualityFactor2(const double ComptonQualityFactor2) { m_ComptonQualityFactor2 = ComptonQualityFactor2; }

  //! Set the smallest lever arm in the Compton interaction sequence 
  void SetLeverArm(const double LeverArm) { m_LeverArm = LeverArm; }

  //! Set the length (in layers) of the recoil electron track
  void SetTrackLength(const unsigned int Length) { m_TrackLength = Length; }
  //! Set the energy of the first deposit of the recoil electron
  void SetTrackInitialDeposit(const double Deposit) { m_TrackInitialDeposit = Deposit; }
  //! Set the quality factor of the electron track
  void SetTrackQualityFactor1(const double TrackQualityFactor1) { m_TrackQualityFactor1 = TrackQualityFactor1; }
  //! Set the quality factor of the second best (i.e. not choosen) electron track
  void SetTrackQualityFactor2(const double TrackQualityFactor2) { m_TrackQualityFactor2 = TrackQualityFactor2; }

  //! Set the coincidence window
  void SetCoincidenceWindow(const MTime& CoincidenceWindow) { m_CoincidenceWindow = CoincidenceWindow; }
  

  // Elementary Compton data, i.e. the stored stuff 
  // inlined for efficiency reasons:
  inline double Eg() const { return m_Eg; }
  inline double dEg() const { return m_dEg; }

  inline double Ee() const { return m_Ee; }
  inline double dEe() const { return m_dEe; }

  inline MVector C1() const { return m_C1; }
  inline MVector dC1() const { return m_dC1; }

  inline MVector C2() const { return m_C2; }
  inline MVector dC2() const { return m_dC2; }

  inline MVector De() const { return m_De; }
  inline MVector dDe() const { return m_dDe; }

  inline double ToF() const { return m_ToF; }
  inline double dToF() const { return m_dToF; }

  inline double ClusteringQualityFactor() const { return m_ClusteringQualityFactor; }

  inline unsigned int SequenceLength() const { return m_SequenceLength; }
  inline double ComptonQualityFactor1() const { return m_ComptonQualityFactor1; }
  inline double ComptonQualityFactor2() const { return m_ComptonQualityFactor2; }

  inline unsigned int TrackLength() { return m_TrackLength; }
  inline double TrackInitialDeposit() { return m_TrackInitialDeposit; }
  inline double TrackQualityFactor1() const { return m_TrackQualityFactor1; }
  inline double TrackQualityFactor2() const { return m_TrackQualityFactor2; }

  inline double LeverArm() const { return m_LeverArm; }



  // Advanced/not elementary Compton data:

  //! Get the total (initial) energy of the gamma-ray
  inline double Ei() const { return m_Ei; }
  //! Get the total (initial) energy error of the gamma-ray
  inline double dEi() const { return sqrt(m_dEe*m_dEe + m_dEg*m_dEg); }

  //! Get the total (initial) energy of the gamma-ray
  virtual double GetEnergy() const { return m_Ei; }

  //! Get the position of the event - this is the location of the initial interaction!
  virtual MVector GetPosition() const { return m_C1; }

  //! Return the direction of the scattered gamma-ray
  inline MVector Dg() const { return m_Dg; }
  //! Return the initial REVERSE direction of the gamma-ray (if known) - points towards sky!!!!
  inline MVector Di() const { return m_Di; }
  //! Return the initial REVERSE direction of the gamma-ray as a projection on the Compton cone - points towards sky!!!!
  inline MVector DiOnCone() const { return m_DiOnCone; }
  //! Get the origin direction of the event - if it has none return g_VectorNotDefined
  //! In detector coordinates - this is the reverse travel direction!
  virtual MVector GetOrigin() const { return (m_HasTrack == true) ? m_DiOnCone : g_VectorNotDefined; }

  //! Return the Compton scatter angle
  inline double Phi() const { return m_Phi; }
  //! Return the Compton scatter angle uncertainty (not const because dPhi is only caluclated when needed)
  double dPhi();
  //! Return the recoil electron scatter angle
  inline double Epsilon() const { return m_Epsilon; }
  //! Return the total scatter angle
  inline double Theta() const { return m_Theta; }

  //! Return the difference between total scatter angle calculated via kinematics and via geometry
  inline double DeltaTheta() const { return m_DeltaTheta; }

  //! Return true if this event has a track
  inline bool HasTrack() const { return m_HasTrack; }

  //! Return the coincidence window
  MTime CoincidenceWindow() const { return m_CoincidenceWindow; }


  //! Return the Angular Resolution Measure value for the gamma cone for the given test position
  double GetARMGamma(const MVector& Position) const;
  //! Return the Angular Resolution Measure value for the electron cone for the given test position
  double GetARMElectron(const MVector& Position) const;
  //! Return the Scatter Plane Deviation value for the given test position
  double GetSPDElectron(const MVector& Position) const;

  //! Representation of the Kleine Nishina cross-section value of this events data
  double GetKleinNishina() const;

  static double GetKleinNishina(const double Ei, const double phi);
  static double GetKleinNishinaNormalized(const double Ei, const double phi); // Normalized to Max = 1
  static double GetKleinNishinaNormalizedByArea(const double Ei, const double phi);

  //! Return the Compton scatter angle
  double PhiUncertainty(); // const;

  //bool GetZenitDistance(double& theta);
  //bool GetDistanceBetweenHits(double& ZenitDistance);
  
  MPhysicalEvent* Data(); 


  // Miscellaneous:
  virtual void Reset();
  virtual MString ToString() const;
  MString ToBasicString() const;

  // To do: Implement all different possibilities...
  static double ComputeEeViaThetaEg(const double Theta, const double Eg);
  static double ComputeEgViaThetaEe(const double Theta, const double Ee);
  static double ComputePhiViaEeEg(const double Ee, const double Eg);
  static double ComputeEeViaPhiEg(const double Phi, const double Eg);
  static double ComputeEeViaPhiEi(const double Phi, const double Ei);

  double CalculateThetaViaAngles() const;

  bool IsKinematicsOK() const;
  static bool IsKinematicsOK(const double Ee, const double Eg);

  //! Switch direction: Reverse the Compton path
  bool SwitchDirection();


  // protected methods:
 protected:


  // private methods:
 private:
  // Calculate angles:
  bool CalculatePhi();
  bool CalculatePhiViaTheta();
  bool CalculateEpsilon();
  bool CalculateThetaViaEnergies();
  bool CalculateDi();

  //! Compute the difference between the total scatter angle calculated by energy and by geometry
  bool CalculateDeltaTheta();


  // protected members:
 public:
  // basic data:

  //! Energy of scattered gamma ray
  double m_Eg;        
  double m_dEg;       // its error
  double m_Ee;        // Energy of recoil electron
  double m_dEe;       // its error
  
  MVector m_C1;        // Location of first Compton interaction
  MVector m_dC1;       // its error

  MVector m_C2;        // Location of second Compton interaction
  MVector m_dC2;       // its error

  MVector m_De;        // Direction of electron path
  MVector m_dDe;       // its error

  double m_ToF;       // Time of flight between first and seconf interaction
  double m_dToF;      // its error

  double m_ClusteringQualityFactor;    // Value of the test statistics/probability of the clustering algorithm

  unsigned int m_SequenceLength;     // Length of the Compton sequence
  double m_ComptonQualityFactor1;    // Value of the test statistics of CSR
  double m_ComptonQualityFactor2;    // Value of the test statistics of CSR

  // Advance data:
  double m_Ei;        // Energy of initial gamma-ray
  double m_dEi;       // its error

  double m_Phi;       // scattering angle of the gamma-ray
  double m_dPhi;      // its error
  double m_Epsilon;   // scattering angle of recoil electron
  double m_sEpsilon;  // its error
  double m_Theta;       // total scattering angle
  double m_dTheta;      // its error

  double m_DeltaTheta;       // total scattering angle: difference between kinematics and geometry

  MVector m_Dg;        // direction of scattered gamma-ray
  MVector m_dDg;       // its error

  MVector m_Di;        // direction of initial gamma-ray
  MVector m_dDi;       // error

  MVector m_DiOnCone;        // direction of initial gamma-ray as a projection on the Compton cone

  bool m_HasTrack;              // true if there is an electron track;
  unsigned int m_TrackLength;   // Length of track in layers
  double m_TrackInitialDeposit; // Energy deposit in first layer of track
  double m_TrackQualityFactor1;    // Quality factor of this track (is any)
  double m_TrackQualityFactor2;    // Quality factor of the second best track combination, which has not be choosen

  double m_LeverArm;    // Minimum lever arm between Compton interactions

  //! The coincidence window
  MTime m_CoincidenceWindow;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MComptonEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
