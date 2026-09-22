/*
 * MReadOutFileFormat.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutFileFormat__
#define __MReadOutFileFormat__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The file format of how read out units are stored on file
//!
//!   UF UH doublesidedstrip adcwithtiming
//!   UF UC detector adc
//!   SE
//!   UH 4 12 l 1111 2222
//!   UC 7 2345
//!
//! The original single-unit form "UF <read-out-element> <read-out-data>" is accepted and defaults to UH
//! Read-out keywords start with "U" (uncalibrated) followed by letters or digits, but not "UF"
class MReadOutFileFormat
{
  // public interface:
 public:
  //! Default constructor
  MReadOutFileFormat();
  //! Default destructor
  virtual ~MReadOutFileFormat();

  //! Clear all data
  void Clear();

  //! Return the number of different read-out units
  unsigned int GetNumberOfReadOutUnits() const { return m_ReadOutKeywords.size(); }

  //! Add a definition of a read-out unit
  //! Returns false if the keyword is invalid or already in use
  bool AddReadOutUnit(const MString& ReadOutKeyword, const MString& ReadOutElementType, const MString& ReadOutDataType);
  //! Add a definition of a read-out unit without keyword
  //! That will use "UH" as the first keyword, "U1", "U2" for latter
  bool AddReadOutUnit(const MString& ReadOutElementType, const MString& ReadOutDataType);

  //! Parse the "UF" line and store the definition
  //! Accepts "UF <element> <data>" and "UF <keyword> <element> <data>"
  bool ParseUF(const MString& Line);

  //! Return the index of the read-out unit of this line, or g_UnsignedIntNotDefined if none is found
  unsigned int FindByLine(const MString& Line) const;
  //! Return the index of the read-out unit defined by these types, or g_UnsignedIntNotDefined if none is found
  unsigned int FindByTypes(const MString& ReadOutElementType, const MString& ReadOutDataType) const;
  //! Return the index of the read-out unit with this read-out keyword, or g_UnsignedIntNotDefined if it does not exist
  unsigned int FindByKeyword(const MString& ReadOutKeyword) const;

  //! Return the keyword of read-out unit i
  const MString& GetKeyword(unsigned int i) const;
  //! Return the read-out element type of read-out unit i
  const MString& GetElementType(unsigned int i) const;
  //! Return the read-out data type of read-out unit i
  const MString& GetDataType(unsigned int i) const;

  //! Return the UF line of read-out unit i
  MString GetUFLine(unsigned int i) const;

  //! Dump a the content of this read-out file format
  MString ToString() const;

  //! Return true if this is a valid read-out keyword: "U" followed by letters or digits, not starting with "UF"
  static bool IsValidKeyword(const MString& Keyword);

  //! The keyword of the original single-unit format, "UH"
  static const MString c_DefaultKeyword;


  // protected methods:
 protected:

  // private methods:
 private:
  //! Create a new keyword in this order: "UH", "U1", "U2", ...
  MString CreateKeyword() const;


  // protected members:
 protected:


  // private members:
 private:
  //! The keyword starting the read-out lines
  vector<MString> m_ReadOutKeywords;
  //! The read-out element type of each unit
  vector<MString> m_ReadOutElementTypes;
  //! The read-out data type of each unit
  vector<MString> m_ReadOutDataTypes;


#ifdef ___CLING___
 public:
  ClassDef(MReadOutFileFormat, 0) // Description of the read-out file format
#endif

};

//! Streamify the read-out format
ostream& operator<<(ostream& os, const MReadOutFileFormat& R);

#endif


////////////////////////////////////////////////////////////////////////////////
