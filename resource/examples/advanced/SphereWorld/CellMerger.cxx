#include "Base.hxx"

#include <fstream>
#include <cstdint>
#include <iostream>
using namespace std;


int main(int argc, char** argv) {
  
  vector<MString> FileNames;
  
  if (argc > 1) {
    FileNames.assign(argv + 1, argv + argc);
  }

  for (auto s: FileNames) {
    cout<<s<<endl; 
  }
  cout<<" --> "<<FileNames.size()<<" files."<<endl;
  
  unsigned int m_NBinsHeight = 0;
  unsigned int m_NBinsDistance = 0;
  
  for (auto Name: FileNames) {
    if (Name.Contains("merged")) continue;
    
    unsigned int h = 0;
    unsigned int d = 0;
    
    vector<MString> Tokens = Name.Tokenize("_");
    h = Tokens[4].ToInt();
    d = Tokens[6].GetSubString(0, Tokens[6].First('.')).ToInt();
   
    if (h+1 > m_NBinsHeight) m_NBinsHeight = h+1;
    if (d+1 > m_NBinsDistance) m_NBinsDistance = d+1;
  }
  
  cout<<"Bins height: "<<m_NBinsHeight<<endl;
  cout<<"Bins distance: "<<m_NBinsDistance<<endl;
  
  vector<vector<vector<MString>>> SortedFileNames(m_NBinsHeight, vector<vector<MString>>(m_NBinsDistance, vector<MString>()));

  for (auto Name: FileNames) {
    if (Name.Contains("merged")) continue;
    
    int h = 0;
    int d = 0;
    
    vector<MString> Tokens = Name.Tokenize("_");
    h = Tokens[4].ToInt();
    d = Tokens[6].GetSubString(0, Tokens[6].First('.')).ToInt();

    SortedFileNames[h][d].push_back(Name);
  }

  for (unsigned int h = 0; h < m_NBinsHeight; ++h) {
    for (unsigned int d = 0; d < m_NBinsDistance; ++d) {
      cout<<"Files in cell "<<h<<" "<<d<<": "<<SortedFileNames[h][d].size()<<endl;
      if (SortedFileNames[h][d].size() == 0) {
        cout<<"Hmmmm, we have no files for height "<<h<<" and distance "<<d<<endl;
        continue;
      }
      DataCell Merged;
      Merged.Load(SortedFileNames[h][d][0]);
      
      for (unsigned int f = 1; f < SortedFileNames[h][d].size(); ++f) {
        DataCell New;
        New.Load(SortedFileNames[h][d][f]);
        Merged.Add(New); 
      }
 
      MString NewFileName = SortedFileNames[h][d][0];
      vector<MString> Tokens = NewFileName.Tokenize("_");
      MString ID = Tokens[2];
      NewFileName.ReplaceAllInPlace(ID, "merged");
 
      Merged.Save(NewFileName);
    }
  }
  
  return 0;
}


