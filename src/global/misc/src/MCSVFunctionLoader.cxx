/*
 * MCVSFunctionLoader.cxx
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


#include "MCSVFunctionLoader.h"

#ifdef ___CINT___
ClassImp(MGUIInfo)
#endif



//////////////////////////////////////////////////////////////////////////////////
//
//  ctor
//
MCSVFunctionLoader::MCSVFunctionLoader()
{

}




//////////////////////////////////////////////////////////////////////////////////
//
//  ctor
//
MCSVFunctionLoader::MCSVFunctionLoader(MString Filename, MString Keyword)
{
  //cout<<"MCSVFunctionLoader!"<<endl;

  SetFilename(Filename);
  SetKeyword(Keyword);
  Load();
}



//////////////////////////////////////////////////////////////////////////////////
//
// dtor
//
MCSVFunctionLoader::~MCSVFunctionLoader()
{

}



//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MCSVFunctionLoader::SetFilename(MString Filename)
{
  m_Filename = Filename;
}



//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MCSVFunctionLoader::SetKeyword(MString Keyword)
{
  m_Keyword = MString("[") + Keyword + MString("]:");
}



//////////////////////////////////////////////////////////////////////////////////
//
// 
//
bool MCSVFunctionLoader::Load()
{
  // Step one: Open the file:
  FILE *File;
  if ((File = fopen(m_Filename, "r")) == NULL) {
    cout<<"MCSVFunctionLoader::Load()"<<endl;
    cout<<"Can't open file m_Filename with key word m_Keyword!"<<endl;
    return false;
  }

  // Now search for the different cross sections:
  int LineLength = 200;
  char *LineBuffer = new char[LineLength];


  // Step two: Count the number of different detectors: 
  m_NEntries = 0;
  while (fgets(LineBuffer, LineLength, File) != NULL)
    if (strstr(LineBuffer, m_Keyword) != NULL)
      m_NEntries++;

  rewind(File);

  m_X = new double[m_NEntries];
  m_Y = new double[m_NEntries];

  int counter = 0;
  char *FormatString = new char[strlen(m_Filename) + 20];
  FormatString[0] = '\0';
  strcat(FormatString, m_Keyword);
  strcat(FormatString, "%lf;%lf\n");


  // Now load the values:
  while(fgets(LineBuffer, LineLength, File) != NULL)
    if(sscanf(LineBuffer, FormatString, 
	      &m_X[counter], 
	      &m_Y[counter]) == 2)
      counter++;
 
  fclose(File);
  delete [] LineBuffer;
  delete [] FormatString;

  // Step Three: Sort them:
  int *Index = new int[m_NEntries];
  TMath::Sort(m_NEntries, m_X, Index, false);
  //Error("MCSVFunctionLoader::Load()", "Something is missing!!");

  double TempX, TempY;
  int TempIndex;
  for (counter = 0; counter < m_NEntries; counter++) {
    while (Index[counter] != counter) {
      // swap X:
      TempX = m_X[Index[counter]];
      m_X[Index[counter]] = m_X[counter];
      m_X[counter] = TempX;
      
      // swap Y:
      TempY = m_Y[Index[counter]];
      m_Y[Index[counter]] = m_Y[counter];
      m_Y[counter] = TempY;
      
      // swap Index:
      TempIndex = Index[Index[counter]];
      Index[Index[counter]] = Index[counter];
      Index[counter] = TempIndex;
    }
  }
  
  delete [] Index;

  return true;
}



//////////////////////////////////////////////////////////////////////////////////
//
// 
//
double MCSVFunctionLoader::GetY(double x)
{
  int IndexPosition;
  double m, t;

	//cout<<m_NEntries<<"!"<<m_X<<"!"<<x<<endl;
  IndexPosition = (int) TMath::BinarySearch(m_NEntries, m_X, x);
  //Error("MCSVFunctionLoader::GetY()", "Something is missing!!");

  if (IndexPosition == m_NEntries - 1) {
    // Now extrapolate:
    m = (m_Y[IndexPosition] - m_Y[IndexPosition-1])/(m_X[IndexPosition] - m_X[IndexPosition-1]);
    if (m == 0)
      t = m_Y[IndexPosition];
    else
      t = m_Y[IndexPosition] - m*m_X[IndexPosition];

    return m*x+t;
  }
  
  if (IndexPosition == - 1) {
    // Now extrapolate:
    m = (m_Y[0] - m_Y[1])/(m_X[0] - m_X[1]);
    if (m == 0)
      t = m_Y[0];
    else
      t = m_Y[0] - m*m_X[0];

    return m*x+t;
  }

  // Now interpolate:
  return m_Y[IndexPosition + 1] - (m_X[IndexPosition + 1]-x)/
    (m_X[IndexPosition + 1] - m_X[IndexPosition])*
    (m_Y[IndexPosition + 1] - m_Y[IndexPosition]);
}




//////////////////////////////////////////////////////////////////////////////////
//
// 
//
void MCSVFunctionLoader::Sort()
{

}


//////////////////////////////////////////////////////////////////////////////////
//
// 
//


//
//////////////////////////////////////////////////////////////////////////////////
