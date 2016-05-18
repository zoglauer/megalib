/*
 * MGUIEMinMaxEntry.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIEMinMaxEntry__
#define __MGUIEMinMaxEntry__


////////////////////////////////////////////////////////////////////////////////


// Standard libs::
#include <limits>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <MString.h>
#include <TGTextEntry.h>
#include <TGTextBuffer.h>
#include <TGGC.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIElement.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIEMinMaxEntry : public MGUIElement
{
  // public interface:
public:
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   double MinValue, double MaxValue,
                   bool Limits = true, 
                   double Min = -numeric_limits<double>::max()/2, 
                   double Max = numeric_limits<double>::max()/2);
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   MString MinLabel, MString MaxLabel, 
                   double MinValue, double MaxValue,
                   bool Limits = true, 
                   double Min = -numeric_limits<double>::max()/2, 
                   double Max = numeric_limits<double>::max()/2);
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   int MinValue, int MaxValue,
                   bool Limits = true, 
                   int Min = -numeric_limits<int>::max()/2, 
                   int Max = numeric_limits<int>::max()/2);
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   MString MinLabel, MString MaxLabel, 
                   int MinValue, int MaxValue,
                   bool Limits = true, 
                   int Min = -numeric_limits<int>::max()/2, 
                   int Max = numeric_limits<int>::max()/2);
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   long MinValue, long MaxValue,
                   bool Limits = true, 
                   long Min = -numeric_limits<long>::max()/2, 
                   long Max = numeric_limits<long>::max()/2);
  MGUIEMinMaxEntry(const TGWindow* Parent, MString Label, bool Emphasize, 
                   MString MinLabel, MString MaxLabel, 
                   long MinValue, long MaxValue,
                   bool Limits = true, 
                   long Min = -numeric_limits<long>::max()/2, 
                   long Max = numeric_limits<long>::max()/2);
  virtual ~MGUIEMinMaxEntry();
  
  bool CheckRange(double MinMin, double MinMax, 
                  double MaxMin, double MaxMax, 
                  bool MinLMax = true);
  
  bool CheckRange(long MinMin, long MinMax, 
                  long MaxMin, long MaxMax, 
                  bool MinLMax = true);

  bool CheckRange(int MinMin, int MinMax, int MaxMin, int MaxMax, bool MinLMax = true) {
    return CheckRange((long) MinMin, (long) MinMax, (long) MaxMin, (long) MaxMax,MinLMax);
  }
  
  double GetMinValue() { return GetMinValueDouble(); } 
  double GetMaxValue() { return GetMaxValueDouble();};
  double GetMinValueDouble();
  double GetMaxValueDouble();
  long GetMinValueInt();
  long GetMaxValueInt();
  
  void SetEntryFieldSize(int Size = 70);
  void SetEnabled(bool flag = true);
  
  //! Return true if the content has been modified 
  bool IsModified();
  
  
  // protected methods:
protected:
  void Init();
  
  // private methods:
private:
  void Create(int Mode);
  MString MakeSmartString(double Number);
  bool CheckForNumber(MString Number);
  double ToNumber(MString Number);
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  MString m_Label;
  
  MString m_MinLabel;
  MString m_MaxLabel;
  
  double m_MinValue;
  double m_MaxValue;
  
  int m_Type;
  bool m_Limits;
  double m_Min;
  double m_Max;
  int m_Size;
  
  TGLabel* m_TextLabel;
  TGLayoutHints* m_TextLabelLayout;
  
  TGLayoutHints* m_EntryLayout;
  MGUIEEntry* m_MinEntry;
  MGUIEEntry* m_MaxEntry;
  
  enum Type { e_Integer, e_Double };
  
  
  #ifdef ___CINT___
public:
  ClassDef(MGUIEMinMaxEntry, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
