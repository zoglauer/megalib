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
#include "TH1D.h"
#include "Math/WrappedTF1.h"
#include "Math/WrappedMultiTF1.h"
#include "Fit/BinData.h"
#include "Fit/UnBinData.h"
#include "HFitInterface.h"
#include "Fit/Fitter.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MCalibrationSpectralPoint.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



//! Definition of the coordinate system IDs
enum class MCalibrationModelType : unsigned int {
  c_Unknown = 0, c_Energy = 1, c_LineWidth = 2
};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
//! It can either be an energy model or a line-width model
class MCalibrationModel : public ROOT::Math::IParamFunction
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
  
  //! ROOT fitting interface: Set the number of parameters
  void SetParameters(const double *p) { m_ROOTParameters.clear(); for (unsigned int i = 0; i < NPar(); ++i) m_ROOTParameters.push_back(p[i]);  }
  //! ROOT fitting interface: Get the parameters
  const double* Parameters() const { return &m_ROOTParameters[0]; }
  //! ROOT fitting interface: Return the number of fit parameters
  virtual unsigned int NPar() const { return 0; }
  //! Return the number of fit parameters 
  unsigned int GetNParameters() const { return NPar(); }
  //! Return the fit parameter or 0 in case of error 
  double GetParameter(unsigned int p) const { if (p >= m_ROOTParameters.size()) return 0; return m_ROOTParameters[p]; }
  
  //! Return true, if the fit is up-to-date
  bool IsFitUpToDate() const { return m_IsFitUpToDate; }
  
  //! Set the model type: energy or line width
  void SetType(MCalibrationModelType Type) { m_Type = Type; }
  
  //! Fit the given histogram in the given range - return the quality of the fit
  virtual double Fit(const vector<MCalibrationSpectralPoint> Points);
  
  //! Get value - return 0 if the fit doesn't exist
  double GetFitValue(double Value) const;
  
  //! Get the fit quality
  double GetFitQuality() const { return m_FitQuality; }
  
  //! Mimic ROOT Draw functionality
  void Draw(MString Options = "");

  //! Return the fit as aparsable string
  virtual MString ToParsableString(bool WithDescriptor);

  // A list of models
  static const unsigned int c_CalibrationModelNone            = 0;
  static const unsigned int c_CalibrationModelPoly1Zero       = 1;
  static const unsigned int c_CalibrationModelPoly1           = 2;
  static const unsigned int c_CalibrationModelPoly2           = 3;
  static const unsigned int c_CalibrationModelPoly3           = 4;
  static const unsigned int c_CalibrationModelPoly4           = 5;
  static const unsigned int c_CalibrationModelPoly1Inv1Zero   = 6;
  static const unsigned int c_CalibrationModelPoly1Inv1       = 7;
  static const unsigned int c_CalibrationModelPoly2Inv1Zero   = 8;
  static const unsigned int c_CalibrationModelPoly2Inv1       = 9;
  static const unsigned int c_CalibrationModelPoly1Exp1       = 10;
  static const unsigned int c_CalibrationModelPoly1Exp2       = 11;
  static const unsigned int c_CalibrationModelPoly1Exp3       = 12;
  static const unsigned int c_CalibrationModelPoly1Log1       = 13;
  static const unsigned int c_CalibrationModelPoly2Log1       = 14;
  
  
  // protected methods:
 protected:
  //! ROOT fitting interface: evaulate the fit function
  virtual double DoEvalPar(double X, const double* P) const { return 0.0; }
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) { cout<<"Base class init"<<endl; }
  
  // private methods:
 private:



  // protected members:
 protected:
  //! The calibration model type 
  MCalibrationModelType m_Type; 
   
  //! The actual fit
  TF1* m_Fit;
  //! True if the fit has been performed and no parameters have been chnaged
  bool m_IsFitUpToDate;
  //! The short keyword of this model, e.g. poly1
  MString m_Keyword;
  //! The fit quality
  double m_FitQuality;
  
  //! The parameters for the ROOT fitting
  vector<double> m_ROOTParameters;
  
  // private members:
 private:


};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Zero : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly1Zero() : MCalibrationModel() { m_Keyword = "poly1zero"; }
  //! Copy constructor
  MCalibrationModelPoly1Zero(const MCalibrationModelPoly1Zero& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Zero() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly1Zero& operator= (const MCalibrationModelPoly1Zero& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Zero* Clone() const { return new MCalibrationModelPoly1Zero(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a1*x"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 1; }
  
  // protected methods:
protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return P[0]*X; }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(1);
  }
  
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 2; }
  
  // protected methods:
protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return P[0] + P[1]*X; }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 3; }
   
  
  // protected methods:
 protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return P[0] + P[1]*X + P[2]*X*X; }

  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
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
    
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
   
  
  // protected methods:
 protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return P[0] + P[1]*X + P[2]*X*X + P[3]*X*X*X; }

  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
    Fitter.Config().ParSettings(3).SetValue(1);
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
    
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 5; }
   
  
  // protected methods:
 protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { return P[0] + P[1]*X + P[2]*X*X + P[3]*X*X*X + P[4]*X*X*X*X; }

  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
    Fitter.Config().ParSettings(3).SetValue(1);
    Fitter.Config().ParSettings(4).SetValue(1);
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
  
  
  // protected methods:
protected:  
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { if (P[2] + P[3]*X == 0) return 1000000000.0; return P[0] + P[1]*X + 1.0/(P[2] + P[3]*X); }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(-10);
    Fitter.Config().ParSettings(3).SetValue(1);
  }
  
  // private members:
private:
  
};



////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Inv1Zero : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly1Inv1Zero() : MCalibrationModel() { m_Keyword = "poly1inv1zero"; }
  //! Copy constructor
  MCalibrationModelPoly1Inv1Zero(const MCalibrationModelPoly1Inv1Zero& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Inv1Zero() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly1Inv1Zero& operator= (const MCalibrationModelPoly1Inv1Zero& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Inv1Zero* Clone() const { return new MCalibrationModelPoly1Inv1Zero(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + 1/(a2*x)"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 3; }
  
  
  // protected methods:
protected:  
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { if (P[2]*X == 0) return 1000000000.0; return P[0] + P[1]*X + 1.0/(P[2]*X); }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
  }
  
  // private members:
private:
  
};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly2Inv1 : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly2Inv1() : MCalibrationModel() { m_Keyword = "poly2inv1"; }
  //! Copy constructor
  MCalibrationModelPoly2Inv1(const MCalibrationModelPoly2Inv1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly2Inv1() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly2Inv1& operator= (const MCalibrationModelPoly2Inv1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly2Inv1* Clone() const { return new MCalibrationModelPoly2Inv1(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2 + 1/(a3 + a4*x)"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 5; }
  
  
  // protected methods:
protected:  
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { if (P[3] + P[4]*X == 0) return 1000000000.0; return P[0] + P[1]*X + P[2]*X + 1.0/(P[3] + P[4]*X); }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
    Fitter.Config().ParSettings(3).SetValue(-10);
    Fitter.Config().ParSettings(4).SetValue(1);
  }
  
  // private members:
private:
  
};



////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly2Inv1Zero : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly2Inv1Zero() : MCalibrationModel() { m_Keyword = "poly2inv1zero"; }
  //! Copy constructor
  MCalibrationModelPoly2Inv1Zero(const MCalibrationModelPoly2Inv1Zero& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly2Inv1Zero() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly2Inv1Zero& operator= (const MCalibrationModelPoly2Inv1Zero& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly2Inv1Zero* Clone() const { return new MCalibrationModelPoly2Inv1Zero(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2 + 1/(a3*x)"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
  
  
  // protected methods:
protected:  
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { if (P[3]*X == 0) return 1000000000.0; return P[0] + P[1]*X + P[2]*X + 1.0/(P[3]*X); }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
    Fitter.Config().ParSettings(3).SetValue(1);
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
   
  
  // protected methods:
 protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { 
    double Return =  P[0] + P[1]*X;
    if (P[3] != 0) Return += P[2]*exp(-X/P[3]); 
    return Return;
  }

  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(-10);
    Fitter.Config().ParSettings(3).SetValue(1000);
    Fitter.Config().ParSettings(3).SetLowerLimit(0.0001);
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
   
  
  // protected methods:
 protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { 
    double Return =  P[0] + P[1]*X;
    if (P[3] != 0) Return += P[2]*exp(-pow(X/P[3], 2)); 
    return Return;
  }

  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(-10);
    Fitter.Config().ParSettings(3).SetValue(1000);
    Fitter.Config().ParSettings(3).SetLowerLimit(0.0001);
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
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
  
  
  // protected methods:
protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { 
    double Return =  P[0] + P[1]*X;
    if (P[3] != 0) Return += P[2]*exp(-pow(X/P[3], 3)); 
    return Return;
  }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(-100);
    Fitter.Config().ParSettings(3).SetValue(1000);
    Fitter.Config().ParSettings(3).SetLowerLimit(0.0001);
  }
  
  // private members:
private:
  
};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly1Log1 : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly1Log1() : MCalibrationModel() { m_Keyword = "poly1ln1"; }
  //! Copy constructor
  MCalibrationModelPoly1Log1(const MCalibrationModelPoly1Log1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly1Log1() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly1Log1& operator= (const MCalibrationModelPoly1Log1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly1Log1* Clone() const { return new MCalibrationModelPoly1Log1(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*log(x)"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 3; }
  
  
  // protected methods:
protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { 
    if (X <= 0) return 0.0;
    double Return =  P[0] + P[1]*X;
    Return += P[2]*log(X); 
    return Return;
  }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
  }
  
  // private members:
private:
  
};


////////////////////////////////////////////////////////////////////////////////


//! Class representing the base class for all calibration models for a set of calibration points
class MCalibrationModelPoly2Log1 : public MCalibrationModel
{
  // public interface:
public:
  //! Default constructor
  MCalibrationModelPoly2Log1() : MCalibrationModel() { m_Keyword = "poly2ln1"; }
  //! Copy constructor
  MCalibrationModelPoly2Log1(const MCalibrationModelPoly2Log1& CalibrationModelLinear) { m_Fit = 0; *this = CalibrationModelLinear; }
  //! Default destuctor 
  virtual ~MCalibrationModelPoly2Log1() {}
  
  //! The assignment operator
  virtual MCalibrationModelPoly2Log1& operator= (const MCalibrationModelPoly2Log1& CalibrationModelLinear) { MCalibrationModel::operator=(CalibrationModelLinear); return *this; }
  
  //! Clone this fit - the returned element must be deleted!
  virtual MCalibrationModelPoly2Log1* Clone() const { return new MCalibrationModelPoly2Log1(*this); }
  
  //! Return the name of this model
  virtual MString GetName() const { return "a0 + a1*x + a2*x^2 + a3*log(x)"; }
  
  //! Return the number of fit parameters
  virtual unsigned int NPar() const { return 4; }
  
  
  // protected methods:
protected:
  //! The function for ROOT fitting
  virtual double DoEvalPar(double X, const double* P) const { 
    if (X <= 0) return 0.0;
    double Return =  P[0] + P[1]*X + P[2]*X*X;
    Return += P[3]*log(X); 
    return Return;
  }
  
  //! Initialize the fit parameters
  virtual void InitializeFitParameters(ROOT::Fit::Fitter& Fitter) {
    Fitter.Config().ParSettings(0).SetValue(0);
    Fitter.Config().ParSettings(1).SetValue(1);
    Fitter.Config().ParSettings(2).SetValue(1);
    Fitter.Config().ParSettings(3).SetValue(1);
  }
  
  // private members:
private:
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
