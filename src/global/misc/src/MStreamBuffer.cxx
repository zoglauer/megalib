/*
 * MStreamBuffer.cxx
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
// MStreamBuffer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStreamBuffer.h"

// Standard libs:
#include <iostream>
#include <cstdlib>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TGClient.h>
#include <TGMsgBox.h>

// MEGAlib libs:
#include "MAssert.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStreamBuffer)
#endif


////////////////////////////////////////////////////////////////////////////////


MStreamBuffer::MStreamBuffer() : streambuf()
{
  int bsize = 100;
  
  if (bsize > 0) {
    char *ptr = new char[bsize];
    setp(ptr, ptr + bsize);
  } else {
    setp(0, 0); 
  }

  setg(0, 0, 0);

  m_IsHeaderShown = false;
  m_IsFirst = false;
  m_Enabled = true;
  m_StdOut = true;
  m_StdErr = false;
  m_Gui = false;
  
  m_ShowNotShown = false;
  m_GuiType = c_Info;
  m_ShowOnce = false;
}


////////////////////////////////////////////////////////////////////////////////


MStreamBuffer::~MStreamBuffer()
{
  sync();
  delete[] pbase();
}


////////////////////////////////////////////////////////////////////////////////


int MStreamBuffer::overflow(int c)
{
  put_buffer();
  
  if (c != EOF) {
    if (pbase() == epptr()) {
      put_char(c);
    } else {
      sputc(c);
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MStreamBuffer::sync()
{
  put_buffer();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::put_char(int)
{
  cout<<"Put char!"<<std::endl<<flush;
}


////////////////////////////////////////////////////////////////////////////////


void  MStreamBuffer::put_buffer()
{
  if (pbase() != pptr()) {
    int len = (pptr() - pbase());
    char* buffer = new char[len + 1];
      
    strncpy(buffer, pbase(), len);
    buffer[len] = 0;

    if (m_Enabled == true) {
      // First generate the stream:
      ostringstream out;
      if (m_Header != "" && m_IsHeaderShown == false) {
        out<<m_Header<<std::endl<<m_Prefix<<flush;
        m_IsHeaderShown = true;
      }
      if (m_IsFirst == true && len > 0) {
        out<<m_Prefix;
      }

      char last = '\0';
      for (int i = 0; i < len+1; ++i) {
        if (last == '\n' && buffer[i] != '\0') {
          if (buffer[i] != '\0') {
            out<<m_Prefix;
          }
        }
        if (last == '\n' && buffer[i] == '\0') {
          m_IsFirst = true;
        } else {
          m_IsFirst = false;
        }
        last = buffer[i];
        if (buffer[i] != '\0') {
          out<<last;
        }
      }
   

      // Then write it:
      if (m_StdOut == true) {
        cout<<out.str()<<flush;
      }
      if (m_StdErr == true) {
        cerr<<out.str()<<flush;
      }
      
      for (unsigned int i = 0; i < m_FileStream.size(); i++) {
        if (m_FileTimePrefix[i] == true) {
          MTime Now;
          *(m_FileStream[i])<<Now.GetShortString()<<":  ";
        }
        *(m_FileStream[i])<<out.str();
        m_FileStream[i]->flush();
      }    
      
      if (m_Gui == true) {
        // We only dump the GUI string, if we have a wanr, info, error, etc. flag
        // So simply store the string for the moment...
        m_NotShownString += out.str().c_str();

        // We have to remove all string end marks - if there are any...
        m_NotShownString.ReplaceAll("\0", "");
        
        if (m_ShowNotShown == true) {
          
          // Remove the last return...
          if (m_NotShownString.EndsWith("\n") == true) {
            m_NotShownString.Remove(m_NotShownString.Length()-1);
          }
          m_NotShownString += '\0';
          
          if (gClient != 0 && gClient->GetRoot() != 0 && gROOT->IsBatch() == false) {
            if (m_GuiType == c_Warn) {
              new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), 
                           "Warning", m_NotShownString, kMBIconExclamation, kMBOk);
            } else if (m_GuiType == c_Error) {
              new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), 
                           "Error", m_NotShownString, kMBIconStop, kMBOk);
            } else {
              new TGMsgBox(gClient->GetRoot(), gClient->GetRoot(), 
                           "Info", m_NotShownString, kMBIconAsterisk, kMBOk);
            }
          }
        }
      }


      if (m_ShowNotShown == true) {
        m_NotShownString = "";
      }
    }

    setp(pbase(), epptr());
    delete [] buffer;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::endl()
{
  sputc('\n');
  sync();
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::fatal()
{
  endl();
  abort();
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::show(int Type)
{
  sputc('\n');
  m_GuiType = Type;
  m_ShowNotShown = true;
  sync();
  m_ShowNotShown = false;
  m_IsHeaderShown = false;
  m_IsFirst = false;
  m_GuiType = c_Info;
  if (m_ShowOnce == true) m_Enabled = false;
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::logonly()
{
  bool StdOut = m_StdOut;
  bool StdErr = m_StdErr;
  bool Gui = m_Gui;

  m_StdOut = false;
  m_StdErr = false;
  m_Gui = false;

  show();

  m_StdOut = StdOut;
  m_StdErr = StdErr;
  m_Gui = Gui;
}


////////////////////////////////////////////////////////////////////////////////


void MStreamBuffer::SetRejection(MString Rejection)
{
  bool Found = false;
  list<MString>::iterator Iter;
  for (Iter = m_Rejections.begin(); Iter != m_Rejections.end(); ++Iter) {
    if ((*Iter) == Rejection) {
      Found = true;
      break;
    }
  }
  
  if (Found == true) {
    m_Enabled = false;
  } else {
    m_Enabled = true;
    m_Rejections.push_front(Rejection);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MStreamBuffer::Connect(MString FileName, bool Append, bool TimePrefix)
{
  if (m_Enabled == false) return false;

  // Check if we are already connected:
  for (unsigned int i = 0; i < m_FileStream.size(); i++) {
    if (m_FileNames[i] == FileName) {
      return false;
    }
  }
  
  ofstream* FileStream = new ofstream();
  if (Append == true) {
    FileStream->open(FileName, ios_base::app);
  }

  // If append is false or we have been unable to open the because it does not exist...
  if (FileStream->is_open() == 0) {
    FileStream->open(FileName, ios_base::out);
    if (FileStream->is_open() == 0) {
      cout<<"MLog::Connect: Error: Can't open file "<<FileName<<std::endl<<flush;
      FileStream = 0;
      return false;
    }
  }

  // Store the data:
  m_FileStream.push_back(FileStream);
  m_FileNames.push_back(FileName);
  m_FileTimePrefix.push_back(TimePrefix);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MStreamBuffer::Disconnect(MString FileName)
{
  if (m_Enabled == false) return false;

  // Check if the name is stored and we are connected
  int pos = -1;
  for (unsigned int i = 0; i < m_FileNames.size(); ++i) {
    if (m_FileNames[i] == FileName) {
      pos = i;
      break;
    }
  }

  if (pos == -1) {
    // We are not connected so return!
    return false;
  }

  m_FileStream[pos]->close();
  m_FileStream[pos] = 0;
  
  // Now shrink the array:
  if (pos != (int) m_FileStream.size()-1) {
    m_FileStream[pos] = m_FileStream[m_FileStream.size()-1];
    m_FileNames[pos] = m_FileNames[m_FileStream.size()-1];
    m_FileTimePrefix[pos] = m_FileTimePrefix[m_FileStream.size()-1];
  }
  m_FileStream.resize(m_FileStream.size()-1);
  m_FileNames.resize(m_FileNames.size()-1);
  m_FileTimePrefix.resize(m_FileNames.size()-1);

  return true;
}


// MStreamBuffer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
