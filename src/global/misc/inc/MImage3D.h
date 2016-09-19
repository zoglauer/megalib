/*
 * MImage3D.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MImage3D__
#define __MImage3D__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImage3D : public MImage2D
{
  // public interface:
 public:
  //! Default constructor
  MImage3D();
  //! Standard constructor
  MImage3D(MString Title, double *IA, 
           MString xTitle, double xMin, double xMax, int xNBins, 
           MString yTitle, double yMin, double yMax, int yNBins, 
           MString zTitle, double zMin, double zMax, int zNBins, 
           int Spectrum = c_Rainbow, int DrawOption = c_COLCONT4Z);
  //! Standard destructor
  virtual ~MImage3D();

  //! Clone this image
  virtual MImage* Clone();

  //! Set the image array and redisplay it
  virtual void SetImageArray(double* IA);

  //! Set the z-Axis attributes
  virtual void SetZAxis(MString zTitle, double zMin, double zMax, int zNBins);

  //! Display the histogram in the given canvas
  virtual void Display(TCanvas* Canvas = 0);
  
  //! Get the dimensions of the histogram
  virtual unsigned int GetDimensions() const { return 3; }
  
  //! Determine the maximum, the vector is filled up to the number of dimensions the histogram has
  virtual void DetermineMaximum(double& MaxValue, vector<double>& Coordinate);



  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  // Title of the z axis
  // Alreday defined in the base class: MString m_zTitle;
  //! Minimum z value
  double m_zMin;
  //! Maximum z value
  double m_zMax;
  //! Number of z bins
  int m_zNBins;

  //! Projection in XY of the main 3D histogram
  TH1* m_HistXY;
  //! Projection in XZ of the main 3D histogram
  TH1* m_HistXZ;
  //! Projection in YZ of the main 3D histogram
  TH1* m_HistYZ;

  //! Canvas for the XY projection
  TCanvas* m_CanvasXY;
  //! Canvas for the XZ projection
  TCanvas* m_CanvasXZ;
  //! Canvas for the YZ projection
  TCanvas* m_CanvasYZ;



#ifdef ___CINT___
 public:
  ClassDef(MImage3D, 0) // a three dimensional image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
