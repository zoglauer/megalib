/*
 * MImageUpdate.cxx
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
// MImageUpdate
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MImageUpdate.h"

// Standard libs:
#include <iostream>
#include <sstream>
using namespace std;

// ROOT libs:
#include <TPaveStats.h>

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MImageUpdate)
#endif


////////////////////////////////////////////////////////////////////////////////


MImageUpdate::MImageUpdate()
{
  // Construct an instance of MImageUpdate
}


////////////////////////////////////////////////////////////////////////////////


MImageUpdate::MImageUpdate(MString Title, double *IA, int NEntries, 
                           MString xTitle, double xMin, double xMax, int xNBins, 
                           unsigned int Mode, unsigned int ShortStorage, unsigned int LongStorage, 
                           int AdditionalText, int Spectrum, int DrawOption) :
  MImage(Title, IA, NEntries, xTitle, xMin, xMax, xNBins, Spectrum, DrawOption)
{
  //

  m_NAdds = 0;
  m_DisplayUpdateFrequency = 10;

  m_AdditionalTextType = 0;
  m_AdditionalText = 0;

  SetMaximumStorage(ShortStorage, LongStorage);
  SetDisplayMode(Mode);
  AddAdditionalText(AdditionalText);

  m_FAC = new double[m_xNBins];
  m_MAC = new double[m_xNBins];

  int x;
  for (x = 0; x < m_xNBins; x++) {
    m_FAC[x] = 0;
    m_MAC[x] = 0;
  }

  m_Canvas = 0;
  m_AAH = 0;
  m_FAH = 0;
  m_MAH = 0;

}


////////////////////////////////////////////////////////////////////////////////


MImageUpdate::~MImageUpdate()
{
  // Delete this instance of MImageUpdate
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::Add(double x, double Value)
{
  // Add a bin entry and update the display when necessary

  if (IsCreated() == false) {
    Warning("MImageUpdate::Add",
            "The histograms are not created!");
    return;
  }


  unsigned int Storage = 0;
  int Bin; 
  if (m_DisplayMode == c_History) {
    Bin = 0;
  } else {
    Bin = (int) ((x-m_xMin)/(m_xMax-m_xMin)*m_xNBins);
  }

  if (Bin >= 0 && Bin < m_xNBins) {

    // We have an add
    m_NAdds++;

    if (m_DisplayMode == c_History) {
      // Only fill the last bin:
      if (Value == 0) {
        Value = 0.0000000000000001; // verbessern!
      }
      
      // If all Bins are already filled:
      // shift left
      for (Bin = 0; Bin < m_xNBins-1; Bin++) {
        m_IA[Bin] = m_IA[Bin+1];
      }
      m_IA[m_xNBins-1] = Value;
    } 
    // All other modes except c_History
    else if (m_DisplayMode == c_Average) {
      m_IA[Bin] = (m_IA[Bin]*m_NAdds + Value)/(m_NAdds+1);
      m_FAC[Bin] = (m_FAC[Bin]*m_NAdds + Value)/(m_NAdds+1);
      m_MAC[Bin] = (m_MAC[Bin]*m_NAdds + Value)/(m_NAdds+1);
      m_Storage[0][Bin] = (m_Storage[0][Bin]*m_NAdds + Value)/(m_NAdds+1);
    }
    else {
      m_IA[Bin] += Value;
      m_FAC[Bin] += Value;
      m_MAC[Bin] += Value;
      m_Storage[0][Bin] += Value;
    }

    // Update the display:
    if (m_NAdds % m_DisplayUpdateFrequency == 0) {

      // All Adds Chart (AAC):
      double Sum = 0;
      
      if (m_DisplayMode == c_Normalize) { 
        for (Bin = 0; Bin < m_xNBins; Bin++) {
          Sum += m_IA[Bin];
        }
        if (Sum > 0) { 
          Sum = 1.0/Sum;
        } else {
          Sum = 1.0;
        }
      } // Modes c_History and c_Accumulate 
      else {
        Sum = 1.0;
      }

      for (Bin = 0; Bin < m_xNBins; Bin++) {
        m_AAH->SetBinContent(Bin, m_IA[Bin]*Sum);
      }
      
      // Few Adds Chart (FAC):
      if (m_DisplayMode != c_History) { 
        Sum = 0;
        if (m_DisplayMode == c_Normalize) { 
          for (Bin = 0; Bin < m_xNBins; Bin++) {
            Sum += m_FAC[Bin];
          }
          if (Sum > 0) { 
            Sum = 1.0/Sum;
          } else {
            Sum = 1.0;
          }
        } // Mode c_Accumulate
        else {
          Sum = 1.0;
        }

        for (Bin = 0; Bin < m_xNBins; Bin++) {
          m_FAH->SetBinContent(Bin, m_FAC[Bin]*Sum);
        }
      } // (m_DisplayMode != c_History)

      m_Canvas->cd();
      if (m_AdditionalTextType != 0 && m_AdditionalText != 0) {
        ostringstream Text;
        switch (m_AdditionalTextType) {
        case c_RMS:
          Text<<"R "<<m_AAH->GetRMS()<<endl;
          DisplayText(0.15, 0.15, Text.str().c_str()); 
          break;
        case c_Mean:
          Text<<"M "<<m_AAH->GetMean()<<endl;
          DisplayText(0.15, 0.15, Text.str().c_str()); 
          break;
        case c_Last:
          Text<<"L "<<Value<<endl;
          DisplayText(0.15, 0.15, Text.str().c_str()); 
          break;
          // no default!
        }
      }

      m_Canvas->Modified();
      m_Canvas->Update();
      gSystem->ProcessEvents();
    }


    // Check if its time for updating the storage:

    // Don't store in History mode:
    if (m_DisplayMode != c_History) {
      if (m_NAdds % m_StorageUpdateFrequency == 0) {
        
        // Make room for the new storage and remove the old one:
        delete [] m_Storage[m_NMACStorages-1];
        for (Bin = m_NMACStorages-1; Bin > 0; Bin--) {
          m_Storage[Bin] = m_Storage[Bin-1];
        }
        m_Storage[0] = new double[m_xNBins];
        for (Bin = 0; Bin < m_xNBins; Bin++) {
          m_Storage[0][Bin] = 0.0;
        }
        
        // Now update the short graph:
        for (Bin = 0; Bin < m_xNBins; Bin++) {
          m_FAC[Bin] = 0.0;
        }
        for (Storage = 1; Storage < 1+m_NFACStorages; Storage++) { 
          for (Bin = 0; Bin < m_xNBins; Bin++) {
            m_FAC[Bin] += m_Storage[Storage][Bin];
          }
        }
      }
    } // (m_DisplayMode != c_History)

  } // Add a data-point

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::Add(double *Image)
{

  if (IsCreated() == false) {
    Warning("MImageUpdate::Add",
            "The histograms are not created!");
    return;
  }

  int Bin;

  // Update all:
  if (m_DisplayMode == c_Average) {
    for (Bin = 0; Bin < m_xNBins; Bin++) {
      m_IA[Bin] = (m_IA[Bin]*m_NAdds + Image[Bin])/(m_NAdds+1);
      m_FAC[Bin] = (m_FAC[Bin]*m_NAdds + Image[Bin])/(m_NAdds+1);
      m_MAC[Bin] = (m_MAC[Bin]*m_NAdds + Image[Bin])/(m_NAdds+1);
      m_Storage[0][Bin] = (m_Storage[0][Bin]*m_NAdds + Image[Bin])/(m_NAdds+1);
    }
  } else {
    for (Bin = 0; Bin < m_xNBins; Bin++) {
      m_IA[Bin] += Image[Bin];
    }
  }

  if (m_NAdds % m_DisplayUpdateFrequency == 0) {
      
    // All Adds Chart (AAC):
    double Sum = 0;
    

    // Mode:
    switch(m_DisplayMode) {
    case c_Accumulate: // Summed
      for (Bin = 0; Bin < m_xNBins; Bin++) {
        Sum += m_IA[Bin];
      }
      Sum = 1.0;
      break;
    case c_Normalize: // Normalized to 1
      for (Bin = 0; Bin < m_xNBins; Bin++) {
        Sum += m_IA[Bin];
      }
      (Sum > 0) ? Sum = 1.0/Sum : Sum = 1.0;
      break;
    case c_Average: // Averaged
      Sum = 1.0;
      break;
    default:
      Sum = 1.0;
      break;
    }
    
    for (Bin = 0; Bin < m_xNBins; Bin++) {
      m_AAH->SetBinContent(Bin, m_IA[Bin]*Sum);
    }
        
    // Few Adds Chart (FAC):
    //Sum = 0;
    //for (Bin = 0; Bin < m_xNBins; Bin++) {
    //Sum += m_FAC[Bin];
    //}
    //Sum = 1.0/Sum;
    //for (Bin = 0; Bin < m_xNBins; Bin++) {
    //m_FAH->SetBinContent(Bin, m_FAC[Bin]*Sum);
    //}
    
    m_Canvas->cd();
    if (m_AdditionalTextType != 0 && m_AdditionalText != 0) {
      ostringstream Text;
      switch (m_AdditionalTextType) {
      case c_RMS:
        //Text<<"R "<<m_AAH->GetRMS()<<endl;
        Text<<"R "<<StandardDeviation()<<endl;
        //cout<<m_AAH->GetRMS()<<"!"<<m_AAH->GetMean()<<endl;
        DisplayText(0.15, 0.2, Text.str().c_str()); 
        break;
      case c_Mean:
        //Text<<"M "<<m_AAH->GetMean()<<endl;
        Text<<"M "<<Mean()<<endl;
        DisplayText(0.15, 0.2, Text.str().c_str()); 
        break;
        // no default!
      }
    }
    
    
    m_Canvas->Modified();
    m_Canvas->Update();
    gSystem->ProcessEvents();
  }
  
  /*
  // Check if its time for updating the storage:
    if (m_NAdds % m_StorageUpdateFrequency == 0) {
    
    // Make room for the new storage and remove the old one:
    delete [] m_Storage[m_NMACStorages-1];
    for (Bin = m_NMACStorages-1; Bin > 0; Bin--) {
    m_Storage[Bin] = m_Storage[Bin-1];
    }
    m_Storage[0] = new double[m_xNBins];
    for (Bin = 0; Bin < m_xNBins; Bin++) {
    m_Storage[0][Bin] = 0.0;
    }
    
    // Now update the short graph:
    for (Bin = 0; Bin < m_xNBins; Bin++) {
    m_FAC[Bin] = 0.0;
    }
    for (Storage = 1; Storage < 1+m_NFACStorages; Storage++) { 
    for (Bin = 0; Bin < m_xNBins; Bin++) {
    m_FAC[Bin] += m_Storage[Storage][Bin];
    }
    }
    }
  */
}


////////////////////////////////////////////////////////////////////////////////


double MImageUpdate::Mean()
{
  //

  int i;
  double Mean = 0;
  for (i = 0; i < m_xNBins; i++) {
    Mean += m_IA[i];
  } 
  
  return Mean / m_xNBins;
}


////////////////////////////////////////////////////////////////////////////////


double MImageUpdate::StandardDeviation()
{
  //

  int i;
  double mean = Mean();
  double RMS = 0;
  for (i = 0; i < m_xNBins; i++) {
    RMS = (mean - m_IA[i]) * (mean - m_IA[i]);
  } 
  
  return sqrt(RMS);
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::AddAdditionalText(int Type)
{
  // If this option is set, some additional text is displayed above the
  // histogram
  
  m_AdditionalTextType = Type;
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::SetDisplayMode(unsigned int Mode)
{
  // Mode, how the y-value of the data is displayed:
  // 0: Summed, 1: Normalized, 2: Averaged, 3: History

  m_DisplayMode = Mode;

  if (m_DisplayMode == c_History) {
    m_DisplayUpdateFrequency = 1;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::SetMaximumStorage(unsigned int ShortStorage, unsigned int LongStorage)
{
  //
  //
  // ShortStorage: Number of updates for the short term curve
  // LongStorage: Number of updates for the short term curve

  m_NAddsFAC = ShortStorage;
  m_NAddsMAC = LongStorage;

  m_NFACStorages = 10; 
  
  m_StorageUpdateFrequency = (int) ((double) m_NAddsFAC/m_NFACStorages);
  if (m_StorageUpdateFrequency == 0) m_StorageUpdateFrequency = 1;
  m_NMACStorages = m_NAddsMAC/m_StorageUpdateFrequency;

  m_Storage = new double*[m_NMACStorages];
  unsigned int i;
  int j;
  for (i = 0; i < m_NMACStorages; i++) {
    m_Storage[i] = new double[m_xNBins];
    for (j = 0; j < m_xNBins; j++) {
      m_Storage[i][j] = 0.0;
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::DisplayText(double x, double y, MString Text)
{
  //

  delete m_AdditionalText;
  m_AdditionalText = new TText(x, y, Text);
  m_AdditionalText->SetNDC(true);
  m_AdditionalText->SetTextFont(32);
  m_AdditionalText->SetTextColor(1);
  m_AdditionalText->SetTextSize(0.15);
  m_AdditionalText->SetTextAlign(12);
  m_AdditionalText->Draw();
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::Display(TCanvas *Canvas)
{
  // Display the image in a canvas

  int x;

  // Individualize the canvas:
  if (m_Canvas != 0) delete m_Canvas;
  if (Canvas == 0) {
    m_Canvas = new TCanvas();
  } else {
    m_Canvas = Canvas;
  }

   // Name of the canvas
  m_Canvas->SetName(MakeCanvasTitle());
  
  m_Canvas->SetTitle(m_Title);
  m_Canvas->SetWindowPosition(40, 40);
  m_Canvas->SetWindowSize(640, 480);
  m_Canvas->SetFillColor(0);
  m_Canvas->SetFrameBorderSize(0);
  m_Canvas->SetFrameBorderMode(0);
  m_Canvas->SetBorderSize(0);
  m_Canvas->SetBorderMode(0);


  // Fill the all-data histogram
  if (m_AAH != 0) delete m_AAH;
  m_AAH = new TH1D(m_Title, m_Title, m_xNBins, m_xMin, m_xMax);
  
  m_AAH->SetDirectory(0);
  m_AAH->SetXTitle(m_xTitle);
  //m_AAH->SetYTitle(m_yTitle);
  m_AAH->SetFillColor(0);
  m_AAH->SetLineColor(38);
  //m_AAH->SetLineWidth(6);
  //m_AAH->SetLabelOffset(20, "Y");
  m_AAH->SetTitleOffset(1.1, "X");
  m_AAH->SetTitleOffset(1.3, "Y");
  m_AAH->SetLabelSize(0.03, "X");
  m_AAH->SetLabelSize(0.03, "Y");
  //m_AAH->SetLabelColor(0, "Y");
  m_AAH->SetTickLength(0.00, "Y");
  m_AAH->SetStats(false);
  m_AAH->SetMinimum(0);

  for (x = 0; x < m_xNBins; x++) {
    m_AAH->Fill((x+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin, m_IA[x]);
  }
  
  // Fill the long-term-data histogram:


  // Fill the short term data histogram:
  if (m_FAH != 0) delete m_FAH;
  m_FAH = new TH1D(m_Title, m_Title, m_xNBins, m_xMin, m_xMax);
  
  m_FAH->SetDirectory(0);
  m_FAH->SetXTitle(m_xTitle);
  //m_FAH->SetYTitle(m_yTitle);
  m_FAH->SetFillColor(0);
  m_FAH->SetLineColor(48);
  //m_FAH->SetLineWidth(6);
  m_FAH->SetLabelOffset(-0.003, "Y");
  m_FAH->SetLabelOffset(10, "Y");
  m_FAH->SetTitleOffset(1.1, "X");
  m_FAH->SetTitleOffset(1.3, "Y");
  m_FAH->SetLabelSize(0.03, "X");
  m_FAH->SetLabelSize(0.00, "Y");
  m_FAH->SetTickLength(0.00, "Y");
  m_FAH->SetStats(false);

  for (x = 0; x < m_xNBins; x++) {
    m_FAH->Fill((x+0.5) * (m_xMax-m_xMin)/m_xNBins + m_xMin, m_FAC[x]);
  }


  if (m_AdditionalTextType != 0) {
    DisplayText(0.2, 0.2, "bla");
  }
  m_AAH->Draw("");
  //m_AAH->SetMinimum(0);
  //m_AAH->SetMaximum(5);
  m_FAH->Draw("same");

  
  m_Canvas->Modified();
  m_Canvas->Update();

  gSystem->ProcessEvents();

  SetCreated();

  return;
}

////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::DrawCopy()
{
  // 

  if (m_AAH != 0) {
    m_AAH->DrawCopy("");
  }
 
  if (m_FAH != 0) {
    m_FAH->Draw("same");
  }
}


////////////////////////////////////////////////////////////////////////////////


void MImageUpdate::Reset()
{
  int x;
  for (x = 0; x < m_xNBins; x++) {
    m_IA[x] = 0.0;
    m_FAC[x] = 0.0;
    m_AAH->SetBinContent(x, m_IA[x]);
    m_FAH->SetBinContent(x, m_FAC[x]);
  }
  m_NAdds = 0;


  m_Canvas->Modified();
  m_Canvas->Update();
  gSystem->ProcessEvents();
}


// MImageUpdate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
