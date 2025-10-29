/*
 * MFileEvents.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEvents__
#define __MFileEvents__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEvents : public MFile
{
  // public interface:
 public:
  //! Default constructor - the read mode as well as file name are passed on via Open!
  MFileEvents();
  //! Default destructor
  virtual ~MFileEvents();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way, bool IsBinary);

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way) { return Open(FileName, Way, false); }

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName) { return Open(FileName, MFile::c_Read, false); }

  //! Update the progress dialog GUI -- allow to skip a certain amount of updates
  virtual bool UpdateProgress(unsigned int UpdatesToSkip = 0);

  //! Rewind to the beginning of the file
  virtual bool Rewind(bool NotUsed = true);

  //! Transfer header / footer information
  virtual void TransferInformation(MFileEvents* File);

  //! Set the geometry name
  void SetGeometryFileName(const MString Geometry);
  //! Retrieve the geometry name
  MString GetGeometryFileName() const;

  //! Retrieve MEGAlib version ID with which this file has be safed
  MString GetMEGAlibVersion() const { return m_MEGAlibVersion; }

  //! Write a header to file - obviously this shoould be done before writting events
  virtual bool WriteHeader();

  //! Add text to the file as footer - it always goes into the master file!
  //! You should NOT add events afterwards!
  virtual bool AddFooter(const MString& Text);

  //! Write a footer to file - obviously this should be done after writing events
  virtual bool WriteFooter();

  //! Close the event list after last event (per default add "END")
  virtual bool CloseEventList();

  //! Set if this is an include file
  void SetIsIncludeFile(bool IsIncludeFile) { m_IsIncludeFile = IsIncludeFile; }

  //! Return the number of events in this file (it counts the "SE")
  int GetNEvents(bool Count);

  //! Return true if we have a valid Geant4 version
  bool HasGeant4Version() const { return m_HasGeant4Version; }
  //! Get the Geant4 version - check with HasGeant4Version() first
  virtual MString GetGeant4Version() { return m_Geant4Version; }
  //! Set the Geant4 version
  void SetGeant4Version(MString Geant4Version) { m_Geant4Version = Geant4Version; m_HasGeant4Version = true;}

  //! Return true if we have a valid number of simulated events
  bool HasSimulatedEvents() const { return m_HasSimulatedEvents; }
  //! Get the number of simulated events - check with HasStartObservationTime() first
  virtual long GetSimulatedEvents() { return m_SimulatedEvents; }
  //! Set the number of simulated events
  void SetSimulatedEvents(long SimulatedEvents) { m_SimulatedEvents = SimulatedEvents; m_HasSimulatedEvents = true;}

  //! Return true if we have a valid start time of the observation
  bool HasStartObservationTime() const { return m_HasStartObservationTime; }
  //! Get the start time of the observation - check with HasStartObservationTime() first
  MTime GetStartObservationTime() const { return m_StartObservationTime; }
  //! Set the start of the observation time
  void SetStartObservationTime(MTime StartObservationTime) { m_StartObservationTime = StartObservationTime; m_HasStartObservationTime = true; }

  //! Return true if we have a valid end time of the observation
  bool HasEndObservationTime() const { return m_HasEndObservationTime; }
  //! Get the end time of the observation - check with HasEndObservationTime() first
  MTime GetEndObservationTime() const { return m_EndObservationTime; }
  //! Set the end of the observation time
  void SetEndObservationTime(MTime EndObservationTime) { m_EndObservationTime = EndObservationTime; m_HasEndObservationTime = true; }

  //! Return the observation time in this event
  //! If it has not yet been read, read it (can be very slow if the file is gzip'ed)
  MTime GetObservationTime();
  //! Set the observation time 
  void SetObservationTime(MTime ObservationTime) { m_ObservationTime = ObservationTime; m_HasObservationTime = true; }

  //! Return true if we have a valid start area of far field simulations
  bool HasSimulationStartAreaFarField() const { return m_HasSimulationStartAreaFarField; }
  //! Get the start area of far field simulations - check with HasSimulationStartAreaFarField() first
  double GetSimulationStartAreaFarField() const { return m_SimulationStartAreaFarField; }
  //! Set the start area of far field simulations
  void SetSimulationStartAreaFarField(double SimulationStartAreaFarField) { m_SimulationStartAreaFarField = SimulationStartAreaFarField; m_HasSimulationStartAreaFarField = true; }

  //! Return true if we have a valid simulation seed
  bool HasSimulationSeed() const { return m_HasSimulationSeed; }
  //! Get the random number seed of simulations - check with HasSimulationSeed() first
  unsigned long GetSimulationSeed() const { return m_SimulationSeed; }
  //! Set the random number seed of the simulations
  void SetSimulationSeed(unsigned long SimulationSeed) { m_SimulationSeed = SimulationSeed; m_HasSimulationSeed = true; }

  //! Return true if we have a valid beam type
  bool HasBeamType() const { return m_HasBeamType; }
  //! Get the beam type - check with HasBeamType() first
  MString GetBeamType() const { return m_BeamType; }
  //! Set the beam type
  void SetBeamType(MString BeamType) { m_BeamType = BeamType; m_HasBeamType = true; }

  //! Return true if we have a valid spectral type
  bool HasSpectralType() const { return m_HasSpectralType; }
  //! Get the spaectral type - check with HasSpectralType() first
  MString GetSpectralType() const { return m_SpectralType; }
  //! Set the spectral type
  void SetSpectralType(MString SpectralType) { m_SpectralType = SpectralType; m_HasSpectralType = true; }


  // protected methods:
 protected:
  //! Open a file given by the "NF" keyword
  virtual bool OpenNextFile(const MString& Line);
  //! Create a file indicated by a "NF" keyword
  virtual bool CreateNextFile();

  //! Open a file given by the "IN" keyword
  virtual bool OpenIncludeFile(const MString& Line);
  //! Close the current include file
  virtual bool CloseIncludeFile();
  
  //! Create a file indicated by a "IN" keyword
  virtual bool CreateIncludeFile();
  //! Create a file name indicated suiting for the "IN" keyword
  MString CreateIncludeFileName(const MString& FileName);

  //! Read the special information at the end of file
  //! If Continue is true, we do not seek to the end of the file, but just read the current file to the end
  bool ReadFooter(bool Continue = false);
  //! Parse the special information at the end of file -- add your special parsing in there
  virtual bool ParseFooter(const MString& Line);
  
  //! ID indicating there is no ID
  static const int c_NoId;

  // private methods:
 private:



  // protected members:
 protected:
  //! The underlying geometry file name
  MString m_GeometryFileName;
  //! The MEGAlib version string used to create the file
  MString m_MEGAlibVersion; 

  //! The number of events in this file
  int m_NEvents;

  //! The number of include files in this file
  int m_NIncludeFiles;
  //! The number of opened include files 
  int m_NOpenedIncludeFiles;
  
  //! The used extension of the file when creating new ones via the NF extension
  int m_ExtensionNumber;

  //! Sim and tra files can have included files:
  MFileEvents* m_IncludeFile;
  //! Is the included file in use
  bool m_IncludeFileUsed;
  //! True is this is an included file:
  bool m_IsIncludeFile;

  //! Extra footer text
  MString m_ExtraFooterText;

  //! The original file name (different from m_FileName in case ofjumping from file to file via NF keyword)
  MString m_OriginalFileName;

  //! True if we have a valid Geant4 version
  bool m_HasGeant4Version;
  //! The Geant4 version as string
  MString m_Geant4Version;

  //! True if we have a valid number of simulated events
  bool m_HasSimulatedEvents;
  //! Number of simulated events
  long m_SimulatedEvents;

  //! Has a start observation time
  bool m_HasStartObservationTime;
  //! The start of the observation time
  MTime m_StartObservationTime;
  
  //! Has end observation time
  bool m_HasEndObservationTime;
  //! The end of the observation time
  MTime m_EndObservationTime;
  
  //! True if we have an observation time
  bool m_HasObservationTime;
  //! The total observation time
  MTime m_ObservationTime;

  //! True if we have a valid start area of far field simulations
  bool m_HasSimulationStartAreaFarField;
  //! The start area of far field simulations
  double m_SimulationStartAreaFarField;

  //! True if we have a valid simulations random number seed
  bool m_HasSimulationSeed;
  //! The simulations random number seed
  unsigned long m_SimulationSeed;

  //! True if we have a valid beam type
  bool m_HasBeamType;
  //! The beam type
  MString m_BeamType;

  //! True if we have a valid spectral type
  bool m_HasSpectralType;
  //! The spectral type
  MString m_SpectralType;


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MFileEvents, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
