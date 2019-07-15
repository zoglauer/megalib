/*
 * MMath.cxx
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
// MMath
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMath.h"

// Standard libs:
#include <float.h>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MMath)
#endif


////////////////////////////////////////////////////////////////////////////////


MMath::MMath()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MMath::~MMath()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MMath::GalacticToSpheric(double &theta, double &phi)
{
  // --> Bogenmass verwenden!
  // Transform galactic coordinates to spherical (theta and phi are in degree)

  theta = -theta + 90;
  
  phi *= -1;
  //if (phi <= -180) phi += 360.0;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::SphericToGalactic(double &theta, double &phi)
{
  // --> Bogenmass verwenden!
  // Transform Spheric coordinates to Galactic (theta and phi are in degree)

  theta = -theta + 90;
  
  phi *= -1;
  //if (phi < 0) phi += 360.0;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::SphericToCartesean(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ)
{
  // Transform spheric coordinates (theta, phi in deg) into Cartesean coordinates

  double x, y, z;
  x = RadiusIsZ * sin(ThetaIsX*c_Rad) * cos(PhiIsY*c_Rad);
  y = RadiusIsZ * sin(ThetaIsX*c_Rad) * sin(PhiIsY*c_Rad); 
  z = RadiusIsZ * cos(ThetaIsX*c_Rad);

  ThetaIsX = x;
  PhiIsY = y;
  RadiusIsZ = z;
}


////////////////////////////////////////////////////////////////////////////////


void MMath::CarteseanToSpheric(double& ThetaIsX, double& PhiIsY, double& RadiusIsZ)
{
  // Transform spheric coordinates (theta, phi in deg) into Cartesean coordinates

  double t, p, r;
  // Now transform:
  r =  sqrt(ThetaIsX*ThetaIsX + PhiIsY*PhiIsY + RadiusIsZ*RadiusIsZ);

  if (RadiusIsZ == 0) {
    t = c_Pi/2.0;
  } else { 
    t = atan(sqrt(ThetaIsX*ThetaIsX + PhiIsY*PhiIsY)/RadiusIsZ);
    if (RadiusIsZ < 0) t += c_Pi;
  }

  p =  atan(PhiIsY/ThetaIsX); 
  if (ThetaIsX < 0) {
    if (PhiIsY > 0) p += c_Pi;
    if (PhiIsY < 0) p -= c_Pi;
  }
  if (ThetaIsX == 0 && PhiIsY > 0) p = c_Pi/2.0;
  if (ThetaIsX == 0 && PhiIsY < 0) p = -c_Pi/2.0;

  ThetaIsX = t;
  PhiIsY = p;
  RadiusIsZ = r;
}


////////////////////////////////////////////////////////////////////////////////


double MMath::Gauss(const double& x, const double& mean, const double& sigma)
{
  // Calculate a gaussian function with mean and sigma
  // It is slightly different from the original ROOT routine, containing
  // the normalization factor: 1/(sigma*c_Sqrt2Pi)

  if (sigma == 0) return 1.e30;
  double arg = (x-mean)/sigma;
  return 1/(sigma*c_Sqrt2Pi)*exp(-0.5*arg*arg);
} 


////////////////////////////////////////////////////////////////////////////////


bool MMath::InRange(double x)
{ 
  //  

  if (x < DBL_MAX && x > -DBL_MAX) {
    return true; 
  } else {
    return false; 
  }
}


////////////////////////////////////////////////////////////////////////////////


double MMath::AngleBetweenTwoVectors(const double& u, const double& v, const double& w, 
              const double& x, const double& y, const double& z)
{
  // Sollte niemand verwenden!!!!!!!!!

  double Nenner = sqrt((u*u + v*v + w*w)*(x*x + y*y + z*z));

  if (Nenner <= 0.0) {
    return 0.0;
  } else {
    Nenner = (u*x + v*y + w*z)/Nenner;
    if (Nenner > 1.0) Nenner = 1.0;
    if (Nenner < -1.0) Nenner = -1.0;
    return acos(Nenner);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Perform a Thomson Tau test
//! Requires at least two input value
//! Return for each value if it is an outlier
vector<bool> MMath::ModifiedThomsonTauTest(vector<double> Values, double Alpha)
{
  vector<bool> IsOutlier(Values.size(), false);
  
  // We need at least 3 values
  if (Values.size() < 3) {
    cout<<"ERROR in ModifiedThomsonTauTest: Need at least 2 values"<<endl;
    return IsOutlier;
  }
  
  // Alpha need to be reasonable
  if (Alpha <= 0 || Alpha > 0.5) {
    cout<<"ERROR in ModifiedThomsonTauTest: Alpha should be between 0.001 and 0.5, a good value is 0.05"<<endl;
    return IsOutlier;
  }
  
  // Protection against nan and inf in Values
  for (unsigned int i = 0; i < Values.size(); ++i) {
    if (std::isnan(Values[i])) {
      cout<<"ERROR in ModifiedThomsonTauTest: Value at index "<<i<<" is nan"<<endl;
      return IsOutlier;      
    }
    if (std::isinf(Values[i])) {
      cout<<"ERROR in ModifiedThomsonTauTest: Value at index "<<i<<" is inf"<<endl;
      return IsOutlier;      
    }
  }
  
  bool OutlierFound = false;
  unsigned int NumberOfValues = 0; 
  do {
    
    // (1) Calculate mean
    NumberOfValues = 0; 
    double Mean = 0.0;
    for (unsigned int i = 0; i < Values.size(); ++i) {
      if (IsOutlier[i] == false) {
        Mean += Values[i];
        ++NumberOfValues;
      }
    }
    Mean /= NumberOfValues;
    
    if (NumberOfValues < 3) return IsOutlier;
    
    // (2) Calculate standard deviation
    double StdDev = 0.0;
    for (unsigned int i = 0; i < Values.size(); ++i) {
      if (IsOutlier[i] == false) {
        StdDev += (Values[i] - Mean)*(Values[i] - Mean);
      }
    }
    StdDev = sqrt(StdDev / (NumberOfValues - 1));
    
    
    // (3) Determine the differences between the mean and the highest and the lowest value
    unsigned int Largest = 0;
    double DiffLargest = -numeric_limits<double>::max();
    unsigned int Smallest = 0;
    double DiffSmallest = numeric_limits<double>::max();
    for (unsigned int i = 0; i < Values.size(); ++i) {
      if (IsOutlier[i] == false) {
        if (Values[i] > DiffLargest) {
          DiffLargest = Values[i];
          Largest = i;
        }
        if (Values[i] > DiffSmallest) {
          DiffSmallest = Values[i];
          Smallest = i;
        }
      }
    }
    DiffLargest = DiffLargest - Mean;
    DiffSmallest = Mean - DiffSmallest;
    
    
    // (4) Test the larger value if it is an outlier
    double StudentTQuantile = TMath::StudentQuantile(1-Alpha/2, NumberOfValues-2); 
    double Tau = (StudentTQuantile*(NumberOfValues-1)) / (sqrt(NumberOfValues)*sqrt(NumberOfValues - 2 + StudentTQuantile*StudentTQuantile));
    double TauTimesStdDev = Tau * StdDev;
    
    //cout<<"Critical value: "<<StudentTQuantile<<" for N ="<<NumberOfValues<<"   Tau: "<<Tau<<" std dev: "<<StdDev<<endl;
    if (DiffLargest > DiffSmallest) {
      //cout<<DiffLargest<<":"<<TauTimesStdDev<<endl;
      if (DiffLargest > TauTimesStdDev) {
        OutlierFound = true;
        IsOutlier[Largest] = true;
      } else {
        OutlierFound = false;
      }
    } else {
      if (DiffSmallest > TauTimesStdDev) {
        OutlierFound = true;
        IsOutlier[Smallest] = true;
      } else {
        OutlierFound = false;
      }
    }
  } while (OutlierFound == true && NumberOfValues > 2);
  
  return IsOutlier;
}


// MMath.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
