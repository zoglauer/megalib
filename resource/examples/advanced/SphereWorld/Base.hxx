// Root classes:
#include "TApplication.h"
#include "TRandom.h"
#include "TMath.h"
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib classes:
#include "MStreams.h"
#include "MSimEvent.h"
#include "MBinnerFISBEL.h"
#include "MExceptions.h"
#include "MFile.h"

// Standard
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

  
/******************************************************************************
 * The global parameters
 */
#include "Parameters.h"
  
/******************************************************************************
 * One data cell
 */
class DataCell
{
public:
  //! The default constructor
  DataCell() { m_NEntries = 0; }
  
  //! Set the number of bins
  void SetNBins(unsigned int NBins, unsigned int EnergyBins) {
    m_NBins = NBins;
    m_NEnergyBins = EnergyBins;
  }
  
  void Create(double Energy) {
    
    m_Binner.Create(m_NBins);
    
    m_EnergyBinEdges.resize(m_NEnergyBins+1);
    m_EnergyBinEdges[m_NEnergyBins] = Energy + 0.5;
    m_EnergyBinEdges[m_NEnergyBins-1] = Energy - 0.5;
    
    /*
    double EnergyGap = (Energy - 0.5) / (m_NEnergyBins-1);
    for (unsigned int e = 0; e < m_NEnergyBins-1; ++e) {
      m_EnergyBinEdges[e] = e*EnergyGap;
    }
    */
    
    double Emin = 0;
    double Ethres = 10;
    double Emax = Energy - 0.5;
    double Index = 3;

    Emin = pow(Emin, 1.0/Index);
    Emax = pow(Emax-(m_NEnergyBins-1)*Ethres, 1.0/Index);
  
    double Edist = (Emax - Emin)/(m_NEnergyBins-2);
  
    m_EnergyBinEdges[0];
    for (unsigned int b = 1; b < m_NEnergyBins; ++b) {
      m_EnergyBinEdges[b] = b*Ethres + pow((b-1)*Edist, Index);
    }
    
    for (unsigned int e = 0; e < m_NEnergyBins-1; ++e) {
      cout<<e<<"-"<<m_EnergyBinEdges[e]<<" ";
    }
    cout<<endl;
    
    m_Data.resize(m_NBins*m_NEnergyBins);
  }
  
  //! Set the height
  void SetHeight(double Height, unsigned int HeightID) { m_Height = Height; m_HeightID = HeightID; }
  
  //! Set the distance
  void SetDistance(double Distance, unsigned int DistanceID) { m_Distance = Distance; m_DistanceID = DistanceID; }
  
  //! Set the number of integration spheres
  void SetNIntegrationSpheres(int NIntegratedSpheres) { m_NIntegrationSpheres = NIntegratedSpheres; }
  
  //! Get the number of bins
  unsigned int GetNBins() const { return m_NBins; }
  
  //! Get the data
  const vector<double>& GetDataRef() const { return m_Data; }
  
  //! Add something
  void Add(double Theta, double Phi, double Energy, double Weight = 1) 
  {
    if (Energy == 0) return;
    
    // Add a tiny bit or randomness to evenly distribute hits on bin edges:
    Theta += 0.0001*(gRandom->Rndm() - 0.5);
    Phi += 0.0001*(gRandom->Rndm() - 0.5);

    
    unsigned int iBin = m_Binner.FindBin(Theta, Phi);
    unsigned int eBin = lower_bound(m_EnergyBinEdges.begin(), m_EnergyBinEdges.end(), Energy) - m_EnergyBinEdges.begin() - 1;
    
    if (eBin >= m_NEnergyBins) {
      new MExceptionIndexOutOfBounds(0, m_NEnergyBins, eBin);
      return;
    }
    
    m_Data[iBin + m_NBins * eBin] += Weight;
    m_NEntries += Weight;
  }
  
  //! Add something
  void Add(DataCell& Cell) 
  {
    // Check if the binners are identical
    if (m_NBins != Cell.m_NBins) return;
    if (m_NEnergyBins != Cell.m_NEnergyBins) return;
    if (m_Binner != Cell.m_Binner) return;
    if (m_NIntegrationSpheres != Cell.m_NIntegrationSpheres) return;
    
    for (unsigned int b = 0; b < m_Data.size(); ++b) { 
      m_Data[b] += Cell.m_Data[b];
    }
    m_NEntries += Cell.m_NEntries;
    m_NStartedEvents += Cell.m_NStartedEvents;
      //cout<<"Adding "<<Cell.m_NStartedEvents<<" to "<<m_NStartedEvents<<endl;
  }
  
  //! Show what we have
  void View(double Energy) {
    if (Energy <= m_EnergyBinEdges[0] || Energy >= m_EnergyBinEdges.back()) {
      cout<<"Energy out of bounds: "<<Energy<<endl;
      return;
    }
    unsigned int eBin = lower_bound(m_EnergyBinEdges.begin(), m_EnergyBinEdges.end(), Energy) - m_EnergyBinEdges.begin() - 1;

    vector<double> DataSlice(m_NBins);
    for (unsigned int b = m_NBins*eBin; b < m_NBins*(eBin+1); ++b) {
      DataSlice[b-m_NBins*eBin] = m_Data[b]; 
    }
    // Switch for display
    vector<double> DataSlice2(m_NBins);
    for (unsigned int b = 0; b < m_NBins; ++b) {
      DataSlice2[b] = DataSlice[m_NBins-b-1];
    }
    m_Binner.View(DataSlice2);
    
  
    
    TH1D* EnergyHist = new TH1D("EnergyHistogram", "EnergyHistogram", m_NEnergyBins, &m_EnergyBinEdges[0]);
    EnergyHist->SetStats(false);
    EnergyHist->SetXTitle("Energy [keV]");
    EnergyHist->SetYTitle("cts/keV");
    for (unsigned int e = 0; e < m_NEnergyBins; ++e) {
      for (unsigned int s = 0; s < m_NBins; ++s) {
        EnergyHist->AddBinContent(e + 1, m_Data[s + m_NBins * e]); 
      }
    }
    
    for (unsigned int b = 1; b <= m_NEnergyBins; ++b) {
      EnergyHist->SetBinContent(b, EnergyHist->GetBinContent(b)/EnergyHist->GetBinWidth(b)); 
    }
    
    TCanvas* EnergyCanvas = new TCanvas();
    EnergyCanvas->cd();
    EnergyCanvas->SetLogy();
    EnergyHist->Draw();
    EnergyCanvas->Update();
  }
  
  //! Save the data cell
  MString ToString(long StartedEvents = -1) {
    if (StartedEvents != -1) {
      m_NStartedEvents = StartedEvents;
    }
    
    ostringstream out;
    
    out<<"CubeID "<<m_HeightID<<" "<<m_DistanceID<<endl;
    out<<endl;
    out<<"Height "<<m_Height<<endl;
    out<<"Distance "<<m_Distance<<endl;
    out<<endl;

    out<<"SkyBins "<<m_NBins<<endl;
    out<<"LatitudeBinEdges ";
    vector<double> LatBinEdges = m_Binner.GetLatitudeBinEdges();
    for (unsigned int l = 0; l < LatBinEdges.size(); ++l) {
      out<<LatBinEdges[l]*c_Deg<<" "; 
    }
    out<<endl;
    out<<"LongitudeBins ";
    vector<unsigned int> LongBins = m_Binner.GetLongitudeBins();
    for (unsigned int l = 0; l < LongBins.size(); ++l) {
      out<<LongBins[l]<<" "; 
    }
    out<<endl;
    out<<endl;
    out<<"EnergyBins "<<m_NEnergyBins<<endl;
    out<<"EnergyBinEdges ";
    for (unsigned int l = 0; l < m_EnergyBinEdges.size(); ++l) {
      out<<m_EnergyBinEdges[l]<<" "; 
    }
    out<<endl;
    out<<endl;

    out<<"IntegrationSpheres "<<m_NIntegrationSpheres<<endl;
    out<<"EnteredEvents "<<m_NEntries<<endl;
    out<<"StartedEvents "<<m_NStartedEvents<<endl;
    out<<"Data ";
    for (unsigned int l = 0; l < m_Data.size(); ++l) {
      out<<m_Data[l]<<" "; 
    }
    out<<endl;
    out<<endl;

    return out.str();
  }
  
  //! Save the data cell
  void Save(MString FileName, long StartedEvents = -1) {
    MFile File;
    File.Open(FileName, MFile::c_Write);
    File.Write(ToString(StartedEvents));
    File.Close();
  }
  
  //! Save the data cell
  bool Load(MString FileName) {
    
    MParser Parser;

    if (Parser.Open(FileName, MFile::c_Read) == false) {
      mout<<"Unable to open file "<<FileName<<endl;
      return false;
    }

    vector<double> LatitudeBinEdges;
    vector<unsigned int> LongitudeBins;
    for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
      if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
      if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "Height") == true) {
        m_Height = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "Distance") == true) {
        m_Distance = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "SkyBins") == true) {
        m_NBins = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "EnergyBins") == true) {
        m_NEnergyBins = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "EnergyBinEdges") == true) {
        m_EnergyBinEdges = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "LatitudeBinEdges") == true) {
        LatitudeBinEdges = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "LongitudeBins") == true) {
        LongitudeBins = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedIntVector(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "IntegrationSpheres") == true) {
        m_NIntegrationSpheres = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "EnteredEvents") == true) {
        m_NEntries = Parser.GetTokenizerAt(i)->GetTokenAtAsLong(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "StartedEvents") == true) {
        m_NStartedEvents = Parser.GetTokenizerAt(i)->GetTokenAtAsLong(1);
      } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "Data") == true) {
        m_Data = Parser.GetTokenizerAt(i)->GetTokenAtAsDoubleVector(1);
      }
    }
    for (auto& B: LatitudeBinEdges) B *= c_Rad;
    m_Binner.Set(LongitudeBins, LatitudeBinEdges, m_NBins);
    
    return true;
  }
  
  
private:
  //! The number of bins of the sky
  unsigned int m_NBins;  
  //! The number of energy bins
  unsigned int m_NEnergyBins;
  
  //! The energy bin edges
  vector<double> m_EnergyBinEdges;

  //! The data
  vector<double> m_Data;
  
  //! Number of data entries
  long m_NEntries;
  
  //! The Binner
  MBinnerFISBEL m_Binner;
  
  //! The height
  double m_Height;
  //! The height ID
  unsigned int m_HeightID;
  
  //! The distance
  double m_Distance;
  //! The distance ID
  unsigned int m_DistanceID;
  
  //! Number of spheres contributing to this data cell
  int m_NIntegrationSpheres;
  
  //! The number of events started in the simulation
  long m_NStartedEvents;
};


/******************************************************************************
 *
 */
class DataCube
{
public:
  //! Standard constructor
  DataCube() {

    m_NSkyBins = pow(2, 11);  
    m_NEnergyBins = 30;
    
    m_StartedEvents = 0;
  }
  
  //! Initialize the data cubes
  void Initialize(double Energy) {
    CreateDataCube(Energy);
  }
  
  //! Set the number of started events
  void SetStartedEvents(long StartedEvents) { m_StartedEvents = StartedEvents; }
  
  //! Create the data cubes
  void CreateDataCube(double Energy) {
    m_GapHeight = (cell_MaxHeight - cell_MinHeight)/(cell_NBinsHeight - 1);
    m_GapDistance = (cell_MaxDistance - cell_MinDistance)/(cell_NBinsDistance - 1);
    
    m_DataCells = vector<vector<DataCell>>(cell_NBinsHeight, vector<DataCell>(cell_NBinsDistance));

    m_NIntegrationSpheres.push_back(1);
    for (int d = 1; d < cell_NBinsDistance; ++d) {
      double TorusRadius = d*m_GapDistance;
      double Circumfence = 2* TorusRadius * c_Pi;
      int Spheres = Circumfence / (2*cell_Radius);
      Spheres -= 1; // Safety to avoid micro-overlaps
      if (Spheres <= 0) Spheres = 1;
      m_NIntegrationSpheres.push_back(Spheres);
      //cout<<"Spheres at d="<<d<<": "<<Spheres<<endl;
    }

    
    for (int h = 0; h < cell_NBinsHeight; ++h) {
      for (int d = 0; d < cell_NBinsDistance; ++d) {
        m_DataCells[h][d].SetNBins(m_NSkyBins, m_NEnergyBins);
        m_DataCells[h][d].SetHeight(cell_MinHeight + h*m_GapHeight, h);
        m_DataCells[h][d].SetDistance(cell_MinDistance + d*m_GapDistance, d);
        m_DataCells[h][d].SetNIntegrationSpheres(m_NIntegrationSpheres[d]);
        m_DataCells[h][d].Create(Energy);
      }
    }
  }
  
  
  void Add(MVector Position, MVector Direction, double Energy) {
    
    // Find the torus
    int hBin = int((Position.GetZ() - cell_MinHeight)/m_GapHeight + 0.5);
    double Radius = sqrt(pow(Position.GetX(), 2) + pow(Position.GetY(), 2));
    
    int dBin = int((Radius - cell_MinDistance)/m_GapDistance + 0.5);

    if (hBin < 0 || hBin >= cell_NBinsHeight) {
      //cout<<Position.GetZ()<<":"<<cell_MinHeight<<":"<<m_GapHeight<<endl;
      cout<<"Error: Did not find correct height: bin="<<hBin<<endl;
      return;
    }
    if (dBin < 0 || dBin >= cell_NBinsDistance) {
      cout<<"Error: Did not find correct distance: bin="<<dBin<<endl;
      return;
    }
    

    double Angle = atan2(Position.GetY(), Position.GetX());
    if (Angle < 0) Angle += 2*TMath::Pi();
    
    // Find the sphere
    int aBin = 0;
    double GapAngle = 0;
    if (Radius > cell_Radius && m_NIntegrationSpheres[dBin] > 1) {
      // Normal case
      GapAngle = 2*TMath::Pi() / m_NIntegrationSpheres[dBin];

      aBin = int((Angle - 0)/GapAngle + 0.5);
      if (aBin == m_NIntegrationSpheres[dBin]) aBin = 0;
      
      //cout<<aBin<<":"<<Angle<<":"<<GapAngle<<":"<<m_NIntegrationSpheres[dBin]<<":"<<dBin<<endl;
      
      // Rotate the direction
      Direction.RotateZ(-aBin*GapAngle);
      
    } else {
      // On-axis case ... nothing to rotate
    }
    
    //cout<<"Bins: "<<hBin<<":"<<dBin<<":"<<aBin<<endl;
    
    // Store
    /*
    if (hBin == 0 && dBin == 1) {
        cout<<"Ready to add: "<<Direction.Phi()*c_Deg<<":"<<Direction.Theta()*c_Deg<<":"<<Energy<<" with abin="<<aBin<<" vs. "<<m_NIntegrationSpheres[dBin]<<endl;
        cout<<"Pos: "<<Position<<"  "<<Angle<<":"<<GapAngle<<endl;
    }
    */
    
    if (cell_LineOnly == false || aBin == 0) { 
      /*
      if (hBin == 0 && dBin == 1) {
        cout<<"Adding by energy: "<<Direction.Phi()*c_Deg<<":"<<Direction.Theta()*c_Deg<<":"<<Energy<<endl;
      }
      */
      m_DataCells[hBin][dBin].Add(Direction.Theta(), Direction.Phi(), Energy, 1);
    }
  }
   
  void Save(MString Prefix, bool Combined) {
    if (Combined == true) {
      ostringstream FileName;
      FileName<<Prefix<<".dat.gz";
      
      MFile File;
      File.Open(FileName, MFile::c_Write);
      
      for (int h = 0; h < cell_NBinsHeight; ++h) {
        for (int d = 0; d < cell_NBinsDistance; ++d) {
          File.Write(m_DataCells[h][d].ToString(m_StartedEvents));
        }
      }
      File.Close();

    } else {
      for (int h = 0; h < cell_NBinsHeight; ++h) {
        for (int d = 0; d < cell_NBinsDistance; ++d) {
          ostringstream FileName;
          FileName<<Prefix<<"_Height_"<<h<<"_Distance_"<<d<<".dat.gz";
          m_DataCells[h][d].Save(FileName.str(), m_StartedEvents);
        }
      }
    }
  }
   
  
private:
  double m_GapHeight;
  double m_GapDistance;
    
  unsigned int m_NSkyBins;
  unsigned int m_NEnergyBins;  
  
  vector<vector<DataCell>> m_DataCells;
  
  vector<double> m_NIntegrationSpheres;
  
  long m_StartedEvents;
};


/******************************************************************************
 *
 */
class Processor
{
public:
  Processor() {    
    for (unsigned int i = 0; i < 10; ++i) {
      m_ID += to_string(gRandom->Integer(10));
    }
  }
  ~Processor() {
    Save();
  }
  
  void Initialize(double Energy) {
    m_DataCube.Initialize(Energy); 
  }
  
  void Add(MSimEvent* Event) {
    for (unsigned int i = 0; i < Event->GetNIAs(); ++i) {
      MSimIA* IA = Event->GetIAAt(i);
      if (IA->GetProcess() == "ENTR" && IA->GetMotherParticleID() == 1) {
        //cout<<"Adding: "<<IA->GetMotherEnergy()<<endl;
        m_DataCube.Add(IA->GetPosition(), -IA->GetMotherDirection(), IA->GetMotherEnergy());
        /*
        cout<<-IA->GetMotherDirection().Theta()*c_Deg + 90<<endl;
        if (-IA->GetMotherDirection().Theta()*c_Deg + 90 < -50) {
          cout<<"ID: "<<Event->GetId()<<":"<<-IA->GetMotherDirection().Theta()*c_Deg<<endl;
        }
        */
      }
    }
    m_DataCube.SetStartedEvents(Event->GetSimulationEventID());
    if (Event->GetSimulationEventID() % 500000 == 0) Save();
  }
  
  void Save() {
    ostringstream out;
    out<<"DataCube_ID_"<<m_ID;
    m_DataCube.Save(out.str(), true);     
  }
  
private:
  //! 
  DataCube m_DataCube;
  //! Unique ID 
  MString m_ID;
};
