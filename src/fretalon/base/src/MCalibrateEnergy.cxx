/*
 * MCalibrateEnergy.cxx
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


// Include the header:
#include "MCalibrateEnergy.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MReadOutDataADCValue.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrateEnergy)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateEnergy::MCalibrateEnergy() : MCalibrate(), m_RangeMinimum(0), m_RangeMaximum(1000)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrateEnergy::~MCalibrateEnergy()
{
}


////////////////////////////////////////////////////////////////////////////////


//!  Sort two ADC values increasing
bool SortADCValuesIncreasing(MReadOutDataADCValue* A, MReadOutDataADCValue* B) 
{ 
  return (A->GetADCValue() < B->GetADCValue()); 
}

//! Create a histogram from the read-out data group using a fixed number of counts per bin
TH1D* MCalibrateEnergy::CreateHistogramCountsBinned(MReadOutDataGroup& G, double Min, double Max, int Counts, double MinBinWidth)
{
  // Make local copy of the read-out elements representing ADC values
  vector<MReadOutDataADCValue*> Data;
  for (unsigned int d = 0; d < G.GetNumberOfReadOutDatas(); ++d) {
    MReadOutDataADCValue* ADC = dynamic_cast<MReadOutDataADCValue*>(G.GetReadOutData(d).Get(MReadOutDataADCValue::m_TypeID));
    if (ADC != nullptr) {
      Data.push_back(ADC);
    }
  }

  // Step 1: Make sure the data is sorted increasingsly
  sort(Data.begin(), Data.end(), SortADCValuesIncreasing);
    
  // Step 3: Create the bins for the histogram:
  vector<double> BinEdges;
  BinEdges.push_back(Min);
  int LocalCounts = Counts;
  double Value;
  double LastValue = -numeric_limits<double>::max();
  for (unsigned int d = 0; d < Data.size(); ++d) {
    Value = Data[d]->GetADCValue();
    if (Value < Min || Value > Max) continue;
    if (Value < BinEdges.back()) continue; // MinBinWidth jumps forward, so make sure we have catched up
                          
    LocalCounts--;
    if (Value != LastValue && LocalCounts <= 0) {
      LocalCounts = Counts;
      if (Value > BinEdges.back() + MinBinWidth) {
        BinEdges.push_back(Value);
      } else {
        BinEdges.push_back(BinEdges.back() + MinBinWidth);
      }
    }
    LastValue = Value;
  }
  BinEdges.push_back(Max);
    
  for (unsigned int i = 0; i < BinEdges.size(); ++i)  cout<<BinEdges[i]<<endl;
    
  // Step 4: Create the histogram
  TH1D* Histogram = new TH1D("", "", BinEdges.size()-1, &BinEdges[0]);
  Histogram->SetBit(kCanDelete);  
  Histogram->SetXTitle("Read-out values");
  Histogram->SetYTitle("counts per read-out value");
  for (unsigned int d = 0; d < Data.size(); ++d) {
    if (Data[d]->GetADCValue() < Min || Data[d]->GetADCValue() > Max) continue;
    Histogram->Fill(Data[d]->GetADCValue());
  }
    
  // Step 5: Normalize
  for (int b = 1; b <= Histogram->GetXaxis()->GetNbins(); ++b) {
    if (Histogram->GetBinWidth(b) > 0) {
      Histogram->SetBinContent(b, Histogram->GetBinContent(b)/Histogram->GetBinWidth(b));
    } else {
      Histogram->SetBinContent(b, 0);
    }
  }
  
  return Histogram;
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out data group and the associated isotopes
void MCalibrateEnergy::AddReadOutDataGroup(const MReadOutDataGroup& ROG, const vector<MIsotope>& Isotopes)
{
  m_ROGs.push_back(ROG);
  m_Isotopes.push_back(Isotopes);
}


// MCalibrateEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
