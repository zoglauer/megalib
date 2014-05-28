/*
 * MCalibrationModel.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationModel__
#define __MCalibrationModel__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MCalibrationSpectralPoint.h"

// Forward declarations:
class TH1D;


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModel();
  //! Copy constructor
  MCalibrationModel(const MCalibrationModel& CalibrationModel);
  //! Default destuctor 
  virtual ~MCalibrationModel();

  //! The assignment operator
  virtual MCalibrationModel& operator= (const MCalibrationModel& CalibrationModel);

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModel* Clone() const;

  //! Return the name of this model
  virtual MString GetName() const { return "Model base class"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return 0; }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 0; }
  
  //! Return true, if the fit is up-to-date
  bool IsFitUpToDate() const { return m_IsFitUpToDate; }
  
  //! Fit the given histogram in the given range - return the quality of the fit
  virtual double Fit(const vector<MCalibrationSpectralPoint> Points);
  
  //! Get value - if the fit doesn't exist throw MExceptionObjectDoesNotExist
  double GetFitValue(double Value) const;
  
  //! Mimic ROOT Draw functionality
  void Draw(MString Options = "");

  //! Return the fit as aparsable string
  virtual MString ToParsableString(bool WithDescriptor);

  // A list of models
  static const unsigned int c_CalibrationModelNone        = 0;
  static const unsigned int c_CalibrationModelPoly1       = 1;
  static const unsigned int c_CalibrationModelPoly2       = 2;
  static const unsigned int c_CalibrationModelPoly3       = 3;
  static const unsigned int c_CalibrationModelPoly4       = 4;
  static const unsigned int c_CalibrationModelPoly1Inv1   = 5;
  static const unsigned int c_CalibrationModelPoly1Exp1   = 6;
  static const unsigned int c_CalibrationModelPoly1Exp2   = 7;
  static const unsigned int c_CalibrationModelPoly1Exp3   = 8;
  
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {}
  
  // private methods:
 private:



  // protected members:
 protected:
  //! The actual fit
  TF1* m_Fit;
  //! True if the fit has been performed and no parameters have been chnaged
  bool m_IsFitUpToDate;
  //! The short keyword of this model, e.g. poly1
  MString m_Keyword;

  
  // private members:
 private:


};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly1() : MCalibrationModel() { m_Keyword = "poly1"; }
  //! Copy constructor
  MCalibrationModelPoly1(const MCalibrationModelPoly1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly1& operator= (const MCalibrationModelPoly1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1* Clone() const { return new MCalibrationModelPoly1(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return P[0] + P[1]*X[0]; }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 2; }
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1); 
  }
  
  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly2 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly2() : MCalibrationModel() { m_Keyword = "poly2"; }
  //! Copy constructor
  MCalibrationModelPoly2(const MCalibrationModelPoly2& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly2() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly2& operator= (const MCalibrationModelPoly2& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly2* Clone() const { return new MCalibrationModelPoly2(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return P[0] + P[1]*X[0] + P[2]*X[0]*X[0]; }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 3; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1); 
  }
  
  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly3 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly3() : MCalibrationModel() { m_Keyword = "poly3"; }
  //! Copy constructor
  MCalibrationModelPoly3(const MCalibrationModelPoly3& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly3() {}

  //! The assignment operator
  virtual MCalibrationModelPoly3& operator= (const MCalibrationModelPoly3& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly3* Clone() const { return new MCalibrationModelPoly3(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2 + a3*x^3"; }
    
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return P[0] + P[1]*X[0] + P[2]*X[0]*X[0] + P[3]*X[0]*X[0]*X[0]; }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 4; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1, 1); 
  }

  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly4 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly4() : MCalibrationModel() { m_Keyword = "poly4"; }
  //! Copy constructor
  MCalibrationModelPoly4(const MCalibrationModelPoly4& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly4() {}

  //! The assignment operator
  virtual MCalibrationModelPoly4& operator= (const MCalibrationModelPoly4& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly4* Clone() const { return new MCalibrationModelPoly4(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2 + a3*x^3 + a4*x^4"; }
    
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { return P[0] + P[1]*X[0] + P[2]*X[0]*X[0] + P[3]*X[0]*X[0]*X[0] + P[4]*X[0]*X[0]*X[0]*X[0]; }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 5; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1, 1, 1); 
  }

  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Inv1 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly1Inv1() : MCalibrationModel() { m_Keyword = "poly1inv1"; }
  //! Copy constructor
  MCalibrationModelPoly1Inv1(const MCalibrationModelPoly1Inv1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Inv1() {}

  //! The assignment operator
  virtual MCalibrationModelPoly1Inv1& operator= (const MCalibrationModelPoly1Inv1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Inv1* Clone() const { return new MCalibrationModelPoly1Inv1(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + 1/(a2 + a3*x)"; }
    
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { if (P[2] + P[3]*X[0] == 0) return 1000000000.0; return P[0] + P[1]*X[0] + 1.0/(P[2] + P[3]*X[0]); }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 4; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, -10, 1); 
  }

  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Exp1 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly1Exp1() : MCalibrationModel() { m_Keyword = "poly1exp1"; }
  //! Copy constructor
  MCalibrationModelPoly1Exp1(const MCalibrationModelPoly1Exp1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Exp1() {}

  //! The assignment operator
  virtual MCalibrationModelPoly1Exp1& operator= (const MCalibrationModelPoly1Exp1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Exp1* Clone() const { return new MCalibrationModelPoly1Exp1(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*exp(-x/a3)"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { 
    double Return =  P[0] + P[1]*X[0];
    if (P[3] != 0) Return += P[2]*exp(-X[0]/P[3]); 
    return Return;
  }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 4; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1, 7); 
  }
  
  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Exp2 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly1Exp2() : MCalibrationModel() { m_Keyword = "poly1exp2"; }
  //! Copy constructor
  MCalibrationModelPoly1Exp2(const MCalibrationModelPoly1Exp2& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Exp2() {}

  //! The assignment operator
  virtual MCalibrationModelPoly1Exp2& operator= (const MCalibrationModelPoly1Exp2& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Exp2* Clone() const { return new MCalibrationModelPoly1Exp2(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*exp(-(x/a3)^2)"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { 
    double Return =  P[0] + P[1]*X[0];
    if (P[3] != 0) Return += P[2]*exp(-pow(X[0]/P[3], 2)); 
    return Return;
  }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 4; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1, 7); 
  }
  
  // private members:
 private:

};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Exp3 : public MCalibrationModel
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationModelPoly1Exp3() : MCalibrationModel() { m_Keyword = "poly1exp3"; }
  //! Copy constructor
  MCalibrationModelPoly1Exp3(const MCalibrationModelPoly1Exp3& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Exp3() {}

  //! The assignment operator
  virtual MCalibrationModelPoly1Exp3& operator= (const MCalibrationModelPoly1Exp3& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }

  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Exp3* Clone() const { return new MCalibrationModelPoly1Exp3(*this); }

  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*exp(-(x/a3)^3)"; }
  
  //! The function for ROOT fitting
  virtual double operator() (double* X, double* P) { 
    double Return =  P[0] + P[1]*X[0];
    if (P[3] != 0) Return += P[2]*exp(-pow(X[0]/P[3], 3)); 
    return Return;
  }

  //! Return the number of fit parameters
  virtual int GetNParameters() { return 4; }
   
  
  // protected methods:
 protected:
  //! Initialize the fit parameters
  virtual void InitializeFitParameters() {
    m_Fit->SetParameters(0, 1, 1, 7); 
  }
  
  // private members:
 private:

};

#endif


////////////////////////////////////////////////////////////////////////////////
