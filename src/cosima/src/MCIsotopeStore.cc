/*
 * MCIsotopeStore.cxx
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


// Cosima:
#include "MCIsotopeStore.hh"
#include "MCActivatorParticle.hh"

// MEGAlib:
#include "MAssert.h"
#include "MStreams.h"
#include "MTokenizer.h"

// Geant4:
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"

// Standard lib:
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

/******************************************************************************
 * Standard constructor
 */
MCIsotopeStore::MCIsotopeStore() : m_Time(0)
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCIsotopeStore::~MCIsotopeStore()
{
  // Intentionally left blank
}


/******************************************************************************
 * Return the particle definition generated from the Cosima ion ID, zero otherwise
 */
G4ParticleDefinition* MCIsotopeStore::GetParticleDefinition(int IonID, double Excitation)
{
  G4IonTable* IonTable = G4IonTable::GetIonTable();
  int AtomicNumber = int(IonID/1000);
  int AtomicMass = IonID - int(IonID/1000)*1000;
  
  return IonTable->GetIon(AtomicNumber, AtomicMass, Excitation);
}


/******************************************************************************
 *  Clear the store
 */
void MCIsotopeStore::Reset()
{
  m_VolumeNames.clear();
  m_IDs.clear();
  m_Excitations.clear();
  m_Values.clear();
}


/******************************************************************************
 * Return the particle definition generated for the given volume, ID, and excitation
 */
G4ParticleDefinition* MCIsotopeStore::GetParticleDefinition(unsigned int v, unsigned int i, unsigned int e)
{
  massert(v < m_VolumeNames.size());
  massert(i < m_IDs[v].size());
  massert(e < m_Excitations[v][i].size());

  return GetParticleDefinition(m_IDs[v][i], m_Excitations[v][i][e]);
}


/******************************************************************************
 *  Add a new particle to the store
 */
void MCIsotopeStore::Add(MString VolumeName, int NucleusID, double Excitation, double Value)
{
  vector<MString>::iterator VIter;
  if ((VIter = find(m_VolumeNames.begin(), m_VolumeNames.end(), VolumeName)) != m_VolumeNames.end()) {
    unsigned int i = VIter - m_VolumeNames.begin();
    vector<int>::iterator IDIter;
    if ((IDIter = find(m_IDs[i].begin(), m_IDs[i].end(), NucleusID)) != m_IDs[i].end()) {
      unsigned int n = IDIter - m_IDs[i].begin();
      vector<double>::iterator ExcitationIter;
      // The double comparision is ok since we never do any calculations on the value
      if ((ExcitationIter = find(m_Excitations[i][n].begin(), m_Excitations[i][n].end(), Excitation)) != m_Excitations[i][n].end()) {
        unsigned int e = ExcitationIter - m_Excitations[i][n].begin();
        m_Values[i][n][e] += Value;
      } else {
        m_Excitations[i][n].push_back(Excitation);
        m_Values[i][n].push_back(Value);
      }
    } else {
      m_IDs[i].push_back(NucleusID);

      vector<double> Excitations;
      Excitations.push_back(Excitation);
      m_Excitations[i].push_back(Excitations);

      vector<double> Values;
      Values.push_back(Value);
      m_Values[i].push_back(Values);
    }
  } else {
    m_VolumeNames.push_back(VolumeName);

    vector<int> IDs;
    IDs.push_back(NucleusID);
    m_IDs.push_back(IDs);

    vector<vector<double> > IDValues;
    vector<double> Values;
    Values.push_back(Value);
    IDValues.push_back(Values);
    m_Values.push_back(IDValues);

    vector<vector<double> > IDExcitations;
    vector<double> Excitations;
    Excitations.push_back(Excitation);
    IDExcitations.push_back(Excitations);
    m_Excitations.push_back(IDExcitations);
  }
}


/******************************************************************************
 *  Add a new particle to the store
 */
void MCIsotopeStore::Remove(unsigned int v, unsigned int i, unsigned int e)
{
  cout<<"Removing: "<<m_VolumeNames[v]<<" ID: "<<m_IDs[v][i]<<" E="<<m_Excitations[v][i][e]<<"  - "<<m_Excitations[v][i].size()/keV<<endl;
  m_Excitations[v][i].erase(m_Excitations[v][i].begin()+e);
  m_Values[v][i].erase(m_Values[v][i].begin()+e);
  cout<<"Removing: "<<m_VolumeNames[v]<<" ID: "<<m_IDs[v][i]<<" E="<<m_Excitations[v][i][e]<<"  - "<<m_Excitations[v][i].size()/keV<<endl;
}


/******************************************************************************
 *  Save the store to disk
 */
bool MCIsotopeStore::Save(MString FileName)
{
  Sort();

  ofstream out;
  out.open(FileName);

  out<<"# Cosima universal isotope store"<<endl;
  out<<"# VN is followed by the volume name in which the isotope was produced"<<endl;
  out<<"# RP is followed by the isotope ID (1000*Z+A), the excitation in keV, and a value (e.g. the number of produced isotopes, activation in Bq)"<<endl;
  out<<endl;
  out<<"TT "<<m_Time/s<<endl;
  out<<endl;

  out.setf(ios_base::fixed, ios_base::floatfield);
  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    out<<"VN "<<m_VolumeNames[v]<<endl;
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
        out<<"RP "<<m_IDs[v][i]<<"   "<<setprecision(2)<<setw(8)<<m_Excitations[v][i][e]/keV<<"   "<<scientific<<setprecision(5)<<m_Values[v][i][e]<<fixed<<endl;
      }
    }
  }
  out<<endl;
  out<<"EN"<<endl;
  out.close();


//   // MGGPOD counts compatible
//   ofstream out2;
//   out2.open(FileName + ".counts");
//   for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
//     for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
//       for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
//         out2<<"BIGS    "<<int(m_IDs[v][i]) - 1000*int(int(m_IDs[v][i])/1000)<<"    "<<int(m_IDs[v][i])/1000<<" "<<setw(9)<<m_Values[v][i][e]<<"     1"<<endl;
//       }
//     }
//   }
//   out2<<endl;
//   out2<<"EN"<<endl;
//   out2.close();


//   // MGGPOD radi compatible
//   ofstream out3;
//   out3.open(FileName + ".radi");
//   for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
//     for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
//       double ExcitationSum = 0;
//       for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
//         if (m_Excitations[v][i][e] == 0.0) {
//           out3<<"  "<<m_IDs[v][i]<<"0  "<<m_Values[v][i][e]<<endl;
//         } else {
//           if (m_Values[v][i][e] > ExcitationSum) ExcitationSum = m_Values[v][i][e];
//         }
//       }
//       if (ExcitationSum > 0) {
//         out3<<"  "<<m_IDs[v][i]<<"1  "<<ExcitationSum<<endl;        
//       }
//     }
//   }
//   out3<<endl;
//   out3.close();

 return true;
}


/******************************************************************************
 *  Load the store from disk
 */
bool MCIsotopeStore::Load(MString FileName)
{
  Reset();

  ifstream in;
  in.open(FileName);

  MTokenizer Tokenizer;

  int LineLength = 10000;
  char* LineBuffer = new char[LineLength];
  while (in.getline(LineBuffer, LineLength, '\n')) {
    Tokenizer.Analyse(LineBuffer);
    if (Tokenizer.GetNTokens() <= 1) continue; 
    if (Tokenizer.GetTokenAt(0) == "VN") {
      m_VolumeNames.push_back(Tokenizer.GetTokenAtAsString(1));

      vector<int> IDs;
      m_IDs.push_back(IDs);
      
      vector<vector<double> > Values;
      m_Values.push_back(Values);
      
      vector<vector<double> > Excitations;
      m_Excitations.push_back(Excitations);
      
    } else if (Tokenizer.GetTokenAt(0) == "RP") {
      if (m_IDs.size() == 0) continue; // file is corrupt
      if (find(m_IDs.back().begin(), m_IDs.back().end(), Tokenizer.GetTokenAtAsInt(1)) == m_IDs.back().end()) {
        m_IDs.back().push_back(Tokenizer.GetTokenAtAsInt(1));
        vector<double> Excitations;
        m_Excitations.back().push_back(Excitations);
        vector<double> Values;
        m_Values.back().push_back(Values);
      }
      m_Excitations.back().back().push_back(Tokenizer.GetTokenAtAsDouble(2)*keV);
      m_Values.back().back().push_back(Tokenizer.GetTokenAtAsDouble(3));
    } else if (Tokenizer.GetTokenAt(0) == "TT") {
      m_Time = Tokenizer.GetTokenAtAsDouble(1)*s;
    }
  }

  in.close();

  return true;
}



/******************************************************************************
 * Scale (multiply) to content by a certain factor S
 */
void MCIsotopeStore::Scale(double S)
{
  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
        m_Values[v][i][e] *= S;
      }
    }
  }
}


/******************************************************************************
 * Add an IsotopeStore
 */
void MCIsotopeStore::Add(const MCIsotopeStore& RPS)
{
  vector<MString>::iterator NameIter;
  vector<int>::iterator IDIter;
  vector<double>::iterator ExcitationIter;

  for (unsigned int v = 0; v < RPS.m_VolumeNames.size(); ++v) {
    NameIter = find(m_VolumeNames.begin(), m_VolumeNames.end(), RPS.m_VolumeNames[v]);
    if (NameIter == m_VolumeNames.end()) {
      // Simply add the volume and all its content
      m_VolumeNames.push_back(RPS.m_VolumeNames[v]);
      m_IDs.push_back(RPS.m_IDs[v]);
      m_Excitations.push_back(RPS.m_Excitations[v]);
      m_Values.push_back(RPS.m_Values[v]);
    } else {
      // Merge
      unsigned int loc_v = NameIter - m_VolumeNames.begin();
      for (unsigned int i = 0; i < RPS.m_IDs[v].size(); ++i) {
        IDIter = find(m_IDs[loc_v].begin(), m_IDs[loc_v].end(), RPS.m_IDs[v][i]);
        if (IDIter == m_IDs[loc_v].end()) {
          // Add a new ID
          m_IDs[loc_v].push_back(RPS.m_IDs[v][i]);
          // ... along with its expectations and values (the whole vector)
          m_Excitations[loc_v].push_back(RPS.m_Excitations[v][i]);
          m_Values[loc_v].push_back(RPS.m_Values[v][i]);
        } else {
          // Add to the existing ID
          unsigned int loc_i = IDIter - m_IDs[loc_v].begin();
          for (unsigned int e = 0; e < RPS.m_Excitations[v][i].size(); ++e) {
            for (ExcitationIter = m_Excitations[loc_v][loc_i].begin(); ExcitationIter != m_Excitations[loc_v][loc_i].end(); ++ExcitationIter) {
              if (fabs(RPS.m_Excitations[v][i][e] - *ExcitationIter) < 1*keV) {
                // Found it
                break;
              }
            }
            //ExcitationIter = find(m_Excitations[loc_v][loc_i].begin(), m_Excitations[loc_v][loc_i].end(), RPS.m_Excitations[v][i][e]);
            if (ExcitationIter == m_Excitations[loc_v][loc_i].end()) {
              // Add new excitation:
              m_Excitations[loc_v][loc_i].push_back(RPS.m_Excitations[v][i][e]);
              m_Values[loc_v][loc_i].push_back(RPS.m_Values[v][i][e]);
            } else {
              unsigned int loc_e = ExcitationIter - m_Excitations[loc_v][loc_i].begin();
              m_Excitations[loc_v][loc_i][loc_e] = RPS.m_Excitations[v][i][e];
              m_Values[loc_v][loc_i][loc_e] += RPS.m_Values[v][i][e];
            } // new excitations
          } // all Excitations
        } // new IDs
      } // all IDs
    } // new volumes
  } // all volumes

  m_Time += RPS.GetTime();
}


/******************************************************************************
 *  Create a source list assuming the value is an activity in Bq
 */
vector<MCSource*> MCIsotopeStore::CreateSourceListByActivity()
{
  vector<MCSource*> List;

  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
        if (m_Values[v][i][e] > 0) {
          MString Name = m_VolumeNames[v];
          Name += "_";
          Name += m_IDs[v][i];
          Name += "[";
          Name += m_Excitations[v][i][e]/keV;
          Name += "]";
          Name = Name.ReplaceAll(" ", "");
          MCSource* Source = new MCSource(Name);
          Source->SetParticleType(m_IDs[v][i]);
          Source->SetParticleExcitation(m_Excitations[v][i][e]);
          Source->SetSpectralType(MCSource::c_Activation);
          Source->SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldActivation);
          Source->SetVolume(m_VolumeNames[v]);
          Source->SetFlux(m_Values[v][i][e]/s);
          cout<<"Flux: "<<m_Values[v][i][e]<<endl;
          List.push_back(Source);
        }
      }
    }
  }

  return List;
}


/******************************************************************************
 *  Create a source list assuming the value is an isotope 
 */
vector<MCSource*> MCIsotopeStore::CreateSourceListByIsotopeCount()
{
  vector<MCSource*> List;

  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      for (unsigned int e = 0; e < m_Excitations[v][i].size(); ++e) {
        if (m_Values[v][i][e] > 0) {
          MString Name = m_VolumeNames[v];
          Name += "_";
          Name += m_IDs[v][i];
          Name += "[";
          Name += m_Excitations[v][i][e]/keV;
          Name += "]";
          Name = Name.ReplaceAll(" ", "");
          MCSource* Source = new MCSource(Name);
          bool Return = true; 
          Return &= Source->SetParticleType(m_IDs[v][i]);
          Return &= Source->SetParticleExcitation(m_Excitations[v][i][e]);
          Return &= Source->SetSpectralType(MCSource::c_Activation);
          Return &= Source->SetBeamType(MCSource::c_NearField, MCSource::c_NearFieldActivation);
          Return &= Source->SetVolume(m_VolumeNames[v]);
          Return &= Source->SetIsotopeCount(m_Values[v][i][e]);
          if (Return == true) {
            List.push_back(Source);
          } else {
            cout<<"An error occurred during source generation. Ignoring particle "<<m_IDs[v][i]<<", "<<m_Excitations[v][i][e]/keV<<" keV"<<endl;
          }
        }
      }
    }
  }

  return List;
}


/******************************************************************************
 * Return the number of volumes:
 */
unsigned int MCIsotopeStore::GetNVolumes() const
{
  return m_VolumeNames.size();
}


/******************************************************************************
 * Return the number of IDs for a given volume:
 */
unsigned int MCIsotopeStore::GetNIDs(unsigned int v) const
{
  massert(v < m_VolumeNames.size());
  return m_IDs[v].size();
}


/******************************************************************************
 * Return the number of IDs for a given volume:
 */
unsigned int MCIsotopeStore::GetNExcitations(unsigned int v, unsigned int i) const
{
  massert(v < m_VolumeNames.size());
  massert(i < m_IDs[v].size());
  return m_Excitations[v][i].size();
}


/******************************************************************************
 * Return a volume name:
 */
MString MCIsotopeStore::GetVolume(unsigned int v) const
{
  massert(v < m_VolumeNames.size());
  return m_VolumeNames[v];
}


/******************************************************************************
 * Return an ID for a given volume:
 */  
int MCIsotopeStore::GetID(unsigned int v, unsigned int i) const
{
  massert(v < m_VolumeNames.size());
  massert(i < m_IDs[v].size());
  return m_IDs[v][i];
}


/******************************************************************************
 * Return an excitation for a given volume, and ID:
 */  
double MCIsotopeStore::GetExcitation(unsigned int v, unsigned int i, unsigned int e) const
{
  massert(v < m_VolumeNames.size());
  massert(i < m_IDs[v].size());
  massert(e < m_Excitations[v][i].size());
  return m_Excitations[v][i][e];
}


/******************************************************************************
 * Return the value for a given volume:
 */  
double MCIsotopeStore::GetValue(unsigned int v, unsigned int i, unsigned int e) const
{
  massert(v < m_VolumeNames.size());
  massert(i < m_Values[v].size());
  massert(e < m_Excitations[v][i].size());
  return m_Values[v][i][e];
}


/******************************************************************************
 * Sort:
 */  
void MCIsotopeStore::Sort() 
{
  // "Insert sort" algorithm:
  cout<<"Sorting generated isotopes for speed up of search..."<<endl;

  // First sort according to volume names
  for (unsigned int v = 1; v < m_VolumeNames.size(); ++v) {
    if (m_VolumeNames[v] < m_VolumeNames[v-1]) {
      // Find the correct position:
      for (unsigned int w = 0; w < v; ++w) {
        if (m_VolumeNames[v] < m_VolumeNames[w]) {
          // We know we have to insert it before position w:
          m_VolumeNames.insert(m_VolumeNames.begin()+w, m_VolumeNames[v]);
          m_VolumeNames.erase(m_VolumeNames.begin()+v+1);
          
          m_IDs.insert(m_IDs.begin()+w, m_IDs[v]);
          m_IDs.erase(m_IDs.begin()+v+1);

          m_Excitations.insert(m_Excitations.begin()+w, m_Excitations[v]);
          m_Excitations.erase(m_Excitations.begin()+v+1);
          
          m_Values.insert(m_Values.begin()+w, m_Values[v]);
          m_Values.erase(m_Values.begin()+v+1);
        }
      }
    }
  }

  // The sort according to IDs
  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 1; i < m_IDs[v].size(); ++i) {
      if (m_IDs[v][i] < m_IDs[v][i-1]) {
        for (unsigned int j = 0; j < i; ++j) {
          if (m_IDs[v][i] < m_IDs[v][j]) {
            // We know we have to insert it before position j:
            
            m_IDs[v].insert(m_IDs[v].begin()+j, m_IDs[v][i]);
            m_IDs[v].erase(m_IDs[v].begin()+i+1);
            
            m_Excitations[v].insert(m_Excitations[v].begin()+j, m_Excitations[v][i]);
            m_Excitations[v].erase(m_Excitations[v].begin()+i+1);
            
            m_Values[v].insert(m_Values[v].begin()+j, m_Values[v][i]);
            m_Values[v].erase(m_Values[v].begin()+i+1);
          }
        }
      }
    }
  }

  // The sort according to excitations (large to small)
  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      for (unsigned int e = 1; e < m_Excitations[v][i].size(); ++e) {
        if (m_Excitations[v][i][e] > m_Excitations[v][i][e-1]) {
          for (unsigned int f = 0; f < e; ++f) {
            if (m_Excitations[v][i][e] > m_Excitations[v][i][f]) {
              // We know we have to insert it before position f:
              m_Excitations[v][i].insert(m_Excitations[v][i].begin()+f, m_Excitations[v][i][e]);
              m_Excitations[v][i].erase(m_Excitations[v][i].begin()+e+1);
              
              m_Values[v][i].insert(m_Values[v][i].begin()+f, m_Values[v][i][e]);
              m_Values[v][i].erase(m_Values[v][i].begin()+e+1);
            }
          }
        }
      }
    }
  }
}


/******************************************************************************
 * Remove all stable elements:
 */  
void MCIsotopeStore::RemoveStableElements() 
{
  // Remove excitations and values
  for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
    for (unsigned int i = 0; i < m_IDs[v].size(); ++i) {
      vector<double>::iterator E = m_Excitations[v][i].begin();
      vector<double>::iterator A = m_Values[v][i].begin();
      while (E != m_Excitations[v][i].end() && A != m_Values[v][i].end()) {
        cout<<"Element: "<<GetParticleDefinition(m_IDs[v][i], (*E))->GetParticleName()<<endl;
        if (MCActivatorParticle::IsStable(GetParticleDefinition(m_IDs[v][i], (*E))) == true) {
          cout<<"Removing stable element: "<<GetParticleDefinition(m_IDs[v][i], (*E))->GetParticleName()<<endl;
          E = m_Excitations[v][i].erase(E);
          A = m_Values[v][i].erase(A);
          continue;
        }
        ++E;
        ++A;
      }
    }
  }

//   // Remove empty IDs
//   for (unsigned int v = 0; v < m_VolumeNames.size(); ++v) {
//     vector<int>::iterator I = m_IDs[v].begin();
//     vector<vector<double> >::iterator E = m_Excitations[v].begin();
//     vector<vector<double> >::iterator A = m_Values[v].begin();
//     while (I != m_IDs[v].end() && E != m_Excitations[v].end() && A != m_Values[v].end()) {
//       if ((*E).size() == 0) {
//         I = m_IDs[v].erase(I);
//         E = m_Excitations[v].erase(E);
//         A = m_Values[v].erase(A);
//         continue;
//       }
//       ++I;
//       ++E;
//       ++A;
//     }
//   }
}


/******************************************************************************
 * The global dumping operator:
 */  
std::ostream& operator<<(std::ostream& os, const MCIsotopeStore& S)
{
  os<<"Content of the particle store (depending on normalization either by counts OR by rate is correct, not both): "<<endl;

  
  for (unsigned int v = 0; v < S.GetNVolumes(); ++v) {
    os<<"Volume: "<<S.GetVolume(v)<<endl;
    double Sum = 0.0;
    for (unsigned int i = 0; i < S.GetNIDs(v); ++i) {
      for (unsigned int e = 0; e < S.GetNExcitations(v, i); ++e) {
        os<<"  Isotope: "<<S.GetID(v, i)<<" ("<<S.GetExcitation(v, i, e)/keV<<" keV) - Value: As counts: "<<S.GetValue(v, i, e)<<" cts or as rate: "<<S.GetValue(v, i, e)*s<<" cts/sec"<<endl;
        Sum += S.GetValue(v, i, e);
      }
    }
    os<<"  --> Sum: As counts: "<<Sum<<" cts or as rate: "<<Sum*s<<" cts/sec"<<endl;
  }

  return os;
}


/*
 * MCIsotopeStore.cc: the end...
 ******************************************************************************/
