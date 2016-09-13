/*
 * MSettingsImaging.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsImaging__
#define __MSettingsImaging__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MCoordinateSystem.h"
#include "MResponseType.h"
#include "MSettings.h"
#include "MPointSource.h"
#include "MExposureMode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsImaging : public MSettingsInterface
{
  // public Session:
 public:
  MSettingsImaging();
  virtual ~MSettingsImaging();

  // ModificationFlags:
  bool IsBackprojectionModified() { return m_BackprojectionModified; }
  void ResetBackprojectionModified() { m_BackprojectionModified = false; }
  bool IsLikelihoodModified() { return m_LikelihoodModified; }
  void ResetLikelihoodModified() { m_LikelihoodModified = false; }


  // Menu backprojection
  //////////////////////


  // Menu Response selection:

  MResponseType GetResponseType() const { return m_ResponseType; }
  void SetResponseType(MResponseType ResponseType) { m_ResponseType = ResponseType; m_BackprojectionModified = true; }


  // Menu Fitparameter for 1D Gauss approximation:

  double GetFitParameterComptonLongSphere() const { return m_FitParameterComptonLongSphere; }
  void SetFitParameterComptonLongSphere(double FitParameterComptonLongSphere) { m_FitParameterComptonLongSphere = FitParameterComptonLongSphere; m_BackprojectionModified = true; }

  double GetFitParameterComptonTransSphere() const { return m_FitParameterComptonTransSphere; }
  void SetFitParameterComptonTransSphere(double FitParameterComptonTransSphere) { m_FitParameterComptonTransSphere = FitParameterComptonTransSphere; m_BackprojectionModified = true; }

  double GetFitParameterPair() const { return m_FitParameterPair; }
  void SetFitParameterPair(double FitParameterPair) { m_FitParameterPair = FitParameterPair; m_BackprojectionModified = true; }

  bool GetUseAbsorptions() const { return m_UseAbsorptions; }
  void SetUseAbsorptions(bool UseAbsorptions) { m_UseAbsorptions = UseAbsorptions; m_BackprojectionModified = true; }

  double GetGauss1DCutOff() const { return m_Gauss1DCutOff; }
  void SetGauss1DCutOff(double Gauss1DCutOff) { m_Gauss1DCutOff = Gauss1DCutOff; m_BackprojectionModified = true; }

  double GetGaussianByUncertaintiesIncrease() const { return m_GaussianByUncertaintiesIncrease; }
  void SetGaussianByUncertaintiesIncrease(double GaussianByUncertaintiesIncrease) { m_GaussianByUncertaintiesIncrease = GaussianByUncertaintiesIncrease; m_BackprojectionModified = true; }

  // Parameters for multi-D ARM shapes
  MString GetImagingResponseConeShapesFileName() const { return m_ImagingResponseConeShapesFileName; }
  void SetImagingResponseConeShapesFileName(MString ImagingResponseConeShapesFileName) { m_ImagingResponseConeShapesFileName = ImagingResponseConeShapesFileName; m_BackprojectionModified = true; }
  
  
  // Parameters for Partially-binned list-mode
  MString GetImagingResponseComptonLongitudinalFileName() const { return m_ImagingResponseComptonLongitudinalFileName; }
  void SetImagingResponseComptonLongitudinalFileName(MString ImagingResponseComptonLongitudinalFileName) { m_ImagingResponseComptonLongitudinalFileName = ImagingResponseComptonLongitudinalFileName; m_BackprojectionModified = true; }

  MString GetImagingResponseComptonTransversalFileName() const { return m_ImagingResponseComptonTransversalFileName; }
  void SetImagingResponseComptonTransversalFileName(MString ImagingResponseComptonTransversalFileName) { m_ImagingResponseComptonTransversalFileName = ImagingResponseComptonTransversalFileName; m_BackprojectionModified = true; }


  MString GetImagingResponsePairRadialFileName() const { return m_ImagingResponsePairRadialFileName; }
  void SetImagingResponsePairRadialFileName(MString ImagingResponsePairRadialFileName) { m_ImagingResponsePairRadialFileName = ImagingResponsePairRadialFileName; m_BackprojectionModified = true; }


  // Menu Coordinate-system

  MCoordinateSystem GetCoordinateSystem() const { return m_CoordinateSystem; }
  void SetCoordinateSystem(MCoordinateSystem CoordinateSystem) { m_CoordinateSystem = CoordinateSystem; m_BackprojectionModified = true; }

  // Menu Dimensions - spherical
  MVector GetImageRotationXAxis() const { return m_ImageRotationXAxis; }
  void SetImageRotationXAxis(MVector ImageRotationXAxis) { m_ImageRotationXAxis = ImageRotationXAxis; m_BackprojectionModified = true; }

  MVector GetImageRotationZAxis() const { return m_ImageRotationZAxis; }
  void SetImageRotationZAxis(MVector ImageRotationZAxis) { m_ImageRotationZAxis = ImageRotationZAxis; m_BackprojectionModified = true; }

  double GetThetaMin() const { return m_ThetaMin; }
  void SetThetaMin(double ThetaMin) { m_ThetaMin = ThetaMin; m_BackprojectionModified = true; }

  double GetThetaMax() const { return m_ThetaMax; }
  void SetThetaMax(double ThetaMax) { m_ThetaMax = ThetaMax; m_BackprojectionModified = true; }

  double GetPhiMin() const { return m_PhiMin; }
  void SetPhiMin(double PhiMin) { m_PhiMin = PhiMin; m_BackprojectionModified = true; }

  double GetPhiMax() const { return m_PhiMax; }
  void SetPhiMax(double PhiMax) { m_PhiMax = PhiMax; m_BackprojectionModified = true; }

  int GetBinsTheta() const { return m_BinsTheta; }
  void SetBinsTheta(int BinsTheta) { m_BinsTheta = BinsTheta; m_BackprojectionModified = true; }

  int GetBinsPhi() const { return m_BinsPhi; }
  void SetBinsPhi(int BinsPhi) { m_BinsPhi = BinsPhi; m_BackprojectionModified = true; }



  // Menu Dimensions - galactic
  double GetGalLatitudeMin() const { return m_GalLatitudeMin; }
  void SetGalLatitudeMin(double GalLatitudeMin) { m_GalLatitudeMin = GalLatitudeMin; m_BackprojectionModified = true; }

  double GetGalLatitudeMax() const { return m_GalLatitudeMax; }
  void SetGalLatitudeMax(double GalLatitudeMax) { m_GalLatitudeMax = GalLatitudeMax; m_BackprojectionModified = true; }
  
  double GetGalLongitudeMin() const { return m_GalLongitudeMin; }
  void SetGalLongitudeMin(double GalLongitudeMin) { m_GalLongitudeMin = GalLongitudeMin; m_BackprojectionModified = true; }

  double GetGalLongitudeMax() const { return m_GalLongitudeMax; }
  void SetGalLongitudeMax(double GalLongitudeMax) { m_GalLongitudeMax = GalLongitudeMax; m_BackprojectionModified = true; }

  int GetBinsGalLatitude() const { return m_BinsGalLatitude; }
  void SetBinsGalLatitude(int BinsGalLatitude) { m_BinsGalLatitude = BinsGalLatitude; m_BackprojectionModified = true; }

  int GetBinsGalLongitude() const { return m_BinsGalLongitude; }
  void SetBinsGalLongitude(int BinsGalLongitude) { m_BinsGalLongitude = BinsGalLongitude; m_BackprojectionModified = true; }


  // Menu Dimensions - Cartesean
  double GetXMin() const { return m_XMin; }
  void SetXMin(double XMin) { m_XMin = XMin; m_BackprojectionModified = true; }

  double GetXMax() const { return m_XMax; }
  void SetXMax(double XMax) { m_XMax = XMax; m_BackprojectionModified = true; }

  int GetBinsX() const { return m_BinsX; }
  void SetBinsX(int BinsX) { m_BinsX = BinsX; m_BackprojectionModified = true; }
  
  double GetYMin() const { return m_YMin; }
  void SetYMin(double YMin) { m_YMin = YMin; m_BackprojectionModified = true; }

  double GetYMax() const { return m_YMax; }
  void SetYMax(double YMax) { m_YMax = YMax; m_BackprojectionModified = true; }

  int GetBinsY() const { return m_BinsY; }
  void SetBinsY(int BinsY) { m_BinsY = BinsY; m_BackprojectionModified = true; }

  double GetZMin() const { return m_ZMin; }
  void SetZMin(double ZMin) { m_ZMin = ZMin; m_BackprojectionModified = true; }

  double GetZMax() const { return m_ZMax; }
  void SetZMax(double ZMax) { m_ZMax = ZMax; m_BackprojectionModified = true; }

  int GetBinsZ() const { return m_BinsZ; }
  void SetBinsZ(int BinsZ) { m_BinsZ = BinsZ; m_BackprojectionModified = true; }


  // Menu memory management
  int GetRAM() const { return m_RAM; }
  void SetRAM(int RAM) { m_RAM = RAM;  m_BackprojectionModified = true; }

  int GetSwap() const { return m_Swap; }
  void SetSwap(int Swap) { m_Swap = Swap; m_BackprojectionModified = true; }

  int GetMemoryExhausted() const { return m_MemoryExhausted; }
  void SetMemoryExhausted(int MemoryExhausted) { m_MemoryExhausted = MemoryExhausted; m_BackprojectionModified = true; }

  int GetBytes() const { return m_Bytes; }
  void SetBytes(int Bytes) { m_Bytes = Bytes;   m_BackprojectionModified = true; }

  bool GetApproximatedMaths() const { return m_ApproximatedMaths; }
  void SetApproximatedMaths(bool ApproximatedMaths) { m_ApproximatedMaths = ApproximatedMaths; m_BackprojectionModified = true; }

  bool GetFastFileParsing() const { return m_FastFileParsing; }
  void SetFastFileParsing(bool FastFileParsing) { m_FastFileParsing = FastFileParsing; }

  int GetNThreads() const { return m_NThreads; }
  void SetNThreads(int NThreads) { m_NThreads = NThreads;  m_BackprojectionModified = true; }

  
  // Menu exposure
  MExposureMode GetExposureMode() const { return m_ExposureMode; }
  void SetExposureMode(MExposureMode ExposureMode) { m_ExposureMode = ExposureMode; m_BackprojectionModified = true; }
 
  MString GetExposureEfficiencyFile() const { return m_ExposureEfficiencyFile; }
  void SetExposureEfficiencyFile(MString ExposureEfficiencyFile) { m_ExposureEfficiencyFile = ExposureEfficiencyFile; }
  
  // Menu likelihood

  // Menu algorithm
  unsigned int GetLHAlgorithm() const { return m_LHAlgorithm; }
  void SetLHAlgorithm(unsigned int LHAlgorithm) { m_LHAlgorithm = LHAlgorithm; m_LikelihoodModified = true; }

  unsigned int GetOSEMSubSets() const { return m_OSEMSubSets; }
  void SetOSEMSubSets(unsigned int OSEMSubSets) { m_OSEMSubSets = OSEMSubSets; m_LikelihoodModified = true; }

  unsigned int GetLHStopCriteria() const { return m_LHStopCriteria; }

  void SetLHStopCriteria(unsigned int LHStopCriteria) { m_LHStopCriteria = LHStopCriteria; m_LikelihoodModified = true; }

  double GetLHIncrease() const { return m_LHIncrease; }
  void SetLHIncrease(double LHIncrease) { m_LHIncrease = LHIncrease; m_LikelihoodModified = true; }

  unsigned int GetNIterations() const { return m_NIterations; }
  void SetNIterations(unsigned int NIterations) { m_NIterations = NIterations; m_LikelihoodModified = true; }

  // Menu penalty
  int GetPenalty() const { return m_Penalty; }
  void SetPenalty(int Penalty) { m_Penalty = Penalty; m_LikelihoodModified = true; }

  double GetPenaltyAlpha() const { return m_PenaltyAlpha; }
  void SetPenaltyAlpha(double PenaltyAlpha) { m_PenaltyAlpha = PenaltyAlpha; m_LikelihoodModified = true; }



  // Image drawing options
  int GetImageDrawMode() const { return m_ImageDrawMode; }
  void SetImageDrawMode(int ImageDrawMode) { m_ImageDrawMode = ImageDrawMode; }

  int GetImagePalette() const { return m_ImagePalette; }
  void SetImagePalette(int ImagePalette) { m_ImagePalette = ImagePalette; }

  MString GetImageSourceCatalog() const { return m_ImageSourceCatalog; }
  void SetImageSourceCatalog(MString ImageSourceCatalog) { m_ImageSourceCatalog = ImageSourceCatalog; }
 

  // Animation options
  int GetAnimationMode() const { return m_AnimationMode; }
  void SetAnimationMode(int AnimationMode) { m_AnimationMode = AnimationMode; }

  double GetAnimationFrameTime() const { return m_AnimationFrameTime; }
  void SetAnimationFrameTime(double AnimationFrameTime) { m_AnimationFrameTime = AnimationFrameTime; }

  MString GetAnimationFileName() const { return m_AnimationFileName; }
  void SetAnimationFileName(MString AnimationFileName) { m_AnimationFileName = AnimationFileName; }

 
  // Significance map
  double GetSignificanceMapRadius() const { return m_SignificanceMapRadius; }
  void SetSignificanceMapRadius(double SignificanceMapRadius) { m_SignificanceMapRadius = SignificanceMapRadius; }
  double GetSignificanceMapDistance() const { return m_SignificanceMapDistance; }
  void SetSignificanceMapDistance(double SignificanceMapDistance) { m_SignificanceMapDistance = SignificanceMapDistance; }



  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 protected:

  // The modified flags:
  bool m_BackprojectionModified;
  bool m_LikelihoodModified;

  // Global stuff
  bool m_StoreImages;

  MResponseType m_ResponseType;
  MCoordinateSystem m_CoordinateSystem;
  
  unsigned int m_LHAlgorithm;
  unsigned int m_OSEMSubSets;
  unsigned int m_LHStopCriteria;
  double m_LHIncrease;
  int m_Penalty;
  double m_PenaltyAlpha;
  unsigned int m_NIterations;

  
  // Image dimensions spherical
  MVector m_ImageRotationXAxis;
  MVector m_ImageRotationZAxis;

  double m_ThetaMin;
  double m_ThetaMax;

  double m_PhiMin;
  double m_PhiMax;

  int m_BinsTheta;
  int m_BinsPhi;

  // Image dimensions galactic
  double m_GalLatitudeMin;
  double m_GalLatitudeMax;

  double m_GalLongitudeMin;
  double m_GalLongitudeMax;

  int m_BinsGalLatitude;
  int m_BinsGalLongitude;

  // Image dimensions Cartesean
  double m_XMin;
  double m_XMax;

  double m_YMin;
  double m_YMax;

  double m_ZMin;
  double m_ZMax;

  int m_BinsX;
  int m_BinsY;
  int m_BinsZ;

  // Image options:
  
  //! The drawing mode, e.g. colz, cont4, etc.
  int m_ImageDrawMode;
  //! The palette ID (see MImage)
  int m_ImagePalette;
  //! The source catalog file name
  MString m_ImageSourceCatalog;

  //! The used exposure mode
  MExposureMode m_ExposureMode;
  //! The effificiency file from which the exposure can be calculated
  MString m_ExposureEfficiencyFile;
  
  
  // Animation options:
  
  //! The animation mode (animate backprojections or animate iterations)
  int m_AnimationMode;
  //! The time between frames in seconds of observation time
  double m_AnimationFrameTime;
  //! The name of the to be genarted animated gif file 
  MString m_AnimationFileName;
  
  
  // Response:
  
  double m_FitParameterComptonLongSphere;
  double m_FitParameterComptonTransSphere;
  double m_FitParameterPair;
  bool m_UseAbsorptions;
  double m_Gauss1DCutOff;
  double m_GaussianByUncertaintiesIncrease;

  MString m_ImagingResponseComptonLongitudinalFileName;
  MString m_ImagingResponseComptonTransversalFileName;
  MString m_ImagingResponsePairRadialFileName;

  MString m_ImagingResponseConeShapesFileName;


  // Memory management
  int m_RAM;
  int m_Swap;
  int m_MemoryExhausted;
  int m_Bytes;
  bool m_ApproximatedMaths;
  bool m_FastFileParsing;
  int m_NThreads;

  
  // Significance map parameters
  double m_SignificanceMapRadius;
  double m_SignificanceMapDistance;


#ifdef ___CINT___
 public:
  ClassDef(MSettingsImaging, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
